#include "firestaff/dm1/v1/G0163_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0163_table_pc34();
    int n = dm1_v1_g0163_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 96);
    /* Row 0 (D0C): {0, 223, 0, 135, 0, 0, 0, 0}. */
    CHECK(dm1_v1_g0163_get_pc34(0, 0) == 0);
    CHECK(dm1_v1_g0163_get_pc34(0, 1) == 223);
    CHECK(dm1_v1_g0163_get_pc34(0, 3) == 135);
    /* Row 9 (D3C): {74, 149, 25, 75, 64, 51, 18, 0}. */
    CHECK(dm1_v1_g0163_get_pc34(9, 0) == 74);
    CHECK(dm1_v1_g0163_get_pc34(9, 7) == 0);
    /* Spot-check last row (D3R): {139, 223, 25, 75, 64, 51, 0, 0}. */
    CHECK(dm1_v1_g0163_get_pc34(11, 0) == 139);
    /* All values fit in uint8_t. */
    for (i = 0; i < 96; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int row, col;
    for (row = 0; row < 12; ++row) {
        for (col = 0; col < 8; ++col) {
            CHECK(dm1_v1_g0163_get_pc34(row, col) >= 0);
            CHECK(dm1_v1_g0163_get_pc34(row, col) <= 255);
        }
    }
    CHECK(dm1_v1_g0163_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0163_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0163_get_pc34(12, 0) == -1);
    CHECK(dm1_v1_g0163_get_pc34(0, 8) == -1);
    CHECK(dm1_v1_g0163_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_g0163_get_pc34(0, 0) == 0);    /* D0C.X1 */
    CHECK(dm1_v1_g0163_get_pc34(11, 1) == 223); /* D3R.X2 */
}

static void test_run_accepted(void)
{
    DM1_V1_G0163ResultPc34 r;
    int ok = dm1_v1_g0163_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 8);
    CHECK(r.tableSize == 96);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.rowD0CD3CValid == 1);
    CHECK(r.rowD1CD2CValid == 1);
    CHECK(r.rowD3CD0CValid == 1);
    CHECK(r.allRowsInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 96; ++i) {
        int row = i / 8;
        int col = i % 8;
        CHECK(r.tableEntries[i] == dm1_v1_g0163_get_pc34(row, col));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0163: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
