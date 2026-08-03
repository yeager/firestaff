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

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MOVEREC_OPS_PC34_COMPAT_H */
