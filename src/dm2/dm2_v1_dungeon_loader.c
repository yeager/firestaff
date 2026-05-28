/*
 * dm2_v1_dungeon_loader.c — DM2 V1 Dungeon Loader
 *
 * DM2 Phase 2: Map, object, tile, and world-state ingestion.
 * Parses DM2 DUNGEON.DAT into DM2_V1_DungeonData.
 *
 * DM2 PC English DUNGEON.DAT format (39,437 bytes, MD5 6caccd7875009e82fe2e28e7f6d6adc0):
 *   - Pre-decompressed (no FTL wrapper — bytes 0-1=0x0000, not 0x8104)
 *   - 'G1' magic at header bytes 2-3 (DM2 file format ID)
 *   - DUNGEON_HEADER at byte 0 (44 bytes, same layout as DM1/DEFS.H:985)
 *   - Map descriptors at byte 44: 28 x 16 bytes each (DM1 MAP descriptor, same format)
 *   - Tile data starts at byte 492 (44 + 28*16)
 *   - Tile type in lower 5 bits of LE uint16 square words, column-major
 *
 *   DUNGEON_HEADER fields (44 bytes, LE):
 *     offset 0:  uint16_t reserved (0x0000)
 *     offset 2:  uint16_t magic ('G1')
 *     offset 4:  uint16_t first_data_offset (=44)
 *     offset 6:  uint8_t  map_count (=28)
 *     offset 8:  uint16_t dungeon_seed
 *     offset 10: uint16_t ornament_random_seed
 *     offset 12: uint16_t raw_map_data_byte_count
 *     offset 14: uint16_t text_data_word_count
 *     offset 16: uint16_t initial_party_location
 *     offset 18: uint16_t square_first_thing_count
 *     offset 20: uint16_t thing_count[16]
 *
 *   MAP DESCRIPTOR (16 bytes each, identical to DM1 DEFS.H:1048-1116):
 *     offset 0:  uint16_t raw_map_data_byte_offset
 *     offset 2:  uint8_t  offset_map_x
 *     offset 3:  uint8_t  offset_map_y
 *     offset 4:  uint16_t bitfield_a: level_id(6) + width-1(5) + height-1(5)
 *     offset 6:  uint16_t bitfield_b: wall_orn(4)+rand_wall(4)+floor(4)+rand_floor(4)
 *     offset 8:  uint16_t bitfield_c: door_orn(4)+creature_type(4)+unref(4)+diff(4)
 *     offset 10: uint16_t bitfield_d: floor_set(4)+wall_set(4)+door_set0(4)+door_set1(4)
 *     offset 12: uint16_t level_width_override  (DM2 extension)
 *     offset 14: uint16_t level_height_override (DM2 extension)
 *
 *   The offset fields in DM2 DMA don't work like DM1 due to the different DMA layout.
 *   Actual width/height come from bytes[12-15] (DM2 extension fields) where present;
 *   fallback to DM1 bitfield_a decoding. The DM2 PC English uses 16-byte descriptors
 *   appended with extra dimensions rather than a separate 8-byte format.
 *
 * FIXES vs stub:
 *   - level_count read from DUNGEON_HEADER.map_count byte offset 6 (stub read byte 0)
 *   - Format is 16-byte DM1 MAP Descriptor with DM2 extensions, not 8-byte
 *   - Tile data offset is relative to tile data region start (tile_data_start = 492)
 *
 * Source: SKULL.ASM T560 DUNGEON_Load, ReDMCSB DEFS.H:985-998, :1048-1116,
 *         DM2 PC English DUNGEON.DAT binary analysis (39,437 bytes),
 *         docs/dm2_v1_phase2_data_formats_H2254.md §2,
 *         docs/dm2_dungeon_files.md
 */

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_world_model.h"
#include "dungeon_decompressor_ftl.h"
#include <stdlib.h>
#include <string.h>

/* ── LE read helpers ──────────────────────────────────────────────── */

static uint16_t rd16le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

#define RD16(p) rd16le(p)

/* DUNGEON_HEADER field offsets */
#define DM2_HDR_MAP_COUNT_OFFSET   6
#define DM2_HDR_SEED_OFFSET       8

/* DUNGEON_HEADER size = 44 (ReDMCSB DEFS.H:985) */
#define DM2_DUNGEON_HEADER_SIZE  44

/* TILE DATA START = DUNGEON_HEADER(44) + MAP_DESCRIPTORS(28*16) = 492 */
#define DM2_TILE_DATA_START       (DM2_DUNGEON_HEADER_SIZE + 28 * 16)

/* ── Public API ───────────────────────────────────────────────────── */

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out,
                         const uint8_t *dat, int size) {
    uint8_t mc;
    int i;

    if (!out || !dat || size < DM2_TILE_DATA_START)
        return -1;
    memset(out, 0, sizeof(*out));

    mc = dat[DM2_HDR_MAP_COUNT_OFFSET];
    if (mc < 1 || mc > DM2_V1_MAX_LEVELS)
        return -1;

    out->level_count = mc;

    /* DM2 PC English: level 0 is OUTDOOR hub, level 1+ are INDOOR/BUILDING */
    for (i = 0; i < mc; i++) {
        const uint8_t *map_desc = dat + DM2_DUNGEON_HEADER_SIZE + i * 16;
        uint16_t w_override = RD16(&map_desc[12]);
        uint16_t h_override = RD16(&map_desc[14]);
        uint16_t bf_a = RD16(&map_desc[4]);

        /* DM2 uses override width/height at bytes[12-13] and [14-15].
         * If both are 0, fall back to DM1 bitfield_a encoding. */
        if (w_override != 0 || h_override != 0) {
            out->level_widths[i]  = (w_override > 0 && w_override <= DM2_V1_MAX_MAP_SIZE * 4)
                                   ? (int)w_override : 64;
            out->level_heights[i] = (h_override > 0 && h_override <= DM2_V1_MAX_MAP_SIZE * 4)
                                   ? (int)h_override : 64;
        } else {
            /* DM1 bitfield_a fallback: width-1 and height-1 in bits */
            int wm1 = ((bf_a >> 5) & 0x1F) + 1;
            int hm1 = (bf_a & 0x1F) + 1;
            out->level_widths[i]  = wm1;
            out->level_heights[i] = hm1;
        }

        /* DM2 level type:
         *   offset_map_x (DMA byte 2) encodes the level type for DM2:
         *     0 = OUTDOOR, 1 = INDOOR/first-floor, 2 = BUILDING.
         *   DM1 used offset_map_x for X offset within larger level;
         *   DM2 repurposes this for per-level type identification. */
        out->level_types[i] = (i == 0) ? DM2_LEVEL_OUTDOOR : DM2_LEVEL_INDOOR;

        /* Tile data byte offset: stored in DMA bytes[0-1] (LE uint16).
         * This is a relative offset into the tile data region.
         * Tile data starts at DM2_TILE_DATA_START = 492.
         * Absolute tile offset = DM2_TILE_DATA_START + raw_map_data_byte_offset */
        out->level_offsets[i] = RD16(&map_desc[0]);
    }

    /* Level 0 is always OUTDOOR in DM2 PC English */
    out->level_types[0] = DM2_LEVEL_OUTDOOR;

    /* Retain raw data reference for square type lookups */
    out->raw_data = (uint8_t *)malloc((size_t)size);
    if (out->raw_data) {
        memcpy(out->raw_data, dat, (size_t)size);
        out->raw_size = size;
    }

    return 0;
}

/*
 * dm2_v1_dungeon_get_square_type — get normalized square type (0-31).
 *
 * DM2 tile data: 2 bytes per square (LE uint16), column-major.
 * Tile region starts at DM2_TILE_DATA_START = 492.
 * Level offset stored as relative byte offset (DM1-style).
 *
 * Source: SKULL.ASM T520 party placement / movement tile access
 */
int dm2_v1_dungeon_get_square_type(const DM2_V1_DungeonData *d,
                                     int level, int x, int y) {
    int offset, w, h;

    if (!d || !d->raw_data) return -1;
    if (level < 0 || level >= d->level_count) return -1;

    w = d->level_widths[level];
    h = d->level_heights[level];
    if (x < 0 || x >= w || y < 0 || y >= h) return -1;

    /* Column-major: (col * height + row) * 2 bytes per tile.
     * Raw offset is relative; add tile data start offset. */
    offset = DM2_TILE_DATA_START + d->level_offsets[level] + ((x * h + y) << 1);
    if (offset < 0 || offset + 1 >= d->raw_size) return -1;

    return RD16(d->raw_data + offset) & 0x1F;
}

int dm2_v1_dungeon_is_outdoor(const DM2_V1_DungeonData *d, int level) {
    if (!d || level < 0 || level >= d->level_count) return 0;
    return d->level_types[level] == DM2_LEVEL_OUTDOOR;
}

void dm2_v1_dungeon_free(DM2_V1_DungeonData *d) {
    if (d && d->raw_data) { free(d->raw_data); d->raw_data = NULL; }
}

const char *dm2_v1_dungeon_source_evidence(void) {
    return
        "DM2 V1 Dungeon Loader — Phase 2 World/Data Model\n"
        "Source: SKULL.ASM T560 DUNGEON_Load — header parsing, level descriptors\n"
        "Source: SKULL.ASM T520 — party placement, movement tile access\n"
        "Source: ReDMCSB DEFS.H:985-998 — DUNGEON_HEADER (44 bytes, LE)\n"
        "Source: ReDMCSB DEFS.H:1048-1116 — DM1 MAP descriptor (16 bytes)\n"
        "Source: DM2 PC English DUNGEON.DAT probe — override width/height at bytes[12-15]\n"
        "Fix: level_count from DUNGEON_HEADER.map_count (byte offset 6), not byte offset 0\n"
        "Fix: 16-byte DM1 MAP descriptor + DM2 width/height override at bytes[12-15\n"
        "Fix: tile offset = DM2_TILE_DATA_START(492) + raw_map_data_byte_offset\n"
        "Fix: column-major tile offset formula (col*height+row)*2\n"
        "Asset: DM2 PC English DUNGEON.DAT 6caccd7875009e82fe2e28e7f6d6adc0\n";
}
