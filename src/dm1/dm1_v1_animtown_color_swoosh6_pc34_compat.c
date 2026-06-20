#include "firestaff/dm1/v1/animtown_color_swoosh6_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G8167_SWOOSH6):
 * - DRAWVIEW.C:319 - declaration + PC 3.4 init
 * - ANIMTOWN.C:302 - duplicate declaration
 * - ANIMTOWN.C:418 - G8176_PaletteTable[21] = G8167_SWOOSH6
 * - ANIMTOWN.C:624-629 - palette-walk loop reads until Index == 0xFF
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - DRAWVIEW.C:319/302/418/624-629, ANIMTOWN.C:302/418/624-629 — anchor references
 */

enum {
    kTableSize  = 2,
    kOutOfRange = -1
};

static const unsigned char s_g8167[kTableSize * 4] = {
    /* 0  */ 0x06, 0x3F, 0x3F, 0x3F,
    /* 1  */ 0xFF, 0x00, 0x00, 0x00,
};

const unsigned char *
dm1_v1_animtown_color_swoosh6_table_pc34(void)
{
    return s_g8167;
}

int
dm1_v1_animtown_color_swoosh6_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_animtown_color_swoosh6_get_pc34(int entry_index, int field_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    if (field_index < 0 || field_index >= 4) {
        return kOutOfRange;
    }
    return (int)s_g8167[entry_index * 4 + field_index];
}

int
dm1_v1_animtown_color_swoosh6_run_pc34(
    DM1_V1_AnimtownColorSwoosh6ResultPc34 *out)
{
    int table_matches_declaration = 1;
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
        out->tableEntries[i] = (int)s_g8167[i];
    }
    out->tableSize = kTableSize;

    if (s_g8167[kTableSize * 4 - 4] != 0xFF) last_entry_sentinel_index_0xff = 0;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g8167[i * 4 + 1] > 63) all_rgb_in_byte_range = 0;
        if (s_g8167[i * 4 + 2] > 63) all_rgb_in_byte_range = 0;
        if (s_g8167[i * 4 + 3] > 63) all_rgb_in_byte_range = 0;
    }
    out->allRgbInByteRange = all_rgb_in_byte_range;

    for (i = 0; i < kTableSize - 1; ++i) {
        if (s_g8167[i * 4 + 0] > 0x1F) all_indices_nonzero_except_last = 0;
    }
    out->allIndicesNonZeroExceptLast = all_indices_nonzero_except_last;

    {
        static const unsigned char kExpected[kTableSize * 4] = {
            0x06, 0x3F, 0x3F, 0x3F,
            0xFF, 0x00, 0x00, 0x00
        };
        for (i = 0; i < kTableSize * 4; ++i) {
            if (s_g8167[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;
    out->lastEntrySentinelIndex0xFF = last_entry_sentinel_index_0xff;

    for (i = 0; i < kTableSize; ++i) {
        int j;
        for (j = 0; j < 4; ++j) {
            if (dm1_v1_animtown_color_swoosh6_get_pc34(i, j) !=
                (int)s_g8167[i * 4 + j]) {
                lookup_function_correct = 0;
            }
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_animtown_color_swoosh6_get_pc34(-1, 0) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_swoosh6_get_pc34(0, -1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_swoosh6_get_pc34(kTableSize, 0) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_swoosh6_get_pc34(0, 4) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_swoosh6_get_pc34(999, 999) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->lastEntrySentinelIndex0xFF &&
        out->allRgbInByteRange &&
        out->allIndicesNonZeroExceptLast &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 6;
    return out->accepted;
}
