#include "firestaff/dm1/v1/palette_top_and_bottom_screen_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0347_aui_Palette_TopAndBottomScreen):
 * - DATA.C:216/170/759/866/790 - declaration + PC 3.4 EN init + BASE.C palette register load + DIALOG.C fade + DRAWVIEW.C viewport { 0x000, 0x666, 0x888, 0x620,
 *                                            0x0CC, 0x840, 0x080, 0x0C0,
 *                                            0xF00, 0xFA0, 0xC86, 0xFF0,
 *                                            0x444, 0xAAA, 0x00F, 0xFFF }
 * - BASE.C palette register load — G0347 load into D0-D7 (Atari ST) (Atari ST palette register load)
 * - DIALOG.C F0436_STARTEND_FadeToPalette(G0347)(G0347_...)
 * - DRAWVIEW.C F0565_VIEWPORT_SetPalette(...G0347...)(...G0347_...)
 * - BASE.C:170 - declaration (Atari ST)
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 16,
    kOutOfRange = 0
};

static const unsigned int s_g0347[kTableSize] = {
    /*  0 */ 0x000, /*  1 */ 0x666, /*  2 */ 0x888, /*  3 */ 0x620,
    /*  4 */ 0x0CC, /*  5 */ 0x840, /*  6 */ 0x080, /*  7 */ 0x0C0,
    /*  8 */ 0xF00, /*  9 */ 0xFA0, /* 10 */ 0xC86, /* 11 */ 0xFF0,
    /* 12 */ 0x444, /* 13 */ 0xAAA, /* 14 */ 0x00F, /* 15 */ 0xFFF
};

const unsigned int *
dm1_v1_palette_top_and_bottom_screen_table_pc34(void)
{
    return s_g0347;
}

int
dm1_v1_palette_top_and_bottom_screen_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_top_and_bottom_screen_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0347[entry_index];
}

int
dm1_v1_palette_top_and_bottom_screen_run_pc34(
    DM1_V1_PaletteTopAndBottomScreenResultPc34 *out)
{
    int table_matches_declaration = 1;
    int entry_0_black = 1;
    int entry_1_dark_gray = 1;
    int entry_15_bright_white = 1;
    int all_entries_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0347[i];
    }
    out->tableSize = kTableSize;

    if (s_g0347[0]  != 0x000) entry_0_black = 0;
    if (s_g0347[1]  != 0x666) entry_1_dark_gray = 0;
    if (s_g0347[15] != 0xFFF) entry_15_bright_white = 0;
    out->entry0Black = entry_0_black;
    out->entry1DarkGray = entry_1_dark_gray;
    out->entry15BrightWhite = entry_15_bright_white;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0347[i] > 0xFFFF) all_entries_in_byte_range = 0;
    }
    out->allEntriesInByteRange = all_entries_in_byte_range;

    {
        static const unsigned int kExpected[kTableSize] = {
            0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0,
            0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0347[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_top_and_bottom_screen_get_pc34(i) != (int)s_g0347[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_palette_top_and_bottom_screen_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_top_and_bottom_screen_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_top_and_bottom_screen_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->entry0Black &&
        out->entry1DarkGray &&
        out->entry15BrightWhite &&
        out->allEntriesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 8;
    return out->accepted;
}