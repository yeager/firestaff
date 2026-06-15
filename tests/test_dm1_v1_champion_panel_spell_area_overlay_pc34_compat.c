#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_spell_area_overlay_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * DM1 V1 champion-panel spell-area overlay draw-contract test.
 *
 * Pinned contracts (all anchored to the ReDMCSB WIP20210206 PC 3.4 path):
 * - CASTER.C:18-21 F0394 same-caster short-circuit.
 * - CASTER.C:18-21 F0394 dead-champion reject.
 * - CASTER.C:23-26 F0394 background blit on a transition from NONE.
 * - CASTER.C:28-32 F0394 CM1_CHAMPION_NONE clear-to-black path.
 * - SPELDRAW.C:36-90 F0393 4-tab x0 columns and CurrentHealth / party
 *   count gate.
 * - MENUDRAW.C:47-80 F0397 6-symbol window starting at 96+6*SymbolStep.
 * - MENUDRAW.C:83-117 F0398 strlen(Symbols) fill + space-pad tail.
 */

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d expected=%d anchor=%s\n",
                label, actual, expected, anchor);
    }
}

static void check_contains(const char *label, const char *text,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!text || !strstr(text, needle)) {
        ++g_failures;
        fprintf(stderr, "FAIL %s missing='%s' anchor=%s\n",
                label, needle ? needle : "(null)", anchor);
    }
}

static void reset_input(DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 *in)
{
    int i;
    memset(in, 0, sizeof(*in));
    for (i = 0; i < DM1_V1_CPSAO_CHAMPION_COUNT_PC34; ++i) {
        in->champions[i].index = i;
        in->champions[i].current_health = 100;
        in->champions[i].symbols[0] = '\0';
    }
}

static void type_symbols(DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 *in,
                         int champion_index, const char *s)
{
    size_t n = strlen(s);
    if (n > DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34) {
        n = DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34;
    }
    memcpy(in->champions[champion_index].symbols, s, n);
    in->champions[champion_index].symbols[n] = '\0';
}

static void test_contract(void)
{
    const DM1_V1_ChampionPanelSpellAreaOverlayPc34Contract *c;
    const char *evidence;

    c = dm1_v1_champion_panel_spell_area_overlay_contract_pc34();
    evidence =
        dm1_v1_champion_panel_spell_area_overlay_source_evidence_pc34();

    check_int("contract.only", c->contract_only, 1,
              "contract-only no game data");
    check_int("contract.champions", c->champion_count, 4,
              "DEFS.H C100..C109 4-champion party");
    check_int("contract.spellX0", c->spell_area_x0, 224,
              "DATA.C:119 G0000 spell-area box");
    check_int("contract.spellX1", c->spell_area_x1, 319,
              "DATA.C:119 G0000 spell-area box");
    check_int("contract.spellY0", c->spell_area_y0, 42,
              "DATA.C:119 G0000 spell-area box");
    check_int("contract.spellY1", c->spell_area_y1, 74,
              "DATA.C:119 G0000 spell-area box");
    check_int("contract.spellW", c->spell_area_width, 96,
              "96x33 spell-area box");
    check_int("contract.spellH", c->spell_area_height, 33,
              "96x33 spell-area box");
    check_int("contract.line2X0", c->line2_x0, 224,
              "DATA.C:530 G1072 line 2 box");
    check_int("contract.line2X1", c->line2_x1, 319,
              "DATA.C:530 G1072 line 2 box");
    check_int("contract.line2Y0", c->line2_y0, 50,
              "DATA.C:530 G1072 line 2 box");
    check_int("contract.line2Y1", c->line2_y1, 61,
              "DATA.C:530 G1072 line 2 box");
    check_int("contract.line3X0", c->line3_x0, 224,
              "DATA.C:531 G1073 line 3 box");
    check_int("contract.line3X1", c->line3_x1, 319,
              "DATA.C:531 G1073 line 3 box");
    check_int("contract.line3Y0", c->line3_y0, 62,
              "DATA.C:531 G1073 line 3 box");
    check_int("contract.line3Y1", c->line3_y1, 73,
              "DATA.C:531 G1073 line 3 box");
    check_int("contract.avalCount", c->available_symbol_count, 6,
              "MENUDRAW.C:47 F0397 6 symbols");
    check_int("contract.avalBase", c->available_symbol_base, 96,
              "MENUDRAW.C:67 96 + 6*SymbolStep base");
    check_int("contract.avalStep", c->available_symbol_step, 14,
              "MENUDRAW.C:73 x += 14 stride");
    check_int("contract.avalX0", c->available_symbol_x0, 225,
              "MENUDRAW.C:71 x starts at 225");
    check_int("contract.avalY", c->available_symbol_y, 58,
              "MENUDRAW.C:73 y=58");
    check_int("contract.champMax", c->champion_symbol_max, 4,
              "MENUDRAW.C:96 max 4 champion symbols");
    check_int("contract.champStep", c->champion_symbol_step, 9,
              "MENUDRAW.C:104 x += 9 stride");
    check_int("contract.champX0", c->champion_symbol_x0, 232,
              "MENUDRAW.C:103 x starts at 232");
    check_int("contract.champY", c->champion_symbol_y, 70,
              "MENUDRAW.C:106 y=70");
    check_int("contract.tab0X0", c->tab_champion_0_x0, 233,
              "SPELDRAW.C F0393 champion 0 x0");
    check_int("contract.tab0X1", c->tab_champion_0_x1, 277,
              "SPELDRAW.C F0393 champion 0 x1 (44 wide)");
    check_int("contract.tab1X0", c->tab_champion_1_x0, 280,
              "SPELDRAW.C F0393 champion 1 x0");
    check_int("contract.tab1X1", c->tab_champion_1_x1, 291,
              "SPELDRAW.C F0393 champion 1 x1 (11 wide)");
    check_int("contract.tab2X0", c->tab_champion_2_x0, 294,
              "SPELDRAW.C F0393 champion 2 x0");
    check_int("contract.tab2X1", c->tab_champion_2_x1, 305,
              "SPELDRAW.C F0393 champion 2 x1");
    check_int("contract.tab3X0", c->tab_champion_3_x0, 308,
              "SPELDRAW.C F0393 champion 3 x0");
    check_int("contract.tab3X1", c->tab_champion_3_x1, 319,
              "SPELDRAW.C F0393 champion 3 x1");
    check_int("contract.tabY0", c->tab_y0, 42,
              "SPELDRAW.C F0393 y0=42");
    check_int("contract.tab0Y1", c->tab_champion_0_y1, 49,
              "SPELDRAW.C F0393 champion 0 y1=49");
    check_int("contract.tabOtherY1", c->tab_other_y1, 48,
              "SPELDRAW.C F0393 other y1=48");
    check_int("contract.colorCyan", c->color_cyan, 4,
              "DEFS.H C04_COLOR_CYAN");
    check_int("contract.colorBlack", c->color_black, 0,
              "DEFS.H C00_COLOR_BLACK");
    check_int("contract.championNone", c->champion_none, -1,
              "DEFS.H CM1_CHAMPION_NONE");
    check_int("contract.gfxBackground", c->gfx_spell_area_background, 9,
              "DEFS.H C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND");
    check_int("contract.gfxLines", c->gfx_spell_area_lines, 11,
              "DEFS.H C011_GRAPHIC_MENU_SPELL_AREA_LINES");
    check_int("contract.zone", c->zone_spell_area, 13,
              "DEFS.H C013_ZONE_SPELL_AREA");
    check_int("contract.cmdClick", c->command_click_in_spell_area, 100,
              "DEFS.H C100_COMMAND_CLICK_IN_SPELL_AREA");
    check_int("contract.cmdSetCaster", c->command_set_caster, 109,
              "DEFS.H C109 set caster");
    check_int("contract.cmdCast", c->command_cast, 108,
              "DEFS.H C108 cast");
    check_int("contract.cmdRecant", c->command_recant, 107,
              "DEFS.H C107 recant");
    check_int("contract.cmdRuneFirst", c->command_rune_first, 101,
              "DEFS.H C101 first rune");
    check_int("contract.cmdRuneLast", c->command_rune_last, 106,
              "DEFS.H C106 last rune");

    check_contains("contract.f0394", c->caster_set_anchor, "F0394",
                   "CASTER.C F0394 anchor");
    check_contains("contract.f0393", c->spell_area_controls_anchor, "F0393",
                   "SPELDRAW.C F0393 anchor");
    check_contains("contract.f0397", c->available_symbols_anchor, "F0397",
                   "MENUDRAW.C F0397 anchor");
    check_contains("contract.f0398", c->champion_symbols_anchor, "F0398",
                   "MENUDRAW.C F0398 anchor");
    check_contains("contract.f0396", c->lines_bitmap_anchor, "F0396",
                   "MENUDRAW.C F0396 anchor");
    check_contains("contract.screenBox", c->screen_box_anchor, "224,319,42,74",
                   "DATA.C spell-area screen box");
    check_contains("contract.defs", c->defs_anchor, "C04/C00",
                   "DEFS.H color ids");
    check_contains("contract.refresh", c->refresh_callers_anchor,
                   "F0393", "REVIVE/CHAMPION.C refresh callers");
    check_contains("source.f0394", evidence, "CASTER.C:18-21",
                   "F0394 short-circuit in evidence");
    check_contains("source.f0393", evidence, "F0393",
                   "F0393 in evidence");
    check_contains("source.f0397", evidence, "F0397",
                   "F0397 in evidence");
    check_contains("source.f0398", evidence, "F0398",
                   "F0398 in evidence");
    check_contains("source.f0396", evidence, "F0396",
                   "F0396 in evidence");
    check_contains("source.g1072", evidence, "G1072",
                   "G1072 line 2 box in evidence");
    check_contains("source.g1073", evidence, "G1073",
                   "G1073 line 3 box in evidence");
    check_contains("source.champion0", evidence, "233..277",
                   "champion 0 tab x0/x1 in evidence");
}

static void test_same_caster_reject(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;

    reset_input(&in);
    in.previous_caster_index = 1;
    in.requested_caster_index = 1;
    in.party_champion_count = 4;
    in.symbol_step = 0;
    type_symbols(&in, 1, "LO");

    check_int("sameCaster.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "CASTER.C:18 same-caster path");
    check_int("sameCaster.valid", plan.valid, 1, "plan still valid");
    check_int("sameCaster.reason", plan.reject_reason,
              DM1_V1_CPSAO_REJECT_SAME_CASTER_PC34,
              "CASTER.C:18-21 reject reason");
    check_int("sameCaster.postCaster", plan.post_caster_index, 1,
              "post caster preserved");
    check_int("sameCaster.background", plan.drew_background_graphic, 0,
              "no background blit on same-caster reject");
    check_int("sameCaster.linesBitmap", plan.drew_lines_bitmap, 0,
              "no lines bitmap on same-caster reject");
    check_int("sameCaster.controls", plan.drew_spell_area_controls, 0,
              "no F0393 on same-caster reject");
    check_int("sameCaster.avail", plan.drew_available_symbols, 0,
              "no F0397 on same-caster reject");
    check_int("sameCaster.champ", plan.drew_champion_symbols, 0,
              "no F0398 on same-caster reject");
    check_int("sameCaster.cleared", plan.cleared_to_black, 0,
              "no clear-to-black on same-caster reject");
    check_int("sameCaster.tabs", plan.tab_count, 0,
              "no tabs on same-caster reject");
    check_int("sameCaster.availCount", plan.available_symbol_count, 0,
              "no available symbols on same-caster reject");
    check_int("sameCaster.champCount", plan.champion_symbol_count, 0,
              "no champion symbols on same-caster reject");
}

static void test_dead_champion_reject(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;

    reset_input(&in);
    in.previous_caster_index = 0;
    in.requested_caster_index = 2;
    in.party_champion_count = 4;
    in.symbol_step = 0;
    in.champions[2].current_health = 0;
    type_symbols(&in, 2, "ZA");

    check_int("deadChamp.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "CASTER.C:18 dead-champion path");
    check_int("deadChamp.reason", plan.reject_reason,
              DM1_V1_CPSAO_REJECT_DEAD_CHAMPION_PC34,
              "CASTER.C:18-21 dead-champion reject");
    check_int("deadChamp.postCaster", plan.post_caster_index, 0,
              "post caster unchanged");
    check_int("deadChamp.drewAll", plan.drew_spell_area_controls +
                                       plan.drew_available_symbols +
                                       plan.drew_champion_symbols,
              0, "no draws on dead-champion reject");
}

static void test_none_clear_path(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;

    reset_input(&in);
    in.previous_caster_index = 1;
    in.requested_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
    in.party_champion_count = 4;
    in.symbol_step = 2;
    type_symbols(&in, 1, "FULL");

    check_int("noneClear.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "CASTER.C:28 NONE clear path");
    check_int("noneClear.reason", plan.reject_reason,
              DM1_V1_CPSAO_REJECT_NONE_PC34, "not a reject");
    check_int("noneClear.postCaster", plan.post_caster_index,
              DM1_V1_CPSAO_CHAMPION_NONE_PC34,
              "G0514 reset to CM1_CHAMPION_NONE");
    check_int("noneClear.clearedBlack", plan.cleared_to_black, 1,
              "M524_FillScreenBox(..., C00_COLOR_BLACK)");
    check_int("noneClear.background", plan.drew_background_graphic, 0,
              "no C009 background on clear path");
    check_int("noneClear.linesBitmap", plan.drew_lines_bitmap, 0,
              "no C011 lines on clear path");
    check_int("noneClear.controls", plan.drew_spell_area_controls, 0,
              "no F0393 on clear path");
    check_int("noneClear.avail", plan.drew_available_symbols, 0,
              "no F0397 on clear path");
    check_int("noneClear.champ", plan.drew_champion_symbols, 0,
              "no F0398 on clear path");
    check_int("noneClear.tabs", plan.tab_count, 0,
              "no tabs on clear path");
    check_int("noneClear.availCount", plan.available_symbol_count, 0,
              "no available symbols on clear path");
    check_int("noneClear.champCount", plan.champion_symbol_count, 0,
              "no champion symbols on clear path");
}

static void test_first_caster_after_none(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;
    int i;

    reset_input(&in);
    in.previous_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
    in.requested_caster_index = 0;
    in.party_champion_count = 4;
    in.symbol_step = 0;
    type_symbols(&in, 0, "AB");

    check_int("firstCaster.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "CASTER.C:23 first caster after NONE");
    check_int("firstCaster.reason", plan.reject_reason,
              DM1_V1_CPSAO_REJECT_NONE_PC34, "not a reject");
    check_int("firstCaster.postCaster", plan.post_caster_index, 0,
              "G0514 = 0");
    check_int("firstCaster.background", plan.drew_background_graphic, 1,
              "C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND on transition from NONE");
    check_int("firstCaster.linesBitmap", plan.drew_lines_bitmap, 1,
              "F0396 lines bitmap loaded");
    check_int("firstCaster.controls", plan.drew_spell_area_controls, 1,
              "F0393 drew");
    check_int("firstCaster.avail", plan.drew_available_symbols, 1,
              "F0397 drew");
    check_int("firstCaster.champ", plan.drew_champion_symbols, 1,
              "F0398 drew");
    check_int("firstCaster.tabs", plan.tab_count, 4,
              "4 healthy champions in a 4-party");
    check_int("firstCaster.availCount", plan.available_symbol_count, 6,
              "F0397 emits 6 symbols");
    check_int("firstCaster.champCount", plan.champion_symbol_count, 4,
              "F0398 emits up to 4 slots");
    check_int("firstCaster.cleared", plan.cleared_to_black, 0,
              "no clear-to-black on draw path");

    check_int("firstCaster.symStep", plan.new_caster_symbol_step, 0,
              "champion 0 SymbolStep preserved");
    check_int("firstCaster.symLen", plan.new_caster_symbols_length, 2,
              "2 symbols 'AB'");
    check_int("firstCaster.sym0", plan.new_caster_symbols[0], 'A',
              "F0398 draws Symbols[0] = 'A'");
    check_int("firstCaster.sym1", plan.new_caster_symbols[1], 'B',
              "F0398 draws Symbols[1] = 'B'");
    check_int("firstCaster.sym2", plan.new_caster_symbols[2], 0,
              "F0398 pads with NUL-terminated 2");
    check_int("firstCaster.sym3", plan.new_caster_symbols[3], 0,
              "F0398 pads with NUL-terminated 3");

    /* Line 1 champion 0 (active) */
    check_int("firstCaster.l1c0idx", plan.line1[0].champion_index, 0,
              "tab 0 champion index");
    check_int("firstCaster.l1c0x0", plan.line1[0].tab_x0, 233,
              "SPELDRAW.C F0393 champion 0 x0");
    check_int("firstCaster.l1c0x1", plan.line1[0].tab_x1, 277,
              "SPELDRAW.C F0393 champion 0 x1");
    check_int("firstCaster.l1c0y0", plan.line1[0].tab_y0, 42,
              "F0393 y0=42");
    check_int("firstCaster.l1c0y1", plan.line1[0].tab_y1, 49,
              "F0393 champion 0 y1=49");
    check_int("firstCaster.l1c0hl", plan.line1[0].highlighted, 1,
              "active champion 0 tab highlighted");
    check_int("firstCaster.l1c0present", plan.line1[0].present, 1,
              "alive champion 0 is present");
    /* Line 1 champion 1..3 (not active) */
    for (i = 1; i < 4; ++i) {
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "firstCaster.l1c%dhl", i);
        check_int(lbl, plan.line1[i].highlighted, 0,
                  "non-active champion not highlighted");
        snprintf(lbl, sizeof(lbl), "firstCaster.l1c%dpresent", i);
        check_int(lbl, plan.line1[i].present, 1,
                  "alive champion is present");
    }
    /* Line 1 column geometry (x0 only, x1 differs for champion 0) */
    check_int("firstCaster.l1c1x0", plan.line1[1].tab_x0, 280,
              "champion 1 x0");
    check_int("firstCaster.l1c1y1", plan.line1[1].tab_y1, 48,
              "champion 1 y1=48 (not 49)");
    check_int("firstCaster.l1c3x0", plan.line1[3].tab_x0, 308,
              "champion 3 x0");
    check_int("firstCaster.l1c3x1", plan.line1[3].tab_x1, 319,
              "champion 3 x1");

    /* Line 2 available symbols at step 0. */
    for (i = 0; i < 6; ++i) {
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "firstCaster.l2c%d", i);
        check_int(lbl, plan.line2[i].character, 96 + i,
                  "F0397 first window 96..101");
        snprintf(lbl, sizeof(lbl), "firstCaster.l2x%d", i);
        check_int(lbl, plan.line2[i].screen_x, 225 + 14 * i,
                  "F0397 x stride 14");
        snprintf(lbl, sizeof(lbl), "firstCaster.l2y%d", i);
        check_int(lbl, plan.line2[i].screen_y, 58, "F0397 y=58");
    }

    /* Line 3 champion symbols. */
    check_int("firstCaster.l3c0x", plan.line3[0].screen_x, 232,
              "F0398 first slot x=232");
    check_int("firstCaster.l3c0char", plan.line3[0].drawn_character, 'A',
              "F0398 slot 0 from Symbols[0]");
    check_int("firstCaster.l3c0src", plan.line3[0].from_champion_symbols, 1,
              "F0398 slot 0 from Symbols");
    check_int("firstCaster.l3c1char", plan.line3[1].drawn_character, 'B',
              "F0398 slot 1 from Symbols[1]");
    check_int("firstCaster.l3c2char", plan.line3[2].drawn_character, ' ',
              "F0398 pads slot 2 with C20_SPACE");
    check_int("firstCaster.l3c2src", plan.line3[2].from_champion_symbols, 0,
              "F0398 slot 2 not from Symbols");
    check_int("firstCaster.l3c3char", plan.line3[3].drawn_character, ' ',
              "F0398 pads slot 3 with C20_SPACE");
    check_int("firstCaster.l3c3x", plan.line3[3].screen_x, 232 + 9 * 3,
              "F0398 slot 3 x stride");
    check_int("firstCaster.l3y", plan.line3[0].screen_y, 70, "F0398 y=70");
}

static void test_caster_swap_with_step_window(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;
    int i;

    reset_input(&in);
    in.previous_caster_index = 0;
    in.requested_caster_index = 2;
    in.party_champion_count = 4;
    in.symbol_step = 3;
    type_symbols(&in, 2, "FO");

    check_int("swapStep.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "CASTER.C:18 swap path");
    check_int("swapStep.postCaster", plan.post_caster_index, 2,
              "G0514 = 2");
    check_int("swapStep.background", plan.drew_background_graphic, 0,
              "no C009 background on caster-caster transition");
    check_int("swapStep.controls", plan.drew_spell_area_controls, 1,
              "F0393 drew");
    check_int("swapStep.avail", plan.drew_available_symbols, 1,
              "F0397 drew");

    check_int("swapStep.l1c0hl", plan.line1[0].highlighted, 0,
              "old caster 0 no longer highlighted");
    check_int("swapStep.l1c0present", plan.line1[0].present, 1,
              "champion 0 still present");
    check_int("swapStep.l1c2hl", plan.line1[2].highlighted, 1,
              "new caster 2 highlighted");
    check_int("swapStep.l1c2present", plan.line1[2].present, 1,
              "champion 2 present");
    check_int("swapStep.l1c2x0", plan.line1[2].tab_x0, 294,
              "champion 2 x0=294");

    /* F0397 step 3 -> characters 96 + 6*3 = 114..119 */
    for (i = 0; i < 6; ++i) {
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "swapStep.l2c%d", i);
        check_int(lbl, plan.line2[i].character, 114 + i,
                  "F0397 step 3 = 114..119");
    }
    check_int("swapStep.l2c0x", plan.line2[0].screen_x, 225,
              "F0397 x origin at 225");
    check_int("swapStep.l2c5x", plan.line2[5].screen_x, 225 + 14 * 5,
              "F0397 stride 14 to slot 5");

    /* F0398 line 3. */
    check_int("swapStep.l3c0char", plan.line3[0].drawn_character, 'F',
              "F0398 champion 2 Symbols[0] = 'F'");
    check_int("swapStep.l3c1char", plan.line3[1].drawn_character, 'O',
              "F0398 champion 2 Symbols[1] = 'O'");
    check_int("swapStep.l3c2char", plan.line3[2].drawn_character, ' ',
              "F0398 pads slot 2 with C20_SPACE");
    check_int("swapStep.l3c3char", plan.line3[3].drawn_character, ' ',
              "F0398 pads slot 3 with C20_SPACE");
}

static void test_dead_champion_excluded_from_tabs(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;
    int i;

    reset_input(&in);
    in.previous_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
    in.requested_caster_index = 1;
    in.party_champion_count = 3;
    in.symbol_step = 0;
    in.champions[3].current_health = 0;
    in.champions[2].current_health = 0;
    in.champions[2].index = 2;
    type_symbols(&in, 1, "K");

    check_int("deadExcl.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "partial party with dead slots");
    check_int("deadExcl.tabs", plan.tab_count, 2,
              "2 healthy + alive champions (0,1)");
    check_int("deadExcl.l1c0present", plan.line1[0].present, 1,
              "champion 0 alive");
    check_int("deadExcl.l1c1present", plan.line1[1].present, 1,
              "champion 1 alive");
    check_int("deadExcl.l1c2present", plan.line1[2].present, 0,
              "champion 2 dead -> not present");
    check_int("deadExcl.l1c3present", plan.line1[3].present, 0,
              "champion 3 dead -> not present");
    check_int("deadExcl.l1c2hl", plan.line1[2].highlighted, 0,
              "dead champion cannot be highlighted");
    check_int("deadExcl.l1c3hl", plan.line1[3].highlighted, 0,
              "dead champion cannot be highlighted");
    check_int("deadExcl.l1c1hl", plan.line1[1].highlighted, 1,
              "active champion 1 highlighted");
    for (i = 0; i < 4; ++i) {
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "deadExcl.l1c%dhl", i);
        if (i != 1)
            check_int(lbl, plan.line1[i].highlighted, 0,
                      "non-active champion not highlighted");
    }
}

static void test_full_symbols_no_padding(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;

    reset_input(&in);
    in.previous_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
    in.requested_caster_index = 0;
    in.party_champion_count = 4;
    in.symbol_step = 0;
    type_symbols(&in, 0, "WXYZ");

    check_int("fullSym.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "full symbol buffer");
    check_int("fullSym.symLen", plan.new_caster_symbols_length, 4,
              "4 symbols 'WXYZ'");
    check_int("fullSym.sym0", plan.line3[0].drawn_character, 'W',
              "F0398 slot 0 = 'W'");
    check_int("fullSym.sym1", plan.line3[1].drawn_character, 'X',
              "F0398 slot 1 = 'X'");
    check_int("fullSym.sym2", plan.line3[2].drawn_character, 'Y',
              "F0398 slot 2 = 'Y'");
    check_int("fullSym.sym3", plan.line3[3].drawn_character, 'Z',
              "F0398 slot 3 = 'Z'");
    check_int("fullSym.sym3src", plan.line3[3].from_champion_symbols, 1,
              "F0398 slot 3 from Symbols");
    check_int("fullSym.symX", plan.line3[3].screen_x, 232 + 9 * 3,
              "F0398 slot 3 x stride");
}

static void test_empty_symbols_full_padding(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;
    int i;

    reset_input(&in);
    in.previous_caster_index = DM1_V1_CPSAO_CHAMPION_NONE_PC34;
    in.requested_caster_index = 0;
    in.party_champion_count = 4;
    in.symbol_step = 0;
    type_symbols(&in, 0, "");

    check_int("emptySym.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              1, "empty symbol buffer");
    check_int("emptySym.symLen", plan.new_caster_symbols_length, 0,
              "0 symbols");
    for (i = 0; i < 4; ++i) {
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "emptySym.sym%d", i);
        check_int(lbl, plan.line3[i].drawn_character, ' ',
                  "F0398 pads all 4 slots with C20_SPACE");
    }
}

static void test_validation_guards(void)
{
    DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 in;
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 plan;

    reset_input(&in);
    in.previous_caster_index = 0;
    in.requested_caster_index = 5;
    in.party_champion_count = 4;
    in.symbol_step = 0;
    check_int("badCaster.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              0, "requested_caster_index out of range");

    reset_input(&in);
    in.previous_caster_index = 5;
    in.requested_caster_index = 0;
    in.party_champion_count = 4;
    in.symbol_step = 0;
    check_int("badPrev.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              0, "previous_caster_index out of range");

    reset_input(&in);
    in.previous_caster_index = 0;
    in.requested_caster_index = 0;
    in.party_champion_count = 5;
    in.symbol_step = 0;
    check_int("badParty.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              0, "party_champion_count out of range");

    reset_input(&in);
    in.previous_caster_index = 0;
    in.requested_caster_index = 0;
    in.party_champion_count = 4;
    in.symbol_step = 6;
    check_int("badStep.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, &plan),
              0, "SymbolStep > 5");

    check_int("nullOut.build",
              dm1_v1_champion_panel_spell_area_overlay_plan_pc34(&in, NULL),
              0, "null output guard");
}

int main(void)
{
    printf("== DM1 V1 champion panel spell-area overlay slice ==\n");
    test_contract();
    test_same_caster_reject();
    test_dead_champion_reject();
    test_none_clear_path();
    test_first_caster_after_none();
    test_caster_swap_with_step_window();
    test_dead_champion_excluded_from_tabs();
    test_full_symbols_no_padding();
    test_empty_symbols_full_padding();
    test_validation_guards();

    if (g_assertions < 100) {
        fprintf(stderr, "FAIL assertion_count got=%d expected>=100\n",
                g_assertions);
        return 1;
    }
    if (g_failures != 0) {
        fprintf(stderr, "FAILURES %d / %d assertions\n",
                g_failures, g_assertions);
        return 1;
    }
    printf("PASS test_dm1_v1_champion_panel_spell_area_overlay_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
