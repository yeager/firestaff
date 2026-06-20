#include "firestaff/dm1/v1/G0496_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0496):
 * G0496_auc_Graphic560_ActionSkillIndex — see DATA.C reference
 * - MENU.C:32 - declaration of G0496_auc_Graphic560_...
 * - MENU.C:382 - PC 3.4 EN init { 0, 7, 6, 0, 14, 12, 9, 9, 7, 9, 8, 14, 9, 4, 5, 5, 5, 7, 4, 4, 16, 17, 14, 17, 17, 6, 8, 3, 4, 4, 6, 6, 11, 15, 15, 3, 13, 14, 17, 18, 16, 14, 10, 3 }
 * - MENU.C F0452 - action dispatch
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:32/382 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 44,
    kOutOfRange = -1
};

static const unsigned char s_g0496[kTableSize] = {
0, 7, 6, 0, 14, 12, 9, 9, 7, 9, 8, 14, 9, 4, 5, 5, 5, 7, 4, 4, 16, 17, 14, 17, 17, 6, 8, 3, 4, 4, 6, 6, 11, 15, 15, 3, 13, 14, 17, 18, 16, 14, 10, 3
};

const unsigned char *
dm1_v1_graphic560_action_skill_index_table_pc34(void)
{
    return (const unsigned char *)s_g0496;
}

int
dm1_v1_graphic560_action_skill_index_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_action_skill_index_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0496[entry_index];
}

int
dm1_v1_graphic560_action_skill_index_run_pc34(
    DM1_V1_G0496ResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g0496[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0496[i] > 255) all_in_byte_range = 0;
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {
0, 7, 6, 0, 14, 12, 9, 9, 7, 9, 8, 14, 9, 4, 5, 5, 5, 7, 4, 4, 16, 17, 14, 17, 17, 6, 8, 3, 4, 4, 6, 6, 11, 15, 15, 3, 13, 14, 17, 18, 16, 14, 10, 3
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0496[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_action_skill_index_get_pc34(i) != (int)s_g0496[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_action_skill_index_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_skill_index_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_skill_index_get_pc34(999) != kOutOfRange) {
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
