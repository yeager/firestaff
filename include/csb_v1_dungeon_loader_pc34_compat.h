
#ifndef FIRESTAFF_CSB_V1_DUNGEON_LOADER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_DUNGEON_LOADER_PC34_COMPAT_H

#include <stdint.h>

/* CSB dungeon.dat shares the DM1 format with extensions:
 * - More dungeon levels (12 vs DM1's 14, but different layout)
 * - DSA (Dungeon Scripting Architecture) embedded scripts
 * - Custom background references per room
 * - Extended creature type table
 *
 * Source: CSBWin/CSBCode.cpp DBank::Initialize (TAG00332a)
 * Secondary: ReDMCSB DUNGEON.C F0148-F0170 (shared format)
 *
 * Dungeon file layout (CSB PC 3.4):
 *   bytes 0-1:  number of levels (LE uint16)
 *   bytes 2-3:  number of thing types (LE uint16, always 16)
 *   per level (6 bytes): width(u8), height(u8), offset(uint32 LE)
 *   then per-level square data at each offset (column-major 2-byte squares)
 *   then thing data section
 *   then DSA script section (CSB-specific)
 */

#define CSB_V1_MAX_LEVELS 12
#define CSB_V1_MAX_SQUARE_SIZE 32
#define CSB_V1_THING_TYPE_GROUP 4
#define CSB_V1_THING_TYPE_DSA 15  /* CSB-specific: DSA script thing */

typedef struct {
    int level_count;
    int level_offsets[CSB_V1_MAX_LEVELS];
    int level_widths[CSB_V1_MAX_LEVELS];
    int level_heights[CSB_V1_MAX_LEVELS];
    uint8_t *raw_data;
    int raw_size;
    /* DSA scripts */
    int dsa_count;
    uint16_t *dsa_offsets;
} CSB_V1_DungeonData;

/* ── Loader API ─────────────────────────────────────────────────────── */

/* Load dungeon from an in-memory buffer.
 * Parses the CSB dungeon.dat header and validates level offsets.
 * Returns 0 on success, -1 on invalid arguments, -2 on bounds error.
 * Caller must call csb_v1_dungeon_free() to release memory.
 *
 * Source: CSBWin/CSBCode.cpp TAG00332a lines 318-480 */
int csb_v1_dungeon_load(CSB_V1_DungeonData *out, const uint8_t *dat, int dat_size);

/* Load dungeon from a file path.
 * Reads the entire file into memory, then delegates to csb_v1_dungeon_load().
 * Returns 0 on success, -1 on file open error, -2 on parse error.
 * Caller must call csb_v1_dungeon_free() to release memory.
 *
 * Source: CSBWin/CSBCode.cpp LoadDungeon lines 6800-6950 */
int csb_v1_dungeon_load_from_file(CSB_V1_DungeonData *out, const char *path);

/* ── Raw square accessors ──────────────────────────────────────────── */

/* Return the square type (low 5 bits of the 16-bit square record).
 * Returns -1 if d is NULL, raw_data is NULL, or coordinates out of bounds.
 *
 * Square record layout (column-major, 2 bytes):
 *   bits 15-10: unused
 *   bits 9-5:   first thing index  (CSB_THING_INDEX(thing) = square >> 5)
 *   bits 4-0:   square type (0-31)
 *
 * ReDMCSB: DUNGEON.C F0151_DUNGEON_GetSquare (lines 1423-1475)
 *           M034_SQUARE_TYPE macro
 */
int csb_v1_dungeon_get_square_type(const CSB_V1_DungeonData *d, int level, int x, int y);

/* Return the first thing index stored in the square record.
 * Returns -1 if d is NULL, raw_data is NULL, or coordinates out of bounds.
 *
 * ReDMCSB: DUNGEON.C F0151 lines 1423-1475 (bits 5-14 of square record)
 */
int csb_v1_dungeon_get_first_thing(const CSB_V1_DungeonData *d, int level, int x, int y);

/* Return the raw 16-bit square record at a given position.
 * Returns -1 on error, or the 16-bit value on success.
 *
 * ReDMCSB: DUNGEON.C F0151 (column-major, 2-byte records)
 */
int csb_v1_dungeon_get_raw_square(const CSB_V1_DungeonData *d, int level, int x, int y);

/* ── Tile decoding ─────────────────────────────────────────────────── */

/* Decode a raw square record into component fields.
 * Does NOT populate wall/ornament/creature fields (those come from
 * runtime world model integration, not the raw dungeon file).
 *
 * Raw square record (16-bit):
 *   low 5 bits  = square type   (CSB_ELEMENT_*)
 *   bits 5-14   = first thing index
 *
 * ReDMCSB: DUNGEON.C F0151 / M034_SQUARE_TYPE / M035_SQUARE
 */
typedef struct {
    uint8_t  type;        /* square element type (CSB_ELEMENT_*) */
    uint8_t  flags;       /* low 5 bits of square record, type-specific */
    uint16_t first_thing; /* thing index (bits 5-14 of raw record) */
    int      has_things;  /* 1 if first_thing != 0 */
} CSB_V1_DecodedSquare;

void csb_v1_dungeon_decode_square(uint16_t raw, CSB_V1_DecodedSquare *out);

/* Decode a square and return decoded tile data for a given position.
 * Returns 0 on success, -1 if d is NULL or coordinates out of bounds.
 *
 * This is the M10 integration bridge: decodes raw dungeon data into
 * a form usable by the world model without DM1-only assumptions.
 *
 * ReDMCSB: DUNGEON.C F0151_DUNGEON_GetSquare + F0172_SetSquareAspect
 */
int csb_v1_dungeon_decode_tile(const CSB_V1_DungeonData *d, int level, int x, int y,
                                CSB_V1_DecodedSquare *out);

/* ── Current-dungeon context (M10 integration) ─────────────────────── */

/* Get the currently-loaded dungeon (set by csb_v1_dungeon_load_from_file).
 * Returns NULL if no dungeon is loaded.
 *
 * This provides the M10 integration point: dungeon-layer accessor stubs
 * in csb_v1_dungeon_world_pc34_compat.c use the current-dungeon context
 * to service F0161/F0159/F0156/F0267 equivalent calls.
 *
 * ReDMCSB: DUNGEON.C globals G0278_ps_DungeonHeader / G0277_ps_DungeonMaps
 *          (same singleton pattern in the original engine)
 */
const CSB_V1_DungeonData *csb_v1_dungeon_get_current(void);

/* Set the current dungeon context (loaded dungeon).
 * Takes ownership of the dungeon: it will be freed on the next call
 * to csb_v1_dungeon_set_current() or csb_v1_dungeon_unload().
 * Pass NULL to unload without allocating a replacement.
 */
void csb_v1_dungeon_set_current(CSB_V1_DungeonData *d);

/* Unload the current dungeon and free all associated memory.
 * Idempotent: safe to call when no dungeon is loaded.
 */
void csb_v1_dungeon_unload(void);

/* ── Current dungeon level ─────────────────────────────────────────── */

/* Set the current dungeon level for dungeon-layer accessor queries.
 * The level is tracked independently of the dungeon data so that
 * dungeon-layer stubs (F0161/F0159 equivalents) can resolve the level
 * without needing a profile reference.
 *
 * Safe to call with any integer: out-of-range levels cause accessor
 * functions to return -1 / ENDOF rather than crashing.
 *
 * Default: 0 (set automatically by csb_v1_dungeon_set_current).
 *
 * ReDMCSB: G0272_i_CurrentMapIndex / G0309_i_PartyMapIndex
 */
void csb_v1_dungeon_set_current_level(int level);

/* Get the current dungeon level (set by csb_v1_dungeon_set_current_level).
 * Returns 0 if no level has been explicitly set.
 */
int  csb_v1_dungeon_get_current_level(void);

/* ── Cleanup ───────────────────────────────────────────────────────── */

void csb_v1_dungeon_free(CSB_V1_DungeonData *d);

/* ── Source evidence ───────────────────────────────────────────────── */

const char *csb_v1_dungeon_source_evidence(void);

#endif

