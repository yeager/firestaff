/* DM2 post-load global effect timer accumulator.
 * Source: sksvgame.cpp:1041-1106 (DM2_PROCEED_GLOBAL_EFFECT_TIMERS).
 *
 * Walks timer array after load and accumulates:
 * - 0x0E: spell effect (delegated to a complete owner)
 * - 0x46: savegames1.w_00 light contribution from table1d6702
 * - 0x47: attack count++
 * - 0x48: enchantment power per hero (actor bitmask, bits 0-3)
 * - 0x4B: poisoned++ and poison per hero */

#include "dm2_v1_save_post_load_global_effects_pc34_compat.h"
#include <string.h>

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
            /* PROCESS_TIMER_0E(timer, 3) has spell and record side effects;
             * do not turn it into a fabricated accumulator. */
            if (!cb->process_timer_0e ||
                !cb->process_timer_0e(cb->ctx, tim)) {
                local.blocked_unimplemented_0e = 1;
                *receipt = local;
                return -1;
            }
            local.timer_0e_processed++;
            break;

        case 0x46: {
            /* sksvgame.cpp:1063-1072: A==0 and values outside [-15,15]
             * do nothing. Positive A subtracts twice table[A]; negative A
             * adds table[-A].  Preserve the original i16 wrap. */
            if (valueA != 0 && valueA >= -15 && valueA <= 15) {
                int16_t delta = valueA >= 0
                    ? (int16_t)(-2 * dm2_v1_light_table[valueA])
                    : dm2_v1_light_table[-valueA];
                local.light_accumulator =
                    (int16_t)(local.light_accumulator + delta);
            }
            break;
        }

        case 0x47:
            /* Source: sksvgame.cpp:1074-1078. Attack count increment. */
            local.attack_count++;
            break;

        case 0x48:
            /* Source: sksvgame.cpp:1080-1093.
             * Enchantment power per hero. Actor is a bitmask (bits 0-3). */
            if (!cb->hero_is_alive || !cb->add_hero_ench_power) {
                *receipt = local;
                return -1;
            }
            for (int h = 0; h < hero_count && h < 4; h++) {
                if (actor & (1u << h)) {
                    if (cb->hero_is_alive(cb->ctx, h)) {
                        cb->add_hero_ench_power(cb->ctx, h, valueA);
                        local.ench_power_applied++;
                    }
                }
            }
            break;

        case 0x4B:
            /* sksvgame.cpp:1095-1104 does not test curHP. Actor is a direct
             * party index; bad authenticated data is rejected rather than
             * indexing outside the retained c_hero array. */
            if (actor >= (uint8_t)hero_count || actor >= 4u ||
                !cb->increment_hero_poisoned || !cb->add_hero_poison) {
                local.invalid_actor = 1;
                *receipt = local;
                return -1;
            }
            cb->increment_hero_poisoned(cb->ctx, (int)actor);
            cb->add_hero_poison(cb->ctx, (int)actor, valueA);
            local.poison_applied++;
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
