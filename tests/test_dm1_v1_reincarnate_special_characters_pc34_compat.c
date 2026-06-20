#include "firestaff/dm1/v1/reincarnate_special_characters_pc34_compat.h"

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
    const char *t = dm1_v1_reincarnate_special_characters_table_pc34();
    int n = dm1_v1_reincarnate_special_characters_size_pc34();
    static const char kExpected[6] = { ',', '.', ';', ':', ' ', '\0' };
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
        CHECK(dm1_v1_reincarnate_special_characters_get_pc34(i) >= 0);
    }
    CHECK(dm1_v1_reincarnate_special_characters_get_pc34(-1) == 0);
    CHECK(dm1_v1_reincarnate_special_characters_get_pc34(6) == 0);
    CHECK(dm1_v1_reincarnate_special_characters_get_pc34(999) == 0);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_reincarnate_special_characters_get_pc34(0) == ',');
    CHECK(dm1_v1_reincarnate_special_characters_get_pc34(5) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_ReincarnateSpecialCharactersResultPc34 r;
    int ok = dm1_v1_reincarnate_special_characters_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableSize == 6);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstCharComma == 1);
    CHECK(r.secondCharPeriod == 1);
    CHECK(r.thirdCharSemicolon == 1);
    CHECK(r.fourthCharColon == 1);
    CHECK(r.fifthCharSpace == 1);
    CHECK(r.sixthCharNulTerminator == 1);
    CHECK(r.nulTerminated == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    for (i = 0; i < 6; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_reincarnate_special_characters_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_reincarnate_special_characters: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}