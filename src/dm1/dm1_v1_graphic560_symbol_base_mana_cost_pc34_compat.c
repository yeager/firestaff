#include "firestaff/dm1/v1/G0485_pc34_compat.h"
#include "firestaff/dm1/v1/G0486_pc34_compat.h"

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
dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
    int symbol_step,
    int symbol_index,
    char power_symbol,
    DM1_V1_G0485SymbolManaCostPc34 *out)
{
    DM1_V1_G0485SymbolManaCostPc34 receipt;
    int base_index;
    int power_index;
    int multiplier;
    int cost;

    if (!out) {
        return 0;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.symbolStep = symbol_step;
    receipt.symbolIndex = symbol_index;
    receipt.powerSymbol = (int)(unsigned char)power_symbol;
    receipt.powerSymbolIndex = -1;
    receipt.sourceAnchorF0399 =
        "ReDMCSB SYMBOL.C F0399_MENUS_AddChampionSymbol: L1222/L1223 mana cost";
    receipt.sourceAnchorG0485 =
        "ReDMCSB MENU.C:16,44 G0485_aauc_Graphic560_SymbolBaseManaCost[4][6]";
    receipt.sourceAnchorG0486 =
        "ReDMCSB MENU.C:17,49 G0486_auc_Graphic560_SymbolManaCostMultiplier[6]";

    if (symbol_step < 0 ||
        symbol_step >= DM1_V1_G0485_PC34_SYMBOL_STEP_COUNT ||
        symbol_index < 0 ||
        symbol_index >= DM1_V1_G0485_PC34_SYMBOLS_PER_STEP) {
        *out = receipt;
        return 0;
    }

    base_index = (symbol_step * DM1_V1_G0485_PC34_SYMBOLS_PER_STEP) +
                 symbol_index;
    cost = dm1_v1_graphic560_symbol_base_mana_cost_get_pc34(base_index);
    if (cost < 0) {
        *out = receipt;
        return 0;
    }

    receipt.baseTableIndex = base_index;
    receipt.baseManaCost = cost;

    if (symbol_step > 0) {
        power_index = (int)(unsigned char)power_symbol - 96;
        receipt.requiresPowerMultiplier = 1;
        receipt.powerSymbolIndex = power_index;
        if (power_index < 0 ||
            power_index >= DM1_V1_G0485_PC34_SYMBOLS_PER_STEP) {
            *out = receipt;
            return 0;
        }

        multiplier =
            dm1_v1_graphic560_symbol_mana_cost_multiplier_get_pc34(
                power_index);
        if (multiplier < 0) {
            *out = receipt;
            return 0;
        }
        receipt.multiplier = multiplier;
        cost = (cost * multiplier) >> 3;
    }

    receipt.manaCost = cost;
    receipt.accepted = 1;
    *out = receipt;
    return 1;
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

    /* s_g0485 is the original unsigned-byte table. Its C type itself is the
     * range proof; comparing every byte with 255 is tautological and trips
     * strict -Wtype-limits builds. */
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
