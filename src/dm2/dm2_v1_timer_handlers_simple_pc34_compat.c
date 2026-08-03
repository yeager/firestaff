/* Simple DM2 runtime timer handlers.
 * Source: skproject/SKULLWIN/c_tim_proc.cpp */

#include "dm2_v1_timer_handlers_simple_pc34_compat.h"
#include "dm2_v1_save_post_load_global_effects_pc34_compat.h"
#include <stdlib.h>

#define TIMER_OFF_TYPE   4u
#define TIMER_OFF_ACTOR  5u
#define TIMER_OFF_VALUEA 6u

static int16_t read_valueA(const uint8_t *t)
{
    return (int16_t)((uint16_t)t[TIMER_OFF_VALUEA] |
                     ((uint16_t)t[TIMER_OFF_VALUEA + 1] << 8));
}

int dm2_v1_handle_timer_light(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb)
{
    if (!timer_12b || !cb) return 0;
    int16_t valueA = read_valueA(timer_12b);
    int idx = abs((int)valueA);
    if (idx > 15) idx = 15;
    int16_t delta = dm2_v1_light_table[idx];
    if (valueA < 0) delta = (int16_t)-delta;
    if (cb->adjust_light)
        cb->adjust_light(cb->ctx, delta);
    return 1;
}

int dm2_v1_handle_timer_hero_ench_flag(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb)
{
    if (!timer_12b || !cb) return 0;
    uint8_t actor = timer_12b[TIMER_OFF_ACTOR];
    if (cb->decrement_hero_ench_flag)
        cb->decrement_hero_ench_flag(cb->ctx, (int)actor);
    return 1;
}

int dm2_v1_handle_timer_ench_power(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb)
{
    if (!timer_12b || !cb) return 0;
    uint8_t actor = timer_12b[TIMER_OFF_ACTOR];
    int16_t valueA = read_valueA(timer_12b);
    int hero_count = cb->get_hero_count ? cb->get_hero_count(cb->ctx) : 4;
    for (int h = 0; h < hero_count && h < 4; h++) {
        if (actor & (1u << h)) {
            if (cb->hero_is_alive && cb->hero_is_alive(cb->ctx, h)) {
                if (cb->decrement_hero_ench_power)
                    cb->decrement_hero_ench_power(cb->ctx, h, valueA);
            }
        }
    }
    return 1;
}

int dm2_v1_handle_timer_poison(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb)
{
    if (!timer_12b || !cb) return 0;
    uint8_t actor = timer_12b[TIMER_OFF_ACTOR];
    int16_t valueA = read_valueA(timer_12b);
    if (cb->hero_is_alive && cb->hero_is_alive(cb->ctx, (int)actor)) {
        if (cb->apply_poison_tick)
            cb->apply_poison_tick(cb->ctx, (int)actor, valueA);
    }
    return 1;
}

int dm2_v1_handle_timer_record_clear(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb)
{
    if (!timer_12b || !cb) return 0;
    int16_t valueA = read_valueA(timer_12b);
    if (cb->record_clear_bit)
        cb->record_clear_bit(cb->ctx, (uint16_t)valueA);
    return 1;
}

int dm2_v1_handle_timer_record_set(
    const uint8_t *timer_12b,
    const DM2_V1_SimpleTimerCallbacks *cb)
{
    if (!timer_12b || !cb) return 0;
    int16_t valueA = read_valueA(timer_12b);
    if (cb->record_set_bit)
        cb->record_set_bit(cb->ctx, (uint16_t)valueA);
    return 1;
}
