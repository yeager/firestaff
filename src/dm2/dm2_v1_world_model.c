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

    {
        DM2_V1_DungeonData loaded;
        if (dm2_v1_dungeon_load(&loaded, decoded, (int)decoded_size) == 0 &&
            loaded.square_bytes == 1) {
            int mc = loaded.level_count;
            int tiles_complete = 1;
            if (mc > DM2_MAX_LEVELS) mc = DM2_MAX_LEVELS;

            world->map_count = mc;
            world->text_word_count = loaded.text_word_count;
            world->dungeon_seed = dm2_rd16le(decoded + 8);
            for (int i = 0; i < DM2_MAX_THING_TYPES; ++i)
                world->thing_pool_counts[i] = loaded.thing_type_counts[i];
            for (int i = 0; i < mc; ++i) {
                if ((size_t)(DM2_DUNGEON_HEADER_SIZE +
                             i * DM2_MAP_DESC_SIZE + DM2_MAP_DESC_SIZE) <=
                    decoded_size) {
                    memcpy(&world->map_descs[i],
                           decoded + DM2_DUNGEON_HEADER_SIZE +
                               i * DM2_MAP_DESC_SIZE,
                           DM2_MAP_DESC_SIZE);
                }
                world->levels[i].width = loaded.level_widths[i];
                world->levels[i].height = loaded.level_heights[i];
                world->levels[i].level_index = i;
                world->levels[i].level_type = loaded.level_types[i];
                world->levels[i].byte_offset = loaded.level_offsets[i];
                if (loaded.level_widths[i] > 0 &&
                    loaded.level_heights[i] > 0) {
                    size_t count = (size_t)loaded.level_widths[i] *
                                   (size_t)loaded.level_heights[i];
                    world->levels[i].tiles =
                        (dm2_tile_t *)malloc(count * sizeof(dm2_tile_t));
                    if (!world->levels[i].tiles) {
                        tiles_complete = 0;
                        break;
                    }
                    for (int y = 0; y < loaded.level_heights[i]; ++y) {
                        for (int x = 0; x < loaded.level_widths[i]; ++x) {
                            int raw = dm2_v1_dungeon_get_tile_raw(
                                &loaded, i, x, y);
                            int type = dm2_v1_dungeon_get_square_type(
                                &loaded, i, x, y);
                            dm2_tile_t tile;
                            tile.raw = (uint16_t)(raw < 0 ? 0 : raw);
                            tile.type = (uint8_t)(type < 0 ? 0 : type);
                            tile.flags = (uint8_t)
                                (loaded.square_bytes == 1
                                     ? (tile.raw & 0x1fu)
                                     : ((tile.raw &
                                         ~DM2_SQUARE_TYPE_MASK) >> 5));
                            world->levels[i].tiles
                                [y * loaded.level_widths[i] + x] = tile;
                        }
                    }
                }
            }
            if (!tiles_complete) {
                dm2_v1_dungeon_free(&loaded);
                dm2_world_free(world);
                return NULL;
            }

            /* Keep the source-locked G1 map/c_record address receipt alive
             * for future world consumers.  This transfer retains raw bytes
             * and exact pool bases, but does not make GenericRecord::w0
             * traversal available.  See skproject c_record.cpp
             * DM2_GET_ADDRESS_OF_RECORD and SkWinCore.cpp
             * READ_DUNGEON_STRUCTURE. */
            world->g1_record_pool_addresses_valid =
                loaded.square_bytes == 1 &&
                loaded.partial_map_boot.committed == 1 &&
                loaded.partial_map_boot.incomplete == 1 &&
                dm2_v1_dungeon_validate_record_pools(&loaded);
            world->g1_record_graph_complete = loaded.record_graph_complete;
            world->source_dungeon = loaded;
            world->source_dungeon_valid = 1;
            memset(&loaded, 0, sizeof(loaded));
            /* DM2-002: move record ownership into the source-ordered
             * c_record pool model (skproject c_record.cpp/c_dballoc.cpp).
             * The parallel reduced structures stay for tile facts only;
             * record bytes, links, and relocation now live in the owned
             * pool set.  Population stays fail-closed: an unvalidated G1
             * span leaves record_pools_valid == 0. */
            world->record_pools_valid =
                dm2_v1_record_pool_set_init_from_world(
                    &world->record_pools, world);
            return world;
        }
    }

    /* SKProject READ_DUNGEON_STRUCTURE owns the PC G1 byte-square layout.
     * The code below used to infer a 16-bit map descriptor/pool layout after
     * the source loader rejected the input.  That can turn arbitrary bytes
     * into a playable-looking world, so it is not a legitimate fallback.
     * Retain only the decoded G1 route above. */
    if (world->raw_decompressed) {
        free(world->raw_decompressed);
    }
    free(world);
    return NULL;

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

const DM2_V1_DungeonData *dm2_world_get_verified_g1_map_source(
    const dm2_dungeon_world_t *world) {
    if (!world || !world->source_dungeon_valid ||
        !world->g1_record_pool_addresses_valid ||
        world->source_dungeon.square_bytes != 1 ||
        !world->source_dungeon.partial_map_boot.committed ||
        !world->source_dungeon.partial_map_boot.incomplete) {
        return NULL;
    }
    return &world->source_dungeon;
}

int dm2_world_has_verified_g1_record_pools(const dm2_dungeon_world_t *world) {
    return dm2_world_get_verified_g1_map_source(world) != NULL;
}

int dm2_v1_record_pool_set_init_from_world(DM2_V1_RecordPoolSet *set,
                                           const dm2_dungeon_world_t *world)
{
    return dm2_v1_record_pool_set_init_from_dungeon(
        set, dm2_world_get_verified_g1_map_source(world));
}

const DM2_V1_RecordPoolSet *dm2_world_get_record_pools(
    const dm2_dungeon_world_t *world) {
    if (!world || !world->record_pools_valid) return NULL;
    return &world->record_pools;
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
    if (world->source_dungeon_valid) {
        dm2_v1_dungeon_free(&world->source_dungeon);
        world->source_dungeon_valid = 0;
    }
    if (world->record_pools_valid) {
        dm2_v1_record_pool_set_free(&world->record_pools);
        world->record_pools_valid = 0;
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
