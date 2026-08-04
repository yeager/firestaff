#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature_ai_pc34_compat.h"

/* ------------------------------------------------------------------ */
/* XACT dispatch tests                                                */
/* ------------------------------------------------------------------ */

static void test_dispatch_null_safety(void)
{
    int r = dm2_v1_xact_dispatch_action(63, NULL);
    assert(r == 0);
    printf("  PASS: dispatch_null_safety\n");
}

static void test_dispatch_out_of_range(void)
{
    DM2_V1_XactDispatchReceipt receipt;

    /* Below range: action 62 -> opt -1 */
    int r = dm2_v1_xact_dispatch_action(62, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    /* Above range: action 99 -> opt 36 */
    r = dm2_v1_xact_dispatch_action(99, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    printf("  PASS: dispatch_out_of_range\n");
}

static void test_dispatch_xact_56_stop(void)
{
    DM2_V1_XactDispatchReceipt receipt;
    /* XACT 56 = opt 0, action_byte = 63 */
    int r = dm2_v1_xact_dispatch_action(63, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.handler_id == 56);
    assert(receipt.sets_ret == 1);
    printf("  PASS: dispatch_xact_56_stop\n");
}

static void test_dispatch_xact_58_inline(void)
{
    DM2_V1_XactDispatchReceipt receipt;
    /* XACT 58 = opt 2, action_byte = 65 */
    int r = dm2_v1_xact_dispatch_action(65, &receipt);
    assert(r == 1);
    assert(receipt.handler_id == 58);
    assert(receipt.inline_action == 1);
    assert(receipt.inline_b_1a == 19);
    printf("  PASS: dispatch_xact_58_inline\n");
}

static void test_dispatch_xact_60_inline(void)
{
    DM2_V1_XactDispatchReceipt receipt;
    /* XACT 60 = opt 4, action_byte = 67 */
    int r = dm2_v1_xact_dispatch_action(67, &receipt);
    assert(r == 1);
    assert(receipt.inline_action == 1);
    assert(receipt.inline_b_1a == 0);
    printf("  PASS: dispatch_xact_60_inline\n");
}

static void test_dispatch_xact_61_nop(void)
{
    DM2_V1_XactDispatchReceipt receipt;
    /* XACT 61 = opt 5, action_byte = 68 */
    int r = dm2_v1_xact_dispatch_action(68, &receipt);
    assert(r == 1);
    assert(receipt.handler_id == 61);
    assert(receipt.sets_ret == 0);
    assert(receipt.inline_action == 0);
    printf("  PASS: dispatch_xact_61_nop\n");
}

static void test_dispatch_xact_76_backward(void)
{
    DM2_V1_XactDispatchReceipt receipt;
    /* XACT 76 = opt 20, action_byte = 83 */
    int r = dm2_v1_xact_dispatch_action(83, &receipt);
    assert(r == 1);
    assert(receipt.handler_id == 76);
    assert(receipt.inline_action == 1);
    assert(receipt.inline_v1e0572 == -1);
    assert(receipt.inline_v1e0574 == 0);
    assert(receipt.sets_ret == 1);
    printf("  PASS: dispatch_xact_76_backward\n");
}

static void test_dispatch_xact_90_random(void)
{
    DM2_V1_XactDispatchReceipt receipt;
    /* XACT 90 = opt 34, action_byte = 97 */
    int r = dm2_v1_xact_dispatch_action(97, &receipt);
    assert(r == 1);
    assert(receipt.handler_id == 90);
    assert(receipt.sets_ret == 1);
    printf("  PASS: dispatch_xact_90\n");
}

static void test_dispatch_xact_91_item(void)
{
    DM2_V1_XactDispatchReceipt receipt;
    /* XACT 91 = opt 35, action_byte = 98 */
    int r = dm2_v1_xact_dispatch_action(98, &receipt);
    assert(r == 1);
    assert(receipt.handler_id == 91);
    assert(receipt.sets_ret == 1);
    printf("  PASS: dispatch_xact_91\n");
}

/* ------------------------------------------------------------------ */
/* XACT 72/87/88 tests                                               */
/* ------------------------------------------------------------------ */

static void test_xact_72_null_safety(void)
{
    DM2_V1_Xact72_87_88Receipt receipt;
    int r = dm2_v1_proceed_xact_72_87_88(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_proceed_xact_72_87_88(NULL, NULL);
    assert(r == 0);
    printf("  PASS: xact_72_null_safety\n");
}

static void test_xact_72_uses_v1e0572(void)
{
    DM2_V1_Xact72_87_88Request req;
    DM2_V1_Xact72_87_88Receipt receipt;

    req.v1e0572 = 5;
    req.v1e07d8_w04 = 10;
    int r = dm2_v1_proceed_xact_72_87_88(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.b_1a == 5);
    printf("  PASS: xact_72_uses_v1e0572\n");
}

static void test_xact_72_fallback_to_w04(void)
{
    DM2_V1_Xact72_87_88Request req;
    DM2_V1_Xact72_87_88Receipt receipt;

    /* v1e0572 low byte = 0xFF = -1 as int8_t */
    req.v1e0572 = 0x00FF; /* low byte -1 */
    req.v1e07d8_w04 = 42;
    int r = dm2_v1_proceed_xact_72_87_88(&req, &receipt);
    assert(r == 1);
    assert(receipt.b_1a == 42);
    printf("  PASS: xact_72_fallback_to_w04\n");
}

/* ------------------------------------------------------------------ */
/* XACT 86 tests                                                     */
/* ------------------------------------------------------------------ */

static void test_xact_86_null_safety(void)
{
    DM2_V1_Xact86Receipt receipt;
    int r = dm2_v1_proceed_xact_86(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact_86_null_safety\n");
}

static void test_xact_86_sets_fields(void)
{
    DM2_V1_Xact86Request req;
    DM2_V1_Xact86Receipt receipt;

    req.v1e0572 = 3;       /* b_1a = 3 + 61 = 64 */
    req.v1e07d8_w04 = 7;   /* b_20 = 7 */
    req.v1e07d8_w06 = 12;  /* b_1e = 12 */

    int r = dm2_v1_proceed_xact_86(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.b_20 == 7);
    assert(receipt.b_1e == 12);
    assert(receipt.b_1a == 64);
    printf("  PASS: xact_86_sets_fields\n");
}

/* ------------------------------------------------------------------ */
/* XACT 90 tests                                                     */
/* ------------------------------------------------------------------ */

static void test_xact_90_null_safety(void)
{
    DM2_V1_Xact90Receipt receipt;
    int r = dm2_v1_proceed_xact_90(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact_90_null_safety\n");
}

static void test_xact_90_pass(void)
{
    DM2_V1_Xact90Request req;
    DM2_V1_Xact90Receipt receipt;

    req.v1e0572 = 80;
    req.rand_value = 50;  /* 80 > 50 => pass (-2) */
    int r = dm2_v1_proceed_xact_90(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -2);
    printf("  PASS: xact_90_pass\n");
}

static void test_xact_90_fail(void)
{
    DM2_V1_Xact90Request req;
    DM2_V1_Xact90Receipt receipt;

    req.v1e0572 = 30;
    req.rand_value = 50;  /* 30 <= 50 => fail (-3) */
    int r = dm2_v1_proceed_xact_90(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -3);
    printf("  PASS: xact_90_fail\n");
}

static void test_xact_90_equal(void)
{
    DM2_V1_Xact90Request req;
    DM2_V1_Xact90Receipt receipt;

    req.v1e0572 = 50;
    req.rand_value = 50;  /* 50 > 50 is false => fail (-3) */
    int r = dm2_v1_proceed_xact_90(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -3);
    printf("  PASS: xact_90_equal\n");
}

/* ------------------------------------------------------------------ */
/* XACT 91 tests                                                     */
/* ------------------------------------------------------------------ */

static int16_t mock_can_handle_always_fail(void *ctx, int16_t item_type,
                                            uint16_t possession_w00,
                                            int32_t mask)
{
    (void)ctx; (void)item_type; (void)possession_w00; (void)mask;
    return -2;
}

static int16_t mock_can_handle_always_pass(void *ctx, int16_t item_type,
                                            uint16_t possession_w00,
                                            int32_t mask)
{
    (void)ctx; (void)item_type; (void)possession_w00; (void)mask;
    return 0;
}

static void test_xact_91_null_safety(void)
{
    DM2_V1_Xact91Receipt receipt;
    int r = dm2_v1_proceed_xact_91(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact_91_null_safety\n");
}

static void test_xact_91_no_callback(void)
{
    DM2_V1_Xact91Request req;
    DM2_V1_Xact91Receipt receipt;
    memset(&req, 0, sizeof(req));
    req.can_handle_item = NULL;

    int r = dm2_v1_proceed_xact_91(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact_91_no_callback\n");
}

static void test_xact_91_both_fail(void)
{
    DM2_V1_Xact91Request req;
    DM2_V1_Xact91Receipt receipt;
    memset(&req, 0, sizeof(req));

    req.can_handle_item = mock_can_handle_always_fail;
    int r = dm2_v1_proceed_xact_91(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -3);
    printf("  PASS: xact_91_both_fail\n");
}

static void test_xact_91_first_passes(void)
{
    DM2_V1_Xact91Request req;
    DM2_V1_Xact91Receipt receipt;
    memset(&req, 0, sizeof(req));

    req.can_handle_item = mock_can_handle_always_pass;
    int r = dm2_v1_proceed_xact_91(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -2);
    printf("  PASS: xact_91_first_passes\n");
}

/* ------------------------------------------------------------------ */

int main(void)
{
    printf("test_dm2_v1_xact_dispatch_pc34_compat:\n");

    test_dispatch_null_safety();
    test_dispatch_out_of_range();
    test_dispatch_xact_56_stop();
    test_dispatch_xact_58_inline();
    test_dispatch_xact_60_inline();
    test_dispatch_xact_61_nop();
    test_dispatch_xact_76_backward();
    test_dispatch_xact_90_random();
    test_dispatch_xact_91_item();

    test_xact_72_null_safety();
    test_xact_72_uses_v1e0572();
    test_xact_72_fallback_to_w04();

    test_xact_86_null_safety();
    test_xact_86_sets_fields();

    test_xact_90_null_safety();
    test_xact_90_pass();
    test_xact_90_fail();
    test_xact_90_equal();

    test_xact_91_null_safety();
    test_xact_91_no_callback();
    test_xact_91_both_fail();
    test_xact_91_first_passes();

    printf("All XACT dispatch tests passed.\n");
    return 0;
}
