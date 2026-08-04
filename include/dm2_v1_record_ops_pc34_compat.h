#ifndef FIRESTAFF_DM2_V1_RECORD_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_RECORD_OPS_PC34_COMPAT_H

/*
 * dm2_v1_record_ops_pc34_compat.h — DM2 V1 record operations from
 * skproject/SKULLWIN/c_record.cpp + SKWIN/SkWinCore2.cpp.
 *
 * Callback-based implementations of:
 *   DM2_QUERY_GDAT_DBSPEC_WORD_VALUE  c_record.cpp:352
 *   DM2_GET_WALL_TILE_ANYITEM_RECORD  c_record.cpp:477
 *   DM2_GET_WALL_DECORATION_OF_ACTUATOR  c_record.cpp:1261
 *   DM2_GET_FLOOR_DECORATION_OF_ACTUATOR  c_record.cpp:1288
 *   DM2_075f_056c                     c_record.cpp:1870
 *   DM2_ROTATE_RECORD_BY_TELEPORTER   c_record.cpp:1839
 *   DM2_IS_OBJECT_VISIBLE_TEXT        SkWinCore2.cpp:592
 *   GET_WALL_TILE_ANY_TAKEABLE_ITEM_RECORD  SkWinCore2.cpp:368
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_QUERY_GDAT_DBSPEC_WORD_VALUE (c_record.cpp:352) ----
 * Queries GDAT entry data index for a record's cls1/cls2 with dtWordValue.
 * Returns 0 for OBJECT_NULL (0xFFFF). */
typedef struct {
    uint8_t (*query_cls1_from_record)(void *ctx, uint16_t record_word);
    uint8_t (*query_cls2_from_record)(void *ctx, uint16_t record_word);
    int16_t (*query_gdat_entry_data_index)(void *ctx, uint8_t cls1,
                                           uint8_t cls2, uint8_t entry_type,
                                           uint8_t data_index);
} DM2_V1_GdatDbspecCallbacks;

int16_t dm2_v1_query_gdat_dbspec_word_value(
    uint16_t record_word, uint8_t data_index,
    const DM2_V1_GdatDbspecCallbacks *cb, void *ctx);

/* ---- DM2_GET_WALL_TILE_ANYITEM_RECORD (c_record.cpp:477) ----
 * Walk tile record chain, skip types 0..3 (wall/floor/door/stairs),
 * return first record with type > 3 (an item). */
typedef struct {
    int16_t (*get_tile_record_link)(void *ctx, int16_t map_x, int16_t map_y);
    int16_t (*get_next_record_link)(void *ctx, uint16_t record_word);
} DM2_V1_TileRecordWalkCallbacks;

int16_t dm2_v1_get_wall_tile_anyitem_record(
    int16_t map_x, int16_t map_y,
    const DM2_V1_TileRecordWalkCallbacks *cb, void *ctx);

/* ---- GET_WALL_TILE_ANY_TAKEABLE_ITEM_RECORD (SkWinCore2.cpp:368) ----
 * Walk tile chain for items in direction, types weapon..misc (5..10). */
int16_t dm2_v1_get_wall_tile_any_takeable_item_record(
    uint16_t map_x, uint16_t map_y, uint16_t direction,
    const DM2_V1_TileRecordWalkCallbacks *cb, void *ctx);

/* ---- DM2_GET_WALL_DECORATION_OF_ACTUATOR (c_record.cpp:1261) ----
 * Extract wall decoration from actuator record: word@4 >> 12 gives the
 * 1-based decoration index into the map's decoration table (looked up via
 * mapdat.map/ddat.v1e03c0 structure). Returns 0xFF if index is 0.
 * The lookup_decoration callback encapsulates the map-relative table
 * addressing: (wall variant) reads ddat.v1e03c0 word@0xC >> 12 << 8,
 * offsets into mapdat.map, and returns the byte at [base + index - 1]. */
typedef struct {
    uint8_t (*lookup_wall_decoration)(void *ctx, int16_t index);
    uint8_t (*lookup_floor_decoration)(void *ctx, int16_t index);
} DM2_V1_DecorationLookupCallbacks;

uint8_t dm2_v1_get_wall_decoration_of_actuator(
    const uint8_t *actuator_record,
    const DM2_V1_DecorationLookupCallbacks *cb, void *ctx);

/* ---- DM2_GET_FLOOR_DECORATION_OF_ACTUATOR (c_record.cpp:1288) ----
 * Same structure as wall variant but uses word@0xA & 0xF from
 * ddat.v1e03c0 for the floor-specific base offset. */
uint8_t dm2_v1_get_floor_decoration_of_actuator(
    const uint8_t *actuator_record,
    const DM2_V1_DecorationLookupCallbacks *cb, void *ctx);

/* ---- DM2_075f_056c (c_record.cpp:1870) ----
 * If record is missile (type 0xE), delete its timer. */
typedef struct {
    const uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    void (*delete_timer)(void *ctx, int16_t timer_index);
} DM2_V1_MissileCleanupCallbacks;

int dm2_v1_missile_timer_cleanup(
    uint16_t record_word,
    const DM2_V1_MissileCleanupCallbacks *cb, void *ctx);

/* ---- DM2_ROTATE_RECORD_BY_TELEPORTER (c_record.cpp:1839) ----
 * Apply teleporter rotation to a record's facing direction.
 * teleporter_flags: byte at offset+3 of teleporter record (bit 4 = relative).
 * stored_dir: current value in ddat.v1e1024.
 * record_word: the 16-bit record word to rotate.
 * Returns the modified record word; *out_dir receives the new direction. */
uint16_t dm2_v1_rotate_record_by_teleporter(
    uint8_t teleporter_flags, uint8_t stored_dir,
    uint16_t record_word, uint8_t *out_dir);

/* ---- DM2_IS_OBJECT_VISIBLE_TEXT (SkWinCore2.cpp:592) ----
 * Check if a record is a visible text object.
 * Returns the text visibility flag, or 0. */
typedef struct {
    int (*get_db_type)(void *ctx, uint16_t record_word);
    const uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
} DM2_V1_ObjectTextCallbacks;

uint8_t dm2_v1_is_object_visible_text(
    uint16_t record_word,
    const DM2_V1_ObjectTextCallbacks *cb, void *ctx);

/* ---- DM2_OVERSEE_RECORD (c_record.cpp:1755) ----
 * Walk a linked thing list, optionally recursing into containers/chests.
 * Calls the filter callback for each record; stops when callback returns
 * nonzero. Sets *prev_link_out to the address of the link that points to
 * the matching record. Returns pointer to the matching record, or NULL. */
typedef struct {
    int16_t (*get_next_record_link)(void *ctx, uint16_t record_word);
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    int (*is_container_chest)(void *ctx, uint16_t record_word);
} DM2_V1_OverseeCallbacks;

typedef int (*DM2_V1_OverseeFilterFn)(int16_t *record_ptr, int16_t *userdata);

int16_t *dm2_v1_oversee_record(
    int16_t *first_link, uint8_t type_filter,
    int16_t **prev_link_out, DM2_V1_OverseeFilterFn filter_fn,
    int16_t *userdata, int recurse_containers, int recurse_chests,
    const DM2_V1_OverseeCallbacks *cb, void *ctx);

/* ---- DM2_GET_ITEMDB_OF_ITEMSPEC_ACTUATOR (c_record.cpp:367) ----
 * Pure lookup: map a distinctive itemtype to its DB category.
 * Returns 5,6,10,4,8,9,7 or 0xFFFF if out of range. */
int16_t dm2_v1_get_itemdb_of_itemspec_actuator(uint16_t itemspec);

/* ---- DM2_GET_ITEMTYPE_OF_ITEMSPEC_ACTUATOR (c_record.cpp:403) ----
 * Pure lookup: extract the item type offset from a distinctive itemtype.
 * Returns the adjusted type value. */
int16_t dm2_v1_get_itemtype_of_itemspec_actuator(uint16_t itemspec);

/* ---- DM2_QUERY_CLS1_FROM_RECORD (c_record.cpp:454) ----
 * Walk record chain (following type-14 containers) to find the
 * database category (cls1). Uses table1d3298 lookup. */
typedef struct {
    const uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    uint8_t table1d3298[16];  /* db type -> cls1 mapping */
} DM2_V1_Cls1Callbacks;

uint8_t dm2_v1_query_cls1_from_record(
    uint16_t record_word,
    const DM2_V1_Cls1Callbacks *cb, void *ctx);

/* ---- DM2_QUERY_CLS2_FROM_RECORD (c_record.cpp:203) ----
 * Determine the sub-type (cls2) of a record by inspecting its
 * db type and record data. Handles all 16 record types. */
typedef struct {
    const uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    uint8_t (*query_cls2_of_text_record)(void *ctx, uint16_t rw);
    uint8_t (*get_wall_decoration_of_actuator)(void *ctx, const uint8_t *rec);
} DM2_V1_Cls2Callbacks;

uint8_t dm2_v1_query_cls2_from_record(
    uint16_t record_word,
    const DM2_V1_Cls2Callbacks *cb, void *ctx);

/* ---- DM2_QUERY_CLS2_OF_TEXT_RECORD (c_record.cpp:1210) ----
 * Extract cls2 from a text record (type 2) based on text subtype. */
typedef struct {
    const uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
} DM2_V1_TextCls2Callbacks;

uint8_t dm2_v1_query_cls2_of_text_record(
    uint16_t record_word,
    const DM2_V1_TextCls2Callbacks *cb, void *ctx);

/* ---- DM2_GET_DISTINCTIVE_ITEMTYPE (c_record.cpp:175) ----
 * Combine db-type and cls2 into a distinctive item type using
 * table1d3278 mapping. Returns 0x1FF for OBJECT_NULL. */
typedef struct {
    uint8_t (*query_cls2_from_record)(void *ctx, uint16_t rw);
    int16_t table1d3278[16];  /* db type -> base itemtype mapping */
} DM2_V1_DistinctiveItemtypeCallbacks;

int16_t dm2_v1_get_distinctive_itemtype(
    uint16_t record_word,
    const DM2_V1_DistinctiveItemtypeCallbacks *cb, void *ctx);

/* ---- DM2_SET_ITEMTYPE (c_record.cpp:284) ----
 * Write the item subtype (cls2) into a record's data field.
 * Handles db types 4..10 with type-specific bit packing. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
} DM2_V1_SetItemtypeCallbacks;

void dm2_v1_set_itemtype(
    uint16_t record_word, uint8_t item_type,
    const DM2_V1_SetItemtypeCallbacks *cb, void *ctx);

/* ---- DM2_SET_ITEM_IMPORTANCE (c_record.cpp:498) ----
 * Set the importance/priority bit on an item record. */
void dm2_v1_set_item_importance(
    uint16_t record_word, uint8_t importance,
    const DM2_V1_SetItemtypeCallbacks *cb, void *ctx);

/* ---- DM2_QUERY_ITEMDB_FROM_DISTINCTIVE_ITEMTYPE (c_record.cpp:449) ----
 * Get the cls1 value for a distinctive item type. */
uint8_t dm2_v1_query_itemdb_from_distinctive_itemtype(
    uint16_t distinctive_type,
    const DM2_V1_Cls1Callbacks *cb, void *ctx);

/* ---- DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD (c_record.cpp:1351) ----
 * Look up AI spec data for a creature by its type byte. */
typedef struct {
    int16_t (*query_gdat_creature_word_value)(void *ctx, uint8_t ctype, uint8_t idx);
    const uint8_t *(*get_ai_spec)(void *ctx, uint16_t spec_index);
    const uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
} DM2_V1_CreatureAICallbacks;

const uint8_t *dm2_v1_query_creature_ai_spec_from_record(
    uint8_t creature_type,
    const DM2_V1_CreatureAICallbacks *cb, void *ctx);

/* ---- DM2_QUERY_CREATURE_AI_SPEC_FROM_TYPE (c_record.cpp:1341) ----
 * Look up AI spec data from a creature record word. */
const uint8_t *dm2_v1_query_creature_ai_spec_from_type(
    uint16_t record_word,
    const DM2_V1_CreatureAICallbacks *cb, void *ctx);

/* ---- DM2_QUERY_CREATURE_AI_SPEC_FLAGS (c_record.cpp:1346) ----
 * Get the AI spec flags word for a creature record. */
int16_t dm2_v1_query_creature_ai_spec_flags(
    uint16_t record_word,
    const DM2_V1_CreatureAICallbacks *cb, void *ctx);

/* ---- DM1_ROTATE_ACTUATOR_LIST (SkWinCore2.cpp:298) ----
 * Rotate the order of actuator records on a tile side.
 * Returns 1 if rotated, 0 if nothing to rotate. */
typedef struct {
    int16_t (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_next_record_link)(void *ctx, uint16_t rw);
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    void (*set_next_record_link)(void *ctx, uint16_t rw, int16_t next);
    void (*set_tile_record_link)(void *ctx, int16_t x, int16_t y, int16_t rw);
} DM2_V1_ActuatorRotateCallbacks;

int dm2_v1_rotate_actuator_list(
    int16_t map_x, int16_t map_y, uint8_t direction,
    const DM2_V1_ActuatorRotateCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RECORD_OPS_PC34_COMPAT_H */
