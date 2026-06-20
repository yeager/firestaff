#include "firestaff/dm1/v1/animtown_color_credits_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G8147_CREDITS):
 * - DRAWVIEW.C:25/9/404/624-629 - declaration + ANIMTOWN.C duplicate + dispatch table + palette-walk loop (17 COLOR_DEF entries)
 * - ANIMTOWN.C:9 - duplicate declaration
 * - ANIMTOWN.C:393-426 - G8176_PaletteTable[6] = G8147_CREDITS
 * - ANIMTOWN.C:431+ - G8175_CREAT_PAL[14][6] (not related)
 * - ANIMTOWN.C:624-629 - palette-walk loop reads until Index == 0xFF
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - ANIMTOWN.C:9/404/624-629 — read sites
 */

enum {
    kTableSize  = 17,
    kOutOfRange = -1
};

/* G8147_CREDITS PC 3.4 EN init (DRAWVIEW.C:25). The sentinel terminator
 * at index 16 has ColorIndex = 0xFF (DRAWVIEW.C:25 last entry). */
static const unsigned char s_g8147[kTableSize * 4] = {
    /* 0  */ 0x00, 0x00, 0x00, 0x1B,
    /* 1  */ 0x01, 0x00, 0x2D, 0x2D,
    /* 2  */ 0x02, 0x3F, 0x3F, 0x1B,
    /* 3  */ 0x03, 0x24, 0x12, 0x00,
    /* 4  */ 0x04, 0x3F, 0x3F, 0x24,
    /* 5  */ 0x05, 0x00, 0x00, 0x00,
    /* 6  */ 0x06, 0x00, 0x24, 0x00,
    /* 7  */ 0x07, 0x2D, 0x00, 0x00,
    /* 8  */ 0x08, 0x36, 0x24, 0x12,
    /* 9  */ 0x09, 0x3F, 0x3F, 0x2D,
    /* 10 */ 0x0A, 0x3F, 0x24, 0x12,
    /* 11 */ 0x0B, 0x3F, 0x36, 0x00,
    /* 12 */ 0x0C, 0x3F, 0x2D, 0x00,
    /* 13 */ 0x0D, 0x00, 0x00, 0x00,
    /* 14 */ 0x0E, 0x1B, 0x09, 0x00,
    /* 15 */ 0x0F, 0x3F, 0x3F, 0x36,
    /* 16 */ 0xFF, 0x00, 0x00, 0x00
};

const unsigned char *
dm1_v1_animtown_color_credits_table_pc34(void)
{
    return s_g8147;
}

int
dm1_v1_animtown_color_credits_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_animtown_color_credits_get_pc34(int entry_index, int field_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    if (field_index < 0 || field_index >= 4) {
        return kOutOfRange;
    }
    return (int)s_g8147[entry_index * 4 + field_index];
}

int
dm1_v1_animtown_color_credits_run_pc34(
    DM1_V1_AnimtownColorCreditsResultPc34 *out)
{
    int table_matches_declaration = 1;
    int first_entry_index_0 = 1;
    int first_entry_color_black = 1;
    int second_entry_index_1_dark_color = 1;
    int last_entry_sentinel_index_0xff = 1;
    int all_rgb_in_byte_range = 1;
    int all_indices_nonzero_except_last = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize * 4; ++i) {
        out->tableEntries[i] = (int)s_g8147[i];
    }
    out->tableSize = kTableSize;

    if (s_g8147[0] != 0x00) first_entry_index_0 = 0;
    if (s_g8147[1] != 0x00 || s_g8147[2] != 0x00 || s_g8147[3] != 0x1B) {
        first_entry_color_black = 0;
    }
    if (s_g8147[4] != 0x01) second_entry_index_1_dark_color = 0;
    if (s_g8147[kTableSize * 4 - 4] != 0xFF) last_entry_sentinel_index_0xff = 0;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g8147[i * 4 + 1] > 63) all_rgb_in_byte_range = 0;
        if (s_g8147[i * 4 + 2] > 63) all_rgb_in_byte_range = 0;
        if (s_g8147[i * 4 + 3] > 63) all_rgb_in_byte_range = 0;
    }
    out->allRgbInByteRange = all_rgb_in_byte_range;
    out->firstEntryIndex0 = first_entry_index_0;
    out->firstEntryColorBlack = first_entry_color_black;
    out->secondEntryIndex1DarkColor = second_entry_index_1_dark_color;
    out->lastEntrySentinelIndex0xFF = last_entry_sentinel_index_0xff;

    /* All indices except last must be in [0, 0x0F]. */
    for (i = 0; i < kTableSize - 1; ++i) {
        if (s_g8147[i * 4 + 0] > 0x0F) all_indices_nonzero_except_last = 0;
    }
    out->allIndicesNonZeroExceptLast = all_indices_nonzero_except_last;

    {
        static const unsigned char kExpected[kTableSize * 4] = {
            0x00, 0x00, 0x00, 0x1B,
            0x01, 0x00, 0x2D, 0x2D,
            0x02, 0x3F, 0x3F, 0x1B,
            0x03, 0x24, 0x12, 0x00,
            0x04, 0x3F, 0x3F, 0x24,
            0x05, 0x00, 0x00, 0x00,
            0x06, 0x00, 0x24, 0x00,
            0x07, 0x2D, 0x00, 0x00,
            0x08, 0x36, 0x24, 0x12,
            0x09, 0x3F, 0x3F, 0x2D,
            0x0A, 0x3F, 0x24, 0x12,
            0x0B, 0x3F, 0x36, 0x00,
            0x0C, 0x3F, 0x2D, 0x00,
            0x0D, 0x00, 0x00, 0x00,
            0x0E, 0x1B, 0x09, 0x00,
            0x0F, 0x3F, 0x3F, 0x36,
            0xFF, 0x00, 0x00, 0x00
        };
        for (i = 0; i < kTableSize * 4; ++i) {
            if (s_g8147[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int j;
        for (j = 0; j < 4; ++j) {
            if (dm1_v1_animtown_color_credits_get_pc34(i, j) !=
                (int)s_g8147[i * 4 + j]) {
                lookup_function_correct = 0;
            }
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_animtown_color_credits_get_pc34(-1, 0) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_credits_get_pc34(0, -1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_credits_get_pc34(kTableSize, 0) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_credits_get_pc34(0, 4) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_credits_get_pc34(999, 999) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntryIndex0 &&
        out->firstEntryColorBlack &&
        out->secondEntryIndex1DarkColor &&
        out->lastEntrySentinelIndex0xFF &&
        out->allRgbInByteRange &&
        out->allIndicesNonZeroExceptLast &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 9;
    return out->accepted;
}