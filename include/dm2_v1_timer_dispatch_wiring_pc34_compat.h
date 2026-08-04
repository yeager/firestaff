#ifndef FIRESTAFF_DM2_V1_TIMER_DISPATCH_WIRING_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_TIMER_DISPATCH_WIRING_PC34_COMPAT_H

/*
 * dm2_v1_timer_dispatch_wiring_pc34_compat.h — Wire implemented timer ops
 * into DM2_V1_TimerDispatcher.
 *
 * Bridges the narrow per-timer callback APIs in dm2_v1_timer_ops_pc34_compat.h
 * to the uniform DM2_V1_TimerTypeHandler signature used by the timer
 * dispatcher (dm2_v1_proceed_timers_pc34_compat.h).
 *
 * Currently wired timer types: 0x01..0x5E (26 of 26)
 *   0x01 STEP_DOOR                0x02 DESTROY_DOOR
 *   0x04 ACTUATE_TILE             0x0C PROCESS_0C
 *   0x0D RESURRECTION (stub)      0x0E PROCESS_0E
 *   0x15 PROCESS_SOUND            0x19 PROCESS_CLOUD (stub)
 *   0x1E STEP_MISSILE (stub)      0x21 THINK_CREATURE_A (stub)
 *   0x22 THINK_CREATURE_B (stub)  0x3D PROCESS_3D
 *   0x46 LIGHT                    0x47 HERO_ENCH_FLAG
 *   0x48 ENCH_POWER               0x4B POISON
 *   0x54 UPDATE_WEATHER           0x55 ORNATE_ANIMATOR
 *   0x56 TICK_GENERATOR           0x58 RELEASE_DOOR_BUTTON
 *   0x59 PROCESS_59               0x5A ORNATE_NOISE (stub)
 *   0x5B RECORD_CLEAR             0x5C RECORD_SET
 *   0x5D MOVE_RECORD_ROTATE       0x5E ALLOC_NEW_CREATURE
 *
 * Source: skproject c_tim_proc.cpp DM2_PROCEED_TIMERS
 */

#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_runtime_parity_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Runtime context for timer dispatch wiring. The host populates the
 * callback fields before calling dm2_v1_timer_dispatch_wiring_init.
 * Fields left NULL cause their corresponding timer types to remain
 * unbound (fail-closed in the dispatcher). */
typedef struct {
    /* Record address lookup (many handlers need this) */
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);

    /* Tile byte access (DESTROY_DOOR) */
    uint8_t *(*get_tile_byte)(void *ctx, uint8_t x, uint8_t y);

    /* Move record (PROCESS_3D) */
    int (*move_record_to)(void *ctx, uint16_t record, int16_t level,
                          int16_t unused, int16_t x, int16_t y);

    /* Noise generation */
    void (*queue_noise)(void *ctx, int16_t x, int16_t y);

    /* Timer re-queue */
    void (*queue_light_timer)(void *ctx, int16_t intensity, uint32_t delay);

    /* Light state */
    int16_t *global_light;
    const int16_t *light_table;
    int light_table_size;

    /* Map state */
    int16_t current_map;
    int16_t party_map;
    int *redraw_flags;

    /* Ornate animator */
    int16_t (*get_ornate_anim_len)(void *ctx, uint8_t *rec, int mode);
    void (*queue_ornate_timer)(void *ctx);

    /* Tick generator */
    void (*invoke_actuator)(void *ctx, uint8_t *rec, uint16_t action,
                            uint16_t param);
    void (*requeue_tick_timer)(void *ctx, uint16_t delay_base,
                               uint8_t multiplier);

    /* Process 0E */
    void *(*alloc_memory)(void *ctx, int32_t size);
    void (*dealloc_memory)(void *ctx, void *ptr, int32_t size);
    void (*set_itemtype)(void *ctx, uint16_t record, uint16_t new_type);
    void (*process_item_bonus)(void *ctx, uint8_t actor, uint16_t record,
                               int mode, uint16_t value);
    void (*copy_memory)(void *dst, const void *src, int32_t size);
    int32_t (*get_item_size)(uint16_t db_type);

    /* Actuator tile subdispatch (0x04) */
    int (*tile_class_at)(void *ctx, int map, int x, int y);
    void (*actuate_pitfall)(int16_t x, int16_t y, int16_t param,
                            const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);
    void (*actuate_door_fn)(int16_t x, int16_t y, int16_t param,
                            const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);
    void (*actuate_teleporter_fn)(int16_t x, int16_t y, int16_t param,
                                  const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);
    void (*actuate_trickwall_fn)(int16_t x, int16_t y, int16_t param,
                                  const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);

    /* Step door (0x01) */
    void (*step_door_fn)(int16_t x, int16_t y, int16_t record_word, int16_t direction,
                         const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);

    /* Sound (0x15) */
    void (*play_sound)(void *ctx, int16_t sound_id);

    /* Weather (0x54) */
    void (*update_weather)(void *ctx, int16_t mode);

    /* Poison (0x4B) */
    void (*process_poison)(void *ctx, int16_t hero_index, int16_t amount);

    /* Timer 59 */
    uint8_t *redraw_byte_59;

    /* Move record rotate (0x5D) */
    void (*party_rotate)(void *ctx, int16_t direction);
    int16_t party_x;
    int16_t party_y;

    /* Alloc new creature (0x5E) */
    int16_t (*alloc_new_creature)(void *ctx, int16_t type, int16_t x_param,
                                   int16_t dir, int16_t x, int16_t y);
    int16_t (*rand_dir)(void *ctx);
    int16_t (*calc_vector_dir)(void *ctx, int16_t x1, int16_t y1, int16_t x2, int16_t y2);

    /* Hero state for 0x0C, 0x47, 0x48, 0x4B */
    int16_t heros_in_party;
    int16_t *hero_timeridx;   /* per hero */
    int16_t *hero_curHP;      /* per hero */
    uint16_t *hero_heroflag;  /* per hero */
    int16_t *hero_ench_power; /* per hero */
    int16_t *hero_poisoned;   /* per hero */
    int16_t *hero_poison;     /* per hero */
    uint8_t *ench_flag_counter; /* savegames1.b_02 */
    int16_t ench_flag_hero_slot; /* v1e0976 */
} DM2_V1_TimerDispatchWiringContext;

/* Populate dispatcher.handlers[] for all implemented timer types.
 * The wiring_ctx must outlive the dispatcher. */
void dm2_v1_timer_dispatch_wiring_init(
    DM2_V1_TimerDispatcher *dispatcher,
    DM2_V1_TimerDispatchWiringContext *wiring_ctx);

/* Count of timer types currently wired by this module. */
int dm2_v1_timer_dispatch_wiring_count(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TIMER_DISPATCH_WIRING_PC34_COMPAT_H */
