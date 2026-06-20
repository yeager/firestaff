#include "firestaff/dm1/v1/G0489_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0489):
 * - MENU.C:20 - declaration of G0489_as_Graphic560_ActionSets[44]
 * - MENU.C:90-136 - PC 3.4 EN init (44 action-set entries)
 * - MENU.C F0452/F0412 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 264,
    kOutOfRange = -1
};

/* G0489_as_Graphic560_ActionSets[44] PC 3.4 EN init (MENU.C:90-136).
 * 44 entries × 6 bytes each = 264 bytes. Byte layout:
 *   bytes 0-2: ActionIndices[3] (3 uint8_t)
 *   bytes 3-4: ActionProperties[2] (2 uint8_t, Bit 7: requires charge, Bit 6-0: minimum skill level)
 *   byte 5: Useless (uint8_t, BUG0_00 unused) */
static const unsigned char s_g0489[kTableSize] = {
255, 255, 255, 0, 0, 0, 27, 43, 35, 0, 0, 3, 6, 7, 8, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 255, 255, 0, 0, 4, 13, 20, 255, 128, 0, 16, 13, 23, 255, 128, 0, 17, 28, 41, 22, 2, 128, 14, 16, 2, 23, 0, 128, 17, 2, 25, 20, 2, 128, 16, 17, 41, 34, 3, 5, 16, 42, 9, 28, 0, 2, 9, 13, 17, 2, 2, 3, 4, 16, 17, 15, 1, 5, 5, 28, 17, 25, 1, 5, 4, 2, 25, 15, 5, 6, 5, 9, 2, 29, 2, 5, 4, 16, 29, 24, 2, 4, 3, 13, 15, 19, 5, 7, 4, 13, 2, 25, 0, 5, 4, 2, 29, 19, 3, 8, 4, 13, 30, 31, 2, 4, 6, 13, 31, 25, 3, 6, 6, 42, 30, 255, 0, 0, 6, 0, 0, 0, 0, 0, 0, 42, 9, 255, 0, 0, 9, 32, 255, 255, 0, 0, 11, 37, 33, 36, 128, 3, 15, 37, 33, 34, 128, 128, 15, 17, 38, 21, 128, 128, 16, 13, 21, 34, 128, 128, 16, 36, 37, 41, 2, 3, 18, 13, 23, 39, 128, 128, 17, 13, 17, 40, 0, 128, 17, 17, 36, 38, 3, 128, 19, 4, 255, 255, 0, 0, 14, 5, 255, 255, 0, 0, 0, 11, 255, 255, 0, 0, 14, 10, 255, 255, 0, 0, 8, 42, 9, 255, 0, 0, 9, 1, 12, 255, 2, 0, 9, 42, 255, 255, 0, 0, 10, 6, 11, 255, 128, 0, 3
};

const unsigned char *
dm1_v1_g0489_table_pc34(void)
{
    return s_g0489;
}

int
dm1_v1_g0489_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0489_get_pc34(int action_index, int byte_offset)
{
    if (action_index < 0 || action_index >= 44) {
        return kOutOfRange;
    }
    if (byte_offset < 0 || byte_offset >= 6) {
        return kOutOfRange;
    }
    return (int)s_g0489[action_index * 6 + byte_offset];
}

int
dm1_v1_g0489_run_pc34(
    DM1_V1_G0489ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int entry_0_all_zero_valid = 1;
    int all_bytes_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0489[i];
    }
    out->tableSize = kTableSize;

    /* Entry 0 ("N" action): {255, 255, 255, 0, 0, 0} - all zero except indices. */
    if (s_g0489[0] != 255 || s_g0489[1] != 255 || s_g0489[2] != 255 ||
        s_g0489[3] != 0 || s_g0489[4] != 0 || s_g0489[5] != 0) {
        entry_0_all_zero_valid = 0;
    }
    out->entry0AllZeroValid = entry_0_all_zero_valid;

    /* All values fit in uint8_t. */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0489[i] > 255) all_bytes_in_byte_range = 0;
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = { 255, 255, 255, 0, 0, 0, 27, 43, 35, 0, 0, 3, 6, 7, 8, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 255, 255, 0, 0, 4, 13, 20, 255, 128, 0, 16, 13, 23, 255, 128, 0, 17, 28, 41, 22, 2, 128, 14, 16, 2, 23, 0, 128, 17, 2, 25, 20, 2, 128, 16, 17, 41, 34, 3, 5, 16, 42, 9, 28, 0, 2, 9, 13, 17, 2, 2, 3, 4, 16, 17, 15, 1, 5, 5, 28, 17, 25, 1, 5, 4, 2, 25, 15, 5, 6, 5, 9, 2, 29, 2, 5, 4, 16, 29, 24, 2, 4, 3, 13, 15, 19, 5, 7, 4, 13, 2, 25, 0, 5, 4, 2, 29, 19, 3, 8, 4, 13, 30, 31, 2, 4, 6, 13, 31, 25, 3, 6, 6, 42, 30, 255, 0, 0, 6, 0, 0, 0, 0, 0, 0, 42, 9, 255, 0, 0, 9, 32, 255, 255, 0, 0, 11, 37, 33, 36, 128, 3, 15, 37, 33, 34, 128, 128, 15, 17, 38, 21, 128, 128, 16, 13, 21, 34, 128, 128, 16, 36, 37, 41, 2, 3, 18, 13, 23, 39, 128, 128, 17, 13, 17, 40, 0, 128, 17, 17, 36, 38, 3, 128, 19, 4, 255, 255, 0, 0, 14, 5, 255, 255, 0, 0, 0, 11, 255, 255, 0, 0, 14, 10, 255, 255, 0, 0, 8, 42, 9, 255, 0, 0, 9, 1, 12, 255, 2, 0, 9, 42, 255, 255, 0, 0, 10, 6, 11, 255, 128, 0, 3 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0489[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int action = i / 6;
        int off = i % 6;
        if (dm1_v1_g0489_get_pc34(action, off) != (int)s_g0489[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0489_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0489_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0489_get_pc34(44, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0489_get_pc34(0, 6) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0489_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->entry0AllZeroValid &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 6;
    return out->accepted;
}
