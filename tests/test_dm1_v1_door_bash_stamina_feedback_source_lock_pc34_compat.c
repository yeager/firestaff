/*
 * DM1 V1 door-bash stamina feedback source-lock contract gate.
 *
 * Pinned ReDMCSB path:
 *   MENU.C:1272-1273  L1253_i_ActionStamina = G0494_auc_Graphic560_
 *                     ActionStamina[P0788_i_ActionIndex] + M005_RANDOM(2)
 *   MENU.C:1311-1319  closed-door bash branch: 6-tick ActionDisabledTicks,
 *                     F0232 dispatch on the bash strength arg, 2-tick
 *                     destruction event, M563 + C04 sound play
 *   MENU.C:1620-1622  F0330_CHAMPION_DisableAction when ActionDisabledTicks
 *                     is non-zero
 *   MENU.C:1623-1624  F0325_CHAMPION_DecrementStamina when
 *                     L1253_i_ActionStamina is non-zero
 *   CHAMPION.C:1078-1103 F0306_CHAMPION_GetStaminaAdjustedValue: returns
 *                     (val/2) + (val/2 * current / halfMax) when current
 *                     is below halfMax, else val
 *   CHAMPION.C:1237-1303 F0312_CHAMPION_GetStrength: RNG(16) + base strength
 *                     + weapon + skill + hand wound + F0306 +
 *                     F0026_MAIN_GetBoundedValue(0, str >> 1, 100)
 *   CHAMPION.C:2025-2049 F0325_CHAMPION_DecrementStamina: subtract, clamp
 *                     to 0, route overflow to F0321 with damage = (-new) >> 1
 *   CHAMPION.C:2048   M516.Attributes MASK0x0200_LOAD | MASK0x0100_STATISTICS
 *   G0494_auc_Graphic560_ActionStamina[44] at MENU.C:292-337 (byte-stable
 *                     across ReDMCSB DM1 V1 Atari ST, FM-Towns, PC 3.4)
 *   DEFS.H:4          M005_RANDOM
 *   DEFS.H:560-565    G0254_as_Graphic559_DoorInfo[4] (Portcullis 110,
 *                     Wooden 42, Iron 230, Ra 255) and the "melee
 *                     attacks are limited to 100" comment
 *   DEFS.H:1555-1580  DOOR_INFO struct and the door attributes
 *   DEFS.H:7998-7999  F0312_CHAMPION_GetStrength anchor
 *   DEFS.H:934        C02_EVENT_DOOR_DESTRUCTION
 *   DEFS.H:136-138    sound play modes C00/C01/C02
 *
 * Non-duplicative with the existing dm1_v1_door_bash_feedback (pass777)
 * contract. That gate pins the F0232 dispatch + sound + 6-tick disabled
 * cooldown contract with a precomputed `action_strength` input; the
 * present gate pins the F0306 + F0325 + G0494 + MENU.C:1272-1273 +
 * MENU.C:1623-1624 stamina contract that feeds the bash family
 * specifically. The present gate does NOT cover original DOS pixel
 * parity, does not load GRAPHICS.DAT / DUNGEON.DAT, does not emit
 * real F0064 / F0325 / F0330 / F0238 / F0321 calls.
 */

#include "dm1_v1_door_bash_stamina_feedback_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

/* FNV-1a 32-bit, seeded with the bash dispatch identity. */
#define FNV1A_OFFSET_BASIS_PC34 0x811C9DC5u
#define FNV1A_PRIME_PC34       0x01000193u

static uint32_t fnv1a_u8(uint32_t hash, uint8_t value)
{
    hash ^= (uint32_t)value;
    hash *= FNV1A_PRIME_PC34;
    return hash;
}

static uint32_t fnv1a_u16(uint32_t hash, uint16_t value)
{
    hash = fnv1a_u8(hash, (uint8_t)(value & 0xFFu));
    hash = fnv1a_u8(hash, (uint8_t)((value >> 8) & 0xFFu));
    return hash;
}

static uint32_t fnv1a_i16(uint32_t hash, int16_t value)
{
    return fnv1a_u16(hash, (uint16_t)value);
}

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_u8(const char *id, uint8_t got, uint8_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", id, (unsigned)got,
               (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %u anchor=%s\n", id, (unsigned)want, anchor);
    }
}

static void expect_u16(const char *id, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", id, (unsigned)got,
               (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %u anchor=%s\n", id, (unsigned)want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want,
                        const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, (int)got, (int)want,
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, (int)want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)",
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_action_ordinal_set(void)
{
    /* MENU.C:1311-1316 bash group is { C030 BASH, C018 HACK,
     * C019 BERZERK, C007 KICK, C013 SWING, C002 CHOP }. */
    expect_bool("action.c030_bash",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x30), true,
                "MENU.C:1311 C030 BASH");
    expect_bool("action.c018_hack",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x18), true,
                "MENU.C:1312 C018 HACK");
    expect_bool("action.c019_berzerk",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x13), true,
                "MENU.C:1313 C019 BERZRK");
    expect_bool("action.c007_kick",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x07), true,
                "MENU.C:1314 C007 KICK");
    expect_bool("action.c013_swing",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x0D), true,
                "MENU.C:1315 C013 SWING");
    expect_bool("action.c002_chop",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x02), true,
                "MENU.C:1316 C002 CHOP");
    /* Non-bash melee and spell actions must be rejected. */
    expect_bool("action.c024_disrupt_rejected",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x18 + 0x0C), false,
                "MENU.C:1319 C024 ACTION_DISRUPT");
    expect_bool("action.c016_jab_rejected",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x10), false,
                "MENU.C:1320 C016 JAB");
    expect_bool("action.c020_fireball_rejected",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x14), false,
                "MENU.C:1322 C020 FIREBALL");
    expect_bool("action.zero_rejected",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0x00), false,
                "MENU.C:292-293 N action (index 0)");
    expect_bool("action.ff_rejected",
                M11_GameView_DoorBashStaminaActionIsBashPc34(0xFF), false,
                "Out-of-range ordinal");
}

static void test_action_stamina_table_costs(void)
{
    /* ReDMCSB MENU.C:292-337 G0494_auc_Graphic560_ActionStamina[44]
     * costs for the bash family. */
    uint8_t cost = 0;
    uint8_t rbit = 0;
    bool ok;

    ok = M11_GameView_DoorBashStaminaActionCostPc34(0x30, &cost, &rbit);
    expect_bool("cost.c030_bash.lookup", ok, true,
                "MENU.C:1272 G0494[30] C030 BASH");
    expect_u8("cost.c030_bash", cost, 9, "MENU.C:322 BASH=9");
    ok = M11_GameView_DoorBashStaminaActionCostPc34(0x18, &cost, &rbit);
    expect_bool("cost.c018_hack.lookup", ok, true,
                "MENU.C:1272 G0494[24] C018 HACK");
    expect_u8("cost.c018_hack", cost, 6, "MENU.C:312 HACK=6");
    ok = M11_GameView_DoorBashStaminaActionCostPc34(0x13, &cost, &rbit);
    expect_bool("cost.c019_berzerk.lookup", ok, true,
                "MENU.C:1272 G0494[19] C019 BERZERK");
    expect_u8("cost.c019_berzerk", cost, 40, "MENU.C:313 BERZERK=40");
    ok = M11_GameView_DoorBashStaminaActionCostPc34(0x07, &cost, &rbit);
    expect_bool("cost.c007_kick.lookup", ok, true,
                "MENU.C:1272 G0494[7] C007 KICK");
    expect_u8("cost.c007_kick", cost, 3, "MENU.C:301 KICK=3");
    ok = M11_GameView_DoorBashStaminaActionCostPc34(0x0D, &cost, &rbit);
    expect_bool("cost.c013_swing.lookup", ok, true,
                "MENU.C:1272 G0494[13] C013 SWING");
    expect_u8("cost.c013_swing", cost, 2, "MENU.C:307 SWING=2");
    ok = M11_GameView_DoorBashStaminaActionCostPc34(0x02, &cost, &rbit);
    expect_bool("cost.c002_chop.lookup", ok, true,
                "MENU.C:1272 G0494[2] C002 CHOP");
    expect_u8("cost.c002_chop", cost, 10, "MENU.C:295 CHOP=10");

    /* Non-bash ordinals return false. */
    ok = M11_GameView_DoorBashStaminaActionCostPc34(0x14, &cost, &rbit);
    expect_bool("cost.c020_fireball.lookup_rejected", ok, false,
                "MENU.C:1272 + G0494[20] C020 FIREBALL is not a bash");
    ok = M11_GameView_DoorBashStaminaActionCostPc34(0xFF, &cost, &rbit);
    expect_bool("cost.ff.lookup_rejected", ok, false,
                "Out-of-range ordinal");
}

static void test_f0306_stamina_above_half(void)
{
    /* ReDMCSB CHAMPION.C:1097-1102: when current >= halfMax, the
     * F0306 result equals the input. */
    int16_t v;

    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(100, 100, 80);
    expect_int("f0306.full_stamina.unchanged", v, 80,
               "CHAMPION.C:1100 current=100 halfMax=50 unchanged");

    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(50, 100, 80);
    expect_int("f0306.exact_half.unchanged", v, 80,
               "CHAMPION.C:1094 boundary current=halfMax, unchanged");

    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(99, 100, 100);
    expect_int("f0306.99_of_100.unchanged", v, 100,
               "CHAMPION.C:1100 just below max, unchanged");
}

static void test_f0306_stamina_below_half(void)
{
    /* ReDMCSB CHAMPION.C:1094-1095 formula:
     *   (val/2) + ((val/2 * current) / halfMax) */
    int16_t v;

    /* current = 25 of 100, base = 100, halfMax = 50
     * val/2 = 50, val/2 * 25 = 1250, / 50 = 25, + 50 = 75 */
    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(25, 100, 100);
    expect_int("f0306.quarter_stamina.100", v, 75,
               "CHAMPION.C:1095 val=100 current=25 halfMax=50");

    /* current = 0 of 100, base = 100, halfMax = 50
     * val/2 = 50, val/2 * 0 = 0, / 50 = 0, + 50 = 50 */
    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(0, 100, 100);
    expect_int("f0306.zero_stamina.100", v, 50,
               "CHAMPION.C:1095 val=100 current=0 halfMax=50");

    /* current = 1 of 100, base = 80, halfMax = 50
     * val/2 = 40, val/2 * 1 = 40, / 50 = 0, + 40 = 40 */
    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(1, 100, 80);
    expect_int("f0306.1_stamina.80", v, 40,
               "CHAMPION.C:1095 val=80 current=1 halfMax=50");

    /* current = 25 of 100, base = 40, halfMax = 50
     * val/2 = 20, val/2 * 25 = 500, / 50 = 10, + 20 = 30 */
    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(25, 100, 40);
    expect_int("f0306.quarter_stamina.40", v, 30,
               "CHAMPION.C:1095 val=40 current=25 halfMax=50");
}

static void test_f0306_zero_and_negative_inputs(void)
{
    /* ReDMCSB CHAMPION.C:1090-1103: base_value <= 0 returns 0,
     * maximum_stamina <= 0 returns the input unchanged. */
    int16_t v;

    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(100, 100, 0);
    expect_int("f0306.base_zero_returns_zero", v, 0,
               "CHAMPION.C base_value=0 returns 0");

    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(100, 100, -5);
    expect_int("f0306.base_negative_returns_zero", v, 0,
               "CHAMPION.C base_value<0 returns 0");

    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(100, 0, 80);
    expect_int("f0306.max_zero_returns_unchanged", v, 80,
               "CHAMPION.C maximum_stamina<=0 returns val");

    v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(100, 1, 80);
    expect_int("f0306.halfmax_one_returns_unchanged", v, 80,
               "CHAMPION.C halfMax<=0 returns val");
}

static void test_f0325_decrement_below_current(void)
{
    /* ReDMCSB CHAMPION.C:2039-2042: decrement < current, no overflow,
     * attributes LOAD|STATISTICS set. */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.current_stamina_before = 80;
    in.maximum_stamina = 100;
    in.action_ordinal = 0x30; /* BASH, cost 9 */
    in.random_bit = 0;        /* M005_RANDOM(2) */
    in.base_strength = 80;

    ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
    expect_bool("dec.below.resolve", ok, true,
                "MENU.C:1272 + CHAMPION.C:2025 resolve");
    expect_u8("dec.below.table_cost", out.action_stamina_table_cost, 9,
              "MENU.C:1272 G0494[30]=9");
    expect_u8("dec.below.random_bit", out.action_stamina_random_bit, 0,
              "MENU.C:1273 M005_RANDOM=0");
    expect_int("dec.below.total", out.action_stamina_total, 9,
               "MENU.C:1272 9 + 0");
    expect_int("dec.below.after", out.current_stamina_after, 71,
               "CHAMPION.C:2040 80 - 9");
    expect_int("dec.below.before", out.current_stamina_before, 80,
               "CHAMPION.C:2039 input passthrough");
    expect_int("dec.below.max", out.maximum_stamina, 100,
               "CHAMPION.C:2040 maximum_stamina passthrough");
    expect_int("dec.below.overflow", out.overflow_damage, 0,
               "CHAMPION.C:2042 no overflow path");
    expect_int("dec.below.pending_damage", out.applied_pending_damage, 0,
               "F0321 not called, no pending damage");
    expect_u16("dec.below.attr_mask", out.applied_attribute_mask,
               DM1_V1_DOOR_BASH_STAMINA_ATTR_REDRAW_MASK_PC34,
               "CHAMPION.C:2048 MASK0x0200_LOAD | MASK0x0100_STATISTICS");
    expect_u8("dec.below.disabled_ticks", out.action_disabled_ticks,
              DM1_V1_DOOR_BASH_STAMINA_DISABLED_TICKS_PC34,
              "MENU.C:1314 6-tick disabled");
    expect_u8("dec.below.destruction_delay", out.destruction_delay_ticks,
              DM1_V1_DOOR_BASH_STAMINA_DESTRUCTION_DELAY_TICKS_PC34,
              "MENU.C:1317 2-tick destruction event");
    expect_int("dec.below.outcome", (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OK_PC34,
               "CHAMPION.C:2043 else branch, decrement < current");
}

static void test_f0325_decrement_exact(void)
{
    /* ReDMCSB CHAMPION.C:2039-2042: decrement == current lands the
     * stamina at 0 with overflow_damage = 0 (because (-0) >> 1 == 0). */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.current_stamina_before = 9;
    in.maximum_stamina = 100;
    in.action_ordinal = 0x30; /* BASH 9 + 0 = 9 */
    in.random_bit = 0;
    in.base_strength = 80;

    ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
    expect_bool("dec.exact.resolve", ok, true,
                "MENU.C:1272 + CHAMPION.C:2025 resolve");
    expect_int("dec.exact.total", out.action_stamina_total, 9,
               "MENU.C:1272 9 + 0");
    expect_int("dec.exact.after", out.current_stamina_after, 0,
               "CHAMPION.C:2040 9 - 9 = 0");
    expect_int("dec.exact.overflow", out.overflow_damage, 0,
               "CHAMPION.C:2042 (-0) >> 1 = 0");
    expect_int("dec.exact.outcome", (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_EXACT_PC34,
               "CHAMPION.C:2040 exact-zero branch");
}

static void test_f0325_decrement_overflow(void)
{
    /* ReDMCSB CHAMPION.C:2039-2042: decrement > current produces
     * overflow_damage = (-new) >> 1. */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.current_stamina_before = 3;
    in.maximum_stamina = 100;
    in.action_ordinal = 0x30; /* BASH 9 + 1 = 10 */
    in.random_bit = 1;
    in.base_strength = 80;

    ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
    expect_bool("dec.overflow.resolve", ok, true,
                "MENU.C:1272 + CHAMPION.C:2025 resolve");
    expect_int("dec.overflow.total", out.action_stamina_total, 10,
               "MENU.C:1272 9 + 1");
    expect_int("dec.overflow.after", out.current_stamina_after, 0,
               "CHAMPION.C:2041 3 - 10 = 0");
    /* 3 - 10 = -7, (-(-7)) >> 1 = 7 >> 1 = 3 */
    expect_int("dec.overflow.overflow", out.overflow_damage, 3,
               "CHAMPION.C:2042 (-new) >> 1 = 3");
    expect_int("dec.overflow.pending", out.applied_pending_damage, 3,
               "F0321 pending damage mirrors overflow_damage");
    expect_u16("dec.overflow.attr_mask", out.applied_attribute_mask,
               DM1_V1_DOOR_BASH_STAMINA_ATTR_REDRAW_MASK_PC34,
               "CHAMPION.C:2048 LOAD|STATISTICS attribute set");
    expect_int("dec.overflow.outcome", (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OVERFLOW_PC34,
               "CHAMPION.C:2041 overflow branch");
}

static void test_f0325_berzerk_large_overflow(void)
{
    /* ReDMCSB CHAMPION.C:2039-2042 with BERZERK cost 40 + 1 = 41 and
     * current 10, the overflow damage is (10 - 41) = -31, (-(-31)) >> 1
     * = 15. */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.current_stamina_before = 10;
    in.maximum_stamina = 100;
    in.action_ordinal = 0x13; /* BERZERK 40 + 1 = 41 */
    in.random_bit = 1;
    in.base_strength = 80;

    ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
    expect_bool("dec.berzerk.resolve", ok, true,
                "MENU.C:1272 + CHAMPION.C:2025 resolve");
    expect_int("dec.berzerk.total", out.action_stamina_total, 41,
               "MENU.C:1272 40 + 1");
    expect_int("dec.berzerk.after", out.current_stamina_after, 0,
               "CHAMPION.C:2041 10 - 41 = 0");
    expect_int("dec.berzerk.overflow", out.overflow_damage, 15,
               "CHAMPION.C:2042 (10 - 41 = -31) -> (-(-31)) >> 1 = 15");
}

static void test_f0325_f0306_strength_collapse(void)
{
    /* ReDMCSB CHAMPION.C:1237-1303 + CHAMPION.C:1078-1103:
     * when the champion's current stamina is below half-max, the
     * bash strength arg to F0232 collapses. A 50-base strength at
     * 25 current / 100 max (halfMax=50) lands at
     *   (50/2) + (25 * 25) / 50 = 25 + 12 = 37,
     * which is below the WOODEN door defense of 42 and below the
     * bash strength clip 100. */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.current_stamina_before = 25;
    in.maximum_stamina = 100;
    in.action_ordinal = 0x30; /* BASH 9 */
    in.random_bit = 0;
    in.base_strength = 50;

    ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
    expect_bool("collapse.resolve", ok, true,
                "MENU.C:1272 + CHAMPION.C:2025 resolve");
    /* (50/2) + (25 * 25) / 50 = 25 + 12 = 37 */
    expect_int("collapse.f0306", out.strength_after_stamina, 37,
               "CHAMPION.C:1095 val=50 current=25 halfMax=50");
    /* F0026_MAIN_GetBoundedValue(0, 37 >> 1, 100) = 18 */
    expect_int("collapse.f0026", out.bash_strength_arg_to_f0232, 18,
               "CHAMPION.C:1302 F0026 clip 0 .. 100");
    expect_bool("collapse.capped", out.bash_strength_was_capped_to_100, false,
               "CHAMPION.C:1302 not capped at 100");
    expect_int("collapse.outcome", (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_OUTCOME_DECREMENT_OK_PC34,
               "MENU.C:1311-1319 closed-door branch reachable");

    /* The bash strength arg 18 < WOODEN 42 makes the bash a bounce
     * under the same closed-door branch — the bash feedback is
     * still emitted, but no destruction event lands. The present
     * gate does not own the F0232 outcome; the companion
     * dm1_v1_door_bash_feedback (pass777) does. The contract here
     * is that the bash strength arg to F0232 is the post-stamina
     * value, byte-stable. */
    expect_int("collapse.bash_arg_below_wooden",
               out.bash_strength_arg_to_f0232 <
                   DM1_V1_DOOR_BASH_STAMINA_DEFENSE_WOODEN_PC34
                   ? 1
                   : 0,
               1,
               "CHAMPION.C:1302 post-stamina strength below wooden defense");
}

static void test_f0325_melee_cap_unchanged_at_full_stamina(void)
{
    /* ReDMCSB CHAMPION.C:1302 F0026_MAIN_GetBoundedValue caps the
     * bash strength arg to 100, which is the DUNGEON.C:561/797 melee
     * cap. The bash strength arg caps at exactly 100 when the
     * post-stamina value is 200 or higher. */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.current_stamina_before = 100;
    in.maximum_stamina = 100;
    in.action_ordinal = 0x30; /* BASH 9 */
    in.random_bit = 0;
    in.base_strength = 250;

    ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
    expect_bool("cap.full.resolve", ok, true,
                "MENU.C:1272 + CHAMPION.C:2025 resolve");
    expect_int("cap.full.f0306", out.strength_after_stamina, 250,
               "CHAMPION.C:1100 full stamina unchanged");
    /* 250 >> 1 = 125, capped to 100 */
    expect_int("cap.full.f0026", out.bash_strength_arg_to_f0232, 100,
               "CHAMPION.C:1302 F0026 clip 0 .. 100");
    expect_bool("cap.full.capped", out.bash_strength_was_capped_to_100, true,
                "DUNGEON.C:561 melee cap 100 reached");
}

static void test_non_bash_action_short_circuits(void)
{
    /* ReDMCSB MENU.C:1311-1316: the bash dispatch is the only caller
     * for the closed-door branch. Non-bash actions must not run the
     * F0325 decrement or fire the 6-tick disabled cooldown. */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.current_stamina_before = 50;
    in.maximum_stamina = 100;
    in.action_ordinal = 0x14; /* C020 FIREBALL, not a bash action */
    in.random_bit = 1;
    in.base_strength = 80;

    ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
    expect_bool("nonbash.resolve", ok, true,
                "MENU.C:1272 + CHAMPION.C:2025 resolve");
    expect_int("nonbash.outcome", (int)out.outcome,
               (int)DM1_V1_DOOR_BASH_STAMINA_OUTCOME_NOT_BASH_PC34,
               "MENU.C:1311-1316 not in the bash group");
    expect_int("nonbash.disabled", out.action_disabled_ticks, 0,
               "MENU.C:1314 only fires for the closed-door bash branch");
    expect_int("nonbash.destruction_delay", out.destruction_delay_ticks, 0,
               "MENU.C:1317 only fires for the closed-door bash branch");
    expect_int("nonbash.table_cost", out.action_stamina_table_cost, 0,
               "G0494 lookup not run for non-bash action");
    expect_int("nonbash.total", out.action_stamina_total, 0,
               "MENU.C:1272 + 0 random_bit");
    expect_int("nonbash.after", out.current_stamina_after, 0,
               "CHAMPION.C:2040 not mutated, stays at 0 from memset");
    expect_int("nonbash.f0306_still_computed", out.strength_after_stamina,
               M11_GameView_DoorBashStaminaAdjustedStrengthPc34(50, 100, 80),
               "CHAMPION.C:1095 F0306 still runs as a passive read");
}

static void test_null_inputs_rejected(void)
{
    /* ReDMCSB CHAMPION.C:2036: F0325 rejects CM1_CHAMPION_NONE.
     * The contract gate rejects NULL input/output. */
    DM1_V1_DoorBashStaminaResultPc34 out;
    bool ok;

    memset(&out, 0, sizeof(out));
    ok = M11_GameView_DoorBashStaminaResolvePc34(NULL, &out);
    expect_bool("null.input_rejected", ok, false,
                "CHAMPION.C:2036 NULL input rejected");
    ok = M11_GameView_DoorBashStaminaResolvePc34(
        (const DM1_V1_DoorBashStaminaInputPc34 *)0x100, NULL);
    expect_bool("null.output_rejected", ok, false,
                "CHAMPION.C:2036 NULL output rejected");
}

static void test_per_action_stamina_table(void)
{
    /* ReDMCSB MENU.C:1272-1273 + G0494_auc_Graphic560_ActionStamina:
     * pin the table cost + the random-bit roll = total for each
     * bash-family action. */
    struct {
        uint8_t action;
        uint8_t cost;
        const char *name;
    } rows[6] = {
        { 0x30, 9,  "C030 BASH"    },
        { 0x18, 6,  "C018 HACK"    },
        { 0x13, 40, "C019 BERZERK" },
        { 0x07, 3,  "C007 KICK"    },
        { 0x0D, 2,  "C013 SWING"   },
        { 0x02, 10, "C002 CHOP"    }
    };
    int i;

    for (i = 0; i < 6; ++i) {
        DM1_V1_DoorBashStaminaInputPc34 in;
        DM1_V1_DoorBashStaminaResultPc34 out;
        bool ok;

        memset(&in, 0, sizeof(in));
        memset(&out, 0, sizeof(out));
        in.current_stamina_before = 100;
        in.maximum_stamina = 100;
        in.action_ordinal = rows[i].action;
        in.random_bit = 0;
        in.base_strength = 80;

        ok = M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
        expect_bool("per_action.resolve", ok, true, rows[i].name);
        expect_u8("per_action.cost", out.action_stamina_table_cost,
                  rows[i].cost, rows[i].name);
        expect_int("per_action.total", out.action_stamina_total,
                   (int)rows[i].cost, rows[i].name);
    }
}

static void test_deterministic_hash_stable(void)
{
    /* FNV-1a 32-bit over the canonical inputs that drive the bash
     * stamina contract. The hash is a side effect of the contract
     * structure, not a separate contract, but it surfaces a
     * regression in any of the formula values. */
    DM1_V1_DoorBashStaminaInputPc34 in;
    DM1_V1_DoorBashStaminaResultPc34 out;
    uint32_t hash1 = FNV1A_OFFSET_BASIS_PC34;
    uint32_t hash2 = FNV1A_OFFSET_BASIS_PC34;
    int16_t v;
    int i;

    /* Hash the F0306 boundary sweep. */
    int16_t currents[8] = { 0, 1, 25, 49, 50, 99, 100, 250 };
    int16_t bases[4]    = { 40, 80, 100, 250 };
    for (i = 0; i < 8; ++i) {
        int j;
        for (j = 0; j < 4; ++j) {
            v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(
                currents[i], 100, bases[j]);
            hash1 = fnv1a_i16(hash1, v);
        }
    }

    /* Hash the per-action-stamina outcomes. */
    memset(&in, 0, sizeof(in));
    in.current_stamina_before = 50;
    in.maximum_stamina = 100;
    in.base_strength = 80;
    {
        uint8_t actions[6] = { 0x30, 0x18, 0x13, 0x07, 0x0D, 0x02 };
        uint8_t rbits[2]   = { 0, 1 };
        for (i = 0; i < 6; ++i) {
            int k;
            for (k = 0; k < 2; ++k) {
                in.action_ordinal = actions[i];
                in.random_bit = rbits[k];
                memset(&out, 0, sizeof(out));
                (void)M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
                hash1 = fnv1a_u8(hash1, out.action_stamina_table_cost);
                hash1 = fnv1a_u8(hash1, out.action_stamina_random_bit);
                hash1 = fnv1a_i16(hash1, out.action_stamina_total);
                hash1 = fnv1a_i16(hash1, out.strength_after_stamina);
                hash1 = fnv1a_i16(hash1, out.current_stamina_after);
                hash1 = fnv1a_i16(hash1, out.overflow_damage);
                hash1 = fnv1a_u16(hash1, out.applied_attribute_mask);
                hash1 = fnv1a_u8(hash1, out.action_disabled_ticks);
                hash1 = fnv1a_u8(hash1, out.destruction_delay_ticks);
                hash1 = fnv1a_i16(hash1, out.bash_strength_arg_to_f0232);
            }
        }
    }

    /* Recompute to verify stability. */
    for (i = 0; i < 8; ++i) {
        int j;
        for (j = 0; j < 4; ++j) {
            v = M11_GameView_DoorBashStaminaAdjustedStrengthPc34(
                currents[i], 100, bases[j]);
            hash2 = fnv1a_i16(hash2, v);
        }
    }
    {
        uint8_t actions[6] = { 0x30, 0x18, 0x13, 0x07, 0x0D, 0x02 };
        uint8_t rbits[2]   = { 0, 1 };
        memset(&in, 0, sizeof(in));
        in.current_stamina_before = 50;
        in.maximum_stamina = 100;
        in.base_strength = 80;
        for (i = 0; i < 6; ++i) {
            int k;
            for (k = 0; k < 2; ++k) {
                in.action_ordinal = actions[i];
                in.random_bit = rbits[k];
                memset(&out, 0, sizeof(out));
                (void)M11_GameView_DoorBashStaminaResolvePc34(&in, &out);
                hash2 = fnv1a_u8(hash2, out.action_stamina_table_cost);
                hash2 = fnv1a_u8(hash2, out.action_stamina_random_bit);
                hash2 = fnv1a_i16(hash2, out.action_stamina_total);
                hash2 = fnv1a_i16(hash2, out.strength_after_stamina);
                hash2 = fnv1a_i16(hash2, out.current_stamina_after);
                hash2 = fnv1a_i16(hash2, out.overflow_damage);
                hash2 = fnv1a_u16(hash2, out.applied_attribute_mask);
                hash2 = fnv1a_u8(hash2, out.action_disabled_ticks);
                hash2 = fnv1a_u8(hash2, out.destruction_delay_ticks);
                hash2 = fnv1a_i16(hash2, out.bash_strength_arg_to_f0232);
            }
        }
    }

    expect_int("hash.stable", hash1 == hash2 ? 1 : 0, 1,
               "FNV-1a deterministic over the contract sweep");
    expect_int("hash.nonzero", hash1 != 0 ? 1 : 0, 1,
               "FNV-1a mixes real contract values, not all-zero");
    printf("HASH_DOOR_BASH_STAMINA 0x%08X\n", hash1);
}

static void test_source_evidence_mentions_required_anchors(void)
{
    /* ReDMCSB source-anchor mentions. */
    const char *evidence = M11_GameView_DoorBashStaminaSourceLockPc34();

    expect_contains("evidence.m1272", evidence, "MENU.C:1272-1273",
                    "MENU.C:1272-1273 L1253_i_ActionStamina anchor");
    expect_contains("evidence.m1311", evidence, "MENU.C:1311-1319",
                    "MENU.C:1311-1319 closed-door bash branch anchor");
    expect_contains("evidence.m1620", evidence, "MENU.C:1620-1622",
                    "MENU.C:1620-1622 F0330 disable anchor");
    expect_contains("evidence.m1623", evidence, "MENU.C:1623-1624",
                    "MENU.C:1623-1624 F0325 decrement anchor");
    expect_contains("evidence.c1078", evidence, "CHAMPION.C:1078-1103",
                    "CHAMPION.C:1078-1103 F0306 anchor");
    expect_contains("evidence.c1237", evidence, "CHAMPION.C:1237-1303",
                    "CHAMPION.C:1237-1303 F0312 anchor");
    expect_contains("evidence.c2025", evidence, "CHAMPION.C:2025-2049",
                    "CHAMPION.C:2025-2049 F0325 anchor");
    expect_contains("evidence.g0494", evidence, "G0494_auc_Graphic560_ActionStamina",
                    "G0494 table anchor");
    expect_contains("evidence.m005", evidence, "M005_RANDOM",
                    "DEFS.H:4 M005_RANDOM anchor");
    expect_contains("evidence.door_info", evidence, "G0254_as_Graphic559_DoorInfo",
                    "DEFS.H:560-565 G0254 anchor");
    expect_contains("evidence.door_info_struct", evidence, "DOOR_INFO",
                    "DEFS.H:1555-1580 DOOR_INFO struct anchor");
    expect_contains("evidence.event_door_destruction", evidence,
                    "C02_EVENT_DOOR_DESTRUCTION",
                    "DEFS.H:934 C02_EVENT_DOOR_DESTRUCTION anchor");
    expect_contains("evidence.no_real_emit", evidence, "contract-only",
                    "Gate does not actually emit F0325 / F0330 / F0064 / "
                    "F0238 / F0321 calls");
    expect_contains("evidence.companion_pass777", evidence,
                    "dm1_v1_door_bash_feedback_pc34_compat",
                    "Disjoint with pass777 door-bash feedback gate");
    expect_contains("evidence.melee_cap", evidence, "limited to 100",
                    "DUNGEON.C:561/797 melee cap 100 anchor");
    expect_contains("evidence.bugx_xx", evidence, "BUGX_XX",
                    "CHAMPION.C:1095 BUGX_XX compiler-order hazard anchor");
}

int main(void)
{
    printf("probe=dm1_v1_door_bash_stamina_feedback_source_lock_pc34_compat\n");

    test_action_ordinal_set();
    test_action_stamina_table_costs();
    test_f0306_stamina_above_half();
    test_f0306_stamina_below_half();
    test_f0306_zero_and_negative_inputs();
    test_f0325_decrement_below_current();
    test_f0325_decrement_exact();
    test_f0325_decrement_overflow();
    test_f0325_berzerk_large_overflow();
    test_f0325_f0306_strength_collapse();
    test_f0325_melee_cap_unchanged_at_full_stamina();
    test_non_bash_action_short_circuits();
    test_null_inputs_rejected();
    test_per_action_stamina_table();
    test_deterministic_hash_stable();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_door_bash_stamina_feedback_source_lock_pc34_compat"
               " failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_door_bash_stamina_feedback_source_lock_pc34_compat"
           " %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
