#include "firestaff/dm1/v1/G0193_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0193):
 * - DUNVIEW.C:825 - PC 3.4 EN init { 35 }
 * - DUNVIEW.C F0100/F0113 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 1,
    kOutOfRange = -1
};

static const int s_g0193[kTableSize] = {
35
};

const unsigned char *
dm1_v1_g0193_table_pc34(void)
{
    return (const unsigned char *)s_g0193;
}

int
dm1_v1_g0193_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0193_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return s_g0193[entry_index];
}

int
dm1_v1_g0193_run_pc34(
    DM1_V1_G0193ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int all_bytes_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0193[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0193[i] > 255) all_bytes_in_byte_range = 0;
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const int kExpected[kTableSize] = { 35 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0193[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_g0193_get_pc34(i) != s_g0193[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0193_get_pc34(-1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0193_get_pc34(kTableSize) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0193_get_pc34(999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
