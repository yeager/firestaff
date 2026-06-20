#include "firestaff/dm1/v1/G0208_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0208_table_pc34();
    int n = dm1_v1_g0208_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 24);
    /* Frame 0 (D3R): {199, 204, 41, 44, 8, 4} */
    CHECK(dm1_v1_g0208_get_pc34(0, 0) == 199);
    CHECK(dm1_v1_g0208_get_pc34(0, 5) == 4);
    /* Frame 3 (D1C): {160, 175, 44, 52, 8, 9} */
    CHECK(dm1_v1_g0208_get_pc34(3, 0) == 160);
    CHECK(dm1_v1_g0208_get_pc34(3, 5) == 9);
    for (i = 0; i < 24; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int frame, field;
    for (frame = 0; frame < 4; ++frame) {
        for (field = 0; field < 6; ++field) {
            CHECK(dm1_v1_g0208_get_pc34(frame, field) >= 0);
            CHECK(dm1_v1_g0208_get_pc34(frame, field) <= 255);
        }
    }
    CHECK(dm1_v1_g0208_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0208_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0208_get_pc34(4, 0) == -1);
    CHECK(dm1_v1_g0208_get_pc34(0, 6) == -1);
    CHECK(dm1_v1_g0208_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_g0208_get_pc34(0, 0) == 199);
    CHECK(dm1_v1_g0208_get_pc34(3, 0) == 160);
}

static void test_run_accepted(void)
{
    DM1_V1_G0208ResultPc34 r;
    int ok = dm1_v1_g0208_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 5);
    CHECK(r.tableSize == 24);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.allBytesInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 24; ++i) {
        int frame = i / 6;
        int field = i % 6;
        CHECK(r.tableEntries[i] == dm1_v1_g0208_get_pc34(frame, field));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0208: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
