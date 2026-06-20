#include "firestaff/dm1/v1/palette_changes_cursor_mask_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G4010_auc_PaletteChanges_CursorMask):
 * - DATA.C:420 - declaration + PC 3.4 init { 15, 0, 0, 0, 15, 0, 0, 0,
 *                                            0, 0, 0, 0, 0, 0, 0, 0 }
 * - IO.C:2144 - F0129_VIDEO_BlitShrinkWithPaletteChanges (cursor path)
 * - IO.C:2151 - F0129_VIDEO_BlitShrinkWithPaletteChanges (cursor path)
 *
 * - DATA.C:420/2144/2151 - declaration + IO.C cursor blit path
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 16,
    kOutOfRange = -1
};

static const unsigned char s_g4010[kTableSize] = {
    /*  0 */ 15, /*  1 */  0, /*  2 */  0, /*  3 */  0,
    /*  4 */ 15, /*  5 */  0, /*  6 */  0, /*  7 */  0,
    /*  8 */  0, /*  9 */  0, /* 10 */  0, /* 11 */  0,
    /* 12 */  0, /* 13 */  0, /* 14 */  0, /* 15 */  0
};

const unsigned char *
dm1_v1_palette_changes_cursor_mask_table_pc34(void)
{
    return s_g4010;
}

int
dm1_v1_palette_changes_cursor_mask_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_changes_cursor_mask_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g4010[entry_index];
}

int
dm1_v1_palette_changes_cursor_mask_run_pc34(
    DM1_V1_PaletteChangesCursorMaskResultPc34 *out)
{
    int table_matches_declaration = 1;
    int entry_0_palette_15 = 1;
    int entry_1_zero = 1;
    int entry_2_zero = 1;
    int entry_3_zero = 1;
    int entry_4_palette_15 = 1;
    int entry_5_through_15_zero = 1;
    int all_entries_in_byte_range = 1;
    int exactly_two_non_zero_entries = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g4010[i];
    }
    out->tableSize = kTableSize;

    if (s_g4010[0]  != 15) entry_0_palette_15 = 0;
    if (s_g4010[1]  != 0)  entry_1_zero = 0;
    if (s_g4010[2]  != 0)  entry_2_zero = 0;
    if (s_g4010[3]  != 0)  entry_3_zero = 0;
    if (s_g4010[4]  != 15) entry_4_palette_15 = 0;
    for (i = 5; i < kTableSize; ++i) {
        if (s_g4010[i] != 0) entry_5_through_15_zero = 0;
    }
    out->entry0Palette15 = entry_0_palette_15;
    out->entry1Zero = entry_1_zero;
    out->entry2Zero = entry_2_zero;
    out->entry3Zero = entry_3_zero;
    out->entry4Palette15 = entry_4_palette_15;
    out->entry5Through15Zero = entry_5_through_15_zero;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g4010[i] > 255) all_entries_in_byte_range = 0;
    }
    out->allEntriesInByteRange = all_entries_in_byte_range;

    {
        int nonzero = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g4010[i] != 0) ++nonzero;
        }
        if (nonzero != 2) exactly_two_non_zero_entries = 0;
    }
    out->exactlyTwoNonZeroEntries = exactly_two_non_zero_entries;

    {
        static const unsigned char kExpected[kTableSize] = {
            15, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g4010[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_changes_cursor_mask_get_pc34(i) != (int)s_g4010[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_palette_changes_cursor_mask_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_changes_cursor_mask_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_changes_cursor_mask_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->entry0Palette15 &&
        out->entry1Zero &&
        out->entry2Zero &&
        out->entry3Zero &&
        out->entry4Palette15 &&
        out->entry5Through15Zero &&
        out->allEntriesInByteRange &&
        out->exactlyTwoNonZeroEntries &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 12;
    return out->accepted;
}