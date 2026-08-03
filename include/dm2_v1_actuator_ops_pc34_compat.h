#ifndef FIRESTAFF_DM2_V1_ACTUATOR_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_ACTUATOR_OPS_PC34_COMPAT_H

/*
 * dm2_v1_actuator_ops_pc34_compat.h — DM2 V1 actuator/relay operations from
 * skproject/SKULLWIN/c_tim_proc.cpp.
 *
 * Callback-based implementations of:
 *   DM2_ACTIVATE_TEST_FLAG              c_tim_proc.cpp:1405
 *   DM2_ACTIVATE_INVERSE_FLAG           c_tim_proc.cpp:1450
 *   DM2_ACTIVATE_RELAY1                 c_tim_proc.cpp:1174
 *   DM2_ACTIVATE_RELAY2                 c_tim_proc.cpp:1249
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Common actuator record layout:
 * word+2 bits 7..15: glob_var_index (or delay base)
 * word+4 bit 2: use_timer_yB for action value (else extract from word+4)
 * word+4 bit 5: invert/scope flag
 * word+4 bits 5..8: action field (shifted)
 * word+6: target coordinates for messages
 */

/* Actuator record accessor */
typedef struct {
    uint8_t data[8];
} DM2_V1_ActuatorRecord;

static inline uint16_t dm2_v1_act_word(const DM2_V1_ActuatorRecord *r, int off)
{
    return (uint16_t)(r->data[off] | (r->data[off + 1] << 8));
}

static inline uint16_t dm2_v1_act_glob_idx(const DM2_V1_ActuatorRecord *r)
{
    return dm2_v1_act_word(r, 2) >> 7;
}

static inline uint16_t dm2_v1_act_action_value(const DM2_V1_ActuatorRecord *r,
                                                uint8_t timer_yb)
{
    if ((r->data[4] & 0x04) == 0)
        return timer_yb;
    return (dm2_v1_act_word(r, 4) << 11) >> 14;
}

/* ---- DM2_ACTIVATE_TEST_FLAG (c_tim_proc.cpp:1405) ----
 * Test a global variable flag and invoke actuator if condition met. */
typedef struct {
    int (*get_glob_var)(void *ctx, uint16_t index);
    void (*invoke_actuator)(void *ctx, const uint8_t *act_rec,
                            uint16_t action, uint16_t param);
} DM2_V1_TestFlagCallbacks;

void dm2_v1_activate_test_flag(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb, uint8_t trigger_type,
    const DM2_V1_TestFlagCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_INVERSE_FLAG (c_tim_proc.cpp:1450) ----
 * Update a global variable and invoke actuator. */
typedef struct {
    void (*update_glob_var)(void *ctx, uint16_t index, int op, uint16_t value);
    void (*invoke_actuator)(void *ctx, const uint8_t *act_rec,
                            uint16_t action, uint16_t param);
} DM2_V1_InverseFlagCallbacks;

void dm2_v1_activate_inverse_flag(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb,
    const DM2_V1_InverseFlagCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_RELAY1 (c_tim_proc.cpp:1174) ----
 * Compute delay and invoke a timed message to another actuator. */
typedef struct {
    uint32_t game_tick;
    void (*invoke_message)(void *ctx, uint16_t map, uint16_t x, uint16_t y,
                           uint16_t action, uint32_t delay);
} DM2_V1_Relay1Callbacks;

int dm2_v1_activate_relay1(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb, int is_delayed,
    const DM2_V1_Relay1Callbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_RELAY2 (c_tim_proc.cpp:1249) ----
 * Similar to relay1 but with different delay computation. */
int dm2_v1_activate_relay2(
    const DM2_V1_ActuatorRecord *act, uint8_t timer_yb, int is_delayed,
    const DM2_V1_Relay1Callbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ACTUATOR_OPS_PC34_COMPAT_H */
