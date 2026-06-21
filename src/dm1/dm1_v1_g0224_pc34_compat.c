#include "firestaff/dm1/v1/G0224_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0224):
 * - DUNVIEW.C:513 - declaration
 * - DUNVIEW.C:1836 - PC 3.4 EN init (3 sets × 11 frames × 5 coords × 2 fields)
 * - DUNVIEW.C:5307 - read site
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kSets      = 3,
    kRows      = 11,
    kCols      = 5,
    kFields    = 2,
    kTableSize = 330,
    kOutOfRange = -1
};

static const unsigned char s_g0224[kTableSize] = {
    95, 70, 127, 70, 129, 75, 93, 75, 111, 72, 131, 70,
    163, 70, 158, 75, 120, 75, 145, 72, 59, 70, 91, 70,
    107, 75, 66, 75, 79, 72, 92, 81, 131, 81, 132, 90,
    91, 90, 111, 85, 99, 81, 146, 81, 135, 90, 80, 90,
    120, 85, 77, 81, 124, 81, 143, 90, 89, 90, 105, 85,
    83, 103, 141, 103, 148, 119, 76, 119, 109, 111, 46, 103,
    118, 103, 101, 119, 0, 0, 79, 111, 107, 103, 177, 103,
    0, 0, 123, 119, 144, 111, 0, 0, 67, 135, 0, 0,
    0, 0, 0, 0, 156, 135, 0, 0, 0, 0, 0, 0,
    0, 0, 94, 75, 128, 75, 111, 70, 111, 72, 111, 75,
    120, 75, 158, 75, 149, 70, 145, 72, 150, 75, 66, 75,
    104, 75, 75, 70, 79, 72, 73, 75, 91, 90, 132, 90,
    111, 83, 111, 85, 111, 90, 80, 90, 135, 90, 125, 83,
    120, 85, 125, 90, 89, 90, 143, 90, 99, 83, 105, 85,
    98, 90, 81, 119, 142, 119, 111, 105, 111, 111, 111, 119,
    0, 0, 101, 119, 84, 105, 70, 111, 77, 119, 123, 119,
    0, 0, 139, 105, 153, 111, 146, 119, 0, 0, 83, 130,
    57, 121, 47, 126, 57, 130, 140, 130, 0, 0, 166, 121,
    176, 126, 166, 130, 95, 59, 127, 59, 129, 61, 93, 61,
    111, 60, 131, 59, 163, 59, 158, 61, 120, 61, 145, 60,
    59, 59, 91, 59, 107, 61, 66, 61, 79, 60, 92, 65,
    131, 65, 132, 67, 91, 67, 111, 66, 99, 65, 146, 65,
    135, 67, 80, 67, 120, 66, 77, 65, 124, 65, 143, 67,
    89, 67, 105, 66, 83, 79, 141, 79, 148, 85, 76, 85,
    111, 81, 46, 79, 118, 79, 101, 85, 0, 0, 79, 81,
    107, 79, 177, 79, 0, 0, 123, 85, 144, 81, 0, 0,
    67, 96, 0, 0, 0, 0, 0, 0, 156, 96, 0, 0,
    0, 0, 0, 0, 0, 0
};

const unsigned char *
dm1_v1_g0224_table_pc34(void)
{
    return s_g0224;
}

int
dm1_v1_g0224_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0224_get_pc34(int set_index, int frame_index, int coord_index, int field_index)
{
    if (set_index < 0 || set_index >= kSets) return kOutOfRange;
    if (frame_index < 0 || frame_index >= kRows) return kOutOfRange;
    if (coord_index < 0 || coord_index >= kCols) return kOutOfRange;
    if (field_index < 0 || field_index >= kFields) return kOutOfRange;
    return (int)s_g0224[((set_index * kRows + frame_index) * kCols + coord_index) * kFields + field_index];
}

int
dm1_v1_g0224_run_pc34(
    DM1_V1_G0224ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int all_bytes_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0224[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0224[i] > 255) all_bytes_in_byte_range = 0;
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {            95, 70, 127, 70, 129, 75, 93, 75, 111, 72, 131, 70,
            163, 70, 158, 75, 120, 75, 145, 72, 59, 70, 91, 70,
            107, 75, 66, 75, 79, 72, 92, 81, 131, 81, 132, 90,
            91, 90, 111, 85, 99, 81, 146, 81, 135, 90, 80, 90,
            120, 85, 77, 81, 124, 81, 143, 90, 89, 90, 105, 85,
            83, 103, 141, 103, 148, 119, 76, 119, 109, 111, 46, 103,
            118, 103, 101, 119, 0, 0, 79, 111, 107, 103, 177, 103,
            0, 0, 123, 119, 144, 111, 0, 0, 67, 135, 0, 0,
            0, 0, 0, 0, 156, 135, 0, 0, 0, 0, 0, 0,
            0, 0, 94, 75, 128, 75, 111, 70, 111, 72, 111, 75,
            120, 75, 158, 75, 149, 70, 145, 72, 150, 75, 66, 75,
            104, 75, 75, 70, 79, 72, 73, 75, 91, 90, 132, 90,
            111, 83, 111, 85, 111, 90, 80, 90, 135, 90, 125, 83,
            120, 85, 125, 90, 89, 90, 143, 90, 99, 83, 105, 85,
            98, 90, 81, 119, 142, 119, 111, 105, 111, 111, 111, 119,
            0, 0, 101, 119, 84, 105, 70, 111, 77, 119, 123, 119,
            0, 0, 139, 105, 153, 111, 146, 119, 0, 0, 83, 130,
            57, 121, 47, 126, 57, 130, 140, 130, 0, 0, 166, 121,
            176, 126, 166, 130, 95, 59, 127, 59, 129, 61, 93, 61,
            111, 60, 131, 59, 163, 59, 158, 61, 120, 61, 145, 60,
            59, 59, 91, 59, 107, 61, 66, 61, 79, 60, 92, 65,
            131, 65, 132, 67, 91, 67, 111, 66, 99, 65, 146, 65,
            135, 67, 80, 67, 120, 66, 77, 65, 124, 65, 143, 67,
            89, 67, 105, 66, 83, 79, 141, 79, 148, 85, 76, 85,
            111, 81, 46, 79, 118, 79, 101, 85, 0, 0, 79, 81,
            107, 79, 177, 79, 0, 0, 123, 85, 144, 81, 0, 0,
            67, 96, 0, 0, 0, 0, 0, 0, 156, 96, 0, 0,
            0, 0, 0, 0, 0, 0
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0224[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int set_idx = i / (kRows * kCols * kFields);
        int frame_idx = (i / (kCols * kFields)) % kRows;
        int coord_idx = (i / kFields) % kCols;
        int field_idx = i % kFields;
        if (dm1_v1_g0224_get_pc34(set_idx, frame_idx, coord_idx, field_idx) != (int)s_g0224[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0224_get_pc34(-1, 0, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(0, -1, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(0, 0, -1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(0, 0, 0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(kSets, 0, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(0, kRows, 0, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(0, 0, kCols, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(0, 0, 0, kFields) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0224_get_pc34(999, 999, 999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
