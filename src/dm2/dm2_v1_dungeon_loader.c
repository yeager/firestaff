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
#define DM2_MAP_DESC_SIZE 16
#define DM2_THING_TYPE_COUNT 16
#define DM2_THING_END_MARKER 0xfffeu

static const uint8_t s_dm2_db_record_size[DM2_THING_TYPE_COUNT] = {
    0x04, 0x06, 0x04, 0x08,
    0x10, 0x04, 0x04, 0x04,
    0x04, 0x08, 0x04, 0x00,
    0x00, 0x00, 0x08, 0x04
};

static int dm2_v1_level_tiles_fit(const DM2_V1_DungeonData *d,
                                  int level,
                                  int raw_size) {
    size_t offset;
    size_t tile_bytes;

    if (!d || level < 0 || level >= d->level_count || raw_size < 0)
        return 0;
    if (d->level_widths[level] <= 0 || d->level_heights[level] <= 0)
        return 0;

    offset = (size_t)DM2_TILE_DATA_START + (size_t)d->level_offsets[level];
    tile_bytes = (size_t)d->level_widths[level] *
                 (size_t)d->level_heights[level] * 2U;
    return offset <= (size_t)raw_size &&
           tile_bytes <= (size_t)raw_size - offset;
}

static int dm2_decode_map_dimensions_from_w8(const uint8_t *map_desc,
                                             int *out_w,
                                             int *out_h) {
    uint16_t w8;
    int w;
    int h;

    if (!map_desc || !out_w || !out_h) return 0;
    w8 = RD16(map_desc + 8);
    w = (int)(((w8 >> 6) & 0x1Fu) + 1u);
    h = (int)(((w8 >> 11) & 0x1Fu) + 1u);
    if (w < 1 || w > DM2_V1_MAX_MAP_SIZE ||
        h < 1 || h > DM2_V1_MAX_MAP_SIZE) {
        return 0;
    }
    *out_w = w;
    *out_h = h;
    return 1;
}

static int dm2_v1_try_load_skproject_layout(DM2_V1_DungeonData *out,
                                            const uint8_t *dat,
                                            int size) {
    int map_count;
    int total_columns = 0;
    int raw_map_bytes = 0;
    int column_index_base;
    int sft_base;
    int text_base;
    int thing_base;
    int thing_cursor;
    long thing_total = 0;

    if (!out || !dat || size < DM2_DUNGEON_HEADER_SIZE) return 0;

    map_count = dat[4];
    if (map_count < 1 || map_count > DM2_V1_MAX_LEVELS) return 0;
    if (size < DM2_DUNGEON_HEADER_SIZE + map_count * DM2_MAP_DESC_SIZE)
        return 0;

    memset(out, 0, sizeof(*out));
    out->level_count = map_count;
    out->square_bytes = 1;
    out->raw_map_data_base = -1;
    out->column_index_base = DM2_DUNGEON_HEADER_SIZE +
                             map_count * DM2_MAP_DESC_SIZE;
    out->square_first_thing_count = (int)RD16(dat + 10);
    out->text_word_count = (int)RD16(dat + 6);

    for (int i = 0; i < DM2_THING_TYPE_COUNT; ++i) {
        out->thing_data_bases[i] = -1;
        out->thing_type_counts[i] = (int)RD16(dat + 12 + i * 2);
    }

    for (int i = 0; i < map_count; ++i) {
        const uint8_t *map_desc =
            dat + DM2_DUNGEON_HEADER_SIZE + i * DM2_MAP_DESC_SIZE;
        int w = 0;
        int h = 0;
        int rel_offset = (int)RD16(map_desc + 0);
        int end;
        if (!dm2_decode_map_dimensions_from_w8(map_desc, &w, &h))
            return 0;
        if (rel_offset < 0) return 0;
        end = rel_offset + w * h;
        if (end < rel_offset || end > raw_map_bytes) {
            raw_map_bytes = end;
        }
        out->level_widths[i] = w;
        out->level_heights[i] = h;
        out->level_offsets[i] = rel_offset;
        out->map_offset_x[i] = (int)map_desc[6];
        out->map_offset_y[i] = (int)map_desc[7];
        out->map_door_set0[i] = (int)((RD16(map_desc + 14) >> 8) & 0x0fu);
        out->map_door_set1[i] = (int)((RD16(map_desc + 14) >> 12) & 0x0fu);
        out->level_types[i] = (i == 0) ? DM2_LEVEL_OUTDOOR : DM2_LEVEL_INDOOR;
        total_columns += w;
    }

    column_index_base = out->column_index_base;
    sft_base = column_index_base + total_columns * 2;
    text_base = sft_base + out->square_first_thing_count * 2;
    thing_base = text_base + out->text_word_count * 2;
    thing_cursor = thing_base;
    if (column_index_base < 0 || sft_base < column_index_base ||
        text_base < sft_base || thing_base < text_base ||
        thing_base > size) {
        return 0;
    }

    for (int i = 0; i < DM2_THING_TYPE_COUNT; ++i) {
        int count = out->thing_type_counts[i];
        int bytes = (int)s_dm2_db_record_size[i];
        long pool_bytes;
        if (count < 0 || bytes < 0) return 0;
        out->thing_data_bases[i] = thing_cursor;
        pool_bytes = (long)count * (long)bytes;
        thing_total += pool_bytes;
        if (thing_total < 0 || thing_cursor + pool_bytes > size) return 0;
        thing_cursor += (int)pool_bytes;
    }

    out->raw_map_data_base = thing_cursor;
    if (raw_map_bytes <= 0 || out->raw_map_data_base < 0 ||
        out->raw_map_data_base + raw_map_bytes > size) {
        return 0;
    }
    out->column_index_base = column_index_base;
    out->square_first_thing_base = sft_base;
    out->text_data_base = text_base;

    out->raw_data = (uint8_t *)malloc((size_t)size);
    if (!out->raw_data) return -1;
    memcpy(out->raw_data, dat, (size_t)size);
    out->raw_size = size;
    return 1;
}

/* ── Public API ───────────────────────────────────────────────────── */

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out,
                         const uint8_t *dat, int size) {
    uint8_t mc;
    int i;
    int skproject_layout;

    if (!out || !dat || size < DM2_DUNGEON_HEADER_SIZE)
        return -1;
    memset(out, 0, sizeof(*out));

    skproject_layout = dm2_v1_try_load_skproject_layout(out, dat, size);
    if (skproject_layout != 0)
        return (skproject_layout > 0) ? 0 : -1;

    mc = dat[DM2_HDR_MAP_COUNT_OFFSET];
    if (mc < 1 || mc > DM2_V1_MAX_LEVELS)
        return -1;
    if (size < DM2_TILE_DATA_START)
        return -1;

    out->level_count = mc;
    out->square_bytes = 2;
    out->raw_map_data_base = DM2_TILE_DATA_START;
    out->column_index_base = -1;
    out->square_first_thing_base = -1;
    out->text_data_base = -1;
    for (i = 0; i < DM2_THING_TYPE_COUNT; ++i)
        out->thing_data_bases[i] = -1;

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

    /* ReDMCSB DEFS.H lines 989-998 define DUNGEON_HEADER.MapCount and
     * lines 1049-1116 define each MAP.RawMapDataByteOffset; map 0 must
     * have its complete column-major tile span available before launch. */
    if (!dm2_v1_level_tiles_fit(out, 0, size))
        return -1;

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

    if (d->square_bytes == 1) {
        offset = d->raw_map_data_base + d->level_offsets[level] +
                 (x * h + y);
        if (offset < 0 || offset >= d->raw_size) return -1;
        return (d->raw_data[offset] >> 5) & 0x07;
    }

    /* Column-major: (col * height + row) * 2 bytes per tile.
     * Raw offset is relative; add tile data start offset. */
    offset = DM2_TILE_DATA_START + d->level_offsets[level] + ((x * h + y) << 1);
    if (offset < 0 || offset + 1 >= d->raw_size) return -1;
    return RD16(d->raw_data + offset) & 0x1F;
}

/*
 * dm2_v1_dungeon_get_tile_raw — get full 16-bit raw tile value.
 *
 * Unlike dm2_v1_dungeon_get_square_type() which returns only the tile TYPE
 * (lower 5 bits), this returns the full raw tile word so callers can
 * extract door state (lower 3 bits) for collision checks.
 *
 * Source: SKULL.ASM T520 — movement tile access (full 16-bit read)
 */
int dm2_v1_dungeon_get_tile_raw(const DM2_V1_DungeonData *d,
                                  int level, int x, int y) {
    int offset, w, h;

    if (!d || !d->raw_data) return -1;
    if (level < 0 || level >= d->level_count) return -1;

    w = d->level_widths[level];
    h = d->level_heights[level];
    if (x < 0 || x >= w || y < 0 || y >= h) return -1;

    if (d->square_bytes == 1) {
        offset = d->raw_map_data_base + d->level_offsets[level] +
                 (x * h + y);
        if (offset < 0 || offset >= d->raw_size) return -1;
        return (int)d->raw_data[offset];
    }

    offset = DM2_TILE_DATA_START + d->level_offsets[level] + ((x * h + y) << 1);
    if (offset < 0 || offset + 1 >= d->raw_size) return -1;

    return RD16(d->raw_data + offset);
}

int dm2_v1_dungeon_set_tile_raw(DM2_V1_DungeonData *d,
                                int level,
                                int x,
                                int y,
                                uint16_t raw) {
    int offset, w, h;

    if (!d || !d->raw_data) return -1;
    if (level < 0 || level >= d->level_count) return -1;

    w = d->level_widths[level];
    h = d->level_heights[level];
    if (x < 0 || x >= w || y < 0 || y >= h) return -1;

    if (d->square_bytes == 1) {
        int offset = d->raw_map_data_base + d->level_offsets[level] +
                     (x * h + y);
        if (offset < 0 || offset >= d->raw_size) return -1;
        d->raw_data[offset] = (uint8_t)(raw & 0xffu);
        return 0;
    }

    offset = DM2_TILE_DATA_START + d->level_offsets[level] + ((x * h + y) << 1);
    if (offset < 0 || offset + 1 >= d->raw_size) return -1;

    d->raw_data[offset] = (uint8_t)(raw & 0xffu);
    d->raw_data[offset + 1] = (uint8_t)((raw >> 8) & 0xffu);
    return 0;
}

int dm2_v1_dungeon_get_first_thing(const DM2_V1_DungeonData *d,
                                   int level,
                                   int x,
                                   int y) {
    int raw;
    int column_index = 0;
    int thing_index;
    int column_offset;
    int square_offset;

    if (!d || !d->raw_data || level < 0 || level >= d->level_count)
        return -1;
    if (x < 0 || x >= d->level_widths[level] ||
        y < 0 || y >= d->level_heights[level]) {
        return -1;
    }
    raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (raw < 0) return -1;
    if (d->square_bytes != 1) {
        return ((raw >> 5) & 0x03ff);
    }
    if ((raw & 0x10) == 0) return -1;
    if (d->column_index_base < 0 || d->square_first_thing_base < 0)
        return -1;

    for (int i = 0; i < level; ++i)
        column_index += d->level_widths[i];
    column_offset = d->column_index_base + (column_index + x) * 2;
    if (column_offset < 0 || column_offset + 1 >= d->raw_size)
        return -1;
    thing_index = (int)RD16(d->raw_data + column_offset);
    square_offset = d->raw_map_data_base + d->level_offsets[level] +
                    x * d->level_heights[level];
    for (int row = 0; row < y; ++row) {
        if (square_offset + row >= d->raw_size) return -1;
        if (d->raw_data[square_offset + row] & 0x10u)
            ++thing_index;
    }
    if (thing_index < 0 || thing_index >= d->square_first_thing_count)
        return -1;
    if (d->square_first_thing_base + thing_index * 2 + 1 >= d->raw_size)
        return -1;
    return (int)RD16(d->raw_data + d->square_first_thing_base +
                     thing_index * 2);
}

const uint8_t *dm2_v1_dungeon_get_thing_record(
    const DM2_V1_DungeonData *d,
    uint16_t thing,
    int *out_type,
    int *out_index,
    int *out_size) {
    int type;
    int index;
    int size;
    int offset;

    if (out_type) *out_type = -1;
    if (out_index) *out_index = -1;
    if (out_size) *out_size = 0;
    if (!d || !d->raw_data || thing == DM2_THING_END_MARKER)
        return NULL;
    type = (int)((thing >> 10) & 0x0fu);
    index = (int)(thing & 0x03ffu);
    if (type < 0 || type >= DM2_THING_TYPE_COUNT) return NULL;
    size = (int)s_dm2_db_record_size[type];
    if (size <= 0 || index < 0 || index >= d->thing_type_counts[type])
        return NULL;
    offset = d->thing_data_bases[type] + index * size;
    if (offset < 0 || offset + size > d->raw_size) return NULL;
    if (out_type) *out_type = type;
    if (out_index) *out_index = index;
    if (out_size) *out_size = size;
    return d->raw_data + offset;
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
        "Source: skproject SKWIN/DME.h File_header/Map_definitions/ObjectID/Door\n"
        "Source: skproject SKWIN/SkWinCore.cpp READ_DUNGEON_STRUCTURE + GET_TILE_RECORD_LINK\n"
        "Fix: level_count from DUNGEON_HEADER.map_count (byte offset 6), not byte offset 0\n"
        "Fix: skproject layout reads column object indexes, square-first things, text, DB pools, then byte map data\n"
        "Fix: tile offset = DM2_TILE_DATA_START(492) + raw_map_data_byte_offset\n"
        "Fix: column-major tile offset formula (col*height+row)*2\n"
        "Asset: DM2 PC English DUNGEON.DAT 6caccd7875009e82fe2e28e7f6d6adc0\n";
}
