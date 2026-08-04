#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature_ai_decision_pc34_compat.h"

/* ================================================================== */
/* DM2_DECIDE_NEXT_XACT tests                                        */
/* ================================================================== */

static void test_decide_next_xact_null_safety(void)
{
    DM2_V1_DecideNextXactReceipt receipt;
    int r = dm2_v1_decide_next_xact(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_decide_next_xact(NULL, NULL);
    assert(r == 0);
    printf("  PASS: decide_next_xact_null_safety\n");
}

static void test_decide_next_xact_simple(void)
{
    /* Single row with opcode 62 (attack) */
    DM2_V1_ActionTableRow rows[2];
    DM2_V1_DecideNextXactRequest req;
    DM2_V1_DecideNextXactReceipt receipt;

    memset(rows, 0, sizeof(rows));
    rows[0].opcode = 62;  /* DM2_XACT_ATTACK */
    rows[0].arg3 = 5;     /* xact_arg0 */
    rows[0].arg4 = 10;    /* xact_arg1 */

    memset(&req, 0, sizeof(req));
    req.table = rows;
    req.table_row_count = 2;
    req.table_index = 0;
    req.row_index = 0;

    int r = dm2_v1_decide_next_xact(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 0);
    assert(receipt.action_opcode == 62);
    assert(receipt.xact_arg0 == 5);
    assert(receipt.xact_arg1 == 10);
    assert(receipt.new_row_index == 0);
    printf("  PASS: decide_next_xact_simple\n");
}

static void test_decide_next_xact_skip_negative(void)
{
    /* Row 0: opcode -1 (skip), Row 1: opcode 65 (flee) */
    DM2_V1_ActionTableRow rows[3];
    DM2_V1_DecideNextXactRequest req;
    DM2_V1_DecideNextXactReceipt receipt;

    memset(rows, 0, sizeof(rows));
    rows[0].opcode = -1;
    rows[1].opcode = 65;
    rows[1].arg3 = 3;
    rows[1].arg4 = 7;

    memset(&req, 0, sizeof(req));
    req.table = rows;
    req.table_row_count = 3;
    req.table_index = 0;
    req.row_index = 0;

    int r = dm2_v1_decide_next_xact(&req, &receipt);
    assert(r == 1);
    assert(receipt.action_opcode == 65);
    assert(receipt.new_row_index == 1);
    assert(receipt.xact_arg0 == 3);
    assert(receipt.xact_arg1 == 7);
    printf("  PASS: decide_next_xact_skip_negative\n");
}

static void test_decide_next_xact_set_register(void)
{
    /* Row 0: opcode -10 (set register 0 to value 42), Row 1: opcode 56 */
    DM2_V1_ActionTableRow rows[3];
    DM2_V1_DecideNextXactRequest req;
    DM2_V1_DecideNextXactReceipt receipt;

    memset(rows, 0, sizeof(rows));
    rows[0].opcode = -10;
    rows[0].arg1 = 0;    /* register 0 */
    rows[0].arg2 = 42;   /* value */
    rows[1].opcode = 56;  /* DM2_XACT_STOP */

    memset(&req, 0, sizeof(req));
    req.table = rows;
    req.table_row_count = 3;
    req.table_index = 0;
    req.row_index = 0;
    req.creature_w0e = 100;
    req.creature_w10 = 200;

    int r = dm2_v1_decide_next_xact(&req, &receipt);
    assert(r == 1);
    assert(receipt.action_opcode == 56);
    assert(receipt.new_row_index == 1);
    assert(receipt.reg_writes == 1);
    assert(receipt.new_creature_w0e == 42);
    assert(receipt.new_creature_w10 == 200);
    printf("  PASS: decide_next_xact_set_register\n");
}

static void test_decide_next_xact_set_register1(void)
{
    /* Set register 1 (creature_w10) */
    DM2_V1_ActionTableRow rows[2];
    DM2_V1_DecideNextXactRequest req;
    DM2_V1_DecideNextXactReceipt receipt;

    memset(rows, 0, sizeof(rows));
    rows[0].opcode = -10;
    rows[0].arg1 = 1;
    rows[0].arg2 = 99;
    rows[1].opcode = 70;

    memset(&req, 0, sizeof(req));
    req.table = rows;
    req.table_row_count = 2;
    req.table_index = 0;
    req.row_index = 0;
    req.creature_w0e = 10;
    req.creature_w10 = 20;

    int r = dm2_v1_decide_next_xact(&req, &receipt);
    assert(r == 1);
    assert(receipt.new_creature_w0e == 10);
    assert(receipt.new_creature_w10 == 99);
    assert(receipt.reg_writes == 1);
    printf("  PASS: decide_next_xact_set_register1\n");
}

static void test_decide_next_xact_no_table(void)
{
    DM2_V1_DecideNextXactRequest req;
    DM2_V1_DecideNextXactReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.table = NULL;
    req.table_row_count = 0;

    int r = dm2_v1_decide_next_xact(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: decide_next_xact_no_table\n");
}

/* ================================================================== */
/* DM2_14cd_08f5 (post_xact_result) tests                             */
/* ================================================================== */

static void test_post_xact_result_null_safety(void)
{
    DM2_V1_PostXactResultReceipt receipt;
    int r = dm2_v1_ai_post_xact_result(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: post_xact_result_null_safety\n");
}

static void test_post_xact_result_success_branch(void)
{
    /* Return code -2 (success): use byte 2 (arg2) as branch.
     * Branch = 5: direct row set. */
    DM2_V1_ActionTableRow rows[8];
    DM2_V1_PostXactResultRequest req;
    DM2_V1_PostXactResultReceipt receipt;

    memset(rows, 0, sizeof(rows));
    rows[3].arg1 = 0;   /* fail branch */
    rows[3].arg2 = 5;   /* success branch: go to row 5 */

    memset(&req, 0, sizeof(req));
    req.xact_return_code = -2;
    req.table_index = 1;
    req.row_index = 3;
    req.table = rows;
    req.table_row_count = 8;

    int r = dm2_v1_ai_post_xact_result(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.new_table_index == 1);
    assert(receipt.new_row_index == 5);
    assert(receipt.state_changed == 1);
    printf("  PASS: post_xact_result_success_branch\n");
}

static void test_post_xact_result_fail_clear(void)
{
    /* Return code -3 (fail): use byte 1 (arg1) as branch.
     * Branch = -3: clear table_index. */
    DM2_V1_ActionTableRow rows[4];
    DM2_V1_PostXactResultRequest req;
    DM2_V1_PostXactResultReceipt receipt;

    memset(rows, 0, sizeof(rows));
    rows[0].arg1 = -3;  /* fail branch: clear */
    rows[0].arg2 = 2;

    memset(&req, 0, sizeof(req));
    req.xact_return_code = -3;
    req.table_index = 0;
    req.row_index = 0;
    req.table = rows;
    req.table_row_count = 4;

    int r = dm2_v1_ai_post_xact_result(&req, &receipt);
    assert(r == 1);
    assert(receipt.new_table_index == -1);
    assert(receipt.new_row_index == 0);
    assert(receipt.state_changed == 1);
    printf("  PASS: post_xact_result_fail_clear\n");
}

static void test_post_xact_result_same_row(void)
{
    /* Branch = current row_index: no state change. */
    DM2_V1_ActionTableRow rows[4];
    DM2_V1_PostXactResultRequest req;
    DM2_V1_PostXactResultReceipt receipt;

    memset(rows, 0, sizeof(rows));
    rows[2].arg2 = 2;  /* success branch = same row */

    memset(&req, 0, sizeof(req));
    req.xact_return_code = -2;
    req.table_index = 0;
    req.row_index = 2;
    req.table = rows;
    req.table_row_count = 4;

    int r = dm2_v1_ai_post_xact_result(&req, &receipt);
    assert(r == 1);
    assert(receipt.state_changed == 0);
    assert(receipt.new_row_index == 2);
    printf("  PASS: post_xact_result_same_row\n");
}

/* ================================================================== */
/* DM2_14cd_0389 (validate_target) tests                              */
/* ================================================================== */

static void test_validate_target_null_safety(void)
{
    DM2_V1_ValidateTargetReceipt receipt;
    int r = dm2_v1_ai_validate_target(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: validate_target_null_safety\n");
}

static void test_validate_target_no_state(void)
{
    DM2_V1_ValidateTargetRequest req;
    DM2_V1_ValidateTargetReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.v1e07d8_b00 = 0;  /* b00 == 0 => invalid */
    req.v1e07d8_b01 = 1;
    req.v1e07d8_b03 = 0;

    int r = dm2_v1_ai_validate_target(&req, &receipt);
    assert(r == 1);
    assert(receipt.target_valid == 0);
    assert(receipt.table_index == (int8_t)0xff);
    printf("  PASS: validate_target_no_state\n");
}

static void test_validate_target_b03_minus1(void)
{
    DM2_V1_ValidateTargetRequest req;
    DM2_V1_ValidateTargetReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.v1e07d8_b00 = 1;
    req.v1e07d8_b01 = 1;
    req.v1e07d8_b03 = -1;  /* -1 => invalid */

    int r = dm2_v1_ai_validate_target(&req, &receipt);
    assert(r == 1);
    assert(receipt.target_valid == 0);
    printf("  PASS: validate_target_b03_minus1\n");
}

static void test_validate_target_no_table(void)
{
    DM2_V1_ValidateTargetRequest req;
    DM2_V1_ValidateTargetReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.v1e07d8_b00 = 1;
    req.v1e07d8_b01 = 1;
    req.v1e07d8_b03 = 0;
    req.creature_b12 = -1;  /* no table => invalid */

    int r = dm2_v1_ai_validate_target(&req, &receipt);
    assert(r == 1);
    assert(receipt.target_valid == 0);
    printf("  PASS: validate_target_no_table\n");
}

static void test_validate_target_has_table(void)
{
    DM2_V1_ValidateTargetRequest req;
    DM2_V1_ValidateTargetReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.v1e07d8_b00 = 1;
    req.v1e07d8_b01 = 1;
    req.v1e07d8_b03 = 0;
    req.creature_b12 = 3;

    int r = dm2_v1_ai_validate_target(&req, &receipt);
    assert(r == 1);
    assert(receipt.fail_closed == 1);  /* needs live dungeon data */
    assert(receipt.table_index == 3);
    printf("  PASS: validate_target_has_table\n");
}

/* ================================================================== */
/* DM2_14cd_0457 (select_target) tests                                */
/* ================================================================== */

static void test_select_target_null_safety(void)
{
    DM2_V1_SelectTargetReceipt receipt;
    int r = dm2_v1_ai_select_target(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: select_target_null_safety\n");
}

static void test_select_target_empty(void)
{
    DM2_V1_SelectTargetRequest req;
    DM2_V1_SelectTargetReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.candidate_count = 0;

    int r = dm2_v1_ai_select_target(&req, &receipt);
    assert(r == 1);
    assert(receipt.selected_index == -1);
    printf("  PASS: select_target_empty\n");
}

/* ================================================================== */
/* DM2_14cd_0067 (select_behavior) tests                              */
/* ================================================================== */

static void test_select_behavior_null_safety(void)
{
    DM2_V1_SelectBehaviorReceipt receipt;
    int r = dm2_v1_ai_select_behavior(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: select_behavior_null_safety\n");
}

static void test_select_behavior_no_table(void)
{
    DM2_V1_SelectBehaviorRequest req;
    DM2_V1_SelectBehaviorReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.behavior_table = NULL;
    req.behavior_entry_count = 0;

    int r = dm2_v1_ai_select_behavior(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: select_behavior_no_table\n");
}

static void test_select_behavior_fail_closed(void)
{
    DM2_V1_BehaviorEntry entries[2];
    DM2_V1_CreatureModeFlags flags;
    DM2_V1_SelectBehaviorRequest req;
    DM2_V1_SelectBehaviorReceipt receipt;

    memset(entries, 0, sizeof(entries));
    memset(&flags, 0, sizeof(flags));
    memset(&req, 0, sizeof(req));

    req.behavior_table = entries;
    req.behavior_entry_count = 2;
    req.mode_flags = &flags;

    int r = dm2_v1_ai_select_behavior(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    printf("  PASS: select_behavior_fail_closed\n");
}

/* ================================================================== */
/* DM2_SELECT_CREATURE_37FC (select_creature_mode) tests              */
/* ================================================================== */

static void test_select_creature_mode_null_safety(void)
{
    DM2_V1_SelectCreatureModeReceipt receipt;
    int r = dm2_v1_ai_select_creature_mode(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: select_creature_mode_null_safety\n");
}

static int16_t mock_query_gdat(void *ctx, uint8_t creature_type, int16_t param)
{
    (void)ctx; (void)creature_type; (void)param;
    return 2;
}

static void test_select_creature_mode_resolve(void)
{
    DM2_V1_SelectCreatureModeRequest req;
    DM2_V1_SelectCreatureModeReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.v1e0584 = -1;
    req.creature_spec_b04 = 5;
    req.query_gdat = mock_query_gdat;
    req.behavior_tables = NULL;
    req.behavior_table_count = 0;

    int r = dm2_v1_ai_select_creature_mode(&req, &receipt);
    assert(receipt.v1e0584 == 2);  /* resolved via gdat */
    assert(receipt.fail_closed == 1);  /* no behavior tables */
    printf("  PASS: select_creature_mode_resolve\n");
    (void)r;
}

/* ================================================================== */
/* DM2_14cd_0550 (invoke_action_handler) tests                        */
/* ================================================================== */

static void test_invoke_action_handler_null_safety(void)
{
    DM2_V1_InvokeActionHandlerReceipt receipt;
    int r = dm2_v1_ai_invoke_action_handler(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: invoke_action_handler_null_safety\n");
}

/* ================================================================== */
/* DM2_14cd_0276 (prepare_action_context) tests                       */
/* ================================================================== */

static void test_prepare_action_context_null_safety(void)
{
    DM2_V1_PrepareActionContextReceipt receipt;
    int r = dm2_v1_ai_prepare_action_context(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: prepare_action_context_null_safety\n");
}

static void test_prepare_action_context_extract(void)
{
    uint8_t entry[0x1a];
    DM2_V1_PrepareActionContextRequest req;
    DM2_V1_PrepareActionContextReceipt receipt;

    memset(entry, 0, sizeof(entry));
    entry[4] = 0x34;  /* w08 low */
    entry[5] = 0x12;  /* w08 high */
    entry[6] = 3;     /* b00/b01 = max(0, 3) = 3 */
    entry[7] = 7;     /* b03 */
    entry[8] = 0xAB;  /* w04 low */
    entry[9] = 0xCD;  /* w04 high */
    entry[0xa] = 0x11; /* w06 low */
    entry[0xb] = 0x22; /* w06 high */
    entry[0x11] = 9;  /* b02 */

    memset(&req, 0, sizeof(req));
    req.entry = entry;
    req.entry_size = 0x1a;

    int r = dm2_v1_ai_prepare_action_context(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.v1e07d8_b00 == 3);
    assert(receipt.v1e07d8_b01 == 3);
    assert(receipt.v1e07d8_w08 == 0x1234);
    assert(receipt.v1e07d8_b03 == 7);
    assert(receipt.v1e07d8_w04 == 0xCDAB);
    assert(receipt.v1e07d8_w06 == 0x2211);
    assert(receipt.v1e07d8_b02 == 9);
    assert(receipt.needs_allocation == 1);
    printf("  PASS: prepare_action_context_extract\n");
}

static void test_prepare_action_context_negative_b06(void)
{
    uint8_t entry[0x1a];
    DM2_V1_PrepareActionContextRequest req;
    DM2_V1_PrepareActionContextReceipt receipt;

    memset(entry, 0, sizeof(entry));
    entry[6] = 0xFE;  /* -2 as signed: max(0, -2) = 0 */

    memset(&req, 0, sizeof(req));
    req.entry = entry;
    req.entry_size = 0x1a;

    int r = dm2_v1_ai_prepare_action_context(&req, &receipt);
    assert(r == 1);
    assert(receipt.v1e07d8_b00 == 0);
    assert(receipt.needs_allocation == 0);
    printf("  PASS: prepare_action_context_negative_b06\n");
}

static void test_prepare_action_context_too_small(void)
{
    uint8_t entry[10];
    DM2_V1_PrepareActionContextRequest req;
    DM2_V1_PrepareActionContextReceipt receipt;

    memset(entry, 0, sizeof(entry));
    memset(&req, 0, sizeof(req));
    req.entry = entry;
    req.entry_size = 10;  /* too small */

    int r = dm2_v1_ai_prepare_action_context(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: prepare_action_context_too_small\n");
}

/* ================================================================== */
/* DM2_14cd_0684 (find_action_table) tests                            */
/* ================================================================== */

static void test_find_action_table_null_safety(void)
{
    DM2_V1_FindActionTableReceipt receipt;
    int r = dm2_v1_ai_find_action_table(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: find_action_table_null_safety\n");
}

static void test_find_action_table_fail_closed(void)
{
    DM2_V1_CreatureModeFlags flags;
    DM2_V1_FindActionTableRequest req;
    DM2_V1_FindActionTableReceipt receipt;

    memset(&flags, 0, sizeof(flags));
    memset(&req, 0, sizeof(req));
    req.mode_flags = &flags;

    int r = dm2_v1_ai_find_action_table(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    assert(receipt.result_table_index == -3);
    printf("  PASS: find_action_table_fail_closed\n");
}

/* ================================================================== */
/* Main                                                               */
/* ================================================================== */

int main(void)
{
    printf("test_dm2_v1_creature_ai_decision_pc34_compat:\n");

    /* decide_next_xact */
    test_decide_next_xact_null_safety();
    test_decide_next_xact_simple();
    test_decide_next_xact_skip_negative();
    test_decide_next_xact_set_register();
    test_decide_next_xact_set_register1();
    test_decide_next_xact_no_table();

    /* post_xact_result */
    test_post_xact_result_null_safety();
    test_post_xact_result_success_branch();
    test_post_xact_result_fail_clear();
    test_post_xact_result_same_row();

    /* validate_target */
    test_validate_target_null_safety();
    test_validate_target_no_state();
    test_validate_target_b03_minus1();
    test_validate_target_no_table();
    test_validate_target_has_table();

    /* select_target */
    test_select_target_null_safety();
    test_select_target_empty();

    /* select_behavior */
    test_select_behavior_null_safety();
    test_select_behavior_no_table();
    test_select_behavior_fail_closed();

    /* select_creature_mode */
    test_select_creature_mode_null_safety();
    test_select_creature_mode_resolve();

    /* invoke_action_handler */
    test_invoke_action_handler_null_safety();

    /* prepare_action_context */
    test_prepare_action_context_null_safety();
    test_prepare_action_context_extract();
    test_prepare_action_context_negative_b06();
    test_prepare_action_context_too_small();

    /* find_action_table */
    test_find_action_table_null_safety();
    test_find_action_table_fail_closed();

    printf("All tests passed.\n");
    return 0;
}
