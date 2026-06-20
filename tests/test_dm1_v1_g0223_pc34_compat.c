#include "firestaff/dm1/v1/G0223_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0223_table_pc34();
    int n = dm1_v1_g0223_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 24);
    /* Row 0: {0, 1, 2, 3, 0, -3, -2, -1} */
    CHECK(dm1_v1_g0223_get_pc34(0, 0) == 0);
    CHECK(dm1_v1_g0223_get_pc34(0, 1) == 1);
    CHECK(dm1_v1_g0223_get_pc34(0, 2) == 2);
    CHECK(dm1_v1_g0223_get_pc34(0, 3) == 3);
    CHECK(dm1_v1_g0223_get_pc34(0, 5) == -3);
    CHECK(dm1_v1_g0223_get_pc34(0, 7) == -1);
    /* Row 2 (last): {0, 1, 1, 1, 0, -1, -1, -1} */
    CHECK(dm1_v1_g0223_get_pc34(2, 0) == 0);
    CHECK(dm1_v1_g0223_get_pc34(2, 7) == -1);
    /* All values fit in int16_t. */
    for (i = 0; i < 24; ++i) {
        CHECK(dm1_v1_g0223_get_pc34(i / 8, i % 8) >= -32768);
        CHECK(dm1_v1_g0223_get_pc34(i / 8, i % 8) <= 32767);
    }
}

static void test_lookup_function(void)
{
    int row, col;
    for (row = 0; row < 3; ++row) {
        for (col = 0; col < 8; ++col) {
            CHECK(dm1_v1_g0223_get_pc34(row, col) >= -32768);
            CHECK(dm1_v1_g0223_get_pc34(row, col) <= 32767);
        }
    }
    CHECK(dm1_v1_g0223_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0223_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0223_get_pc34(3, 0) == -1);
    CHECK(dm1_v1_g0223_get_pc34(0, 8) == -1);
    CHECK(dm1_v1_g0223_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_g0223_get_pc34(0, 0) == 0);
    CHECK(dm1_v1_g0223_get_pc34(2, 0) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_G0223ResultPc34 r;
    int ok = dm1_v1_g0223_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 5);
    CHECK(r.tableSize == 24);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.allInInt16Range == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 24; ++i) {
        int row = i / 8;
        int col = i % 8;
        CHECK(r.tableEntries[i] == dm1_v1_g0223_get_pc34(row, col));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0223: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
