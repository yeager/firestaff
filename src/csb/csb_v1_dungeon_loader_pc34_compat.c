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

/* ── Core loader ────────────────────────────────────────────────────── */

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

    /* ReDMCSB DUNGEON.C F0151: column-major x*height+y, 2 bytes per square */
    offset = d->level_offsets[level] + (x * d->level_heights[level] + y) * 2;
    if (offset + 2 > d->raw_size) return -1;
    return (int)rd16(d->raw_data + offset);
}

int csb_v1_dungeon_get_square_type(const CSB_V1_DungeonData *d, int level, int x, int y) {
    int v = csb_v1_dungeon_get_raw_square(d, level, x, y);
    return v < 0 ? -1 : (v & 0x1F);
}

int csb_v1_dungeon_get_first_thing(const CSB_V1_DungeonData *d, int level, int x, int y) {
    int v = csb_v1_dungeon_get_raw_square(d, level, x, y);
    return v < 0 ? -1 : ((v >> 5) & 0x3FF);
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
        "CSB-specific: DSA thing type 15, custom backgrounds\n"
    ;
}
