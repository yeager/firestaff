#include "firestaff/dm1/v1/G0497_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0497):
 * - MENU.C:34 - declaration
 * - MENU.C:427-470 - PC 3.4 EN init (44 entries)
 *   (uses MEDIA728 branch for BLOW HORN=1, HEAL=5, CALM=1, BRANDISH=3)
 * - MENU.C F0452/F0412 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 44,
    kOutOfRange = -1
};

/* G0497 PC 3.4 EN init (MENU.C:427-470, MEDIA728 branch). */
static const unsigned char s_g0497[kTableSize] = {
0, 8, 10, 0, 1, 0, 8, 13, 7, 15, 15, 22, 10, 6, 12, 19, 11, 17, 9, 40, 35, 25, 0, 30, 10, 24, 0, 25, 9, 12, 11, 10, 20, 20, 20, 12, 5, 1, 20, 30, 25, 3, 5, 1
};

const unsigned char *
dm1_v1_g0497_table_pc34(void)
{
    return s_g0497;
}

int
dm1_v1_g0497_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0497_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0497[entry_index];
}

int
dm1_v1_g0497_run_pc34(
    DM1_V1_G0497ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int entry_0_is_zero = 1;
    int entry_1_is_block_8 = 1;
    int all_values_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0497[i];
    }
    out->tableSize = kTableSize;

    if (s_g0497[0] != 0) entry_0_is_zero = 0;
    out->entry0IsZero = entry_0_is_zero;

    if (s_g0497[1] != 8) entry_1_is_block_8 = 0;
    out->entry1IsBlock8 = entry_1_is_block_8;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0497[i] > 255) all_values_in_byte_range = 0;
    }
    out->allValuesInByteRange = all_values_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = { 0, 8, 10, 0, 1, 0, 8, 13, 7, 15, 15, 22, 10, 6, 12, 19, 11, 17, 9, 40, 35, 25, 0, 30, 10, 24, 0, 25, 9, 12, 11, 10, 20, 20, 20, 12, 5, 1, 20, 30, 25, 3, 5, 1 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0497[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_g0497_get_pc34(i) != (int)s_g0497[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0497_get_pc34(-1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0497_get_pc34(kTableSize) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0497_get_pc34(999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->entry0IsZero &&
        out->entry1IsBlock8 &&
        out->allValuesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 7;
    return out->accepted;
}
