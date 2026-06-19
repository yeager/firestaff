#include "firestaff/dm1/v1/palette_changes_mouse_pointer_icon_pc34_compat.h"

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
    const unsigned char *t = dm1_v1_palette_changes_mouse_pointer_icon_table_pc34();
    int n = dm1_v1_palette_changes_mouse_pointer_icon_size_pc34();
    int i;
    static const unsigned char kExpected[16] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 12, 4, 4, 4 };
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
        CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(i) >= 0);
        CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(i) <= 255);
    }
    CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(-1) == 0);
    CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(16) == 0);
    CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(999) == 0);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(0) == 4);
    CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(15) == 4);
}

static void test_entry_12_specific(void)
{
    /* Entry 12 is the special spotlight pixel index. */
    CHECK(dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(12) == 12);
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteChangesChangesMousePointerIconResultPc34 r;
    int ok = dm1_v1_palette_changes_mouse_pointer_icon_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 8);
    CHECK(r.tableSize == 16);
    CHECK(r.tableEntries[0] == 4);
    CHECK(r.tableEntries[15] == 4);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntry4 == 1);
    CHECK(r.lastEntry4 == 1);
    CHECK(r.allValuesInRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    CHECK(r.entry12Special == 1);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_entry_12_specific();
    test_run_accepted();
    printf("dm1_v1_palette_changes_mouse_pointer_icon: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
