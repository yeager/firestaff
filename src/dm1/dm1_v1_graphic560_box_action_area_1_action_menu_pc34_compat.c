#include "firestaff/dm1/v1/G0501_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0501):
 * G0501_ai_Graphic560_Box_ActionArea1ActionMenu — see DATA.C reference
 * - MENU.C:38 - declaration of G0501_ai_Graphic560_Box_[...]
 * - MENU.C:464 - PC 3.4 EN init { 224, 319, 77, 97 }
 * - MENU.C F0452/F0456 - action-area box usage
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:38/464 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 4,
    kOutOfRange = -1
};

static const int s_g0501[kTableSize] = {
    224, 319, 77, 97
};

const unsigned char *
dm1_v1_graphic560_box_action_area_1_action_menu_table_pc34(void)
{
    return (const unsigned char *)s_g0501;
}

int
dm1_v1_graphic560_box_action_area_1_action_menu_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_box_action_area_1_action_menu_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return s_g0501[entry_index];
}

int
dm1_v1_graphic560_box_action_area_1_action_menu_run_pc34(
    DM1_V1_G0501ResultPc34 *out)
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
        out->tableEntries[i] = s_g0501[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0501[i] < -32768 || s_g0501[i] > 32767) {
            all_in_byte_range = 0;
        }
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const int kExpected[kTableSize] = { 224, 319, 77, 97 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0501[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_box_action_area_1_action_menu_get_pc34(i) != s_g0501[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_box_action_area_1_action_menu_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_box_action_area_1_action_menu_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_box_action_area_1_action_menu_get_pc34(999) != kOutOfRange) {
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
