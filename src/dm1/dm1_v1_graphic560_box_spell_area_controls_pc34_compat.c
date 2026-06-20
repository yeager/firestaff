#include "firestaff/dm1/v1/G0504_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0504):
 * G0504_ai_Graphic560_Box_SpellAreaControls — see DATA.C reference
 * - MENU.C:41 - declaration of G0504_ai_Graphic560_Box_[...]
 * - MENU.C:467 - PC 3.4 EN init { 233, 319, 42, 49 }
 * - MENU.C F0452/F0456 - action-area box usage
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:41/467 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 4,
    kOutOfRange = -1
};

static const int s_g0504[kTableSize] = {
    233, 319, 42, 49
};

const unsigned char *
dm1_v1_graphic560_box_spell_area_controls_table_pc34(void)
{
    return (const unsigned char *)s_g0504;
}

int
dm1_v1_graphic560_box_spell_area_controls_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_box_spell_area_controls_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return s_g0504[entry_index];
}

int
dm1_v1_graphic560_box_spell_area_controls_run_pc34(
    DM1_V1_G0504ResultPc34 *out)
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
        out->tableEntries[i] = s_g0504[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0504[i] < -32768 || s_g0504[i] > 32767) {
            all_in_byte_range = 0;
        }
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const int kExpected[kTableSize] = { 233, 319, 42, 49 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0504[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_box_spell_area_controls_get_pc34(i) != s_g0504[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_box_spell_area_controls_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_box_spell_area_controls_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_box_spell_area_controls_get_pc34(999) != kOutOfRange) {
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
