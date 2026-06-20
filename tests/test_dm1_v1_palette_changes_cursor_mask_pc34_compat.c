#include "firestaff/dm1/v1/palette_changes_cursor_mask_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_palette_changes_cursor_mask_table_pc34();
    int n = dm1_v1_palette_changes_cursor_mask_size_pc34();
    static const unsigned char kExpected[16] = {
        15, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int i;
    CHECK(t != 0);
    CHECK(n == 16);
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] == kExpected[i]);
    }
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 16; ++i) {
        CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(i) >= 0);
        CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(i) <= 255);
    }
    CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(-1) == -1);
    CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(16) == -1);
    CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(0) == 15);
    CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(4) == 15);
    CHECK(dm1_v1_palette_changes_cursor_mask_get_pc34(15) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteChangesCursorMaskResultPc34 r;
    int ok = dm1_v1_palette_changes_cursor_mask_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 16);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.entry0Palette15 == 1);
    CHECK(r.entry1Zero == 1);
    CHECK(r.entry2Zero == 1);
    CHECK(r.entry3Zero == 1);
    CHECK(r.entry4Palette15 == 1);
    CHECK(r.entry5Through15Zero == 1);
    CHECK(r.allEntriesInByteRange == 1);
    CHECK(r.exactlyTwoNonZeroEntries == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_palette_changes_cursor_mask_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_palette_changes_cursor_mask: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}