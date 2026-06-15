#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_SPELL_AREA_OVERLAY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_SPELL_AREA_OVERLAY_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel spell-area overlay draw contract.
 *
 * Contract-only, no-asset fixture. This pins the upper-right panel HUD strip
 * (screen box {224, 319, 42, 74}, 96x33 px) the original ReDMCSB PC 3.4
 * BIOS paints every time the acting caster changes. The existing
 * spell_area_routes gate covers the *input* dispatch (C100..C109 touch
 * matrix -> F0370_CLIKMENU_ProcessChampionCommand routes) and the
 * dm1_v1_menu_render gate covers the *orchestrator* flag. This gate
 * covers the *draw contract* of the three lines:
 *
 *   Line 1 (y=42..49): 4 champion tab highlight boxes drawn by
 *     F0393_MENUS_DrawSpellAreaControls when called from F0394 / REVIVE.C
 *     (resurrect/reincarnate routes) / CHAMPION.C F0284 (party direction
 *     rotation) / MENU.C:1657 (after F0412_GetChampionSpellCastResult).
 *     Each tab is x0=233/280/294/308 by 11 wide, y0=42 by 7 tall, gated
 *     by Champion[n].CurrentHealth > 0 and PartyChampionCount > n.
 *   Line 2 (y=50..61): 6 available rune symbols drawn by
 *     F0397_MENUS_DrawAvailableSymbols. First char = 96 + 6*SymbolStep,
 *     6 chars in C04_COLOR_CYAN on C00_COLOR_BLACK, x = 225 + 14*i,
 *     y = 58.
 *   Line 3 (y=62..73): up to 4 currently-typed champion symbols drawn by
 *     F0398_MENUS_DrawChampionSymbols. The first strlen(Symbols) chars
 *     are filled from Champion->Symbols[0..N-1], the rest are padded
 *     with C20_SPACE. x = 232 + 9*i, y = 70, cyan on black.
 *
 * The CASTER.C F0394 entry has two early-out / state-mutate paths the
 * test pins:
 *   - the same-caster short-circuit:
 *       if (ChampionIndex == G0514_i_MagicCasterChampionIndex) return;
 *   - the dead-champion reject:
 *       if (ChampionIndex != CM1_CHAMPION_NONE &&
 *           !M516_CHAMPIONS[ChampionIndex].CurrentHealth) return;
 *   - the CM1_CHAMPION_NONE clear path:
 *       G0514_i_MagicCasterChampionIndex = CM1_CHAMPION_NONE;
 *       M524_FillScreenBox(G0000_ai_Graphic562_Box_SpellArea,
 *                          C00_COLOR_BLACK);
 *
 * ReDMCSB source anchors (WIP20210206, Media042 PC 3.4 path):
 * - CASTER.C F0394_MENUS_SetMagicCasterAndDrawSpellArea: same-caster
 *   reject, dead-campion reject, CM1_CHAMPION_NONE clear path.
 * - SPELDRAW.C F0393_MENUS_DrawSpellAreaControls: 4 tab x0 columns and
 *   the per-champion CurrentHealth gate.
 * - MENUDRAW.C F0397_MENUS_DrawAvailableSymbols: 6-char SymbolStep
 *   window and x/y origin.
 * - MENUDRAW.C F0398_MENUS_DrawChampionSymbols: strlen(Symbols) fill +
 *   space-pad tail + x = 232 + 9*i and y = 70 origin.
 * - MENUDRAW.C F0396_MENUS_LoadSpellAreaLinesBitmap: C011 graphic load
 *   into the 3-row stack (header + line 2 + line 3).
 * - DATA.C:119 G0000_ai_Graphic562_Box_SpellArea = {224,319,42,74}.
 * - DATA.C:530-531 G1072_ai_Box_SpellAreaLine2 = {224,319,50,61},
 *   G1073_ai_Box_SpellAreaLine3 = {224,319,62,73}.
 * - DEFS.H C100_COMMAND_CLICK_IN_SPELL_AREA / C101..C106 / C107 recant /
 *   C108 cast / C109 set caster (input side, not draw side).
 * - DEFS.H C013_ZONE_SPELL_AREA (alt path for newer MEDIA529 builds).
 * - DEFS.H C255..C260 ZONE_SPELL_AREA_AVAILABLE_SYMBOL_0..5 (alt path).
 * - DEFS.H C261..C264 ZONE_SPELL_AREA_CHAMPION_SYMBOL_0..3 (alt path).
 * - DEFS.H C04_COLOR_CYAN, C00_COLOR_BLACK text colors.
 * - DEFS.H CM1_CHAMPION_NONE sentinel.
 * - CHAMPION.C F0284_CHAMPION_SetPartyDirection:1681 F0393 refresh on
 *   leader rotation, preserving F0394 short-circuit.
 * - REVIVE.C F0280/F0282:292,845,931 F0393 refresh on resurrect/reincarnate
 *   candidate selection/cancel/clear, all gated by MagicCasterChampionIndex
 *   != CM1_CHAMPION_NONE before the F0393 draw call.
 *
 * Public contract:
 *   - dm1_v1_champion_panel_spell_area_overlay_contract_pc34()
 *       returns the static struct pinning the screen box, line geometry,
 *       tab x0 columns, color ids, and 6 source-anchor strings.
 *   - dm1_v1_champion_panel_spell_area_overlay_source_evidence_pc34()
 *       returns the long-form evidence string.
 *   - dm1_v1_champion_panel_spell_area_overlay_plan_pc34(input, out)
 *       consumes a 4-champion party + 1 caster input and returns the
 *       deterministic pre/post draw plan (which lines drew, which tabs
 *       were highlighted, which rune window, which champion symbols and
 *       which trailing space pads, plus the F0394 short-circuit
 *       accepted/rejected reason). The function is a contract checker
 *       and never reads or mutates real GRAPHICS.DAT bytes.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPSAO_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPSAO_SPELL_AREA_X0_PC34 224
#define DM1_V1_CPSAO_SPELL_AREA_X1_PC34 319
#define DM1_V1_CPSAO_SPELL_AREA_Y0_PC34 42
#define DM1_V1_CPSAO_SPELL_AREA_Y1_PC34 74
#define DM1_V1_CPSAO_SPELL_AREA_WIDTH_PC34 \
    (DM1_V1_CPSAO_SPELL_AREA_X1_PC34 - DM1_V1_CPSAO_SPELL_AREA_X0_PC34 + 1)
#define DM1_V1_CPSAO_SPELL_AREA_HEIGHT_PC34 \
    (DM1_V1_CPSAO_SPELL_AREA_Y1_PC34 - DM1_V1_CPSAO_SPELL_AREA_Y0_PC34 + 1)

#define DM1_V1_CPSAO_LINE2_X0_PC34 224
#define DM1_V1_CPSAO_LINE2_X1_PC34 319
#define DM1_V1_CPSAO_LINE2_Y0_PC34 50
#define DM1_V1_CPSAO_LINE2_Y1_PC34 61

#define DM1_V1_CPSAO_LINE3_X0_PC34 224
#define DM1_V1_CPSAO_LINE3_X1_PC34 319
#define DM1_V1_CPSAO_LINE3_Y0_PC34 62
#define DM1_V1_CPSAO_LINE3_Y1_PC34 73

#define DM1_V1_CPSAO_AVAILABLE_SYMBOL_COUNT_PC34 6
#define DM1_V1_CPSAO_AVAILABLE_SYMBOL_BASE_PC34 96
#define DM1_V1_CPSAO_AVAILABLE_SYMBOL_STEP_PC34 14
#define DM1_V1_CPSAO_AVAILABLE_SYMBOL_X0_PC34 225
#define DM1_V1_CPSAO_AVAILABLE_SYMBOL_Y_PC34 58

#define DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34 4
#define DM1_V1_CPSAO_CHAMPION_SYMBOL_STEP_PC34 9
#define DM1_V1_CPSAO_CHAMPION_SYMBOL_X0_PC34 232
#define DM1_V1_CPSAO_CHAMPION_SYMBOL_Y_PC34 70

/* F0393 champion-tab x0 columns from SPELDRAW.C (PC 3.4 path). */
#define DM1_V1_CPSAO_TAB_CHAMPION_0_X0_PC34 233
#define DM1_V1_CPSAO_TAB_CHAMPION_0_X1_PC34 277
#define DM1_V1_CPSAO_TAB_CHAMPION_1_X0_PC34 280
#define DM1_V1_CPSAO_TAB_CHAMPION_1_X1_PC34 291
#define DM1_V1_CPSAO_TAB_CHAMPION_2_X0_PC34 294
#define DM1_V1_CPSAO_TAB_CHAMPION_2_X1_PC34 305
#define DM1_V1_CPSAO_TAB_CHAMPION_3_X0_PC34 308
#define DM1_V1_CPSAO_TAB_CHAMPION_3_X1_PC34 319
#define DM1_V1_CPSAO_TAB_Y0_PC34 42
#define DM1_V1_CPSAO_TAB_CHAMPION_0_Y1_PC34 49
#define DM1_V1_CPSAO_TAB_OTHER_Y1_PC34 48

/* DEFS.H color ids used by F0397 / F0398. */
#define DM1_V1_CPSAO_COLOR_CYAN_PC34 4
#define DM1_V1_CPSAO_COLOR_BLACK_PC34 0

/* DEFS.H sentinel. */
#define DM1_V1_CPSAO_CHAMPION_NONE_PC34 (-1)

/* DEFS.H spell-area input command ids (referenced for completeness). */
#define DM1_V1_CPSAO_CMD_CLICK_SPELL_AREA_PC34 100
#define DM1_V1_CPSAO_CMD_RUNE_0_PC34 101
#define DM1_V1_CPSAO_CMD_RUNE_1_PC34 102
#define DM1_V1_CPSAO_CMD_RUNE_2_PC34 103
#define DM1_V1_CPSAO_CMD_RUNE_3_PC34 104
#define DM1_V1_CPSAO_CMD_RUNE_4_PC34 105
#define DM1_V1_CPSAO_CMD_RUNE_5_PC34 106
#define DM1_V1_CPSAO_CMD_RUNE_FIRST_PC34 DM1_V1_CPSAO_CMD_RUNE_0_PC34
#define DM1_V1_CPSAO_CMD_RUNE_LAST_PC34 DM1_V1_CPSAO_CMD_RUNE_5_PC34
#define DM1_V1_CPSAO_CMD_RECANT_PC34 107
#define DM1_V1_CPSAO_CMD_CAST_PC34 108
#define DM1_V1_CPSAO_CMD_SET_CASTER_PC34 109

/* DEFS.H graphic / zone ids for the spell-area background and lines. */
#define DM1_V1_CPSAO_GFX_SPELL_AREA_BACKGROUND_PC34 9
#define DM1_V1_CPSAO_GFX_SPELL_AREA_LINES_PC34 11
#define DM1_V1_CPSAO_ZONE_SPELL_AREA_PC34 13

typedef enum DM1_V1_CpsaoRejectReasonPc34 {
    DM1_V1_CPSAO_REJECT_NONE_PC34 = 0,
    DM1_V1_CPSAO_REJECT_SAME_CASTER_PC34 = 1,
    DM1_V1_CPSAO_REJECT_DEAD_CHAMPION_PC34 = 2,
    DM1_V1_CPSAO_REJECT_NO_CASTER_LIVE_PC34 = 3
} DM1_V1_CpsaoRejectReasonPc34;

typedef struct DM1_V1_ChampionPanelSpellAreaOverlayPc34Contract {
    int contract_only;
    int champion_count;
    int spell_area_x0;
    int spell_area_x1;
    int spell_area_y0;
    int spell_area_y1;
    int spell_area_width;
    int spell_area_height;
    int line2_x0;
    int line2_x1;
    int line2_y0;
    int line2_y1;
    int line3_x0;
    int line3_x1;
    int line3_y0;
    int line3_y1;
    int available_symbol_count;
    int available_symbol_base;
    int available_symbol_step;
    int available_symbol_x0;
    int available_symbol_y;
    int champion_symbol_max;
    int champion_symbol_step;
    int champion_symbol_x0;
    int champion_symbol_y;
    int tab_champion_0_x0;
    int tab_champion_0_x1;
    int tab_champion_1_x0;
    int tab_champion_1_x1;
    int tab_champion_2_x0;
    int tab_champion_2_x1;
    int tab_champion_3_x0;
    int tab_champion_3_x1;
    int tab_y0;
    int tab_champion_0_y1;
    int tab_other_y1;
    int color_cyan;
    int color_black;
    int champion_none;
    int gfx_spell_area_background;
    int gfx_spell_area_lines;
    int zone_spell_area;
    int command_click_in_spell_area;
    int command_set_caster;
    int command_cast;
    int command_recant;
    int command_rune_first;
    int command_rune_last;
    const char *caster_set_anchor;
    const char *spell_area_controls_anchor;
    const char *available_symbols_anchor;
    const char *champion_symbols_anchor;
    const char *lines_bitmap_anchor;
    const char *screen_box_anchor;
    const char *defs_anchor;
    const char *refresh_callers_anchor;
} DM1_V1_ChampionPanelSpellAreaOverlayPc34Contract;

typedef struct DM1_V1_ChampionPanelSpellAreaOverlayChampionPc34 {
    int index;
    int current_health; /* 0 = dead, otherwise alive */
    char symbols[DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34 + 1];
} DM1_V1_ChampionPanelSpellAreaOverlayChampionPc34;

typedef struct DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 {
    int previous_caster_index; /* G0514 before F0394 */
    int requested_caster_index; /* F0394 arg */
    int party_champion_count; /* G0305_ui_PartyChampionCount */
    unsigned int symbol_step; /* Champion->SymbolStep for the new caster */
    DM1_V1_ChampionPanelSpellAreaOverlayChampionPc34 champions
        [DM1_V1_CPSAO_CHAMPION_COUNT_PC34];
} DM1_V1_ChampionPanelSpellAreaOverlayInputPc34;

typedef struct DM1_V1_ChampionPanelSpellAreaOverlayLine1Pc34 {
    int champion_index;
    int tab_x0;
    int tab_x1;
    int tab_y0;
    int tab_y1;
    int highlighted; /* F0393_Main_HighlightScreenBox called */
    int present;     /* CurrentHealth > 0 && index < party_champion_count */
} DM1_V1_ChampionPanelSpellAreaOverlayLine1Pc34;

typedef struct DM1_V1_ChampionPanelSpellAreaOverlayLine2Pc34 {
    int symbol_index;
    int screen_x;
    int screen_y;
    int character; /* 96 + 6*SymbolStep + i */
    int color_cyan;
    int color_black;
} DM1_V1_ChampionPanelSpellAreaOverlayLine2Pc34;

typedef struct DM1_V1_ChampionPanelSpellAreaOverlayLine3Pc34 {
    int slot_index;
    int screen_x;
    int screen_y;
    char source_symbol; /* Symbols[i] if i < strlen, else 0 */
    char drawn_character; /* Symbols[i] if i < strlen, else ' ' */
    int from_champion_symbols;
} DM1_V1_ChampionPanelSpellAreaOverlayLine3Pc34;

typedef struct DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 {
    int valid;
    DM1_V1_CpsaoRejectReasonPc34 reject_reason;
    int post_caster_index;
    int cleared_to_black; /* CM1_CHAMPION_NONE path filled with C00 */
    int drew_background_graphic;
    int drew_lines_bitmap;
    int drew_spell_area_controls;
    int drew_available_symbols;
    int drew_champion_symbols;
    int new_caster_symbol_step;
    int new_caster_symbols_length;
    char new_caster_symbols[DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34 + 1];
    int tab_count;
    int available_symbol_count;
    int champion_symbol_count;
    DM1_V1_ChampionPanelSpellAreaOverlayLine1Pc34 line1
        [DM1_V1_CPSAO_CHAMPION_COUNT_PC34];
    DM1_V1_ChampionPanelSpellAreaOverlayLine2Pc34 line2
        [DM1_V1_CPSAO_AVAILABLE_SYMBOL_COUNT_PC34];
    DM1_V1_ChampionPanelSpellAreaOverlayLine3Pc34 line3
        [DM1_V1_CPSAO_CHAMPION_SYMBOL_MAX_PC34];
} DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34;

const DM1_V1_ChampionPanelSpellAreaOverlayPc34Contract *
dm1_v1_champion_panel_spell_area_overlay_contract_pc34(void);

const char *
dm1_v1_champion_panel_spell_area_overlay_source_evidence_pc34(void);

int dm1_v1_champion_panel_spell_area_overlay_plan_pc34(
    const DM1_V1_ChampionPanelSpellAreaOverlayInputPc34 *input,
    DM1_V1_ChampionPanelSpellAreaOverlayPlanPc34 *out_plan);

#ifdef __cplusplus
}
#endif

#endif
