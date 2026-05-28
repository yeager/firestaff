
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

/* pass603: CSB V1 dungeon loader
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
 *   bytes 2-3:  number of thing types
 *   per level:  width (uint8), height (uint8), offset (uint32 LE)
 */

static uint16_t rd16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }

int csb_v1_dungeon_load(CSB_V1_DungeonData *out, const uint8_t *dat, int dat_size) {
    int i, levels, offset;
    if (!out || !dat || dat_size < 4) return -1;
    memset(out, 0, sizeof(*out));

    levels = rd16(dat);
    if (levels > CSB_V1_MAX_LEVELS) levels = CSB_V1_MAX_LEVELS;
    out->level_count = levels;

    /* CSBWin TAG00332a: level headers start at offset 4 */
    offset = 4;
    for (i = 0; i < levels && offset + 6 <= dat_size; i++) {
        uint32_t lvl_offset;
        uint32_t square_bytes;
        uint8_t width = dat[offset];
        uint8_t height = dat[offset + 1];
        lvl_offset = rd32(dat + offset + 2);
        square_bytes = (uint32_t)width * (uint32_t)height * 2U;
        /* ReDMCSB DUNGEON.C F0151 reads 16-bit square records from the
         * per-level offset using column-major x*height+y indexing. Reject
         * headers whose square span cannot fit in the supplied buffer. */
        if (lvl_offset > (uint32_t)dat_size ||
            square_bytes > (uint32_t)dat_size ||
            lvl_offset + square_bytes > (uint32_t)dat_size) {
            csb_v1_dungeon_free(out);
            return -2;
        }
        out->level_widths[i] = width;
        out->level_heights[i] = height;
        out->level_offsets[i] = (int)lvl_offset;
        offset += 6;
    }

    /* Store raw data reference */
    out->raw_data = (uint8_t *)malloc(dat_size);
    if (out->raw_data) {
        memcpy(out->raw_data, dat, dat_size);
        out->raw_size = dat_size;
    }

    return 0;
}

int csb_v1_dungeon_get_square_type(const CSB_V1_DungeonData *d, int level, int x, int y) {
    int offset, w;
    uint16_t square;
    if (!d || !d->raw_data || level < 0 || level >= d->level_count) return -1;
    w = d->level_widths[level];
    if (x < 0 || x >= w || y < 0 || y >= d->level_heights[level]) return -1;

    /* ReDMCSB DUNGEON.C F0151: column-major: index = x * height + y */
    offset = d->level_offsets[level] + (x * d->level_heights[level] + y) * 2;
    if (offset + 2 > d->raw_size) return -1;
    square = rd16(d->raw_data + offset);
    return square & 0x1F; /* low 5 bits = square type */
}

int csb_v1_dungeon_get_first_thing(const CSB_V1_DungeonData *d, int level, int x, int y) {
    int offset, w;
    uint16_t square;
    if (!d || !d->raw_data || level < 0 || level >= d->level_count) return -1;
    w = d->level_widths[level];
    if (x < 0 || x >= w || y < 0 || y >= d->level_heights[level]) return -1;

    offset = d->level_offsets[level] + (x * d->level_heights[level] + y) * 2;
    if (offset + 2 > d->raw_size) return -1;
    square = rd16(d->raw_data + offset);
    return (square >> 5) & 0x3FF; /* bits 5-14 = first thing index */
}

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
        "CSB-specific: DSA thing type 15, custom backgrounds\n";
}
