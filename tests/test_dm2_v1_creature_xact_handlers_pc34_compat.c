#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature_xact_handlers_pc34_compat.h"

/* ================================================================== */
/* XACT 56 — Move forward                                             */
/* ================================================================== */

static void test_xact56_null_safety(void)
{
    DM2_V1_XactMoveForwardReceipt receipt;
    int r = dm2_v1_xact_move_forward(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_xact_move_forward(NULL, NULL);
    assert(r == 0);
    printf("  PASS: xact56_null_safety\n");
}

static void test_xact56_no_callback(void)
{
    DM2_V1_XactMoveForwardReceipt receipt;
    DM2_V1_XactMoveForwardRequest req;
    memset(&req, 0, sizeof(req));
    int r = dm2_v1_xact_move_forward(&req, &receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact56_no_callback\n");
}

/* ================================================================== */
/* XACT 57 — Random turn                                              */
/* ================================================================== */

static void test_xact57_null_safety(void)
{
    DM2_V1_XactRandomTurnReceipt receipt;
    int r = dm2_v1_xact_random_turn(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact57_null_safety\n");
}

/* ================================================================== */
/* XACT 59/76 — Move to target                                        */
/* ================================================================== */

static void test_xact59_null_safety(void)
{
    DM2_V1_XactMoveToTargetReceipt receipt;
    int r = dm2_v1_xact_move_to_target(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact59_null_safety\n");
}

static void test_xact59_resolve_v1e0572(void)
{
    DM2_V1_XactMoveToTargetReceipt receipt;
    DM2_V1_XactMoveToTargetRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e0572 = -1;
    req.v1e07d8_w04 = 42;
    req.v1e0574 = 0; /* skip item check */
    int r = dm2_v1_xact_move_to_target(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    /* fail_closed because needs live 19f0_2165 */
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact59_resolve_v1e0572\n");
}

/* ================================================================== */
/* XACT 63 — Cast spell                                               */
/* ================================================================== */

static void test_xact63_null_safety(void)
{
    DM2_V1_XactCastSpellReceipt receipt;
    int r = dm2_v1_xact_cast_spell(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact63_null_safety\n");
}

static void test_xact63_no_spell(void)
{
    DM2_V1_XactCastSpellReceipt receipt;
    DM2_V1_XactCastSpellRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e0572 = -1; /* no spell */
    int r = dm2_v1_xact_cast_spell(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.result == -3);
    printf("  PASS: xact63_no_spell\n");
}

/* ================================================================== */
/* XACT 64 — Shoot missile                                            */
/* ================================================================== */

static void test_xact64_null_safety(void)
{
    DM2_V1_XactShootMissileReceipt receipt;
    int r = dm2_v1_xact_shoot_missile(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact64_null_safety\n");
}

static void test_xact64_no_possession(void)
{
    DM2_V1_XactShootMissileReceipt receipt;
    DM2_V1_XactShootMissileRequest req;
    memset(&req, 0, sizeof(req));
    req.possession_w00 = 0xFFFE; /* -2 = no possession */
    int r = dm2_v1_xact_shoot_missile(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -3);
    printf("  PASS: xact64_no_possession\n");
}

static void test_xact64_no_missile_flag(void)
{
    DM2_V1_XactShootMissileReceipt receipt;
    DM2_V1_XactShootMissileRequest req;
    memset(&req, 0, sizeof(req));
    req.possession_w00 = 0x1000;
    req.v1e057c = 0; /* bit 3 not set */
    int r = dm2_v1_xact_shoot_missile(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -3);
    printf("  PASS: xact64_no_missile_flag\n");
}

/* ================================================================== */
/* XACT 65 — Flee                                                     */
/* ================================================================== */

static void test_xact65_null_safety(void)
{
    DM2_V1_XactFleeReceipt receipt;
    int r = dm2_v1_xact_flee(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact65_null_safety\n");
}

/* ================================================================== */
/* XACT 66 — Attack ranged                                            */
/* ================================================================== */

static void test_xact66_null_safety(void)
{
    DM2_V1_XactAttackRangedReceipt receipt;
    int r = dm2_v1_xact_attack_ranged(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact66_null_safety\n");
}

/* ================================================================== */
/* XACT 67 — Combat evaluation                                        */
/* ================================================================== */

static void test_xact67_null_safety(void)
{
    DM2_V1_XactCombatEvaluationReceipt receipt;
    int r = dm2_v1_xact_combat_evaluation(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact67_null_safety\n");
}

/* ================================================================== */
/* XACT 68 — Guard position                                           */
/* ================================================================== */

static void test_xact68_null_safety(void)
{
    DM2_V1_XactGuardPositionReceipt receipt;
    int r = dm2_v1_xact_guard_position(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact68_null_safety\n");
}

/* ================================================================== */
/* XACT 69 — Emit sound (fully implemented)                           */
/* ================================================================== */

static void test_xact69_null_safety(void)
{
    DM2_V1_XactEmitSoundReceipt receipt;
    int r = dm2_v1_xact_emit_sound(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact69_null_safety\n");
}

static void test_xact69_direction_north(void)
{
    DM2_V1_XactEmitSoundReceipt receipt;
    DM2_V1_XactEmitSoundRequest req;
    memset(&req, 0, sizeof(req));
    req.pos_x = 10;
    req.pos_y = 10;
    req.direction = 0; /* North: dx=0, dy=-1 */
    req.v1e0572 = 1;
    int r = dm2_v1_xact_emit_sound(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.target_x == 10);     /* 10 + 0 */
    assert(receipt.target_y == 9);      /* 10 + (-1) */
    assert(receipt.b_1d == 1);
    assert(receipt.b_1a == 22);         /* (1==1 ? 1 : 0) + 21 = 22 */
    printf("  PASS: xact69_direction_north\n");
}

static void test_xact69_direction_east(void)
{
    DM2_V1_XactEmitSoundReceipt receipt;
    DM2_V1_XactEmitSoundRequest req;
    memset(&req, 0, sizeof(req));
    req.pos_x = 5;
    req.pos_y = 5;
    req.direction = 1; /* East: dx=1, dy=0 */
    req.v1e0572 = 0;
    int r = dm2_v1_xact_emit_sound(&req, &receipt);
    assert(r == 1);
    assert(receipt.target_x == 6);
    assert(receipt.target_y == 5);
    assert(receipt.b_1d == 0);
    assert(receipt.b_1a == 21);         /* (0==1 ? 1 : 0) + 21 = 21 */
    printf("  PASS: xact69_direction_east\n");
}

static void test_xact69_direction_wrap(void)
{
    DM2_V1_XactEmitSoundReceipt receipt;
    DM2_V1_XactEmitSoundRequest req;
    memset(&req, 0, sizeof(req));
    req.pos_x = 0;
    req.pos_y = 0;
    req.direction = 0; /* North: dy=-1 wraps to 31 */
    req.v1e0572 = 5;
    int r = dm2_v1_xact_emit_sound(&req, &receipt);
    assert(r == 1);
    assert(receipt.target_x == 0);
    assert(receipt.target_y == 31);     /* (0 + (-1)) & 0x1f = 31 */
    assert(receipt.b_1d == 5);
    assert(receipt.b_1a == 21);
    printf("  PASS: xact69_direction_wrap\n");
}

static void test_xact69_invalid_direction(void)
{
    DM2_V1_XactEmitSoundReceipt receipt;
    DM2_V1_XactEmitSoundRequest req;
    memset(&req, 0, sizeof(req));
    req.direction = 5; /* invalid */
    int r = dm2_v1_xact_emit_sound(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact69_invalid_direction\n");
}

/* ================================================================== */
/* XACT 70 — Face party                                               */
/* ================================================================== */

static void test_xact70_null_safety(void)
{
    DM2_V1_XactFacePartyReceipt receipt;
    int r = dm2_v1_xact_face_party(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact70_null_safety\n");
}

/* ================================================================== */
/* XACT 71 — Patrol                                                   */
/* ================================================================== */

static void test_xact71_null_safety(void)
{
    DM2_V1_XactPatrolReceipt receipt;
    int r = dm2_v1_xact_patrol(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact71_null_safety\n");
}

/* ================================================================== */
/* XACT 73 — Manage flags                                             */
/* ================================================================== */

static void test_xact73_null_safety(void)
{
    DM2_V1_XactManageFlagsReceipt receipt;
    int r = dm2_v1_xact_manage_flags(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact73_null_safety\n");
}

static void test_xact73_clear_flag(void)
{
    DM2_V1_XactManageFlagsReceipt receipt;
    DM2_V1_XactManageFlagsRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e0574 = 0;        /* op = 0: clear */
    req.v1e0572 = 3;        /* bit 3 */
    req.v1e054e_w0a = 0xFF; /* all bits set */
    int r = dm2_v1_xact_manage_flags(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.w_0a == (int16_t)(0xFF & ~(1 << 3)));
    assert(receipt.result == (1 - 3)); /* bit was set, so flag1=1, result=-2 */
    printf("  PASS: xact73_clear_flag\n");
}

static void test_xact73_set_flag(void)
{
    DM2_V1_XactManageFlagsReceipt receipt;
    DM2_V1_XactManageFlagsRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e0574 = 1;        /* op = 1: set */
    req.v1e0572 = 5;        /* bit 5 */
    req.v1e054e_w0a = 0x00; /* no bits set */
    int r = dm2_v1_xact_manage_flags(&req, &receipt);
    assert(r == 1);
    assert(receipt.w_0a == (1 << 5));
    assert(receipt.result == (0 - 3)); /* bit was clear, so flag1=0, result=-3 */
    printf("  PASS: xact73_set_flag\n");
}

static void test_xact73_test_flag(void)
{
    DM2_V1_XactManageFlagsReceipt receipt;
    DM2_V1_XactManageFlagsRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e0574 = 2;        /* op = 2: test only */
    req.v1e0572 = 7;        /* bit 7 */
    req.v1e054e_w0a = 0x80; /* bit 7 set */
    int r = dm2_v1_xact_manage_flags(&req, &receipt);
    assert(r == 1);
    assert(receipt.w_0a == 0x80); /* unchanged */
    assert(receipt.result == -2); /* flag1=1, result=(1-3)=-2 */
    printf("  PASS: xact73_test_flag\n");
}

/* ================================================================== */
/* XACT 74 — Navigate                                                 */
/* ================================================================== */

static void test_xact74_null_safety(void)
{
    DM2_V1_XactNavigateReceipt receipt;
    int r = dm2_v1_xact_navigate(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact74_null_safety\n");
}

/* ================================================================== */
/* XACT 75 — Use item                                                 */
/* ================================================================== */

static void test_xact75_null_safety(void)
{
    DM2_V1_XactUseItemReceipt receipt;
    int r = dm2_v1_xact_use_item(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact75_null_safety\n");
}

/* ================================================================== */
/* XACT 77 — Follow                                                   */
/* ================================================================== */

static void test_xact77_null_safety(void)
{
    DM2_V1_XactFollowReceipt receipt;
    int r = dm2_v1_xact_follow(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact77_null_safety\n");
}

/* ================================================================== */
/* XACT 78 — Face direction                                           */
/* ================================================================== */

static void test_xact78_null_safety(void)
{
    DM2_V1_XactFaceDirectionReceipt receipt;
    int r = dm2_v1_xact_face_direction(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact78_null_safety\n");
}

/* ================================================================== */
/* XACT 79 — Wander setup (fully implemented)                         */
/* ================================================================== */

static void test_xact79_null_safety(void)
{
    DM2_V1_XactWanderSetupReceipt receipt;
    int r = dm2_v1_xact_wander_setup(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact79_null_safety\n");
}

static void test_xact79_randbit_zero(void)
{
    DM2_V1_XactWanderSetupReceipt receipt;
    DM2_V1_XactWanderSetupRequest req;
    memset(&req, 0, sizeof(req));
    req.rand_bit = 0;  /* RANDBIT = 0, so rg3 = 0 */
    req.rand_dir = 2;  /* RANDDIR = 2 */
    int r = dm2_v1_xact_wander_setup(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.b_1e == (int8_t)0x82);
    assert(receipt.b_1a == 39);         /* (0!=0 ? 1 : 0) + 39 = 39 */
    assert(receipt.b_1b == 2);          /* RANDDIR & 3 */
    assert(receipt.b_1c == 2);          /* (2 + 0) & 3 = 2 */
    assert(receipt.b_20 == 0);
    printf("  PASS: xact79_randbit_zero\n");
}

static void test_xact79_randbit_one(void)
{
    DM2_V1_XactWanderSetupReceipt receipt;
    DM2_V1_XactWanderSetupRequest req;
    memset(&req, 0, sizeof(req));
    req.rand_bit = 1;  /* RANDBIT = 1, so rg3 = 1 */
    req.rand_dir = 3;  /* RANDDIR = 3 */
    int r = dm2_v1_xact_wander_setup(&req, &receipt);
    assert(r == 1);
    assert(receipt.b_1a == 40);         /* (1!=0 ? 1 : 0) + 39 = 40 */
    assert(receipt.b_1b == 3);
    assert(receipt.b_1c == 0);          /* (3 + 1) & 3 = 0 */
    printf("  PASS: xact79_randbit_one\n");
}

/* ================================================================== */
/* XACT 80 — Move flagged                                             */
/* ================================================================== */

static void test_xact80_null_safety(void)
{
    DM2_V1_XactMoveFlaggedReceipt receipt;
    int r = dm2_v1_xact_move_flagged(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact80_null_safety\n");
}

/* ================================================================== */
/* XACT 81 — Ranged attack                                            */
/* ================================================================== */

static void test_xact81_null_safety(void)
{
    DM2_V1_XactRangedAttackReceipt receipt;
    int r = dm2_v1_xact_ranged_attack(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact81_null_safety\n");
}

/* ================================================================== */
/* XACT 82 — Commerce                                                 */
/* ================================================================== */

static void test_xact82_null_safety(void)
{
    DM2_V1_XactCommerceReceipt receipt;
    int r = dm2_v1_xact_commerce(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact82_null_safety\n");
}

/* ================================================================== */
/* XACT 83 — Sleep check (mostly implemented)                         */
/* ================================================================== */

static void test_xact83_null_safety(void)
{
    DM2_V1_XactSleepCheckReceipt receipt;
    int r = dm2_v1_xact_sleep_check(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact83_null_safety\n");
}

static void test_xact83_awake(void)
{
    DM2_V1_XactSleepCheckReceipt receipt;
    DM2_V1_XactSleepCheckRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e054e_w0a = 0;    /* bit 7 not set */
    req.v1e0572 = 0;        /* v1e0572 == 0 */
    int r = dm2_v1_xact_sleep_check(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -3); /* not sleeping */
    printf("  PASS: xact83_awake\n");
}

static void test_xact83_sleeping_bit7(void)
{
    DM2_V1_XactSleepCheckReceipt receipt;
    DM2_V1_XactSleepCheckRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e054e_w0a = 0x80;  /* bit 7 set */
    req.v1e0572 = 0;
    int r = dm2_v1_xact_sleep_check(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -2); /* sleeping */
    assert(receipt.b_1a == 0x23); /* BETWEEN(0,2,0) + 0x23 = 35 */
    printf("  PASS: xact83_sleeping_bit7\n");
}

static void test_xact83_sleeping_v1e0572(void)
{
    DM2_V1_XactSleepCheckReceipt receipt;
    DM2_V1_XactSleepCheckRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e054e_w0a = 0;
    req.v1e0572 = 2;
    int r = dm2_v1_xact_sleep_check(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -2);
    assert(receipt.b_1a == 0x25); /* BETWEEN(0,2,2) + 0x23 = 37 */
    printf("  PASS: xact83_sleeping_v1e0572\n");
}

static void test_xact83_sleeping_blocked(void)
{
    DM2_V1_XactSleepCheckReceipt receipt;
    DM2_V1_XactSleepCheckRequest req;
    memset(&req, 0, sizeof(req));
    req.v1e054e_w0a = 0x80; /* bit 7 set */
    req.v1e0572 = 1;        /* v1e0572 == 1 -> result = -4 */
    int r = dm2_v1_xact_sleep_check(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -4);
    assert(receipt.b_1a == 0x24); /* BETWEEN(0,2,1) + 0x23 = 36 */
    printf("  PASS: xact83_sleeping_blocked\n");
}

/* ================================================================== */
/* XACT 84 — Consume item                                             */
/* ================================================================== */

static void test_xact84_null_safety(void)
{
    DM2_V1_XactConsumeItemReceipt receipt;
    int r = dm2_v1_xact_consume_item(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact84_null_safety\n");
}

static void test_xact84_no_possession(void)
{
    DM2_V1_XactConsumeItemReceipt receipt;
    DM2_V1_XactConsumeItemRequest req;
    memset(&req, 0, sizeof(req));
    req.possession_w00 = 0xFFFE;
    int r = dm2_v1_xact_consume_item(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == -3);
    assert(receipt.fail_closed == 0);
    printf("  PASS: xact84_no_possession\n");
}

/* ================================================================== */
/* XACT 89 — Ranged special                                           */
/* ================================================================== */

static void test_xact89_null_safety(void)
{
    DM2_V1_XactRangedSpecialReceipt receipt;
    int r = dm2_v1_xact_ranged_special(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: xact89_null_safety\n");
}

/* ================================================================== */
/* Rebalance coin wallet                                              */
/* ================================================================== */

static void test_rebalance_null_safety(void)
{
    DM2_V1_RebalanceCoinWalletReceipt receipt;
    int r = dm2_v1_rebalance_coin_wallet(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: rebalance_null_safety\n");
}

/* ================================================================== */
/* Main                                                               */
/* ================================================================== */

int main(void)
{
    printf("test_dm2_v1_creature_xact_handlers_pc34_compat:\n");

    /* Null safety for all 25 handlers */
    test_xact56_null_safety();
    test_xact56_no_callback();
    test_xact57_null_safety();
    test_xact59_null_safety();
    test_xact59_resolve_v1e0572();
    test_xact63_null_safety();
    test_xact63_no_spell();
    test_xact64_null_safety();
    test_xact64_no_possession();
    test_xact64_no_missile_flag();
    test_xact65_null_safety();
    test_xact66_null_safety();
    test_xact67_null_safety();
    test_xact68_null_safety();
    test_xact69_null_safety();
    test_xact69_direction_north();
    test_xact69_direction_east();
    test_xact69_direction_wrap();
    test_xact69_invalid_direction();
    test_xact70_null_safety();
    test_xact71_null_safety();
    test_xact73_null_safety();
    test_xact73_clear_flag();
    test_xact73_set_flag();
    test_xact73_test_flag();
    test_xact74_null_safety();
    test_xact75_null_safety();
    test_xact77_null_safety();
    test_xact78_null_safety();
    test_xact79_null_safety();
    test_xact79_randbit_zero();
    test_xact79_randbit_one();
    test_xact80_null_safety();
    test_xact81_null_safety();
    test_xact82_null_safety();
    test_xact83_null_safety();
    test_xact83_awake();
    test_xact83_sleeping_bit7();
    test_xact83_sleeping_v1e0572();
    test_xact83_sleeping_blocked();
    test_xact84_null_safety();
    test_xact84_no_possession();
    test_xact89_null_safety();
    test_rebalance_null_safety();

    printf("All creature XACT handler tests passed.\n");
    return 0;
}
