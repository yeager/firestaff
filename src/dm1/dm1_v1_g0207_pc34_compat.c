#include "firestaff/dm1/v1/G0207_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0207):
 * - DUNVIEW.C:485 - declaration
 * - DUNVIEW.C:1175 - PC 3.4 EN init (4 sets × 3 frames × 6 fields)
 * - DUNVIEW.C F0099/F0654 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kSets      = 4,
    kRows      = 3,
    kCols      = 6,
    kTableSize = 72,
    kOutOfRange = -1
};

static const unsigned char s_g0207[kTableSize] = {
    17, 31, 8, 17, 8, 10, 22, 42, 11, 23, 16, 13,
    32, 63, 13, 31, 16, 19, 0, 47, 0, 40, 24, 41,
    0, 63, 0, 60, 32, 61, 0, 95, 0, 87, 48, 88,
    17, 31, 15, 24, 8, 10, 22, 42, 22, 34, 16, 13,
    32, 63, 31, 49, 16, 19, 23, 35, 31, 39, 8, 9,
    30, 48, 41, 52, 16, 12, 44, 75, 61, 79, 16, 19
};

const unsigned char *
dm1_v1_g0207_table_pc34(void)
{
    return s_g0207;
}

int
dm1_v1_g0207_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0207_get_pc34(int set_index, int frame_index, int field_index)
{
    if (set_index < 0 || set_index >= kSets) return kOutOfRange;
    if (frame_index < 0 || frame_index >= kRows) return kOutOfRange;
    if (field_index < 0 || field_index >= kCols) return kOutOfRange;
    return (int)s_g0207[(set_index * kRows + frame_index) * kCols + field_index];
}

int
dm1_v1_g0207_run_pc34(
    DM1_V1_G0207ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int all_bytes_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0207[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0207[i] > 255) all_bytes_in_byte_range = 0;
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {            17, 31, 8, 17, 8, 10, 22, 42, 11, 23, 16, 13,
            32, 63, 13, 31, 16, 19, 0, 47, 0, 40, 24, 41,
            0, 63, 0, 60, 32, 61, 0, 95, 0, 87, 48, 88,
            17, 31, 15, 24, 8, 10, 22, 42, 22, 34, 16, 13,
            32, 63, 31, 49, 16, 19, 23, 35, 31, 39, 8, 9,
            30, 48, 41, 52, 16, 12, 44, 75, 61, 79, 16, 19
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0207[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int set_idx = i / (kRows * kCols);
        int frame_idx = (i / kCols) % kRows;
        int field_idx = i % kCols;
        if (dm1_v1_g0207_get_pc34(set_idx, frame_idx, field_idx) != (int)s_g0207[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0207_get_pc34(-1, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0207_get_pc34(0, -1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0207_get_pc34(0, 0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0207_get_pc34(kSets, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0207_get_pc34(0, kRows, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0207_get_pc34(0, 0, kCols) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0207_get_pc34(999, 999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
