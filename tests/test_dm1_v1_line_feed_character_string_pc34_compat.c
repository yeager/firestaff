#include "firestaff/dm1/v1/line_feed_character_string_pc34_compat.h"

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
    const char *t = dm1_v1_line_feed_character_string_table_pc34();
    int n = dm1_v1_line_feed_character_string_size_pc34();
    CHECK(t != 0);
    CHECK(n == 2);
    CHECK(t[0] == '\n');
    CHECK(t[1] == '\0');
}

static void test_lookup_function(void)
{
    CHECK(dm1_v1_line_feed_character_string_get_pc34(0) == '\n');
    CHECK(dm1_v1_line_feed_character_string_get_pc34(1) == 0);
    CHECK(dm1_v1_line_feed_character_string_get_pc34(-1) == 0);
    CHECK(dm1_v1_line_feed_character_string_get_pc34(2) == 0);
    CHECK(dm1_v1_line_feed_character_string_get_pc34(999) == 0);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_line_feed_character_string_get_pc34(0) == '\n');
    CHECK(dm1_v1_line_feed_character_string_get_pc34(1) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_LINE_FEED_CHARACTER_STRINGResultPc34 r;
    int ok = dm1_v1_line_feed_character_string_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 7);
    CHECK(r.tableSize == 2);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstCharNewline == 1);
    CHECK(r.lastCharNulTerminator == 1);
    CHECK(r.nulTerminated == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    for (i = 0; i < 2; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_line_feed_character_string_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_line_feed_character_string: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
