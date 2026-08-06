
#ifndef FIRESTAFF_CSB_V1_DUNGEON_LOADER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_DUNGEON_LOADER_PC34_COMPAT_H

#include <stddef.h>
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
 *   Disk data is often FTL-compressed (signature bytes 81 04). After
 *   decompression and byte swapping, it uses the same 44-byte DUNGEON_HEADER
 *   and 16-byte MAP descriptors as DM1, followed by byte-sized square data.
 *
 * The small synthetic unit fixtures keep an older Firestaff-only shape:
 *   levels(LE16), thing-type-count(LE16), per-level width/height/offset,
 *   and 16-bit synthetic square records. The loader supports those fixtures
 *   separately so real CSB assets use the source format.
 */

#define CSB_V1_MAX_LEVELS 12
#define CSB_V1_MAX_SQUARE_SIZE 32
#define CSB_V1_THING_TYPE_GROUP 4
#define CSB_V1_THING_TYPE_ACTUATOR 3
#define CSB_V1_THING_TYPE_DSA 15  /* CSB-specific: DSA script thing */

/* CSBWin DSA filter locations are Expool EDT_SpecialLocations records.
 * Monster.cpp resolves only a type-47 DB3 actuator on the decoded square.
 * ReDMCSB has no DSA equivalent; this is CSBWin-specific compatibility. */
#define CSB_V1_EXPOOL_EDT_SPECIAL_LOCATIONS 3u
#define CSB_V1_EXPOOL_ESL_MONSTER_ATTACK_FILTER 0u
#define CSB_V1_EXPOOL_ESL_MONSTER_MOVE_FILTER 2u
#define CSB_V1_EXPOOL_ESL_CURSOR_FILTER 9u
#define CSB_V1_EXPOOL_ESL_CHAR_DEATH_FILTER 11u
#define CSB_V1_EXPOOL_ESL_EQUIP_FILTER 12u
#define CSB_V1_EXPOOL_ESL_DAMAGE_CHAR_FILTER 15u
#define CSB_V1_DSA_FILTER_ACTUATOR_TYPE 47u

typedef struct {
    int level;
    int x;
    int y;
    int position;
    int party_level_only;
    int max_distance;
    uint16_t actuator_thing;
} CSB_V1_DSAFilterLocation;

typedef struct CSB_V1_DungeonData {
    int level_count;
    /* ReDMCSB LOADSAVE.C F0435 consumes DUNGEON_HEADER.InitialPartyLocation
     * for a new game. Retain the raw packed value rather than substituting
     * a CSB-specific host default. */
    uint16_t initial_party_location;
    int level_offsets[CSB_V1_MAX_LEVELS];
    int level_widths[CSB_V1_MAX_LEVELS];
    int level_heights[CSB_V1_MAX_LEVELS];
    int map_levels[CSB_V1_MAX_LEVELS];
    int map_offset_x[CSB_V1_MAX_LEVELS];
    int map_offset_y[CSB_V1_MAX_LEVELS];
    /* ReDMCSB DEFS.H MAP.C's final word stores the four source selectors
     * low-to-high as FloorSet, WallSet, DoorSet0 and DoorSet1. */
    int map_floor_set[CSB_V1_MAX_LEVELS];
    int map_wall_set[CSB_V1_MAX_LEVELS];
    int map_door_set0[CSB_V1_MAX_LEVELS];
    int map_door_set1[CSB_V1_MAX_LEVELS];
    /* ReDMCSB DUNGEON_HEADER MAP.C high nibble. PANEL.C F0337 treats a
     * zero value as the special fully-lit map rule before considering
     * torches or magical light. */
    int map_difficulty[CSB_V1_MAX_LEVELS];
    int map_experience_multiplier[CSB_V1_MAX_LEVELS];
    int map_wall_ornament_count[CSB_V1_MAX_LEVELS];
    int map_floor_ornament_count[CSB_V1_MAX_LEVELS];
    int map_random_floor_ornament_count[CSB_V1_MAX_LEVELS];
    int map_creature_type_count[CSB_V1_MAX_LEVELS];
    uint8_t map_floor_ornament_indices[CSB_V1_MAX_LEVELS][16];
    uint16_t ornament_random_seed;
    int square_bytes;
    int raw_map_data_base;
    int square_first_thing_base;
    int square_first_thing_count;
    /* ReDMCSB DUNGEON.C F0168 text-word bank.  Keep its disk offset explicit
     * so CSBWin timer paths can consume a real DB2 TextString rather than a
     * host message substitute. */
    int text_data_base;
    int text_word_count;
    int thing_data_bases[16];
    int thing_type_counts[16];
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

/* Load an in-memory original CSB dungeon payload. Unlike the historical
 * compatibility entry above, this is a production boundary: it accepts only
 * the ReDMCSB DUNGEON_HEADER/MAP one-byte-square layout after any required
 * FTL decompression. The old 16-bit fixture layout exists solely for focused
 * parser regressions and must never become a save/import runtime world.
 *
 * ReDMCSB DUNGEON.C F0237 and LOADSAVE.C F0435. */
int csb_v1_dungeon_load_source_bytes(CSB_V1_DungeonData *out,
                                     const uint8_t *dat, int dat_size);

/* Load dungeon from a file path.
 * Reads the entire file into memory, then delegates to csb_v1_dungeon_load().
 * File-backed loading accepts only the original one-byte-square CSB format;
 * retired 16-bit test fixtures are rejected at this boundary.
 * Returns 0 on success, -1 on file open error, -2 on parse error.
 * Caller must call csb_v1_dungeon_free() to release memory.
 *
 * Source: CSBWin/CSBCode.cpp LoadDungeon lines 6800-6950 */
int csb_v1_dungeon_load_from_file(CSB_V1_DungeonData *out, const char *path);

/* Decode DUNGEON_HEADER.InitialPartyLocation exactly as LOADSAVE.C F0435:
 * x = bits 0..4, y = bits 5..9, direction = bits 10..11, map = 0. */
int csb_v1_dungeon_initial_party_pose_pc34(const CSB_V1_DungeonData *d,
                                           int *out_map_index,
                                           int *out_x,
                                           int *out_y,
                                           int *out_direction);

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

/* Return the square's first thing.
 * Returns -1 if d is NULL, raw_data is NULL, or coordinates out of bounds.
 * Real CSB-format maps return the full THING handle from the imported
 * square-first-thing table. Legacy synthetic fixtures return the older
 * Firestaff-only 10-bit index stored in the 16-bit test square record.
 *
 * ReDMCSB: DUNGEON.C F0160 lines 1699-1728, F0161 lines 1730-1746.
 */
int csb_v1_dungeon_get_first_thing(const CSB_V1_DungeonData *d, int level, int x, int y);

/* Return the raw 16-bit square record at a given position.
 * Returns -1 on error, or the 16-bit value on success.
 *
 * ReDMCSB: DUNGEON.C F0151 (column-major, 2-byte records)
 */
int csb_v1_dungeon_get_raw_square(const CSB_V1_DungeonData *d, int level, int x, int y);

/* Source-owned current-map accessors for the real PC34 byte-map layout.
 * These preserve ReDMCSB DUNGEON.C F0151--F0153's wall result at the four
 * immediately adjacent map edges.  They reject legacy 16-bit fixtures and
 * malformed maps rather than manufacturing a square. */
int csb_v1_dungeon_f0151_get_square_pc34(
    const CSB_V1_DungeonData *d, int level, int x, int y);
/* ReDMCSB DUNGEON.C F0150's relative-view coordinate transform, exposed so
 * source consumers can obtain the same map position before resolving a
 * square-owned Thing chain. */
int csb_v1_dungeon_f0150_get_relative_location_pc34(
    int direction, int steps_forward, int steps_right,
    int map_x, int map_y, int *out_map_x, int *out_map_y);
int csb_v1_dungeon_f0152_get_relative_square_pc34(
    const CSB_V1_DungeonData *d, int level, int direction,
    int steps_forward, int steps_right, int map_x, int map_y);
int csb_v1_dungeon_f0153_get_relative_square_type_pc34(
    const CSB_V1_DungeonData *d, int level, int direction,
    int steps_forward, int steps_right, int map_x, int map_y);

/* Explicit real-data owners for ReDMCSB DUNGEON.C F0159/F0162.  A missing
 * record, malformed chain, or non-PC34 byte map returns THING_ENDOFLIST
 * (0xfffe); no decoded or fixture-only chain is substituted. */
uint16_t csb_v1_dungeon_f0159_get_next_thing_pc34(
    const CSB_V1_DungeonData *d, uint16_t thing);
uint16_t csb_v1_dungeon_f0162_get_square_first_object_pc34(
    const CSB_V1_DungeonData *d, int level, int x, int y);

/* ReDMCSB DUNGEON.C F0154/F0155 over explicit loaded PC34 MAP ownership.
 * F0154 commits x/y only when a same-global-coordinate target map exists;
 * F0155 returns -1 when the source square cannot be read. */
int csb_v1_dungeon_f0154_get_location_after_level_change_pc34(
    const CSB_V1_DungeonData *d, int map_index, int level_delta,
    int *inout_map_x, int *inout_map_y);
int csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
    const CSB_V1_DungeonData *d, int level, int map_x, int map_y);

/* Caller-owned F0165 discard path used only after F0166 has exhausted the
 * actual DB record range. It must return a matching live Thing or 0xffff. */
typedef uint16_t (*CSB_V1_DungeonDiscardThingPc34Compat)(
    uint16_t thing_type, void *user);

/* ReDMCSB DUNGEON.C F0166. Allocates only an actual unused DB record, keeps
 * three ordinary JUNK entries reserved for champion bones, and clears the
 * complete source record before setting Generic.Next to ENDOFLIST. */
uint16_t csb_v1_dungeon_f0166_get_unused_thing_pc34(
    CSB_V1_DungeonData *d, uint16_t requested_thing_type,
    CSB_V1_DungeonDiscardThingPc34Compat discard_thing, void *discard_user);

/* Return a read-only pointer to a decoded dungeon thing record.
 * Returns NULL if the thing handle is invalid, the type has no records, or
 * the record would exceed raw_data bounds.  out_type/out_index/out_size are
 * optional.
 *
 * ReDMCSB: DEFS.H M012_TYPE/M013_INDEX and DUNGEON.C
 * F0156_DUNGEON_GetThingData. */
const uint8_t *csb_v1_dungeon_get_thing_record(
    const CSB_V1_DungeonData *d,
    uint16_t thing,
    int *out_type,
    int *out_index,
    int *out_size);

/* Locate a CSBWin Expool record in DB11. Returns 1 when found and fills
 * out_bytes/out_size with the record payload bytes, or 0 when the pool is
 * absent, malformed, or the key is missing.
 *
 * Source: CSBWin data.cpp EXPOOL::Locate lines 1575-1599. */
int csb_v1_dungeon_expool_locate_record(
    const CSB_V1_DungeonData *d,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size);

/* Decode a CSBWin Monster.cpp special-location word. `movement_filter` adds
 * the bit-18 party-level gate and bits 19..23 maximum-distance fields. */
int csb_v1_dungeon_decode_dsa_filter_location(
    const CSB_V1_DungeonData *d, uint32_t record_word, int movement_filter,
    CSB_V1_DSAFilterLocation *out);

/* Resolve Monster.cpp's attack key or level-specific/global movement key,
 * then select the first type-47 actuator in the decoded square's Thing list.
 * It decodes locations only; it deliberately does not execute DSA actions. */
int csb_v1_dungeon_resolve_dsa_filter_location(
    const CSB_V1_DungeonData *d, int level, int movement_filter,
    CSB_V1_DSAFilterLocation *out);

/* Resolve a non-monster CSBWin EDT_SpecialLocations DSA filter.  The caller
 * supplies the source ESL ordinal; this accepts no inferred fallback key and
 * retains CSBWin's first type-47 actuator selection on the decoded square. */
int csb_v1_dungeon_resolve_dsa_special_location(
    const CSB_V1_DungeonData *d, uint8_t special_location,
    CSB_V1_DSAFilterLocation *out);

/* Adapter matching CSB_V1_SkinCacheRecordLookup. Pass CSB_V1_DungeonData*
 * as user. */
int csb_v1_dungeon_skin_cache_record_lookup(
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size,
    void *user);

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

/* Mutable companion to csb_v1_dungeon_get_current().  The loaded dungeon
 * remains the original decoded DUNGEON.DAT image; M10 mutates its live Thing
 * chains in place, as ReDMCSB mutates G0283/G0284. */
CSB_V1_DungeonData *csb_v1_dungeon_get_current_mutable(void);

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
