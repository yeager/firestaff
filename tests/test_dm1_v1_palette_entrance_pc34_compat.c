#include "firestaff/dm1/v1/palette_entrance_pc34_compat.h"

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
    /* DATA.C:213 G0020 init (PC 3.4):
     *   { 0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0,
     *     0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF }
     */
    const unsigned int *t = dm1_v1_palette_entrance_table_pc34();
    int n = dm1_v1_palette_entrance_size_pc34();
    int i;
    static const unsigned int kExpected[16] = {
        0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0,
        0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF
    };
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
        CHECK(dm1_v1_palette_entrance_pc34(i) >= 0);
        CHECK(dm1_v1_palette_entrance_pc34(i) <= 0xFFF);
    }
    CHECK(dm1_v1_palette_entrance_pc34(-1) == 0);
    CHECK(dm1_v1_palette_entrance_pc34(16) == 0);
    CHECK(dm1_v1_palette_entrance_pc34(999) == 0);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_palette_entrance_pc34(0) == 0x000);
    CHECK(dm1_v1_palette_entrance_pc34(15) == 0xFFF);
}

static void test_black_white_entries(void)
{
    /* Entry 0 = black, Entry 15 = white. */
    CHECK(dm1_v1_palette_entrance_pc34(0) == 0x000);
    CHECK(dm1_v1_palette_entrance_pc34(15) == 0xFFF);
}

static void test_12bit_range(void)
{
    int i;
    const unsigned int *t = dm1_v1_palette_entrance_table_pc34();
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] <= 0xFFF);
    }
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteEntranceResultPc34 r;
    int ok = dm1_v1_palette_entrance_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.tableSize == 16);
    CHECK(r.tableEntries[0] == 0x000);
    CHECK(r.tableEntries[15] == 0xFFF);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntry0x000 == 1);
    CHECK(r.lastEntry0xFFF == 1);
    CHECK(r.allValues12Bit == 1);
    CHECK(r.entry0IsBlack == 1);
    CHECK(r.entry15IsWhite == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_palette_entrance_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_black_white_entries();
    test_12bit_range();
    test_run_accepted();
    printf("dm1_v1_palette_entrance: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}