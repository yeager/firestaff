#include "firestaff/dm1/v1/G0489_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0489_table_pc34();
    int n = dm1_v1_g0489_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 264);
    /* Entry 0 ("N"): {255, 255, 255, 0, 0, 0}. */
    CHECK(dm1_v1_g0489_get_pc34(0, 0) == 255);
    CHECK(dm1_v1_g0489_get_pc34(0, 1) == 255);
    CHECK(dm1_v1_g0489_get_pc34(0, 2) == 255);
    CHECK(dm1_v1_g0489_get_pc34(0, 3) == 0);
    /* Entry 1 (BLOCK): {27, 43, 35, 0, 0, 3}. */
    CHECK(dm1_v1_g0489_get_pc34(1, 0) == 27);
    CHECK(dm1_v1_g0489_get_pc34(1, 1) == 43);
    CHECK(dm1_v1_g0489_get_pc34(1, 2) == 35);
    /* Last entry (FUSE): {6, 11, 255, 0|0x80, 0, 3}. */
    CHECK(dm1_v1_g0489_get_pc34(43, 0) == 6);
    CHECK(dm1_v1_g0489_get_pc34(43, 2) == 255);
    /* All values fit in uint8_t. */
    for (i = 0; i < 264; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int action, off;
    for (action = 0; action < 44; ++action) {
        for (off = 0; off < 6; ++off) {
            CHECK(dm1_v1_g0489_get_pc34(action, off) >= 0);
            CHECK(dm1_v1_g0489_get_pc34(action, off) <= 255);
        }
    }
    CHECK(dm1_v1_g0489_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0489_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0489_get_pc34(44, 0) == -1);
    CHECK(dm1_v1_g0489_get_pc34(0, 6) == -1);
    CHECK(dm1_v1_g0489_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    /* Entry 0 ActionIndices[0] = 255 (N means "no action"). */
    CHECK(dm1_v1_g0489_get_pc34(0, 0) == 255);
    /* Entry 43 ActionIndices[2] = 255 (FUSE has only 2 actions). */
    CHECK(dm1_v1_g0489_get_pc34(43, 2) == 255);
}

static void test_run_accepted(void)
{
    DM1_V1_G0489ResultPc34 r;
    int ok = dm1_v1_g0489_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 6);
    CHECK(r.tableSize == 264);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.entry0AllZeroValid == 1);
    CHECK(r.allBytesInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 264; ++i) {
        int action = i / 6;
        int off = i % 6;
        CHECK(r.tableEntries[i] == dm1_v1_g0489_get_pc34(action, off));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0489: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
