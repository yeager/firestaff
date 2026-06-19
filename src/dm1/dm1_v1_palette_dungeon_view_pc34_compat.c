#include "firestaff/dm1/v1/palette_dungeon_view_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:27  - declaration of G0021_aaui_Graphic562_Palette_DungeonView[6][16]
 * - DATA.C:218 - PC 3.4 init (6 rows of 16)
 * - DATA.C:833 - Atari ST init (different values)
 * - BASE.C:968 - address-of for F0007_MAIN_CopyBytes
 * - DRAWVIEW.C:744/776/880/1040 - F0007_MAIN_CopyBytes(G0021[Idx], ...)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820
 * (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8). This gate
 * is a non-mirror-candidate contract for the G0021 dungeon-view
 * palette.
 */

enum {
    kRows        = DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_ROWS,
    kCols        = DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_COLS,
    kTableSize   = DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_SIZE,
    kMax12Bit    = 0xFFF,
    kOutOfRange  = 0
};

/* G0021 PC 3.4 init (DATA.C:218-224). 6 rows of 16. */
static const unsigned int s_g0021[kRows][kCols] = {
    /* row 0 (brightest, full color) */
    { 0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0,
      0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF },
    /* row 1 */
    { 0x000, 0x444, 0x666, 0x620, 0x0CC, 0x820, 0x060, 0x0A0,
      0xC00, 0x000, 0x000, 0xFC0, 0x222, 0x888, 0x00C, 0xCCC },
    /* row 2 */
    { 0x000, 0x222, 0x444, 0x420, 0x0CC, 0x620, 0x040, 0x080,
      0xA00, 0x000, 0x000, 0xFA0, 0x000, 0x666, 0x00A, 0xAAA },
    /* row 3 */
    { 0x000, 0x000, 0x222, 0x200, 0x0CC, 0x420, 0x020, 0x060,
      0x800, 0x000, 0x000, 0xC80, 0x000, 0x444, 0x008, 0x888 },
    /* row 4 */
    { 0x000, 0x000, 0x000, 0x000, 0x0CC, 0x200, 0x000, 0x040,
      0x600, 0x000, 0x000, 0xA60, 0x000, 0x222, 0x006, 0x666 },
    /* row 5 (darkest, almost black) */
    { 0x000, 0x000, 0x000, 0x000, 0x0CC, 0x000, 0x000, 0x020,
      0x400, 0x000, 0x000, 0x640, 0x000, 0x000, 0x004, 0x444 }
};

const unsigned int *
dm1_v1_palette_dungeon_view_table_pc34(void)
{
    return &s_g0021[0][0];
}

int
dm1_v1_palette_dungeon_view_rows_pc34(void)
{
    return kRows;
}

int
dm1_v1_palette_dungeon_view_cols_pc34(void)
{
    return kCols;
}

int
dm1_v1_palette_dungeon_view_pc34(int row, int col)
{
    if (row < 0 || row >= kRows || col < 0 || col >= kCols) {
        return kOutOfRange;
    }
    return (int)s_g0021[row][col];
}

int
dm1_v1_palette_dungeon_view_run_pc34(
    DM1_V1_PaletteDungeonViewResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int first_entry_0x000 = 1;
    int last_entry_0x444 = 1;
    int all_values_12_bit = 1;
    int row_0_is_brightest = 1;
    int row_5_is_darkest = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    static const unsigned int kExpected[kRows][kCols] = {
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

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values (96 ints) + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        int row = i / kCols;
        int col = i % kCols;
        out->tableEntries[i] = (int)s_g0021[row][col];
        if (s_g0021[row][col] != kExpected[row][col]) {
            table_matches_declaration = 0;
        }
    }
    out->tableRows = kRows;
    out->tableCols = kCols;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: first entry is 0x000 (always black). */
    if (s_g0021[0][0] != 0x000) first_entry_0x000 = 0;
    out->firstEntry0x000 = first_entry_0x000;

    /* Phase 3: last entry of row 5 is 0x444 (a near-black dim red). */
    if (s_g0021[kRows - 1][kCols - 1] != 0x444) last_entry_0x444 = 0;
    out->lastEntry0x444 = last_entry_0x444;

    /* Phase 4: all values are 12-bit RGB. */
    for (i = 0; i < kTableSize; ++i) {
        int row = i / kCols;
        int col = i % kCols;
        if ((int)s_g0021[row][col] < 0 || (int)s_g0021[row][col] > kMax12Bit) {
            all_values_12_bit = 0;
        }
    }
    out->allValues12Bit = all_values_12_bit;

    /* Phase 5: row 0 contains the brightest entry (0xFFF = white). */
    {
        int found_white = 0;
        for (i = 0; i < kCols; ++i) {
            if (s_g0021[0][i] == 0xFFF) {
                found_white = 1;
                break;
            }
        }
        if (!found_white) row_0_is_brightest = 0;
    }
    out->row0IsBrightest = row_0_is_brightest;

    /* Phase 6: row 5 has at least 8 zero entries (most colors clip
     * to black in the darkest palette). Row 5 also has at least one
     * non-zero entry (the torch 0x0CC and the visible-but-dim red
     * 0x640 — these are the colors that stay visible even at the
     * darkest dungeon light level).
     */
    {
        int zero_count = 0;
        int nonzero_count = 0;
        for (i = 0; i < kCols; ++i) {
            if (s_g0021[kRows - 1][i] == 0x000) {
                ++zero_count;
            } else if (s_g0021[kRows - 1][i] != 0) {
                ++nonzero_count;
            }
        }
        if (zero_count < 8) row_5_is_darkest = 0;
        if (nonzero_count < 1) row_5_is_darkest = 0;
    }
    out->row5IsDarkest = row_5_is_darkest;

    /* Phase 7: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        int row = i / kCols;
        int col = i % kCols;
        if (dm1_v1_palette_dungeon_view_pc34(row, col) !=
            (int)kExpected[row][col]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 8: out-of-range lookup returns 0. */
    if (dm1_v1_palette_dungeon_view_pc34(-1, 0) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_dungeon_view_pc34(6, 0) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_dungeon_view_pc34(0, -1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_dungeon_view_pc34(0, 16) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntry0x000 &&
        out->lastEntry0x444 &&
        out->allValues12Bit &&
        out->row0IsBrightest &&
        out->row5IsDarkest &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 9;
    return out->accepted;
}