/*
 * dm2_v1_dungeon_loader.c — DM2 V1 Dungeon Loader
 *
 * DM2 Phase 2: Map, object, tile, and world-state ingestion.
 * Parses DM2 DUNGEON.DAT into DM2_V1_DungeonData.
 *
 * DM2 PC English DUNGEON.DAT format (39,437 bytes, MD5 6caccd7875009e82fe2e28e7f6d6adc0):
 *   - Pre-decompressed (no FTL wrapper — bytes 0-1=0x0000, not 0x8104)
 *   - 'G1' magic at header bytes 2-3 (DM2 file format ID)
 *   - DUNGEON_HEADER-style preamble at byte 0 (44 bytes)
 *   - Map definitions at byte 44: 28 x 16 bytes each
 *   - Byte-sized map squares are stored in the trailing map-data block
 *   - Square type is stored in the high three bits, column-major
 *
 *   PC English G1 preamble fields:
 *     offset 0:  uint16_t reserved (0x0000)
 *     offset 2:  uint16_t magic ('G1')
 *     offset 4:  uint16_t first_data_offset (=44)
 *     offset 6:  uint8_t  map_count (=28)
 *     offset 8:  uint16_t dungeon_seed
 *     offset 10: uint16_t initial_party_location / flags
 *     offset 12: uint16_t ground-stack list size
 *     offset 14: uint16_t thing_count[dbDoor]
 *     offset 16: uint16_t thing_count[dbTeleporter]
 *     ...
 *     offset 42: uint16_t thing_count[dbCloud]
 *
 *   The seed/count profile is effectively the skproject File_header shifted
 *   two bytes after the leading reserved word/G1 marker, while map
 *   definitions still start at byte 44.  Firestaff currently treats the
 *   real-data DB pool bases as unproven because the bytes between the
 *   skproject-sized DB pools and the trailing map-data block contain an
 *   additional PC G1 block that still needs source proof.
 *
 *   PC G1 MAP DEFINITION (16 bytes each; source-compatible with
 *   skproject SKWIN/DME.h Map_definitions for offset + w8 dimensions):
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
 *   The legacy 16-bit square-word path below remains for older Firestaff
 *   synthetic fixtures only. Real PC G1 data uses the byte-square path.
 *
 * FIXES vs stub:
 *   - level_count read from DUNGEON_HEADER.map_count byte offset 6 (stub read byte 0)
 *   - Real PC G1 map definitions use w8 dimensions and byte-sized squares
 *   - Tile data offset is relative to the trailing byte map-data block
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
#define DM2_THING_NULL_MARKER 0xffffu
#define DM2_THING_END_MARKER 0xfffeu

/* PC DOS G1 keeps a 256-byte extension between the 28 Map_definitions and
 * the c_map.cpp-owned tables.  The real EN/FR dungeon member is identical;
 * the offsets below are derived from its 480-column monotonic prefix table.
 * skproject/SKWIN/SkWinCore.cpp READ_DUNGEON_STRUCTURE reads the same table
 * before dunGroundStacks, and c_map.cpp maps tile bit 0x10 through it. */
#define DM2_PC_G1_MAP_EXTENSION_BYTES       256
#define DM2_PC_G1_GROUND_STACK_COUNT_OFFSET 10
#define DM2_PC_G1_DB3_EXTENDED_COUNT         1024
#define DM2_PC_G1_DB4_EXTENDED_COUNT          300
#define DM2_PC_G1_EXTENSION_UNTYPED_TAIL_BYTES 9

static const uint8_t s_dm2_db_record_size[DM2_THING_TYPE_COUNT] = {
    0x04, 0x06, 0x04, 0x08,
    0x10, 0x04, 0x04, 0x04,
    0x04, 0x08, 0x04, 0x00,
    0x00, 0x00, 0x08, 0x04
};

/* The canonical PC G1 continuation has two source-sized c_record runs after
 * the declared pools. c_map.cpp supplies raw ObjectIDs from dunGroundStacks;
 * c_record.cpp defines the 10-bit index and per-DB byte stride. The corpus
 * fixes DB3's continuation at the ObjectID ceiling, then DB4's at 300 rows.
 * Its final nine bytes are not records and remain deliberately untyped. */
static void dm2_v1_configure_pc_g1_extension_records(
    DM2_V1_DungeonData *d) {
    int db3_bytes;
    int db4_bytes;

    if (!d) return;
    for (int type = 0; type < DM2_THING_TYPE_COUNT; ++type) {
        d->g1_extension_record_bases[type] = -1;
        d->g1_extension_record_counts[type] = 0;
    }
    if (d->square_bytes != 1 || d->g1_extension_base < 0 ||
        d->thing_type_counts[3] != 299 || d->thing_type_counts[4] != 173) {
        return;
    }
    db3_bytes = (DM2_PC_G1_DB3_EXTENDED_COUNT -
                 d->thing_type_counts[3]) * s_dm2_db_record_size[3];
    db4_bytes = (DM2_PC_G1_DB4_EXTENDED_COUNT -
                 d->thing_type_counts[4]) * s_dm2_db_record_size[4];
    if (d->g1_extension_size != db3_bytes + db4_bytes +
                                DM2_PC_G1_EXTENSION_UNTYPED_TAIL_BYTES) {
        return;
    }
    d->g1_extension_record_bases[3] = d->g1_extension_base;
    d->g1_extension_record_counts[3] =
        DM2_PC_G1_DB3_EXTENDED_COUNT - d->thing_type_counts[3];
    d->g1_extension_record_bases[4] = d->g1_extension_base + db3_bytes;
    d->g1_extension_record_counts[4] =
        DM2_PC_G1_DB4_EXTENDED_COUNT - d->thing_type_counts[4];
}

static int dm2_v1_g1_extension_record_offset(const DM2_V1_DungeonData *d,
                                              int type,
                                              int index,
                                              int *out_offset) {
    int extension_index;
    int record_size;
    int offset;

    if (!d || !out_offset || type < 0 || type >= DM2_THING_TYPE_COUNT ||
        index < d->thing_type_counts[type] ||
        d->g1_extension_record_bases[type] < 0) {
        return 0;
    }
    extension_index = index - d->thing_type_counts[type];
    record_size = s_dm2_db_record_size[type];
    if (record_size <= 0 || extension_index < 0 ||
        extension_index >= d->g1_extension_record_counts[type]) {
        return 0;
    }
    offset = d->g1_extension_record_bases[type] +
             extension_index * record_size;
    if (offset < d->g1_extension_record_bases[type] ||
        offset + record_size > d->g1_extension_base + d->g1_extension_size) {
        return 0;
    }
    *out_offset = offset;
    return 1;
}

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
    for (int i = 0; i < DM2_THING_TYPE_COUNT; ++i)
        out->g1_extension_record_bases[i] = -1;
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

static int dm2_v1_try_load_pc_g1_byte_layout(DM2_V1_DungeonData *out,
                                             const uint8_t *dat,
                                             int size) {
    int map_count;
    int raw_map_bytes = 0;
    int total_columns = 0;
    int column_index_base;
    int sft_base;
    int text_base;
    int thing_cursor;
    long pool_bytes_total = 0;

    if (!out || !dat || size < DM2_DUNGEON_HEADER_SIZE) return 0;
    if (RD16(dat + 2) != 0x3147u || RD16(dat + 4) != DM2_DUNGEON_HEADER_SIZE)
        return 0;

    map_count = (int)dat[DM2_HDR_MAP_COUNT_OFFSET];
    if (map_count < 1 || map_count > DM2_V1_MAX_LEVELS) return 0;
    if (size < DM2_DUNGEON_HEADER_SIZE + map_count * DM2_MAP_DESC_SIZE)
        return 0;

    memset(out, 0, sizeof(*out));
    for (int i = 0; i < DM2_THING_TYPE_COUNT; ++i)
        out->g1_extension_record_bases[i] = -1;
    out->level_count = map_count;
    out->square_bytes = 1;
    out->raw_map_data_base = -1;
    out->column_index_base = -1;
    out->square_first_thing_base = -1;
    out->text_data_base = -1;
    out->g1_extension_base = -1;
    out->g1_extension_size = 0;
    out->square_first_thing_count =
        (int)RD16(dat + DM2_PC_G1_GROUND_STACK_COUNT_OFFSET);
    out->text_word_count = (int)RD16(dat + 8);
    for (int i = 0; i < DM2_THING_TYPE_COUNT; ++i) {
        out->thing_data_bases[i] = -1;
        out->thing_type_counts[i] = (int)RD16(dat + 14 + i * 2);
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
        end = rel_offset + w * h;
        if (end < rel_offset) return 0;
        if (end > raw_map_bytes) raw_map_bytes = end;

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

    /* The G1 file has an observed 256-byte extension directly after its map
     * definitions.  Its following table is the c_map.cpp column-prefix table:
     * 480 LE words, monotonically increasing from 0 to 1187 in the real DOS
     * EN/FR member.  The next G1 header word bounds dunGroundStacks. */
    column_index_base = DM2_DUNGEON_HEADER_SIZE +
                        map_count * DM2_MAP_DESC_SIZE +
                        DM2_PC_G1_MAP_EXTENSION_BYTES;
    sft_base = column_index_base + total_columns * 2;
    text_base = sft_base + out->square_first_thing_count * 2;
    thing_cursor = text_base + out->text_word_count * 2;
    if (total_columns <= 0 || column_index_base < 0 ||
        sft_base < column_index_base || text_base < sft_base ||
        thing_cursor < text_base || thing_cursor > size) {
        return 0;
    }

    /* skproject SKWIN/SkWinCore.cpp READ_DUNGEON_STRUCTURE:40037-40056
     * reads each c_record category immediately after dunTextData, in DB type
     * order, using glbItemSizePerDB[type] * File_header::nRecords[type].
     * GenericRecord::w0 is the next ObjectID (DME.h:831-847), so assigning
     * these bases is sufficient for the bounded c_record traversal below.
     * The later G1 extension remains outside this ownership contract. */
    for (int type = 0; type < DM2_THING_TYPE_COUNT; ++type) {
        int count = out->thing_type_counts[type];
        int record_size = (int)s_dm2_db_record_size[type];
        long pool_bytes;

        if (count < 0 || record_size < 0) return 0;
        pool_bytes = (long)count * (long)record_size;
        if (pool_bytes < 0 || pool_bytes_total + pool_bytes > INT32_MAX ||
            thing_cursor + pool_bytes > size) {
            return 0;
        }
        out->thing_data_bases[type] =
            (count > 0 && record_size > 0) ? thing_cursor : -1;
        thing_cursor += (int)pool_bytes;
        pool_bytes_total += pool_bytes;
    }
    if (raw_map_bytes <= 0 || thing_cursor < 0 ||
        thing_cursor + raw_map_bytes > size) {
        return 0;
    }

    /* The trailing map tail is the only G1 region whose address is proven
     * by every Map_definitions::w0 + dimensions span. The intervening bytes
     * are deliberately not assigned to the standard ownership graph. */
    out->g1_extension_base = thing_cursor;
    out->g1_extension_size = (size - raw_map_bytes) - thing_cursor;
    if (out->g1_extension_size <= 0) return 0;
    out->raw_map_data_base = size - raw_map_bytes;
    out->column_index_base = column_index_base;
    out->square_first_thing_base = sft_base;
    out->text_data_base = text_base;
    dm2_v1_configure_pc_g1_extension_records(out);
    out->raw_data = (uint8_t *)malloc((size_t)size);
    if (!out->raw_data) return -1;
    memcpy(out->raw_data, dat, (size_t)size);
    out->raw_size = size;
    /* First classify every map-owned root without following GenericRecord::w0.
     * The canonical G1 corpus has five DB8/DB10 roots outside the direct and
     * DB3/DB4 address transforms, so a complete graph must not be attempted
     * for it.  ReDMCSB DUNGEON.C F0159 only follows Next after GetThingData;
     * partial boot deliberately stops before that operation. */
    {
        DM2_V1_G1RecordPoolEvidence evidence;
        if (dm2_v1_dungeon_collect_g1_record_pool_evidence(out, &evidence) &&
            evidence.map_root_shape_invalid == 0) {
            out->record_graph_complete = 1;
            if (!dm2_v1_dungeon_validate_record_graph(out))
                out->record_graph_complete = 0;
        }
    }
    (void)dm2_v1_dungeon_materialize_g1_partial_map_boot(
        out, &out->partial_map_boot);
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

    skproject_layout = dm2_v1_try_load_pc_g1_byte_layout(out, dat, size);
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
        if (raw <= 7u) {
            uint8_t current = d->raw_data[offset];
            uint8_t prefix = (uint8_t)(current & 0xf8u);
            if (((current >> 5) & 0x07u) != 4u)
                prefix = (uint8_t)(4u << 5);
            d->raw_data[offset] = (uint8_t)(prefix | (raw & 0x07u));
        } else {
            d->raw_data[offset] = (uint8_t)(raw & 0xffu);
        }
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
    if (size <= 0 || index < 0)
        return NULL;
    if (index < d->thing_type_counts[type]) {
        offset = d->thing_data_bases[type] + index * size;
    } else if (!dm2_v1_g1_extension_record_offset(d, type, index, &offset)) {
        return NULL;
    }
    if (offset < 0 || offset + size > d->raw_size) return NULL;
    if (out_type) *out_type = type;
    if (out_index) *out_index = index;
    if (out_size) *out_size = size;
    return d->raw_data + offset;
}

int dm2_v1_dungeon_get_next_thing(const DM2_V1_DungeonData *d,
                                  uint16_t thing) {
    const uint8_t *record;
    int size = 0;

    /* skproject SKWINSPX/src/v4/skcore.cpp GET_NEXT_RECORD_LINK
     * returns GET_ADDRESS_OF_RECORD(rl)->w0; GenericRecord::w0 is the
     * first little-endian word in every bounded DB pool record. */
    /* The PC G1 corpus proves the pool addresses but not every ObjectID
     * shape in the ground-stack graph. Do not promote GenericRecord::w0
     * traversal for that variant until its complete graph validates. */
    if (d && d->square_bytes == 1 && d->g1_extension_size > 0 &&
        !d->record_graph_complete) {
        return -1;
    }
    record = dm2_v1_dungeon_get_thing_record(d, thing, NULL, NULL, &size);
    if (!record || size < 2) return -1;
    return (int)RD16(record);
}

static int dm2_v1_g1_link_has_declared_shape(const DM2_V1_DungeonData *d,
                                              uint16_t link) {
    int type;
    int index;

    if (!d || link == DM2_THING_END_MARKER) return 0;
    type = (int)((link >> 10) & 0x0fu);
    index = (int)(link & 0x03ffu);
    return s_dm2_db_record_size[type] > 0 &&
           index >= 0 && index < d->thing_type_counts[type];
}

static int dm2_v1_g1_link_has_extension_shape(const DM2_V1_DungeonData *d,
                                               uint16_t link) {
    int type;
    int index;
    int offset;

    if (!d || link == DM2_THING_END_MARKER) return 0;
    type = (int)((link >> 10) & 0x0fu);
    index = (int)(link & 0x03ffu);
    return dm2_v1_g1_extension_record_offset(d, type, index, &offset);
}

static int dm2_v1_g1_link_has_record_shape(const DM2_V1_DungeonData *d,
                                            uint16_t link) {
    return dm2_v1_g1_link_has_declared_shape(d, link) ||
           dm2_v1_g1_link_has_extension_shape(d, link);
}

static int dm2_v1_g1_read_teleporter_root(
    const DM2_V1_DungeonData *d,
    uint16_t object_id,
    int x,
    int y,
    DM2_V1_G1TeleporterRoot *out) {
    const uint8_t *record;
    int offset;
    int index;

    /* skproject/SKULLWIN/c_record.cpp DM2_GET_ADDRESS_OF_RECORD lines 45-52
     * resolves a DB1 ObjectID by its 10-bit index and six-byte stride.
     * skproject/SKWIN/DME.h Teleporter lines 367-382 fixes w2/w4, including
     * destination coordinates/map, scope, sound, and rotation.
     * This deliberately does not call GET_NEXT_RECORD_LINK or read w0. */
    if (!d || !out || ((object_id >> 10) & 0x0fu) != 1u) return 0;
    index = object_id & 0x03ffu;
    if (index >= d->thing_type_counts[1]) {
        return 0;
    }
    offset = d->thing_data_bases[1] + index * s_dm2_db_record_size[1];
    if (offset < 0 || offset + s_dm2_db_record_size[1] > d->raw_size) return 0;
    record = d->raw_data + offset;
    out->x = x;
    out->y = y;
    out->object_id = object_id;
    out->index = index;
    out->destination_x = (uint8_t)(RD16(record + 2) & 0x001fu);
    out->destination_y = (uint8_t)((RD16(record + 2) >> 5) & 0x001fu);
    out->destination_map = (uint8_t)(RD16(record + 4) >> 8);
    out->scope = (uint8_t)((RD16(record + 2) >> 13) & 0x0003u);
    out->sound = (uint8_t)((RD16(record + 2) >> 15) & 0x0001u);
    out->rotation = (uint8_t)((RD16(record + 2) >> 10) & 0x0003u);
    out->rotation_type = (uint8_t)((RD16(record + 2) >> 12) & 0x0001u);
    return 1;
}

int dm2_v1_dungeon_validate_record_graph(const DM2_V1_DungeonData *d) {
    int total_records = 0;
    int level;

    /* skproject/SKULLWIN/c_map.cpp DM2_GET_OBJECT_INDEX_FROM_TILE and
     * DM2_GET_TILE_RECORD_LINK derive every square's first ObjectID from
     * the column index and dunGroundStacks tables.  c_record.cpp then
     * resolves the ObjectID through the DB pool selected by bits 10..13. */
    if (!d || !d->record_graph_complete || !d->raw_data ||
        d->square_bytes != 1 || d->column_index_base < 0 ||
        d->square_first_thing_base < 0 || d->square_first_thing_count < 0) {
        return 0;
    }
    for (int type = 0; type < DM2_THING_TYPE_COUNT; ++type) {
        if (d->thing_type_counts[type] < 0 ||
            (s_dm2_db_record_size[type] > 0 &&
             d->thing_type_counts[type] > 0 &&
             d->thing_data_bases[type] < 0)) {
            return 0;
        }
        total_records += d->thing_type_counts[type] +
                         d->g1_extension_record_counts[type];
    }
    if (total_records <= 0) return 0;

    for (level = 0; level < d->level_count; ++level) {
        int x;
        for (x = 0; x < d->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < d->level_heights[level]; ++y) {
                int raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
                int thing;
                int steps = 0;
                uint8_t seen[DM2_THING_TYPE_COUNT][1024] = {{0}};
                if (raw < 0) return 0;
                if ((raw & 0x10) == 0) continue;
                thing = dm2_v1_dungeon_get_first_thing(d, level, x, y);
                while (thing != (int)DM2_THING_END_MARKER) {
                    int next;
                    int type;
                    int index;

                    /* skproject SKWINSPX/src/v4/skcore.cpp:1184-1200 reads
                     * GenericRecord::w0 only after GET_ADDRESS_OF_RECORD()
                     * has accepted the ObjectID.  Therefore only records
                     * reached from a map-owned root participate here; unused
                     * pool slots are allocation state, not inferred links.
                     * SKWIN/DME.h:831-847 names w0 the next ObjectID. */
                    if (thing < 0 || !dm2_v1_g1_link_has_record_shape(
                                         d, (uint16_t)thing) ||
                        ++steps > total_records) {
                        return 0;
                    }
                    type = ((uint16_t)thing >> 10) & 0x0f;
                    index = (uint16_t)thing & 0x03ff;
                    if (seen[type][index]) return 0;
                    seen[type][index] = 1;
                    if (!dm2_v1_dungeon_get_thing_record(
                            d, (uint16_t)thing, NULL, NULL, NULL)) {
                        return 0;
                    }
                    next = dm2_v1_dungeon_get_next_thing(d, (uint16_t)thing);
                    if (next < 0 || (next != (int)DM2_THING_END_MARKER &&
                                     !dm2_v1_g1_link_has_record_shape(
                                         d, (uint16_t)next))) {
                        return 0;
                    }
                    thing = next;
                }
            }
        }
    }
    return 1;
}

int dm2_v1_dungeon_validate_record_pools(const DM2_V1_DungeonData *d) {
    int cursor;

    /* ReDMCSB/skproject c_record.cpp DM2_GET_ADDRESS_OF_RECORD lines 46-52
     * resolves recordptr[type] + glbItemSizePerDB[type] * (ObjectID & 0x3ff).
     * skproject SKWIN/SkWinCore.cpp READ_DUNGEON_STRUCTURE lines 40037-40056
     * loads those recordptr bases consecutively after dunTextData. PC DOS G1
     * proves this bounded pool transform, but not GenericRecord::w0 graph
     * semantics, so boot may validate addresses without enabling traversal. */
    if (!d || !d->raw_data || d->text_data_base < 0 ||
        d->text_word_count < 0 || d->raw_size < 0) {
        return 0;
    }
    cursor = d->text_data_base + d->text_word_count * 2;
    if (cursor < d->text_data_base || cursor > d->raw_size) return 0;

    for (int type = 0; type < DM2_THING_TYPE_COUNT; ++type) {
        int count = d->thing_type_counts[type];
        int record_size = (int)s_dm2_db_record_size[type];
        long pool_bytes;
        int expected_base;

        if (count < 0 || record_size < 0) return 0;
        pool_bytes = (long)count * (long)record_size;
        if (pool_bytes < 0 || pool_bytes > INT32_MAX ||
            cursor + pool_bytes > d->raw_size) {
            return 0;
        }
        expected_base = (count > 0 && record_size > 0) ? cursor : -1;
        if (d->thing_data_bases[type] != expected_base) return 0;
        cursor += (int)pool_bytes;
    }

    /* G1 has a proven untyped extension between c_record pools and map data.
     * Other accepted layouts place map bytes immediately after the pools. */
    if (d->square_bytes == 1 && d->g1_extension_base >= 0) {
        return cursor == d->g1_extension_base &&
               d->g1_extension_size >= 0 &&
               d->g1_extension_base + d->g1_extension_size ==
                   d->raw_map_data_base;
    }
    return cursor == d->raw_map_data_base;
}

int dm2_v1_dungeon_collect_g1_record_pool_evidence(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1RecordPoolEvidence *out) {
    int cursor;
    int type;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    for (type = 0; type < DM2_THING_TYPE_COUNT; ++type)
        out->candidate_pool_bases[type] = -1;
    out->tail_pool_base = -1;

    /* skproject READ_DUNGEON_STRUCTURE orders c_map's column, ground-stack,
     * and text tables before c_record ownership. This receipt records only
     * that bounded, non-tail sequence; it does not assert that the bytes are
     * the PC G1 DB pools or allow GET_ADDRESS_OF_RECORD-style access. */
    if (!d || !d->raw_data || d->square_bytes != 1 ||
        d->column_index_base < 0 || d->square_first_thing_base < 0 ||
        d->text_data_base < 0 || d->text_word_count < 0 ||
        d->g1_extension_base < 0 || d->raw_map_data_base < 0) {
        return 0;
    }

    cursor = d->text_data_base + d->text_word_count * 2;
    if (cursor < d->text_data_base || cursor > d->raw_size) return 0;
    out->text_end = cursor;
    out->candidate_base = cursor;

    for (type = 0; type < DM2_THING_TYPE_COUNT; ++type) {
        int count = d->thing_type_counts[type];
        int record_size = (int)s_dm2_db_record_size[type];
        long bytes;

        if (count < 0 || record_size < 0) return 0;
        bytes = (long)count * record_size;
        if (bytes < 0 || bytes > INT32_MAX || cursor + bytes > d->raw_size)
            return 0;
        if (count > 0 && record_size > 0)
            out->candidate_pool_bases[type] = cursor;
        cursor += (int)bytes;
    }
    out->candidate_end = cursor;
    out->candidate_bytes = cursor - out->candidate_base;

    /* A tail-aligned span would start at raw_map_data_base - candidate_bytes.
     * It is rejected when it differs from the source-ordered text boundary. */
    out->tail_pool_base = d->raw_map_data_base - out->candidate_bytes;
    out->tail_pool_base_rejected =
        out->tail_pool_base >= 0 && out->tail_pool_base != out->candidate_base;
    if (out->candidate_end != d->g1_extension_base) return 0;

    for (int i = 0; i < d->square_first_thing_count; ++i) {
        int offset = d->square_first_thing_base + i * 2;
        uint16_t link;
        if (offset < 0 || offset + 1 >= d->raw_size) return 0;
        link = RD16(d->raw_data + offset);
        ++out->root_count;
        if (link == DM2_THING_END_MARKER)
            ++out->root_end_markers;
        else if (dm2_v1_g1_link_has_declared_shape(d, link))
            ++out->root_shape_valid;
        else
            ++out->root_shape_invalid;
    }

    /* skproject/SKULLWIN/c_map.cpp DM2_GET_OBJECT_INDEX_FROM_TILE only
     * indexes dunGroundStacks for squares whose raw map byte has bit 0x10.
     * Do not infer map roots from unused capacity entries. c_record.h:10-11
     * separately defines OBJECT_NULL (-1) and OBJECT_END_MARKER (-2). */
    for (int level = 0; level < d->level_count; ++level) {
        int column_index = 0;
        for (int previous = 0; previous < level; ++previous)
            column_index += d->level_widths[previous];
        for (int x = 0; x < d->level_widths[level]; ++x) {
            int thing_index;
            int column_offset = d->column_index_base + (column_index + x) * 2;
            int square_offset = d->raw_map_data_base + d->level_offsets[level] +
                                x * d->level_heights[level];

            if (column_offset < 0 || column_offset + 1 >= d->raw_size ||
                square_offset < 0 ||
                square_offset + d->level_heights[level] > d->raw_size) {
                return 0;
            }
            thing_index = (int)RD16(d->raw_data + column_offset);
            for (int y = 0; y < d->level_heights[level]; ++y) {
                uint16_t link;
                if ((d->raw_data[square_offset + y] & 0x10u) == 0)
                    continue;
                if (thing_index < 0 ||
                    thing_index >= d->square_first_thing_count) {
                    return 0;
                }
                link = RD16(d->raw_data + d->square_first_thing_base +
                            thing_index * 2);
                ++out->map_root_count;
                ++out->map_root_count_by_map[level];
                if (link == DM2_THING_END_MARKER)
                    ++out->map_root_end_markers;
                else if (link == DM2_THING_NULL_MARKER)
                    ++out->map_root_null_markers;
                else if (dm2_v1_g1_link_has_declared_shape(d, link)) {
                    ++out->map_root_shape_valid;
                    ++out->map_root_direct_by_type[(link >> 10) & 0x0f];
                } else if (dm2_v1_g1_link_has_extension_shape(d, link)) {
                    ++out->map_root_extension_shape_valid;
                    ++out->map_root_extension_by_type[(link >> 10) & 0x0f];
                    ++out->map_root_extension_by_map[level];
                } else {
                    ++out->map_root_shape_invalid;
                    ++out->map_root_unresolved_after_extension;
                    ++out->map_root_unresolved_by_type[(link >> 10) & 0x0f];
                    ++out->map_root_unresolved_by_map[level];
                }
                ++thing_index;
            }
        }
    }

    for (type = 0; type < DM2_THING_TYPE_COUNT; ++type) {
        int base = out->candidate_pool_bases[type];
        int count = d->thing_type_counts[type];
        int record_size = (int)s_dm2_db_record_size[type];
        if (base < 0 || record_size < 2) continue;
        for (int index = 0; index < count; ++index) {
            uint16_t link = RD16(d->raw_data + base + index * record_size);
            ++out->candidate_record_count;
            if (link == DM2_THING_END_MARKER)
                ++out->candidate_first_link_end_markers;
            else if (dm2_v1_g1_link_has_declared_shape(d, link))
                ++out->candidate_first_link_shape_valid;
            else
                ++out->candidate_first_link_shape_invalid;
        }
    }

    out->available = 1;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_partial_map_boot(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1PartialMapBootReceipt *out) {
    DM2_V1_G1PartialMapBootReceipt candidate;
    int column_index = 0;

    if (!out || !d || !d->raw_data || d->square_bytes != 1 ||
        d->record_graph_complete || d->column_index_base < 0 ||
        d->square_first_thing_base < 0 || d->raw_map_data_base < 0) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.level_count = d->level_count;

    /* c_map.cpp selects dunGroundStacks only for bit 0x10 squares.  Read the
     * root ObjectID once, classify its source-proven address transform, and
     * never read its GenericRecord::w0. */
    for (int level = 0; level < d->level_count; ++level) {
        for (int x = 0; x < d->level_widths[level]; ++x) {
            int stack = (int)RD16(d->raw_data + d->column_index_base +
                                  (column_index + x) * 2);
            for (int y = 0; y < d->level_heights[level]; ++y) {
                int raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
                uint16_t root;
                int type;
                int index;

                if (raw < 0) return 0;
                if ((raw & 0x10) == 0) continue;
                if (stack < 0 || stack >= d->square_first_thing_count)
                    return 0;
                root = RD16(d->raw_data + d->square_first_thing_base +
                            stack * 2);
                ++candidate.map_root_count;
                type = (root >> 10) & 0x0f;
                index = root & 0x03ff;
                if (dm2_v1_g1_link_has_declared_shape(d, root)) {
                    ++candidate.direct_root_count;
                    ++candidate.materialized_root_count;
                } else if (dm2_v1_g1_link_has_extension_shape(d, root)) {
                    if (type == 3) ++candidate.db3_root_count;
                    else if (type == 4) ++candidate.db4_root_count;
                    else return 0;
                    ++candidate.materialized_root_count;
                } else {
                    DM2_V1_G1BlockedRoot *blocked;
                    if ((type != 8 && type != 10) ||
                        candidate.blocked_root_count >=
                            DM2_V1_G1_PARTIAL_BOOT_MAX_BLOCKED_ROOTS) {
                        return 0;
                    }
                    blocked = &candidate.blocked_roots[
                        candidate.blocked_root_count++];
                    blocked->map = level;
                    blocked->x = x;
                    blocked->y = y;
                    blocked->object_id = root;
                    blocked->type = type;
                    blocked->index = index;
                    ++candidate.blocked_root_count_by_map[level];
                }
                ++stack;
            }
        }
        column_index += d->level_widths[level];
    }

    if (candidate.map_root_count != 883 ||
        candidate.direct_root_count != 676 || candidate.db3_root_count != 174 ||
        candidate.db4_root_count != 28 || candidate.materialized_root_count != 878 ||
        candidate.blocked_root_count != DM2_V1_G1_PARTIAL_BOOT_MAX_BLOCKED_ROOTS) {
        return 0;
    }
    candidate.incomplete = 1;
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_first_map_runtime(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1FirstMapRuntimeReceipt *out) {
    DM2_V1_G1PartialMapBootReceipt partial;
    DM2_V1_G1FirstMapRuntimeReceipt candidate;
    int stack;

    if (!out || !d || d->level_count < 1 || !d->raw_data ||
        !dm2_v1_dungeon_materialize_g1_partial_map_boot(d, &partial) ||
        !partial.committed || !partial.incomplete) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.committed = 1;
    candidate.incomplete_world = 1;
    candidate.map = 0;
    candidate.width = d->level_widths[0];
    candidate.height = d->level_heights[0];
    if (candidate.width <= 0 || candidate.height <= 0) return 0;

    /* ReDMCSB c_map.cpp GET_TILE_RECORD_LINK: map squares select a
     * dunGroundStacks word through the column index. Stop at that ObjectID;
     * DUNGEON.C F0159 is the later boundary that reads GenericRecord::w0. */
    stack = (int)RD16(d->raw_data + d->column_index_base);
    for (int x = 0; x < candidate.width; ++x) {
        for (int y = 0; y < candidate.height; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(d, 0, x, y);
            uint16_t root;
            int type;
            DM2_V1_G1VerifiedRoot *verified;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            type = (root >> 10) & 0x0f;
            if (candidate.root_count >= DM2_V1_G1_FIRST_MAP_MAX_ROOTS) {
                return 0;
            }
            verified = &candidate.roots[candidate.root_count];
            verified->x = x;
            verified->y = y;
            verified->object_id = root;
            verified->type = type;
            verified->index = root & 0x03ff;
            ++candidate.root_count;
            if (dm2_v1_g1_link_has_declared_shape(d, root)) {
                ++candidate.direct_root_count;
                ++candidate.verified_root_count;
            } else if (dm2_v1_g1_link_has_extension_shape(d, root)) {
                if (type == 3) ++candidate.db3_root_count;
                else if (type == 4) ++candidate.db4_root_count;
                else return 0;
                ++candidate.verified_root_count;
            } else {
                /* Map 0 cannot consume an unresolved root. The five known
                 * DB8/DB10 roots stay represented by the world receipt. */
                ++candidate.blocked_root_count;
                return 0;
            }
            if (type == 1) {
                if (candidate.teleporter_root_count >=
                    DM2_V1_G1_FIRST_MAP_MAX_ROOTS ||
                    !dm2_v1_g1_read_teleporter_root(
                        d, root, x, y,
                        &candidate.teleporters[candidate.teleporter_root_count])) {
                    return 0;
                }
                ++candidate.teleporter_root_count;
                ++candidate.teleporter_record_reads;
            }
            ++stack;
        }
        if (x + 1 < candidate.width) {
            stack = (int)RD16(d->raw_data + d->column_index_base +
                              (x + 1) * 2);
        }
    }
    if (candidate.root_count <= 0 ||
        candidate.verified_root_count != candidate.root_count ||
        candidate.object_count != 0 || candidate.blocked_record_reads != 0) {
        return 0;
    }
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_map5_text_runtime(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1Map5TextRuntimeReceipt *out) {
    DM2_V1_G1PartialMapBootReceipt partial;
    DM2_V1_G1Map5TextRuntimeReceipt candidate;
    const int map = 5;
    int column_index = 0;

    if (!out || !d || d->level_count <= map || !d->raw_data ||
        !dm2_v1_dungeon_materialize_g1_partial_map_boot(d, &partial) ||
        !partial.committed || !partial.incomplete) {
        return 0;
    }
    for (int previous = 0; previous < map; ++previous)
        column_index += d->level_widths[previous];

    memset(&candidate, 0, sizeof(candidate));
    candidate.committed = 1;
    candidate.incomplete_world = 1;
    candidate.map = map;

    /* skproject/SKWIN/DME.h Text: w2 is visibility (bit 0), mode (1..2),
     * and text-table index (3..15). c_map.cpp supplies the root ObjectID.
     * Do not call GET_NEXT_RECORD_LINK or inspect the text-table payload. */
    for (int x = 0; x < d->level_widths[map]; ++x) {
        int stack = (int)RD16(d->raw_data + d->column_index_base +
                              (column_index + x) * 2);
        for (int y = 0; y < d->level_heights[map]; ++y) {
            const uint8_t *record;
            uint16_t root;
            uint16_t w2;
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            int type;
            int index;
            DM2_V1_G1TextRoot *text;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            ++stack;
            type = (root >> 10) & 0x0f;
            if (type != 2) continue;
            if (!dm2_v1_g1_link_has_declared_shape(d, root) ||
                candidate.text_root_count >= DM2_V1_G1_MAP5_MAX_TEXT_ROOTS) {
                return 0;
            }
            index = root & 0x03ff;
            record = d->raw_data + d->thing_data_bases[2] +
                     index * s_dm2_db_record_size[2];
            if (record + s_dm2_db_record_size[2] > d->raw_data + d->raw_size)
                return 0;
            w2 = RD16(record + 2);
            text = &candidate.texts[candidate.text_root_count++];
            text->x = x;
            text->y = y;
            text->object_id = root;
            text->index = index;
            text->direction = (uint8_t)(root >> 14);
            text->visible = (uint8_t)(w2 & 0x0001u);
            text->mode = (uint8_t)((w2 >> 1) & 0x0003u);
            text->text_index = (uint16_t)((w2 >> 3) & 0x1fffu);
            ++candidate.text_record_reads;
        }
    }
    if (candidate.text_root_count != 7 || candidate.text_record_reads != 7 ||
        candidate.generic_record_reads != 0 ||
        candidate.blocked_record_reads != 0) {
        return 0;
    }
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_find_thing_of_type(const DM2_V1_DungeonData *d,
                                      uint16_t first_thing,
                                      int desired_type,
                                      int max_steps) {
    uint16_t thing = first_thing;

    if (!d || desired_type < 0 || desired_type >= DM2_THING_TYPE_COUNT)
        return -1;
    if (max_steps <= 0) max_steps = 32;
    for (int step = 0; step < max_steps; ++step) {
        int type = -1;
        int next;
        if (thing == DM2_THING_END_MARKER) return -1;
        if (!dm2_v1_dungeon_get_thing_record(d, thing, &type, NULL, NULL))
            return -1;
        if (type == desired_type) return (int)thing;
        next = dm2_v1_dungeon_get_next_thing(d, thing);
        if (next < 0 || next == (int)thing) return -1;
        thing = (uint16_t)next;
    }
    return -1;
}

int dm2_v1_dungeon_find_text_wall_gfx(const DM2_V1_DungeonData *d,
                                      uint16_t first_thing,
                                      int view_dir,
                                      int side_index,
                                      int max_steps,
                                      int *out_wall_gfx_index,
                                      int *out_wall_gfx_field) {
    uint16_t thing = first_thing;

    if (out_wall_gfx_index) *out_wall_gfx_index = -1;
    if (out_wall_gfx_field) *out_wall_gfx_field = -1;
    if (!d || !out_wall_gfx_index || !out_wall_gfx_field) return -1;
    if (side_index < 0 || side_index > 3) return -1;
    if (max_steps <= 0) max_steps = 32;

    for (int step = 0; step < max_steps; ++step) {
        int type = -1;
        int size = 0;
        int next;
        const uint8_t *record;
        uint16_t w2;
        int text_mode;
        int text_index;
        int ext_usage;
        int text_visible;
        int object_dir;
        int relative_side;
        int ornate;
        int frame = 0;
        int packed;
        int accepts_static_wall_gfx = 0;

        if (thing == DM2_THING_END_MARKER) return -1;
        record = dm2_v1_dungeon_get_thing_record(d, thing, &type, NULL, &size);
        if (!record || size < 2) return -1;
        if (type > 3) return -1;

        if (type == 2 && size >= 4) {
            w2 = RD16(record + 2);
            text_mode = (int)((w2 >> 1) & 3u);
            text_index = (int)((w2 >> 3) & 0x1fffu);
            ext_usage = (text_index >> 8) & 0x1f;
            text_visible = (int)(w2 & 1u);
            object_dir = (int)((thing >> 14) & 3u);
            relative_side = ((object_dir - (view_dir & 3)) & 3);
            ornate = text_index & 0xff;

            /* skproject SKWINSPX/src/v4/skdungn.cpp _0cee_1a46 handles
             * DB2 TextMode()==1 as WALL_GFX ornament metadata.  This bounded
             * startup helper covers the static cases that do not need the
             * GDAT animation query: ext usages 0/6 are accepted immediately,
             * 2 is always present, and 5/13 depend on TextVisibility(). */
            if (text_mode == 1 && relative_side == side_index) {
                switch (ext_usage) {
                    case 0:
                    case 6:
                        accepts_static_wall_gfx = 1;
                        break;
                    case 2:
                        accepts_static_wall_gfx = 1;
                        frame = 0;
                        break;
                    case 5:
                    case 13:
                        accepts_static_wall_gfx = text_visible != 0;
                        frame = 0;
                        break;
                    default:
                        accepts_static_wall_gfx = 0;
                        break;
                }
                if (accepts_static_wall_gfx) {
                    packed = (frame << 10) | ornate;
                    *out_wall_gfx_index = packed & 0xff;
                    *out_wall_gfx_field = ((packed >> 8) & 0xff) + 1;
                    return 0;
                }
            }
        }

        next = dm2_v1_dungeon_get_next_thing(d, thing);
        if (next < 0 || next == (int)thing) return -1;
        thing = (uint16_t)next;
    }
    return -1;
}

int dm2_v1_dungeon_find_actuator_wall_gfx_ordinal(
    const DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int view_dir,
    int side_index,
    int max_steps,
    int *out_wall_gfx_ordinal) {
    uint16_t thing = first_thing;

    if (out_wall_gfx_ordinal) *out_wall_gfx_ordinal = -1;
    if (!d || !out_wall_gfx_ordinal) return -1;
    if (side_index < 0 || side_index > 3) return -1;
    if (max_steps <= 0) max_steps = 32;

    for (int step = 0; step < max_steps; ++step) {
        int type = -1;
        int size = 0;
        int next;
        const uint8_t *record;
        int object_dir;
        int relative_side;

        if (thing == DM2_THING_END_MARKER) return -1;
        record = dm2_v1_dungeon_get_thing_record(d, thing, &type, NULL, &size);
        if (!record || size < 2) return -1;
        if (type > 3) return -1;

        if (type == 3 && size >= 8) {
            int ordinal = (int)((RD16(record + 4) >> 12) & 0x0fu);
            object_dir = (int)((thing >> 14) & 3u);
            relative_side = ((object_dir - (view_dir & 3)) & 3);
            /* skproject SKWINSPX/src/v4/skdungn.cpp
             * GET_WALL_DECORATION_OF_ACTUATOR uses Actuator::GraphicNumber()
             * as a one-based ordinal into the current map's wall-gfx list.
             * Firestaff exposes that ordinal here; resolving it to the real
             * GDAT WALL_GFX index belongs to the map graphics-list handoff. */
            if (relative_side == side_index && ordinal > 0) {
                *out_wall_gfx_ordinal = ordinal;
                return 0;
            }
        }

        next = dm2_v1_dungeon_get_next_thing(d, thing);
        if (next < 0 || next == (int)thing) return -1;
        thing = (uint16_t)next;
    }
    return -1;
}

int dm2_v1_dungeon_resolve_actuator_wall_gfx(
    const DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int view_dir,
    int side_index,
    int max_steps,
    const uint8_t *wall_gfx_list,
    int wall_gfx_count,
    int *out_wall_gfx_index,
    int *out_wall_gfx_field) {
    int ordinal = -1;

    if (out_wall_gfx_index) *out_wall_gfx_index = -1;
    if (out_wall_gfx_field) *out_wall_gfx_field = -1;
    if (!wall_gfx_list || wall_gfx_count <= 0 ||
        !out_wall_gfx_index || !out_wall_gfx_field) {
        return -1;
    }
    if (dm2_v1_dungeon_find_actuator_wall_gfx_ordinal(
            d, first_thing, view_dir, side_index, max_steps, &ordinal) != 0) {
        return -1;
    }
    /* skproject GET_WALL_DECORATION_OF_ACTUATOR treats GraphicNumber() as
     * one-based and returns current_map_wall_gfx_list[ordinal - 1].  The
     * DRAW_DOOR_FRAMES custom-button path then uses field high-byte + 1;
     * a plain resolved actuator graphic has no animation frame here. */
    if (ordinal <= 0 || ordinal > wall_gfx_count) return -1;
    *out_wall_gfx_index = (int)wall_gfx_list[ordinal - 1];
    *out_wall_gfx_field = 1;
    return 0;
}

int dm2_v1_dungeon_get_map_wall_gfx_list(
    const DM2_V1_DungeonData *d,
    int level,
    uint8_t *out_wall_gfx_list,
    int out_capacity) {
    const uint8_t *map_desc;
    int wall_gfx_count;
    int creature_count;
    int map_base;
    int list_base;

    if (out_wall_gfx_list && out_capacity > 0) {
        memset(out_wall_gfx_list, 0, (size_t)out_capacity);
    }
    if (!d || level < 0 || level >= d->level_count ||
        !d->raw_data || d->raw_size <= 0 || out_capacity < 0) {
        return -1;
    }
    if (level >= DM2_V1_MAX_LEVELS ||
        d->raw_size < DM2_DUNGEON_HEADER_SIZE +
                      (level + 1) * DM2_MAP_DESC_SIZE) {
        return -1;
    }
    map_desc = d->raw_data + DM2_DUNGEON_HEADER_SIZE +
               level * DM2_MAP_DESC_SIZE;
    wall_gfx_count = (int)(RD16(map_desc + 10) & 0x0fu);
    creature_count = (int)((RD16(map_desc + 12) >> 4) & 0x0fu);
    if (wall_gfx_count <= 0) return 0;
    if (!out_wall_gfx_list || out_capacity < wall_gfx_count) return -1;
    if (d->raw_map_data_base < 0 ||
        d->level_widths[level] <= 0 ||
        d->level_heights[level] <= 0) {
        return -1;
    }

    /* skproject SKWIN/DME.h Map_definitions::WallGraphics/CreaturesTypes
     * and SKWINSPX skcore.cpp LOAD_LOCALLEVEL_GRAPHICS_TABLE line ~17500:
     * glbCurrentTileMap[width - 1][height + creature_count] starts the
     * current map's wall decoration list. */
    map_base = d->raw_map_data_base + d->level_offsets[level];
    list_base = map_base +
                d->level_widths[level] * d->level_heights[level] +
                creature_count;
    if (map_base < 0 || list_base < map_base ||
        list_base + wall_gfx_count > d->raw_size) {
        return -1;
    }
    memcpy(out_wall_gfx_list, d->raw_data + list_base,
           (size_t)wall_gfx_count);
    return wall_gfx_count;
}

int dm2_v1_dungeon_get_map_graphics_style(
    const DM2_V1_DungeonData *d,
    int level) {
    const uint8_t *map_desc;
    if (!d || level < 0 || level >= d->level_count ||
        !d->raw_data || d->raw_size <= 0) {
        return -1;
    }
    if (level >= DM2_V1_MAX_LEVELS ||
        d->raw_size < DM2_DUNGEON_HEADER_SIZE +
                      (level + 1) * DM2_MAP_DESC_SIZE) {
        return -1;
    }
    map_desc = d->raw_data + DM2_DUNGEON_HEADER_SIZE +
               level * DM2_MAP_DESC_SIZE;
    /* skproject/SKWIN/DME.h Map_definitions::MapGraphicsStyle() returns
     * `(w14 >> 4) & 15`; SkWinCore.cpp uses that index for
     * glbMapGraphicsSet before reading GRAPHICSSET scene words. */
    return (int)((RD16(map_desc + 14) >> 4) & 0x0fu);
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
        "Source: skproject SKWINSPX/src/v4/skcore.cpp GET_NEXT_RECORD_LINK reads record w0\n"
        "Fix: level_count from DUNGEON_HEADER.map_count (byte offset 6), not byte offset 0\n"
        "Fix: PC G1 real-data maps use byte squares with high-bit tile types from Map_definitions.w8\n"
        "Fix: skproject layout reads column object indexes, square-first things, text, DB pools, then byte map data\n"
        "Fix: tile offset = DM2_TILE_DATA_START(492) + raw_map_data_byte_offset\n"
        "Fix: column-major tile offset formula (col*height+row)*2\n"
        "Asset: DM2 PC English DUNGEON.DAT 6caccd7875009e82fe2e28e7f6d6adc0\n";
}
