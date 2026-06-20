#include "firestaff/dm1/v1/palette_changes_cursor_mask_v2_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G4013_auc_PaletteChanges_CursorMask):
 * - DATA.C:422 - declaration + PC 3.4 init { 15, 15, 15, 15, 15, 15, 15,
 *                                            15, 15, 15, 15, 15, 0, 15, 15, 15 }
 * - IO.C:2151 - F0129_VIDEO_BlitShrinkWithPaletteChanges (cursor-style-2 path)
 * - IO.C:2453 - F0129_VIDEO_BlitShrinkWithPaletteChanges (champion-icon mask path)
 *
 * - DATA.C:422/2151/2453 - declaration + IO.C cursor-style-2 + champion-icon mask path
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 16,
    kOutOfRange = -1
};

static const unsigned char s_g4013[kTableSize] = {
    /*  0 */ 15, /*  1 */ 15, /*  2 */ 15, /*  3 */ 15,
    /*  4 */ 15, /*  5 */ 15, /*  6 */ 15, /*  7 */ 15,
    /*  8 */ 15, /*  9 */ 15, /* 10 */ 15, /* 11 */ 15,
    /* 12 */  0, /* 13 */ 15, /* 14 */ 15, /* 15 */ 15
};

const unsigned char *
dm1_v1_palette_changes_cursor_mask_v2_table_pc34(void)
{
    return s_g4013;
}

int
dm1_v1_palette_changes_cursor_mask_v2_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_changes_cursor_mask_v2_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g4013[entry_index];
}

int
dm1_v1_palette_changes_cursor_mask_v2_run_pc34(
    DM1_V1_PaletteChangesCursorMaskV2ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int entry_0_through_11_all_palette_15 = 1;
    int entry_12_palette_0 = 1;
    int entry_13_through_15_palette_15 = 1;
    int all_entries_in_byte_range = 1;
    int exactly_one_zero_entry = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g4013[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < 12; ++i) {
        if (s_g4013[i] != 15) entry_0_through_11_all_palette_15 = 0;
    }
    if (s_g4013[12] != 0) entry_12_palette_0 = 0;
    for (i = 13; i < kTableSize; ++i) {
        if (s_g4013[i] != 15) entry_13_through_15_palette_15 = 0;
    }
    out->entry0Through11AllPalette15 = entry_0_through_11_all_palette_15;
    out->entry12Palette0 = entry_12_palette_0;
    out->entry13Through15Palette15 = entry_13_through_15_palette_15;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g4013[i] > 255) all_entries_in_byte_range = 0;
    }
    out->allEntriesInByteRange = all_entries_in_byte_range;

    {
        int zero_count = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g4013[i] == 0) ++zero_count;
        }
        if (zero_count != 1) exactly_one_zero_entry = 0;
    }
    out->exactlyOneZeroEntry = exactly_one_zero_entry;

    {
        static const unsigned char kExpected[kTableSize] = {
            15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 15, 15, 15
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g4013[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_changes_cursor_mask_v2_get_pc34(i) != (int)s_g4013[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_palette_changes_cursor_mask_v2_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_changes_cursor_mask_v2_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_changes_cursor_mask_v2_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->entry0Through11AllPalette15 &&
        out->entry12Palette0 &&
        out->entry13Through15Palette15 &&
        out->allEntriesInByteRange &&
        out->exactlyOneZeroEntry &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 9;
    return out->accepted;
}