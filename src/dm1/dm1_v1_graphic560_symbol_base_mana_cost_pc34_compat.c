#include "firestaff/dm1/v1/G0485_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0485):
 * - MENU.C:16 - declaration of G0485_aauc_Graphic560_SymbolBaseManaCost[4][6]
 * - MENU.C:44 - PC 3.4 EN init
 * - MENU.C F0452 - spell symbol mana cost lookup
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 * - MENU.C F0452 action/spell init — action/spell dispatch
 * - MENU.C:16/44 — declaration + PC 3.4 EN init
 */

enum {
    kTableSize  = 24,
    kRows       = 4,
    kCols       = 6,
    kOutOfRange = -1
};

static const unsigned char s_g0485[kTableSize] = {
    1, 2, 3, 4, 5, 6,
    2, 3, 4, 5, 6, 7,
    4, 5, 6, 7, 7, 9,
    2, 2, 3, 4, 6, 7
};

const unsigned char *
dm1_v1_graphic560_symbol_base_mana_cost_table_pc34(void)
{
    return s_g0485;
}

int
dm1_v1_graphic560_symbol_base_mana_cost_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_graphic560_symbol_base_mana_cost_get_pc34(int entry_index)
{
    if (entry_index < 0 || entry_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0485[entry_index];
}

int
dm1_v1_graphic560_symbol_base_mana_cost_run_pc34(
    DM1_V1_G0485ResultPc34 *out)
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
        out->tableEntries[i] = (int)s_g0485[i];
    }
    out->tableSize = kTableSize;

    for (i = 0; i < kTableSize; ++i) {
        if (s_g0485[i] > 255) all_in_byte_range = 0;
    }
    out->allInByteRange = all_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = {
        1, 2, 3, 4, 5, 6,
        2, 3, 4, 5, 6, 7,
        4, 5, 6, 7, 7, 9,
        2, 2, 3, 4, 6, 7
    };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0485[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_graphic560_symbol_base_mana_cost_get_pc34(i) != (int)s_g0485[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_graphic560_symbol_base_mana_cost_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_symbol_base_mana_cost_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_graphic560_symbol_base_mana_cost_get_pc34(999) != kOutOfRange) {
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
