#include "dm1_v1_spell_casting_pc34_compat.h"
#include "firestaff/dm1/v1/G0485_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void
check_int(const char *label, int actual, int expected)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d expected=%d\n",
                label, actual, expected);
    }
}

static void
check_true(const char *label, int condition)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        fprintf(stderr, "FAIL %s\n", label);
    }
}

static void
test_power_symbol_uses_g0485_only(void)
{
    DM1_V1_G0485SymbolManaCostPc34 receipt;

    check_int("power Lo accepted",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  DM1_SYMBOL_STEP_POWER, DM1_POWER_LO, '\0', &receipt),
              1);
    check_int("power base index", receipt.baseTableIndex, 0);
    check_int("power base cost", receipt.baseManaCost, 1);
    check_int("power does not multiply", receipt.requiresPowerMultiplier, 0);
    check_int("power mana cost", receipt.manaCost, 1);
    check_true("power source anchor", receipt.sourceAnchorF0399 != NULL);

    check_int("power Mon accepted",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  DM1_SYMBOL_STEP_POWER, DM1_POWER_MON, '\0', &receipt),
              1);
    check_int("power Mon base index", receipt.baseTableIndex, 5);
    check_int("power Mon cost", receipt.manaCost, 6);
}

static void
test_later_symbols_use_power_multiplier(void)
{
    DM1_V1_G0485SymbolManaCostPc34 receipt;
    char mon = dm1_encodeSymbol(DM1_SYMBOL_STEP_POWER, DM1_POWER_MON);
    char lo = dm1_encodeSymbol(DM1_SYMBOL_STEP_POWER, DM1_POWER_LO);

    check_int("Mon/Ful accepted",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  DM1_SYMBOL_STEP_ELEMENT, DM1_ELEM_FUL, mon, &receipt),
              1);
    check_int("Mon/Ful base index", receipt.baseTableIndex, 9);
    check_int("Mon/Ful base cost", receipt.baseManaCost, 5);
    check_int("Mon multiplier index", receipt.powerSymbolIndex, DM1_POWER_MON);
    check_int("Mon multiplier", receipt.multiplier, 28);
    check_int("Mon/Ful mana cost", receipt.manaCost, 17);

    check_int("Lo/Ra accepted",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  DM1_SYMBOL_STEP_ALIGN, DM1_ALIGN_RA, lo, &receipt),
              1);
    check_int("Lo/Ra base index", receipt.baseTableIndex, 22);
    check_int("Lo/Ra multiplier", receipt.multiplier, 8);
    check_int("Lo/Ra mana cost", receipt.manaCost, 6);
}

static void
test_rejects_corrupt_symbol_state_without_cost(void)
{
    DM1_V1_G0485SymbolManaCostPc34 receipt;

    memset(&receipt, 0x5a, sizeof(receipt));
    check_int("bad step rejected",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  DM1_SYMBOL_STEP_COUNT, 0, '`', &receipt),
              0);
    check_int("bad step accepted flag", receipt.accepted, 0);
    check_int("bad step cost", receipt.manaCost, 0);

    check_int("bad index rejected",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  DM1_SYMBOL_STEP_ELEMENT, DM1_SYMBOLS_PER_STEP, '`',
                  &receipt),
              0);
    check_int("bad index accepted flag", receipt.accepted, 0);
    check_int("bad index cost", receipt.manaCost, 0);

    check_int("bad power rejected",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  DM1_SYMBOL_STEP_ELEMENT, DM1_ELEM_YA, 'Z', &receipt),
              0);
    check_int("bad power accepted flag", receipt.accepted, 0);
    check_int("bad power index", receipt.powerSymbolIndex, -6);
    check_int("bad power cost", receipt.manaCost, 0);
}

static void
test_spell_input_consumes_same_receipt(void)
{
    DM1_SpellCastingState state;
    DM1_ChampionSpellStats stats;
    DM1_V1_G0485SymbolManaCostPc34 receipt;
    int before;

    dm1_spell_init(&state);
    memset(&stats, 0, sizeof(stats));
    stats.currentMana = 50;

    check_int("add Mon",
              dm1_spell_addSymbol(&state, 0, &stats, DM1_POWER_MON), 1);
    check_int("Mon char", state.input[0].symbols[0],
              dm1_encodeSymbol(DM1_SYMBOL_STEP_POWER, DM1_POWER_MON));
    check_int("Mon mana", stats.currentMana, 44);

    before = stats.currentMana;
    check_int("receipt Mon/Ful",
              dm1_v1_graphic560_symbol_mana_cost_f0399_pc34(
                  state.input[0].symbolStep, DM1_ELEM_FUL,
                  state.input[0].symbols[0], &receipt),
              1);
    check_int("spell symbol mana cost",
              dm1_spell_symbolManaCost(&state, 0, DM1_ELEM_FUL),
              receipt.manaCost);
    check_int("add Ful",
              dm1_spell_addSymbol(&state, 0, &stats, DM1_ELEM_FUL), 1);
    check_int("Ful deducted", stats.currentMana, before - receipt.manaCost);
    check_int("Ful char", state.input[0].symbols[1],
              dm1_encodeSymbol(DM1_SYMBOL_STEP_ELEMENT, DM1_ELEM_FUL));
}

static void
test_spell_input_rejects_corrupt_power_without_mutation(void)
{
    DM1_SpellCastingState state;
    DM1_ChampionSpellStats stats;

    dm1_spell_init(&state);
    memset(&stats, 0, sizeof(stats));
    stats.currentMana = 50;
    state.input[0].symbolStep = DM1_SYMBOL_STEP_ELEMENT;
    state.input[0].symbols[0] = 'Z';
    state.input[0].symbols[1] = '\0';

    check_int("corrupt spell cost",
              dm1_spell_symbolManaCost(&state, 0, DM1_ELEM_YA), 0);
    check_int("corrupt add rejected",
              dm1_spell_addSymbol(&state, 0, &stats, DM1_ELEM_YA), 0);
    check_int("corrupt mana unchanged", stats.currentMana, 50);
    check_int("corrupt symbol unchanged", state.input[0].symbols[0], 'Z');
    check_int("corrupt step unchanged", state.input[0].symbolStep,
              DM1_SYMBOL_STEP_ELEMENT);
}

int
main(void)
{
    test_power_symbol_uses_g0485_only();
    test_later_symbols_use_power_multiplier();
    test_rejects_corrupt_symbol_state_without_cost();
    test_spell_input_consumes_same_receipt();
    test_spell_input_rejects_corrupt_power_without_mutation();

    printf("dm1_v1_g0485_g0486_f0399_symbol_mana_cost: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
