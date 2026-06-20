#include "firestaff/dm1/v1/G0490_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0490_table_pc34();
    int n = dm1_v1_g0490_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 300);
    /* Byte 0 = 'N', byte 1 = NUL, byte 2-6 = 'BLOCK'. */
    CHECK(dm1_v1_g0490_get_pc34(0) == 'N');
    CHECK(dm1_v1_g0490_get_pc34(1) == 0);
    CHECK(dm1_v1_g0490_get_pc34(2) == 'B');
    CHECK(dm1_v1_g0490_get_pc34(3) == 'L');
    /* "FUSE" ends at byte 284-287, byte 288 = NUL. */
    CHECK(dm1_v1_g0490_get_pc34(284) == 'F');
    CHECK(dm1_v1_g0490_get_pc34(285) == 'U');
    CHECK(dm1_v1_g0490_get_pc34(286) == 'S');
    CHECK(dm1_v1_g0490_get_pc34(287) == 'E');
    CHECK(dm1_v1_g0490_get_pc34(288) == 0);
    /* Trailing zeros (288-299). */
    for (i = 288; i < 300; ++i) {
        CHECK(dm1_v1_g0490_get_pc34(i) == 0);
    }
    /* All values fit in uint8_t. */
    for (i = 0; i < 300; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 300; ++i) {
        CHECK(dm1_v1_g0490_get_pc34(i) >= 0);
        CHECK(dm1_v1_g0490_get_pc34(i) <= 255);
    }
    CHECK(dm1_v1_g0490_get_pc34(-1) == -1);
    CHECK(dm1_v1_g0490_get_pc34(300) == -1);
    CHECK(dm1_v1_g0490_get_pc34(999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_g0490_get_pc34(0) == 'N');
    CHECK(dm1_v1_g0490_get_pc34(284) == 'F'); /* 'FUSE' start */
    CHECK(dm1_v1_g0490_get_pc34(299) == 0);   /* trailing zero */
}

static void test_run_accepted(void)
{
    DM1_V1_G0490ResultPc34 r;
    int ok = dm1_v1_g0490_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 8);
    CHECK(r.tableSize == 300);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.entry0IsN == 1);
    CHECK(r.entry1IsBLOCK == 1);
    CHECK(r.entry43IsFUSE == 1);
    CHECK(r.exactly44Names == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 300; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_g0490_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0490: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
