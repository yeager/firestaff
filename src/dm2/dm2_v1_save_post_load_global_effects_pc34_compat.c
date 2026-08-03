/* DM2 post-load global effect timer accumulator.
 * Source: sksvgame.cpp:1041-1106 (DM2_PROCEED_GLOBAL_EFFECT_TIMERS).
 *
 * Walks timer array after load and accumulates:
 * - 0x0E: spell effect (delegated)
 * - 0x46: light level from table1d6702[abs(valueA)]
 * - 0x47: attack count++
 * - 0x48: enchantment power per hero (actor bitmask, bits 0-3)
 * - 0x4B: poison per hero */

#include "dm2_v1_save_post_load_global_effects_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

#define TIMER_SIZE 12u
#define TIMER_OFF_TYPE   4u
#define TIMER_OFF_ACTOR  5u
#define TIMER_OFF_VALUEA 6u

int dm2_v1_post_load_global_effects(
    const uint8_t *timer_array, int num_timers,
    int hero_count,
    const DM2_V1_GlobalEffectCallbacks *cb,
    DM2_V1_GlobalEffectReceipt *receipt)
{
    DM2_V1_GlobalEffectReceipt local;
    memset(&local, 0, sizeof(local));
    if (receipt) memset(receipt, 0, sizeof(*receipt));

    if (!cb || !receipt) {
        if (receipt) receipt->valid = 0;
        return -1;
    }

    if (!timer_array || num_timers <= 0) {
        local.valid = 1;
        *receipt = local;
        return 0;
    }

    for (int t = 0; t < num_timers; t++) {
        const uint8_t *tim = timer_array + (size_t)t * TIMER_SIZE;
        uint8_t type = tim[TIMER_OFF_TYPE];
        uint8_t actor = tim[TIMER_OFF_ACTOR];
        int16_t valueA = (int16_t)((uint16_t)tim[TIMER_OFF_VALUEA] |
                                   ((uint16_t)tim[TIMER_OFF_VALUEA + 1] << 8));

        switch (type) {
        case 0x0E:
            if (cb->process_timer_0e)
                cb->process_timer_0e(cb->ctx, tim);
            local.timer_0e_processed++;
            break;

        case 0x46: {
            /* Source: sksvgame.cpp:1063-1072.
             * Light delta from table1d6702[abs(valueA)].
             * Negative valueA subtracts; positive adds. */
            int idx = abs((int)valueA);
            if (idx > 15) idx = 15;
            int16_t delta = dm2_v1_light_table[idx];
            if (valueA < 0) delta = (int16_t)-delta;
            local.light_accumulator = (int16_t)(local.light_accumulator + delta);
            break;
        }

        case 0x47:
            /* Source: sksvgame.cpp:1074-1078. Attack count increment. */
            local.attack_count++;
            break;

        case 0x48:
            /* Source: sksvgame.cpp:1080-1093.
             * Enchantment power per hero. Actor is a bitmask (bits 0-3). */
            for (int h = 0; h < hero_count && h < 4; h++) {
                if (actor & (1u << h)) {
                    if (cb->hero_is_alive && cb->hero_is_alive(cb->ctx, h)) {
                        if (cb->add_hero_ench_power)
                            cb->add_hero_ench_power(cb->ctx, h, valueA);
                        local.ench_power_applied++;
                    }
                }
            }
            break;

        case 0x4B:
            /* Source: sksvgame.cpp:1095-1104.
             * Poison per hero. Actor is direct hero index. */
            if (actor < (uint8_t)hero_count && actor < 4) {
                if (cb->hero_is_alive && cb->hero_is_alive(cb->ctx, (int)actor)) {
                    if (cb->add_hero_poison)
                        cb->add_hero_poison(cb->ctx, (int)actor, valueA);
                    local.poison_applied++;
                }
            }
            break;

        default:
            break;
        }
        local.timers_scanned++;
    }

    local.valid = 1;
    *receipt = local;
    return 0;
}
