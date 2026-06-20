#include "firestaff/dm1/v1/G0220_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0220_table_pc34();
    int n = dm1_v1_g0220_size_pc34();
    int i;
    int kExpected[182] = { 12, 160, 10, 128, 8, 96, 6, 64, 4, 32, 2, 0, 9, 9, 0, 96, 0, 64, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 8, 96, 6, 64, 4, 32, 2, 0, 0, 0, 0, 0, 10, 10, 6, 64, 4, 32, 2, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 10, 0, 8, 0, 6, 0, 4, 0, 2, 0, 0, 9, 10, 0, 8, 0, 6, 0, 4, 0, 2, 0, 0, 0, 0, 10, 0, 8, 8, 6, 6, 4, 4, 2, 2, 0, 0, 0, 0, 9, 0, 10, 10, 8, 8, 6, 6, 4, 4, 2, 2, 0, 0, 10, 9, 15, 160, 12, 128, 10, 96, 8, 64, 6, 32, 4, 0, 10, 5, 15, 128, 12, 96, 10, 64, 8, 32, 6, 0, 2, 0, 5, 7, 8, 0, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0, 10, 12, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 12, 0, 12, 134, 10, 100, 8, 66, 6, 32, 4, 0, 2, 0, 10, 5 };
    CHECK(t != 0);
    CHECK(n == 182);
    /* Entry 0: 0x0CA0 = bytes 0,1 = 0x0C, 0xA0. D2=9, D3=9 (bytes 12, 13). */
    CHECK(dm1_v1_g0220_get_pc34(0, 0) == 0x0C);
    CHECK(dm1_v1_g0220_get_pc34(0, 1) == 0xA0);
    CHECK(dm1_v1_g0220_get_pc34(0, 12) == 9);
    CHECK(dm1_v1_g0220_get_pc34(0, 13) == 9);
    /* Entry 12 (last): 0x0C86, D2=10, D3=5. */
    CHECK(dm1_v1_g0220_get_pc34(12, 0) == 0x0C);
    CHECK(dm1_v1_g0220_get_pc34(12, 1) == 0x86);
    CHECK(dm1_v1_g0220_get_pc34(12, 12) == 10);
    CHECK(dm1_v1_g0220_get_pc34(12, 13) == 5);
    for (i = 0; i < 182; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int entry, off;
    for (entry = 0; entry < 13; ++entry) {
        for (off = 0; off < 14; ++off) {
            CHECK(dm1_v1_g0220_get_pc34(entry, off) >= 0);
            CHECK(dm1_v1_g0220_get_pc34(entry, off) <= 255);
        }
    }
    CHECK(dm1_v1_g0220_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0220_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0220_get_pc34(13, 0) == -1);
    CHECK(dm1_v1_g0220_get_pc34(0, 14) == -1);
    CHECK(dm1_v1_g0220_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    /* Entry 0 byte 0 = 0x0C. */
    CHECK(dm1_v1_g0220_get_pc34(0, 0) == 0x0C);
    /* Entry 12 byte 13 = 5. */
    CHECK(dm1_v1_g0220_get_pc34(12, 13) == 5);
}

static void test_run_accepted(void)
{
    DM1_V1_G0220ResultPc34 r;
    int ok = dm1_v1_g0220_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 5);
    CHECK(r.tableSize == 182);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.allBytesInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 182; ++i) {
        int entry = i / 14;
        int off = i % 14;
        CHECK(r.tableEntries[i] == dm1_v1_g0220_get_pc34(entry, off));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0220: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
