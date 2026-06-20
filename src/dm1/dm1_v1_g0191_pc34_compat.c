#include "firestaff/dm1/v1/G0191_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements[11] (PC 3.4 EN init).
 *
 * G0191 is the 11-int frame coord table
 * for FloorOrnamentNativeBitmapIndexIncrements.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-933.
 */

enum {
    kTableSize  = 11,
    kOutOfRange = -1
};

static const int s_g0191[kTableSize] = {
    0, 0, 0, 1, 0, 2, 3, 2, 4, 5, 4
};

const int *
dm1_v1_g0191_table_pc34(void)
{
    return s_g0191;
}

int
dm1_v1_g0191_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0191_get_pc34(int value_index)
{
    if (value_index < 0 || value_index >= kTableSize) {
        return kOutOfRange;
    }
    return s_g0191[value_index];
}

int
dm1_v1_g0191_run_pc34(
    DM1_V1_G0191ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int all_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0191[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0191[i] < 0 || s_g0191[i] > 255) {
            all_in_byte_range = 0;
        }
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const int kExpected[kTableSize] = { 0, 0, 0, 1, 0, 2, 3, 2, 4, 5, 4 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0191[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_g0191_get_pc34(i) != s_g0191[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0191_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_g0191_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_g0191_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
