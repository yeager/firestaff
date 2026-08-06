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

static void wr16le(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)(v >> 8);
}

static __attribute__((unused)) uint32_t
dm2_v1_g1_receipt_hash(const uint8_t *data, uint32_t byte_count)
{
    uint32_t hash = 2166136261u;
    uint32_t i;

    if (!data || byte_count == 0u) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

/* DUNGEON_HEADER field offsets */
#define DM2_HDR_MAP_COUNT_OFFSET   6
#define DM2_HDR_SEED_OFFSET       8

/* DUNGEON_HEADER size = 44 (ReDMCSB DEFS.H:985) */
#define DM2_DUNGEON_HEADER_SIZE  44

/* Dungeon magic identifiers at header offset 2.
 * PC DOS:     0x3147 = 'G1' (ASCII little-endian)
 * FM Towns:   0x3094 = different build, same LE header layout
 * Mac/Amiga:  0x313b = big-endian 68k build; reads as 0x3b31 via LE RD16 */
#define DM2_DUNGEON_MAGIC_PC       0x3147u
#define DM2_DUNGEON_MAGIC_FMTOWNS  0x3094u
#define DM2_DUNGEON_MAGIC_PC9821   0x3093u  /* PC-9821 JP (LE) */
#define DM2_DUNGEON_MAGIC_BE_LE    0x3b31u  /* Mac/Amiga 68k */
#define DM2_DUNGEON_MAGIC_MEGACD_BE_LE 0x9330u  /* Mega CD 68k */

static uint16_t rd16be(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

/* TILE DATA START = DUNGEON_HEADER(44) + MAP_DESCRIPTORS(28*16) = 492 */
#define DM2_TILE_DATA_START       (DM2_DUNGEON_HEADER_SIZE + 28 * 16)
#define DM2_MAP_DESC_SIZE 16
#define DM2_THING_TYPE_COUNT 16

/* PC DOS G1 keeps a 256-byte extension between the 28 Map_definitions and
 * the c_map.cpp-owned tables.  The real EN/FR dungeon member is identical;
 * the offsets below are derived from its 480-column monotonic prefix table.
 * skproject/SKWIN/SkWinCore.cpp READ_DUNGEON_STRUCTURE reads the same table
 * before dunGroundStacks, and c_map.cpp maps tile bit 0x10 through it. */
#define DM2_PC_G1_MAP_EXTENSION_BYTES       256
#define DM2_PC_G1_GROUND_STACK_COUNT_OFFSET 10
#define DM2_PC_G1_DB3_EXTENDED_COUNT        1024
#define DM2_PC_G1_DB4_EXTENDED_COUNT        300
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

static int dm2_v1_dungeon_record_list_traversal_allowed(
    const DM2_V1_DungeonData *d);

static int dm2_v1_dungeon_record_list_traversal_allowed(
    const DM2_V1_DungeonData *d) {
    return d && d->raw_data && d->record_graph_complete;
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
    out->level_count = map_count;
    out->square_bytes = 1;
    out->raw_map_data_base = -1;
    /* A raw SKSave has the source pool sequence directly before map bytes.
     * It has no PC G1 extension and must never enter the G1 partial-world
     * route merely because zero is a plausible byte offset. */
    out->g1_extension_base = -1;
    out->g1_extension_size = 0;
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
        out->map_use_door0[i] = (int)((RD16(map_desc + 2) >> 7) & 1u);
        out->map_use_door1[i] = (int)((RD16(map_desc + 2) >> 8) & 1u);
        out->map_door_ornate_count[i] = (int)(RD16(map_desc + 12) & 0x0fu);
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
    out->record_graph_complete = 1;
    if (!dm2_v1_dungeon_validate_record_graph(out))
        out->record_graph_complete = 0;
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
    {
        uint16_t magic = RD16(dat + 2);
        if (magic != DM2_DUNGEON_MAGIC_PC && magic != DM2_DUNGEON_MAGIC_FMTOWNS &&
            magic != DM2_DUNGEON_MAGIC_PC9821)
            return 0;
    }
    if (RD16(dat + 4) != DM2_DUNGEON_HEADER_SIZE)
        return 0;

    map_count = (int)dat[DM2_HDR_MAP_COUNT_OFFSET];
    if (map_count < 1 || map_count > DM2_V1_MAX_LEVELS) return 0;
    if (size < DM2_DUNGEON_HEADER_SIZE + map_count * DM2_MAP_DESC_SIZE)
        return 0;

    memset(out, 0, sizeof(*out));
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
        out->map_use_door0[i] = (int)((RD16(map_desc + 2) >> 7) & 1u);
        out->map_use_door1[i] = (int)((RD16(map_desc + 2) >> 8) & 1u);
        out->map_door_ornate_count[i] = (int)(RD16(map_desc + 12) & 0x0fu);
        out->level_types[i] = (i == 0) ? DM2_LEVEL_OUTDOOR : DM2_LEVEL_INDOOR;
        total_columns += w;
    }

    /* File_header stores the start pose in 5/5/2 bits; Map_definitions
     * supplies the map origin.  Admit only the wrapped local coordinate
     * when it is owned by map 0, never a fixed boot position. */
    {
        uint16_t start = RD16(dat + 10);
        int x = ((int)(start & 0x1fu) - (out->map_offset_x[0] & 0x1f)) & 0x1f;
        int y = ((int)((start >> 5) & 0x1fu) -
                 (out->map_offset_y[0] & 0x1f)) & 0x1f;
        if (x < out->level_widths[0] && y < out->level_heights[0]) {
            out->initial_party_pose_valid = 1;
            out->initial_party_x = x;
            out->initial_party_y = y;
            out->initial_party_dir = (int)((start >> 10) & 0x03u);
        }
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
    out->raw_data = (uint8_t *)malloc((size_t)size);
    if (!out->raw_data) return -1;
    memcpy(out->raw_data, dat, (size_t)size);
    out->raw_size = size;
    dm2_v1_configure_pc_g1_extension_records(out);
    /* The public validator intentionally rejects unpromoted graphs. Enable
     * this candidate only for its complete bounded walk, then clear it again
     * on the first invalid root/link/cycle. */
    out->record_graph_complete = 1;
    out->g1_w0_chains_disabled =
        (out->g1_extension_record_counts[3] > 0 ||
         out->g1_extension_record_counts[4] > 0) ? 1 : 0;
    if (!dm2_v1_dungeon_validate_record_graph(out))
        out->record_graph_complete = 0;
    out->partial_map_boot.valid = 1;
    out->partial_map_boot.committed = 1;
    out->partial_map_boot.incomplete = out->record_graph_complete ? 0 : 1;
    out->partial_map_boot.map_count = out->level_count;
    out->partial_map_boot.square_bytes = out->square_bytes;
    out->partial_map_boot.column_index_base = out->column_index_base;
    out->partial_map_boot.ground_stack_base = out->square_first_thing_base;
    out->partial_map_boot.ground_stack_count = out->square_first_thing_count;
    out->partial_map_boot.text_data_base = out->text_data_base;
    out->partial_map_boot.text_word_count = out->text_word_count;
    out->partial_map_boot.candidate_pool_base =
        out->text_data_base + out->text_word_count * 2;
    out->partial_map_boot.candidate_pool_end = out->g1_extension_base;
    out->partial_map_boot.g1_extension_base = out->g1_extension_base;
    out->partial_map_boot.g1_extension_size = out->g1_extension_size;
    out->partial_map_boot.raw_map_data_base = out->raw_map_data_base;
    out->partial_map_boot.record_graph_complete =
        out->record_graph_complete;
    return 1;
}

/* ── Mac/Amiga big-endian G1 byte-square loader ──────────────────── */

static int dm2_decode_map_dimensions_from_w8_be(const uint8_t *map_desc,
                                                int *out_w,
                                                int *out_h) {
    uint16_t w8;
    int w;
    int h;

    if (!map_desc || !out_w || !out_h) return 0;
    w8 = rd16be(map_desc + 8);
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

/* Mac 68k and Amiga AGA DUNGEON.DAT: big-endian u16 fields throughout,
 * except offset 4-5 (header_size) which is LE, and map descriptor byte
 * fields at desc+6,desc+7 which are individual bytes unaffected by endian.
 * Magic 0x313b at offset 2-3 (reads as 0x3b31 via LE RD16). */
static int dm2_v1_try_load_be_byte_layout(DM2_V1_DungeonData *out,
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
    {
        uint16_t be_magic = RD16(dat + 2);
        if (be_magic != DM2_DUNGEON_MAGIC_BE_LE &&
            be_magic != DM2_DUNGEON_MAGIC_MEGACD_BE_LE)
            return 0;
    }
    if (RD16(dat + 4) != DM2_DUNGEON_HEADER_SIZE)
        return 0;

    map_count = (int)rd16be(dat + 6);
    if (map_count < 1 || map_count > DM2_V1_MAX_LEVELS) return 0;
    if (size < DM2_DUNGEON_HEADER_SIZE + map_count * DM2_MAP_DESC_SIZE)
        return 0;

    memset(out, 0, sizeof(*out));
    out->level_count = map_count;
    out->square_bytes = 1;
    out->raw_map_data_base = -1;
    out->column_index_base = -1;
    out->square_first_thing_base = -1;
    out->text_data_base = -1;
    out->g1_extension_base = -1;
    out->g1_extension_size = 0;
    out->square_first_thing_count =
        (int)rd16be(dat + DM2_PC_G1_GROUND_STACK_COUNT_OFFSET);
    out->text_word_count = (int)rd16be(dat + 8);
    for (int i = 0; i < DM2_THING_TYPE_COUNT; ++i) {
        out->thing_data_bases[i] = -1;
        out->thing_type_counts[i] = (int)rd16be(dat + 14 + i * 2);
    }

    for (int i = 0; i < map_count; ++i) {
        const uint8_t *map_desc =
            dat + DM2_DUNGEON_HEADER_SIZE + i * DM2_MAP_DESC_SIZE;
        int w = 0;
        int h = 0;
        int rel_offset = (int)rd16be(map_desc + 0);
        int end;

        if (!dm2_decode_map_dimensions_from_w8_be(map_desc, &w, &h))
            return 0;
        end = rel_offset + w * h;
        if (end < rel_offset) return 0;
        if (end > raw_map_bytes) raw_map_bytes = end;

        out->level_widths[i] = w;
        out->level_heights[i] = h;
        out->level_offsets[i] = rel_offset;
        out->map_offset_x[i] = (int)map_desc[6];
        out->map_offset_y[i] = (int)map_desc[7];
        out->map_door_set0[i] = (int)((rd16be(map_desc + 14) >> 8) & 0x0fu);
        out->map_door_set1[i] = (int)((rd16be(map_desc + 14) >> 12) & 0x0fu);
        out->map_use_door0[i] = (int)((rd16be(map_desc + 2) >> 7) & 1u);
        out->map_use_door1[i] = (int)((rd16be(map_desc + 2) >> 8) & 1u);
        out->map_door_ornate_count[i] = (int)(rd16be(map_desc + 12) & 0x0fu);
        out->level_types[i] = (i == 0) ? DM2_LEVEL_OUTDOOR : DM2_LEVEL_INDOOR;
        total_columns += w;
    }

    {
        uint16_t start = rd16be(dat + 10);
        int x = ((int)(start & 0x1fu) - (out->map_offset_x[0] & 0x1f)) & 0x1f;
        int y = ((int)((start >> 5) & 0x1fu) -
                 (out->map_offset_y[0] & 0x1f)) & 0x1f;
        if (x < out->level_widths[0] && y < out->level_heights[0]) {
            out->initial_party_pose_valid = 1;
            out->initial_party_x = x;
            out->initial_party_y = y;
            out->initial_party_dir = (int)((start >> 10) & 0x03u);
        }
    }

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

    out->g1_extension_base = thing_cursor;
    out->g1_extension_size = (size - raw_map_bytes) - thing_cursor;
    if (out->g1_extension_size <= 0) return 0;
    out->raw_map_data_base = size - raw_map_bytes;
    out->column_index_base = column_index_base;
    out->square_first_thing_base = sft_base;
    out->text_data_base = text_base;
    out->raw_data = (uint8_t *)malloc((size_t)size);
    if (!out->raw_data) return -1;
    memcpy(out->raw_data, dat, (size_t)size);
    out->raw_size = size;
    out->record_graph_complete = 1;
    out->g1_w0_chains_disabled = 1;
    if (!dm2_v1_dungeon_validate_record_graph(out))
        out->record_graph_complete = 0;
    out->partial_map_boot.valid = 1;
    out->partial_map_boot.committed = 1;
    out->partial_map_boot.incomplete = out->record_graph_complete ? 0 : 1;
    out->partial_map_boot.map_count = out->level_count;
    out->partial_map_boot.square_bytes = out->square_bytes;
    out->partial_map_boot.column_index_base = out->column_index_base;
    out->partial_map_boot.ground_stack_base = out->square_first_thing_base;
    out->partial_map_boot.ground_stack_count = out->square_first_thing_count;
    out->partial_map_boot.text_data_base = out->text_data_base;
    out->partial_map_boot.text_word_count = out->text_word_count;
    out->partial_map_boot.candidate_pool_base =
        out->text_data_base + out->text_word_count * 2;
    out->partial_map_boot.candidate_pool_end = out->g1_extension_base;
    out->partial_map_boot.g1_extension_base = out->g1_extension_base;
    out->partial_map_boot.g1_extension_size = out->g1_extension_size;
    out->partial_map_boot.raw_map_data_base = out->raw_map_data_base;
    out->partial_map_boot.record_graph_complete =
        out->record_graph_complete;
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

    skproject_layout = dm2_v1_try_load_be_byte_layout(out, dat, size);
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
    /* Every public loader result promises a complete owned raw source
     * buffer.  Returning success with a NULL buffer would turn allocation
     * failure into a partial dungeon handoff.  Keep the source-data boundary
     * fail-closed instead. */
    if (!out->raw_data) return -1;
    memcpy(out->raw_data, dat, (size_t)size);
    out->raw_size = size;

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

uint32_t dm2_v1_skproject_tile_to_ulong(uint16_t tile)
{
    return (uint32_t)tile;
}

uint8_t dm2_v1_skproject_tile_to_ubyte(uint16_t tile)
{
    return (uint8_t)tile;
}

uint16_t dm2_v1_skproject_mk_record(int16_t record)
{
    return (uint16_t)record;
}

int16_t dm2_v1_skproject_record_to_word(uint16_t record)
{
    return (int16_t)record;
}

int32_t dm2_v1_skproject_record_to_long(uint16_t record)
{
    return (int32_t)(int16_t)record;
}

static int dm2_v1_dungeon_c_map_dimension_ok(const DM2_V1_DungeonData *d,
                                             int level)
{
    return d && d->raw_data && level >= 0 && level < d->level_count &&
           d->level_widths[level] > 0 && d->level_heights[level] > 0;
}

const uint8_t *dm2_v1_dungeon_c_map_get_address_of_tile_record(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    uint16_t *out_object_id,
    int *out_record_offset)
{
    uint16_t object_id;
    const uint8_t *record;
    int type = -1;
    int index = -1;
    int size = 0;

    if (out_object_id) *out_object_id = DM2_THING_END_MARKER;
    if (out_record_offset) *out_record_offset = -1;
    if (!dm2_v1_dungeon_c_map_dimension_ok(d, level) ||
        x < 0 || x >= d->level_widths[level] ||
        y < 0 || y >= d->level_heights[level]) {
        return NULL;
    }
    object_id = (uint16_t)dm2_v1_dungeon_get_first_thing(d, level, x, y);
    if (object_id == DM2_THING_END_MARKER ||
        object_id == DM2_THING_NULL_MARKER) {
        return NULL;
    }
    record = dm2_v1_dungeon_get_thing_record(
        d, object_id, &type, &index, &size);
    if (!record || type < 0 || index < 0 || size <= 0) return NULL;
    if (out_object_id) *out_object_id = object_id;
    if (out_record_offset)
        *out_record_offset = (int)(record - d->raw_data);
    return record;
}

int dm2_v1_dungeon_c_map_is_tile_passage(const DM2_V1_DungeonData *d,
                                         int level,
                                         int x,
                                         int y)
{
    int raw;
    int tile_type;

    if (!dm2_v1_dungeon_c_map_dimension_ok(d, level) ||
        x < 0 || x >= d->level_widths[level] ||
        y < 0 || y >= d->level_heights[level]) {
        return 0;
    }
    raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (raw < 0) return 0;
    raw &= 0xff;
    tile_type = raw >> 5;
    if (tile_type == 0 || tile_type == 7) return 0;
    return 1;
}

int dm2_v1_dungeon_c_map_is_tile_solid(const DM2_V1_DungeonData *d,
                                       int level,
                                       int x,
                                       int y)
{
    int raw;
    int tile_type;

    if (!dm2_v1_dungeon_c_map_dimension_ok(d, level) ||
        x < 0 || x >= d->level_widths[level] ||
        y < 0 || y >= d->level_heights[level]) {
        return 1;
    }
    raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (raw < 0) return 1;
    if (d->square_bytes == 1) {
        tile_type = (((uint8_t)raw) >> 5) & 0x07;
        return tile_type == 0 || tile_type == 7;
    }
    tile_type = raw & 0x1f;
    return tile_type == 0 || tile_type == 4;
}

int dm2_v1_dungeon_c_map_get_tile_value(const DM2_V1_DungeonData *d,
                                        int level,
                                        int x,
                                        int y)
{
    int width;
    int height;
    int x_in;
    int y_in;
    int edge_x;
    int edge_y;

    if (!dm2_v1_dungeon_c_map_dimension_ok(d, level)) return -1;
    width = d->level_widths[level];
    height = d->level_heights[level];
    x_in = (x >= 0 && x < width);
    y_in = (y >= 0 && y < height);
    if (x_in && y_in) {
        int raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
        return raw < 0 ? -1 : (raw & 0xff);
    }
    if (x < -1 || x > width || y < -1 || y > height) return 0xe0;

    edge_x = x;
    edge_y = y;
    if (!y_in && !x_in) {
        if (x == -1) edge_x = 0;
        else if (x == width) edge_x = width - 1;
        else return 0xe0;
        if (y == -1) {
            if (!dm2_v1_dungeon_c_map_is_tile_passage(d, level, edge_x, 0))
                return 0xe0;
            edge_y = 0;
        } else if (y == height) {
            edge_y = height - 1;
        } else {
            return 0xe0;
        }
    } else if (!y_in) {
        if (y == -1) {
            edge_y = 0;
            if (dm2_v1_dungeon_c_map_is_tile_passage(d, level, x, edge_y))
                return 0x02;
            if (x > 0 &&
                dm2_v1_dungeon_c_map_is_tile_passage(d, level, x - 1, edge_y))
                return 0x00;
            if (x + 1 < width) edge_x = x + 1;
            else return 0xe0;
        } else if (y == height) {
            edge_y = height - 1;
            if (dm2_v1_dungeon_c_map_is_tile_passage(d, level, x, edge_y))
                return 0x08;
            if (x > 0 &&
                dm2_v1_dungeon_c_map_is_tile_passage(d, level, x - 1, edge_y))
                return 0x00;
            if (x + 1 < width) edge_x = x + 1;
            else return 0xe0;
        } else {
            return 0xe0;
        }
    } else if (!x_in) {
        if (x == -1) {
            edge_x = 0;
            if (dm2_v1_dungeon_c_map_is_tile_passage(d, level, edge_x, y))
                return 0x04;
            if (y > 0 &&
                dm2_v1_dungeon_c_map_is_tile_passage(d, level, edge_x, y - 1))
                return 0x00;
            if (y + 1 < height) edge_y = y + 1;
            else return 0xe0;
        } else if (x == width) {
            edge_x = width - 1;
            if (dm2_v1_dungeon_c_map_is_tile_passage(d, level, edge_x, y))
                return 0x01;
            if (y > 0 &&
                dm2_v1_dungeon_c_map_is_tile_passage(d, level, edge_x, y - 1))
                return 0x00;
            if (y + 1 < height) edge_y = y + 1;
            else return 0xe0;
        } else {
            return 0xe0;
        }
    }

    if (edge_x < 0 || edge_x >= width || edge_y < 0 || edge_y >= height)
        return 0xe0;
    if (dm2_v1_dungeon_c_map_is_tile_passage(d, level, edge_x, edge_y))
        return 0x00;
    return 0xe0;
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

int dm2_v1_dungeon_set_first_thing(DM2_V1_DungeonData *d,
                                   int level,
                                   int x,
                                   int y,
                                   uint16_t first) {
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
    if (d->square_bytes != 1) return -1; /* inline head: no table slot */
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
    wr16le(d->raw_data + d->square_first_thing_base + thing_index * 2,
           first);
    return 0;
}

int dm2_v1_skproject_get_tile_value(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTileValueReceipt *out) {
    DM2_V1_SkprojectTileValueReceipt receipt;
    int raw;

    if (out) memset(out, 0, sizeof(*out));
    if (!out) return 0;
    raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (raw < 0) return 0;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.level = level;
    receipt.x = x;
    receipt.y = y;
    receipt.raw_tile = (uint16_t)raw;
    receipt.tile_value = d && d->square_bytes == 1
                             ? (uint8_t)(((uint8_t)raw >> 5) & 0x07u)
                             : (uint8_t)((uint16_t)raw & 0x1fu);
    receipt.source_symbol = "DM2_GET_TILE_VALUE";
    receipt.source_line = 107;
    *out = receipt;
    return 1;
}

int dm2_v1_skproject_is_tile_passage(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTilePassageReceipt *out) {
    DM2_V1_SkprojectTileValueReceipt value;
    DM2_V1_SkprojectTilePassageReceipt receipt;

    if (out) memset(out, 0, sizeof(*out));
    if (!out || !dm2_v1_skproject_get_tile_value(d, level, x, y, &value))
        return 0;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.level = level;
    receipt.x = x;
    receipt.y = y;
    receipt.raw_tile = value.raw_tile;
    receipt.tile_value = value.tile_value;
    /* skproject DME.h::tileTypeIndex: G1 byte maps store passage/floor as
     * tile class 1. Older 16-bit fixtures keep the DM1-style low five bits;
     * only non-wall floor-like values are admitted as passage here. */
    receipt.is_passage = d && d->square_bytes == 1
                             ? (value.tile_value == 1u)
                             : (value.tile_value != 0u &&
                                value.tile_value != 4u);
    receipt.source_symbol = "DM2_IS_TILE_PASSAGE";
    receipt.source_line = 79;
    *out = receipt;
    return 1;
}

int dm2_v1_skproject_is_tile_solid(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTileSolidReceipt *out) {
    DM2_V1_SkprojectTileValueReceipt value;
    DM2_V1_SkprojectTileSolidReceipt receipt;

    if (out) memset(out, 0, sizeof(*out));
    if (!out || !dm2_v1_skproject_get_tile_value(d, level, x, y, &value))
        return 0;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.level = level;
    receipt.x = x;
    receipt.y = y;
    receipt.raw_tile = value.raw_tile;
    receipt.tile_value = value.tile_value;
    receipt.is_solid = dm2_v1_dungeon_c_map_is_tile_solid(d, level, x, y);
    receipt.source_symbol = "DM2_IS_TILE_SOLID";
    receipt.source_line = 3871;
    *out = receipt;
    return 1;
}

int dm2_v1_skproject_get_address_of_tile_record(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTileRecordAddressReceipt *out) {
    DM2_V1_SkprojectTileRecordAddressReceipt receipt;
    int raw;
    int object_id;
    int type = -1;
    int index = -1;
    int size = 0;
    const uint8_t *record;

    if (out) memset(out, 0, sizeof(*out));
    if (!out) return 0;
    raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (raw < 0) return 0;

    memset(&receipt, 0, sizeof(receipt));
    receipt.level = level;
    receipt.x = x;
    receipt.y = y;
    receipt.raw_tile = (uint16_t)raw;
    receipt.source_symbol = "DM2_GET_ADDRESS_OF_TILE_RECORD";
    receipt.source_line = 69;

    object_id = dm2_v1_dungeon_get_first_thing(d, level, x, y);
    if (object_id < 0 || object_id == (int)DM2_THING_END_MARKER) {
        receipt.blocked_no_tile_record_link = 1;
        *out = receipt;
        return 0;
    }

    receipt.object_id = (uint16_t)object_id;
    record = dm2_v1_dungeon_get_thing_record(
        d, (uint16_t)object_id, &type, &index, &size);
    receipt.type = type;
    receipt.index = index;
    receipt.record_size = size;
    if (!record || size <= 0) {
        receipt.blocked_missing_record = 1;
        *out = receipt;
        return 0;
    }

    receipt.valid = 1;
    receipt.direct_or_proven_extension_address = 1;
    *out = receipt;
    return 1;
}

static uint32_t dm2_v1_raw_sksave_scene_hash_step(uint32_t hash,
                                                   uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

int dm2_v1_dungeon_collect_raw_sksave_map_scene(
    const DM2_V1_DungeonData *d, int map,
    DM2_V1_RawSKSaveMapSceneReceipt *out)
{
    DM2_V1_RawSKSaveMapSceneReceipt candidate;
    uint32_t map_hash = 2166136261u;
    uint32_t terrain_hash = 2166136261u;
    uint32_t object_hash = 2166136261u;
    int x;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    /* The raw save route is intentionally not a second PC G1 decoder.
     * SKSAVE READ_DUNGEON_STRUCTURE owns its directly contiguous pools;
     * a positive extension belongs to the separate PC G1 admission path. */
    if (!d || !d->raw_data || d->square_bytes != 1 ||
        d->g1_extension_base >= 0 ||
        map < 0 || map >= d->level_count || d->level_widths[map] <= 0 ||
        d->level_heights[map] <= 0 || d->raw_map_data_base < 0 ||
        d->level_offsets[map] < 0) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.map = map;
    candidate.width = d->level_widths[map];
    candidate.height = d->level_heights[map];
    for (x = 0; x < candidate.width; ++x) {
        int y;
        for (y = 0; y < candidate.height; ++y) {
            DM2_V1_SkprojectTileRecordAddressReceipt address;
            const uint8_t *record;
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            int record_type = -1;
            int record_index = -1;
            int record_size = 0;

            if (raw < 0) return 0;
            terrain_hash = dm2_v1_raw_sksave_scene_hash_step(
                terrain_hash, (uint32_t)(uint8_t)raw);
            if ((raw & 0x10) == 0) continue;
            if (candidate.thing_bearing_tile_count == UINT16_MAX ||
                !d->record_graph_complete ||
                !dm2_v1_skproject_get_address_of_tile_record(
                    d, map, x, y, &address) || !address.valid) {
                return 0;
            }
            record = dm2_v1_dungeon_get_thing_record(
                d, address.object_id, &record_type, &record_index,
                &record_size);
            if (!record || record_type != address.type ||
                record_index != address.index || record_size != address.record_size ||
                record_type < 0 || record_type >= 16 || record_size <= 0 ||
                candidate.addressable_root_count == UINT16_MAX ||
                candidate.root_count_by_type[record_type] == UINT16_MAX) {
                return 0;
            }
            ++candidate.thing_bearing_tile_count;
            ++candidate.addressable_root_count;
            ++candidate.root_count_by_type[record_type];
            object_hash = dm2_v1_raw_sksave_scene_hash_step(
                object_hash, address.object_id);
            object_hash = dm2_v1_raw_sksave_scene_hash_step(
                object_hash, (uint32_t)record_type);
            object_hash = dm2_v1_raw_sksave_scene_hash_step(
                object_hash, (uint32_t)record_index);
            for (int byte = 0; byte < record_size; ++byte) {
                object_hash = dm2_v1_raw_sksave_scene_hash_step(
                    object_hash, record[byte]);
            }
        }
    }
    for (int byte = 0; byte < candidate.width * candidate.height; ++byte) {
        map_hash = dm2_v1_raw_sksave_scene_hash_step(
            map_hash,
            d->raw_data[d->raw_map_data_base + d->level_offsets[map] + byte]);
    }
    candidate.map_data_hash = map_hash;
    candidate.terrain_hash = terrain_hash;
    candidate.object_record_hash = object_hash;
    candidate.valid = candidate.map_data_hash != 0u &&
                      candidate.terrain_hash != 0u &&
                      candidate.object_record_hash != 0u;
    if (!candidate.valid) return 0;
    *out = candidate;
    return 1;
}

int dm2_v1_skproject_get_object_index_from_tile(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectObjectIndexReceipt *out) {
    DM2_V1_SkprojectObjectIndexReceipt receipt;
    int raw;
    int column_index = 0;
    int column_offset;
    int square_offset;
    int object_index;
    int preceding_root_count = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level = level;
    receipt.x = x;
    receipt.y = y;
    receipt.object_index = -1;
    receipt.source_symbol = "DM2_GET_OBJECT_INDEX_FROM_TILE";
    receipt.source_line = 44;

    raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (!d || !d->raw_data || raw < 0 || level < 0 ||
        level >= d->level_count || x < 0 ||
        x >= d->level_widths[level] || y < 0 ||
        y >= d->level_heights[level]) {
        *out = receipt;
        return 0;
    }
    receipt.raw_tile = (uint16_t)raw;
    if (d->square_bytes != 1 || (raw & 0x10) == 0 ||
        d->column_index_base < 0 || d->square_first_thing_base < 0) {
        receipt.blocked_no_tile_record_link = 1;
        *out = receipt;
        return 0;
    }

    for (int i = 0; i < level; ++i)
        column_index += d->level_widths[i];
    column_offset = d->column_index_base + (column_index + x) * 2;
    square_offset = d->raw_map_data_base + d->level_offsets[level] +
                    x * d->level_heights[level];
    if (column_offset < 0 || column_offset + 1 >= d->raw_size ||
        square_offset < 0 || square_offset + y >= d->raw_size) {
        *out = receipt;
        return 0;
    }

    object_index = (int)RD16(d->raw_data + column_offset);
    for (int row = 0; row < y; ++row) {
        if ((d->raw_data[square_offset + row] & 0x10u) != 0u) {
            ++object_index;
            ++preceding_root_count;
        }
    }
    if (object_index < 0 || object_index >= d->square_first_thing_count) {
        *out = receipt;
        return 0;
    }
    if (d->square_first_thing_base + object_index * 2 + 1 >= d->raw_size) {
        *out = receipt;
        return 0;
    }

    receipt.valid = 1;
    receipt.object_index = object_index;
    receipt.column_base_index = (int)RD16(d->raw_data + column_offset);
    receipt.column_index_offset = column_offset;
    receipt.object_index_offset = d->square_first_thing_base + object_index * 2;
    receipt.object_id = RD16(d->raw_data + receipt.object_index_offset);
    receipt.preceding_root_count = preceding_root_count;
    *out = receipt;
    return 1;
}

int dm2_v1_skproject_change_current_map_to(
    const DM2_V1_DungeonData *d,
    int previous_map,
    int new_map,
    int player_x,
    int player_y,
    int player_map,
    int player_dir,
    DM2_V1_SkprojectChangeCurrentMapReceipt *out) {
    DM2_V1_SkprojectChangeCurrentMapReceipt receipt;
    int column_index = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.requested_map = new_map;
    receipt.previous_map = previous_map;
    receipt.current_map = previous_map;
    receipt.player_x = player_x;
    receipt.player_y = player_y;
    receipt.player_map = player_map;
    receipt.player_dir = player_dir;
    receipt.source_symbol = "CHANGE_CURRENT_MAP_TO";
    receipt.source_line = 2774;

    if (!d || !d->raw_data) {
        *out = receipt;
        return 0;
    }
    if (new_map == previous_map) {
        receipt.valid = 1;
        receipt.unchanged = 1;
        *out = receipt;
        return 1;
    }
    if (new_map < 0) {
        receipt.blocked_negative_map = 1;
        *out = receipt;
        return 0;
    }
    if (new_map >= d->level_count) {
        receipt.blocked_map_range = 1;
        *out = receipt;
        return 0;
    }

    for (int i = 0; i < new_map; ++i)
        column_index += d->level_widths[i];
    receipt.current_map = new_map;
    receipt.width = d->level_widths[new_map];
    receipt.height = d->level_heights[new_map];
    receipt.raw_tile_map_offset =
        d->raw_map_data_base + d->level_offsets[new_map];
    receipt.column_index_offset =
        d->column_index_base >= 0 ? d->column_index_base + column_index * 2 : -1;
    if (receipt.width <= 0 || receipt.height <= 0 ||
        receipt.raw_tile_map_offset < 0 ||
        receipt.raw_tile_map_offset >= d->raw_size ||
        receipt.column_index_offset < 0 ||
        receipt.column_index_offset + 1 >= d->raw_size) {
        *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    *out = receipt;
    return 1;
}

static int dm2_v1_dungeon_total_columns(const DM2_V1_DungeonData *d) {
    int total = 0;
    if (!d) return 0;
    for (int i = 0; i < d->level_count; ++i)
        total += d->level_widths[i];
    return total;
}

static int dm2_v1_dungeon_column_base_index(const DM2_V1_DungeonData *d,
                                             int level) {
    int column = 0;
    if (!d || level < 0 || level >= d->level_count) return -1;
    for (int i = 0; i < level; ++i)
        column += d->level_widths[i];
    return column;
}

static int dm2_v1_dungeon_byte_square_offset(const DM2_V1_DungeonData *d,
                                              int level,
                                              int x,
                                              int y) {
    if (!d || level < 0 || level >= d->level_count || x < 0 || y < 0 ||
        x >= d->level_widths[level] || y >= d->level_heights[level] ||
        d->square_bytes != 1) {
        return -1;
    }
    return d->raw_map_data_base + d->level_offsets[level] +
           x * d->level_heights[level] + y;
}

static int dm2_v1_dungeon_compute_object_index_for_square(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    int *out_object_index,
    int *out_object_index_offset) {
    int column_base;
    int column_offset;
    int square_column_offset;
    int object_index;

    if (out_object_index) *out_object_index = -1;
    if (out_object_index_offset) *out_object_index_offset = -1;
    if (!d || !d->raw_data || d->square_bytes != 1 ||
        d->column_index_base < 0 || d->square_first_thing_base < 0) {
        return 0;
    }
    column_base = dm2_v1_dungeon_column_base_index(d, level);
    square_column_offset = dm2_v1_dungeon_byte_square_offset(d, level, x, 0);
    if (column_base < 0 || square_column_offset < 0) return 0;
    column_offset = d->column_index_base + (column_base + x) * 2;
    if (column_offset < 0 || column_offset + 1 >= d->raw_size)
        return 0;
    object_index = (int)RD16(d->raw_data + column_offset);
    for (int row = 0; row < y; ++row) {
        if ((d->raw_data[square_column_offset + row] & 0x10u) != 0u)
            ++object_index;
    }
    if (object_index < 0 || object_index >= d->square_first_thing_count)
        return 0;
    if (d->square_first_thing_base + object_index * 2 + 1 >= d->raw_size)
        return 0;
    if (out_object_index) *out_object_index = object_index;
    if (out_object_index_offset)
        *out_object_index_offset =
            d->square_first_thing_base + object_index * 2;
    return 1;
}

int dm2_v1_skproject_append_record_to(
    DM2_V1_DungeonData *d,
    uint16_t record_to_append,
    uint16_t *parent_link,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectAppendRecordReceipt *out) {
    DM2_V1_SkprojectAppendRecordReceipt receipt;
    uint8_t *append_record;
    int append_type = -1;
    int append_index = -1;
    int append_size = 0;
    int square_offset;

    if (out) memset(out, 0, sizeof(*out));
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level = level;
    receipt.x = x;
    receipt.y = y;
    receipt.appended_object_id = record_to_append;
    receipt.source_symbol = "APPEND_RECORD_TO";
    receipt.source_line = 2544;

    if (record_to_append == DM2_THING_END_MARKER ||
        record_to_append == DM2_THING_NULL_MARKER) {
        receipt.blocked_null_or_end_append = 1;
        *out = receipt;
        return 0;
    }
    append_record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
        d, record_to_append, &append_type, &append_index, &append_size);
    receipt.appended_type = append_type;
    receipt.appended_index = append_index;
    receipt.appended_record_size = append_size;
    if (!append_record || append_size < 2) {
        receipt.blocked_missing_appended_record = 1;
        *out = receipt;
        return 0;
    }
    receipt.appended_previous_next = RD16(append_record);
    append_record[0] = (uint8_t)(DM2_THING_END_MARKER & 0xffu);
    append_record[1] = (uint8_t)(DM2_THING_END_MARKER >> 8);

    if (x < 0) {
        uint16_t current;
        int steps = 0;
        int max_steps = 0;

        if (!parent_link || !d) {
            receipt.blocked_invalid_parent = 1;
            *out = receipt;
            return 0;
        }
        for (int type = 0; type < DM2_THING_TYPE_COUNT; ++type)
            max_steps += d->thing_type_counts[type] +
                         d->g1_extension_record_counts[type];
        if (max_steps <= 0) max_steps = 1;
        receipt.parent_link_route = 1;
        receipt.parent_previous_link = *parent_link;
        if (*parent_link == DM2_THING_END_MARKER) {
            *parent_link = record_to_append;
            receipt.parent_new_link = *parent_link;
            receipt.valid = 1;
            *out = receipt;
            return 1;
        }
        current = *parent_link;
        while (current != DM2_THING_END_MARKER) {
            uint8_t *record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
                d, current, NULL, NULL, NULL);
            uint16_t next;
            if (!record || ++steps > max_steps) {
                receipt.blocked_unbounded_graph = 1;
                *out = receipt;
                return 0;
            }
            next = RD16(record);
            if (next == DM2_THING_END_MARKER) {
                record[0] = (uint8_t)(record_to_append & 0xffu);
                record[1] = (uint8_t)(record_to_append >> 8);
                receipt.tail_object_id = current;
                receipt.parent_new_link = *parent_link;
                receipt.valid = 1;
                *out = receipt;
                return 1;
            }
            current = next;
        }
    }

    square_offset = dm2_v1_dungeon_byte_square_offset(d, level, x, y);
    if (!d || !d->raw_data || square_offset < 0 ||
        d->square_first_thing_count <= 0) {
        receipt.blocked_invalid_tile = 1;
        *out = receipt;
        return 0;
    }

    if ((d->raw_data[square_offset] & 0x10u) != 0u) {
        int first = dm2_v1_dungeon_get_first_thing(d, level, x, y);
        uint16_t current = (uint16_t)first;
        int steps = 0;
        int max_steps = 0;

        if (first < 0 || first == (int)DM2_THING_END_MARKER) {
            receipt.blocked_invalid_tile = 1;
            *out = receipt;
            return 0;
        }
        for (int type = 0; type < DM2_THING_TYPE_COUNT; ++type)
            max_steps += d->thing_type_counts[type] +
                         d->g1_extension_record_counts[type];
        if (max_steps <= 0) max_steps = 1;
        receipt.existing_tile_chain_route = 1;
        receipt.parent_previous_link = (uint16_t)first;
        while (current != DM2_THING_END_MARKER) {
            uint8_t *record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
                d, current, NULL, NULL, NULL);
            uint16_t next;
            if (!record || ++steps > max_steps) {
                receipt.blocked_unbounded_graph = 1;
                *out = receipt;
                return 0;
            }
            next = RD16(record);
            if (next == DM2_THING_END_MARKER) {
                record[0] = (uint8_t)(record_to_append & 0xffu);
                record[1] = (uint8_t)(record_to_append >> 8);
                receipt.tail_object_id = current;
                receipt.parent_new_link = (uint16_t)first;
                receipt.valid = 1;
                *out = receipt;
                return 1;
            }
            current = next;
        }
    } else {
        int object_index = -1;
        int object_index_offset = -1;
        int column_base;
        int total_columns;
        int last_offset;

        if (!dm2_v1_dungeon_compute_object_index_for_square(
                d, level, x, y, &object_index, &object_index_offset)) {
            receipt.blocked_invalid_tile = 1;
            *out = receipt;
            return 0;
        }
        last_offset = d->square_first_thing_base +
                      (d->square_first_thing_count - 1) * 2;
        if (last_offset < 0 || last_offset + 1 >= d->raw_size ||
            RD16(d->raw_data + last_offset) != DM2_THING_NULL_MARKER) {
            receipt.blocked_no_ground_stack_space = 1;
            *out = receipt;
            return 0;
        }
        memmove(d->raw_data + object_index_offset + 2,
                d->raw_data + object_index_offset,
                (size_t)(last_offset - object_index_offset));
        d->raw_data[object_index_offset] =
            (uint8_t)(record_to_append & 0xffu);
        d->raw_data[object_index_offset + 1] =
            (uint8_t)(record_to_append >> 8);
        d->raw_data[square_offset] |= 0x10u;

        column_base = dm2_v1_dungeon_column_base_index(d, level);
        total_columns = dm2_v1_dungeon_total_columns(d);
        for (int col = column_base + x + 1; col < total_columns; ++col) {
            int offset = d->column_index_base + col * 2;
            uint16_t value;
            if (offset < 0 || offset + 1 >= d->raw_size) {
                receipt.blocked_invalid_tile = 1;
                *out = receipt;
                return 0;
            }
            value = (uint16_t)(RD16(d->raw_data + offset) + 1u);
            d->raw_data[offset] = (uint8_t)(value & 0xffu);
            d->raw_data[offset + 1] = (uint8_t)(value >> 8);
            receipt.incremented_column_offsets++;
        }
        receipt.empty_tile_insert_route = 1;
        receipt.object_index = object_index;
        receipt.object_index_offset = object_index_offset;
        receipt.shifted_ground_stack_words =
            d->square_first_thing_count - object_index - 1;
        receipt.parent_new_link = record_to_append;
        receipt.valid = 1;
        *out = receipt;
        return 1;
    }

    receipt.blocked_unbounded_graph = 1;
    *out = receipt;
    return 0;
}

int dm2_v1_skproject_cut_record_from(
    DM2_V1_DungeonData *d,
    uint16_t record_to_cut,
    uint16_t *parent_link,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectCutRecordReceipt *out) {
    DM2_V1_SkprojectCutRecordReceipt receipt;
    uint16_t masked_record;
    uint8_t *cut_record;
    uint16_t source_next;
    int max_steps = 0;

    if (out) memset(out, 0, sizeof(*out));
    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.level = level;
    receipt.x = x;
    receipt.y = y;
    receipt.cut_object_id = record_to_cut;
    receipt.source_symbol = "DM2_CUT_RECORD_FROM";
    receipt.source_line = 121;

    if (record_to_cut == DM2_THING_END_MARKER ||
        record_to_cut == DM2_THING_NULL_MARKER) {
        receipt.blocked_null_or_end_cut = 1;
        *out = receipt;
        return 0;
    }

    masked_record = (uint16_t)(record_to_cut & 0x3fffu);
    receipt.masked_object_id = masked_record;
    cut_record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
        d, masked_record, &receipt.cut_type, &receipt.cut_index,
        &receipt.cut_record_size);
    if (!cut_record || receipt.cut_record_size < 2) {
        receipt.blocked_missing_cut_record = 1;
        *out = receipt;
        return 0;
    }
    source_next = RD16(cut_record);
    receipt.source_previous_next = source_next;
    for (int type = 0; d && type < DM2_THING_TYPE_COUNT; ++type)
        max_steps += d->thing_type_counts[type] +
                     d->g1_extension_record_counts[type];
    if (max_steps <= 0) max_steps = 1;

    if (x < 0) {
        uint16_t current;
        int steps = 0;

        if (!d || !parent_link) {
            receipt.blocked_invalid_parent = 1;
            *out = receipt;
            return 0;
        }
        receipt.parent_link_route = 1;
        receipt.parent_previous_link = *parent_link;
        if (((*parent_link) & 0x3fffu) == masked_record) {
            *parent_link = source_next;
            receipt.parent_new_link = *parent_link;
            wr16le(cut_record, DM2_THING_END_MARKER);
            receipt.source_reset_to_end = 1;
            receipt.valid = 1;
            *out = receipt;
            return 1;
        }
        current = *parent_link;
        while (current != DM2_THING_END_MARKER &&
               ((current & 0x3fffu) != masked_record)) {
            uint8_t *record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
                d, (uint16_t)(current & 0x3fffu), NULL, NULL, NULL);
            uint16_t next;
            if (!record || ++steps > max_steps) {
                receipt.blocked_unbounded_graph = 1;
                *out = receipt;
                return 0;
            }
            next = RD16(record);
            if ((next & 0x3fffu) == masked_record) {
                wr16le(record, source_next);
                wr16le(cut_record, DM2_THING_END_MARKER);
                receipt.preceding_object_id = current;
                receipt.parent_new_link = *parent_link;
                receipt.source_reset_to_end = 1;
                receipt.valid = 1;
                *out = receipt;
                return 1;
            }
            current = next;
        }
        wr16le(cut_record, DM2_THING_END_MARKER);
        receipt.parent_new_link = *parent_link;
        receipt.source_reset_to_end = 1;
        receipt.valid = 1;
        *out = receipt;
        return 1;
    } else {
        int square_offset;
        int object_index = -1;
        int object_index_offset = -1;
        uint16_t root;

        square_offset = dm2_v1_dungeon_byte_square_offset(d, level, x, y);
        if (!d || !d->raw_data || square_offset < 0 ||
            (d->raw_data[square_offset] & 0x10u) == 0u ||
            !dm2_v1_dungeon_compute_object_index_for_square(
                d, level, x, y, &object_index, &object_index_offset)) {
            receipt.blocked_invalid_tile = 1;
            *out = receipt;
            return 0;
        }
        root = RD16(d->raw_data + object_index_offset);
        receipt.object_index = object_index;
        receipt.object_index_offset = object_index_offset;
        receipt.tile_previous_root = root;

        if (source_next == DM2_THING_END_MARKER &&
            (root & 0x3fffu) == masked_record) {
            int last_offset = d->square_first_thing_base +
                              (d->square_first_thing_count - 1) * 2;
            int column_base;
            int total_columns;
            int moved_words;

            if (last_offset < object_index_offset ||
                last_offset + 1 >= d->raw_size) {
                receipt.blocked_invalid_tile = 1;
                *out = receipt;
                return 0;
            }
            moved_words = d->square_first_thing_count - object_index - 1;
            if (moved_words > 0) {
                memmove(d->raw_data + object_index_offset,
                        d->raw_data + object_index_offset + 2,
                        (size_t)moved_words * 2u);
            }
            wr16le(d->raw_data + last_offset, DM2_THING_NULL_MARKER);
            d->raw_data[square_offset] &= 0xefu;

            column_base = dm2_v1_dungeon_column_base_index(d, level);
            total_columns = dm2_v1_dungeon_total_columns(d);
            for (int col = column_base + x + 1; col < total_columns; ++col) {
                int offset = d->column_index_base + col * 2;
                uint16_t value;
                if (offset < 0 || offset + 1 >= d->raw_size) {
                    receipt.blocked_invalid_tile = 1;
                    *out = receipt;
                    return 0;
                }
                value = (uint16_t)(RD16(d->raw_data + offset) - 1u);
                wr16le(d->raw_data + offset, value);
                receipt.decremented_column_offsets++;
            }
            wr16le(cut_record, DM2_THING_END_MARKER);
            receipt.shifted_ground_stack_words = moved_words;
            receipt.tile_single_root_remove_route = 1;
            receipt.tile_new_root = DM2_THING_NULL_MARKER;
            receipt.source_reset_to_end = 1;
            receipt.valid = 1;
            *out = receipt;
            return 1;
        }

        if ((root & 0x3fffu) == masked_record) {
            wr16le(d->raw_data + object_index_offset, source_next);
            wr16le(cut_record, DM2_THING_END_MARKER);
            receipt.tile_root_replace_route = 1;
            receipt.tile_new_root = source_next;
            receipt.source_reset_to_end = 1;
            receipt.valid = 1;
            *out = receipt;
            return 1;
        }

        {
            uint16_t current = root;
            int steps = 0;

            while (current != DM2_THING_END_MARKER &&
                   ((current & 0x3fffu) != masked_record)) {
                uint8_t *record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
                    d, (uint16_t)(current & 0x3fffu), NULL, NULL, NULL);
                uint16_t next;
                if (!record || ++steps > max_steps) {
                    receipt.blocked_unbounded_graph = 1;
                    *out = receipt;
                    return 0;
                }
                next = RD16(record);
                if ((next & 0x3fffu) == masked_record) {
                    wr16le(record, source_next);
                    wr16le(cut_record, DM2_THING_END_MARKER);
                    receipt.tile_chain_unlink_route = 1;
                    receipt.preceding_object_id = current;
                    receipt.tile_new_root = root;
                    receipt.source_reset_to_end = 1;
                    receipt.valid = 1;
                    *out = receipt;
                    return 1;
                }
                current = next;
            }
        }
    }

    wr16le(cut_record, DM2_THING_END_MARKER);
    receipt.source_reset_to_end = 1;
    receipt.valid = 1;
    *out = receipt;
    return 1;
}

int dm2_v1_skproject_3d93b_text_scan(
    DM2_V1_DungeonData *d,
    int mode,
    int countdown,
    int target_low_byte,
    int *out_y,
    int *out_x,
    DM2_V1_Skproject3D93BReceipt *out)
{
    DM2_V1_Skproject3D93BReceipt receipt;
    int found_map = -1;
    int found_x = 0;
    int found_y = 0;
    int count = 0;

    if (out_y) *out_y = 0;
    if (out_x) *out_x = 0;
    if (!out) return mode == 5 ? -1 : 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.mode = mode;
    receipt.target_low_byte = target_low_byte & 0xff;
    receipt.countdown_start = countdown;
    receipt.countdown_remaining = countdown;
    receipt.result_map = -1;
    receipt.source_symbol = "DM2_3D93B";
    receipt.source_line = 2108;

    if (!dm2_v1_dungeon_record_list_traversal_allowed(d) || !d->raw_data) {
        receipt.blocked_incomplete_record_graph = 1;
        receipt.return_value = mode == 5 ? -1 : 0;
        *out = receipt;
        return receipt.return_value;
    }

    for (int map = 0; map < d->level_count; ++map) {
        int width = d->level_widths[map];
        int height = d->level_heights[map];

        if (width <= 0 || height <= 0) {
            receipt.blocked_missing_record = 1;
            receipt.return_value = mode == 5 ? -1 : 0;
            *out = receipt;
            return receipt.return_value;
        }
        ++receipt.maps_scanned;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
                int thing;

                if (raw < 0) {
                    receipt.blocked_missing_record = 1;
                    receipt.return_value = mode == 5 ? -1 : 0;
                    *out = receipt;
                    return receipt.return_value;
                }
                ++receipt.squares_scanned;
                if ((raw & 0x10) == 0) continue;
                ++receipt.root_tiles_scanned;
                thing = dm2_v1_dungeon_get_first_thing(d, map, x, y);
                if (thing < 0) {
                    receipt.blocked_missing_record = 1;
                    receipt.return_value = mode == 5 ? -1 : 0;
                    *out = receipt;
                    return receipt.return_value;
                }
                while (thing != (int)DM2_THING_END_MARKER) {
                    int type = -1;
                    int size = 0;
                    int next;
                    uint16_t next_type;
                    const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                        d, (uint16_t)thing, &type, NULL, &size);

                    if (!record || size < 2) {
                        receipt.blocked_missing_record = 1;
                        receipt.return_value = mode == 5 ? -1 : 0;
                        *out = receipt;
                        return receipt.return_value;
                    }

                    if (mode >= 2 && mode <= 5 && type == 2 && size >= 4) {
                        uint16_t w2 = RD16(record + 2);
                        if ((w2 & 0x0006u) == 0x0002u) {
                            uint16_t shifted = (uint16_t)(w2 >> 3);
                            int ext_usage = (int)((shifted >> 8) & 0x1fu);
                            int low = (int)(shifted & 0xffu);
                            ++receipt.text_records_scanned;

                            if (ext_usage < 0x0f) {
                                if (ext_usage == 0x0b &&
                                    low == receipt.target_low_byte) {
                                    ++receipt.matched_ext_usage_0b;
                                    if (mode == 3) {
                                        ++count;
                                    } else {
                                        --receipt.countdown_remaining;
                                        if (receipt.countdown_remaining == 0) {
                                            if (out_y) *out_y = y;
                                            if (out_x) *out_x = x;
                                            receipt.result_map = map;
                                            receipt.result_y = y;
                                            receipt.result_x = x;
                                            receipt.return_value = map;
                                            receipt.valid = 1;
                                            *out = receipt;
                                            return map;
                                        }
                                    }
                                }
                            } else if (ext_usage <= 0x0f) {
                                if (mode == 4) {
                                    uint8_t *mutable_record = (uint8_t *)record;
                                    uint16_t cleared = (uint16_t)(w2 & ~1u);
                                    mutable_record[2] =
                                        (uint8_t)(cleared & 0xffu);
                                    mutable_record[3] = (uint8_t)(cleared >> 8);
                                    ++receipt.cleared_ext_usage_0f_visibility;
                                } else if (mode == 5 && (w2 & 1u) != 0u) {
                                    if (out_y) *out_y = y;
                                    if (out_x) *out_x = x;
                                    receipt.result_map = map;
                                    receipt.result_y = y;
                                    receipt.result_x = x;
                                    receipt.return_value = map;
                                    receipt.valid = 1;
                                    *out = receipt;
                                    return map;
                                }
                            } else if (ext_usage == 0x10 &&
                                       low == receipt.target_low_byte) {
                                ++receipt.matched_ext_usage_10;
                                found_map = map;
                                found_x = x;
                                found_y = y;
                            }
                        }
                    }

                    next = dm2_v1_dungeon_get_next_thing(d, (uint16_t)thing);
                    ++receipt.link_word_reads;
                    if (next < 0) {
                        receipt.blocked_missing_record = 1;
                        receipt.return_value = mode == 5 ? -1 : 0;
                        *out = receipt;
                        return receipt.return_value;
                    }
                    if (next == (int)DM2_THING_END_MARKER) break;
                    next_type = (uint16_t)((next >> 10) & 0x0fu);
                    if (next_type > 3u) break;
                    thing = next;
                }
            }
        }
    }

    if (found_map >= 0) {
        if (out_y) *out_y = found_y;
        if (out_x) *out_x = found_x;
    }
    receipt.result_map = found_map;
    receipt.result_x = found_x;
    receipt.result_y = found_y;
    receipt.return_value =
        mode == 4 ? found_map : (mode == 5 ? -1 : count);
    receipt.valid = 1;
    *out = receipt;
    return receipt.return_value;
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
    if (!d || !d->raw_data || thing == DM2_THING_END_MARKER ||
        thing == DM2_THING_NULL_MARKER)
        return NULL;
    type = (int)((thing >> 10) & 0x0fu);
    index = (int)(thing & 0x03ffu);
    if (type < 0 || type >= DM2_THING_TYPE_COUNT) return NULL;
    size = (int)s_dm2_db_record_size[type];
    if (size <= 0 || index < 0) return NULL;
    if (index < d->thing_type_counts[type] &&
        d->thing_data_bases[type] >= 0) {
        offset = d->thing_data_bases[type] + index * size;
        if (offset >= 0 && offset + size <= d->raw_size)
            goto found;
    }
    if (d->square_bytes == 1 &&
        dm2_v1_g1_extension_record_offset(d, type, index, &offset)) {
        goto found;
    }
    return NULL;
found:
    if (out_type) *out_type = type;
    if (out_index) *out_index = index;
    if (out_size) *out_size = size;
    return d->raw_data + offset;
}

int dm2_v1_dungeon_get_next_thing(const DM2_V1_DungeonData *d,
                                  uint16_t thing) {
    const uint8_t *record;
    int size = 0;

    /* G1 byte-square format: w0 in the file is game data, not a next-link.
     * Each ground-stack entry is a standalone record -- no w0 chains. */
    if (d && d->g1_w0_chains_disabled)
        return (int)DM2_THING_END_MARKER;
    record = dm2_v1_dungeon_get_thing_record(d, thing, NULL, NULL, &size);
    if (!record || size < 2) return -1;
    {
        uint16_t w0 = RD16(record);
        if (w0 == DM2_THING_NULL_MARKER)
            return -1;
        return (int)w0;
    }
}

int dm2_v1_dungeon_walk_square_things(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    int max_steps,
    DM2_V1_DungeonThingVisitor visitor,
    void *user) {
    int thing;
    int steps = 0;

    /* skproject c_map.cpp GET_TILE_RECORD_LINK owns the square root and
     * c_record.cpp/GET_NEXT_RECORD_LINK owns GenericRecord::w0 traversal.
     * Keep callers on this bounded loader route so incomplete PC G1 graphs
     * fail closed instead of letting consumers read w0 locally. */
    if (!d || !visitor || max_steps <= 0) return -1;
    thing = dm2_v1_dungeon_get_first_thing(d, level, x, y);
    if (thing < 0) return 0;

    while (thing != (int)DM2_THING_END_MARKER) {
        int type = -1;
        int index = -1;
        int record_size = 0;
        int next;
        const uint8_t *record;

        if (++steps > max_steps) return -1;
        record = dm2_v1_dungeon_get_thing_record(
            d, (uint16_t)thing, &type, &index, &record_size);
        if (!record || record_size < 2) return -1;
        if (visitor(user, (uint16_t)thing, type, index, record,
                    record_size, level, x, y) != 0) {
            return -1;
        }
        next = dm2_v1_dungeon_get_next_thing(d, (uint16_t)thing);
        if (next < 0) return -1;
        thing = next;
    }
    return steps;
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

    if (!d || link == DM2_THING_END_MARKER ||
        link == DM2_THING_NULL_MARKER) {
        return 0;
    }
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
    uint16_t root,
    int x,
    int y,
    DM2_V1_G1DirectTeleporterRoot *out) {
    const uint8_t *record;
    uint16_t w2;
    uint16_t w4;

    if (!out || ((root >> 10) & 0x0fu) != 1u ||
        !dm2_v1_g1_link_has_declared_shape(d, root)) {
        return 0;
    }
    record = dm2_v1_dungeon_get_thing_record(d, root, NULL, NULL, NULL);
    if (!record) return 0;
    w2 = RD16(record + 2);
    w4 = RD16(record + 4);
    memset(out, 0, sizeof(*out));
    out->x = x;
    out->y = y;
    out->object_id = root;
    out->index = root & 0x03ffu;
    out->direction = (uint8_t)(root >> 14);
    out->destination_x = (uint8_t)(w2 & 0x001fu);
    out->destination_y = (uint8_t)((w2 >> 5) & 0x001fu);
    out->destination_map = (uint8_t)(w4 >> 8);
    out->scope = (uint8_t)(w4 & 0x000fu);
    out->sound = (uint8_t)((w4 >> 4) & 1u);
    out->rotation = (uint8_t)((w4 >> 5) & 3u);
    out->rotation_type = (uint8_t)((w4 >> 7) & 1u);
    return 1;
}

static uint32_t dm2_arrange_hash_step(uint32_t hash, uint32_t value) {
    hash ^= value;
    return hash * 16777619u;
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
        total_records += d->thing_type_counts[type];
    }
    if (total_records <= 0) return 0;

    /* G1 byte-square format: w0 in the file is game data, not a next-link.
     * The runtime (READ_DUNGEON_STRUCTURE) builds w0 chains at load time.
     * Validate that ground-stack entries resolve to valid records;
     * unresolvable entries on specific maps are blocked roots, not errors. */
    if (d->square_bytes == 1) {
        for (level = 0; level < d->level_count; ++level) {
            int x;
            for (x = 0; x < d->level_widths[level]; ++x) {
                int y;
                for (y = 0; y < d->level_heights[level]; ++y) {
                    int raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
                    int thing;
                    if (raw < 0) return 0;
                    if ((raw & 0x10) == 0) continue;
                    thing = dm2_v1_dungeon_get_first_thing(d, level, x, y);
                    if (thing < 0) return 0;
                    if (thing == (int)DM2_THING_END_MARKER) continue;
                    if (!dm2_v1_dungeon_get_thing_record(
                            d, (uint16_t)thing, NULL, NULL, NULL)) {
                        continue;
                    }
                }
            }
        }
        return 1;
    }

    /* Full format: w0 IS the next-link; walk complete tile chains. */
    for (level = 0; level < d->level_count; ++level) {
        int x;
        for (x = 0; x < d->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < d->level_heights[level]; ++y) {
                int raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
                int thing;
                int steps = 0;
                if (raw < 0) return 0;
                if ((raw & 0x10) == 0) continue;
                thing = dm2_v1_dungeon_get_first_thing(d, level, x, y);
                while (thing != (int)DM2_THING_END_MARKER) {
                    int next;
                    if (thing < 0 || ++steps > total_records ||
                        !dm2_v1_dungeon_get_thing_record(
                            d, (uint16_t)thing, NULL, NULL, NULL)) {
                        return 0;
                    }
                    next = dm2_v1_dungeon_get_next_thing(d, (uint16_t)thing);
                    if (next < 0) return 0;
                    thing = next;
                }
            }
        }
    }
    return 1;
}

int dm2_v1_DM2_ARRANGE_DUNGEON_receipt(
    const uint8_t *dat,
    int size,
    DM2_V1_ArrangeDungeonReceipt *out) {
    DM2_V1_DungeonData dungeon;
    uint32_t dimension_hash = 2166136261u;
    uint32_t style_hash = 2166136261u;
    uint32_t arrangement_hash = 2166136261u;
    int outdoor_count = 0;
    int indoor_count = 0;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    memset(&dungeon, 0, sizeof(dungeon));
    if (!dat || size <= 0 ||
        dm2_v1_dungeon_load(&dungeon, dat, size) != 0 ||
        dungeon.level_count <= 0 ||
        dungeon.level_count > DM2_V1_MAX_LEVELS ||
        !dungeon.raw_data ||
        dungeon.raw_size != size ||
        dungeon.square_bytes <= 0 ||
        dungeon.raw_map_data_base < 0) {
        dm2_v1_dungeon_free(&dungeon);
        return 0;
    }

    for (int level = 0; level < dungeon.level_count; ++level) {
        int w = dungeon.level_widths[level];
        int h = dungeon.level_heights[level];
        int style = dm2_v1_dungeon_get_map_graphics_style(&dungeon, level);
        int first_tile;
        int last_tile;

        if (w <= 0 || w > DM2_V1_MAX_MAP_SIZE ||
            h <= 0 || h > DM2_V1_MAX_MAP_SIZE ||
            dungeon.level_offsets[level] < 0 ||
            style < 0 || style > 15) {
            dm2_v1_dungeon_free(&dungeon);
            return 0;
        }
        first_tile = dm2_v1_dungeon_get_tile_raw(&dungeon, level, 0, 0);
        last_tile = dm2_v1_dungeon_get_tile_raw(&dungeon, level, w - 1, h - 1);
        if (first_tile < 0 || last_tile < 0) {
            dm2_v1_dungeon_free(&dungeon);
            return 0;
        }

        if (dm2_v1_dungeon_is_outdoor(&dungeon, level))
            ++outdoor_count;
        else
            ++indoor_count;

        dimension_hash = dm2_arrange_hash_step(dimension_hash, (uint32_t)level);
        dimension_hash = dm2_arrange_hash_step(dimension_hash, (uint32_t)w);
        dimension_hash = dm2_arrange_hash_step(dimension_hash, (uint32_t)h);
        dimension_hash = dm2_arrange_hash_step(
            dimension_hash, (uint32_t)dungeon.level_offsets[level]);
        dimension_hash = dm2_arrange_hash_step(
            dimension_hash, (uint32_t)(first_tile & 0xffff));
        dimension_hash = dm2_arrange_hash_step(
            dimension_hash, (uint32_t)(last_tile & 0xffff));
        style_hash = dm2_arrange_hash_step(style_hash, (uint32_t)level);
        style_hash = dm2_arrange_hash_step(style_hash, (uint32_t)style);
    }

    arrangement_hash = dm2_arrange_hash_step(arrangement_hash,
                                             (uint32_t)dungeon.level_count);
    arrangement_hash = dm2_arrange_hash_step(arrangement_hash,
                                             (uint32_t)dungeon.square_bytes);
    arrangement_hash = dm2_arrange_hash_step(
        arrangement_hash, (uint32_t)dungeon.raw_map_data_base);
    arrangement_hash = dm2_arrange_hash_step(
        arrangement_hash, (uint32_t)(dungeon.column_index_base < 0 ? 0xffffu :
                                     (uint32_t)dungeon.column_index_base));
    arrangement_hash = dm2_arrange_hash_step(
        arrangement_hash, (uint32_t)(dungeon.square_first_thing_count < 0 ? 0u :
                                     (unsigned int)dungeon.square_first_thing_count));
    arrangement_hash = dm2_arrange_hash_step(
        arrangement_hash, (uint32_t)(dungeon.text_word_count < 0 ? 0u :
                                     (unsigned int)dungeon.text_word_count));
    arrangement_hash = dm2_arrange_hash_step(
        arrangement_hash, (uint32_t)(dungeon.record_graph_complete != 0));
    arrangement_hash = dm2_arrange_hash_step(arrangement_hash,
                                             dimension_hash);
    arrangement_hash = dm2_arrange_hash_step(arrangement_hash, style_hash);
    if (dimension_hash == 0u || style_hash == 0u ||
        arrangement_hash == 0u || outdoor_count <= 0) {
        dm2_v1_dungeon_free(&dungeon);
        return 0;
    }

    out->valid = 1;
    out->committed = 1;
    out->incomplete = dungeon.record_graph_complete ? 0 : 1;
    out->map_count = dungeon.level_count;
    out->outdoor_map_count = outdoor_count;
    out->indoor_map_count = indoor_count;
    out->square_bytes = dungeon.square_bytes;
    out->raw_map_data_base = dungeon.raw_map_data_base;
    out->column_index_base = dungeon.column_index_base;
    out->ground_stack_base = dungeon.square_first_thing_base;
    out->ground_stack_count = dungeon.square_first_thing_count;
    out->text_data_base = dungeon.text_data_base;
    out->text_word_count = dungeon.text_word_count;
    out->candidate_pool_base = dungeon.partial_map_boot.candidate_pool_base;
    out->candidate_pool_end = dungeon.partial_map_boot.candidate_pool_end;
    out->g1_extension_base = dungeon.g1_extension_base;
    out->g1_extension_size = dungeon.g1_extension_size;
    out->record_graph_complete = dungeon.record_graph_complete;
    out->map_dimension_hash = dimension_hash;
    out->map_graphics_style_hash = style_hash;
    out->arrangement_hash = arrangement_hash;
    dm2_v1_dungeon_free(&dungeon);
    return 1;
}

int dm2_v1_dungeon_collect_g1_ground_stack_map_corpus_receipt(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1GroundStackMapCorpusReceipt *out) {
    int column_bytes;
    int ground_bytes;
    int map_bytes;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!d || !d->raw_data || d->square_bytes != 1 ||
        d->column_index_base < 0 || d->square_first_thing_base < 0 ||
        d->raw_map_data_base < 0 || d->raw_map_data_base > d->raw_size) {
        out->g1_layout_absent = 1;
        return 1;
    }
    column_bytes = d->square_first_thing_base - d->column_index_base;
    ground_bytes = d->square_first_thing_count * 2;
    map_bytes = d->raw_size - d->raw_map_data_base;
    if (column_bytes <= 0 || (column_bytes & 1) != 0 ||
        ground_bytes <= 0 || d->square_first_thing_base + ground_bytes >
            d->raw_size || map_bytes <= 0) {
        out->g1_layout_absent = 1;
        return 1;
    }
    out->available = 1;
    out->raw_only = 1;
    out->column_index_base = d->column_index_base;
    out->column_index_word_count = column_bytes / 2;
    out->column_index_byte_count = (uint32_t)column_bytes;
    out->column_index_hash = dm2_v1_g1_receipt_hash(
        d->raw_data + d->column_index_base, (uint32_t)column_bytes);
    out->ground_stack_base = d->square_first_thing_base;
    out->ground_stack_word_count = d->square_first_thing_count;
    out->ground_stack_byte_count = (uint32_t)ground_bytes;
    out->ground_stack_hash = dm2_v1_g1_receipt_hash(
        d->raw_data + d->square_first_thing_base, (uint32_t)ground_bytes);
    out->map_data_base = d->raw_map_data_base;
    out->map_data_byte_count = (uint32_t)map_bytes;
    out->map_data_hash = dm2_v1_g1_receipt_hash(
        d->raw_data + d->raw_map_data_base, (uint32_t)map_bytes);
    out->column_index_semantics_unresolved = 1;
    out->ground_stack_semantics_unresolved = 1;
    return 1;
}

int dm2_v1_dungeon_collect_g1_map_corpus_receipt(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1MapCorpusReceipt *out) {
    int map_bytes;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!d || !d->raw_data || d->square_bytes != 1 ||
        d->raw_map_data_base < 0 || d->raw_map_data_base > d->raw_size ||
        d->level_count <= 0 || d->level_count > DM2_V1_MAX_LEVELS) {
        out->g1_layout_absent = 1;
        return 1;
    }
    map_bytes = d->raw_size - d->raw_map_data_base;
    if (map_bytes <= 0) {
        out->g1_layout_absent = 1;
        return 1;
    }
    out->available = 1;
    out->raw_only = 1;
    out->tile_semantics_unresolved = 1;
    out->map_count = d->level_count;
    out->map_data_base = d->raw_map_data_base;
    out->map_data_byte_count = (uint32_t)map_bytes;
    out->map_data_hash = dm2_v1_g1_receipt_hash(
        d->raw_data + d->raw_map_data_base, (uint32_t)map_bytes);
    for (int map = 0; map < d->level_count; ++map) {
        DM2_V1_G1MapCorpusEntry *entry = &out->maps[map];
        int descriptor_base = DM2_DUNGEON_HEADER_SIZE + map * DM2_MAP_DESC_SIZE;
        int width = d->level_widths[map];
        int height = d->level_heights[map];
        int offset = d->level_offsets[map];
        int bytes = width * height;

        if (descriptor_base < 0 ||
            descriptor_base + DM2_MAP_DESC_SIZE > d->raw_size ||
            width <= 0 || height <= 0 || offset < 0 || bytes <= 0 ||
            offset + bytes > map_bytes) {
            memset(out, 0, sizeof(*out));
            out->g1_layout_absent = 1;
            return 1;
        }
        entry->map = map;
        entry->descriptor_base = descriptor_base;
        entry->map_data_offset = offset;
        entry->width = width;
        entry->height = height;
        entry->map_byte_count = (uint32_t)bytes;
        entry->descriptor_hash = dm2_v1_g1_receipt_hash(
            d->raw_data + descriptor_base, DM2_MAP_DESC_SIZE);
        entry->map_hash = dm2_v1_g1_receipt_hash(
            d->raw_data + d->raw_map_data_base + offset, (uint32_t)bytes);
    }
    return 1;
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
        else if (dm2_v1_g1_link_has_extension_shape(d, link))
            ++out->map_root_extension_shape_valid;
        else
            ++out->root_shape_invalid;
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
            else if (dm2_v1_g1_link_has_extension_shape(d, link))
                ++out->candidate_first_link_extension_shape_valid;
            else
                ++out->candidate_first_link_shape_invalid;
        }
    }

    out->available = 1;
    return 1;
}

int dm2_v1_dungeon_validate_record_pools(const DM2_V1_DungeonData *d) {
    DM2_V1_G1RecordPoolEvidence evidence;

    if (!d || !dm2_v1_dungeon_collect_g1_record_pool_evidence(d, &evidence))
        return 0;
    if (!evidence.available || evidence.candidate_bytes <= 0 ||
        evidence.candidate_base != d->partial_map_boot.candidate_pool_base ||
        evidence.candidate_end != d->partial_map_boot.candidate_pool_end ||
        evidence.candidate_end != d->g1_extension_base ||
        evidence.text_end != evidence.candidate_base) {
        return 0;
    }
    return 1;
}

int dm2_v1_dungeon_materialize_g1_partial_map_boot(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1PartialMapBootReceipt *out) {
    DM2_V1_G1PartialMapBootReceipt candidate;
    int column_index = 0;

    if (!out || !dm2_v1_dungeon_validate_record_pools(d)) return 0;
    memset(&candidate, 0, sizeof(candidate));
    candidate.valid = 1;
    candidate.level_count = d->level_count;
    candidate.map_count = d->level_count;
    candidate.square_bytes = d->square_bytes;
    candidate.column_index_base = d->column_index_base;
    candidate.ground_stack_base = d->square_first_thing_base;
    candidate.ground_stack_count = d->square_first_thing_count;
    candidate.text_data_base = d->text_data_base;
    candidate.text_word_count = d->text_word_count;
    candidate.candidate_pool_base = d->partial_map_boot.candidate_pool_base;
    candidate.candidate_pool_end = d->partial_map_boot.candidate_pool_end;
    candidate.g1_extension_base = d->g1_extension_base;
    candidate.g1_extension_size = d->g1_extension_size;
    candidate.raw_map_data_base = d->raw_map_data_base;
    candidate.record_graph_complete = d->record_graph_complete;

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
                    ++candidate.direct_root_count_by_type[type];
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
                    ++candidate.blocked_root_count_by_type[type];
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

int dm2_v1_dungeon_validate_g1_runtime_map(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapValidationReceipt *out) {
    DM2_V1_G1PartialMapBootReceipt partial;
    DM2_V1_G1MapCorpusReceipt corpus;
    DM2_V1_G1RuntimeMapValidationReceipt candidate;
    int column_index = 0;

    /* skproject SKULLWIN/c_map.cpp DM2_GET_OBJECT_INDEX_FROM_TILE and
     * DM2_GET_TILE_RECORD_LINK select only the map's column and
     * dunGroundStacks root. c_record.cpp DM2_GET_NEXT_RECORD_LINK is a later
     * operation and is deliberately absent here. */
    if (!out || !d || !d->raw_data || map < 0 || map >= d->level_count ||
        !dm2_v1_dungeon_validate_record_pools(d) ||
        !dm2_v1_dungeon_materialize_g1_partial_map_boot(d, &partial) ||
        !partial.committed || !partial.incomplete ||
        !dm2_v1_dungeon_collect_g1_map_corpus_receipt(d, &corpus) ||
        !corpus.available || corpus.g1_layout_absent) {
        return 0;
    }
    for (int level = 0; level < map; ++level)
        column_index += d->level_widths[level];
    if (column_index < 0 ||
        column_index + d->level_widths[map] >
            (d->square_first_thing_base - d->column_index_base) / 2) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.map = map;
    candidate.width = d->level_widths[map];
    candidate.height = d->level_heights[map];
    candidate.map_data_base = d->raw_map_data_base;
    candidate.map_data_offset = d->level_offsets[map];
    candidate.map_data_byte_count = corpus.maps[map].map_byte_count;
    candidate.map_data_hash = corpus.maps[map].map_hash;
    if (candidate.width <= 0 || candidate.height <= 0 ||
        candidate.map_data_byte_count == 0u || candidate.map_data_hash == 0u) {
        return 0;
    }

    for (int x = 0; x < candidate.width; ++x) {
        int stack = (int)RD16(d->raw_data + d->column_index_base +
                              (column_index + x) * 2);
        for (int y = 0; y < candidate.height; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            uint16_t root;
            int type;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            type = (root >> 10) & 0x0f;
            ++candidate.root_count;
            if (dm2_v1_g1_link_has_declared_shape(d, root)) {
                ++candidate.direct_root_count;
            } else if (dm2_v1_g1_link_has_extension_shape(d, root)) {
                if (type == 3) ++candidate.db3_root_count;
                else if (type == 4) ++candidate.db4_root_count;
                else return 0;
            } else if (type == 8 || type == 10) {
                ++candidate.blocked_root_count;
            } else {
                return 0;
            }
            ++stack;
        }
    }

    if (candidate.root_count <= 0 ||
        candidate.root_count != candidate.direct_root_count +
            candidate.db3_root_count + candidate.db4_root_count +
            candidate.blocked_root_count ||
        candidate.blocked_root_count != partial.blocked_root_count_by_map[map]) {
        return 0;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_resolve_g1_direct_root_record(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_G1DirectRootRecordAddressReceipt *out) {
    DM2_V1_G1RuntimeMapValidationReceipt validation;
    DM2_V1_G1DirectRootRecordAddressReceipt candidate;
    uint16_t root;
    int type;
    int index;
    int record_size;
    int record_offset;

    /* skproject SKULLWIN/c_map.cpp selects dunGroundStacks for a tile, then
     * c_record.cpp computes recordptr[type] + size[type] * index. Stop at
     * that address: GET_NEXT_RECORD_LINK's w0 read is not part of this path. */
    if (!out || !d || !d->raw_data ||
        !dm2_v1_dungeon_validate_g1_runtime_map(d, level, &validation) ||
        !validation.committed || !validation.incomplete_world ||
        x < 0 || x >= validation.width || y < 0 || y >= validation.height) {
        return 0;
    }
    if (dm2_v1_dungeon_get_tile_raw(d, level, x, y) < 0) return 0;
    root = (uint16_t)dm2_v1_dungeon_get_first_thing(d, level, x, y);
    if (root == DM2_THING_END_MARKER || root == DM2_THING_NULL_MARKER ||
        !dm2_v1_g1_link_has_declared_shape(d, root)) {
        return 0;
    }
    type = (root >> 10) & 0x0f;
    index = root & 0x03ff;
    if (!((type >= 0 && type <= 5) || type == 9) ||
        index < 0 || index >= d->thing_type_counts[type] ||
        d->thing_data_bases[type] < 0) {
        return 0;
    }
    record_size = (int)s_dm2_db_record_size[type];
    record_offset = d->thing_data_bases[type] + index * record_size;
    if (record_size <= 0 || record_offset < d->thing_data_bases[type] ||
        record_offset + record_size > d->g1_extension_base ||
        record_offset + record_size > d->raw_size) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.level = level;
    candidate.x = x;
    candidate.y = y;
    candidate.object_id = root;
    candidate.type = (uint8_t)type;
    candidate.index = (uint16_t)index;
    candidate.record_offset = record_offset;
    candidate.record_size = record_size;
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_collect_g1_direct_root_chain(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_G1DirectRootChainReceipt *out) {
    DM2_V1_G1DirectRootRecordAddressReceipt address;
    DM2_V1_G1DirectRootChainReceipt candidate;
    uint16_t thing;

    if (!out || !dm2_v1_dungeon_resolve_g1_direct_root_record(
                    d, level, x, y, &address)) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.level = level;
    candidate.x = x;
    candidate.y = y;
    thing = address.object_id;

    /* skproject c_record.cpp GET_NEXT_RECORD_LINK is the only record-word
     * read here. Do not decode node payloads or widen the direct DB set. */
    for (;;) {
        const uint8_t *record;
        uint16_t next;
        int type;
        int index;
        int record_size;
        int record_offset;
        DM2_V1_G1DirectChainNode *node;

        if (thing == DM2_THING_END_MARKER || thing == DM2_THING_NULL_MARKER ||
            !dm2_v1_g1_link_has_declared_shape(d, thing) ||
            candidate.node_count >= DM2_V1_G1_DIRECT_CHAIN_MAX) {
            return 0;
        }
        type = (thing >> 10) & 0x0f;
        index = thing & 0x03ff;
        if (!((type >= 0 && type <= 5) || type == 9) ||
            index < 0 || index >= d->thing_type_counts[type] ||
            d->thing_data_bases[type] < 0) {
            return 0;
        }
        record_size = (int)s_dm2_db_record_size[type];
        record_offset = d->thing_data_bases[type] + index * record_size;
        if (record_size < 2 || record_offset < d->thing_data_bases[type] ||
            record_offset + record_size > d->g1_extension_base ||
            record_offset + record_size > d->raw_size) {
            return 0;
        }
        for (int i = 0; i < candidate.node_count; ++i) {
            if (candidate.nodes[i].object_id == thing) return 0;
        }
        record = d->raw_data + record_offset;
        node = &candidate.nodes[candidate.node_count++];
        node->object_id = thing;
        node->type = (uint8_t)type;
        node->index = (uint16_t)index;
        node->record_offset = record_offset;
        node->record_size = record_size;
        next = RD16(record);
        ++candidate.link_word_reads;
        if (next == DM2_THING_END_MARKER) break;
        thing = next;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_classify_g1_direct_root_scene(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_G1DungeonSceneClassificationReceipt *out) {
    DM2_V1_G1DungeonSceneClassificationReceipt candidate;
    DM2_V1_G1DirectRootChainReceipt chain;
    int raw_tile;

    if (!out || !d ||
        !dm2_v1_dungeon_collect_g1_direct_root_chain(
            d, level, x, y, &chain)) {
        return 0;
    }
    raw_tile = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (raw_tile < 0 || chain.node_count <= 0) return 0;

    memset(&candidate, 0, sizeof(candidate));
    candidate.chain = chain;
    candidate.incomplete_world = 1;
    candidate.level = level;
    candidate.x = x;
    candidate.y = y;
    candidate.raw_tile = (uint8_t)raw_tile;

    /* skproject c_map.cpp obtains the byte square before c_record.cpp gets
     * its root. DME.h::tileTypeIndex makes only these three scene classes
     * explicit; no substitute is assigned to pit/stair/teleporter classes. */
    switch ((candidate.raw_tile >> 5) & 0x07u) {
    case 0:
        candidate.tile_class = DM2_V1_G1_SCENE_TILE_WALL;
        break;
    case 1:
        candidate.tile_class = DM2_V1_G1_SCENE_TILE_FLOOR;
        break;
    case 4:
        candidate.tile_class = DM2_V1_G1_SCENE_TILE_DOOR;
        break;
    default:
        return 0;
    }
    /* DME.h::dbIndex calls DB0 Door and DB4 Creature. This maps only the
     * root ObjectID type; it never consumes a record payload or a later node. */
    switch (candidate.chain.nodes[0].type) {
    case 0:
        candidate.root_class = DM2_V1_G1_SCENE_ROOT_DOOR;
        break;
    case 4:
        candidate.root_class = DM2_V1_G1_SCENE_ROOT_CREATURE;
        break;
    default:
        candidate.root_class = DM2_V1_G1_SCENE_ROOT_GENERIC;
        break;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_runtime_map_doors(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapDoorReceipt *out) {
    DM2_V1_G1RuntimeMapValidationReceipt validation;
    DM2_V1_G1RuntimeMapDoorReceipt candidate;
    int column_index = 0;

    /* skproject SKULLWIN/c_map.cpp DM2_GET_TILE_RECORD_LINK selects the
     * root. SKWIN/DME.h::Door fixes the six w2 fields below. c_record.cpp
     * DM2_GET_NEXT_RECORD_LINK reads w0, which this source-bounded route
     * deliberately never calls. */
    if (!out || !d || !d->raw_data ||
        !dm2_v1_dungeon_validate_g1_runtime_map(d, map, &validation) ||
        !validation.committed || !validation.incomplete_world) {
        return 0;
    }
    for (int level = 0; level < map; ++level)
        column_index += d->level_widths[level];

    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.map = map;
    for (int x = 0; x < validation.width; ++x) {
        int stack = (int)RD16(d->raw_data + d->column_index_base +
                              (column_index + x) * 2);
        for (int y = 0; y < validation.height; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            uint16_t root;
            const uint8_t *record;
            uint16_t attributes;
            DM2_V1_G1DirectDoorRoot *door;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            if (((root >> 10) & 0x0fu) == 0u) {
                if (!dm2_v1_g1_link_has_declared_shape(d, root) ||
                    candidate.door_root_count >=
                        DM2_V1_G1_RUNTIME_MAP_MAX_DOOR_ROOTS) {
                    return 0;
                }
                record = dm2_v1_dungeon_get_thing_record(
                    d, root, NULL, NULL, NULL);
                if (!record) return 0;
                attributes = RD16(record + 2);
                door = &candidate.doors[candidate.door_root_count++];
                door->x = x;
                door->y = y;
                door->object_id = root;
                door->index = root & 0x03ff;
                door->direction = (uint8_t)(root >> 14);
                door->button = (uint8_t)((attributes >> 6) & 1u);
                door->door_type = (uint8_t)(attributes & 1u);
                door->button_state = (uint8_t)((attributes >> 11) & 1u);
                door->opening_dir = (uint8_t)((attributes >> 5) & 1u);
                door->ornate_index = (uint8_t)((attributes >> 1) & 0x0fu);
                door->destroyable_by_fireball =
                    (uint8_t)((attributes >> 7) & 1u);
                door->bashable_by_chopping =
                    (uint8_t)((attributes >> 8) & 1u);
                ++candidate.door_record_reads;
            }
            ++stack;
        }
    }
    if (candidate.door_record_reads != candidate.door_root_count ||
        candidate.generic_record_reads != 0 ||
        candidate.blocked_record_reads != 0) {
        return 0;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_g1_runtime_map_door_at(
    const DM2_V1_G1RuntimeMapDoorReceipt *receipt,
    int x,
    int y,
    const DM2_V1_G1DirectDoorRoot **out_door)
{
    if (out_door) *out_door = NULL;
    if (!receipt || !out_door || !receipt->committed ||
        !receipt->incomplete_world || receipt->door_root_count < 0 ||
        receipt->door_root_count > DM2_V1_G1_RUNTIME_MAP_MAX_DOOR_ROOTS) {
        return 0;
    }
    for (int i = 0; i < receipt->door_root_count; ++i) {
        const DM2_V1_G1DirectDoorRoot *door = &receipt->doors[i];
        if (door->x == x && door->y == y) {
            *out_door = door;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_dungeon_materialize_g1_runtime_map_actuators(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapActuatorReceipt *out) {
    DM2_V1_G1RuntimeMapValidationReceipt validation;
    DM2_V1_G1RuntimeMapActuatorReceipt candidate;
    int column_index = 0;

    /* skproject SKULLWIN/c_map.cpp obtains the map root before
     * c_record.cpp's GET_NEXT_RECORD_LINK. SKWIN/DME.h::Actuator defines the
     * w2/w4/w6 field layout read below; this route never reads the w0 word. */
    if (!out || !d || !d->raw_data ||
        !dm2_v1_dungeon_validate_g1_runtime_map(d, map, &validation) ||
        !validation.committed || !validation.incomplete_world) {
        return 0;
    }
    for (int level = 0; level < map; ++level)
        column_index += d->level_widths[level];

    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.map = map;
    for (int x = 0; x < validation.width; ++x) {
        int stack = (int)RD16(d->raw_data + d->column_index_base +
                              (column_index + x) * 2);
        for (int y = 0; y < validation.height; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            uint16_t root;
            const uint8_t *record;
            uint16_t w2;
            uint16_t w4;
            uint16_t w6;
            DM2_V1_G1DirectActuatorRoot *actuator;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            if (((root >> 10) & 0x0fu) == 3u &&
                dm2_v1_g1_link_has_declared_shape(d, root)) {
                if (candidate.actuator_root_count >=
                    DM2_V1_G1_RUNTIME_MAP_MAX_ACTUATOR_ROOTS) {
                    return 0;
                }
                record = dm2_v1_dungeon_get_thing_record(
                    d, root, NULL, NULL, NULL);
                if (!record) return 0;
                w2 = RD16(record + 2);
                w4 = RD16(record + 4);
                w6 = RD16(record + 6);
                actuator = &candidate.actuators[candidate.actuator_root_count++];
                actuator->x = x;
                actuator->y = y;
                actuator->object_id = root;
                actuator->index = root & 0x03ff;
                actuator->direction = (uint8_t)(root >> 14);
                actuator->actuator_type = (uint8_t)(w2 & 0x007fu);
                actuator->actuator_data = (uint16_t)((w2 >> 7) & 0x01ffu);
                actuator->graphic_number = (uint8_t)((w4 >> 12) & 0x000fu);
                actuator->disabled = (uint8_t)((w4 >> 11) & 1u);
                actuator->delay = (uint8_t)((w4 >> 7) & 0x000fu);
                actuator->sound_effect = (uint8_t)((w4 >> 6) & 1u);
                actuator->revert_effect = (uint8_t)((w4 >> 5) & 1u);
                actuator->action_type = (uint8_t)((w4 >> 3) & 3u);
                actuator->once_only = (uint8_t)((w4 >> 2) & 1u);
                actuator->active_status = (uint8_t)(w4 & 1u);
                actuator->target_direction = (uint8_t)((w6 >> 4) & 3u);
                actuator->target_x = (uint8_t)((w6 >> 6) & 0x001fu);
                actuator->target_y = (uint8_t)((w6 >> 11) & 0x001fu);
                ++candidate.actuator_record_reads;
            }
            ++stack;
        }
    }
    if (candidate.actuator_record_reads != candidate.actuator_root_count ||
        candidate.generic_record_reads != 0 ||
        candidate.blocked_record_reads != 0) {
        return 0;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_collect_g1_champion_mirrors(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1ChampionMirrorReceipt *out)
{
    DM2_V1_G1ChampionMirrorReceipt candidate;
    int column_index = 0;

    /* ReDMCSB/SKProject c_hero.cpp DM2_SELECT_CHAMPION:1081-1098 first
     * reaches the tile root through c_map, then accepts DB3 only when
     * Actuator::Type() (w2 & 0x7f) equals 0x7e.  PC G1 has a proven DB3
     * continuation beyond the standard pool; c_record.cpp's address rule
     * resolves both pools.  This walk deliberately never reads w0. */
    if (!out || !d || !d->raw_data || d->square_bytes != 1 ||
        d->level_count <= 0 || d->level_count > DM2_V1_MAX_LEVELS) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    for (int map = 0; map < d->level_count; ++map) {
        DM2_V1_G1RuntimeMapValidationReceipt validation;

        if (!dm2_v1_dungeon_validate_g1_runtime_map(d, map, &validation) ||
            !validation.committed || !validation.incomplete_world) {
            return 0;
        }
        for (int x = 0; x < validation.width; ++x) {
            int stack = (int)RD16(d->raw_data + d->column_index_base +
                                  (column_index + x) * 2);
            for (int y = 0; y < validation.height; ++y) {
                int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
                uint16_t root;
                int type = -1;
                int record_size = 0;
                const uint8_t *record;
                uint16_t w2;
                DM2_V1_G1ChampionMirrorRoot *mirror;

                if (raw < 0) return 0;
                if ((raw & 0x10) == 0) continue;
                if (stack < 0 || stack >= d->square_first_thing_count) return 0;
                root = RD16(d->raw_data + d->square_first_thing_base +
                            stack * 2);
                if (((root >> 10) & 0x0fu) != 3u) {
                    ++stack;
                    continue;
                }
                record = dm2_v1_dungeon_get_thing_record(
                    d, root, &type, NULL, &record_size);
                if (!record || type != 3 || record_size < 8) return 0;
                ++candidate.actuator_record_reads;
                w2 = RD16(record + 2);
                if ((w2 & 0x007fu) == 0x007eu) {
                    if (candidate.mirror_count >=
                        DM2_V1_G1_MAX_CHAMPION_MIRRORS) {
                        return 0;
                    }
                    mirror = &candidate.mirrors[candidate.mirror_count++];
                    mirror->map = map;
                    mirror->x = x;
                    mirror->y = y;
                    mirror->object_id = root;
                    mirror->direction = (uint8_t)(root >> 14);
                    mirror->actuator_data =
                        (uint16_t)((w2 >> 7) & 0x01ffu);
                    /* SKProject c_loadlevel.cpp:604-611 reads the same w2,
                     * keeps RG1Blo, and calls DM2_MARK_DYN_LOAD with
                     * (hero_type << 16) + 0x1600ffff.  The byte is also what
                     * c_hero.cpp::DM2_SELECT_CHAMPION passes to the signed
                     * i8 REVIVE_PLAYER htype parameter.  Retain both forms:
                     * raw 9-bit data is evidence, the dynamic key is the
                     * source-owned handoff required before GDAT queries. */
                    mirror->dynamic_hero_type =
                        (uint8_t)mirror->actuator_data;
                    mirror->dynamic_load_id =
                        ((uint32_t)mirror->dynamic_hero_type << 16) |
                        0x1600ffffu;
                }
                ++stack;
            }
        }
        column_index += validation.width;
    }
    if (candidate.mirror_count <= 0 || candidate.actuator_record_reads <= 0) {
        return 0;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_runtime_map_creatures(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapCreatureReceipt *out) {
    DM2_V1_G1RuntimeMapValidationReceipt validation;
    DM2_V1_G1RuntimeMapCreatureReceipt candidate;
    int column_index = 0;

    /* skproject SKULLWIN/c_map.cpp obtains the root before c_record.cpp's
     * GET_NEXT_RECORD_LINK. SKWIN/DME.h::Creature names b4 CreatureType,
     * w6 HP1, and b15 bits 0..1 as the facing consumed by DRAW_MAP_CHIP;
     * no link or possession ObjectID is read by this receipt. */
    if (!out || !d || !d->raw_data ||
        !dm2_v1_dungeon_validate_g1_runtime_map(d, map, &validation) ||
        !validation.committed || !validation.incomplete_world) {
        return 0;
    }
    for (int level = 0; level < map; ++level)
        column_index += d->level_widths[level];

    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.map = map;
    for (int x = 0; x < validation.width; ++x) {
        int stack = (int)RD16(d->raw_data + d->column_index_base +
                              (column_index + x) * 2);
        for (int y = 0; y < validation.height; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            uint16_t root;
            const uint8_t *record;
            DM2_V1_G1DirectCreatureRoot *creature;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            if (((root >> 10) & 0x0fu) == 4u &&
                dm2_v1_g1_link_has_declared_shape(d, root)) {
                if (candidate.creature_root_count >=
                    DM2_V1_G1_RUNTIME_MAP_MAX_CREATURE_ROOTS) {
                    return 0;
                }
                record = dm2_v1_dungeon_get_thing_record(
                    d, root, NULL, NULL, NULL);
                if (!record) return 0;
                creature = &candidate.creatures[candidate.creature_root_count++];
                creature->x = x;
                creature->y = y;
                creature->object_id = root;
                creature->index = root & 0x03ff;
                creature->direction = (uint8_t)(record[15] & 3u);
                creature->creature_type = record[4];
                creature->hit_points_1 = RD16(record + 6);
                creature->info_slot = record[5];
                creature->cursor_w8 = RD16(record + 8);
                creature->cursor_w10 = RD16(record + 10);
                ++candidate.creature_record_reads;
            }
            ++stack;
        }
    }
    if (candidate.creature_record_reads != candidate.creature_root_count ||
        candidate.generic_record_reads != 0 ||
        candidate.blocked_record_reads != 0) {
        return 0;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_runtime_map_weapons(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapWeaponReceipt *out) {
    DM2_V1_G1RuntimeMapValidationReceipt validation;
    DM2_V1_G1RuntimeMapWeaponReceipt candidate;
    int column_index = 0;

    /* skproject SKULLWIN/c_map.cpp obtains the root before c_record.cpp's
     * GET_NEXT_RECORD_LINK. SKWIN/DME.h::Weapon defines the w2 fields below;
     * this receipt does not read the w0 link. */
    if (!out || !d || !d->raw_data ||
        !dm2_v1_dungeon_validate_g1_runtime_map(d, map, &validation) ||
        !validation.committed || !validation.incomplete_world) {
        return 0;
    }
    for (int level = 0; level < map; ++level)
        column_index += d->level_widths[level];

    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.map = map;
    for (int x = 0; x < validation.width; ++x) {
        int stack = (int)RD16(d->raw_data + d->column_index_base +
                              (column_index + x) * 2);
        for (int y = 0; y < validation.height; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            uint16_t root;
            const uint8_t *record;
            uint16_t attributes;
            DM2_V1_G1DirectWeaponRoot *weapon;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            if (((root >> 10) & 0x0fu) == 5u &&
                dm2_v1_g1_link_has_declared_shape(d, root)) {
                if (candidate.weapon_root_count >=
                    DM2_V1_G1_RUNTIME_MAP_MAX_WEAPON_ROOTS) {
                    return 0;
                }
                record = dm2_v1_dungeon_get_thing_record(
                    d, root, NULL, NULL, NULL);
                if (!record) return 0;
                attributes = RD16(record + 2);
                weapon = &candidate.weapons[candidate.weapon_root_count++];
                weapon->x = x;
                weapon->y = y;
                weapon->object_id = root;
                weapon->index = root & 0x03ff;
                weapon->direction = (uint8_t)(root >> 14);
                weapon->item_type = (uint8_t)(attributes & 0x007fu);
                weapon->important = (uint8_t)((attributes >> 7) & 1u);
                weapon->charges = (uint8_t)((attributes >> 10) & 0x000fu);
                ++candidate.weapon_record_reads;
            }
            ++stack;
        }
    }
    if (candidate.weapon_record_reads != candidate.weapon_root_count ||
        candidate.generic_record_reads != 0 ||
        candidate.blocked_record_reads != 0) {
        return 0;
    }
    candidate.committed = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_runtime_map_containers(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapContainerReceipt *out) {
    DM2_V1_G1RuntimeMapValidationReceipt validation;
    DM2_V1_G1RuntimeMapContainerReceipt candidate;
    int column_index = 0;

    /* skproject SKULLWIN/c_map.cpp obtains the root before c_record.cpp's
     * GET_NEXT_RECORD_LINK. SKWIN/DME.h::Container defines b4's open/type
     * bits; this receipt does not read w0 or the w2 contained ObjectID. */
    if (!out || !d || !d->raw_data ||
        !dm2_v1_dungeon_validate_g1_runtime_map(d, map, &validation) ||
        !validation.committed || !validation.incomplete_world) {
        return 0;
    }
    for (int level = 0; level < map; ++level)
        column_index += d->level_widths[level];

    memset(&candidate, 0, sizeof(candidate));
    candidate.incomplete_world = 1;
    candidate.map = map;
    for (int x = 0; x < validation.width; ++x) {
        int stack = (int)RD16(d->raw_data + d->column_index_base +
                              (column_index + x) * 2);
        for (int y = 0; y < validation.height; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            uint16_t root;
            const uint8_t *record;
            DM2_V1_G1DirectContainerRoot *container;

            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            if (stack < 0 || stack >= d->square_first_thing_count) return 0;
            root = RD16(d->raw_data + d->square_first_thing_base + stack * 2);
            if (((root >> 10) & 0x0fu) == 9u &&
                dm2_v1_g1_link_has_declared_shape(d, root)) {
                if (candidate.container_root_count >=
                    DM2_V1_G1_RUNTIME_MAP_MAX_CONTAINER_ROOTS) {
                    return 0;
                }
                record = dm2_v1_dungeon_get_thing_record(
                    d, root, NULL, NULL, NULL);
                if (!record) return 0;
                container =
                    &candidate.containers[candidate.container_root_count++];
                container->x = x;
                container->y = y;
                container->object_id = root;
                container->index = root & 0x03ff;
                container->direction = (uint8_t)(root >> 14);
                container->opened = (uint8_t)(record[4] & 1u);
                container->container_type = (uint8_t)((record[4] >> 1) & 3u);
                ++candidate.container_record_reads;
            }
            ++stack;
        }
    }
    if (candidate.container_record_reads != candidate.container_root_count ||
        candidate.generic_record_reads != 0 ||
        candidate.blocked_record_reads != 0) {
        return 0;
    }
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

static int dm2_v1_g1_decode_dungeon_text(
    const DM2_V1_DungeonData *d,
    uint16_t text_index,
    char out_text[DM2_V1_G1_TEXT_MESSAGE_CHARS],
    uint16_t *out_word_count)
{
    int word_index;
    int output = 0;

    if (!d || !out_text || !out_word_count || !d->raw_data ||
        d->text_data_base < 0 || d->text_word_count <= 0 ||
        text_index >= (uint16_t)d->text_word_count) {
        return 0;
    }
    out_text[0] = '\0';
    *out_word_count = 0u;
    for (word_index = (int)text_index;
         word_index < d->text_word_count;
         ++word_index) {
        const uint8_t *word_ptr = d->raw_data + d->text_data_base +
                                  word_index * 2;
        uint16_t word;
        int groups[3];

        if (word_ptr < d->raw_data || word_ptr + 2 > d->raw_data + d->raw_size ||
            word_index - (int)text_index >= UINT16_MAX) {
            return 0;
        }
        /* skproject QUERY_MESSAGE_TEXT: dunTextData is host-endian U16 in
         * memory after the file loader's little-endian read. */
        word = RD16(word_ptr);
        groups[0] = (word >> 10) & 0x1f;
        groups[1] = (word >> 5) & 0x1f;
        groups[2] = word & 0x1f;
        for (int group = 0; group < 3; ++group) {
            int code = groups[group];
            char c;

            if (code == 31) {
                out_text[output] = '\0';
                *out_word_count = (uint16_t)(word_index - (int)text_index + 1);
                return output > 0;
            }
            /* Codes 29 and 30 expand through skproject's private phrase
             * banks (_4976_0362/_4976_0262).  Do not substitute text. */
            if (code == 29 || code == 30 || output + 1 >= DM2_V1_G1_TEXT_MESSAGE_CHARS) {
                return 0;
            }
            if (code < 26) c = (char)('A' + code);
            else if (code == 26) c = ' ';
            else if (code == 27) c = '.';
            else if (code == 28) c = '\n';
            else return 0;
            out_text[output++] = c;
        }
    }
    return 0;
}

int dm2_v1_dungeon_materialize_g1_map5_text_messages(
    const DM2_V1_DungeonData *d,
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1TextMessageRuntimeReceipt *out)
{
    DM2_V1_G1TextMessageRuntimeReceipt candidate;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!d || !texts || !texts->committed || !texts->incomplete_world ||
        texts->map != 5 || texts->generic_record_reads != 0 ||
        texts->blocked_record_reads != 0 || texts->text_root_count < 0 ||
        texts->text_root_count > DM2_V1_G1_MAP5_MAX_TEXT_ROOTS) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.map = texts->map;
    candidate.source_text_root_count = texts->text_root_count;
    for (i = 0; i < texts->text_root_count; ++i) {
        const DM2_V1_G1TextRoot *root = &texts->texts[i];
        DM2_V1_G1TextMessage *message;

        if (!root->visible) continue;
        if (root->mode != 0u) {
            ++candidate.skipped_non_dungeon_message_count;
            continue;
        }
        if (candidate.decoded_message_count >= DM2_V1_G1_TEXT_MESSAGE_MAX) return 0;
        message = &candidate.messages[candidate.decoded_message_count];
        if (!dm2_v1_g1_decode_dungeon_text(d, root->text_index, message->text,
                                            &message->source_word_count)) {
            ++candidate.blocked_phrase_message_count;
            memset(message, 0, sizeof(*message));
            continue;
        }
        message->x = root->x;
        message->y = root->y;
        message->object_id = root->object_id;
        message->text_index = root->text_index;
        ++candidate.decoded_message_count;
    }
    candidate.valid = 1;
    *out = candidate;
    return 1;
}

static uint32_t dm2_v1_g1_gdat_text_hash(const uint8_t *data,
                                          uint32_t byte_count)
{
    uint32_t hash = 2166136261u;
    uint32_t i;

    if (!data || byte_count == 0u) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        hash = (hash ^ data[i]) * 16777619u;
    }
    return hash ? hash : 1u;
}

int dm2_v1_dungeon_materialize_g1_map5_gdat_text_messages(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatTextRead read_text,
    void *read_userdata,
    DM2_V1_G1GdatTextMessageRuntimeReceipt *out)
{
    DM2_V1_G1GdatTextMessageRuntimeReceipt candidate;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!texts || !read_text || !texts->committed ||
        !texts->incomplete_world || texts->map != 5 ||
        texts->generic_record_reads != 0 ||
        texts->blocked_record_reads != 0 || texts->text_root_count < 0 ||
        texts->text_root_count > DM2_V1_G1_MAP5_MAX_TEXT_ROOTS) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.map = texts->map;
    candidate.source_text_root_count = texts->text_root_count;
    for (i = 0; i < texts->text_root_count; ++i) {
        const DM2_V1_G1TextRoot *root = &texts->texts[i];
        const uint8_t *raw = NULL;
        uint32_t raw_byte_count = 0u;
        uint8_t extension_usage;
        uint8_t field;
        DM2_V1_G1GdatTextMessage *message;

        if (!root->visible || root->mode != 1u) continue;
        extension_usage = (uint8_t)((root->text_index >> 8) & 0x1fu);
        if (extension_usage != 14u) continue;
        if (candidate.material_count >= DM2_V1_G1_GDAT_TEXT_MESSAGE_MAX) {
            return 0;
        }
        field = (uint8_t)(root->text_index & 0xffu);
        /* skproject c_querydb.cpp DM2_QUERY_MESSAGE_TEXT E091-E146:
         * extension usage 14 sets vb_90 to the low TextIndex byte, then
         * queries GDAT category 3, index 0. Do not decode FORMAT_SKSTR here. */
        if (!read_text(read_userdata, DM2_GDAT_CATEGORY_MESSAGES, 0, field,
                       &raw, &raw_byte_count) || !raw || raw_byte_count == 0u) {
            ++candidate.blocked_missing_text_count;
            return 0;
        }
        message = &candidate.messages[candidate.material_count++];
        message->x = root->x;
        message->y = root->y;
        message->object_id = root->object_id;
        message->text_index = root->text_index;
        message->gdat_field = field;
        message->raw_byte_count = raw_byte_count;
        message->raw_hash = dm2_v1_g1_gdat_text_hash(raw, raw_byte_count);
        if (message->raw_hash == 0u) return 0;
    }
    candidate.valid = 1;
    *out = candidate;
    return 1;
}

typedef struct {
    uint16_t colorkey;
    uint16_t position;
    uint16_t do_not_flip;
    uint16_t alcove_type;
    uint16_t image_offset;
} DM2_V1_G1WallGfxScalars;

static __attribute__((unused)) int dm2_v1_g1_read_wall_gfx_scalars(
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    uint8_t wall_gfx_index,
    DM2_V1_G1WallGfxScalars *out)
{
    if (!read_scalar || !out ||
        !read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x04,
                     &out->colorkey) ||
        !read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x05,
                     &out->position) ||
        !read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x07,
                     &out->do_not_flip) ||
        !read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x0a,
                     &out->alcove_type) ||
        !read_scalar(read_userdata, 0x0c, 0x09, wall_gfx_index, 0xfd,
                     &out->image_offset) ||
        out->position > 24u || out->alcove_type > 3u) {
        return 0;
    }
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
    /* skdungn.cpp consumes DB2 TextMode wall-GFX only through the G1
     * map-to-record graph; an incomplete graph must fail closed (the
     * 3cf040333 gate, lost when 6ab20e42d inlined the owner walk). */
    if (!dm2_v1_dungeon_record_list_traversal_allowed(d) ||
        !out_wall_gfx_index || !out_wall_gfx_field) return -1;
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
    /* GET_WALL_DECORATION_OF_ACTUATOR walks the same G1 record-list route;
     * an incomplete graph must fail closed (the 3cf040333 gate, lost in a
     * later rewrite of this walk). */
    if (!dm2_v1_dungeon_record_list_traversal_allowed(d) ||
        !out_wall_gfx_ordinal) return -1;
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

static int dm2_v1_g1_wall_gfx_read_scalars(
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    int wall_gfx_index,
    uint16_t *out_colorkey,
    uint16_t *out_position,
    uint16_t *out_do_not_flip,
    uint16_t *out_alcove_type,
    uint16_t *out_image_offset)
{
    return read_scalar && out_colorkey && out_position && out_do_not_flip &&
           out_alcove_type && out_image_offset &&
           read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x04,
                       out_colorkey) &&
           read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x05,
                       out_position) &&
           read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x07,
                       out_do_not_flip) &&
           read_scalar(read_userdata, 0x0b, 0x09, wall_gfx_index, 0x0a,
                       out_alcove_type) &&
           read_scalar(read_userdata, 0x0c, 0x09, wall_gfx_index, 0xfd,
                       out_image_offset);
}

static int dm2_v1_g1_text_index_allows_wall_gfx(uint16_t text_index)
{
    switch ((text_index >> 8) & 0xffu) {
        case 0x00:
        case 0x02:
        case 0x03:
        case 0x05:
        case 0x0d:
            return 1;
        default:
            return 0;
    }
}

int dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out)
{
    int i;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!texts || !texts->committed || texts->generic_record_reads ||
        texts->blocked_record_reads || !read_scalar) {
        return 0;
    }
    out->valid = 1;
    out->map = texts->map;
    out->source_text_root_count = texts->text_root_count;
    for (i = 0; i < texts->text_root_count &&
                i < DM2_V1_G1_MAP5_MAX_TEXT_ROOTS &&
                out->material_count < DM2_V1_G1_TEXT_WALL_GFX_MAX; ++i) {
        const DM2_V1_G1TextRoot *root = &texts->texts[i];
        DM2_V1_G1TextWallGfxMaterial *mat;
        int wall_gfx_index;
        if (root->mode != 1) continue;
        if (!dm2_v1_g1_text_index_allows_wall_gfx(root->text_index)) continue;
        wall_gfx_index = (int)(root->text_index & 0xffu);
        mat = &out->materials[out->material_count];
        memset(mat, 0, sizeof(*mat));
        if (!dm2_v1_g1_wall_gfx_read_scalars(
                read_scalar, read_userdata, wall_gfx_index, &mat->colorkey,
                &mat->position, &mat->do_not_flip, &mat->alcove_type,
                &mat->image_offset)) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
        mat->x = root->x;
        mat->y = root->y;
        mat->object_id = root->object_id;
        mat->direction = root->direction;
        mat->text_index = root->text_index;
        mat->wall_gfx_index = (uint8_t)wall_gfx_index;
        ++out->material_count;
    }
    return 1;
}

int dm2_v1_dungeon_materialize_g1_text_wall_gfx_image_runtime(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatScalarRead read_scalar,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    void *read_userdata,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out)
{
    int i;
    if (!dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
            texts, read_scalar, read_userdata, out)) return 0;
    if (!read_image_metadata) return 1;
    for (i = 0; i < out->material_count; ++i) {
        int w = 0, h = 0, f = 0;
        if (read_image_metadata(read_userdata, 0x09,
                                out->materials[i].wall_gfx_index, 1,
                                &w, &h, &f)) {
            out->materials[i].front_image_ready = 1;
            out->materials[i].front_image_width = (uint16_t)w;
            out->materials[i].front_image_height = (uint16_t)h;
            out->materials[i].front_image_format = (uint8_t)f;
        }
    }
    return 1;
}

int dm2_v1_dungeon_materialize_g1_text_wall_gfx_image_material_runtime(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatScalarRead read_scalar,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out)
{
    int i;
    if (!dm2_v1_dungeon_materialize_g1_text_wall_gfx_image_runtime(
            texts, read_scalar, read_image_metadata, read_userdata, out))
        return 0;
    if (!read_local_palette) return 1;
    for (i = 0; i < out->material_count; ++i) {
        uint32_t hash = 0;
        if (out->materials[i].front_image_ready &&
            read_local_palette(read_userdata, 0x09,
                               out->materials[i].wall_gfx_index, 1,
                               out->materials[i].local_palette16, &hash)) {
            out->materials[i].local_palette_hash = hash;
        } else {
            out->materials[i].front_image_ready = 0;
            out->materials[i].local_palette_hash = 0;
            memset(out->materials[i].local_palette16, 0, 16u);
        }
    }
    return 1;
}

int dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out)
{
    DM2_V1_G1ActuatorWallGfxMaterial *mat;
    int ordinal;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!d || !d->raw_data || !d->record_graph_complete || !read_scalar) {
        return 0;
    }
    ordinal = (int)((RD16(d->raw_data + 8) >> 12) & 0x0fu);
    if (ordinal <= 0 || d->raw_map_data_base < 0) return 0;
    out->valid = 1;
    out->map = map;
    out->source_actuator_root_count = 1;
    out->material_count = 1;
    mat = &out->materials[0];
    memset(mat, 0, sizeof(*mat));
    mat->object_id = RD16(d->raw_data + 2);
    mat->graphic_ordinal = (uint8_t)ordinal;
    mat->wall_gfx_index = d->raw_data[d->raw_map_data_base + ordinal];
    if (!dm2_v1_g1_wall_gfx_read_scalars(
            read_scalar, read_userdata, mat->wall_gfx_index, &mat->colorkey,
            &mat->position, &mat->do_not_flip, &mat->alcove_type,
            &mat->image_offset)) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    return 1;
}

int dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_image_material_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatScalarRead read_scalar,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out)
{
    if (!dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_runtime(
            d, map, read_scalar, read_userdata, out)) return 0;
    if (out->material_count > 0 && read_image_metadata) {
        int w = 0, h = 0, f = 0;
        DM2_V1_G1ActuatorWallGfxMaterial *mat = &out->materials[0];
        if (read_image_metadata(read_userdata, 0x09, mat->wall_gfx_index, 1,
                                &w, &h, &f)) {
            mat->front_image_ready = 1;
            mat->front_image_width = (uint16_t)w;
            mat->front_image_height = (uint16_t)h;
            mat->front_image_format = (uint8_t)f;
            if (read_local_palette) {
                uint32_t hash = 0;
                if (read_local_palette(read_userdata, 0x09,
                                       mat->wall_gfx_index, 1,
                                       mat->local_palette16, &hash)) {
                    mat->local_palette_hash = hash;
                } else {
                    mat->front_image_ready = 0;
                    memset(mat->local_palette16, 0, 16u);
                    mat->local_palette_hash = 0;
                }
            }
        }
    }
    return 1;
}

int dm2_v1_g1_text_wall_gfx_allows_button_material(
    const DM2_V1_G1TextWallGfxRuntimeReceipt *receipt,
    int wall_gfx_index,
    int image_field)
{
    int i;

    if (!receipt || !receipt->valid || wall_gfx_index < 0 ||
        wall_gfx_index > 0xff || image_field != 1) return 0;
    for (i = 0; i < receipt->material_count; ++i) {
        if (receipt->materials[i].wall_gfx_index == (uint8_t)wall_gfx_index) {
            return 1;
        }
    }
    return 0;
}

int dm2_v1_g1_actuator_wall_gfx_allows_button_material(
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *receipt,
    int wall_gfx_index,
    int image_field)
{
    int i;

    if (!receipt || !receipt->valid || wall_gfx_index < 0 ||
        wall_gfx_index > 0xff || image_field != 1) return 0;
    for (i = 0; i < receipt->material_count; ++i) {
        if (receipt->materials[i].wall_gfx_index == (uint8_t)wall_gfx_index) {
            return 1;
        }
    }
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

int dm2_v1_dungeon_get_map_floor_gfx_list(
    const DM2_V1_DungeonData *d,
    int level,
    uint8_t *out_floor_gfx_list,
    int out_capacity)
{
    const uint8_t *map_desc;
    int wall_gfx_count;
    int floor_gfx_count;
    int creature_count;
    int map_base;
    int list_base;

    if (out_floor_gfx_list && out_capacity > 0) {
        memset(out_floor_gfx_list, 0, (size_t)out_capacity);
    }
    if (!d || level < 0 || level >= d->level_count || !d->raw_data ||
        d->raw_size <= 0 || out_capacity < 0 || level >= DM2_V1_MAX_LEVELS ||
        d->raw_size < DM2_DUNGEON_HEADER_SIZE +
                          (level + 1) * DM2_MAP_DESC_SIZE) {
        return -1;
    }
    map_desc = d->raw_data + DM2_DUNGEON_HEADER_SIZE +
               level * DM2_MAP_DESC_SIZE;
    wall_gfx_count = (int)(RD16(map_desc + 10) & 0x0fu);
    floor_gfx_count = (int)((RD16(map_desc + 10) >> 8) & 0x0fu);
    creature_count = (int)((RD16(map_desc + 12) >> 4) & 0x0fu);
    if (floor_gfx_count <= 0) return 0;
    if (!out_floor_gfx_list || out_capacity < floor_gfx_count ||
        d->raw_map_data_base < 0 || d->level_widths[level] <= 0 ||
        d->level_heights[level] <= 0) {
        return -1;
    }
    /* DME.h Map_definitions::FloorGraphics and SkWinCore.cpp
     * LOAD_LOCALLEVEL_DYN: creature list, wall list, then floor list. */
    map_base = d->raw_map_data_base + d->level_offsets[level];
    list_base = map_base + d->level_widths[level] * d->level_heights[level] +
                creature_count + wall_gfx_count;
    if (map_base < 0 || list_base < map_base ||
        list_base + floor_gfx_count > d->raw_size) {
        return -1;
    }
    memcpy(out_floor_gfx_list, d->raw_data + list_base,
           (size_t)floor_gfx_count);
    return floor_gfx_count;
}

int dm2_v1_dungeon_get_map_door_ornate_list(
    const DM2_V1_DungeonData *d,
    int level,
    uint8_t *out_door_ornate_list,
    int out_capacity)
{
    const uint8_t *map_desc;
    int creature_count;
    int wall_gfx_count;
    int floor_gfx_count;
    int ornate_count;
    int map_base;
    int list_base;

    if (out_door_ornate_list && out_capacity > 0)
        memset(out_door_ornate_list, 0, (size_t)out_capacity);
    if (!d || level < 0 || level >= d->level_count || !d->raw_data ||
        d->raw_size <= 0 || out_capacity < 0 || level >= DM2_V1_MAX_LEVELS ||
        d->raw_size < DM2_DUNGEON_HEADER_SIZE +
                          (level + 1) * DM2_MAP_DESC_SIZE) return -1;
    map_desc = d->raw_data + DM2_DUNGEON_HEADER_SIZE +
               level * DM2_MAP_DESC_SIZE;
    creature_count = (int)((RD16(map_desc + 12) >> 4) & 0x0fu);
    wall_gfx_count = (int)(RD16(map_desc + 10) & 0x0fu);
    floor_gfx_count = (int)((RD16(map_desc + 10) >> 8) & 0x0fu);
    ornate_count = (int)(RD16(map_desc + 12) & 0x0fu);
    if (ornate_count <= 0) return 0;
    if (!out_door_ornate_list || out_capacity < ornate_count ||
        d->raw_map_data_base < 0 || d->level_widths[level] <= 0 ||
        d->level_heights[level] <= 0) return -1;
    /* DME.h::DoorDecorationGraphics and SkWinCore.cpp::
     * LOAD_LOCALLEVEL_GRAPHICS_TABLE: creature, wall, floor, then door
     * ornament graphics. DB0's OrnamentIndex remains one-based. */
    map_base = d->raw_map_data_base + d->level_offsets[level];
    list_base = map_base + d->level_widths[level] * d->level_heights[level] +
                creature_count + wall_gfx_count + floor_gfx_count;
    if (map_base < 0 || list_base < map_base ||
        list_base + ornate_count > d->raw_size) return -1;
    memcpy(out_door_ornate_list, d->raw_data + list_base,
           (size_t)ornate_count);
    return ornate_count;
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

int dm2_v1_dungeon_c_light_map_descriptor_receipt(
    const DM2_V1_DungeonData *d, int level,
    DM2_V1_CLightMapDescriptorReceipt *out)
{
    const uint8_t *map_desc;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!d || !d->raw_data || d->raw_size <= 0 || level < 0 ||
        level >= d->level_count || level >= DM2_V1_MAX_LEVELS ||
        d->raw_size < DM2_DUNGEON_HEADER_SIZE +
                          (level + 1) * DM2_MAP_DESC_SIZE) {
        return 0;
    }
    map_desc = d->raw_data + DM2_DUNGEON_HEADER_SIZE +
               level * DM2_MAP_DESC_SIZE;
    for (int i = 0; i < DM2_MAP_DESC_SIZE; ++i) {
        hash ^= map_desc[i];
        hash *= 16777619u;
    }
    if (hash == 0u) return 0;
    out->valid = 1;
    out->level = level;
    out->difficulty = (uint8_t)((RD16(map_desc + 12u) >> 12) & 0x0fu);
    out->dynamic_light = out->difficulty != 0u;
    out->descriptor_hash = hash;
    return 1;
}

static uint32_t dm2_v1_g1_raw_hash(const uint8_t *data, uint32_t byte_count)
{
    uint32_t hash = 2166136261u;
    uint32_t i;

    for (i = 0; i < byte_count; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t dm2_v1_g1_material_identity_step(uint32_t hash,
                                                   uint32_t value)
{
    int shift;

    for (shift = 0; shift < 32; shift += 8) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static int dm2_v1_g1_static_object_material_selector_fill(
    uint16_t object_id, int x, int y, uint8_t category, uint8_t item_type,
    uint8_t image_field, uint8_t direction, uint8_t container_open,
    uint16_t image_offset, DM2_V1_G1StaticObjectMaterialSelector *out)
{
    uint32_t hash = 2166136261u;

    if (!out || object_id == 0xfffeu ||
        (category != 0x10u && category != 0x14u) ||
        (category == 0x10u && image_field != 0u) ||
        (category == 0x14u && image_field != 0u && image_field != 4u)) {
        if (out) memset(out, 0, sizeof(*out));
        return 0;
    }
    memset(out, 0, sizeof(*out));
    hash = dm2_v1_g1_material_identity_step(hash, object_id);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)x);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)y);
    hash = dm2_v1_g1_material_identity_step(hash, category);
    hash = dm2_v1_g1_material_identity_step(hash, item_type);
    hash = dm2_v1_g1_material_identity_step(hash, image_field);
    hash = dm2_v1_g1_material_identity_step(hash, direction);
    hash = dm2_v1_g1_material_identity_step(hash, container_open);
    hash = dm2_v1_g1_material_identity_step(hash, image_offset);
    out->object_id = object_id; out->x = x; out->y = y;
    out->category = category; out->item_type = item_type;
    out->image_field = image_field; out->direction = direction;
    out->container_open = container_open; out->image_offset = image_offset;
    out->identity_hash = hash ? hash : 1u; out->valid = 1;
    return 1;
}

int dm2_v1_g1_static_object_material_selector(
    const DM2_V1_G1DirectWeaponRoot *weapon, uint16_t image_offset,
    DM2_V1_G1StaticObjectMaterialSelector *out)
{
    if (!weapon) { if (out) memset(out, 0, sizeof(*out)); return 0; }
    return dm2_v1_g1_static_object_material_selector_fill(
        weapon->object_id, weapon->x, weapon->y, 0x10u, weapon->item_type,
        0u, weapon->direction, 0u, image_offset, out);
}

int dm2_v1_g1_static_container_material_selector(
    const DM2_V1_G1DirectContainerRoot *container, uint16_t image_offset,
    DM2_V1_G1StaticObjectMaterialSelector *out)
{
    if (!container) { if (out) memset(out, 0, sizeof(*out)); return 0; }
    return dm2_v1_g1_static_object_material_selector_fill(
        container->object_id, container->x, container->y, 0x14u,
        container->container_type, container->opened ? 4u : 0u,
        container->direction, container->opened ? 1u : 0u, image_offset, out);
}

int dm2_v1_g1_query_creature_blit_recti(int cell_pos, int position_5x5,
                                        int direction,
                                        uint16_t *out_rect_id)
{
    int x;
    int y;
    int tmp;

    if (!out_rect_id) return 0;
    *out_rect_id = 0u;
    if (cell_pos < 0 || cell_pos > 22 || position_5x5 < 0 ||
        position_5x5 > 24 || direction < 0 || direction > 3) return 0;
    x = position_5x5 % 5 - 2;
    y = position_5x5 / 5 - 2;
    switch (direction) {
    case 1: tmp = x; x = y; y = -tmp; break;
    case 2: x = -x; y = -y; break;
    case 3: tmp = x; x = -y; y = tmp; break;
    default: break;
    }
    *out_rect_id = (uint16_t)(5000 + cell_pos * 25 + x + (y + 2) * 5 + 2);
    return 1;
}

int dm2_v1_g1_direct_missile_timer_receipt(const uint8_t *timer_table,
                                           size_t timer_table_size,
                                           uint16_t timer_index,
                                           DM2_V1_G1MissileTimerReceipt *out)
{
    const uint8_t *raw;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* SKWIN/DME.h::Timer is a ten-byte raw table row. */
    if (!timer_table || timer_index == 0xffffu ||
        timer_index >= timer_table_size / 10u || timer_table_size % 10u != 0u)
        return 0;
    raw = timer_table + (size_t)timer_index * 10u;
    for (int i = 0; i < 10; ++i) { hash ^= raw[i]; hash *= 16777619u; }
    out->timer_index = timer_index;
    out->timer_type = raw[4];
    out->actor = raw[5];
    out->value = RD16(raw + 6);
    out->action_word = RD16(raw + 8);
    /* DME.h::Timer::Direction() is ActionType's bits 2..3. */
    out->direction = (uint8_t)((out->action_word >> 10) & 3u);
    out->raw_timer_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
}

int dm2_v1_g1_flying_item_source_receipt(
    uint16_t missile_object_id, int category, int item_type, int image_field,
    int flip_flags, int cell_pos, int position_5x5, int stretch_factor64,
    DM2_V1_G1FlyingItemSourceReceipt *out)
{
    uint32_t hash = 2166136261u;
    uint16_t clip_rect_id;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (missile_object_id == 0xfffeu || (category != 0x0d && category != 0x0e) ||
        item_type < 0 || item_type > 0xff || (image_field != 8 && image_field != 9 &&
        image_field != 10 && image_field != 12) || flip_flags < 0 || flip_flags > 3 ||
        stretch_factor64 <= 0 || !dm2_v1_g1_query_creature_blit_recti(
            cell_pos, position_5x5, 0, &clip_rect_id)) return 0;
    hash = dm2_v1_g1_material_identity_step(hash, missile_object_id);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)category);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)item_type);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)image_field);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)flip_flags);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)cell_pos);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)position_5x5);
    hash = dm2_v1_g1_material_identity_step(hash, clip_rect_id);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)stretch_factor64);
    out->missile_object_id=missile_object_id; out->category=(uint8_t)category;
    out->item_type=(uint8_t)item_type; out->image_field=(uint8_t)image_field;
    out->flip_flags=(uint8_t)flip_flags; out->cell_pos=(uint8_t)cell_pos;
    out->position_5x5=(uint8_t)position_5x5;
    out->clip_rect_id=clip_rect_id;
    out->stretch_factor64=(uint16_t)stretch_factor64; out->identity_hash=hash?hash:1u;
    out->valid=1; return 1;
}

int dm2_v1_g1_direct_missile_receipt(const DM2_V1_DungeonData *d,
                                     uint16_t object_id,
                                     DM2_V1_G1DirectMissileReceipt *out)
{
    const uint8_t *record;
    int type = -1, index = -1, size = 0;
    uint32_t hash = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    record = dm2_v1_dungeon_get_thing_record(d, object_id, &type, &index, &size);
    if (!record || type != 14 || index < 0 || size != 8) return 0;
    /* DME.h::Missile: w2, b4, b5, w6. w0 is deliberately never read. */
    for (int i = 2; i < 8; ++i) { hash ^= record[i]; hash *= 16777619u; }
    out->object_id = object_id;
    out->missile_object = RD16(record + 2);
    out->energy_remaining = record[4]; out->energy_remaining2 = record[5];
    out->timer_index = RD16(record + 6);
    out->record_hash = hash ? hash : 1u; out->valid = 1;
    return 1;
}

int dm2_v1_g1_flying_item_selector_receipt(
    const DM2_V1_DungeonData *d, const DM2_V1_AssetLoader *loader,
    const DM2_V1_G1DirectMissileReceipt *missile,
    DM2_V1_G1FlyingItemSelectorReceipt *out)
{
    static const uint8_t class1_by_type[16] = {
        0x0e, 0x18, 0xff, 0xff, 0x0f, 0x10, 0x11, 0x12,
        0x13, 0x14, 0x15, 0xff, 0xff, 0xff, 0xff, 0x0d };
    const uint8_t *record;
    const uint8_t *missile_record;
    int type = -1, index = -1, size = 0;
    uint16_t object;
    uint32_t hash = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!d || !loader || !missile || !missile->valid) return 0;
    object = missile->missile_object;
    /* Missile::missile_object is resolved through the same record path as
     * DM2_DRAW_FLYING_ITEM; only its DB14 indirection is followed. */
    missile_record = dm2_v1_dungeon_get_thing_record(
        d, missile->object_id, NULL, NULL, NULL);
    record = dm2_v1_dungeon_get_thing_record(d, object, &type, &index, &size);
    if (!missile_record || !record || index < 0 || type < 0 || type > 15 || size < 4) return 0;
    if (type == 14) {
        object = RD16(record + 2);
        record = dm2_v1_dungeon_get_thing_record(d, object, &type, &index, &size);
        if (!record || index < 0 || type < 0 || type > 15 || size < 4) return 0;
    }
    if (class1_by_type[type] == 0xff) return 0;
    out->class1 = class1_by_type[type];
    /* c_record.cpp QUERY_CLS2_FROM_RECORD's bounded item branches. */
    if (type == 4) out->class2 = record[4];
    else if (type == 5 || type == 6 || type == 10 || type == 15)
        out->class2 = (uint8_t)(RD16(record + 2) & 0x7fu);
    else if (type == 7) out->class2 = 0u;
    else return 0;
    out->record_byte4 = missile_record[4];
    if (out->class1 == 0x0du) {
        if (missile_record[4] == 0xffu ||
            !dm2_v1_query_gdat_entry_data_index(loader, 0x0d, out->class2,
                                                  0x0b, 1,
                                                  &out->image_data_index) ||
            !out->image_data_index) return 0;
        out->branch_temp_picst = 1;
    }
    hash ^= missile->record_hash; hash *= 16777619u;
    hash ^= object; hash *= 16777619u;
    hash ^= out->class1; hash *= 16777619u;
    hash ^= out->class2; hash *= 16777619u;
    hash ^= out->record_byte4; hash *= 16777619u;
    hash ^= out->image_data_index; hash *= 16777619u;
    out->missile_object_id = missile->object_id;
    out->missile_object = object;
    out->identity_hash = hash ? hash : 1u;
    out->valid = 1;
    return 1;
}

int dm2_v1_g1_flying_item_geometry_receipt(
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    int view_position, DM2_V1_G1FlyingItemGeometryReceipt *out)
{
    static const int8_t table1d6afe[23] = {
        0,-1,1,0,-1,1,0,-1,1,-2,2,0,-1,1,-2,2,0,-1,1,-2,2,-3,3 };
    static const int8_t table1d6b43[23] = {
        11,-1,-1,8,9,10,5,6,7,-1,-1,0,1,2,3,4,-1,-1,-1,-1,-1,-1,-1 };
    static const int8_t table1d6b15[23] = {
        0,0,0,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,4,4 };
    uint32_t hash = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!selector || !selector->valid || view_position < 0 ||
        view_position >= 23 || table1d6b43[view_position] < 0) return 0;
    for (int i = 0; i < 23; ++i) {
        hash ^= (uint8_t)table1d6afe[i]; hash *= 16777619u;
        hash ^= (uint8_t)table1d6b43[i]; hash *= 16777619u;
        hash ^= (uint8_t)table1d6b15[i]; hash *= 16777619u;
    }
    out->valid = 1; out->no_draw = 1;
    out->view_position = (uint8_t)view_position;
    out->depth_band = table1d6b15[view_position];
    out->placement_x = table1d6afe[view_position];
    out->placement_y = table1d6b43[view_position];
    out->temp_picst_eligible = selector->class1 == 0x0d &&
        selector->branch_temp_picst;
    out->draw_item_opaque = !out->temp_picst_eligible;
    out->table_hash = hash ? hash : 1u;
    hash ^= selector->identity_hash; hash *= 16777619u;
    hash ^= (uint32_t)view_position; hash *= 16777619u;
    out->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_g1_flying_item_vb30_receipt(
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Inputs *in,
    DM2_V1_G1FlyingItemVb30Receipt *out)
{
    uint32_t hash = 2166136261u;
    uint8_t vb30;
    int blocked = 0;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!selector || !selector->valid || !in || in->query_48ae_state > 3u ||
        in->timer_direction > 3u || in->viewport_direction > 3u ||
        in->direction_5x5 > 3u || in->table_b43 < 0 || in->table_b15 < 0) return 0;
    /* c_gui_vp.cpp:3610-3745: vb30 is selected solely by this observed
     * query_48ae/timer/viewport branch; no caller may provide a field. */
    if (in->query_48ae_state == 3u) vb30 = 8u;
    else if ((in->timer_direction & 1u) != (in->viewport_direction & 1u)) vb30 = 12u;
    else if (in->query_48ae_state == 2u ||
             (in->query_48ae_state == 1u && in->timer_direction != in->viewport_direction)) vb30 = 8u;
    else if (in->query_48ae_state == 1u) vb30 = 10u;
    else if (((in->table_afe + in->table_b43) & 1) == 0) {
        vb30 = in->direction_5x5 < 2u ? 9u : 8u;
    } else {
        vb30 = in->direction_5x5 >= 2u ? 9u : 8u;
    }
    /* skip00923 belongs only to the state-zero, parity-matched branch. */
    if (in->query_48ae_state == 0u &&
        (in->timer_direction & 1u) == (in->viewport_direction & 1u) &&
        (in->direction_5x5 & 1u) && selector->class1 == 0x0du) blocked = 1;
    hash ^= selector->identity_hash; hash *= 16777619u;
    hash ^= vb30; hash *= 16777619u;
    hash ^= in->query_48ae_state; hash *= 16777619u;
    hash ^= in->timer_direction; hash *= 16777619u;
    hash ^= in->viewport_direction; hash *= 16777619u;
    out->valid = 1; out->vb30 = vb30; out->temp_picst_blocked = (uint8_t)blocked;
    out->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_g1_flying_item_summary_image_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    DM2_V1_QueryGdatSummaryImageReceipt *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !selector || !vb30 || !selector->valid || !vb30->valid ||
        vb30->temp_picst_blocked || (vb30->vb30 != 8u && vb30->vb30 != 9u &&
        vb30->vb30 != 10u && vb30->vb30 != 12u)) return 0;
    return dm2_v1_query_gdat_summary_image_receipt(
        loader, selector->class1, selector->class2, vb30->vb30, out) &&
        out->accepted && !out->gdat_bypassed_for_ff && out->palette_hash;
}

int dm2_v1_g1_flying_item_decoded_material_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    DM2_V1_G1FlyingItemDecodedMaterialReceipt *out)
{
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    DM2_V1_GdatGfxRawMaterialReceipt raw;
    uint8_t *pixels;
    int width = 0, height = 0;
    DM2_ImageFormat format;
    size_t pixel_count;
    uint32_t pixel_hash = 2166136261u;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !selector || !vb30 || !geometry || !selector->valid ||
        !vb30->valid || vb30->temp_picst_blocked || !geometry->valid ||
        !geometry->no_draw || !geometry->temp_picst_eligible ||
        geometry->draw_item_opaque || geometry->image_field_available ||
        !selector->branch_temp_picst || selector->class1 != 0x0du ||
        !selector->identity_hash || !vb30->identity_hash ||
        !geometry->identity_hash || !dm2_v1_g1_flying_item_summary_image_receipt(
            loader, selector, vb30, &summary) || !summary.accepted ||
        !summary.palette_hash || !summary.receipt_hash ||
        !summary.offset_receipt_hash ||
        !dm2_v1_gdat_image_raw_material_receipt(
            loader, selector->class1, selector->class2, vb30->vb30, &raw) ||
        !raw.accepted || !raw.source_bytes || !raw.source_byte_count ||
        !raw.source_hash || !raw.receipt_hash ||
        raw.raw_index != (uint16_t)(summary.data_index & 0x7fffu)) return 0;
    pixels = dm2_v1_asset_load_image_field(loader, selector->class1,
        selector->class2, vb30->vb30, &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0 || width != summary.metadata.width ||
        height != summary.metadata.height || (format != DM2_IMG_FMT_U4 &&
        format != DM2_IMG_FMT_U8) || (size_t)width > SIZE_MAX / (size_t)height) {
        free(pixels);
        return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    for (size_t i = 0u; i < pixel_count; ++i) {
        pixel_hash ^= pixels[i]; pixel_hash *= 16777619u;
    }
    free(pixels);
    if (!pixel_hash) return 0;
    hash = dm2_v1_g1_material_identity_step(hash, selector->identity_hash);
    hash = dm2_v1_g1_material_identity_step(hash, vb30->identity_hash);
    hash = dm2_v1_g1_material_identity_step(hash, geometry->identity_hash);
    hash = dm2_v1_g1_material_identity_step(hash, summary.receipt_hash);
    hash = dm2_v1_g1_material_identity_step(hash, summary.offset_receipt_hash);
    hash = dm2_v1_g1_material_identity_step(hash, raw.source_hash);
    hash = dm2_v1_g1_material_identity_step(hash, raw.receipt_hash);
    hash = dm2_v1_g1_material_identity_step(hash, pixel_hash);
    hash = dm2_v1_g1_material_identity_step(hash, summary.palette_hash);
    out->valid = 1; out->no_draw = 1;
    out->category = selector->class1; out->index = selector->class2;
    out->field = vb30->vb30; out->raw_index = raw.raw_index;
    out->width = (uint16_t)width; out->height = (uint16_t)height;
    out->format = format;
    out->source_offset_x = summary.metadata.query_offset_x;
    out->source_offset_y = summary.metadata.query_offset_y;
    out->offset_receipt_hash = summary.offset_receipt_hash;
    out->selector_identity_hash = selector->identity_hash;
    out->vb30_identity_hash = vb30->identity_hash;
    out->geometry_identity_hash = geometry->identity_hash;
    out->summary_receipt_hash = summary.receipt_hash;
    out->raw_gfx256_hash = raw.source_hash;
    out->raw_gfx256_receipt_hash = raw.receipt_hash;
    out->decoded_pixels_hash = pixel_hash;
    out->palette_hash = summary.palette_hash;
    out->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_g1_creature_map_chip_material_identity(
    const DM2_V1_G1CreatureMapChipMaterial *material,
    uint32_t *out_identity)
{
    uint32_t hash = 2166136261u;

    if (out_identity) *out_identity = 0u;
    if (!material || !out_identity || material->object_id == 0xfffeu ||
        material->raw_hash == 0u || material->raw_byte_count == 0u ||
        material->image_width <= 0 || material->image_height <= 0 ||
        material->image_format <= 0 || material->local_palette_hash == 0u) {
        return 0;
    }
    hash = dm2_v1_g1_material_identity_step(hash, material->raw_hash);
    hash = dm2_v1_g1_material_identity_step(hash, material->raw_byte_count);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             material->local_palette_hash);
    hash = dm2_v1_g1_material_identity_step(hash, material->object_id);
    hash = dm2_v1_g1_material_identity_step(hash, material->direction);
    hash = dm2_v1_g1_material_identity_step(hash, material->creature_type);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)material->x);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)material->y);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_width);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_height);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_format);
    *out_identity = hash ? hash : 1u;
    return 1;
}

int dm2_v1_g1_weapon_map_chip_material_identity(
    const DM2_V1_G1WeaponMapChipMaterial *material,
    uint32_t *out_identity)
{
    uint32_t hash = 2166136261u;

    if (out_identity) *out_identity = 0u;
    if (!material || !out_identity || material->object_id == 0xfffeu ||
        material->raw_hash == 0u || material->raw_byte_count == 0u ||
        material->image_width <= 0 || material->image_height <= 0 ||
        material->image_format <= 0 || material->local_palette_hash == 0u) {
        return 0;
    }
    hash = dm2_v1_g1_material_identity_step(hash, material->raw_hash);
    hash = dm2_v1_g1_material_identity_step(hash, material->raw_byte_count);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             material->local_palette_hash);
    hash = dm2_v1_g1_material_identity_step(hash, material->object_id);
    hash = dm2_v1_g1_material_identity_step(hash, material->direction);
    hash = dm2_v1_g1_material_identity_step(hash, material->item_type);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)material->x);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)material->y);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_width);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_height);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_format);
    *out_identity = hash ? hash : 1u;
    return 1;
}

int dm2_v1_g1_container_map_chip_material_identity(
    const DM2_V1_G1ContainerMapChipMaterial *material,
    uint32_t *out_identity)
{
    uint32_t hash = 2166136261u;

    if (out_identity) *out_identity = 0u;
    if (!material || !out_identity || material->object_id == 0xfffeu ||
        material->raw_hash == 0u || material->raw_byte_count == 0u ||
        material->image_width <= 0 || material->image_height <= 0 ||
        material->image_format <= 0 || material->local_palette_hash == 0u) {
        return 0;
    }
    hash = dm2_v1_g1_material_identity_step(hash, material->raw_hash);
    hash = dm2_v1_g1_material_identity_step(hash, material->raw_byte_count);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             material->local_palette_hash);
    hash = dm2_v1_g1_material_identity_step(hash, material->object_id);
    hash = dm2_v1_g1_material_identity_step(hash, material->direction);
    hash = dm2_v1_g1_material_identity_step(hash, material->container_type);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)material->x);
    hash = dm2_v1_g1_material_identity_step(hash, (uint32_t)material->y);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_width);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_height);
    hash = dm2_v1_g1_material_identity_step(hash,
                                             (uint32_t)material->image_format);
    *out_identity = hash ? hash : 1u;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_creature_map_chip_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatRawRead read_raw,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out)
{
    DM2_V1_G1CreatureMapChipRuntimeReceipt candidate;
    int x;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* skproject/SKWIN/c_map.cpp reaches a DB4 Creature through the tile
     * record list, then DME.h Creature::CreatureType() feeds
     * QUERY_DUNGEON_MAP_CHIP_PICT.  A partial G1 corpus may classify DB4
     * roots, but it cannot promote one into a GDAT material request. */
    if (!dm2_v1_dungeon_record_list_traversal_allowed(d) ||
        !d->raw_data || !read_raw || !read_image_metadata ||
        !read_local_palette ||
        d->square_bytes != 1 ||
        map < 0 || map >= d->level_count) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.map = map;
    for (x = 0; x < d->level_widths[map]; ++x) {
        int y;
        for (y = 0; y < d->level_heights[map]; ++y) {
            int first_thing;
            int raw;
            uint16_t root;
            const uint8_t *record;
            const uint8_t *raw_map_chip = NULL;
            uint32_t raw_byte_count = 0u;
            uint8_t creature_type;
            int image_width = 0;
            int image_height = 0;
            int image_format = 0;
            DM2_V1_G1CreatureMapChipMaterial *material;

            raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
            if (raw < 0) return 0;
            if ((raw & 0x10) == 0) continue;
            first_thing = dm2_v1_dungeon_get_first_thing(d, map, x, y);
            if (first_thing < 0) return 0;
            root = (uint16_t)first_thing;
            if (((root >> 10) & 0x0f) != 4) continue;
            if (!dm2_v1_g1_link_has_record_shape(d, root)) return 0;
            record = dm2_v1_dungeon_get_thing_record(d, root, NULL, NULL, NULL);
            if (!record || candidate.material_count >=
                               DM2_V1_G1_CREATURE_MAP_CHIP_MAX) {
                return 0;
            }
            ++candidate.source_creature_root_count;

            /* skproject/SKWIN/DME.h Creature::CreatureType is DB4 b4.
             * QUERY_DUNGEON_MAP_CHIP_PICT asks CREATURES/type dtImage F9
             * for the original map-chip surface. */
            creature_type = record[4];
            if (!read_raw(read_userdata, 0x01, 0x0f, creature_type, 0xf9,
                          &raw_map_chip, &raw_byte_count) ||
                !raw_map_chip || raw_byte_count == 0u) {
                return 0;
            }
            /* The actual GDAT image route must accept the original F9 data
             * before it can become runtime material. Metadata only: decoded
             * pixels are owned and discarded by the boot adapter. */
            if (!read_image_metadata(read_userdata, 0x0f, creature_type,
                                     0xf9, &image_width, &image_height,
                                     &image_format) ||
                image_width <= 0 || image_height <= 0 ||
                (image_format != 3 && image_format != 4 &&
                 image_format != 8 && image_format != 9)) {
                return 0;
            }
            material = &candidate.materials[candidate.material_count++];
            /* QUERY_DUNGEON_MAP_CHIP_PICT pairs CREATURES/type/F9 with
             * QUERY_GDAT_IMAGE_LOCALPAL before DRAW_CHIP_OF_MAGIC_MAP.
             * A DB4 root cannot borrow an interface or another creature's
             * palette merely because its decoded image dimensions match. */
            if (!read_local_palette(read_userdata, 0x0f, creature_type, 0xf9,
                                    material->local_palette16,
                                    &material->local_palette_hash) ||
                material->local_palette_hash == 0u) {
                return 0;
            }
            material->x = x;
            material->y = y;
            material->object_id = root;
            /* DRAW_MAP_CHIP uses Creature::b15_0_1(), not the ObjectID
             * direction bits, to select the view-relative atlas frame. */
            material->direction = (uint8_t)(record[15] & 3u);
            material->creature_type = creature_type;
            material->raw_byte_count = raw_byte_count;
            material->raw_hash = dm2_v1_g1_raw_hash(raw_map_chip,
                                                     raw_byte_count);
            material->image_width = image_width;
            material->image_height = image_height;
            material->image_format = image_format;
            if (material->raw_hash == 0u) return 0;
        }
    }
    candidate.valid = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_weapon_map_chip_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatRawRead read_raw,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1WeaponMapChipRuntimeReceipt *out)
{
    DM2_V1_G1RuntimeMapWeaponReceipt roots;
    DM2_V1_G1WeaponMapChipRuntimeReceipt candidate;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* skproject/SKWIN/SkWinCore.cpp::DRAW_MAP_CHIP asks
     * QUERY_DUNGEON_MAP_CHIP_PICT(QUERY_CLS1_FROM_RECORD,
     * QUERY_CLS2_FROM_RECORD, F9).  The direct DB5 receipt is the only
     * admitted source of those two selectors; no GenericRecord::w0 walk or
     * item-type-only cache can promote a weapon into a drawable material. */
    if (!d || !read_raw || !read_image_metadata || !read_local_palette ||
        !dm2_v1_dungeon_materialize_g1_runtime_map_weapons(d, map, &roots) ||
        !roots.committed || !roots.incomplete_world) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.map = map;
    candidate.source_weapon_root_count = roots.weapon_root_count;
    for (i = 0; i < roots.weapon_root_count; ++i) {
        const DM2_V1_G1DirectWeaponRoot *weapon = &roots.weapons[i];
        const uint8_t *raw_map_chip = NULL;
        uint32_t raw_byte_count = 0u;
        int image_width = 0;
        int image_height = 0;
        int image_format = 0;
        DM2_V1_G1WeaponMapChipMaterial *material;

        if (candidate.material_count >= DM2_V1_G1_WEAPON_MAP_CHIP_MAX ||
            !read_raw(read_userdata, 0x01, 0x10, weapon->item_type, 0xf9,
                      &raw_map_chip, &raw_byte_count) ||
            !raw_map_chip || raw_byte_count == 0u ||
            !read_image_metadata(read_userdata, 0x10, weapon->item_type,
                                 0xf9, &image_width, &image_height,
                                 &image_format) ||
            image_width <= 0 || image_height <= 0 ||
            (image_format != 3 && image_format != 4 &&
             image_format != 8 && image_format != 9)) {
            return 0;
        }
        material = &candidate.materials[candidate.material_count++];
        if (!read_local_palette(read_userdata, 0x10, weapon->item_type, 0xf9,
                                material->local_palette16,
                                &material->local_palette_hash) ||
            material->local_palette_hash == 0u) {
            return 0;
        }
        material->x = weapon->x;
        material->y = weapon->y;
        material->object_id = weapon->object_id;
        material->direction = weapon->direction;
        material->item_type = weapon->item_type;
        material->raw_byte_count = raw_byte_count;
        material->raw_hash = dm2_v1_g1_raw_hash(raw_map_chip,
                                                 raw_byte_count);
        material->image_width = image_width;
        material->image_height = image_height;
        material->image_format = image_format;
        if (material->raw_hash == 0u) return 0;
    }
    candidate.valid = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_dungeon_materialize_g1_container_map_chip_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatRawRead read_raw,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1ContainerMapChipRuntimeReceipt *out)
{
    DM2_V1_G1RuntimeMapContainerReceipt roots;
    DM2_V1_G1ContainerMapChipRuntimeReceipt candidate;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* skproject SkWinCore.cpp::DRAW_MAP_CHIP selects the record-owned class
     * through QUERY_DUNGEON_MAP_CHIP_PICT. For DB9 that is
     * CONTAINERS/ContainerType/F9; w0 and w2 never participate. */
    if (!d || !read_raw || !read_image_metadata || !read_local_palette ||
        !dm2_v1_dungeon_materialize_g1_runtime_map_containers(d, map, &roots) ||
        !roots.committed || !roots.incomplete_world) {
        return 0;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.map = map;
    candidate.source_container_root_count = roots.container_root_count;
    for (i = 0; i < roots.container_root_count; ++i) {
        const DM2_V1_G1DirectContainerRoot *container = &roots.containers[i];
        const uint8_t *raw_map_chip = NULL;
        uint32_t raw_byte_count = 0u;
        int image_width = 0;
        int image_height = 0;
        int image_format = 0;
        DM2_V1_G1ContainerMapChipMaterial *material;

        if (candidate.material_count >= DM2_V1_G1_CONTAINER_MAP_CHIP_MAX ||
            !read_raw(read_userdata, 0x01, 0x14, container->container_type,
                      0xf9, &raw_map_chip, &raw_byte_count) ||
            !raw_map_chip || raw_byte_count == 0u ||
            !read_image_metadata(read_userdata, 0x14, container->container_type,
                                 0xf9, &image_width, &image_height,
                                 &image_format) ||
            image_width <= 0 || image_height <= 0 ||
            (image_format != 3 && image_format != 4 &&
             image_format != 8 && image_format != 9)) {
            return 0;
        }
        material = &candidate.materials[candidate.material_count++];
        if (!read_local_palette(read_userdata, 0x14, container->container_type,
                                0xf9, material->local_palette16,
                                &material->local_palette_hash) ||
            material->local_palette_hash == 0u) {
            return 0;
        }
        material->x = container->x;
        material->y = container->y;
        material->object_id = container->object_id;
        material->direction = container->direction;
        material->container_type = container->container_type;
        material->raw_byte_count = raw_byte_count;
        material->raw_hash = dm2_v1_g1_raw_hash(raw_map_chip, raw_byte_count);
        material->image_width = image_width;
        material->image_height = image_height;
        material->image_format = image_format;
        if (material->raw_hash == 0u) return 0;
    }
    candidate.valid = 1;
    *out = candidate;
    return 1;
}

int dm2_v1_g1_creature_map_chip_matches_decoded_material(
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt,
    int creature_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash)
{
    int i;

    if (!receipt || !receipt->valid || creature_type < 0 ||
        creature_type > 0xff || image_width <= 0 || image_height <= 0 ||
        local_palette_hash == 0u) {
        return 0;
    }
    for (i = 0; i < receipt->material_count; ++i) {
        const DM2_V1_G1CreatureMapChipMaterial *material =
            &receipt->materials[i];
        if (material->creature_type == (uint8_t)creature_type &&
            material->image_width == image_width &&
            material->image_height == image_height &&
            material->local_palette_hash == local_palette_hash) {
            return 1;
        }
    }
    return 0;
}

int dm2_v1_g1_creature_map_chip_matches_decoded_instance(
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt,
    uint16_t object_id,
    int x,
    int y,
    int direction,
    int creature_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash)
{
    int i;

    if (!receipt || !receipt->valid || object_id == 0xfffeu ||
        direction < 0 || direction > 3 ||
        creature_type < 0 || creature_type > 0xff || image_width <= 0 ||
        image_height <= 0 || local_palette_hash == 0u) {
        return 0;
    }
    for (i = 0; i < receipt->material_count; ++i) {
        const DM2_V1_G1CreatureMapChipMaterial *material =
            &receipt->materials[i];
        if (material->object_id == object_id && material->x == x &&
            material->y == y && material->direction == (uint8_t)direction &&
            material->creature_type == (uint8_t)creature_type &&
            material->image_width == image_width &&
            material->image_height == image_height &&
            material->local_palette_hash == local_palette_hash) {
            return 1;
        }
    }
    return 0;
}

int dm2_v1_g1_weapon_map_chip_matches_decoded_instance(
    const DM2_V1_G1WeaponMapChipRuntimeReceipt *receipt,
    uint16_t object_id,
    int x,
    int y,
    int item_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash,
    uint32_t decoded_pixel_hash)
{
    int i;

    if (!receipt || !receipt->valid || object_id == 0xfffeu ||
        item_type < 0 || item_type > 0xff || image_width <= 0 ||
        image_height <= 0 || local_palette_hash == 0u ||
        decoded_pixel_hash == 0u) {
        return 0;
    }
    for (i = 0; i < receipt->material_count; ++i) {
        const DM2_V1_G1WeaponMapChipMaterial *material =
            &receipt->materials[i];
        if (material->object_id == object_id && material->x == x &&
            material->y == y && material->item_type == (uint8_t)item_type &&
            material->image_width == image_width &&
            material->image_height == image_height &&
            material->local_palette_hash == local_palette_hash &&
            material->decoded_pixel_hash == decoded_pixel_hash) {
            return 1;
        }
    }
    return 0;
}

int dm2_v1_g1_container_map_chip_matches_decoded_instance(
    const DM2_V1_G1ContainerMapChipRuntimeReceipt *receipt,
    uint16_t object_id,
    int x,
    int y,
    int container_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash,
    uint32_t decoded_pixel_hash)
{
    int i;

    if (!receipt || !receipt->valid || object_id == 0xfffeu ||
        container_type < 0 || container_type > 3 || image_width <= 0 ||
        image_height <= 0 || local_palette_hash == 0u ||
        decoded_pixel_hash == 0u) {
        return 0;
    }
    for (i = 0; i < receipt->material_count; ++i) {
        const DM2_V1_G1ContainerMapChipMaterial *material =
            &receipt->materials[i];
        if (material->object_id == object_id && material->x == x &&
            material->y == y &&
            material->container_type == (uint8_t)container_type &&
            material->image_width == image_width &&
            material->image_height == image_height &&
            material->local_palette_hash == local_palette_hash &&
            material->decoded_pixel_hash == decoded_pixel_hash) {
            return 1;
        }
    }
    return 0;
}

int dm2_v1_dungeon_stone_room_input_receipt(const DM2_V1_DungeonData *d,int level,int dir,int x,int y,DM2_V1_StoneRoomInputReceipt *out){
 static const int dx[4]={0,1,0,-1},dy[4]={-1,0,1,0};int raw;
 if(!out)return 0;
 memset(out,0,sizeof(*out));if(!d||(dir&~3)!=0)return 0;raw=dm2_v1_dungeon_get_tile_raw(d,level,x,y);if(raw<0)return 0;
 out->dir=dir;out->x=x;out->y=y;out->tile_w2=(uint8_t)raw;out->tile_type=(uint8_t)((uint8_t)raw>>5);out->first_record_link=dm2_v1_dungeon_get_first_thing(d,level,x,y);
 for(int side=0;side<4;++side){out->oriented_bits[side]=(uint8_t)(raw&(1u<<((3-dir-side)&3)));int n=dm2_v1_dungeon_get_tile_raw(d,level,x+dx[(dir+side)&3],y+dy[(dir+side)&3]);if(n<0)return 0;out->neighbor_tile_w2[side]=(uint8_t)n;}out->valid=1;return 1;}

int dm2_v1_dungeon_stone_room_base_cell(const DM2_V1_StoneRoomInputReceipt *in,DM2_V1_StoneRoomBaseCellReceipt *out){if(!out)return 0;
    memset(out,0,sizeof(*out));if(!in||!in->valid)return 0;out->w2=in->tile_w2;out->w0=(uint8_t)(in->tile_w2>>5);memset(out->w6,0xff,sizeof(out->w6));out->valid=1;return 1;}

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

const char *dm2_v1_DM2_ARRANGE_DUNGEON_source_evidence(void) {
    return
        "skproject SKULLWIN/c_map.cpp DM2_ARRANGE_DUNGEON arranges the "
        "loaded dungeon maps before runtime use; Firestaff admits only the "
        "dm2_v1_dungeon_load-proven map descriptors, byte/word square layout, "
        "MapGraphicsStyle values, ground-stack/text table addresses, and "
        "record-graph completion state. PC G1 partial graphs remain marked "
        "incomplete instead of receiving fabricated c_record semantics.";
}

const uint8_t *dm2_v1_dungeon_level_tile_data(
    const DM2_V1_DungeonData *d,
    int level,
    int16_t *out_width,
    int16_t *out_height)
{
    int offset;

    if (!d || !d->raw_data || level < 0 || level >= d->level_count)
        return NULL;
    if (out_width) *out_width = (int16_t)d->level_widths[level];
    if (out_height) *out_height = (int16_t)d->level_heights[level];

    if (d->square_bytes == 1) {
        offset = d->raw_map_data_base + d->level_offsets[level];
    } else {
        offset = DM2_TILE_DATA_START + d->level_offsets[level];
    }
    if (offset < 0 || offset >= d->raw_size)
        return NULL;
    return d->raw_data + offset;
}
