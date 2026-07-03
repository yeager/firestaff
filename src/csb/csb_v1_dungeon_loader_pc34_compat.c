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
        uint16_t square_first_thing_count = rd16(decoded + 10);
        int thing_data_base;
        long raw_map_data_base;

        levels = decoded[4];
        out->level_count = levels;
        out->square_bytes = 1;
        out->square_first_thing_count = (int)square_first_thing_count;

        for (i = 0; i < levels; i++) {
            const uint8_t *map_desc = decoded + CSB_DUNGEON_HEADER_SIZE +
                                      i * CSB_DUNGEON_MAP_DESC_SIZE;
            int level_id = 0;
            uint16_t raw_bit_a = rd16(map_desc + 8);
            csb_decode_map_bitfield_a(raw_bit_a, &level_id,
                                      &out->level_widths[i],
                                      &out->level_heights[i]);
            out->map_levels[i] = level_id;
            out->map_offset_x[i] = (int)map_desc[4];
            out->map_offset_y[i] = (int)map_desc[5];
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
        thing_data_base = out->square_first_thing_base +
                          (int)square_first_thing_count * 2 +
                          (int)text_word_count * 2;
        for (i = 0; i < CSB_THING_TYPE_COUNT; i++) {
            int count = (int)rd16(decoded + 12 + i * 2);
            int byte_count = (int)csb_thing_data_byte_count[i];
            out->thing_data_bases[i] = thing_data_base + (int)thing_data_total;
            out->thing_type_counts[i] = count;
            thing_data_total += (long)count * (long)byte_count;
        }

        out->raw_map_data_base = thing_data_base + (int)thing_data_total;
        raw_map_data_base = out->raw_map_data_base;

        if (raw_map_data_base < 0 || raw_map_data_base >= decoded_size) {
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
    if (byte_count <= 0) return NULL;
    if (index < 0 || index >= d->thing_type_counts[type]) return NULL;

    offset = d->thing_data_bases[type] + index * byte_count;
    if (offset < 0 || offset + byte_count > d->raw_size) return NULL;
    if (out_type) *out_type = type;
    if (out_index) *out_index = index;
    if (out_size) *out_size = byte_count;
    return d->raw_data + offset;
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
        "ReDMCSB DUNGEON.C F0148-F0170 shared format\n"
        "ReDMCSB DUNGEON.C F0160/F0161 square-first-thing table lookup\n"
        "CSB-specific: DSA thing type 15, custom backgrounds\n"
    ;
}
