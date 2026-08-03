#ifndef DM2_V1_SAVE_POST_LOAD_GLOBAL_EFFECTS_PC34_COMPAT_H
#define DM2_V1_SAVE_POST_LOAD_GLOBAL_EFFECTS_PC34_COMPAT_H

/* DM2 post-load global effect timer accumulator.
 * Source: sksvgame.cpp:1041-1106 (DM2_PROCEED_GLOBAL_EFFECT_TIMERS).
 *
 * After loading a save game, walks the timer array and accumulates
 * global effect state from active timers:
 * - Type 0x0E: process spell effect (delegated to callback)
 * - Type 0x46: light level delta from table1d6702
 * - Type 0x47: attack counter increment
 * - Type 0x48: enchantment power per hero (from actor bitmask)
 * - Type 0x4B: poison per hero */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Light level lookup table (table1d6702).
 * Source: dm2data.cpp — 16 entries, indexed by abs(valueA). */
static const int16_t dm2_v1_light_table[16] = {
    0, 8, 24, 56, 120, 248, 504, 1016,
    2040, 4088, 8184, 16376, 32760, 32760, 32760, 32760
};

typedef struct {
    void *ctx;
    /* Process timer 0E effect.  mode=3 (post-load).
     * timer: raw 12-byte timer record. */
    void (*process_timer_0e)(void *ctx, const uint8_t *timer);
    /* Add enchantment power to hero.  hero_idx: 0..3, power: valueA. */
    void (*add_hero_ench_power)(void *ctx, int hero_idx, int16_t power);
    /* Add poison to hero.  hero_idx: actor, count: 1, amount: valueA. */
    void (*add_hero_poison)(void *ctx, int hero_idx, int16_t amount);
    /* Check if hero is alive.  Returns nonzero if curHP != 0. */
    int (*hero_is_alive)(void *ctx, int hero_idx);
} DM2_V1_GlobalEffectCallbacks;

typedef struct {
    int valid;
    int timers_scanned;
    int16_t light_accumulator;
    int attack_count;
    int ench_power_applied;
    int poison_applied;
    int timer_0e_processed;
} DM2_V1_GlobalEffectReceipt;

/* Accumulate global effects from timer array.
 * timer_array: packed 12-byte timer records.
 * Returns 0 on success. */
int dm2_v1_post_load_global_effects(
    const uint8_t *timer_array, int num_timers,
    int hero_count,
    const DM2_V1_GlobalEffectCallbacks *cb,
    DM2_V1_GlobalEffectReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_POST_LOAD_GLOBAL_EFFECTS_PC34_COMPAT_H */
