#ifndef FIRESTAFF_DM2_V1_LIGHT_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_LIGHT_OPS_PC34_COMPAT_H

/*
 * dm2_v1_light_ops_pc34_compat.h — DM2 V1 light operations from
 * skproject/SKULLWIN/c_light.cpp.
 *
 * Callback-based implementations of:
 *   DM2_PROCEED_LIGHT                    c_light.cpp:596
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_PROCEED_LIGHT (c_light.cpp:596) ----
 * Initiate a light spell effect: compute step, delay, and queue timer.
 * light_type: 6 = darkness, 0x26 = torch, 0x27 = bright.
 * intensity: current running value. */
typedef struct {
    int16_t *global_light;
    const int16_t *light_table;
    int light_table_size;
    uint32_t game_tick;
    void (*queue_light_timer)(void *ctx, int16_t value, uint32_t fire_tick);
    void (*recalc_light)(void *ctx);
} DM2_V1_ProceedLightCallbacks;

void dm2_v1_proceed_light(
    uint16_t light_type, int16_t intensity,
    const DM2_V1_ProceedLightCallbacks *cb, void *ctx);

static inline int16_t dm2_v1_between_value(int16_t lo, int16_t hi, int16_t val)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_LIGHT_OPS_PC34_COMPAT_H */
