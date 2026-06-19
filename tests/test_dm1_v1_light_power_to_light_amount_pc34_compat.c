#include "firestaff/dm1/v1/light_power_to_light_amount_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_table_values(void)
{
    /* DATA.C:359 G0039 init:
     *   { 0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100 }
     */
    const int *t = dm1_v1_light_power_to_light_amount_table_pc34();
    int n = dm1_v1_light_power_to_light_amount_size_pc34();
    CHECK(t != 0);
    CHECK(n == 16);
    CHECK(t[0]  ==   0);
    CHECK(t[1]  ==   5);
    CHECK(t[2]  ==  12);
    CHECK(t[3]  ==  24);
    CHECK(t[4]  ==  33);
    CHECK(t[5]  ==  40);
    CHECK(t[6]  ==  46);
    CHECK(t[7]  ==  51);
    CHECK(t[8]  ==  59);
    CHECK(t[9]  ==  68);
    CHECK(t[10] ==  76);
    CHECK(t[11] ==  82);
    CHECK(t[12] ==  89);
    CHECK(t[13] ==  94);
    CHECK(t[14] ==  97);
    CHECK(t[15] == 100);
}

static void test_lookup_function(void)
{
    /* All 16 valid indices return the expected value. */
    int i;
    static const int kExpected[16] = {
        0, 5, 12, 24, 33, 40, 46, 51,
        59, 68, 76, 82, 89, 94, 97, 100
    };
    for (i = 0; i < 16; ++i) {
        CHECK(dm1_v1_light_power_to_light_amount_pc34(i) == kExpected[i]);
    }
    /* Out-of-range returns 0 (sentinel). */
    CHECK(dm1_v1_light_power_to_light_amount_pc34(-1) == 0);
    CHECK(dm1_v1_light_power_to_light_amount_pc34(16) == 0);
    CHECK(dm1_v1_light_power_to_light_amount_pc34(999) == 0);
}

static void test_illumulet_constant(void)
{
    /* CHAMPION.C:529 + 645 — F0291_CHAMPION_DrawSlot reads G0039[2]
     * (the Illumulet-equip magical light delta).
     */
    CHECK(dm1_v1_light_power_to_light_amount_illumulet_index_pc34() == 2);
    CHECK(dm1_v1_light_power_to_light_amount_illumulet_amount_pc34() == 12);
    CHECK(dm1_v1_light_power_to_light_amount_pc34(2) == 12);
}

static void test_diff_helper(void)
{
    /* TIMELINE.C:1754 — Diff = G0039[Strong] - G0039[Weaker]. */
    /* 15 - 0 = 100 - 0 = 100 */
    CHECK(dm1_v1_light_power_to_light_amount_diff_pc34(15, 0) == 100);
    /* 8 - 4 = 59 - 33 = 26 */
    CHECK(dm1_v1_light_power_to_light_amount_diff_pc34(8, 4) == 26);
    /* 5 - 2 = 40 - 12 = 28 */
    CHECK(dm1_v1_light_power_to_light_amount_diff_pc34(5, 2) == 28);
    /* Same index -> 0. */
    CHECK(dm1_v1_light_power_to_light_amount_diff_pc34(7, 7) == 0);
    /* Reversed -> negative. The source flips the sign for negative
     * light powers (TIMELINE.C:1757); the helper here is the raw
     * diff.
     */
    CHECK(dm1_v1_light_power_to_light_amount_diff_pc34(2, 5) == -28);
    /* Out-of-range diff returns the raw diff with the OOB side as 0. */
    CHECK(dm1_v1_light_power_to_light_amount_diff_pc34(15, -1) == 100);
    CHECK(dm1_v1_light_power_to_light_amount_diff_pc34(-1, 0) == 0);
}

static void test_monotonic_and_range(void)
{
    /* The values form a monotonically non-decreasing curve that
     * saturates at 100.
     */
    const int *t = dm1_v1_light_power_to_light_amount_table_pc34();
    int i;
    for (i = 0; i < 15; ++i) {
        CHECK(t[i] <= t[i + 1]);
    }
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 100);
    }
}

static void test_max_value(void)
{
    /* The max value is 100 (used by PANEL.C:412 with the
     * Multiplier+right-shift-6 scaling for the torch-light sum).
     */
    CHECK(dm1_v1_light_power_to_light_amount_max_value_pc34() == 100);
}

static void test_run_accepted(void)
{
    DM1_V1_LightPowerToLightAmountResultPc34 r;
    int ok = dm1_v1_light_power_to_light_amount_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 16);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] ==
              dm1_v1_light_power_to_light_amount_pc34(i));
    }
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntryZero == 1);
    CHECK(r.lastEntry100 == 1);
    CHECK(r.monotonicallyNonDecreasing == 1);
    CHECK(r.allWithinRange0_100 == 1);
    CHECK(r.illumuletConstant12 == 1);
    CHECK(r.lookupFunctionInRange == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    CHECK(r.diffHelperCorrect == 1);
    CHECK(r.diffHelperZeroWhenSame == 1);
    CHECK(r.diffHelperSignAware == 1);
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_illumulet_constant();
    test_diff_helper();
    test_monotonic_and_range();
    test_max_value();
    test_run_accepted();
    printf("dm1_v1_light_power_to_light_amount: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}