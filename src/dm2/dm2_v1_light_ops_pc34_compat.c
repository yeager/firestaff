/* DM2 V1 light operations — skproject c_light.cpp. */

#include "dm2_v1_light_ops_pc34_compat.h"
#include <stddef.h>

void dm2_v1_proceed_light(
    uint16_t light_type, int16_t intensity,
    const DM2_V1_ProceedLightCallbacks *cb, void *ctx)
{
    if (!cb)
        return;

    int dir_mult = 1;
    intensity = (int16_t)(intensity + 1);
    intensity = dm2_v1_between_value(32, 256, intensity);
    int16_t step = (int16_t)(intensity / 8);
    if (step < 8) step = 8;

    int16_t r2 = (int16_t)(step - 8);
    uint16_t delay;

    if (light_type == 0x06) {
        /* Darkness spell */
        delay = (uint16_t)(16 * r2 + 16);
        dir_mult = -2;
    } else if (light_type == 0x26) {
        /* Torch-class */
        delay = (uint16_t)(((step - 3) << 7) + 2000);
        step = (int16_t)(step >> 2);
        step = (int16_t)(step + 1);
    } else if (light_type == 0x27) {
        /* Bright light */
        delay = (uint16_t)((r2 << 9) + 10000);
    } else {
        return;
    }

    if (light_type != 0x06) {
        /* Non-darkness: halve and decrement */
        step = (int16_t)(step >> 1);
        step--;
    }

    /* Queue light timer (type 0x46) */
    int16_t timer_val;
    if (light_type != 0x06)
        timer_val = (int16_t)-step;
    else
        timer_val = step;

    cb->queue_light_timer(ctx, timer_val,
                          (uint32_t)delay + cb->game_tick);

    /* Apply initial light delta */
    if (step >= 0 && step < cb->light_table_size) {
        int16_t light_delta = (int16_t)(cb->light_table[step] * dir_mult);
        *cb->global_light = (int16_t)(*cb->global_light + light_delta);
    }

    cb->recalc_light(ctx);
}
