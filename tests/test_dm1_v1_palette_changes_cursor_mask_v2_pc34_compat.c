#include "firestaff/dm1/v1/palette_changes_cursor_mask_v2_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_palette_changes_cursor_mask_v2_table_pc34();
    int n = dm1_v1_palette_changes_cursor_mask_v2_size_pc34();
    static const unsigned char kExpected[16] = {
        15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 15, 15, 15
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
        CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(i) >= 0);
        CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(i) <= 255);
    }
    CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(-1) == -1);
    CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(16) == -1);
    CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(0) == 15);
    CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(12) == 0);
    CHECK(dm1_v1_palette_changes_cursor_mask_v2_get_pc34(15) == 15);
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteChangesCursorMaskV2ResultPc34 r;
    int ok = dm1_v1_palette_changes_cursor_mask_v2_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.tableSize == 16);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.entry0Through11AllPalette15 == 1);
    CHECK(r.entry12Palette0 == 1);
    CHECK(r.entry13Through15Palette15 == 1);
    CHECK(r.allEntriesInByteRange == 1);
    CHECK(r.exactlyOneZeroEntry == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_palette_changes_cursor_mask_v2_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_palette_changes_cursor_mask_v2: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}