#include "firestaff/dm1/v1/G0494_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0494):
 * G0494_auc_Graphic560_ActionStamina — see DATA.C reference
 * - MENU.C:30 - declaration of G0494_auc_Graphic560_...
 * - MENU.C:292 - PC 3.4 EN init { 0, 4, 10, 0, 1, 0, 1, 3, 1, 3, 40, 3, 3, 2, 4, 17, 3, 1, 6, 40, 5, 2, 2, 4, 5, 25, 1, 2, 2, 10, 9, 2, 3, 1, 2, 6, 1, 1, 3, 2, 3, 2, 0, 2 }
 * - MENU.C F0452 - action dispatch
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:30/292 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 44,
    kOutOfRange = -1
};

static const unsigned char s_g0494[kTableSize] = {
0, 4, 10, 0, 1, 0, 1, 3, 1, 3, 40, 3, 3, 2, 4, 17, 3, 1, 6, 40, 5, 2, 2, 4, 5, 25, 1, 2, 2, 10, 9, 2, 3, 1, 2, 6, 1, 1, 3, 2, 3, 2, 0, 2
};

const unsigned char *
dm1_v1_graphic560_action_stamina_table_pc34(void)
{
    return (const unsigned char *)s_g0494;
}

int
dm1_v1_graphic560_action_stamina_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_action_stamina_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0494[entry_index];
}

int
dm1_v1_graphic560_action_stamina_run_pc34(
    DM1_V1_G0494ResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g0494[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0494[i] > 255) all_in_byte_range = 0;
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {
0, 4, 10, 0, 1, 0, 1, 3, 1, 3, 40, 3, 3, 2, 4, 17, 3, 1, 6, 40, 5, 2, 2, 4, 5, 25, 1, 2, 2, 10, 9, 2, 3, 1, 2, 6, 1, 1, 3, 2, 3, 2, 0, 2
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0494[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_action_stamina_get_pc34(i) != (int)s_g0494[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_action_stamina_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_stamina_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_action_stamina_get_pc34(999) != kOutOfRange) {
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
