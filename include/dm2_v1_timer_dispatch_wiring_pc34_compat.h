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
 * Currently wired timer types:
 *   0x02 DESTROY_DOOR
 *   0x0E PROCESS_0E
 *   0x3D PROCESS_3D
 *   0x46 LIGHT
 *   0x55 ORNATE_ANIMATOR
 *   0x56 TICK_GENERATOR
 *   0x58 RELEASE_DOOR_BUTTON
 *
 * Source: skproject c_tim_proc.cpp DM2_PROCEED_TIMERS
 */

#include "dm2_v1_proceed_timers_pc34_compat.h"

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
