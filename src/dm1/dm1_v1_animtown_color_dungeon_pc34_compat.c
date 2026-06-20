#include "firestaff/dm1/v1/animtown_color_dungeon_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G8160_DUNGEON):
 * - DRAWVIEW.C:291 - declaration + PC 3.4 init
 * - ANIMTOWN.C:274 - duplicate declaration
 * - ANIMTOWN.C:410 - G8176_PaletteTable[13] = G8160_DUNGEON
 * - ANIMTOWN.C:624-629 - palette-walk loop reads until Index == 0xFF
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - DRAWVIEW.C:291/274/410/624-629, ANIMTOWN.C:274/410/624-629 — anchor references
 */

enum {
    kTableSize  = 9,
    kOutOfRange = -1
};

static const unsigned char s_g8160[kTableSize * 4] = {
    /* 0  */ 0x03, 0x2F, 0x27, 0x0F,
    /* 1  */ 0x04, 0x27, 0x17, 0x0F,
    /* 2  */ 0x05, 0x37, 0x2F, 0x0F,
    /* 3  */ 0x06, 0x2F, 0x17, 0x0F,
    /* 4  */ 0x07, 0x37, 0x37, 0x17,
    /* 5  */ 0x08, 0x3F, 0x3F, 0x0F,
    /* 6  */ 0x0B, 0x1F, 0x0F, 0x07,
    /* 7  */ 0x0C, 0x3F, 0x00, 0x00,
    /* 8  */ 0xFF, 0x00, 0x00, 0x00,
};

const unsigned char *
dm1_v1_animtown_color_dungeon_table_pc34(void)
{
    return s_g8160;
}

int
dm1_v1_animtown_color_dungeon_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_animtown_color_dungeon_get_pc34(int entry_index, int field_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    if (field_index < 0 || field_index >= 4) {
        return kOutOfRange;
    }
    return (int)s_g8160[entry_index * 4 + field_index];
}

int
dm1_v1_animtown_color_dungeon_run_pc34(
    DM1_V1_AnimtownColorDungeonResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g8160[i];
    }
    out->tableSize = kTableSize;

    if (s_g8160[kTableSize * 4 - 4] != 0xFF) last_entry_sentinel_index_0xff = 0;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g8160[i * 4 + 1] > 63) all_rgb_in_byte_range = 0;
        if (s_g8160[i * 4 + 2] > 63) all_rgb_in_byte_range = 0;
        if (s_g8160[i * 4 + 3] > 63) all_rgb_in_byte_range = 0;
    }
    out->allRgbInByteRange = all_rgb_in_byte_range;

    for (i = 0; i < kTableSize - 1; ++i) {
        if (s_g8160[i * 4 + 0] > 0x1F) all_indices_nonzero_except_last = 0;
    }
    out->allIndicesNonZeroExceptLast = all_indices_nonzero_except_last;

    {
        static const unsigned char kExpected[kTableSize * 4] = {
            0x03, 0x2F, 0x27, 0x0F,
            0x04, 0x27, 0x17, 0x0F,
            0x05, 0x37, 0x2F, 0x0F,
            0x06, 0x2F, 0x17, 0x0F,
            0x07, 0x37, 0x37, 0x17,
            0x08, 0x3F, 0x3F, 0x0F,
            0x0B, 0x1F, 0x0F, 0x07,
            0x0C, 0x3F, 0x00, 0x00,
            0xFF, 0x00, 0x00, 0x00
        };
        for (i = 0; i < kTableSize * 4; ++i) {
            if (s_g8160[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;
    out->lastEntrySentinelIndex0xFF = last_entry_sentinel_index_0xff;

    for (i = 0; i < kTableSize; ++i) {
        int j;
        for (j = 0; j < 4; ++j) {
            if (dm1_v1_animtown_color_dungeon_get_pc34(i, j) !=
                (int)s_g8160[i * 4 + j]) {
                lookup_function_correct = 0;
            }
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_animtown_color_dungeon_get_pc34(-1, 0) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_dungeon_get_pc34(0, -1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_dungeon_get_pc34(kTableSize, 0) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_dungeon_get_pc34(0, 4) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_animtown_color_dungeon_get_pc34(999, 999) != kOutOfRange) {
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
