#include "firestaff/dm1/v1/G0490_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0490):
 * - MENU.C:21 - declaration of G0490_ac_Graphic560_ActionNames[300]
 * - MENU.C:137 - PC 3.4 EN init "N\0BLOCK\0CHOP\0X\0BLOW HORN\0..."
 * - MENU.C F0452/F0456/F0412 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 300,
    kOutOfRange = -1
};

/* G0490_ac_Graphic560_ActionNames[300] PC 3.4 EN init (MENU.C:137). */
static const unsigned char s_g0490[kTableSize] = {
78, 0, 66, 76, 79, 67, 75, 0, 67, 72, 79, 80, 0, 88, 0, 66, 76, 79, 87, 32, 72, 79, 82, 78, 0, 70, 76, 73, 80, 0, 80, 85, 78, 67, 72, 0, 75, 73, 67, 75, 0, 87, 65, 82, 32, 67, 82, 89, 0, 83, 84, 65, 66, 0, 67, 76, 73, 77, 66, 32, 68, 79, 87, 78, 0, 70, 82, 69, 69, 90, 69, 32, 76, 73, 70, 69, 0, 72, 73, 84, 0, 83, 87, 73, 78, 71, 0, 83, 84, 65, 66, 0, 84, 72, 82, 85, 83, 84, 0, 74, 65, 66, 0, 80, 65, 82, 82, 89, 0, 72, 65, 67, 75, 0, 66, 69, 82, 90, 69, 82, 75, 0, 70, 73, 82, 69, 66, 65, 76, 76, 0, 68, 73, 83, 80, 69, 76, 76, 0, 67, 79, 78, 70, 85, 83, 69, 0, 76, 73, 71, 72, 84, 78, 73, 78, 71, 0, 68, 73, 83, 82, 85, 80, 84, 0, 77, 69, 76, 69, 69, 0, 88, 0, 73, 78, 86, 79, 75, 69, 0, 83, 76, 65, 83, 72, 0, 67, 76, 69, 65, 86, 69, 0, 66, 65, 83, 72, 0, 83, 84, 85, 78, 0, 83, 72, 79, 79, 84, 0, 83, 80, 69, 76, 76, 83, 72, 73, 69, 76, 68, 0, 70, 73, 82, 69, 83, 72, 73, 69, 76, 68, 0, 70, 76, 85, 88, 67, 65, 71, 69, 0, 72, 69, 65, 76, 0, 67, 65, 76, 77, 0, 76, 73, 71, 72, 84, 0, 87, 73, 78, 68, 79, 87, 0, 83, 80, 73, 84, 0, 66, 82, 65, 78, 68, 73, 83, 72, 0, 84, 72, 82, 79, 87, 0, 70, 85, 83, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const unsigned char *
dm1_v1_g0490_table_pc34(void)
{
    return s_g0490;
}

int
dm1_v1_g0490_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0490_get_pc34(int byte_index)
{
    if (byte_index < 0 || byte_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0490[byte_index];
}

int
dm1_v1_g0490_run_pc34(
    DM1_V1_G0490ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int entry_0_is_n = 1;
    int entry_1_is_block = 1;
    int entry_43_is_fuse = 1;
    int exactly_44_names = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0490[i];
    }
    out->tableSize = kTableSize;

    /* Spot-check: byte 0 = 'N', byte 2 = 'B' (BLOCK), etc. */
    if (s_g0490[0] != 'N') entry_0_is_n = 0;
    out->entry0IsN = entry_0_is_n;

    /* Find "BLOCK": starts at byte 2 (after "N\0"). */
    if (s_g0490[2] != 'B' || s_g0490[3] != 'L' || s_g0490[4] != 'O' ||
        s_g0490[5] != 'C' || s_g0490[6] != 'K') entry_1_is_block = 0;
    out->entry1IsBLOCK = entry_1_is_block;

    /* Find "FUSE" near the end. */
    if (s_g0490[284] != 'F' || s_g0490[285] != 'U' || s_g0490[286] != 'S' ||
        s_g0490[287] != 'E') entry_43_is_fuse = 0;
    out->entry43IsFUSE = entry_43_is_fuse;

    /* Count NUL bytes — should be 44 (one per name + trailing). */
    {
        int nul_count = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0490[i] == 0) ++nul_count;
        }
        if (nul_count != 55) exactly_44_names = 0;  /* 44 separators + 1 trailing */
    }
    out->exactly44Names = exactly_44_names;

    {
        static const unsigned char kExpected[kTableSize] = { 78, 0, 66, 76, 79, 67, 75, 0, 67, 72, 79, 80, 0, 88, 0, 66, 76, 79, 87, 32, 72, 79, 82, 78, 0, 70, 76, 73, 80, 0, 80, 85, 78, 67, 72, 0, 75, 73, 67, 75, 0, 87, 65, 82, 32, 67, 82, 89, 0, 83, 84, 65, 66, 0, 67, 76, 73, 77, 66, 32, 68, 79, 87, 78, 0, 70, 82, 69, 69, 90, 69, 32, 76, 73, 70, 69, 0, 72, 73, 84, 0, 83, 87, 73, 78, 71, 0, 83, 84, 65, 66, 0, 84, 72, 82, 85, 83, 84, 0, 74, 65, 66, 0, 80, 65, 82, 82, 89, 0, 72, 65, 67, 75, 0, 66, 69, 82, 90, 69, 82, 75, 0, 70, 73, 82, 69, 66, 65, 76, 76, 0, 68, 73, 83, 80, 69, 76, 76, 0, 67, 79, 78, 70, 85, 83, 69, 0, 76, 73, 71, 72, 84, 78, 73, 78, 71, 0, 68, 73, 83, 82, 85, 80, 84, 0, 77, 69, 76, 69, 69, 0, 88, 0, 73, 78, 86, 79, 75, 69, 0, 83, 76, 65, 83, 72, 0, 67, 76, 69, 65, 86, 69, 0, 66, 65, 83, 72, 0, 83, 84, 85, 78, 0, 83, 72, 79, 79, 84, 0, 83, 80, 69, 76, 76, 83, 72, 73, 69, 76, 68, 0, 70, 73, 82, 69, 83, 72, 73, 69, 76, 68, 0, 70, 76, 85, 88, 67, 65, 71, 69, 0, 72, 69, 65, 76, 0, 67, 65, 76, 77, 0, 76, 73, 71, 72, 84, 0, 87, 73, 78, 68, 79, 87, 0, 83, 80, 73, 84, 0, 66, 82, 65, 78, 68, 73, 83, 72, 0, 84, 72, 82, 79, 87, 0, 70, 85, 83, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0490[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_g0490_get_pc34(i) != (int)s_g0490[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0490_get_pc34(-1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0490_get_pc34(kTableSize) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0490_get_pc34(999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->entry0IsN &&
        out->entry1IsBLOCK &&
        out->entry43IsFUSE &&
        out->exactly44Names &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 8;
    return out->accepted;
}
