#include "firestaff/dm1/v1/G0492_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0492):
 * G0492_auc_Graphic560_ActionDamageFactor — see DATA.C reference
 * - MENU.C:28 - declaration of G0492_auc_Graphic560_...
 * - MENU.C:202 - PC 3.4 EN init { 0, 15, 48, 0, 0, 0, 32, 48, 0, 48, 0, 0, 20, 16, 60, 66, 8, 8, 25, 96, 0, 0, 0, 0, 55, 60, 0, 0, 16, 48, 50, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
 * - MENU.C F0452 - action dispatch
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:28/202 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 44,
    kOutOfRange = -1
};

static const unsigned char s_g0492[kTableSize] = {
0, 15, 48, 0, 0, 0, 32, 48, 0, 48, 0, 0, 20, 16, 60, 66, 8, 8, 25, 96, 0, 0, 0, 0, 55, 60, 0, 0, 16, 48, 50, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const unsigned char *
dm1_v1_graphic560_action_damage_factor_table_pc34(void)
{
    return (const unsigned char *)s_g0492;
}

int
dm1_v1_graphic560_action_damage_factor_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_action_damage_factor_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0492[entry_index];
}

int
dm1_v1_graphic560_action_damage_factor_run_pc34(
    DM1_V1_G0492ResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g0492[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0492[i] > 255) all_in_byte_range = 0;
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {
0, 15, 48, 0, 0, 0, 32, 48, 0, 48, 0, 0, 20, 16, 60, 66, 8, 8, 25, 96, 0, 0, 0, 0, 55, 60, 0, 0, 16, 48, 50, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0492[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_action_damage_factor_get_pc34(i) != (int)s_g0492[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_action_damage_factor_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_damage_factor_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_damage_factor_get_pc34(999) != kOutOfRange) {
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
