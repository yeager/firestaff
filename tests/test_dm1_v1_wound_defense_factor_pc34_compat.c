#include "firestaff/dm1/v1/wound_defense_factor_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_wound_defense_factor_table_pc34();
    int n = dm1_v1_wound_defense_factor_size_pc34();
    static const unsigned char kExpected[6] = { 5, 5, 4, 6, 3, 1 };
    int i;
    CHECK(t != 0);
    CHECK(n == 6);
    for (i = 0; i < 6; ++i) {
        CHECK(t[i] == kExpected[i]);
    }
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 6; ++i) {
        CHECK(dm1_v1_wound_defense_factor_get_pc34(i) > 0);
        CHECK(dm1_v1_wound_defense_factor_get_pc34(i) <= 255);
    }
    CHECK(dm1_v1_wound_defense_factor_get_pc34(-1) == 0);
    CHECK(dm1_v1_wound_defense_factor_get_pc34(6) == 0);
    CHECK(dm1_v1_wound_defense_factor_get_pc34(999) == 0);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_wound_defense_factor_get_pc34(0) == 5);
    CHECK(dm1_v1_wound_defense_factor_get_pc34(5) == 1);
}

static void test_run_accepted(void)
{
    DM1_V1_WoundDefenseFactorResultPc34 r;
    int ok = dm1_v1_wound_defense_factor_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 6);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.headFactor5 == 1);
    CHECK(r.torsoFactor5 == 1);
    CHECK(r.legsFactor4 == 1);
    CHECK(r.armsFactor6 == 1);
    CHECK(r.feetFactor3 == 1);
    CHECK(r.handFactor1 == 1);
    CHECK(r.allFactorsPositive == 1);
    CHECK(r.allFactorsInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    for (i = 0; i < 6; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_wound_defense_factor_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_wound_defense_factor: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}