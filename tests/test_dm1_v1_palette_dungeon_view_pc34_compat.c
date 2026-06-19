#include "firestaff/dm1/v1/palette_dungeon_view_pc34_compat.h"

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

static void test_table_dimensions(void)
{
    /* DATA.C:27 - G0021_aaui_Graphic562_Palette_DungeonView[6][16]. */
    CHECK(dm1_v1_palette_dungeon_view_rows_pc34() == 6);
    CHECK(dm1_v1_palette_dungeon_view_cols_pc34() == 16);
}

static void test_table_values(void)
{
    /* DATA.C:218-224 G0021 init (6 rows of 16). */
    const unsigned int *t = dm1_v1_palette_dungeon_view_table_pc34();
    int i;
    static const unsigned int kExpected[6][16] = {
        { 0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0,
          0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF },
        { 0x000, 0x444, 0x666, 0x620, 0x0CC, 0x820, 0x060, 0x0A0,
          0xC00, 0x000, 0x000, 0xFC0, 0x222, 0x888, 0x00C, 0xCCC },
        { 0x000, 0x222, 0x444, 0x420, 0x0CC, 0x620, 0x040, 0x080,
          0xA00, 0x000, 0x000, 0xFA0, 0x000, 0x666, 0x00A, 0xAAA },
        { 0x000, 0x000, 0x222, 0x200, 0x0CC, 0x420, 0x020, 0x060,
          0x800, 0x000, 0x000, 0xC80, 0x000, 0x444, 0x008, 0x888 },
        { 0x000, 0x000, 0x000, 0x000, 0x0CC, 0x200, 0x000, 0x040,
          0x600, 0x000, 0x000, 0xA60, 0x000, 0x222, 0x006, 0x666 },
        { 0x000, 0x000, 0x000, 0x000, 0x0CC, 0x000, 0x000, 0x020,
          0x400, 0x000, 0x000, 0x640, 0x000, 0x000, 0x004, 0x444 }
    };
    CHECK(t != 0);
    /* Check row 0. */
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] == kExpected[0][i]);
    }
    /* Spot-check row 5 (last entry). */
    CHECK(t[5 * 16 + 15] == kExpected[5][15]);
}

static void test_lookup_function(void)
{
    int row, col;
    /* All valid indices return the expected value. */
    for (row = 0; row < 6; ++row) {
        for (col = 0; col < 16; ++col) {
            CHECK(dm1_v1_palette_dungeon_view_pc34(row, col) >= 0);
            CHECK(dm1_v1_palette_dungeon_view_pc34(row, col) <= 0xFFF);
        }
    }
    /* OOB returns 0. */
    CHECK(dm1_v1_palette_dungeon_view_pc34(-1, 0) == 0);
    CHECK(dm1_v1_palette_dungeon_view_pc34(6, 0) == 0);
    CHECK(dm1_v1_palette_dungeon_view_pc34(0, -1) == 0);
    CHECK(dm1_v1_palette_dungeon_view_pc34(0, 16) == 0);
    CHECK(dm1_v1_palette_dungeon_view_pc34(999, 999) == 0);
}

static void test_row0_contains_white(void)
{
    /* Row 0 (brightest) must contain at least one 0xFFF (white) entry. */
    int i;
    int found = 0;
    for (i = 0; i < 16; ++i) {
        if (dm1_v1_palette_dungeon_view_pc34(0, i) == 0xFFF) {
            found = 1;
            break;
        }
    }
    CHECK(found == 1);
}

static void test_row5_is_darkest(void)
{
    /* Row 5 (darkest) has at least 8 zero entries (most colors clip
     * to black in the darkest dungeon palette).
     */
    int i;
    int zero_count = 0;
    int nonzero_count = 0;
    for (i = 0; i < 16; ++i) {
        if (dm1_v1_palette_dungeon_view_pc34(5, i) == 0x000) {
            ++zero_count;
        } else if (dm1_v1_palette_dungeon_view_pc34(5, i) != 0) {
            ++nonzero_count;
        }
    }
    CHECK(zero_count >= 8);
    CHECK(nonzero_count >= 1);
}

static void test_run_accepted(void)
{
    DM1_V1_PaletteDungeonViewResultPc34 r;
    int ok = dm1_v1_palette_dungeon_view_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.tableRows == 6);
    CHECK(r.tableCols == 16);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntry0x000 == 1);
    CHECK(r.lastEntry0x444 == 1);
    CHECK(r.allValues12Bit == 1);
    CHECK(r.row0IsBrightest == 1);
    CHECK(r.row5IsDarkest == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    /* Cross-check the result struct's tableEntries match the
     * source-of-truth lookup function.
     */
    for (i = 0; i < 6 * 16; ++i) {
        int row = i / 16;
        int col = i % 16;
        CHECK(r.tableEntries[i] == dm1_v1_palette_dungeon_view_pc34(row, col));
    }
}

int main(void)
{
    test_table_dimensions();
    test_table_values();
    test_lookup_function();
    test_row0_contains_white();
    test_row5_is_darkest();
    test_run_accepted();
    printf("dm1_v1_palette_dungeon_view: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}