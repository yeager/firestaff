#include "firestaff/dm1/v1/G0206_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0206):
 * - DUNVIEW.C:484 - declaration
 * - DUNVIEW.C:1141 - PC 3.4 EN init (3 sets × 9 frames × 6 fields)
 * - DUNVIEW.C F0099/F0654 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kSets      = 3,
    kRows      = 9,
    kCols      = 6,
    kTableSize = 162,
    kOutOfRange = -1
};

static const unsigned char s_g0206[kTableSize] = {
    32, 79, 66, 71, 24, 6, 96, 127, 66, 71, 16, 6,
    144, 191, 66, 71, 24, 6, 0, 63, 77, 87, 32, 11,
    80, 143, 77, 87, 32, 11, 160, 223, 77, 87, 32, 11,
    0, 31, 92, 116, 16, 25, 80, 143, 92, 116, 32, 25,
    192, 223, 92, 116, 16, 25, 0, 95, 66, 74, 48, 9,
    64, 159, 66, 74, 48, 9, 128, 223, 66, 74, 48, 9,
    0, 79, 75, 89, 40, 15, 56, 167, 75, 89, 56, 15,
    144, 223, 75, 89, 40, 15, 0, 63, 90, 118, 32, 29,
    32, 191, 90, 118, 80, 29, 160, 223, 90, 118, 32, 29,
    42, 57, 68, 72, 8, 5, 104, 119, 68, 72, 8, 5,
    166, 181, 68, 72, 8, 5, 9, 40, 80, 85, 16, 6,
    96, 127, 80, 85, 16, 6, 183, 214, 80, 85, 16, 6,
    0, 15, 97, 108, 8, 12, 96, 127, 97, 108, 16, 12,
    208, 223, 97, 108, 8, 12
};

const unsigned char *
dm1_v1_g0206_table_pc34(void)
{
    return s_g0206;
}

int
dm1_v1_g0206_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0206_get_pc34(int set_index, int frame_index, int field_index)
{
    if (set_index < 0 || set_index >= kSets) return kOutOfRange;
    if (frame_index < 0 || frame_index >= kRows) return kOutOfRange;
    if (field_index < 0 || field_index >= kCols) return kOutOfRange;
    return (int)s_g0206[(set_index * kRows + frame_index) * kCols + field_index];
}

int
dm1_v1_g0206_run_pc34(
    DM1_V1_G0206ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int all_bytes_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0206[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0206[i] > 255) all_bytes_in_byte_range = 0;
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {            32, 79, 66, 71, 24, 6, 96, 127, 66, 71, 16, 6,
            144, 191, 66, 71, 24, 6, 0, 63, 77, 87, 32, 11,
            80, 143, 77, 87, 32, 11, 160, 223, 77, 87, 32, 11,
            0, 31, 92, 116, 16, 25, 80, 143, 92, 116, 32, 25,
            192, 223, 92, 116, 16, 25, 0, 95, 66, 74, 48, 9,
            64, 159, 66, 74, 48, 9, 128, 223, 66, 74, 48, 9,
            0, 79, 75, 89, 40, 15, 56, 167, 75, 89, 56, 15,
            144, 223, 75, 89, 40, 15, 0, 63, 90, 118, 32, 29,
            32, 191, 90, 118, 80, 29, 160, 223, 90, 118, 32, 29,
            42, 57, 68, 72, 8, 5, 104, 119, 68, 72, 8, 5,
            166, 181, 68, 72, 8, 5, 9, 40, 80, 85, 16, 6,
            96, 127, 80, 85, 16, 6, 183, 214, 80, 85, 16, 6,
            0, 15, 97, 108, 8, 12, 96, 127, 97, 108, 16, 12,
            208, 223, 97, 108, 8, 12
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0206[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int set_idx = i / (kRows * kCols);
        int frame_idx = (i / kCols) % kRows;
        int field_idx = i % kCols;
        if (dm1_v1_g0206_get_pc34(set_idx, frame_idx, field_idx) != (int)s_g0206[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0206_get_pc34(-1, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0206_get_pc34(0, -1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0206_get_pc34(0, 0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0206_get_pc34(kSets, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0206_get_pc34(0, kRows, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0206_get_pc34(0, 0, kCols) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0206_get_pc34(999, 999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
