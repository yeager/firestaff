#include "firestaff/dm1/v1/palette_changes_no_changes_pc34_compat.h"

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
    /* DATA.C:136 G0017 init (PC 3.4): { 0, 1, 2, ..., 15 }. */
    const unsigned char *t = dm1_v1_palette_changes_no_changes_table_pc34();
    int n = dm1_v1_palette_changes_no_changes_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 16);
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] == (unsigned char)i);
    }
}

static void test_lookup_function(void)
{
    /* The lookup function returns each table entry as int, 0 for OOB. */
    int i;
    for (i = 0; i < 16; ++i) {
        CHECK(dm1_v1_palette_changes_no_changes_pc34(i) == i);
    }
    CHECK(dm1_v1_palette_changes_no_changes_pc34(-1) == 0);
    CHECK(dm1_v1_palette_changes_no_changes_pc34(16) == 0);
    CHECK(dm1_v1_palette_changes_no_changes_pc34(999) == 0);
}

static void test_identity_helper(void)
{
    /* PC 3.4 init is the identity permutation {0, 1, ..., 15}. */
    CHECK(dm1_v1_palette_changes_no_changes_is_identity_pc34() == 1);
}

static void test_range_and_order(void)
{
    /* All values in [0, 255]; strictly increasing. */
    const unsigned char *t = dm1_v1_palette_changes_no_changes_table_pc34();
    int i;
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] <= 255);
    }
    for (i = 0; i < 15; ++i) {
        CHECK(t[i] < t[i + 1]);
    }
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteChangesNoChangesResultPc34 r;
    int ok = dm1_v1_palette_changes_no_changes_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 10);
    CHECK(r.tableSize == 16);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] == i);
    }
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntry0 == 1);
    CHECK(r.lastEntry15 == 1);
    CHECK(r.allValuesInRange0to255 == 1);
    CHECK(r.isIdentityPermutation == 1);
    CHECK(r.monotonicStrictlyIncreasing == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    CHECK(r.isSameAsIdentityHelper == 1);
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_identity_helper();
    test_range_and_order();
    test_run_accepted();
    printf("dm1_v1_palette_changes_no_changes: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}