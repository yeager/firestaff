/*
 * dm2_v1_world_model.c — DM2 V1 Dungeon/World Data Model Implementation
 *
 * DM2 Phase 2: Complete DM2 map, object, tile, and world-state ingestion.
 * Parses DM2 DUNGEON.DAT into an in-memory dm2_dungeon_world_t.
 *
 * Binary format:
 *   Files with FTL signature (0x8104 at byte 0):
 *     12-byte COMPRESSED_DUNGEON_HEADER at byte 0
 *     20-byte lookup table + compressed bitstream at byte 12
 *     ftl_decompress_dungeon() decodes to 'decompressed_bytes' output
 *     Output contains: lookup table (20 bytes) + FTL bitstream +
 *                      12 bytes padding + 44-byte DUNGEON_HEADER
 *   Files without FTL signature: raw pre-decompressed data
 *     First 44 bytes = DUNGEON_HEADER, followed by MAP desc + tile data
 *
 * Source: SKULL.ASM T560 DUNGEON_Load, SKULL.ASM T000 (file I/O),
 *         ReDMCSB DEFS.H:985-998 (DUNGEON_HEADER),
 *         ReDMCSB DEFS.H:1048-1116 (MAP descriptor 16 bytes),
 *         ReDMCSB DECOMPDU.C F0455_FLOPPY_DecompressDungeon
 */

#include "dm2_v1_world_model.h"
#include "dm2_v1_dungeon_loader.h"
#include "dungeon_decompressor_ftl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

/* ── Internal helpers ──────────────────────────────────────────────── */

/*
 * DM2 FTL format note:
 *   The FTL decompression function expects compressed data with a 20-byte
 *   lookup table at the start. The compressed data stored in DUNGEON.DAT
 *   can be in two forms:
 *
 *   (A) FTL-wrapped (byte 0 == 0x81, byte 1 == 0x04):
 *       The file contains: [COMPRESSED_HEADER(12)] + [lookup_table(20)] +
 *                          [compressed_bitstream]
 *       Caller skips the 12-byte header and feeds byte 12 onward to
 *       ftl_decompress_dungeon(lookup_table_at_data, data_size-12,
 *                              decomp_buf, decompressed_size).
 *
 *   (B) Pre-decompressed (no FTL wrapper):
 *       First 44 bytes are DUNGEON_HEADER. Data is already decompressed.
 *
 *   DM2 PC English DUNGEON.DAT (39,437 bytes) appears to be pre-decompressed
 *   (no 0x8104 at byte 0). Many DM2 variants are distributed pre-decompressed
 *   to avoid runtime decompression overhead.
 *
 * Source: dm2_v1_dungeon_loader.c PROBE_NOTES, SKULL.ASM T000
 */
#define DM2_FTL_WRAPPED(d, sz) ((sz) >= 2 && (d)[0] == 0x81 && (d)[1] == 0x04)

static uint16_t dm2_rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t dm2_rd32be(const uint8_t *p) {
    /* Big-endian uint32 (FTL convention) */
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] <<  8) | ((uint32_t)p[3]);
}

/* Extract type byte from DM2 dungeon format level descriptors.
 * In the DM2-specific format, levels are described by 8-byte entries:
 *   byte[0]: level_type (DM2_LEVELTYPE_*)
 *   byte[1]: width
 *   byte[2]: height
 *   byte[4-5]: offset_low (LE uint16)
 *   byte[6-7]: offset_high (LE uint16)
 *
 * Source: include/dm2_v1_dungeon_loader.h PROBE_NOTES, SKULL.ASM T560 */

/* ── Tile parsing ──────────────────────────────────────────────────── */

/*
 * dm2_parse_tile — unpack a 16-bit DM2 tile value into dm2_tile_t.
 *
 * Lower 5 bits: square type (DM2_SQUARE_*)
 * bit 5 (0x0020): wall blocks movement
 * bit 6 (0x0040): corrupted tile
 * bit 7 (0x0080): above-tile connection
 *
 * Source: ReDMCSB HASHBUCKET.C, DEFS.H:385-390
 */
static dm2_tile_t dm2_parse_tile(uint16_t raw) {
    dm2_tile_t t;
    t.raw   = raw;
    t.type  = (uint8_t)(raw & DM2_SQUARE_TYPE_MASK);
    t.flags = (uint8_t)((raw & ~DM2_SQUARE_TYPE_MASK) >> 5);
    return t;
}

/* ── World model builder ────────────────────────────────────────────── */

/*
 * dm2_build_world_from_decompressed —
 *   Build world model from a post-FTL-decompression buffer.
 *
 * The decompressed buffer layout:
 *   [20 bytes lookup table] + [FTL bitstream output] +
 *   [12 bytes padding] + [Dungeon Header 44 bytes] + [Map Descriptors] +
 *   [Tile data] + [Thing data] + [Text data]
 *
 * But the actual FTLDecompress output places the LOOKUP table at the
 * START of the output buffer. So:
 *   offset 0..19:  lookup table (DM1 legacy — not used after decomp)
 *   offset 20..63: 44 bytes of padding / metadata
 *   offset 44+:    DUNGEON_HEADER + map descriptors + map data + ...
 *
 * DM2 DM1 compatibility: the DUNGEON_HEADER starts at offset 44
 * in the decompressed buffer (since FTLDecompress writes 20 bytes of
 * lookup table first, then game data).
 *
 * For pre-decompressed files (no FTL wrapper), the layout is:
 *   offset 0: DUNGEON_HEADER (44 bytes)
 *   offset 44+: MAP descriptors + tile data
 *
 * We detect which layout we have by checking for the DM1/DM2 magic.
 * If decompressed_bytes == file_size with no FTL sig → pre-decompressed.
 *
 * Source: SKULL.ASM T560 DUNGEON_Load, ReDMCSB FTL.H
 */
static void dm2_parse_header(const dm2_dungeon_header_t *hdr,
                               dm2_dungeon_world_t      *world) {
    world->header          = *hdr;
    world->map_count       = hdr->map_count;
    world->text_word_count = dm2_rd16le((const uint8_t *)&hdr->text_data_word_count);
    /* Things: copy counts */
    for (int i = 0; i < DM2_MAX_THING_TYPES; i++) {
        world->thing_pool_counts[i] = dm2_rd16le((const uint8_t *)&hdr->thing_count[i]);
    }
}

/* ── Public API implementation ──────────────────────────────────────── */

dm2_dungeon_world_t *dm2_world_from_mem(const uint8_t *data, size_t size) {
    dm2_dungeon_world_t *world;
    const uint8_t *decoded;
    size_t decoded_size;

    if (!data || size < DM2_DUNGEON_HEADER_SIZE)
        return NULL;

    world = calloc(1, sizeof(dm2_dungeon_world_t));
    if (!world)
        return NULL;

    /* ── FTL decompression if needed ──────────────────────── */
    if (DM2_FTL_WRAPPED(data, size)) {
        /* FTL-wrapped: 12-byte header + lookup table starts at data+12 */
        uint32_t decomp_bytes = dm2_rd32be(data + 2);
        if (decomp_bytes == 0 || decomp_bytes > 16*1024*1024) {
            free(world);
            return NULL;
        }

        uint8_t *decomp_buf = malloc((size_t)decomp_bytes);
        if (!decomp_buf) {
            free(world);
            return NULL;
        }

        /* Feed compressed data starting at lookup table (data+12).
         * ftl_decompress_dungeon uses the first 20 bytes as lookup table,
         * then the rest as compressed bitstream.
         * Source: ReDMCSB DECOMPDU.C F0455 */
        int ok = ftl_decompress_dungeon(data + 12, size - 12,
                                        decomp_buf, (long)decomp_bytes);
        if (!ok) {
            free(decomp_buf);
            free(world);
            return NULL;
        }

        world->raw_decompressed     = decomp_buf;
        world->raw_decompressed_size = (size_t)decomp_bytes;
        decoded     = decomp_buf;
        decoded_size = (size_t)decomp_bytes;

        /* In FTL-wrapped files, DUNGEON_HEADER starts at byte 44
         * of the decompressed buffer (after 20-byte lookup + 24-byte pad).
         * Source: ReDMCSB FTL.H, SKULL.ASM T560 */
        if (decoded_size < 44) {
            free(decomp_buf);
            free(world);
            return NULL;
        }
        decoded += 44;

    } else {
        /* Pre-decompressed — data starts directly with DUNGEON_HEADER */
        decoded      = data;
        decoded_size = size;
    }

    /* ── Parse DUNGEON_HEADER (44 bytes, same as DM1) ─────── */
    if (decoded_size < DM2_DUNGEON_HEADER_SIZE) {
        if (world->raw_decompressed) free(world->raw_decompressed);
        free(world);
        return NULL;
    }

    dm2_parse_header((const dm2_dungeon_header_t *)decoded, world);
    decoded      += DM2_DUNGEON_HEADER_SIZE;
    decoded_size -= DM2_DUNGEON_HEADER_SIZE;

    /* ── Read map descriptors (16 bytes each) ─────────────── */
    /* DM2 uses the 16-byte MAP descriptor format (same as DM1).
     * Source: ReDMCSB DEFS.H:1048-1116, SKULL.ASM T560 */
    int mc = world->map_count;
    if (mc > DM2_MAX_LEVELS) mc = DM2_MAX_LEVELS;

    for (int i = 0; i < mc; i++) {
        if (decoded_size < DM2_MAP_DESC_SIZE) break;
        memcpy(&world->map_descs[i], decoded, DM2_MAP_DESC_SIZE);
        decoded      += DM2_MAP_DESC_SIZE;
        decoded_size -= DM2_MAP_DESC_SIZE;
    }

    /* ── Parse tile data for each level ────────────────────── */
    /*
     * DM2 uses override width/height at MAP descriptor bytes[12-15]
     * (same as dm2_v1_dungeon_loader.c override path).
     * Fall back to DM1 bitfield_a only when overrides are both 0.
     * Source: SKULL.ASM T560 — override vs bitfield_a fallback.
     *
     * Level type: stored in offset_map_x (MAP descriptor byte 6, lower 2 bits).
     * Source: docs/dm2_v1_phase2_data_formats_H2254.md §2, SKULL.ASM T600 outdoor.
     *
     * Tile data: column-major uint16[level_width * level_height].
     * Source: ReDMCSB DEFS.H, SKULL.ASM T520 tile access.
     */
    for (int i = 0; i < mc; i++) {
        const dm2_map_descriptor_t *md = &world->map_descs[i];

        /* DM2 override width/height at bytes[12-13] and [14-15] (LE uint16). */
        uint16_t w_override = dm2_rd16le((const uint8_t *)md + 12);
        uint16_t h_override = dm2_rd16le((const uint8_t *)md + 14);

        int w, h;
        if (w_override != 0 || h_override != 0) {
            w = (w_override > 0 && w_override <= 256) ? (int)w_override : 64;
            h = (h_override > 0 && h_override <= 256) ? (int)h_override : 64;
        } else {
            w = dm2_md_width(md);
            h = dm2_md_height(md);
        }

        world->levels[i].width  = w;
        world->levels[i].height = h;
        world->levels[i].level_index = i;

        /* Level type from MAP descriptor offset_map_x (byte 6, lower 2 bits).
         * 0=OUTDOOR, 1=INDOOR, 2=BUILDING. Level 0 is always OUTDOOR.
         * Source: docs/dm2_v1_phase2_data_formats_H2254.md §2 */
        if (i == 0) {
            world->levels[i].level_type = DM2_LEVEL_OUTDOOR;
        } else {
            world->levels[i].level_type = ((int)(md->offset_map_x & 0x03) <= 2)
                                            ? (int)(md->offset_map_x & 0x03)
                                            : DM2_LEVEL_INDOOR;
        }

        /* byte_offset: raw_map_data_byte_offset from MAP descriptor bytes[0-1]. */
        world->levels[i].byte_offset = (int)dm2_rd16le((const uint8_t *)md);

        /* Each tile is 2 bytes (LE uint16), column-major: (x*height+y)*2.
         * Source: dm2_v1_dungeon_loader.c column-major tile access formula. */
        size_t tile_bytes = (size_t)w * (size_t)h * 2;
        if ((size_t)decoded_size < tile_bytes) break;
        world->levels[i].tiles = malloc((size_t)w * h * sizeof(dm2_tile_t));
        if (!world->levels[i].tiles) break;
        for (int x = 0; x < w; x++) {
            for (int y = 0; y < h; y++) {
                uint16_t raw = dm2_rd16le(decoded + ((x * h) + y) * 2);
                world->levels[i].tiles[y * w + x] = dm2_parse_tile(raw);
            }
        }
        decoded      += tile_bytes;
        decoded_size -= tile_bytes;
    }

    /* ── Text data ────────────────────────────────────────── */
    if (decoded_size >= 2) {
        world->text_word_count = decoded_size / 2;
        world->text_data = malloc(world->text_word_count * 2);
        if (world->text_data) {
            memcpy(world->text_data, decoded, world->text_word_count * 2);
        }
    }

    return world;
}

dm2_dungeon_world_t *dm2_world_from_file(const char *path) {
    FILE *f;
    uint8_t *raw;
    long fsize;
    dm2_dungeon_world_t *world;

    if (!path) return NULL;

    f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "DM2 world: cannot open %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || fsize > 16*1024*1024) {
        fclose(f);
        return NULL;
    }

    raw = malloc((size_t)fsize);
    if (!raw) {
        fclose(f);
        return NULL;
    }
    if (fread(raw, 1, (size_t)fsize, f) != (size_t)fsize) {
        free(raw);
        fclose(f);
        return NULL;
    }
    fclose(f);

    world = dm2_world_from_mem(raw, (size_t)fsize);
    free(raw);
    return world;
}

const dm2_tile_t *dm2_world_get_tile(const dm2_dungeon_world_t *world,
                                      int level, int x, int y) {
    if (!world) return NULL;
    if (level < 0 || level >= world->map_count) return NULL;
    const dm2_level_t *lv = &world->levels[level];
    if (!lv->tiles) return NULL;
    if (x < 0 || x >= lv->width) return NULL;
    if (y < 0 || y >= lv->height) return NULL;
    return &lv->tiles[y * lv->width + x];
}

int dm2_world_get_tile_type(const dm2_dungeon_world_t *world,
                             int level, int x, int y) {
    const dm2_tile_t *t = dm2_world_get_tile(world, level, x, y);
    return t ? (int)t->type : DM2_SQUARE_COUNT;
}

int dm2_world_is_walkable(const dm2_dungeon_world_t *world,
                           int level, int x, int y) {
    const dm2_tile_t *t = dm2_world_get_tile(world, level, x, y);
    if (!t) return 0;

    /* Blocked tile types in DM2 are same as DM1 */
    switch (t->type) {
        case DM2_SQUARE_WALL:
        case DM2_SQUARE_INACCESSIBLE:
        case DM2_SQUARE_PIT:
        case DM2_SQUARE_LAVA:
            return 0;
        default:
            return 1;
    }
}

int dm2_world_is_outdoor(const dm2_dungeon_world_t *world, int level) {
    if (!world || level < 0 || level >= world->map_count) return 0;
    return world->levels[level].level_type == 0;
}

void dm2_world_free(dm2_dungeon_world_t *world) {
    if (!world) return;
    for (int i = 0; i < DM2_MAX_LEVELS; i++) {
        if (world->levels[i].tiles) {
            free(world->levels[i].tiles);
            world->levels[i].tiles = NULL;
        }
    }
    if (world->text_data) {
        free(world->text_data);
        world->text_data = NULL;
    }
    if (world->raw_decompressed) {
        free(world->raw_decompressed);
        world->raw_decompressed = NULL;
    }
    free(world);
}

const char *dm2_world_source_evidence(void) {
    return
        "DM2 V1 World Model — Phase 2 Data Ingestion\n"
        "Source: SKULL.ASM T560 DUNGEON_Load — header parsing, level descriptors\n"
        "Source: SKULL.ASM T000 — DM2 startup / file I/O\n"
        "Source: SKULL.ASM T520 — party placement, initial position\n"
        "Source: SKULL.ASM T600 — outdoor tick, level type distinction\n"
        "Source: ReDMCSB DEFS.H:985-998 — DUNGEON_HEADER (44 bytes)\n"
        "Source: ReDMCSB DEFS.H:1048-1116 — MAP descriptor (16 bytes)\n"
        "Source: ReDMCSB HASHBUCKET.C — square type constants, tile bitfields\n"
        "Source: ReDMCSB DECOMPDU.C F0455_FLOPPY_DecompressDungeon — FTL decomp\n"
        "Source: docs/dm2_v1_phase2_data_formats_H2254.md (SHA256 source-lock)\n"
        "Source: docs/dm2_dungeon_files.md — DUNGEON.DAT format audit\n"
        "Asset: DM2 PC English DUNGEON.DAT 6caccd7875009e82fe2e28e7f6d6adc0 (39,437 bytes)\n";
}
