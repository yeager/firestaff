#include "firestaff/dm1/v1/print_text_masks2_pc34_compat.h"

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
    const unsigned int *t = dm1_v1_print_text_masks2_table_pc34();
    int n = dm1_v1_print_text_masks2_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 0xFFF0FFF0);
    CHECK(t[3] == 0xFFFEFFFE);
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 4; ++i) {
        CHECK(dm1_v1_print_text_masks2_get_pc34(i) != 0u);
    }
    CHECK(dm1_v1_print_text_masks2_get_pc34(-1) == 0u);
    CHECK(dm1_v1_print_text_masks2_get_pc34(4) == 0u);
    CHECK(dm1_v1_print_text_masks2_get_pc34(999) == 0u);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_print_text_masks2_get_pc34(0) == 0xFFF0FFF0u);
    CHECK(dm1_v1_print_text_masks2_get_pc34(3) == 0xFFFEFFFEu);
}

static void test_run_accepted(void)
{
    DM1_V1_PRINT_TEXT_MASKS2ResultPc34 r;
    int ok = dm1_v1_print_text_masks2_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 4);
    CHECK(r.tableSize == 4);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    for (i = 0; i < 4; ++i) {
        CHECK((unsigned)r.tableEntries[i] == (unsigned)dm1_v1_print_text_masks2_table_pc34()[i]);
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_print_text_masks2: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
