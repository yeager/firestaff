#include "firestaff/dm1/v1/G0494_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_graphic560_action_stamina_table_pc34();
    int n = dm1_v1_graphic560_action_stamina_size_pc34();
    CHECK(t != 0);
    CHECK(n == 44);
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(0) == 0);
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(1) == 4);
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(2) == 10);
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(43) == 2);
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 44; ++i) {
        CHECK(dm1_v1_graphic560_action_stamina_get_pc34(i) >= 0);
        CHECK(dm1_v1_graphic560_action_stamina_get_pc34(i) <= 255);
    }
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(-1) == -1);
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(44) == -1);
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(0) == 0);
    CHECK(dm1_v1_graphic560_action_stamina_get_pc34(43) == 2);
}

static void test_run_accepted(void)
{
    DM1_V1_G0494ResultPc34 r;
    int ok = dm1_v1_graphic560_action_stamina_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 5);
    CHECK(r.tableSize == 44);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.allInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 44; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_graphic560_action_stamina_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_graphic560_action_stamina: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
