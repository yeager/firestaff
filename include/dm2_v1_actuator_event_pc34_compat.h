#ifndef FIRESTAFF_DM2_V1_ACTUATOR_EVENT_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_ACTUATOR_EVENT_PC34_COMPAT_H

/*
 * dm2_v1_actuator_event_pc34_compat.h — DM2 actuator event dispatch.
 *
 * Ports the skevent.cpp actuator sub-functions that ACTUATE_WALL_MECHA
 * and ACTUATE_FLOOR_MECHA dispatch to.  Record field accessors decode
 * the 8-byte actuator record layout from skproject dme.h.
 *
 * Source: skproject/SKWINSPX/src/v4/skevent.cpp
 *         skproject/SKWINSPX/src/v0/dme.h (Actuator class)
 */

#include <stdint.h>

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_timeline.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Actuator record field accessors (dme.h Actuator class) ────────── */

/* w2 (record bytes 2-3, little-endian) */
static inline uint16_t dm2_actu_w2(const uint8_t *r) {
    return (uint16_t)r[2] | ((uint16_t)r[3] << 8);
}
static inline uint8_t dm2_actu_type(const uint8_t *r) {
    return (uint8_t)(dm2_actu_w2(r) & 0x7Fu);
}
static inline uint16_t dm2_actu_data(const uint8_t *r) {
    return (uint16_t)((dm2_actu_w2(r) >> 7) & 0x01FFu);
}
static inline void dm2_actu_set_data(uint8_t *r, uint16_t val) {
    uint16_t w2 = dm2_actu_w2(r);
    w2 = (w2 & 0x007Fu) | ((val & 0x01FFu) << 7);
    r[2] = (uint8_t)(w2 & 0xFFu);
    r[3] = (uint8_t)(w2 >> 8);
}

/* w4 (record bytes 4-5, little-endian) */
static inline uint16_t dm2_actu_w4(const uint8_t *r) {
    return (uint16_t)r[4] | ((uint16_t)r[5] << 8);
}
static inline void dm2_actu_set_w4(uint8_t *r, uint16_t v) {
    r[4] = (uint8_t)(v & 0xFFu);
    r[5] = (uint8_t)(v >> 8);
}
static inline uint8_t dm2_actu_active_status(const uint8_t *r) {
    return (uint8_t)(dm2_actu_w4(r) & 1u);
}
static inline void dm2_actu_set_active_status(uint8_t *r, uint8_t v) {
    uint16_t w4 = dm2_actu_w4(r);
    w4 = (w4 & ~1u) | (v & 1u);
    dm2_actu_set_w4(r, w4);
}
static inline uint8_t dm2_actu_once_only(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w4(r) >> 2) & 1u);
}
static inline void dm2_actu_set_once_only(uint8_t *r, uint8_t v) {
    uint16_t w4 = dm2_actu_w4(r);
    w4 = (w4 & ~(1u << 2)) | ((v & 1u) << 2);
    dm2_actu_set_w4(r, w4);
}
static inline uint8_t dm2_actu_action_type(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w4(r) >> 3) & 3u);
}
static inline uint8_t dm2_actu_revert_effect(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w4(r) >> 5) & 1u);
}
static inline uint8_t dm2_actu_sound_effect(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w4(r) >> 6) & 1u);
}
static inline uint8_t dm2_actu_delay(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w4(r) >> 7) & 0xFu);
}

/* w6 (record bytes 6-7, little-endian) */
static inline uint16_t dm2_actu_w6(const uint8_t *r) {
    return (uint16_t)r[6] | ((uint16_t)r[7] << 8);
}
static inline uint8_t dm2_actu_direction(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w6(r) >> 4) & 3u);
}
static inline uint8_t dm2_actu_xcoord(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w6(r) >> 6) & 0x1Fu);
}
static inline uint8_t dm2_actu_ycoord(const uint8_t *r) {
    return (uint8_t)((dm2_actu_w6(r) >> 11) & 0x1Fu);
}

/* w0 (record bytes 0-1, little-endian) — next record link */
static inline int16_t dm2_actu_next_link(const uint8_t *r) {
    return (int16_t)((uint16_t)r[0] | ((uint16_t)r[1] << 8));
}

/* DB type from a record handle (ObjectID): bits 10-13 */
static inline int dm2_handle_db_type(int16_t handle) {
    return (int)((handle >> 10) & 0xFu);
}
/* Direction from a record handle: bits 0-1 */
static inline int dm2_handle_dir(int16_t handle) {
    return (int)(handle & 3u);
}

/* DB type constants (skproject dme.h) */
#define DM2_DB_TEXT     1
#define DM2_DB_ACTUATOR 3
#define DM2_DB_CREATURE 4

/* Action message types (skevent.cpp C00/C01/C02) */
#define DM2_ACTMSG_OPEN_SET    0
#define DM2_ACTMSG_CLOSE_CLEAR 1
#define DM2_ACTMSG_TOGGLE      2

/* TOGGLE_ACTUATOR_MESSAGE (skevent.cpp) — toggle once-only status. */
static inline uint8_t dm2_toggle_actuator_message(int action_type,
                                                   uint8_t current) {
    switch (action_type) {
        case DM2_ACTMSG_OPEN_SET:    return 1;
        case DM2_ACTMSG_CLOSE_CLEAR: return 0;
        case DM2_ACTMSG_TOGGLE:      return (uint8_t)(current ^ 1u);
        default:                     return current;
    }
}

/* ── Actuator type constants (dme.h ACTUATOR_TYPE_*) ───────────────── */

#define DM2_ACTU_SOME_SHOOTER        0x07
#define DM2_ACTU_MISSILE_SHOOTER     0x08
#define DM2_ACTU_WEAPON_SHOOTER      0x09
#define DM2_ACTU_MISSILE_SHOOTER_2   0x0A
#define DM2_ACTU_CREATURE_KILLER     0x0B
#define DM2_ACTU_ITEM_SHOOTER        0x0E
#define DM2_ACTU_ITEM_SHOOTER_X2     0x0F
#define DM2_ACTU_THE_END             0x12
#define DM2_ACTU_CROSS_MAP           0x16
#define DM2_ACTU_2_STATE_SWITCH      0x17
#define DM2_ACTU_WALL_SWITCH         0x18
#define DM2_ACTU_KEY_HOLE            0x1A
#define DM2_ACTU_COUNTER             0x1D
#define DM2_ACTU_TICK_GENERATOR      0x1E
#define DM2_ACTU_RELAY_1             0x20
#define DM2_ACTU_ARRIVAL_DEPARTURE   0x21
#define DM2_ACTU_SWITCH_SIGN_CREATURE 0x26
#define DM2_ACTU_ORNATE_ANIMATOR     0x2C
#define DM2_ACTU_FINITE_RELAY        0x2D
#define DM2_ACTU_CREATURE_GENERATOR  0x2E
#define DM2_ACTU_WORK_TIMER          0x31
#define DM2_ACTU_ORNATE_ANIMATOR_2   0x32
#define DM2_ACTU_TICK_GEN_V008       0x33
#define DM2_ACTU_TICK_GEN_V016       0x34
#define DM2_ACTU_TICK_GEN_V032       0x35
#define DM2_ACTU_TICK_GEN_V064       0x36
#define DM2_ACTU_TICK_GEN_V128       0x37
#define DM2_ACTU_PLACED_ITEM_TELEPORTER 0x3B
#define DM2_ACTU_ITEM_GENERATOR      0x3C
#define DM2_ACTU_RELAY_2             0x3D
#define DM2_ACTU_SHOP_PANEL          0x3F
#define DM2_ACTU_ITEM_RECYCLER       0x40
#define DM2_ACTU_ORNATE_STEP_ANIMATOR 0x41
#define DM2_ACTU_INVERSE_FLAG        0x43
#define DM2_ACTU_TEST_FLAG           0x44
#define DM2_ACTU_RELAY_3             0x45
#define DM2_ACTU_PUSH_BUTTON_SWITCH  0x46
#define DM2_ACTU_ITEM_CAPTURE        0x47
#define DM2_ACTU_ITEM_TELEPORT_UNK   0x48
#define DM2_ACTU_ITEM_CAPTURE_CREATURE 0x49
#define DM2_ACTU_RESURECTOR          0x7E

/* ── Receipt for wall/floor mecha dispatch ─────────────────────────── */

typedef struct {
    int valid;
    int records_visited;
    int actuators_dispatched;
    int text_records_toggled;
    int relay1_invoked;
    int relay2_invoked;
    int shooter_invoked;
    int tick_generator_invoked;
    int ornate_animator_invoked;
    int item_teleport_invoked;
    int counter_invoked;
    int cross_map_invoked;
    int creature_generator_invoked;
    int work_timer_invoked;
    int item_generator_invoked;
    int inverse_flag_invoked;
    int test_flag_invoked;
    int push_button_invoked;
    int unknown_type_skipped;
    int fail_closed;
} DM2_V1_ActuatorEventReceipt;

/* ── Wall/floor mecha dispatch ─────────────────────────────────────── */

/* Full ACTUATE_WALL_MECHA dispatch: walks the actuator record chain
 * at (map, x, y) and dispatches each actuator by type.  Requires
 * record_pools and dungeon to be valid.
 *
 * `action_type` is the timer's ActionType (0=open/set, 1=close/clear, 2=toggle).
 * `direction` is the timer's Value2 (facing direction 0..3).
 *
 * Source: skevent.cpp:1633-2068 ACTUATE_WALL_MECHA */
int dm2_v1_actuate_wall_mecha(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_DungeonData *dungeon,
                              DM2_V1_SourceTimerQueue *queue,
                              int map, int x, int y,
                              int action_type, int direction,
                              uint32_t game_tick,
                              DM2_V1_ActuatorEventReceipt *receipt);

/* Full ACTUATE_FLOOR_MECHA dispatch.
 * Source: skevent.cpp:2072-2232 ACTUATE_FLOOR_MECHA */
int dm2_v1_actuate_floor_mecha(DM2_V1_RecordPoolSet *pool_set,
                               DM2_V1_DungeonData *dungeon,
                               DM2_V1_SourceTimerQueue *queue,
                               int map, int x, int y,
                               int action_type, int direction,
                               uint32_t game_tick,
                               DM2_V1_ActuatorEventReceipt *receipt);

/* ACTIVATE_TICK_GENERATOR — queue a tick generator timer.
 * Source: skevent.cpp:357-400 */
int dm2_v1_activate_tick_generator(DM2_V1_SourceTimerQueue *queue,
                                   const uint8_t *actu_record,
                                   int16_t record_link,
                                   int map, uint32_t game_tick);

/* ACTIVATE_RELAY1 — relay with delay.
 * Source: skevent.cpp:1154-1183 */
int dm2_v1_activate_relay1(DM2_V1_RecordPoolSet *pool_set,
                           DM2_V1_DungeonData *dungeon,
                           DM2_V1_SourceTimerQueue *queue,
                           const uint8_t *actu_record,
                           int timer_action_type,
                           int delay_as_mult,
                           int map, uint32_t game_tick);

/* ACTIVATE_RELAY2 — relay dispatch chain.
 * Source: skevent.cpp:1185-1232 */
int dm2_v1_activate_relay2(DM2_V1_RecordPoolSet *pool_set,
                           DM2_V1_DungeonData *dungeon,
                           DM2_V1_SourceTimerQueue *queue,
                           const uint8_t *actu_record,
                           int timer_action_type,
                           int actu_data_param,
                           int map, uint32_t game_tick);

/* INVOKE_ACTUATOR — decode actuator target and queue a timer message.
 * Source: c_tim_proc.cpp:4367-4392 */
int dm2_v1_invoke_actuator(DM2_V1_SourceTimerQueue *queue,
                           const uint8_t *actu_record,
                           int action_type, int delay_add,
                           int map, uint32_t game_tick);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ACTUATOR_EVENT_PC34_COMPAT_H */
