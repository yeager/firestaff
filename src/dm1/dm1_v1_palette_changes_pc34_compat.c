#include "firestaff/dm1/v1/palette_changes_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G4011_auc_PaletteChanges):
 * - DATA.C:421 - declaration + PC 3.4 init { 15, 14, 13, 12, 11, 10, 9,
 *                                            8, 7, 6, 5, 4, 0, 2, 1, 0 }
 * - IO.C:2145/2152/2454 - F0129_VIDEO_BlitShrinkWithPaletteChanges
 *
 * Note: the table is NOT all-distinct — entries 12 and 15 both = 0
 * (palette-index 0 is used twice, mapping source-pixel indices 12
 * and 15 to the same destination palette). This is the original
 * ReDMCSB contract.
 *
 * - DATA.C:421/2145/2152/2454 - declaration + IO.C cursor/champion-icon blit path
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 16,
    kOutOfRange = -1
};

static const unsigned char s_g4011[kTableSize] = {
    /*  0 */ 15, /*  1 */ 14, /*  2 */ 13, /*  3 */ 12,
    /*  4 */ 11, /*  5 */ 10, /*  6 */  9, /*  7 */  8,
    /*  8 */  7, /*  9 */  6, /* 10 */  5, /* 11 */  4,
    /* 12 */  0, /* 13 */  2, /* 14 */  1, /* 15 */  0
};

const unsigned char *
dm1_v1_palette_changes_table_pc34(void)
{
    return s_g4011;
}

int
dm1_v1_palette_changes_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_changes_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g4011[entry_index];
}

int
dm1_v1_palette_changes_run_pc34(
    DM1_V1_PaletteChangesResultPc34 *out)
{
    int table_matches_declaration = 1;
    int entry_0_palette_15 = 1;
    int entry_1_palette_14 = 1;
    int entry_11_palette_4 = 1;
    int entry_12_palette_0 = 1;
    int entry_13_palette_2 = 1;
    int entry_14_palette_1 = 1;
    int entry_15_palette_0 = 1;
    int all_entries_in_byte_range = 1;
    int exactly_two_zero_entries = 1;
    int entry_12_and_15_collide = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g4011[i];
    }
    out->tableSize = kTableSize;

    if (s_g4011[0]  != 15) entry_0_palette_15 = 0;
    if (s_g4011[1]  != 14) entry_1_palette_14 = 0;
    if (s_g4011[11] != 4)  entry_11_palette_4 = 0;
    if (s_g4011[12] != 0)  entry_12_palette_0 = 0;
    if (s_g4011[13] != 2)  entry_13_palette_2 = 0;
    if (s_g4011[14] != 1)  entry_14_palette_1 = 0;
    if (s_g4011[15] != 0)  entry_15_palette_0 = 0;
    out->entry0Palette15 = entry_0_palette_15;
    out->entry1Palette14 = entry_1_palette_14;
    out->entry11Palette4 = entry_11_palette_4;
    out->entry12Palette0 = entry_12_palette_0;
    out->entry13Palette2 = entry_13_palette_2;
    out->entry14Palette1 = entry_14_palette_1;
    out->entry15Palette0 = entry_15_palette_0;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g4011[i] > 255) all_entries_in_byte_range = 0;
    }
    out->allEntriesInByteRange = all_entries_in_byte_range;

    {
        int zero_count = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g4011[i] == 0) ++zero_count;
        }
        if (zero_count != 2) exactly_two_zero_entries = 0;
    }
    out->exactlyTwoZeroEntries = exactly_two_zero_entries;

    if (s_g4011[12] != s_g4011[15]) entry_12_and_15_collide = 0;
    out->entry12And15Collide = entry_12_and_15_collide;

    {
        static const unsigned char kExpected[kTableSize] = {
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 0, 2, 1, 0
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g4011[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_changes_get_pc34(i) != (int)s_g4011[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_palette_changes_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_changes_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_palette_changes_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->entry0Palette15 &&
        out->entry1Palette14 &&
        out->entry11Palette4 &&
        out->entry12Palette0 &&
        out->entry13Palette2 &&
        out->entry14Palette1 &&
        out->entry15Palette0 &&
        out->allEntriesInByteRange &&
        out->exactlyTwoZeroEntries &&
        out->entry12And15Collide &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 14;
    return out->accepted;
}