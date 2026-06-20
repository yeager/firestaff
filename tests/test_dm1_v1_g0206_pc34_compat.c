#include "firestaff/dm1/v1/G0206_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_g0206_table_pc34();
    int n = dm1_v1_g0206_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 162);
    for (i = 0; i < 162; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int set, frame, field;
    for (set = 0; set < 3; ++set) {
        for (frame = 0; frame < 9; ++frame) {
            for (field = 0; field < 6; ++field) {
                CHECK(dm1_v1_g0206_get_pc34(set, frame, field) >= 0);
                CHECK(dm1_v1_g0206_get_pc34(set, frame, field) <= 255);
            }
        }
    }
    CHECK(dm1_v1_g0206_get_pc34(-1, 0, 0) == -1);
    CHECK(dm1_v1_g0206_get_pc34(0, -1, 0) == -1);
    CHECK(dm1_v1_g0206_get_pc34(0, 0, -1) == -1);
    CHECK(dm1_v1_g0206_get_pc34(3, 0, 0) == -1);
    CHECK(dm1_v1_g0206_get_pc34(0, 9, 0) == -1);
    CHECK(dm1_v1_g0206_get_pc34(0, 0, 6) == -1);
    CHECK(dm1_v1_g0206_get_pc34(999, 999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_g0206_get_pc34(0, 0, 0) == 32);
    CHECK(dm1_v1_g0206_get_pc34(2, 8, 5) == 12);
}

static void test_run_accepted(void)
{
    DM1_V1_G0206ResultPc34 r;
    int ok = dm1_v1_g0206_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 5);
    CHECK(r.tableSize == 162);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.allBytesInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 162; ++i) {
        int set_idx = i / 54;
        int frame_idx = (i / 6) % 9;
        int field_idx = i % 6;
        CHECK(r.tableEntries[i] == dm1_v1_g0206_get_pc34(set_idx, frame_idx, field_idx));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0206: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
