/* DM2 V1 actuator/relay operations — skproject c_tim_proc.cpp. */

#include "dm2_v1_actuator_ops_pc34_compat.h"
#include <stddef.h>

void dm2_v1_activate_test_flag(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb, uint8_t trigger_type,
    const DM2_V1_TestFlagCallbacks *cb, void *ctx)
{
    if (!act || !cb)
        return;

    uint16_t glob_idx = dm2_v1_act_glob_idx(act);
    int glob_val = cb->get_glob_var(ctx, glob_idx);
    uint8_t flag_set = glob_val != 0 ? 1 : 0;

    /* word+4 bit 5 (0x20) = expected state */
    uint16_t w4 = dm2_v1_act_word(act, 4);
    uint8_t expected = (uint8_t)((w4 << 10) >> 15);

    /* Check trigger condition */
    if (flag_set == expected || (trigger_type != 0 && trigger_type != 2)) {
        if (flag_set != expected)
            return;
        if (trigger_type != 1)
            return;
    }

    uint16_t action = dm2_v1_act_action_value(act, timer_yb);
    cb->invoke_actuator(ctx, act->data, action, 0);
}

void dm2_v1_activate_inverse_flag(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb,
    const DM2_V1_InverseFlagCallbacks *cb, void *ctx)
{
    if (!act || !cb)
        return;

    uint16_t w4 = dm2_v1_act_word(act, 4);
    uint16_t base = (w4 & 0x20) ? 0 : 3;
    uint16_t value = (uint16_t)(timer_yb + base);

    uint16_t glob_idx = dm2_v1_act_glob_idx(act);
    cb->update_glob_var(ctx, glob_idx, 1, value);

    uint16_t action = dm2_v1_act_action_value(act, timer_yb);
    cb->invoke_actuator(ctx, act->data, action, 0);
}

int dm2_v1_activate_relay1(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb, int is_delayed,
    const DM2_V1_Relay1Callbacks *cb, void *ctx)
{
    if (!act || !cb)
        return 0;

    uint8_t b4 = act->data[4];
    if ((b4 & 0x04) != 0) {
        if ((b4 & 0x20) != 0 || timer_yb != 0) {
            if ((b4 & 0x20) == 0)
                return 0;
            if (timer_yb != 1)
                return 0;
        }
    }

    uint16_t delay_base = dm2_v1_act_glob_idx(act);
    uint16_t delay_shift = (dm2_v1_act_word(act, 4) << 5) >> 12;
    uint32_t fire_tick;

    if (is_delayed == 0) {
        fire_tick = cb->game_tick + delay_base;
        fire_tick += delay_shift;
    } else {
        uint32_t shifted = (uint32_t)delay_base << (delay_shift & 0xF);
        fire_tick = shifted + cb->game_tick;
    }

    uint16_t action = dm2_v1_act_action_value(act, timer_yb);

    /* Extract target from word+6 */
    uint16_t w6 = dm2_v1_act_word(act, 6);
    uint16_t target_y = (w6 << 10) >> 14;
    uint16_t target_map = w6 >> 11;
    uint16_t target_x = (w6 << 5) >> 11;

    cb->invoke_message(ctx, target_map, target_x, target_y, action, fire_tick);
    return 1;
}

int dm2_v1_activate_relay2(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb, int is_delayed,
    const DM2_V1_Relay1Callbacks *cb, void *ctx)
{
    return dm2_v1_activate_relay1(act, timer_yb, is_delayed, cb, ctx);
}
