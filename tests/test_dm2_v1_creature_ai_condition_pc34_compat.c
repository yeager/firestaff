#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature_ai_condition_pc34_compat.h"

/* ================================================================== */
/* DM2_14cd_1316 — condition evaluator tests                          */
/* ================================================================== */

static void test_condition_null_safety(void)
{
    DM2_V1_AiConditionReceipt receipt;
    int r = dm2_v1_ai_check_condition(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_ai_check_condition(NULL, NULL);
    assert(r == 0);
    printf("  PASS: condition_null_safety\n");
}

static void test_condition_case0_always_true(void)
{
    /* c_ai.cpp:2570-2571 — case 0 always returns 1 */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 0; /* case 0 */

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.result == 1);
    assert(receipt.condition_case == 0);
    printf("  PASS: condition_case0_always_true\n");
}

static void test_condition_case0_inverted(void)
{
    /* c_ai.cpp:2558-2562 — bit 0x80 inverts result */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 0x80; /* case 0 + invert */

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.inverted == 1);
    assert(receipt.result == 0); /* true inverted = false */
    printf("  PASS: condition_case0_inverted\n");
}

static void test_condition_case2_same_tile(void)
{
    /* c_ai.cpp:2607-2629 — party on same tile */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 2;
    req.creature_x = 5;
    req.creature_y = 10;
    req.creature_map = 3;
    req.party_x = 5;
    req.party_y = 10;
    req.party_map = 3;

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    /* Different tile */
    req.party_x = 6;
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case2_same_tile\n");
}

static void test_condition_case4_timing_flag(void)
{
    /* c_ai.cpp:2636-2639 — returns v1e058d */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 4;
    req.v1e058d = 1;

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    req.v1e058d = 0;
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case4_timing_flag\n");
}

static void test_condition_case6_flag_bit(void)
{
    /* c_ai.cpp:2669-2676 — bit check on SPX w_0a */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 6;
    req.condition_param = 3; /* check bit 3 */
    req.spx.w_0a = 0x08;    /* bit 3 set */

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    req.spx.w_0a = 0x04; /* bit 3 clear */
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case6_flag_bit\n");
}

static void test_condition_case7_same_map(void)
{
    /* c_ai.cpp:2678-2682 — same map level */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 7;
    req.creature_map = 5;
    req.party_map = 5;

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    req.party_map = 6;
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case7_same_map\n");
}

static void test_condition_case14_hp_percentage(void)
{
    /* c_ai.cpp:2878-2885 — (hp * 52) / max_hp <= param */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 14;
    req.condition_param = 26; /* 50% of 52 */
    req.spx.w_06 = 50;       /* current HP */
    req.ai_spec_max_hp = 100; /* max HP */

    /* 50 * 52 / 100 = 26, 26 <= 26 → true */
    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    req.spx.w_06 = 80; /* 80 * 52 / 100 = 41, 41 > 26 → false */
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case14_hp_percentage\n");
}

static void test_condition_case16_home_map(void)
{
    /* c_ai.cpp:2964-2971 — creature on home map */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 16;
    req.creature_map = 3;
    req.spx.w_0c = (3 << 10) | (5 << 5) | 10; /* map=3, y=5, x=10 */

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    req.creature_map = 4;
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case16_home_map\n");
}

static void test_condition_case18_starting_map(void)
{
    /* c_ai.cpp:2985-2987 — creature on starting map */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 18;
    req.creature_map = 2;
    req.starting_map = 2;

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    req.creature_map = 3;
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case18_starting_map\n");
}

static void test_condition_case19_not_starting_map(void)
{
    /* c_ai.cpp:2989-2999 — NOT on starting map + flag clear */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 19;
    req.condition_param = 2; /* check bit 2 */
    req.spx.w_0a = 0;       /* bit 2 clear */
    req.creature_map = 5;
    req.starting_map = 2;

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1); /* not on starting map + flag clear */

    req.creature_map = 2; /* on starting map */
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);

    req.creature_map = 5;
    req.spx.w_0a = 0x04; /* bit 2 set — early exit */
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case19_not_starting_map\n");
}

static void test_condition_case21_waypoint(void)
{
    /* c_ai.cpp:3011-3032 — creature at specific waypoint */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    uint16_t waypoints[4];
    memset(&req, 0, sizeof(req));
    req.condition_byte = 21;
    req.condition_param = 1; /* waypoint index 1 */

    /* Pack waypoint: map=2, y=10, x=5 -> (2<<10)|(10<<5)|5 = 0x0945 */
    waypoints[0] = 0;
    waypoints[1] = (2 << 10) | (10 << 5) | 5;
    waypoints[2] = 0;
    waypoints[3] = 0;
    req.creature_rec.waypoint_words = waypoints;
    req.creature_rec.waypoint_count = 4;

    req.creature_x = 5;
    req.creature_y = 10;
    req.creature_map = 2;

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1);

    req.creature_x = 6; /* wrong x */
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);
    printf("  PASS: condition_case21_waypoint\n");
}

static void test_condition_subtype_check(void)
{
    /* c_ai.cpp:2550-2557 — bit 0x40 subtype shortcut */
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 0x40 | 7; /* subtype check + case 7 */
    req.creature_subtype = 5;
    req.creature_rec.b_12 = 5; /* match */

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.subtype_matched == 1);
    assert(receipt.result == 1);

    req.creature_rec.b_12 = 6; /* no match, falls through to case 7 */
    req.creature_map = 3;
    req.party_map = 4; /* different map -> case 7 returns false */
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.subtype_matched == 0);
    assert(receipt.result == 0);
    printf("  PASS: condition_subtype_check\n");
}

static void test_condition_out_of_range(void)
{
    DM2_V1_AiConditionRequest req;
    DM2_V1_AiConditionReceipt receipt;
    memset(&req, 0, sizeof(req));
    req.condition_byte = 23; /* > 22, out of range */

    int r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 0);

    req.condition_byte = 0x80 | 23; /* inverted out of range */
    r = dm2_v1_ai_check_condition(&req, &receipt);
    assert(r == 1);
    assert(receipt.result == 1); /* inverted 0 = 1 */
    printf("  PASS: condition_out_of_range\n");
}

/* ================================================================== */
/* DM2_ai_14cd_0f3c — create action entry tests                      */
/* ================================================================== */

static void test_create_action_null_safety(void)
{
    DM2_V1_CreateActionEntryReceipt receipt;
    int r = dm2_v1_ai_create_action_entry(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_ai_create_action_entry(NULL, NULL);
    assert(r == 0);
    printf("  PASS: create_action_null_safety\n");
}

static void test_create_action_full(void)
{
    /* c_ai.cpp:1516 — max 16 entries */
    DM2_V1_CreateActionEntryRequest req;
    DM2_V1_CreateActionEntryReceipt receipt;
    DM2_V1_HexeEntry hexe;
    memset(&req, 0, sizeof(req));
    memset(&hexe, 0, sizeof(hexe));
    req.hexe = &hexe;
    req.current_entry_count = 16;

    int r = dm2_v1_ai_create_action_entry(&req, &receipt);
    assert(r == 1);
    assert(receipt.rejected_full == 1);
    assert(receipt.entry_created == 0);
    printf("  PASS: create_action_full\n");
}

static void test_create_action_basic(void)
{
    DM2_V1_CreateActionEntryRequest req;
    DM2_V1_CreateActionEntryReceipt receipt;
    DM2_V1_HexeEntry hexe;
    memset(&req, 0, sizeof(req));
    memset(&hexe, 0, sizeof(hexe));

    hexe.b_08 = 10;  /* attack strength */
    hexe.b_09 = 3;   /* secondary */
    hexe.w_04 = 42;
    hexe.w_06 = (int16_t)0xFFFF;

    req.hexe = &hexe;
    req.hexe_raw = (const uint8_t *)&hexe;
    req.current_entry_count = 5;
    req.creature_map = 2;
    req.party_map = 2; /* same map — no shift */
    req.v1e0580 = (int16_t)0xFFFF;
    req.priority = 7;
    req.strength_adjust = 2;
    req.argw1 = 100;
    req.arg_0e = 1;
    req.arg_0f = 2;
    req.group_byte = 3;

    int r = dm2_v1_ai_create_action_entry(&req, &receipt);
    assert(r == 1);
    assert(receipt.entry_created == 1);
    assert(receipt.new_entry_count == 6);
    assert(receipt.entry.strength == 12); /* 10 + 2 */
    assert(receipt.entry.b_01 == 3);
    assert(receipt.entry.b_07 == 7);
    assert(receipt.entry.w_08 == 42);
    assert(receipt.entry.w_0c == 100);
    assert(receipt.entry.b_0e == 1);
    assert(receipt.entry.b_0f == 2);
    assert(receipt.entry.b_11 == 3);
    printf("  PASS: create_action_basic\n");
}

static void test_create_action_different_map_shift(void)
{
    /* c_ai.cpp:1518-1531 — different map, no bit 0x40 → shift by 2 */
    DM2_V1_CreateActionEntryRequest req;
    DM2_V1_CreateActionEntryReceipt receipt;
    DM2_V1_HexeEntry hexe;
    memset(&req, 0, sizeof(req));
    memset(&hexe, 0, sizeof(hexe));

    hexe.b_08 = 20;
    req.hexe = &hexe;
    req.current_entry_count = 0;
    req.creature_map = 1;
    req.party_map = 2; /* different map */
    req.ai_spec_byte1 = 0; /* bit 0x40 clear */
    req.v1e0580 = (int16_t)0xFFFF;
    req.strength_adjust = 4;

    int r = dm2_v1_ai_create_action_entry(&req, &receipt);
    assert(r == 1);
    assert(receipt.entry_created == 1);
    /* 20 >> 2 = 5, 4 >> 2 = 1, 5 + 1 = 6 */
    assert(receipt.entry.strength == 6);
    printf("  PASS: create_action_different_map_shift\n");
}

static void test_create_action_negative_rejected(void)
{
    /* Negative combined strength should be rejected */
    DM2_V1_CreateActionEntryRequest req;
    DM2_V1_CreateActionEntryReceipt receipt;
    DM2_V1_HexeEntry hexe;
    memset(&req, 0, sizeof(req));
    memset(&hexe, 0, sizeof(hexe));

    hexe.b_08 = 2;
    req.hexe = &hexe;
    req.current_entry_count = 0;
    req.creature_map = 1;
    req.party_map = 2; /* different map → shift */
    req.ai_spec_byte1 = 0;
    req.v1e0580 = (int16_t)0xFFFF;
    req.strength_adjust = -8;

    int r = dm2_v1_ai_create_action_entry(&req, &receipt);
    assert(r == 1);
    /* 2 >> 2 = 0, -8 >> 2 = -2, 0 + (-2) = -2, clamped to -1, still < 0 */
    assert(receipt.rejected_negative == 1);
    assert(receipt.entry_created == 0);
    printf("  PASS: create_action_negative_rejected\n");
}

/* ================================================================== */
/* DM2_14cd_18f2 — hexe condition walk tests                         */
/* ================================================================== */

static void test_hexe_walk_null_safety(void)
{
    DM2_V1_HexeWalkReceipt receipt;
    int r = dm2_v1_ai_hexe_condition_walk(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: hexe_walk_null_safety\n");
}

static void test_hexe_walk_basic(void)
{
    DM2_V1_HexeWalkRequest req;
    DM2_V1_HexeWalkReceipt receipt;
    DM2_V1_HexeEntry table[3];
    memset(&req, 0, sizeof(req));
    memset(table, 0, sizeof(table));

    /* Entry 0: matches key=2, condition=0 (always true), has continuation */
    table[0].b_0c = 2;
    table[0].b_01 = 0; /* case 0 = always true */
    table[0].b_0d = 1; /* continue */

    /* Entry 1: doesn't match key */
    table[1].b_0c = 3;
    table[1].b_01 = 0;
    table[1].b_0d = 1;

    /* Entry 2: matches key, condition=0, last entry */
    table[2].b_0c = 2;
    table[2].b_01 = 0;
    table[2].b_0d = 0; /* last */

    req.walk_key = 2;
    req.table = table;
    req.table_count = 3;
    req.argw0 = 0xFFFF;

    int r = dm2_v1_ai_hexe_condition_walk(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.entries_checked == 3);
    assert(receipt.conditions_passed == 2);
    printf("  PASS: hexe_walk_basic\n");
}

/* ================================================================== */
/* DM2_14cd_0f0a — action dispatch tests                             */
/* ================================================================== */

static void test_action_dispatch_null_safety(void)
{
    DM2_V1_ActionDispatchReceipt receipt;
    int r = dm2_v1_ai_action_dispatch(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: action_dispatch_null_safety\n");
}

static void test_action_dispatch_valid_cases(void)
{
    DM2_V1_ActionDispatchRequest req;
    DM2_V1_ActionDispatchReceipt receipt;
    int i;

    for (i = 0; i <= 16; i++) {
        memset(&req, 0, sizeof(req));
        req.dispatch_type = (uint8_t)i;

        int r = dm2_v1_ai_action_dispatch(&req, &receipt);
        assert(r == 1);
        assert(receipt.valid == 1);
        assert(receipt.dispatch_case == i);
    }

    /* Out of range */
    memset(&req, 0, sizeof(req));
    req.dispatch_type = 17;
    int r = dm2_v1_ai_action_dispatch(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: action_dispatch_valid_cases\n");
}

/* ================================================================== */
/* Unified wrapper tests                                              */
/* ================================================================== */

static void test_wrapper_null_safety(void)
{
    DM2_V1_AiWrapperReceipt receipt;
    int r = dm2_v1_ai_action_wrapper(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: wrapper_null_safety\n");
}

static void test_wrapper_all_types(void)
{
    DM2_V1_AiWrapperRequest req;
    DM2_V1_AiWrapperReceipt receipt;
    int i;

    for (i = 0; i <= 16; i++) {
        memset(&req, 0, sizeof(req));
        req.type = (DM2_V1_AiWrapperType)i;

        int r = dm2_v1_ai_action_wrapper(&req, &receipt);
        assert(r == 1);
        assert(receipt.valid == 1);
        assert(receipt.fail_closed == 1); /* all fail-closed until wired */
        assert((int)receipt.type_dispatched == i);
    }
    printf("  PASS: wrapper_all_types\n");
}

/* ================================================================== */
/* Main                                                               */
/* ================================================================== */

int main(void)
{
    printf("test_dm2_v1_creature_ai_condition_pc34_compat:\n");

    /* Condition evaluator */
    test_condition_null_safety();
    test_condition_case0_always_true();
    test_condition_case0_inverted();
    test_condition_case2_same_tile();
    test_condition_case4_timing_flag();
    test_condition_case6_flag_bit();
    test_condition_case7_same_map();
    test_condition_case14_hp_percentage();
    test_condition_case16_home_map();
    test_condition_case18_starting_map();
    test_condition_case19_not_starting_map();
    test_condition_case21_waypoint();
    test_condition_subtype_check();
    test_condition_out_of_range();

    /* Action entry creator */
    test_create_action_null_safety();
    test_create_action_full();
    test_create_action_basic();
    test_create_action_different_map_shift();
    test_create_action_negative_rejected();

    /* Hexe walk */
    test_hexe_walk_null_safety();
    test_hexe_walk_basic();

    /* Action dispatch */
    test_action_dispatch_null_safety();
    test_action_dispatch_valid_cases();

    /* Wrapper */
    test_wrapper_null_safety();
    test_wrapper_all_types();

    printf("All tests passed.\n");
    return 0;
}
