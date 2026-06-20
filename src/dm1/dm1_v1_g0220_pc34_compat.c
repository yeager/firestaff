#include "firestaff/dm1/v1/G0220_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0220):
 * - DUNVIEW.C:497 - declaration
 * - DUNVIEW.C:1705 - PC 3.4 EN init (13 CREATURE_REPLACEMENT_COLOR_SET entries)
 * - DUNVIEW.C:2011-2014 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kEntries    = 13,
    kBytesPerEntry = 14,
    kTableSize  = 182,   /* kEntries * kBytesPerEntry */
    kOutOfRange = -1
};

static const unsigned char s_g0220[kTableSize] = {
12, 160, 10, 128, 8, 96, 6, 64, 4, 32, 2, 0, 9, 9, 0, 96, 0, 64, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 8, 96, 6, 64, 4, 32, 2, 0, 0, 0, 0, 0, 10, 10, 6, 64, 4, 32, 2, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 10, 0, 8, 0, 6, 0, 4, 0, 2, 0, 0, 9, 10, 0, 8, 0, 6, 0, 4, 0, 2, 0, 0, 0, 0, 10, 0, 8, 8, 6, 6, 4, 4, 2, 2, 0, 0, 0, 0, 9, 0, 10, 10, 8, 8, 6, 6, 4, 4, 2, 2, 0, 0, 10, 9, 15, 160, 12, 128, 10, 96, 8, 64, 6, 32, 4, 0, 10, 5, 15, 128, 12, 96, 10, 64, 8, 32, 6, 0, 2, 0, 5, 7, 8, 0, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0, 10, 12, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 12, 0, 12, 134, 10, 100, 8, 66, 6, 32, 4, 0, 2, 0, 10, 5
};

const unsigned char *
dm1_v1_g0220_table_pc34(void)
{
    return s_g0220;
}

int
dm1_v1_g0220_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0220_get_pc34(int entry_index, int byte_offset)
{
    if (entry_index < 0 || entry_index >= kEntries) {
        return kOutOfRange;
    }
    if (byte_offset < 0 || byte_offset >= kBytesPerEntry) {
        return kOutOfRange;
    }
    return (int)s_g0220[entry_index * kBytesPerEntry + byte_offset];
}

int
dm1_v1_g0220_run_pc34(
    DM1_V1_G0220ResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g0220[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0220[i] > 255) all_bytes_in_byte_range = 0;
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = { 12, 160, 10, 128, 8, 96, 6, 64, 4, 32, 2, 0, 9, 9, 0, 96, 0, 64, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 8, 96, 6, 64, 4, 32, 2, 0, 0, 0, 0, 0, 10, 10, 6, 64, 4, 32, 2, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 10, 0, 8, 0, 6, 0, 4, 0, 2, 0, 0, 9, 10, 0, 8, 0, 6, 0, 4, 0, 2, 0, 0, 0, 0, 10, 0, 8, 8, 6, 6, 4, 4, 2, 2, 0, 0, 0, 0, 9, 0, 10, 10, 8, 8, 6, 6, 4, 4, 2, 2, 0, 0, 10, 9, 15, 160, 12, 128, 10, 96, 8, 64, 6, 32, 4, 0, 10, 5, 15, 128, 12, 96, 10, 64, 8, 32, 6, 0, 2, 0, 5, 7, 8, 0, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0, 10, 12, 6, 0, 4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 12, 0, 12, 134, 10, 100, 8, 66, 6, 32, 4, 0, 2, 0, 10, 5 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0220[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int entry = i / kBytesPerEntry;
        int off = i % kBytesPerEntry;
        if (dm1_v1_g0220_get_pc34(entry, off) != (int)s_g0220[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0220_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0220_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0220_get_pc34(kEntries, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0220_get_pc34(0, kBytesPerEntry) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0220_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
