#include "firestaff/dm1/v1/palette_index_to_light_amount_pc34_compat.h"

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
    /* DATA.C:360 G0040 init:
     *   { 99, 75, 50, 25, 1, 0 }
     * The thresholds for palette index 0..5 (brightest..darkest).
     */
    const int *t = dm1_v1_palette_index_to_light_amount_table_pc34();
    int n = dm1_v1_palette_index_to_light_amount_size_pc34();
    CHECK(t != 0);
    CHECK(n == 6);
    CHECK(t[0] == 99);
    CHECK(t[1] == 75);
    CHECK(t[2] == 50);
    CHECK(t[3] == 25);
    CHECK(t[4] == 1);
    CHECK(t[5] == 0);
}

static void test_lookup_function(void)
{
    /* Each palette index returns the expected threshold. */
    int i;
    static const int kExpected[6] = { 99, 75, 50, 25, 1, 0 };
    for (i = 0; i < 6; ++i) {
        CHECK(dm1_v1_palette_index_to_light_amount_pc34(i) == kExpected[i]);
    }
    /* Out-of-range returns 0 (sentinel). */
    CHECK(dm1_v1_palette_index_to_light_amount_pc34(-1) == 0);
    CHECK(dm1_v1_palette_index_to_light_amount_pc34(6) == 0);
    CHECK(dm1_v1_palette_index_to_light_amount_pc34(999) == 0);
}

static void test_index_constants(void)
{
    /* Palette 0 is brightest (highest threshold), palette 5 is darkest. */
    CHECK(dm1_v1_palette_index_to_light_amount_brightest_index_pc34() == 0);
    CHECK(dm1_v1_palette_index_to_light_amount_darkest_index_pc34() == 5);
    CHECK(dm1_v1_palette_index_to_light_amount_brightest_threshold_pc34() == 99);
}

static void test_select_brightest(void)
{
    /* PANEL.C:419-423 — for TotalLightAmount >= 99 the loop does not
     * advance past palette 0. */
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(99) == 0);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(100) == 0);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(200) == 0);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(1000) == 0);
}

static void test_select_darkest(void)
{
    /* PANEL.C:419-423 — for TotalLightAmount <= 0 the dark branch
     * selects palette 5 directly. */
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(0) == 5);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(-1) == 5);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(-1000) == 5);
}

static void test_select_boundaries(void)
{
    /* Walk each threshold boundary:
     *   total=99 -> PI=0 (brightest)
     *   total=76 -> PI=1 (99>76, 75>76 false)
     *   total=75 -> PI=1 (99>75, 75>75 false)
     *   total=74 -> PI=2 (99>74, 75>74, 50>74 false)
     *   total=51 -> PI=2 (99>51, 75>51, 50>51 false)
     *   total=50 -> PI=2 (50>50 false)
     *   total=49 -> PI=3 (50>49 true, 25>49 false)
     *   total=26 -> PI=3 (25>26 false)
     *   total=25 -> PI=3 (25>25 false)
     *   total=24 -> PI=4 (25>24 true, 1>24 false)
     *   total=2  -> PI=4 (1>2 false)
     *   total=1  -> PI=4 (1>1 false)
     */
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(76) == 1);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(75) == 1);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(74) == 2);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(51) == 2);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(50) == 2);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(49) == 3);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(26) == 3);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(25) == 3);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(24) == 4);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(2) == 4);
    CHECK(dm1_v1_palette_index_to_light_amount_select_pc34(1) == 4);
}

static void test_monotonic_and_range(void)
{
    const int *t = dm1_v1_palette_index_to_light_amount_table_pc34();
    int i;
    for (i = 0; i < 5; ++i) {
        CHECK(t[i] >= t[i + 1]);
    }
    for (i = 0; i < 6; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 99);
    }
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteIndexToLightAmountResultPc34 r;
    int ok = dm1_v1_palette_index_to_light_amount_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 6);
    CHECK(r.tableEntries[0] == 99);
    CHECK(r.tableEntries[1] == 75);
    CHECK(r.tableEntries[2] == 50);
    CHECK(r.tableEntries[3] == 25);
    CHECK(r.tableEntries[4] == 1);
    CHECK(r.tableEntries[5] == 0);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntry99 == 1);
    CHECK(r.lastEntry0 == 1);
    CHECK(r.monotonicallyNonIncreasing == 1);
    CHECK(r.allWithinRange0_99 == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    CHECK(r.selectPaletteIndexBrightestFor99Plus == 1);
    CHECK(r.selectPaletteIndexDarkestFor0 == 1);
    CHECK(r.selectPaletteIndexBoundariesCorrect == 1);
    CHECK(r.selectPaletteIndexBoundaryTests == 1);
    /* Spot-check that the table copies to the result struct match the
     * source-of-truth lookup function for all six entries.
     */
    for (i = 0; i < 6; ++i) {
        CHECK(r.tableEntries[i] ==
              dm1_v1_palette_index_to_light_amount_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_index_constants();
    test_select_brightest();
    test_select_darkest();
    test_select_boundaries();
    test_monotonic_and_range();
    test_run_accepted();
    printf("dm1_v1_palette_index_to_light_amount: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}