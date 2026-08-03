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
 * Extract wall decoration index from actuator record byte layout.
 * Returns 0xFF if no decoration. */
uint8_t dm2_v1_get_wall_decoration_of_actuator(
    const uint8_t *actuator_record,
    const uint8_t *decoration_table, int decoration_table_size);

/* ---- DM2_GET_FLOOR_DECORATION_OF_ACTUATOR (c_record.cpp:1288) ----
 * Extract floor decoration index from actuator record byte layout.
 * Returns 0xFF if no decoration. */
uint8_t dm2_v1_get_floor_decoration_of_actuator(
    const uint8_t *actuator_record,
    const uint8_t *decoration_table, int decoration_table_size);

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

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RECORD_OPS_PC34_COMPAT_H */
