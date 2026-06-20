#include "firestaff/dm1/v1/bar_graph_masks_pc34_compat.h"

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
    const unsigned int *t = dm1_v1_bar_graph_masks_table_pc34();
    int n = dm1_v1_bar_graph_masks_size_pc34();
    CHECK(t != 0);
    CHECK(n == 24);
    /* Champion 0 health mask pair = {0x0003, 0xC000}. */
    CHECK(t[0] == 0x0003);
    CHECK(t[1] == 0xC000);
    /* Champion 3 health mask pair = {0x0007, 0x8000}. */
    CHECK(t[18] == 0x0007);
    CHECK(t[19] == 0x8000);
    /* Champion 1 mana mask pair = {0x003C, 0x0000}. */
    CHECK(t[8]  == 0x003C);
    CHECK(t[9]  == 0x0000);
    /* Last entry is stamina mask pair for C3 = {0x001E, 0x0000}. */
    CHECK(t[22] == 0x001E);
    CHECK(t[23] == 0x0000);
}

static void test_lookup_function(void)
{
    int c, g, m;
    for (c = 0; c < 4; ++c) {
        for (g = 0; g < 3; ++g) {
            for (m = 0; m < 2; ++m) {
                CHECK(dm1_v1_bar_graph_masks_get_pc34(c, g, m) >= 0);
                CHECK(dm1_v1_bar_graph_masks_get_pc34(c, g, m) <= 0xFFFF);
            }
        }
    }
    CHECK(dm1_v1_bar_graph_masks_get_pc34(-1, 0, 0) == -1);
    CHECK(dm1_v1_bar_graph_masks_get_pc34(0, -1, 0) == -1);
    CHECK(dm1_v1_bar_graph_masks_get_pc34(0, 0, -1) == -1);
    CHECK(dm1_v1_bar_graph_masks_get_pc34(4, 0, 0) == -1);
    CHECK(dm1_v1_bar_graph_masks_get_pc34(0, 3, 0) == -1);
    CHECK(dm1_v1_bar_graph_masks_get_pc34(0, 0, 2) == -1);
    CHECK(dm1_v1_bar_graph_masks_get_pc34(999, 999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_bar_graph_masks_get_pc34(0, 0, 0) == 0x0003);
    CHECK(dm1_v1_bar_graph_masks_get_pc34(3, 2, 1) == 0x0000);
}

static void test_run_accepted(void)
{
    DM1_V1_BarGraphMasksResultPc34 r;
    int ok = dm1_v1_bar_graph_masks_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.tableSize == 24);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.champion0HealthMaskPair0003C000 == 1);
    CHECK(r.champion1HealthMaskPair1E000000 == 1);
    CHECK(r.champion2HealthMaskPair00F00000 == 1);
    CHECK(r.champion3HealthMaskPair00078000 == 1);
    CHECK(r.allMasksNonZero == 1);
    CHECK(r.allMasksInUint16Range == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 24; ++i) {
        CHECK(r.tableEntries[i] == (int)dm1_v1_bar_graph_masks_table_pc34()[i]);
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_bar_graph_masks: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}