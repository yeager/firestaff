#ifndef FIRESTAFF_DM2_V1_MOVEREC_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_MOVEREC_OPS_PC34_COMPAT_H

/*
 * dm2_v1_moverec_ops_pc34_compat.h — DM2 V1 record movement operations from
 * skproject/SKULLWIN/c_moverec.cpp.
 *
 * Callback-based implementations of:
 *   DM2_SET_MINION_RECENT_OPEN_DOOR_LOCATION  c_moverec.cpp:312
 *   DM2_moverec_2fcf_01c5                     c_moverec.cpp:365
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_SET_MINION_RECENT_OPEN_DOOR_LOCATION (c_moverec.cpp:312) ----
 * Walk creature's linked record list; for type-0xE records, write x/y/map/flag. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    int16_t (*get_next_record_link)(void *ctx, uint16_t record_word);
} DM2_V1_MinionDoorCallbacks;

void dm2_v1_set_minion_recent_open_door_location(
    uint16_t first_record, uint8_t x, uint8_t y, uint8_t map, int open_flag,
    const DM2_V1_MinionDoorCallbacks *cb, void *ctx);

/* ---- DM2_moverec_2fcf_01c5 (c_moverec.cpp:365) ----
 * Queue a timer and call set_minion_recent_open_door_location. */
typedef struct {
    uint32_t game_tick;
    void (*queue_timer)(void *ctx, uint8_t type, uint8_t x, uint8_t y,
                        uint16_t record, uint32_t fire_tick);
    void (*set_minion_door)(void *ctx, uint16_t record, uint8_t x, uint8_t y,
                            uint8_t map, int open_flag);
} DM2_V1_MoverecTimerCallbacks;

void dm2_v1_moverec_2fcf_01c5(
    uint16_t record, uint8_t x, uint8_t y, uint8_t map, int direction_flag,
    const DM2_V1_MoverecTimerCallbacks *cb, void *ctx);

/* ---- dm2_v1_try_push_object_to (c_moverec.cpp:25) ----
 * Try 4 directions to push an object to a free adjacent tile. */
typedef struct {
    int (*is_tile_free)(void *ctx, int16_t x, int16_t y);
    void (*move_record_to)(void *ctx, int32_t record, int16_t x, int16_t y);
} DM2_V1_TryPushObjectToCallbacks;

int32_t dm2_v1_try_push_object_to(
    int32_t record, int16_t dst_x, int16_t dst_y,
    int16_t *out_x, int16_t *out_y,
    const DM2_V1_TryPushObjectToCallbacks *cb, void *ctx);

/* ---- dm2_v1_moverec_2fcf_0234 (c_moverec.cpp:144) ----
 * Relink a record from source tile to destination tile. */
typedef struct {
    void (*unlink_record)(void *ctx, int32_t record, int16_t x, int16_t y);
    void (*link_record)(void *ctx, int32_t record, int16_t dst_x, int16_t dst_y);
} DM2_V1_Moverec2fcf0234Callbacks;

void dm2_v1_moverec_2fcf_0234(
    int32_t record, int16_t src_x, int16_t src_y,
    int16_t dst_x, int16_t dst_y,
    const DM2_V1_Moverec2fcf0234Callbacks *cb, void *ctx);

/* ---- dm2_v1_moverec_3ce7d (c_moverec.cpp:1147) ----
 * Post-move dispatcher — delegates to a single dispatch callback. */
typedef struct {
    int32_t (*dispatch)(void *ctx, int32_t record, int16_t x, int16_t y,
                        int32_t kind, int32_t flags);
} DM2_V1_Moverec3ce7dCallbacks;

void dm2_v1_moverec_3ce7d(
    int32_t record, int16_t x, int16_t y, int32_t kind, int32_t flags,
    const DM2_V1_Moverec3ce7dCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MOVEREC_OPS_PC34_COMPAT_H */
