#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_spell_area_overlay_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d expected=%d\n",
                label, actual, expected);
    }
}

static void check_contains(const char *label, const char *text,
                           const char *needle)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        ++g_failures;
        fprintf(stderr, "FAIL %s missing %s\n",
                label, needle ? needle : "(null)");
    }
}

static void reset_input(DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 *in)
{
    int i;
    memset(in, 0, sizeof(*in));
    in->previous_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
    in->requested_caster_index = 1;
    in->party_champion_count = DM1_V1_CPSAO_CHAMPION_COUNT_PC34;
    in->symbol_step = 2u;
    for (i = 0; i < DM1_V1_CPSAO_CHAMPION_COUNT_PC34; ++i) {
        in->champions[i].index = i;
        in->champions[i].current_health = 100;
        in->champions[i].symbols[0] = '\0';
    }
    in->champions[1].symbols[0] = (char)110;
    in->champions[1].symbols[1] = (char)111;
    in->champions[1].symbols[2] = '\0';
}

static void test_contract_evidence_names_f0396_f0397_f0398(void)
{
    const DM1_V1_ChampionPanelSpellAreaOverlayPc34Contract *contract =
        dm1_v1_champion_panel_spell_area_overlay_contract_pc34();
    const char *evidence =
        dm1_v1_champion_panel_spell_area_overlay_source_evidence_pc34();

    check_int("contract lines graphic id", contract->gfx_spell_area_lines, 11);
    check_contains("lines anchor", contract->lines_bitmap_anchor,
                   "F0396_MENUS_LoadSpellAreaLinesBitmap");
    check_contains("available anchor", contract->available_symbols_anchor,
                   "F0397_MENUS_DrawAvailableSymbols");
    check_contains("champion anchor", contract->champion_symbols_anchor,
                   "F0398_MENUS_DrawChampionSymbols");
    check_contains("evidence lines stack", evidence,
                   "F0396 loads C011_GRAPHIC_MENU_SPELL_AREA_LINES");
    check_contains("evidence available", evidence, "F0397 emits 6 chars");
    check_contains("evidence champion", evidence, "F0398 emits up to 4 chars");
}

static void test_live_caster_draws_line_bitmap_and_symbols(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;

    reset_input(&in);
    check_int("plan ok",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1);
    check_int("line bitmap drawn", plan.drew_lines_bitmap, 1);
    check_int("available symbols drawn", plan.drew_available_symbols, 1);
    check_int("champion symbols drawn", plan.drew_champion_symbols, 1);
    check_int("available count", plan.available_symbol_count, 6);
    check_int("step 2 first available char", plan.line2[0].character, 108);
    check_int("step 2 last available char", plan.line2[5].character, 113);
    check_int("first typed symbol", plan.line3[0].drawn_character, 110);
    check_int("second typed symbol", plan.line3[1].drawn_character, 111);
    check_int("first padded symbol", plan.line3[2].drawn_character, ' ');
    check_int("second padded symbol", plan.line3[3].drawn_character, ' ');
}

static void test_rejects_do_not_draw_symbol_stack(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;

    reset_input(&in);
    in.previous_caster_index = 1;
    in.requested_caster_index = 1;
    check_int("same caster ok",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1);
    check_int("same caster reason", plan.reject_reason,
              DM1_V1_CPSAO_REJECT_SAME_CASTER_PC34);
    check_int("same caster no F0396", plan.drew_lines_bitmap, 0);
    check_int("same caster no F0397", plan.drew_available_symbols, 0);
    check_int("same caster no F0398", plan.drew_champion_symbols, 0);

    reset_input(&in);
    in.champions[1].current_health = 0;
    check_int("dead caster ok",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1);
    check_int("dead caster reason", plan.reject_reason,
              DM1_V1_CPSAO_REJECT_DEAD_CHAMPION_PC34);
    check_int("dead caster no F0396", plan.drew_lines_bitmap, 0);
    check_int("dead caster no F0397", plan.drew_available_symbols, 0);
    check_int("dead caster no F0398", plan.drew_champion_symbols, 0);

    reset_input(&in);
    in.previous_caster_index = 1;
    in.requested_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
    check_int("none caster ok",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1);
    check_int("none caster clears black", plan.cleared_to_black, 1);
    check_int("none caster no F0396", plan.drew_lines_bitmap, 0);
    check_int("none caster no F0397", plan.drew_available_symbols, 0);
    check_int("none caster no F0398", plan.drew_champion_symbols, 0);
}

static void reset_spell_stats(DM1_ChampionSpellStats *stats)
{
    memset(stats, 0, sizeof(*stats));
    stats->currentHealth = 100;
    stats->currentMana = 50;
    stats->maximumMana = 80;
    stats->wisdom = 90;
    memset(stats->skillLevels, 12, sizeof(stats->skillLevels));
}

static void test_f0412_cast_result_receipt_owns_hud_refresh(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_ChampionSpellStats stats;
    DM1_SpellF0412RuntimeReceipt receipt;
    DM1_V1_ChampionPanelSpellAreaCastResultRefreshPc34 refresh;

    reset_input(&in);
    in.previous_caster_index = 1;
    in.requested_caster_index = 1;
    in.symbol_step = 2u;
    in.champions[1].symbols[0] = dm1_encodeSymbol(DM1_SYMBOL_STEP_POWER,
                                                  DM1_POWER_MON);
    in.champions[1].symbols[1] = dm1_encodeSymbol(DM1_SYMBOL_STEP_ELEMENT,
                                                  DM1_ELEM_FUL);
    in.champions[1].symbols[2] = dm1_encodeSymbol(DM1_SYMBOL_STEP_CLASS,
                                                  DM1_CLASS_IR);
    in.champions[1].symbols[3] = '\0';

    reset_spell_stats(&stats);
    check_int("F0412 fireball receipt",
              dm1_spell_f0412RuntimeReceiptForTableIndex(
                  8, DM1_POWER_MON + 1, 1, &stats, 0x0004, 3, 1, 0,
                  &receipt),
              1);
    check_int("F0412 fireball success", receipt.castResult,
              DM1_SPELL_CAST_SUCCESS);
    check_int("F0412 success clears symbols", receipt.symbolsCleared, 1);
    check_int("cast result refresh ok",
              dm1_v1_champion_panel_spell_area_cast_result_refresh_pc34(
                  &in, &receipt, &refresh),
              1);
    check_int("refresh consumes F0412", refresh.consumed_f0412_receipt, 1);
    check_int("success redraws F0397", refresh.redrew_available_symbols, 1);
    check_int("success redraws F0398", refresh.redrew_champion_symbols, 1);
    check_int("success line2 reset first rune",
              refresh.overlay_plan.line2[0].character, 96);
    check_int("success line3 blank 0",
              refresh.overlay_plan.line3[0].drawn_character, ' ');
    check_int("success no F0396", refresh.overlay_plan.drew_lines_bitmap, 0);
    check_int("success no F0393",
              refresh.overlay_plan.drew_spell_area_controls, 0);

    reset_spell_stats(&stats);
    check_int("F0412 potion receipt",
              dm1_spell_f0412RuntimeReceiptForTableIndex(
                  9, DM1_POWER_MON + 1, 1, &stats, 0x0004, 0, 0, 0,
                  &receipt),
              1);
    check_int("F0412 potion needs flask", receipt.castResult,
              DM1_SPELL_CAST_FAILURE_NEEDS_FLASK);
    check_int("F0412 needs flask keeps symbols", receipt.symbolsCleared, 0);
    check_int("needs flask refresh ok",
              dm1_v1_champion_panel_spell_area_cast_result_refresh_pc34(
                  &in, &receipt, &refresh),
              1);
    check_int("needs flask no F0397", refresh.redrew_available_symbols, 0);
    check_int("needs flask no F0398", refresh.redrew_champion_symbols, 0);
    check_int("needs flask retained flag",
              refresh.retained_symbols_for_needs_hand_item, 1);
    check_int("needs flask retained first symbol",
              refresh.overlay_plan.new_caster_symbols[0],
              dm1_encodeSymbol(DM1_SYMBOL_STEP_POWER, DM1_POWER_MON));
}

int main(void)
{
    test_contract_evidence_names_f0396_f0397_f0398();
    test_live_caster_draws_line_bitmap_and_symbols();
    test_rejects_do_not_draw_symbol_stack();
    test_f0412_cast_result_receipt_owns_hud_refresh();
    printf("dm1_v1_spell_area_f0396_f0398_symbol_audit: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
