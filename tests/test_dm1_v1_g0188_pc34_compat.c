#include "firestaff/dm1/v1/G0188_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0188_table_pc34();
    int n = dm1_v1_g0188_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 96);
    /* D3C row (index 0): {0, 63, 0x8A, 0xFF, 0, 0, 0, 64}. */
    CHECK(dm1_v1_g0188_get_pc34(0, 0) == 0);
    CHECK(dm1_v1_g0188_get_pc34(0, 2) == 0x8A);
    CHECK(dm1_v1_g0188_get_pc34(0, 3) == 0xFF);
    /* D0L row (index 10): {0, 63, 0x0A, 0x83, 16, 136, 0, 64}. */
    CHECK(dm1_v1_g0188_get_pc34(10, 4) == 16);
    CHECK(dm1_v1_g0188_get_pc34(10, 5) == 136);
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
            CHECK(dm1_v1_g0188_get_pc34(row, col) >= 0);
            CHECK(dm1_v1_g0188_get_pc34(row, col) <= 255);
        }
    }
    CHECK(dm1_v1_g0188_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0188_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0188_get_pc34(12, 0) == -1);
    CHECK(dm1_v1_g0188_get_pc34(0, 8) == -1);
    CHECK(dm1_v1_g0188_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_g0188_get_pc34(0, 0) == 0);
    CHECK(dm1_v1_g0188_get_pc34(11, 7) == 64);  /* D0R.BitPlaneWordCount */
}

static void test_run_accepted(void)
{
    DM1_V1_G0188ResultPc34 r;
    int ok = dm1_v1_g0188_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 8);
    CHECK(r.tableSize == 96);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.rowD3CTransparentColor == 1);
    CHECK(r.rowD1LMaskByte == 1);
    CHECK(r.rowD0LValid == 1);
    CHECK(r.allRowsInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 96; ++i) {
        int row = i / 8;
        int col = i % 8;
        CHECK(r.tableEntries[i] == dm1_v1_g0188_get_pc34(row, col));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0188: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
