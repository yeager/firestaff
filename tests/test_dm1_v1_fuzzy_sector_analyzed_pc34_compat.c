#include "firestaff/dm1/v1/fuzzy_sector_analyzed_pc34_compat.h"

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

static void test_initial_value(void)
{
    /* DATA.C:179 G0031 init = C00255_FALSE = 0. */
    int v = dm1_v1_fuzzy_sector_analyzed_get_pc34();
    CHECK(v == 0);
    CHECK(dm1_v1_fuzzy_sector_analyzed_size_pc34() == 1);
}

static void test_value_in_int16_range(void)
{
    int v = dm1_v1_fuzzy_sector_analyzed_get_pc34();
    CHECK(v >= -32768);
    CHECK(v <= 32767);
}

static void test_run_accepted(void)
{
    DM1_V1_FuzzySectorAnalyzedResultPc34 r;
    int ok = dm1_v1_fuzzy_sector_analyzed_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 6);
    CHECK(r.tableSize == 1);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.initializedFalse == 1);
    CHECK(r.valueIsC00255 == 1);
    CHECK(r.valueInRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
}

int main(void)
{
    test_initial_value();
    test_value_in_int16_range();
    test_run_accepted();
    printf("dm1_v1_fuzzy_sector_analyzed: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}