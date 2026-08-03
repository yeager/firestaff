#ifndef DM2_V1_TIMER_HANDLERS_SIMPLE_PC34_COMPAT_H
#define DM2_V1_TIMER_HANDLERS_SIMPLE_PC34_COMPAT_H

/* Simple DM2 runtime timer handlers — types that can be implemented
 * without deep runtime coupling.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp
 *
 * 0x46 — DM2_PROCESS_TIMER_LIGHT (lines ~4060-4080)
 * 0x47 — hero enchantment flag decrement (lines ~4082-4095)
 * 0x48 — enchantment power decrement (lines ~4097-4115)
 * 0x4B — DM2_PROCESS_POISON (lines ~4120-4145)
 * 0x5B — record byte@4 &= ~1 (lines ~4170-4175)
 * 0x5C — record byte@2 |= 1 (lines ~4177-4182) */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *ctx;
    /* Light: add delta to global light level. */
    void (*adjust_light)(void *ctx, int16_t delta);
    /* Hero ench flag: decrement hero[actor].ench_flag if > 0.
     * Returns the new value. */
    int (*decrement_hero_ench_flag)(void *ctx, int hero_idx);
    /* Hero ench power: subtract |valueA| from hero ench power.
     * actor is bitmask of heroes. */
    void (*decrement_hero_ench_power)(void *ctx, int hero_idx, int16_t amount);
    /* Poison: apply poison tick to hero[actor]. */
    void (*apply_poison_tick)(void *ctx, int hero_idx, int16_t amount);
    /* Hero alive check. */
    int (*hero_is_alive)(void *ctx, int hero_idx);
    /* Record bit operations. */
    void (*record_clear_bit)(void *ctx, uint16_t link);
    void (*record_set_bit)(void *ctx, uint16_t link);
    /* Re-enqueue timer. */
    void (*requeue_timer)(void *ctx, const uint8_t *timer_12b, uint32_t new_ticks);
    /* Get hero count. */
    int (*get_hero_count)(void *ctx);
} DM2_V1_SimpleTimerCallbacks;

typedef struct {
    int valid;
    int light_handled;
    int ench_flag_handled;
    int ench_power_handled;
    int poison_handled;
    int record_5b_handled;
    int record_5c_handled;
} DM2_V1_SimpleTimerReceipt;

/* Handle timer type 0x46 (light).
 * Source: c_tim_proc.cpp ~4060-4080. */
int dm2_v1_handle_timer_light(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb);

/* Handle timer type 0x47 (hero ench flag).
 * Source: c_tim_proc.cpp ~4082-4095. */
int dm2_v1_handle_timer_hero_ench_flag(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb);

/* Handle timer type 0x48 (ench power decrement).
 * Source: c_tim_proc.cpp ~4097-4115. */
int dm2_v1_handle_timer_ench_power(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb);

/* Handle timer type 0x4B (poison).
 * Source: c_tim_proc.cpp ~4120-4145. */
int dm2_v1_handle_timer_poison(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb);

/* Handle timer type 0x5B (record clear bit).
 * Source: c_tim_proc.cpp ~4170-4175. */
int dm2_v1_handle_timer_record_clear(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb);

/* Handle timer type 0x5C (record set bit).
 * Source: c_tim_proc.cpp ~4177-4182. */
int dm2_v1_handle_timer_record_set(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_TIMER_HANDLERS_SIMPLE_PC34_COMPAT_H */
