#include "firestaff/dm1/v1/palette_top_and_bottom_screen_pc34_compat.h"

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
    const unsigned int *t = dm1_v1_palette_top_and_bottom_screen_table_pc34();
    int n = dm1_v1_palette_top_and_bottom_screen_size_pc34();
    static const unsigned int kExpected[16] = {
        0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0,
        0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF
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
        CHECK(dm1_v1_palette_top_and_bottom_screen_get_pc34(i) >= 0);
        CHECK(dm1_v1_palette_top_and_bottom_screen_get_pc34(i) <= 0xFFFF);
    }
    CHECK(dm1_v1_palette_top_and_bottom_screen_get_pc34(-1) == 0);
    CHECK(dm1_v1_palette_top_and_bottom_screen_get_pc34(16) == 0);
    CHECK(dm1_v1_palette_top_and_bottom_screen_get_pc34(999) == 0);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_palette_top_and_bottom_screen_get_pc34(0) == 0x000);
    CHECK(dm1_v1_palette_top_and_bottom_screen_get_pc34(15) == 0xFFF);
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteTopAndBottomScreenResultPc34 r;
    int ok = dm1_v1_palette_top_and_bottom_screen_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 8);
    CHECK(r.tableSize == 16);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.entry0Black == 1);
    CHECK(r.entry1DarkGray == 1);
    CHECK(r.entry15BrightWhite == 1);
    CHECK(r.allEntriesInByteRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] == (int)dm1_v1_palette_top_and_bottom_screen_table_pc34()[i]);
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_palette_top_and_bottom_screen: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}