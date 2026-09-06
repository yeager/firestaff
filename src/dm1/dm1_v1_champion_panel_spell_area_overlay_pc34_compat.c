#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_spell_area_overlay_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 champion-panel spell-area overlay draw contract implementation.
 *
 * Contract-only: no GRAPHICS.DAT or real screen access. Computes the
 * deterministic draw plan that the ReDMCSB WIP20210206 PC 3.4 path
 * (CASTER.C F0394 + SPELDRAW.C F0393 + MENUDRAW.C F0397/F0398)
 * would emit given a fixed {previous_caster, requested_caster,
 * party_champion_count, symbol_step, 4-champion party with typed
 * Symbols[]} input. The existing spell_area_routes gate covers the
 * input dispatch (C100..C109 -> F0370) and the dm1_v1_menu_render gate
 * covers the orchestrator flag; this gate covers the *draw* contract
 * that the BIOS writes to the screen box {224, 319, 42, 74} every time
 * the caster identity or symbol buffer changes.
 *
 * The plan mirrors the ReDMCSB code path:
 *   1. F0394 short-circuits when the requested caster equals the previous
 *      caster, including CM1_CHAMPION_NONE (REJECT_SAME_CASTER).
 *   2. F0394 rejects a non-NONE caster with CurrentHealth == 0
 *      (REJECT_DEAD_CHAMPION).
 *   3. On a NONE caster F0394 clears the spell-area box to black and
 *      sets G0514_i_MagicCasterChampionIndex = CM1_CHAMPION_NONE
 *      (cleared_to_black = 1, no lines drawn).
 *   4. Otherwise F0394 sets G0514 to the new caster, paints the
 *      C009 background graphic on every accepted caster change, calls
 *      F0393 (line 1), F0397 (line 2), and F0398 (line 3). The early
 *      MEDIA009 F0396/F0392 C011 path is not part of I34.
 *
 * F0393 paints 0..4 tab highlight boxes, gated per tab by
 * Champions[i].CurrentHealth > 0 and i < party_champion_count.
 *
 * F0397 emits 6 characters starting at ASCII 96 + 6*SymbolStep
 * (C00..C05 rune alphabet window), cyan on black, at
 * (239 + 14*i, 58) for i in 0..5. SymbolStep is the F0399/F0400
 * champion-owned 0..3 ring; rows 4 and 5 are not source-owned.
 *
 * F0398 emits up to 4 characters from the new caster's Symbols[],
 * space-padding the tail. X = 241 + 9*i, y = 70, cyan on black.
 *
 * Tab geometry is selected by caster*5+C224 in I34E item696. The selected
 * caster's tab is 45x8; each other living champion receives a 12x7 tab.
 */

static int safe_strlen4(const char *s)
{
    int n = 0;
    if (!s) return 0;
    while (n < DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34 && s[n] != '\0') {
        ++n;
    }
    return n;
}

static const DM1_V1_ChampionPanelSpellAreaOverlayPc34Contract s_contract = {
    1,
    DM1_V1_CPSAO_CHAMPION_COUNT_PC34,
    DM1_V1_CPSAO_SPELL_AREA_X0_PC34,
    DM1_V1_CPSAO_SPELL_AREA_X1_PC34,
    DM1_V1_CPSAO_SPELL_AREA_Y0_PC34,
    DM1_V1_CPSAO_SPELL_AREA_Y1_PC34,
    DM1_V1_CPSAO_SPELL_AREA_WIDTH_PC34,
    DM1_V1_CPSAO_SPELL_AREA_HEIGHT_PC34,
    DM1_V1_CPSAO_LINE2_X0_PC34,
    DM1_V1_CPSAO_LINE2_X1_PC34,
    DM1_V1_CPSAO_LINE2_Y0_PC34,
    DM1_V1_CPSAO_LINE2_Y1_PC34,
    DM1_V1_CPSAO_LINE3_X0_PC34,
    DM1_V1_CPSAO_LINE3_X1_PC34,
    DM1_V1_CPSAO_LINE3_Y0_PC34,
    DM1_V1_CPSAO_LINE3_Y1_PC34,
    DM1_V1_CPSAO_AVAILABLE_SYMBOL_COUNT_PC34,
    DM1_V1_CPSAO_AVAILABLE_SYMBOL_BASE_PC34,
    DM1_V1_CPSAO_AVAILABLE_SYMBOL_STEP_PC34,
    DM1_V1_CPSAO_AVAILABLE_SYMBOL_X0_PC34,
    DM1_V1_CPSAO_AVAILABLE_SYMBOL_Y_PC34,
    DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34,
    DM1_V1_CPSAO_CHAMPION_SYMBOL_STEP_PC34,
    DM1_V1_CPSAO_CHAMPION_SYMBOL_X0_PC34,
    DM1_V1_CPSAO_CHAMPION_SYMBOL_Y_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_0_X0_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_0_X1_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_1_X0_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_1_X1_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_2_X0_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_2_X1_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_3_X0_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_3_X1_PC34,
    DM1_V1_CPSAO_TAB_Y0_PC34,
    DM1_V1_CPSAO_TAB_CHAMPION_0_Y1_PC34,
    DM1_V1_CPSAO_TAB_OTHER_Y1_PC34,
    DM1_V1_CPSAO_COLOR_CYAN_PC34,
    DM1_V1_CPSAO_COLOR_BLACK_PC34,
    DM1_V1_CPSAO_CHAMPION_NONE_PC34,
    DM1_V1_CPSAO_GFX_SPELL_AREA_BACKGROUND_PC34,
    DM1_V1_CPSAO_GFX_SPELL_AREA_LINES_PC34,
    DM1_V1_CPSAO_ZONE_SPELL_AREA_PC34,
    DM1_V1_CPSAO_CMD_CLICK_SPELL_AREA_PC34,
    DM1_V1_CPSAO_CMD_SET_CASTER_PC34,
    DM1_V1_CPSAO_CMD_CAST_PC34,
    DM1_V1_CPSAO_CMD_RECANT_PC34,
    DM1_V1_CPSAO_CMD_RUNE_FIRST_PC34,
    DM1_V1_CPSAO_CMD_RUNE_LAST_PC34,
    "CASTER.C:18-33 F0394_MENUS_SetMagicCasterAndDrawSpellArea short-circuit "
        "and CM1_CHAMPION_NONE clear path",
    "SPELDRAW.C:2-90 F0393_MENUS_DrawSpellAreaControls 4-tab x0 columns and "
        "CurrentHealth / PartyChampionCount gate",
    "MENUDRAW.C:47-80 F0397_MENUS_DrawAvailableSymbols 6-symbol window "
        "starting at 96+6*SymbolStep",
    "MENUDRAW.C:83-117 F0398_MENUS_DrawChampionSymbols strlen(Symbols) fill "
        "and space-pad tail",
    "MENUDRAW.C:31-45 F0396 C011 is MEDIA009 only; I34 CASTER.C:89-93 uses C009",
    "DATA.C:119 G0000_ai_Graphic562_Box_SpellArea={224,319,42,74}; "
        "DATA.C:530-531 G1072_ai_Box_SpellAreaLine2/Line3",
    "DEFS.H C04/C00 colors, CM1_CHAMPION_NONE sentinel, C009/C011 gfx ids, "
        "C013_ZONE_SPELL_AREA, C100..C109 command ids",
    "REVIVE.C:292/845/931 resurrect/reincarnate F0393 refresh; "
        "CHAMPION.C:1681 party direction rotation F0393 refresh; "
        "MENU.C:1657 F0412 spell cast result F0397+F0398 refresh; "
        "SYMBOL.C:62-63/102-103 symbol step change refresh"
};

static const char s_source_evidence[] =
    "contract_only=1; CASTER.C:18-21 F0394 short-circuits when "
    "requested_caster_index == G0514_i_MagicCasterChampionIndex and rejects "
    "non-NONE with !CurrentHealth; COMMAND.C/CLIKMENU.C C109 can only "
    "select a party tab already admitted by G0305_ui_PartyChampionCount, so "
    "a skipped out-of-party tab cannot publish C009/C011 spell material; "
    "CASTER.C:23-33 F0394 paints the "
    "C009_GRAPHIC_MENU_SPELL_AREA_LINES (87x25) into C013 at233,50 on "
    "every accepted caster change in the I34 branch at CASTER.C:89-93; "
    "CASTER.C:28-32 F0394 CM1_CHAMPION_NONE path sets "
    "G0514_i_MagicCasterChampionIndex = CM1_CHAMPION_NONE and "
    "M524_FillScreenBox(..., C00_COLOR_BLACK). SPELDRAW.C:87-94 F0393 "
    "selects item696 zones C224+5*caster: the selected caster tab is 45x8, "
    "all other tabs are 12x7, with y0=42. Caster identity, not leader or "
    "inventory identity, chooses all four tab positions. "
    "F0393 only highlights champion n if Champion[n].CurrentHealth > 0 and "
    "G0305_ui_PartyChampionCount > n. MENUDRAW.C:47-80 F0397 emits 6 chars "
    "starting at ASCII 96 + 6*SymbolStep in C04_COLOR_CYAN on C00_COLOR_BLACK "
    "at screen x = 239 + 14*i, y = 58; SYMBOL.C F0399:39 and F0400 keep "
    "Champion.SymbolStep in the 0..3 ring, so rows 4/5 are not source-owned. "
    "MENUDRAW.C:83-117 F0398 emits up to "
    "4 chars from Champion->Symbols[0..N-1] (N = strlen clamped to 4) with "
    "space-pad tail, screen x = 241 + 9*i, y = 70, cyan on black. "
    "MENUDRAW.C:31-45 F0396 is excluded from I34; early MEDIA009 loads C011 into "
    "the 3-row stack bitmap used by F0392/F0394. DATA.C:119 pins the "
    "spell-area screen box {224, 319, 42, 74} and DATA.C:530-531 pins "
    "G1072_ai_Box_SpellAreaLine2={224,319,50,61} and "
    "G1073_ai_Box_SpellAreaLine3={224,319,62,73}. The same-caster reject "
    "is the gate that lets REVIVE.C:292/845/931 / CHAMPION.C:1681 / "
    "MENU.C:1657 / SYMBOL.C:62-63,102-103 reissue F0393 without retriggering "
    "F0394; the dead-champion reject is the gate that keeps the spell area "
    "stable while one of the four slots is down. The CM1_CHAMPION_NONE clear "
    "path is the only place where F0394 writes C00_COLOR_BLACK to the full "
    "spell-area box (ENDGAME.C:1010 reuses the same fill for the closing "
    "scene).";

const DM1_V1_ChampionPanelSpellAreaOverlayPc34Contract *
dm1_v1_champion_panel_spell_area_overlay_contract_pc34(void)
{
    return &s_contract;
}

const char *
dm1_v1_champion_panel_spell_area_overlay_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void plan_reject(
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 *out,
    DM1_V1_CpsaoRejectReasonPc34 reason,
    int previous_caster)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->reject_reason = reason;
    out->post_caster_index = previous_caster;
    out->cleared_to_black = 0;
    out->drew_background_graphic = 0;
    out->drew_lines_bitmap = 0;
    out->drew_spell_area_controls = 0;
    out->drew_available_symbols = 0;
    out->drew_champion_symbols = 0;
}

static int fill_line1(
    const DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 *input,
    int active_caster,
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 *out)
{
    /* Authentic I34E GRAPHICS.DAT SHA256 2c3aa836925c64c09402bafb03c6459
     * 32bd03c4f003ad9a86542383b078ecf8e, item696 C224..C243. All records
     * match data/zones_h_reconstruction.json. COORD.C:337-360 defines
     * parent222=45x8 and parent223=12x7; F0393 selects caster*5+224. */
    static const int tabX[4][4] = {
        {233, 280, 294, 308}, {233, 247, 294, 308},
        {233, 247, 261, 308}, {233, 247, 261, 275}
    };
    int i;
    for (i = 0; i < DM1_V1_CPSAO_CHAMPION_COUNT_PC34; ++i) {
        DM1_V1_ChampionPanelSpellAreaOverlayLine1Pc34 *row =
            &out->line1[i];
        row->champion_index = i;
        row->tab_x0 = tabX[active_caster][i];
        row->tab_x1 = row->tab_x0 + (i == active_caster ? 44 : 11);
        row->tab_y0 = DM1_V1_CPSAO_TAB_Y0_PC34;
        row->tab_y1 = i == active_caster ? 49 : 48;
        row->highlighted = 0;
        row->present = 0;
        if (i >= input->party_champion_count) continue;
        if (input->champions[i].current_health <= 0) continue;
        row->present = 1;
        /* SPELDRAW.C F0393:87-94 (I34E/I34M): invert every living
         * champion's zone, not just the expanded selected-caster tab. */
        row->highlighted = 1;
        ++out->tab_count;
    }
    return out->tab_count;
}

static void fill_line2(
    unsigned int symbol_step,
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 *out)
{
    int i;
    for (i = 0; i < DM1_V1_CPSAO_AVAILABLE_SYMBOL_COUNT_PC34; ++i) {
        DM1_V1_ChampionPanelSpellAreaOverlayLine2Pc34 *row =
            &out->line2[i];
        row->symbol_index = i;
        row->screen_x =
            DM1_V1_CPSAO_AVAILABLE_SYMBOL_X0_PC34 +
            DM1_V1_CPSAO_AVAILABLE_SYMBOL_STEP_PC34 * i;
        row->screen_y = DM1_V1_CPSAO_AVAILABLE_SYMBOL_Y_PC34;
        row->character =
            DM1_V1_CPSAO_AVAILABLE_SYMBOL_BASE_PC34 +
            (int)(DM1_V1_CPSAO_AVAILABLE_SYMBOL_COUNT_PC34 * symbol_step) + i;
        row->color_cyan = DM1_V1_CPSAO_COLOR_CYAN_PC34;
        row->color_black = DM1_V1_CPSAO_COLOR_BLACK_PC34;
    }
    out->available_symbol_count =
        DM1_V1_CPSAO_AVAILABLE_SYMBOL_COUNT_PC34;
}

static void fill_line3(
    const DM1_V1_ChampionPanelSpellAreaOverlayChampionPc34 *champion,
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 *out)
{
    int i;
    int len = safe_strlen4(champion->symbols);
    for (i = 0; i < DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34; ++i) {
        DM1_V1_ChampionPanelSpellAreaOverlayLine3Pc34 *row =
            &out->line3[i];
        row->slot_index = i;
        row->screen_x =
            DM1_V1_CPSAO_CHAMPION_SYMBOL_X0_PC34 +
            DM1_V1_CPSAO_CHAMPION_SYMBOL_STEP_PC34 * i;
        row->screen_y = DM1_V1_CPSAO_CHAMPION_SYMBOL_Y_PC34;
        if (i < len) {
            row->source_symbol = champion->symbols[i];
            row->drawn_character = champion->symbols[i];
            row->from_champion_symbols = 1;
        } else {
            row->source_symbol = 0;
            row->drawn_character = ' ';
            row->from_champion_symbols = 0;
        }
    }
    out->champion_symbol_count = DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34;
    out->new_caster_symbols_length = len;
    memcpy(out->new_caster_symbols, champion->symbols,
           (size_t)len);
    out->new_caster_symbols[len] = '\0';
}

int dm1_v1_champion_panel_spell_area_overlay_plan_pc34(
    const DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 *input,
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 *out_plan)
{
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;
    int active_caster;

    if (!input || !out_plan) return 0;
    if (input->party_champion_count < 0 ||
        input->party_champion_count > DM1_V1_CPSAO_CHAMPION_COUNT_PC34) {
        return 0;
    }
    if (input->requested_caster_index < DM1_V1_CPSAO_CHAMPION_NONE_PC34 ||
        input->requested_caster_index >= DM1_V1_CPSAO_CHAMPION_COUNT_PC34) {
        return 0;
    }
    if (input->previous_caster_index < DM1_V1_CPSAO_CHAMPION_NONE_PC34 ||
        input->previous_caster_index >= DM1_V1_CPSAO_CHAMPION_COUNT_PC34) {
        return 0;
    }
    if (input->symbol_step >= DM1_V1_CPSAO_SYMBOL_STEP_COUNT_PC34) {
        return 0;
    }

    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;

    /* CASTER.C:18-21 F0394 same-caster short-circuit. */
    if (input->requested_caster_index == input->previous_caster_index) {
        plan_reject(&plan, DM1_V1_CPSAO_REJECT_SAME_CASTER_PC34,
                    input->previous_caster_index);
        *out_plan = plan;
        return 1;
    }

    /* C109 can only select a live party slot; skipped tabs do not materialize. */
    if (input->requested_caster_index != DM1_V1_CPSAO_CHAMPION_NONE_PC34 &&
        input->requested_caster_index >= input->party_champion_count) {
        plan_reject(&plan, DM1_V1_CPSAO_REJECT_OUT_OF_PARTY_PC34,
                    input->previous_caster_index);
        *out_plan = plan;
        return 1;
    }

    /* CASTER.C:18-21 F0394 dead-champion reject. */
    if (input->requested_caster_index != DM1_V1_CPSAO_CHAMPION_NONE_PC34 &&
        input->champions[input->requested_caster_index].current_health <= 0) {
        plan_reject(&plan, DM1_V1_CPSAO_REJECT_DEAD_CHAMPION_PC34,
                    input->previous_caster_index);
        *out_plan = plan;
        return 1;
    }

    if (input->requested_caster_index == DM1_V1_CPSAO_CHAMPION_NONE_PC34) {
        /*
         * CASTER.C:28-32 F0394 NONE clear path: writes
         * G0000_ai_Graphic562_Box_SpellArea with C00_COLOR_BLACK and
         * sets G0514 to CM1_CHAMPION_NONE. ENDGAME.C:1010 reuses the
         * same fill for the closing scene.
         */
        plan.post_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
        plan.cleared_to_black = 1;
        plan.drew_background_graphic = 0;
        plan.drew_lines_bitmap = 0;
        plan.drew_spell_area_controls = 0;
        plan.drew_available_symbols = 0;
        plan.drew_champion_symbols = 0;
        plan.tab_count = 0;
        plan.available_symbol_count = 0;
        plan.champion_symbol_count = 0;
        plan.reject_reason = DM1_V1_CPSAO_REJECT_NONE_PC34;
        *out_plan = plan;
        return 1;
    }

    active_caster = input->requested_caster_index;
    plan.post_caster_index = active_caster;
    plan.reject_reason = DM1_V1_CPSAO_REJECT_NONE_PC34;
    plan.cleared_to_black = 0;

    /* CASTER.C:75-95 MEDIA529 includes I34E/I34M: every accepted
     * non-NONE caster paints C009 then controls and source glyphs.
     * F0396/F0392 strip construction belongs to early MEDIA009 only. */
    plan.drew_background_graphic = 1;
    plan.drew_lines_bitmap = 0;
    plan.drew_spell_area_controls = 1;
    plan.drew_available_symbols = 1;
    plan.drew_champion_symbols = 1;
    plan.new_caster_symbol_step = (int)input->symbol_step;

    fill_line1(input, active_caster, &plan);
    fill_line2(input->symbol_step, &plan);
    fill_line3(&input->champions[active_caster], &plan);

    *out_plan = plan;
    return 1;
}

static int material_matches_source_surface(int loaded_pixels,
                                           int graphic_index,
                                           int expected_index,
                                           int width,
                                           int expected_width,
                                           int height,
                                           int expected_height)
{
    return loaded_pixels &&
           graphic_index == expected_index &&
           width == expected_width &&
           height == expected_height;
}

int dm1_v1_champion_panel_spell_area_overlay_material_receipt_pc34(
    const DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 *plan,
    const DM1_V1_ChampionPanelSpellAreaOverlayMaterialFactsPc34 *facts,
    DM1_V1_ChampionPanelSpellAreaOverlayMaterialReceiptPc34 *out_receipt)
{
    DM1_V1_ChampionPanelSpellAreaOverlayMaterialReceiptPc34 receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_host_fallback_visuals = 1;

    if (!plan || !facts || !plan->valid) {
        receipt.reject_reason =
            DM1_V1_CPSAO_MATERIAL_REJECT_INVALID_PLAN_PC34;
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = 1;
    receipt.plan_draws_background = plan->drew_background_graphic;
    receipt.plan_draws_lines = plan->drew_lines_bitmap;
    receipt.plan_draws_font =
        plan->drew_spell_area_controls ||
        plan->drew_available_symbols ||
        plan->drew_champion_symbols;
    receipt.c009_required = receipt.plan_draws_background;
    receipt.c011_required = receipt.plan_draws_lines;
    receipt.m653_required = receipt.plan_draws_font;
    receipt.c009_graphic_index = facts->c009_graphic_index;
    receipt.c009_width = facts->c009_width;
    receipt.c009_height = facts->c009_height;
    receipt.c011_graphic_index = facts->c011_graphic_index;
    receipt.c011_width = facts->c011_width;
    receipt.c011_height = facts->c011_height;
    receipt.m653_graphic_index = facts->m653_graphic_index;

    if (!receipt.plan_draws_background &&
        !receipt.plan_draws_lines &&
        !receipt.plan_draws_font) {
        receipt.reject_reason =
            DM1_V1_CPSAO_MATERIAL_REJECT_PLAN_NO_DRAW_PC34;
        *out_receipt = receipt;
        return 1;
    }

    receipt.c009_source_bound =
        !receipt.c009_required ||
        material_matches_source_surface(
            facts->c009_loaded_pixels,
            facts->c009_graphic_index,
            DM1_V1_CPSAO_GFX_SPELL_AREA_BACKGROUND_PC34,
            facts->c009_width,
            DM1_V1_CPSAO_C009_STORED_WIDTH_PC34,
            facts->c009_height,
            DM1_V1_CPSAO_C009_STORED_HEIGHT_PC34);
    if (!receipt.c009_source_bound) {
        receipt.reject_reason = DM1_V1_CPSAO_MATERIAL_REJECT_C009_PC34;
        *out_receipt = receipt;
        return 1;
    }

    receipt.c011_source_bound =
        !receipt.c011_required ||
        material_matches_source_surface(
            facts->c011_loaded_pixels,
            facts->c011_graphic_index,
            DM1_V1_CPSAO_GFX_SPELL_AREA_LINES_PC34,
            facts->c011_width,
            DM1_V1_CPSAO_LINES_WIDTH_PC34,
            facts->c011_height,
            DM1_V1_CPSAO_LINES_HEIGHT_PC34);
    if (!receipt.c011_source_bound) {
        receipt.reject_reason = DM1_V1_CPSAO_MATERIAL_REJECT_C011_PC34;
        *out_receipt = receipt;
        return 1;
    }

    receipt.m653_source_bound =
        !receipt.m653_required ||
        (facts->m653_source_bound &&
         (facts->m653_graphic_index == DM1_V1_CPSAO_M653_GRAPHIC_PC34 ||
          facts->m653_graphic_index == DM1_V1_CPSAO_M653_GRAPHIC_LEGACY_PC34));
    if (!receipt.m653_source_bound) {
        receipt.reject_reason = DM1_V1_CPSAO_MATERIAL_REJECT_M653_PC34;
        *out_receipt = receipt;
        return 1;
    }

    receipt.drawable = 1;
    receipt.reject_reason = DM1_V1_CPSAO_MATERIAL_ACCEPTED_PC34;
    *out_receipt = receipt;
    return 1;
}
