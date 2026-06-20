#include "firestaff/dm1/v1/animtown_color_light0_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_animtown_color_light0_table_pc34();
    int n = dm1_v1_animtown_color_light0_size_pc34();
    CHECK(t != 0);
    CHECK(n == 17);
    CHECK(t[0] == 0x10);
    CHECK(t[16 * 4 + 0] == 0xFF);
}

static void test_lookup_function(void)
{
    int i, j;
    for (i = 0; i < 17; ++i) {
        for (j = 0; j < 4; ++j) {
            CHECK(dm1_v1_animtown_color_light0_get_pc34(i, j) >= 0);
            CHECK(dm1_v1_animtown_color_light0_get_pc34(i, j) <= 0xFF);
        }
    }
    CHECK(dm1_v1_animtown_color_light0_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_animtown_color_light0_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_animtown_color_light0_get_pc34(17, 0) == -1);
    CHECK(dm1_v1_animtown_color_light0_get_pc34(0, 4) == -1);
    CHECK(dm1_v1_animtown_color_light0_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_animtown_color_light0_get_pc34(0, 0) == 0x10);
    CHECK(dm1_v1_animtown_color_light0_get_pc34(16, 0) == 0xFF);
}

static void test_run_accepted(void)
{
    DM1_V1_AnimtownColorLight0ResultPc34 r;
    int ok = dm1_v1_animtown_color_light0_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 7);
    CHECK(r.tableSize == 17);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntryIndex0x10 == 1);
    CHECK(r.lastEntrySentinelIndex0xFF == 1);
    CHECK(r.allRgbInByteRange == 1);
    CHECK(r.allIndicesNonZeroExceptLast == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 17 * 4; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_animtown_color_light0_table_pc34()[i]);
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_animtown_color_light0: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
