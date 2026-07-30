/*
 * csb_v1_dungeon_loader_pc34_compat.c
 *
 * pass603: CSB V1 dungeon loader
 *
 * Source-locked to:
 *   CSBWin/CSBCode.cpp: DBank::Initialize (TAG00332a, lines 318-480)
 *   CSBWin/CSBCode.cpp: LoadDungeon (lines 6800-6950)
 *   ReDMCSB DUNGEON.C: F0148_DUNGEON_GetSquareFirstThingType (shared format)
 *   ReDMCSB DUNGEON.C: F0151_DUNGEON_GetSquare
 *   ReDMCSB DUNGEON.C: F0156_DUNGEON_GetThingData
 *
 * CSB dungeon.dat header:
 *   bytes 0-1:  number of levels (LE uint16)
 *   bytes 2-3:  number of thing types (always 16)
 *   per level:  width (uint8), height (uint8), offset (uint32 LE)
 *   then per-level square data at each offset (column-major 2-byte records)
 *   then thing data section
 *   then DSA script section (CSB-specific)
 */

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "dungeon_decompressor_ftl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Current dungeon context (M10 integration) ──────────────────────── */

/*
 * File-scoped singleton: the currently-loaded dungeon.
 * Set by csb_v1_dungeon_load_from_file() and csb_v1_dungeon_set_current().
 * Dungeon-layer accessor stubs (csb_dungeon_get_first_thing_default, etc.)
 * use this context so the world model can service F0161/F0159/F0156
 * calls without needing the dungeon passed in explicitly.
 *
 * ReDMCSB: DUNGEON.C globals G0278_ps_DungeonHeader, G0277_ps_DungeonMaps
 *          (same singleton pattern in the original engine)
 */
static CSB_V1_DungeonData *s_current_dungeon = NULL;
static int s_current_level = 0;  /* current dungeon level for accessor queries */

const CSB_V1_DungeonData *csb_v1_dungeon_get_current(void) {
    return s_current_dungeon;
}

CSB_V1_DungeonData *csb_v1_dungeon_get_current_mutable(void) {
    return s_current_dungeon;
}

void csb_v1_dungeon_set_current(CSB_V1_DungeonData *d) {
    if (s_current_dungeon != d) {
        csb_v1_dungeon_free(s_current_dungeon);
        s_current_dungeon = NULL;
    }
    if (d) {
        s_current_dungeon = d;
        /* Default to level 0 when a new dungeon is loaded */
        s_current_level = 0;
    }
}

void csb_v1_dungeon_unload(void) {
    csb_v1_dungeon_free(s_current_dungeon);
    s_current_dungeon = NULL;
    s_current_level = 0;
}

void csb_v1_dungeon_set_current_level(int level) {
    s_current_level = level;
}

int csb_v1_dungeon_get_current_level(void) {
    return s_current_level;
}

/* ── Helper readers ─────────────────────────────────────────────────── */

static uint16_t rd16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
static uint32_t rd32be(const uint8_t *p) { return ((uint32_t)p[0] << 24) | ((uint32_t)p[1]<<16) | ((uint32_t)p[2]<<8) | (uint32_t)p[3]; }

#define CSB_DUNGEON_HEADER_SIZE 44
#define CSB_DUNGEON_MAP_DESC_SIZE 16
#define CSB_THING_TYPE_COUNT 16

static const unsigned char csb_thing_data_byte_count[CSB_THING_TYPE_COUNT] = {
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

static int csb_thing_data_record_byte_count(int type)
{
    if (type < 0 || type >= CSB_THING_TYPE_COUNT) return 0;
    if (type == 11) {
        /* CSBWin CSB.h DB11: ui16 size + ui32 d[63], total 256 bytes.
         * data.cpp EXPOOL::Setup/enlarge stores DB11 blocks in dbEXPOOL. */
        return 256;
    }
    return (int)csb_thing_data_byte_count[type];
}

static void csb_decode_map_bitfield_a(uint16_t raw, int *level, int *width, int *height) {
    if (level) *level = (int)(raw & 0x3Fu);
    if (width) *width = (int)(((raw >> 6) & 0x1Fu) + 1U);
    if (height) *height = (int)(((raw >> 11) & 0x1Fu) + 1U);
}

static void csb_swap_big_endian_dungeon_words(uint8_t *buf, int size) {
    int pos;
    uint8_t tmp;

    if (!buf || size < 6) return;
    for (pos = 0; pos < 4 && pos + 1 < size; pos += 2) {
        tmp = buf[pos]; buf[pos] = buf[pos + 1]; buf[pos + 1] = tmp;
    }
    /* Header bytes 4..5 are map_count/unreferenced, not a uint16. */
    for (pos = 6; pos + 1 < size; pos += 2) {
        tmp = buf[pos]; buf[pos] = buf[pos + 1]; buf[pos + 1] = tmp;
    }
}

static uint8_t *csb_decode_dungeon_if_needed(const uint8_t *dat, int dat_size, int *out_size) {
    uint8_t *decoded;
    uint32_t decomp_size;
    int big_endian_wrapped;

    if (!dat || dat_size <= 0 || !out_size) return NULL;

    big_endian_wrapped = (dat_size >= 8 && dat[0] == 0x81 && dat[1] == 0x04);
    if (big_endian_wrapped || (dat_size >= 8 && dat[0] == 0x04 && dat[1] == 0x81)) {
        decomp_size = big_endian_wrapped ? rd32be(dat + 2) : rd32(dat + 2);
        if (decomp_size == 0 || decomp_size > 16U * 1024U * 1024U) return NULL;

        decoded = (uint8_t *)calloc(1, (size_t)decomp_size);
        if (!decoded) return NULL;

        /* ReDMCSB DECOMPDU.C F0455: CSB disk DUNGEON.DAT has an 8-byte
         * compressed header; the 20-byte lookup table starts at byte 8. */
        if (!ftl_decompress_dungeon(dat + 8, (size_t)dat_size - 8U,
                                    decoded, (long)decomp_size)) {
            free(decoded);
            return NULL;
        }
        if (big_endian_wrapped) {
            csb_swap_big_endian_dungeon_words(decoded, (int)decomp_size);
        }
        *out_size = (int)decomp_size;
        return decoded;
    }

    decoded = (uint8_t *)malloc((size_t)dat_size);
    if (!decoded) return NULL;
    memcpy(decoded, dat, (size_t)dat_size);
    *out_size = dat_size;
    return decoded;
}

/* ── Core loader ────────────────────────────────────────────────────── */

int csb_v1_dungeon_load(CSB_V1_DungeonData *out, const uint8_t *dat, int dat_size) {
    int i, levels, offset;
    uint8_t *decoded = NULL;
    int decoded_size = 0;

    if (!out || !dat || dat_size < 4) return -1;
    memset(out, 0, sizeof(*out));

    decoded = csb_decode_dungeon_if_needed(dat, dat_size, &decoded_size);
    if (!decoded || decoded_size < 4) {
        free(decoded);
        return -1;
    }

    /* Real CSB PC data after FTL decompression uses the DM1-compatible
     * 44-byte DUNGEON_HEADER and 16-byte MAP descriptor layout.
     * ReDMCSB: DEFS.H DUNGEON_HEADER/MAP and DECOMPDU.C F0455. */
    if (decoded_size >= CSB_DUNGEON_HEADER_SIZE &&
        decoded[4] > 0 &&
        decoded[4] <= CSB_V1_MAX_LEVELS &&
        decoded_size >= CSB_DUNGEON_HEADER_SIZE +
                        decoded[4] * CSB_DUNGEON_MAP_DESC_SIZE) {
        int total_columns = 0;
        long thing_data_total = 0;
        uint16_t text_word_count = rd16(decoded + 6);
        uint16_t initial_party_location = rd16(decoded + 8);
        uint16_t square_first_thing_count = rd16(decoded + 10);
        int thing_data_base;
        long raw_map_data_base;

        levels = decoded[4];
        out->level_count = levels;
        out->initial_party_location = initial_party_location;
        out->square_bytes = 1;
        out->square_first_thing_count = (int)square_first_thing_count;

        for (i = 0; i < levels; i++) {
            const uint8_t *map_desc = decoded + CSB_DUNGEON_HEADER_SIZE +
                                      i * CSB_DUNGEON_MAP_DESC_SIZE;
            int level_id = 0;
            uint16_t raw_bit_a = rd16(map_desc + 8);
            uint16_t raw_bit_c = rd16(map_desc + 12);
            uint16_t raw_bit_d = rd16(map_desc + 14);
            csb_decode_map_bitfield_a(raw_bit_a, &level_id,
                                      &out->level_widths[i],
                                      &out->level_heights[i]);
            out->map_levels[i] = level_id;
            out->map_offset_x[i] = (int)map_desc[4];
            out->map_offset_y[i] = (int)map_desc[5];
            /* ReDMCSB DEFS.H MAP.D for PC/I34E stores FloorSet, WallSet,
             * DoorSet0, DoorSet1 as low-to-high nibbles in the final map
             * descriptor word. F0094/F0095 consume the first two before
             * F0128; F0174 copies the latter pair into CurrentMapDoorInfo. */
            out->map_floor_set[i] = (int)(raw_bit_d & 0x0Fu);
            out->map_wall_set[i] = (int)((raw_bit_d >> 4) & 0x0Fu);
            out->map_door_set0[i] = (int)((raw_bit_d >> 8) & 0x0Fu);
            out->map_door_set1[i] = (int)((raw_bit_d >> 12) & 0x0Fu);
            /* ReDMCSB DEFS.H MAP.C stores DungeonView palette difficulty
             * in the high nibble. PANEL.C F0337 bypasses the normal light
             * calculation entirely for map difficulty zero. */
            out->map_difficulty[i] = (int)((raw_bit_c >> 12) & 0x0Fu);
            out->map_experience_multiplier[i] =
                (int)((raw_bit_c >> 12) & 0x0Fu);
            if (out->level_widths[i] < 1 ||
                out->level_widths[i] > CSB_V1_MAX_SQUARE_SIZE ||
                out->level_heights[i] < 1 ||
                out->level_heights[i] > CSB_V1_MAX_SQUARE_SIZE) {
                csb_v1_dungeon_free(out);
                free(decoded);
                return -2;
            }
            total_columns += out->level_widths[i];
        }

        out->square_first_thing_base = CSB_DUNGEON_HEADER_SIZE +
                                       levels * CSB_DUNGEON_MAP_DESC_SIZE +
                                       total_columns * 2;
        out->text_data_base = out->square_first_thing_base +
                              (int)square_first_thing_count * 2;
        out->text_word_count = (int)text_word_count;
        thing_data_base = out->square_first_thing_base +
                          (int)square_first_thing_count * 2 +
                          (int)text_word_count * 2;
        for (i = 0; i < CSB_THING_TYPE_COUNT; i++) {
            int count = (int)rd16(decoded + 12 + i * 2);
            int byte_count = csb_thing_data_record_byte_count(i);
            out->thing_data_bases[i] = thing_data_base + (int)thing_data_total;
            out->thing_type_counts[i] = count;
            thing_data_total += (long)count * (long)byte_count;
        }

        out->raw_map_data_base = thing_data_base + (int)thing_data_total;
        raw_map_data_base = out->raw_map_data_base;

        if (out->text_data_base < 0 ||
            out->text_data_base > decoded_size ||
            (long)out->text_data_base + (long)out->text_word_count * 2L >
                decoded_size ||
            raw_map_data_base < 0 || raw_map_data_base >= decoded_size) {
            csb_v1_dungeon_free(out);
            free(decoded);
            return -2;
        }

        for (i = 0; i < levels; i++) {
            const uint8_t *map_desc = decoded + CSB_DUNGEON_HEADER_SIZE +
                                      i * CSB_DUNGEON_MAP_DESC_SIZE;
            uint32_t rel_offset = rd16(map_desc);
            uint32_t square_bytes = (uint32_t)out->level_widths[i] *
                                    (uint32_t)out->level_heights[i];
            uint32_t abs_offset = (uint32_t)raw_map_data_base + rel_offset;
            if (abs_offset > (uint32_t)decoded_size ||
                square_bytes > (uint32_t)decoded_size ||
                abs_offset + square_bytes > (uint32_t)decoded_size) {
                csb_v1_dungeon_free(out);
                free(decoded);
                return -2;
            }
            out->level_offsets[i] = (int)abs_offset;
        }

        out->raw_data = decoded;
        out->raw_size = decoded_size;
        return 0;
    }

    /* Legacy synthetic fixture format used by early CSB unit tests. */
    levels = rd16(decoded);
    if (levels > CSB_V1_MAX_LEVELS) levels = CSB_V1_MAX_LEVELS;
    out->level_count = levels;
    out->square_bytes = 2;

    /* CSBWin TAG00332a: level headers start at offset 4 */
    offset = 4;
    for (i = 0; i < levels && offset + 6 <= decoded_size; i++) {
        uint32_t lvl_offset;
        uint32_t square_bytes;
        uint8_t width = decoded[offset];
        uint8_t height = decoded[offset + 1];
        lvl_offset = rd32(decoded + offset + 2);
        square_bytes = (uint32_t)width * (uint32_t)height * 2U;
        /* ReDMCSB DUNGEON.C F0151 reads 16-bit square records from the
         * per-level offset using column-major x*height+y indexing. Reject
         * headers whose square span cannot fit in the supplied buffer. */
        if (lvl_offset > (uint32_t)decoded_size ||
            square_bytes > (uint32_t)decoded_size ||
            lvl_offset + square_bytes > (uint32_t)decoded_size) {
            csb_v1_dungeon_free(out);
            free(decoded);
            return -2;
        }
        out->level_widths[i] = width;
        out->level_heights[i] = height;
        out->level_offsets[i] = (int)lvl_offset;
        out->map_levels[i] = i;
        out->map_offset_x[i] = 0;
        out->map_offset_y[i] = 0;
        out->map_door_set0[i] = 0;
        out->map_door_set1[i] = 0;
        out->map_difficulty[i] = -1;
        out->map_experience_multiplier[i] = 0;
        offset += 6;
    }

    out->raw_data = decoded;
    out->raw_size = decoded_size;

    return 0;
}

/* ── File I/O ───────────────────────────────────────────────────────── */

int csb_v1_dungeon_load_from_file(CSB_V1_DungeonData *out, const char *path) {
    FILE *f;
    uint8_t *buf = NULL;
    long filesize;
    size_t nread;
    int ret = -1;

    if (!out || !path) return -1;
    memset(out, 0, sizeof(*out));

    f = fopen(path, "rb");
    if (!f) return -1;

    if (fseek(f, 0, SEEK_END) != 0) goto done;
    filesize = ftell(f);
    if (filesize <= 0 || filesize > 16 * 1024 * 1024) goto done; /* sanity cap: 16 MB */
    if (fseek(f, 0, SEEK_SET) != 0) goto done;

    buf = (uint8_t *)malloc((size_t)filesize);
    if (!buf) goto done;

    nread = fread(buf, 1, (size_t)filesize, f);
    if (nread != (size_t)filesize) goto done;

    ret = csb_v1_dungeon_load(out, buf, (int)filesize);

done:
    free(buf);
    fclose(f);
    if (ret != 0) memset(out, 0, sizeof(*out));
    return ret;
}

int csb_v1_dungeon_initial_party_pose_pc34(const CSB_V1_DungeonData *d,
                                           int *out_map_index,
                                           int *out_x,
                                           int *out_y,
                                           int *out_direction)
{
    uint16_t location;

    if (!d || !d->raw_data || d->square_bytes != 1 ||
        d->level_count <= 0 || !out_map_index || !out_x || !out_y ||
        !out_direction) {
        return 0;
    }
    location = d->initial_party_location;
    *out_map_index = 0;
    *out_x = (int)(location & 0x001fu);
    *out_y = (int)((location >> 5) & 0x001fu);
    *out_direction = (int)((location >> 10) & 0x0003u);
    if (*out_x >= d->level_widths[0] || *out_y >= d->level_heights[0]) {
        return 0;
    }
    return 1;
}

/* ── Raw square accessors ────────────────────────────────────────────── */

int csb_v1_dungeon_get_raw_square(const CSB_V1_DungeonData *d, int level, int x, int y) {
    int offset, w;
    if (!d || !d->raw_data || level < 0 || level >= d->level_count) return -1;
    w = d->level_widths[level];
    if (x < 0 || x >= w || y < 0 || y >= d->level_heights[level]) return -1;

    if (d->square_bytes == 1) {
        offset = d->level_offsets[level] + (x * d->level_heights[level] + y);
        if (offset >= d->raw_size) return -1;
        return (int)d->raw_data[offset];
    }

    /* Legacy synthetic fixture path: column-major 16-bit records. */
    offset = d->level_offsets[level] + (x * d->level_heights[level] + y) * 2;
    if (offset + 2 > d->raw_size) return -1;
    return (int)rd16(d->raw_data + offset);
}

enum {
    CSB_F0151_ELEMENT_CORRIDOR = 1,
    CSB_F0151_ELEMENT_WALL = 0,
    CSB_F0151_WALL_WEST_RANDOM_ORNAMENT_ALLOWED = 0x01,
    CSB_F0151_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED = 0x02,
    CSB_F0151_WALL_EAST_RANDOM_ORNAMENT_ALLOWED = 0x04,
    CSB_F0151_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED = 0x08
};

static int csb_v1_f0151_real_byte_map_valid(
    const CSB_V1_DungeonData *d, int level)
{
    int width;
    int height;
    int offset;

    if (!d || !d->raw_data || d->square_bytes != 1 ||
        level < 0 || level >= d->level_count) {
        return 0;
    }
    width = d->level_widths[level];
    height = d->level_heights[level];
    offset = d->level_offsets[level];
    if (width <= 0 || height <= 0 || offset < 0 || offset > d->raw_size) {
        return 0;
    }
    return width <= (d->raw_size - offset) / height;
}

static int csb_v1_f0151_square_type(uint8_t square)
{
    return (int)(square >> 5);
}

static int csb_v1_f0151_raw_square(
    const CSB_V1_DungeonData *d, int level, int x, int y)
{
    int offset;
    int height;

    if (!csb_v1_f0151_real_byte_map_valid(d, level)) return -1;
    if (x < 0 || x >= d->level_widths[level] ||
        y < 0 || y >= d->level_heights[level]) {
        return -1;
    }
    height = d->level_heights[level];
    offset = d->level_offsets[level] + x * height + y;
    if (offset < 0 || offset >= d->raw_size) return -1;
    return (int)d->raw_data[offset];
}

int csb_v1_dungeon_f0151_get_square_pc34(
    const CSB_V1_DungeonData *d, int level, int x, int y)
{
    int width;
    int height;
    int adjacent_square;

    if (!csb_v1_f0151_real_byte_map_valid(d, level)) return -1;
    width = d->level_widths[level];
    height = d->level_heights[level];

    adjacent_square = csb_v1_f0151_raw_square(d, level, x, y);
    if (adjacent_square >= 0) return adjacent_square;

    /* ReDMCSB DUNGEON.C F0151 lines 1423-1478.  Its decompiled pit tests
     * use an uninitialized temporary; the source notes they have no visible
     * consequence.  Preserve only the defined adjacent-corridor edge cases. */
    if (y >= 0 && y < height) {
        if (x == -1 &&
            csb_v1_f0151_square_type((uint8_t)csb_v1_f0151_raw_square(
                d, level, 0, y)) == CSB_F0151_ELEMENT_CORRIDOR) {
            return CSB_F0151_WALL_EAST_RANDOM_ORNAMENT_ALLOWED;
        }
        if (x == width &&
            csb_v1_f0151_square_type((uint8_t)csb_v1_f0151_raw_square(
                d, level, width - 1, y)) == CSB_F0151_ELEMENT_CORRIDOR) {
            return CSB_F0151_WALL_WEST_RANDOM_ORNAMENT_ALLOWED;
        }
    } else if (x >= 0 && x < width) {
        if (y == -1 &&
            csb_v1_f0151_square_type((uint8_t)csb_v1_f0151_raw_square(
                d, level, x, 0)) == CSB_F0151_ELEMENT_CORRIDOR) {
            return CSB_F0151_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED;
        }
        if (y == height &&
            csb_v1_f0151_square_type((uint8_t)csb_v1_f0151_raw_square(
                d, level, x, height - 1)) == CSB_F0151_ELEMENT_CORRIDOR) {
            return CSB_F0151_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED;
        }
    }

    return CSB_F0151_ELEMENT_WALL << 5;
}

static int csb_v1_f0150_update_relative_coordinates(
    int direction, int steps_forward, int steps_right, int *map_x, int *map_y)
{
    static const int direction_to_east_count[4] = { 0, 1, 0, -1 };
    static const int direction_to_north_count[4] = { -1, 0, 1, 0 };
    int right_direction;

    if (!map_x || !map_y || direction < 0 || direction > 3) return -1;
    *map_x += direction_to_east_count[direction] * steps_forward;
    *map_y += direction_to_north_count[direction] * steps_forward;
    right_direction = (direction + 1) & 3;
    *map_x += direction_to_east_count[right_direction] * steps_right;
    *map_y += direction_to_north_count[right_direction] * steps_right;
    return 0;
}

int csb_v1_dungeon_f0150_get_relative_location_pc34(
    int direction, int steps_forward, int steps_right,
    int map_x, int map_y, int *out_map_x, int *out_map_y)
{
    if (!out_map_x || !out_map_y ||
        csb_v1_f0150_update_relative_coordinates(
            direction, steps_forward, steps_right, &map_x, &map_y) != 0) {
        return -1;
    }
    *out_map_x = map_x;
    *out_map_y = map_y;
    return 0;
}

int csb_v1_dungeon_f0152_get_relative_square_pc34(
    const CSB_V1_DungeonData *d, int level, int direction,
    int steps_forward, int steps_right, int map_x, int map_y)
{
    if (csb_v1_f0150_update_relative_coordinates(
            direction, steps_forward, steps_right, &map_x, &map_y) != 0) {
        return -1;
    }
    return csb_v1_dungeon_f0151_get_square_pc34(d, level, map_x, map_y);
}

int csb_v1_dungeon_f0153_get_relative_square_type_pc34(
    const CSB_V1_DungeonData *d, int level, int direction,
    int steps_forward, int steps_right, int map_x, int map_y)
{
    int square = csb_v1_dungeon_f0152_get_relative_square_pc34(
        d, level, direction, steps_forward, steps_right, map_x, map_y);
    if (square < 0) return -1;
    return csb_v1_f0151_square_type((uint8_t)square);
}

uint16_t csb_v1_dungeon_f0159_get_next_thing_pc34(
    const CSB_V1_DungeonData *d, uint16_t thing)
{
    const uint8_t *record;
    int size;

    if (!d || d->square_bytes != 1 || thing == 0xfffeu || thing == 0xffffu) {
        return 0xfffeu;
    }
    record = csb_v1_dungeon_get_thing_record(d, thing, NULL, NULL, &size);
    if (!record || size < 2) return 0xfffeu;

    /* ReDMCSB DUNGEON.C F0159 lines 1664-1676: GENERIC.Next is the first
     * little-endian word of every concrete Thing record. */
    return rd16(record);
}

uint16_t csb_v1_dungeon_f0162_get_square_first_object_pc34(
    const CSB_V1_DungeonData *d, int level, int x, int y)
{
    uint16_t thing;
    int first_thing;
    int maximum_links = 0;
    int type;

    if (!d || d->square_bytes != 1) return 0xfffeu;
    for (type = 0; type < CSB_THING_TYPE_COUNT; ++type) {
        if (d->thing_type_counts[type] < 0 ||
            maximum_links > 0x7fff - d->thing_type_counts[type]) {
            return 0xfffeu;
        }
        maximum_links += d->thing_type_counts[type];
    }
    if (maximum_links == 0) return 0xfffeu;

    /* ReDMCSB DUNGEON.C F0162 lines 1748-1760: start at F0161's square
     * head and advance F0159 while the Thing type is below C04_GROUP. */
    first_thing = csb_v1_dungeon_get_first_thing(d, level, x, y);
    if (first_thing < 0) return 0xfffeu;
    thing = (uint16_t)first_thing;
    while (thing != 0xfffeu && maximum_links-- > 0) {
        if (((thing >> 10) & 0x0fu) >= CSB_V1_THING_TYPE_GROUP) return thing;
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(d, thing);
    }
    return 0xfffeu;
}

int csb_v1_dungeon_f0154_get_location_after_level_change_pc34(
    const CSB_V1_DungeonData *d, int map_index, int level_delta,
    int *inout_map_x, int *inout_map_y)
{
    int global_x;
    int global_y;
    int target_level;
    int target_map;

    if (!d || d->square_bytes != 1 || !inout_map_x || !inout_map_y ||
        map_index < 0 || map_index >= d->level_count) {
        return -1;
    }

    /* ReDMCSB DUNGEON.C F0154 lines 1508-1554: convert through the source
     * map's global offsets, then find a target MAP on the requested source
     * level covering that same global coordinate. */
    global_x = d->map_offset_x[map_index] + *inout_map_x;
    global_y = d->map_offset_y[map_index] + *inout_map_y;
    target_level = d->map_levels[map_index] + level_delta;
    for (target_map = 0; target_map < d->level_count; ++target_map) {
        int min_x = d->map_offset_x[target_map];
        int min_y = d->map_offset_y[target_map];
        int max_x = min_x + d->level_widths[target_map] - 1;
        int max_y = min_y + d->level_heights[target_map] - 1;

        if (d->level_widths[target_map] <= 0 ||
            d->level_heights[target_map] <= 0 ||
            d->map_levels[target_map] != target_level ||
            global_x < min_x || global_x > max_x ||
            global_y < min_y || global_y > max_y) {
            continue;
        }
        *inout_map_x = global_x - min_x;
        *inout_map_y = global_y - min_y;
        return target_map;
    }
    return -1;
}

int csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
    const CSB_V1_DungeonData *d, int level, int map_x, int map_y)
{
    int square;
    int north_south_oriented;
    int exit_x;
    int exit_y;
    int exit_type;

    square = csb_v1_dungeon_f0151_get_square_pc34(d, level, map_x, map_y);
    if (square < 0) return -1;

    /* ReDMCSB DUNGEON.C F0155 lines 1560-1582. */
    north_south_oriented = (square & 0x08) == 0;
    exit_x = map_x + (north_south_oriented ? 1 : 0);
    exit_y = map_y + (north_south_oriented ? 0 : -1);
    exit_type = csb_v1_dungeon_f0153_get_relative_square_type_pc34(
        d, level, 0, 0, 0, exit_x, exit_y);
    if (exit_type < 0) return -1;
    return (((exit_type == 0 || exit_type == 3) ? 1 : 0) << 1) |
           north_south_oriented;
}

int csb_v1_dungeon_get_square_type(const CSB_V1_DungeonData *d, int level, int x, int y) {
    int v = csb_v1_dungeon_get_raw_square(d, level, x, y);
    if (v < 0) return -1;
    return (d && d->square_bytes == 1) ? ((v >> 5) & 0x07) : (v & 0x1F);
}

int csb_v1_dungeon_get_first_thing(const CSB_V1_DungeonData *d, int level, int x, int y) {
    int v = csb_v1_dungeon_get_raw_square(d, level, x, y);
    if (v < 0) return -1;
    if (d && d->square_bytes == 1) {
        int i;
        int column_index = 0;
        int column_counts_base;
        int thing_index;
        int thing_offset;
        int square_offset;

        if ((v & 0x10) == 0) return -1;
        column_counts_base = CSB_DUNGEON_HEADER_SIZE +
                             d->level_count * CSB_DUNGEON_MAP_DESC_SIZE;
        for (i = 0; i < level; i++) {
            column_index += d->level_widths[i];
        }
        column_counts_base += (column_index + x) * 2;
        if (column_counts_base + 2 > d->raw_size) return -1;

        /* ReDMCSB DUNGEON.C F0160:1699-1728 starts from
         * G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount[x],
         * then counts MASK0x0010_THING_LIST_PRESENT in earlier rows of the
         * same column. F0161:1730-1746 returns G0283_pT_SquareFirstThings[index]. */
        thing_index = (int)rd16(d->raw_data + column_counts_base);
        square_offset = d->level_offsets[level] + x * d->level_heights[level];
        for (i = 0; i < y; i++) {
            if (square_offset + i >= d->raw_size) return -1;
            if (d->raw_data[square_offset + i] & 0x10u) thing_index++;
        }
        if (thing_index < 0 || thing_index >= d->square_first_thing_count) return -1;

        thing_offset = d->square_first_thing_base + thing_index * 2;
        if (thing_offset + 2 > d->raw_size) return -1;
        return (int)rd16(d->raw_data + thing_offset);
    }
    return ((v >> 5) & 0x3FF);
}

const uint8_t *csb_v1_dungeon_get_thing_record(
    const CSB_V1_DungeonData *d,
    uint16_t thing,
    int *out_type,
    int *out_index,
    int *out_size)
{
    int type;
    int index;
    int byte_count;
    int offset;

    if (out_type) *out_type = -1;
    if (out_index) *out_index = -1;
    if (out_size) *out_size = 0;
    if (!d || !d->raw_data) return NULL;

    /* ReDMCSB: DEFS.H lines 394-402 encodes THING as cell/type/index;
     * DUNGEON.C F0156 indexes G0284_apuc_ThingData[M012_TYPE(thing)] by
     * M013_INDEX(thing). */
    type = (int)((thing & 0x3C00u) >> 10);
    index = (int)(thing & 0x03FFu);
    if (type < 0 || type >= CSB_THING_TYPE_COUNT) return NULL;
    byte_count = (int)csb_thing_data_byte_count[type];
    if (type == 11) {
        byte_count = csb_thing_data_record_byte_count(type);
    }
    if (byte_count <= 0) return NULL;
    if (index < 0 || index >= d->thing_type_counts[type]) return NULL;

    offset = d->thing_data_bases[type] + index * byte_count;
    if (offset < 0 || offset + byte_count > d->raw_size) return NULL;
    if (out_type) *out_type = type;
    if (out_index) *out_index = index;
    if (out_size) *out_size = byte_count;
    return d->raw_data + offset;
}

uint16_t csb_v1_dungeon_f0166_get_unused_thing_pc34(
    CSB_V1_DungeonData *d, uint16_t requested_thing_type,
    CSB_V1_DungeonDiscardThingPc34Compat discard_thing, void *discard_user)
{
    const uint16_t type_mask = 0x7fffu;
    const uint16_t champion_bones = 0x8000u;
    const uint16_t thing_none = 0xffffu;
    const uint16_t thing_end = 0xfffeu;
    uint16_t thing_type = requested_thing_type & type_mask;
    int thing_count;
    int record_size;
    int index;
    uint16_t thing = thing_none;
    uint8_t *record = NULL;
    int actual_type;
    int actual_index;
    int actual_size;

    if (!d || !d->raw_data || d->square_bytes != 1 ||
        thing_type >= CSB_THING_TYPE_COUNT ||
        ((requested_thing_type & champion_bones) != 0 && thing_type != 10)) {
        return thing_none;
    }
    thing_count = d->thing_type_counts[thing_type];
    record_size = csb_thing_data_record_byte_count((int)thing_type);
    if (thing_count <= 0 || record_size <= 0 ||
        d->thing_data_bases[thing_type] < 0) {
        return thing_none;
    }

    /* ReDMCSB DUNGEON.C F0166 lines 2085-2127: ordinary JUNK allocation
     * excludes the final three source records, reserved for champion bones. */
    if ((requested_thing_type & champion_bones) == 0 && thing_type == 10) {
        if (thing_count <= 3) return thing_none;
        thing_count -= 3;
    }
    for (index = 0; index < thing_count; ++index) {
        int offset = d->thing_data_bases[thing_type] + index * record_size;
        if (offset < 0 || offset > d->raw_size - record_size) return thing_none;
        if (rd16(d->raw_data + offset) == thing_none) {
            thing = (uint16_t)((thing_type << 10) | (uint16_t)index);
            record = d->raw_data + offset;
            break;
        }
    }
    if (!record && discard_thing) {
        thing = discard_thing(thing_type, discard_user);
        if (thing == thing_none || ((thing >> 10) & 0x0fu) != thing_type) {
            return thing_none;
        }
        record = (uint8_t *)csb_v1_dungeon_get_thing_record(
            d, thing, &actual_type, &actual_index, &actual_size);
        if (!record || actual_type != thing_type || actual_index < 0 ||
            actual_size != record_size) {
            return thing_none;
        }
    }
    if (!record) return thing_none;

    memset(record, 0, (size_t)record_size);
    record[0] = (uint8_t)(thing_end & 0xffu);
    record[1] = (uint8_t)(thing_end >> 8);
    return thing;
}

int csb_v1_dungeon_decode_dsa_filter_location(
    const CSB_V1_DungeonData *d, uint32_t record_word, int movement_filter,
    CSB_V1_DSAFilterLocation *out)
{
    /* CSBWin DSA.cpp LOCATIONREL::Integer (lines 292-308) packs p/l/x/y;
     * Monster.cpp MONSTERMOVEFILTERCACHE::GetLocation (3084-3096) overlays
     * bit 18 and bits 19..23 for the movement filter metadata. */
    int level = (int)((record_word >> 10) & 0x3fu);
    int x = (int)((record_word >> 5) & 0x1fu);
    int y = (int)(record_word & 0x1fu);

    if (!d || !out || level < 0 || level >= d->level_count ||
        x >= d->level_widths[level] || y >= d->level_heights[level]) return 0;
    out->level = level;
    out->x = x;
    out->y = y;
    out->position = (int)((record_word >> 16) & 3u);
    out->party_level_only = movement_filter ?
        (int)((record_word >> 18) & 1u) : 0;
    out->max_distance = movement_filter ?
        (int)((record_word >> 19) & 0x1fu) : 0;
    out->actuator_thing = 0xffffu;
    return 1;
}

static int csb_v1_dungeon_find_dsa_filter_actuator(
    const CSB_V1_DungeonData *d, CSB_V1_DSAFilterLocation *location)
{
    int thing;
    int limit = 0;
    int type;
    int index;
    int size;
    const uint8_t *record;

    /* CSBWin Monster.cpp GetLocation (3098-3119, 3138-3157) chooses the
     * first dbACTUATOR whose DB3 actuatorType is 47. */
    if (!d || !location) return 0;
    for (type = 0; type < CSB_THING_TYPE_COUNT; ++type) limit += d->thing_type_counts[type];
    thing = csb_v1_dungeon_get_first_thing(d, location->level,
                                             location->x, location->y);
    while (thing >= 0 && thing != 0xfffe && limit-- > 0) {
        record = csb_v1_dungeon_get_thing_record(d, (uint16_t)thing,
                                                   &type, &index, &size);
        (void)index;
        if (!record || size < 4) return 0;
        if (type == CSB_V1_THING_TYPE_ACTUATOR &&
            (rd16(record + 2) & 0x007fu) == CSB_V1_DSA_FILTER_ACTUATOR_TYPE) {
            location->actuator_thing = (uint16_t)thing;
            return 1;
        }
        thing = (int)rd16(record);
    }
    return 0;
}

int csb_v1_dungeon_resolve_dsa_filter_location(
    const CSB_V1_DungeonData *d, int level, int movement_filter,
    CSB_V1_DSAFilterLocation *out)
{
    const uint8_t *bytes;
    size_t size;
    uint32_t key;
    uint32_t word;

    if (!d || !out || (movement_filter &&
        (level < 0 || level >= d->level_count))) return 0;
    key = (CSB_V1_EXPOOL_EDT_SPECIAL_LOCATIONS << 24) |
        (movement_filter ? CSB_V1_EXPOOL_ESL_MONSTER_MOVE_FILTER :
                           CSB_V1_EXPOOL_ESL_MONSTER_ATTACK_FILTER);
    if (movement_filter) key |= (uint32_t)(level + 1) << 8;
    if (!csb_v1_dungeon_expool_locate_record(d, key, &bytes, &size) && movement_filter) {
        key = (CSB_V1_EXPOOL_EDT_SPECIAL_LOCATIONS << 24) |
            CSB_V1_EXPOOL_ESL_MONSTER_MOVE_FILTER;
        if (!csb_v1_dungeon_expool_locate_record(d, key, &bytes, &size)) return 0;
    }
    if (!bytes || size < 4) return 0;
    word = rd32(bytes);
    if (!csb_v1_dungeon_decode_dsa_filter_location(d, word, movement_filter, out)) return 0;
    return csb_v1_dungeon_find_dsa_filter_actuator(d, out);
}

int csb_v1_dungeon_resolve_dsa_special_location(
    const CSB_V1_DungeonData *d, uint8_t special_location,
    CSB_V1_DSAFilterLocation *out)
{
    const uint8_t *bytes;
    size_t size;
    uint32_t key;

    if (!d || !out) return 0;
    key = (CSB_V1_EXPOOL_EDT_SPECIAL_LOCATIONS << 24) |
        (uint32_t)special_location;
    if (!csb_v1_dungeon_expool_locate_record(d, key, &bytes, &size) ||
        !bytes || size < sizeof(uint32_t) ||
        !csb_v1_dungeon_decode_dsa_filter_location(
            d, rd32(bytes), 0, out)) {
        return 0;
    }
    return csb_v1_dungeon_find_dsa_filter_actuator(d, out);
}

static uint32_t csb_v1_dungeon_read_le32_at_word(
    const CSB_V1_DungeonData *d,
    int word_index)
{
    int offset;

    if (!d || !d->raw_data || word_index < 0) return 0u;
    offset = d->thing_data_bases[11] + word_index * 4;
    if (offset < 0 || offset + 4 > d->raw_size) return 0u;
    return rd32(d->raw_data + offset);
}

static uint16_t csb_v1_dungeon_read_le16_at_db11_offset(
    const CSB_V1_DungeonData *d,
    int byte_offset)
{
    int offset;

    if (!d || !d->raw_data || byte_offset < 0) return 0u;
    offset = d->thing_data_bases[11] + byte_offset;
    if (offset < 0 || offset + 2 > d->raw_size) return 0u;
    return rd16(d->raw_data + offset);
}

int csb_v1_dungeon_expool_locate_record(
    const CSB_V1_DungeonData *d,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size)
{
    uint32_t hash;
    uint32_t hashi;
    uint32_t bucket;
    uint32_t p;
    uint32_t size_words;
    int total_words;
    int guard;

    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!d || !d->raw_data || d->thing_type_counts[11] <= 0 ||
        d->thing_data_bases[11] < 0) {
        return 0;
    }
    total_words = d->thing_type_counts[11] * 64;
    if (d->thing_data_bases[11] + total_words * 4 > d->raw_size) {
        return 0;
    }

    /* CSBWin data.cpp EXPOOL::Locate: hash = key * 0xbb40e62d,
     * bucket = 32 + top 5 hash bits, optional secondary table when the
     * high bit is set, then linked record nodes at word offsets. */
    hash = record_id * 0xbb40e62du;
    hashi = 32u + (hash >> 27);
    if (hashi >= (uint32_t)total_words) return 0;
    bucket = csb_v1_dungeon_read_le32_at_word(d, (int)hashi);
    if ((bucket & 0x80000000u) != 0u) {
        hashi = (bucket & 0x7fffffffu) + ((hash >> 21) & 0x3fu);
        if (hashi >= (uint32_t)total_words) return 0;
        bucket = csb_v1_dungeon_read_le32_at_word(d, (int)hashi);
    }

    for (guard = 0; guard < total_words && bucket != 0u; ++guard) {
        p = bucket;
        if (p + 2u > (uint32_t)total_words) return 0;
        if (csb_v1_dungeon_read_le32_at_word(d, (int)(p + 1u)) ==
            record_id) {
            uint32_t block_base = p & 0xffffffc0u;
            int payload_offset;
            if (block_base >= (uint32_t)total_words) return 0;
            size_words = csb_v1_dungeon_read_le16_at_db11_offset(
                d, (int)(block_base * 4u + 2u));
            if (size_words < 2u || p + size_words > (uint32_t)total_words) {
                return 0;
            }
            payload_offset = d->thing_data_bases[11] + (int)(p + 2u) * 4;
            if (payload_offset < 0 ||
                payload_offset + (int)((size_words - 2u) * 4u) >
                    d->raw_size) {
                return 0;
            }
            if (out_bytes) *out_bytes = d->raw_data + payload_offset;
            if (out_size) *out_size = (size_t)(size_words - 2u) * 4u;
            return 1;
        }
        bucket = csb_v1_dungeon_read_le32_at_word(d, (int)p);
    }
    return 0;
}

int csb_v1_dungeon_skin_cache_record_lookup(
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size,
    void *user)
{
    return csb_v1_dungeon_expool_locate_record(
        (const CSB_V1_DungeonData *)user,
        record_id,
        out_bytes,
        out_size);
}

/* ── Square decoding ─────────────────────────────────────────────────── */

/*
 * Decode a raw 16-bit square record into component fields.
 *
 * Square record layout (DUNGEON.C F0151, DEFS.H M034/M035):
 *   bits 15-10: unused / random ornament seed bits
 *   bits  9-5:  first thing index (M012_TYPE encoding)
 *   bit   4:     THING_LIST_PRESENT (MASK0x0010)
 *   bits  3-0:   type-specific flags / square type in WALL context
 *
 *   Square type = raw >> 5 = raw & 0x1F  (M034_SQUARE_TYPE macro)
 *
 * For WALL squares (type 0), bits 3-0 carry random ornament flags:
 *   bit 0: west  wall random ornament
 *   bit 1: south wall random ornament
 *   bit 2: east  wall random ornament
 *   bit 3: north wall random ornament
 *
 * ReDMCSB: DUNGEON.C F0151 lines 1423-1475, DEFS.H M034_M035,
 *          BugsAndChanges.htm:BUG0_10 (bit15 sensitivity in M012_TYPE)
 */
void csb_v1_dungeon_decode_square(uint16_t raw, CSB_V1_DecodedSquare *out) {
    if (!out) return;
    memset(out, 0, sizeof(*out));
    out->type        = (uint8_t)(raw & 0x1Fu);
    out->flags       = (uint8_t)(raw & 0x1Fu);
    out->first_thing = (uint16_t)((raw >> 5) & 0x3FFu);
    out->has_things  = (raw & 0x10u) ? 1 : 0;
}

int csb_v1_dungeon_decode_tile(const CSB_V1_DungeonData *d, int level, int x, int y,
                                CSB_V1_DecodedSquare *out) {
    int raw_val;
    if (!out) return -1;
    raw_val = csb_v1_dungeon_get_raw_square(d, level, x, y);
    if (raw_val < 0) return -1;
    csb_v1_dungeon_decode_square((uint16_t)raw_val, out);
    return 0;
}

/* ── Cleanup ─────────────────────────────────────────────────────────── */

void csb_v1_dungeon_free(CSB_V1_DungeonData *d) {
    if (d && d->raw_data) { free(d->raw_data); d->raw_data = NULL; }
    if (d) {
        d->raw_size = 0;
        d->level_count = 0;
        d->dsa_count = 0;
        if (d->dsa_offsets) { free(d->dsa_offsets); d->dsa_offsets = NULL; }
    }
}

const char *csb_v1_dungeon_source_evidence(void) {
    return
        "CSBWin/CSBCode.cpp:318-480 DBank::Initialize TAG00332a\n"
        "CSBWin/CSBCode.cpp:6800-6950 LoadDungeon\n"
        "CSBWin CSB.h DB11 + data.cpp EXPOOL::Locate DB11 hash records\n"
        "ReDMCSB DUNGEON.C F0148-F0170 shared format\n"
        "ReDMCSB DUNGEON.C F0160/F0161 square-first-thing table lookup\n"
        "CSB-specific: DSA thing type 15, custom backgrounds\n"
    ;
}
