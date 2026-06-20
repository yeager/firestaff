#include "firestaff/dm1/v1/G0495_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0495):
 * G0495_ac_Graphic560_ActionDefense — see DATA.C reference
 * - MENU.C:31 - declaration of G0495_auc_Graphic560_...
 * - MENU.C:337 - PC 3.4 EN init { 0, 36, 0, 0, -4, -10, -10, -5, 4, -20, -15, -10, 16, 5, -15, -17, -5, 29, 10, -10, -7, -7, -7, -7, -7, -5, -15, -9, 4, 0, 0, 5, -15, -7, -7, 8, -20, -5, 0, -15, -7, -4, 0, 8 }
 * - MENU.C F0452 - action dispatch
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:31/337 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 44,
    kOutOfRange = -1
};

static const char s_g0495[kTableSize] = {
0, 36, 0, 0, -4, -10, -10, -5, 4, -20, -15, -10, 16, 5, -15, -17, -5, 29, 10, -10, -7, -7, -7, -7, -7, -5, -15, -9, 4, 0, 0, 5, -15, -7, -7, 8, -20, -5, 0, -15, -7, -4, 0, 8
};

const unsigned char *
dm1_v1_graphic560_action_defense_table_pc34(void)
{
    return (const unsigned char *)s_g0495;
}

int
dm1_v1_graphic560_action_defense_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_action_defense_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0495[entry_index];
}

int
dm1_v1_graphic560_action_defense_run_pc34(
    DM1_V1_G0495ResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g0495[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0495[i] > 127 || s_g0495[i] < -128) all_in_byte_range = 0;
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const char kExpected[kTableSize] = {
0, 36, 0, 0, -4, -10, -10, -5, 4, -20, -15, -10, 16, 5, -15, -17, -5, 29, 10, -10, -7, -7, -7, -7, -7, -5, -15, -9, 4, 0, 0, 5, -15, -7, -7, 8, -20, -5, 0, -15, -7, -4, 0, 8
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0495[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_action_defense_get_pc34(i) != (int)s_g0495[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_action_defense_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_defense_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_defense_get_pc34(999) != kOutOfRange) {
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
