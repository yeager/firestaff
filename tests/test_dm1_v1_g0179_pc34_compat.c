#include "firestaff/dm1/v1/G0179_pc34_compat.h"

#include <stdio.h>

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
    const unsigned char *t = dm1_v1_g0179_table_pc34();
    int i;
    CHECK(t != 0);
    CHECK(dm1_v1_g0179_size_pc34() == 80);
    CHECK(dm1_v1_g0179_get_pc34(0, 0) == 24);
    CHECK(dm1_v1_g0179_get_pc34(0, 1) == 71);
    CHECK(dm1_v1_g0179_get_pc34(4, 6) == 18);
    CHECK(dm1_v1_g0179_get_pc34(8, 0) == 60);
    CHECK(dm1_v1_g0179_get_pc34(9, 7) == 0);
    for (i = 0; i < 80; ++i) {
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int frame;
    int value;
    for (frame = 0; frame < 10; ++frame) {
        for (value = 0; value < 8; ++value) {
            CHECK(dm1_v1_g0179_get_pc34(frame, value) >= 0);
            CHECK(dm1_v1_g0179_get_pc34(frame, value) <= 255);
        }
    }
    CHECK(dm1_v1_g0179_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0179_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0179_get_pc34(10, 0) == -1);
    CHECK(dm1_v1_g0179_get_pc34(0, 8) == -1);
    CHECK(dm1_v1_g0179_get_pc34(999, 999) == -1);
}

static void test_run_accepted(void)
{
    DM1_V1_G0179ResultPc34 r;
    int ok = dm1_v1_g0179_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.tableSize == 80);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.closedOrDestroyedValid == 1);
    CHECK(r.verticalFramesValid == 1);
    CHECK(r.leftHorizontalFramesValid == 1);
    CHECK(r.rightHorizontalFramesValid == 1);
    CHECK(r.allBytesInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 80; ++i) {
        int frame = i / 8;
        int value = i % 8;
        CHECK(r.tableEntries[i] == dm1_v1_g0179_get_pc34(frame, value));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_run_accepted();
    printf("dm1_v1_g0179: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
