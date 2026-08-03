/* Test DM2 V1 actuator/relay operations (c_tim_proc.cpp). */

#include "dm2_v1_actuator_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Test flag ---- */
static int g_glob_val;
static int g_invoked_action;
static int g_invoked;

static int mock_get_glob(void *ctx, uint16_t idx)
{
    (void)ctx; (void)idx;
    return g_glob_val;
}

static void mock_invoke_act(void *ctx, const uint8_t *rec,
                            uint16_t action, uint16_t param)
{
    (void)ctx; (void)rec; (void)param;
    g_invoked = 1;
    g_invoked_action = action;
}

static void test_activate_test_flag(void)
{
    DM2_V1_TestFlagCallbacks cb = { mock_get_glob, mock_invoke_act };
    DM2_V1_ActuatorRecord act;
    memset(&act, 0, sizeof(act));
    /* glob_idx at word+2 bits 7+: set to index 5 = 5<<7 = 0x0280 */
    act.data[2] = 0x80;
    act.data[3] = 0x02;
    /* word+4 bit 2 clear => use timer_yb for action */
    act.data[4] = 0x00;

    /* Glob var set, expected set (bit 5 set in w4 => expected=1) */
    act.data[4] = 0x20;
    g_glob_val = 1;
    g_invoked = 0;
    dm2_v1_activate_test_flag(&act, 7, 1, &cb, NULL);
    assert(g_invoked == 1);
    assert(g_invoked_action == 7);

    printf("  PASS: activate_test_flag\n");
}

/* ---- Inverse flag ---- */
static uint16_t g_updated_idx, g_updated_val;
static int g_updated_op;

static void mock_update_glob(void *ctx, uint16_t idx, int op, uint16_t val)
{
    (void)ctx;
    g_updated_idx = idx;
    g_updated_op = op;
    g_updated_val = val;
}

static void test_activate_inverse_flag(void)
{
    DM2_V1_InverseFlagCallbacks cb = { mock_update_glob, mock_invoke_act };
    DM2_V1_ActuatorRecord act;
    memset(&act, 0, sizeof(act));
    act.data[2] = 0x80;
    act.data[3] = 0x02; /* glob_idx = 5 */
    act.data[4] = 0x20; /* bit 5 set => base=0 */

    g_invoked = 0;
    dm2_v1_activate_inverse_flag(&act, 2, &cb, NULL);
    assert(g_updated_idx == 5);
    assert(g_updated_val == 2); /* timer_yb + 0 */
    assert(g_invoked == 1);

    /* bit 5 clear => base=3 */
    act.data[4] = 0x00;
    dm2_v1_activate_inverse_flag(&act, 1, &cb, NULL);
    assert(g_updated_val == 4); /* 1 + 3 */

    printf("  PASS: activate_inverse_flag\n");
}

/* ---- Relay1 ---- */
static uint32_t g_msg_delay;
static uint16_t g_msg_action;
static int g_msg_sent;

static void mock_invoke_msg(void *ctx, uint16_t map, uint16_t x, uint16_t y,
                            uint16_t action, uint32_t delay)
{
    (void)ctx; (void)map; (void)x; (void)y;
    g_msg_action = action;
    g_msg_delay = delay;
    g_msg_sent = 1;
}

static void test_activate_relay1(void)
{
    DM2_V1_Relay1Callbacks cb = { 1000, mock_invoke_msg };
    DM2_V1_ActuatorRecord act;
    memset(&act, 0, sizeof(act));
    /* delay_base (glob_idx) = 10 = 10<<7 = 0x0500 at word+2 */
    act.data[2] = 0x00;
    act.data[3] = 0x05;

    g_msg_sent = 0;
    int r = dm2_v1_activate_relay1(&act, 3, 0, &cb, NULL);
    assert(r == 1);
    assert(g_msg_sent == 1);
    assert(g_msg_action == 3);

    /* Gate check: bit 4 set, bit 5 clear, yb != 0 => blocked */
    act.data[4] = 0x04;
    g_msg_sent = 0;
    r = dm2_v1_activate_relay1(&act, 2, 0, &cb, NULL);
    assert(r == 0);

    printf("  PASS: activate_relay1\n");
}

int main(void)
{
    printf("test_dm2_v1_actuator_ops:\n");
    test_activate_test_flag();
    test_activate_inverse_flag();
    test_activate_relay1();
    printf("All actuator_ops tests passed.\n");
    return 0;
}
