#include "firestaff/dm1/v1/champion_color_pc34_compat.h"

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
    /* DATA.C:423 G0046 init: { 7, 11, 8, 14 }. */
    const unsigned char *t = dm1_v1_champion_color_table_pc34();
    int n = dm1_v1_champion_color_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 7);   /* leader */
    CHECK(t[1] == 11);  /* 1st follower */
    CHECK(t[2] == 8);
    CHECK(t[3] == 14);
}

static void test_lookup_function(void)
{
    /* Lookup returns the expected value for each champion index. */
    CHECK(dm1_v1_champion_color_pc34(0) == 7);
    CHECK(dm1_v1_champion_color_pc34(1) == 11);
    CHECK(dm1_v1_champion_color_pc34(2) == 8);
    CHECK(dm1_v1_champion_color_pc34(3) == 14);
    /* OOB returns 0. */
    CHECK(dm1_v1_champion_color_pc34(-1) == 0);
    CHECK(dm1_v1_champion_color_pc34(4) == 0);
    CHECK(dm1_v1_champion_color_pc34(999) == 0);
}

static void test_leader_helper(void)
{
    /* The leader (champion 0) gets color 7 (LIGHT_GRAY). */
    CHECK(dm1_v1_champion_color_leader_pc34() == 7);
    CHECK(dm1_v1_champion_color_leader_pc34() ==
          dm1_v1_champion_color_pc34(0));
}

static void test_colors_distinct(void)
{
    /* All 4 champion colors must be distinct. */
    int i, j;
    const unsigned char *t = dm1_v1_champion_color_table_pc34();
    for (i = 0; i < 4; ++i) {
        for (j = i + 1; j < 4; ++j) {
            CHECK(t[i] != t[j]);
        }
    }
}

static void test_colors_in_palette_range(void)
{
    /* All 4 colors must be valid 4-bit palette indices [0, 15]. */
    int i;
    const unsigned char *t = dm1_v1_champion_color_table_pc34();
    for (i = 0; i < 4; ++i) {
        CHECK(t[i] <= 15);
    }
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionColorResultPc34 r;
    int ok = dm1_v1_champion_color_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.tableEntries[0] == 7);
    CHECK(r.tableEntries[1] == 11);
    CHECK(r.tableEntries[2] == 8);
    CHECK(r.tableEntries[3] == 14);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.leaderColorIs7 == 1);
    CHECK(r.firstFollowerColorIs11 == 1);
    CHECK(r.allColorsDistinct == 1);
    CHECK(r.allColorsInRange0to15 == 1);
    CHECK(r.lookupFunctionInRange == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    CHECK(r.dispatchByChampionIndexCorrect == 1);
    /* Cross-check the table entries match the source-of-truth lookup
     * function.
     */
    for (i = 0; i < 4; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_champion_color_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_leader_helper();
    test_colors_distinct();
    test_colors_in_palette_range();
    test_run_accepted();
    printf("dm1_v1_champion_color: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}