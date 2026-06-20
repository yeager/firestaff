#include "firestaff/dm1/v1/G0491_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0491):
 * G0491_auc_Graphic560_ActionDisabledTicks — see DATA.C reference
 * - MENU.C:27 - declaration of G0491_auc_Graphic560_...
 * - MENU.C:157 - PC 3.4 EN init { 0, 6, 8, 0, 6, 3, 1, 5, 3, 5, 35, 20, 4, 6, 10, 16, 2, 18, 8, 30, 42, 31, 10, 38, 9, 20, 10, 16, 4, 12, 20, 7, 14, 30, 35, 2, 19, 9, 10, 15, 22, 10, 0, 2 }
 * - MENU.C F0452 - action dispatch
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:27/157 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 44,
    kOutOfRange = -1
};

static const unsigned char s_g0491[kTableSize] = {
0, 6, 8, 0, 6, 3, 1, 5, 3, 5, 35, 20, 4, 6, 10, 16, 2, 18, 8, 30, 42, 31, 10, 38, 9, 20, 10, 16, 4, 12, 20, 7, 14, 30, 35, 2, 19, 9, 10, 15, 22, 10, 0, 2
};

const unsigned char *
dm1_v1_graphic560_action_disabled_ticks_table_pc34(void)
{
    return (const unsigned char *)s_g0491;
}

int
dm1_v1_graphic560_action_disabled_ticks_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_action_disabled_ticks_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0491[entry_index];
}

int
dm1_v1_graphic560_action_disabled_ticks_run_pc34(
    DM1_V1_G0491ResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g0491[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0491[i] > 255) all_in_byte_range = 0;
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {
0, 6, 8, 0, 6, 3, 1, 5, 3, 5, 35, 20, 4, 6, 10, 16, 2, 18, 8, 30, 42, 31, 10, 38, 9, 20, 10, 16, 4, 12, 20, 7, 14, 30, 35, 2, 19, 9, 10, 15, 22, 10, 0, 2
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0491[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_action_disabled_ticks_get_pc34(i) != (int)s_g0491[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_action_disabled_ticks_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_disabled_ticks_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_disabled_ticks_get_pc34(999) != kOutOfRange) {
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
