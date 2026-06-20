#include "firestaff/dm1/v1/animtown_color_swoosh8_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_animtown_color_swoosh8_table_pc34();
    int n = dm1_v1_animtown_color_swoosh8_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 5);
    /* Last entry must be sentinel (255, 0, 0, 0). */
    CHECK(t[(5 - 1) * 4 + 0] == 0xFF);
    /* All non-last entries have Index in [0, 0x1F]. */
    for (i = 0; i < 5 - 1; ++i) {
        CHECK(t[i * 4 + 0] <= 0x1F);
    }
}

static void test_lookup_function(void)
{
    int i, j;
    for (i = 0; i < 5; ++i) {
        for (j = 0; j < 4; ++j) {
            CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(i, j) >= 0);
            CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(i, j) <= 0xFF);
        }
    }
    CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(5, 0) == -1);
    CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(0, 4) == -1);
    CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    /* Sentinel terminator at last entry. */
    CHECK(dm1_v1_animtown_color_swoosh8_get_pc34(5 - 1, 0) == 0xFF);
}

static void test_run_accepted(void)
{
    DM1_V1_AnimtownColorSwoosh8ResultPc34 r;
    int ok = dm1_v1_animtown_color_swoosh8_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 6);
    CHECK(r.tableSize == 5);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.lastEntrySentinelIndex0xFF == 1);
    CHECK(r.allRgbInByteRange == 1);
    CHECK(r.allIndicesNonZeroExceptLast == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 5 * 4; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_animtown_color_swoosh8_table_pc34()[i]);
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_animtown_color_swoosh8: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
