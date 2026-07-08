#ifndef FIRESTAFF_DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0000_ai_Graphic562_Box_SpellArea[4].
 *
 * G0000 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * CASTER.C M520_F0021_MAIN_BlitToScreen to draw the spell-area
 * background on the champion panel. The X coordinate is a byte
 * offset into the row, and Y is the byte width of the blit area.
 * Init value (DATA.C:119 + DATA.C:539): { 224, 319, 42, 74 }.
 *
 * Read sites:
 * - CASTER.C:24 M520_F0021_MAIN_BlitToScreen(C009_GRAPHIC_MENU_
 *   SPELL_AREA_BACKGROUND, G0000, C048_BYTE_WIDTH, ...) — blit the
 *   spell-area background.
 * - CASTER.C:31 M524_FillScreenBox(G0000, C00_COLOR_BLACK) — clear
 *   the spell area to black.
 * - STARTUP2.C:376 F0136_VIDEO_HatchScreenBox(G0000, C00_COLOR_BLACK)
 *   — hatch the spell area during startup.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9).
 * This gate is a non-mirror-candidate contract for the G0000
 * spell-area box.
 */

#define DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxSpellAreaResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs224;
    int yIs319;
    int wIs42;
    int hIs74;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int byteAligned;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxSpellAreaResultPc34;

typedef struct DM1_V1_SpellAreaRectPc34 {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_SpellAreaRectPc34;

typedef struct DM1_V1_SpellLabelSourceZonePc34 {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_SpellLabelSourceZonePc34;

typedef struct DM1_V1_SpellPanelStatePc34 {
    int active;
    int party_dead;
    int candidate_panel_active;
    int panel_open;
    int rune_row;
    int rune_count;
} DM1_V1_SpellPanelStatePc34;

typedef struct DM1_V1_SpellPanelReceiptPc34 {
    int accepted;
    int panel_open;
    int rune_row;
    int rune_count;
    int clear_runes;
    int append_rune;
    int rune_value;
    int rune_symbol_index;
    const char *rune_name;
} DM1_V1_SpellPanelReceiptPc34;

/*
 * ReDMCSB PC 3.x: CASTER.C F0394 draws C009_GRAPHIC_MENU_SPELL_AREA_LINES
 * through C013_ZONE_SPELL_AREA.  G0000 is the older full byte box used by
 * clear/hatch paths; the visible PC34 screen graphic is the 87-pixel C013
 * zone at x=233..319.
 */
enum {
    DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 = 9,
    DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 = 9,
    DM1_V1_SPELL_AREA_ZONE_ID_PC34 = 13,
    DM1_V1_SPELL_CASTER_PANEL_ZONE_ID_PC34 = 221,
    DM1_V1_SPELL_CASTER_TAB_ZONE_ID_PC34 = 224,
    DM1_V1_SPELL_AREA_CAST_ZONE_ID_PC34 = 252,
    DM1_V1_SPELL_AREA_RECANT_ZONE_ID_PC34 = 254,
    DM1_V1_SPELL_AVAILABLE_SYMBOL_PARENT_ZONE_ID_BASE_PC34 = 245,
    DM1_V1_SPELL_AVAILABLE_SYMBOL_ZONE_ID_BASE_PC34 = 255,
    DM1_V1_SPELL_CHAMPION_SYMBOL_ZONE_ID_BASE_PC34 = 261,

    DM1_V1_SPELL_RUNE_ROW_COUNT_PC34 = 4,
    DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34 = 6,
    DM1_V1_SPELL_RUNE_SEQUENCE_MAX_PC34 = 4,
    DM1_V1_SPELL_RUNE_VALUE_BASE_PC34 = 0x60,

    DM1_V1_SPELL_LABEL_CELL_W_PC34 = 14,
    DM1_V1_SPELL_LABEL_CELL_H_PC34 = 13,
    DM1_V1_SPELL_LABEL_AVAILABLE_Y_PC34 = 13,
    DM1_V1_SPELL_LABEL_SELECTED_Y_PC34 = 26
};

static inline DM1_V1_SpellAreaRectPc34
dm1_v1_spell_area_graphic_rect_pc34(void)
{
    DM1_V1_SpellAreaRectPc34 r = { 233, 42, 87, 33 };
    return r;
}

static inline DM1_V1_SpellAreaRectPc34
dm1_v1_spell_area_click_rect_pc34(void)
{
    DM1_V1_SpellAreaRectPc34 r = { 233, 42, 87, 33 };
    return r;
}

static inline DM1_V1_SpellAreaRectPc34
dm1_v1_spell_area_source_box_rect_pc34(void)
{
    DM1_V1_SpellAreaRectPc34 r = { 224, 42, 96, 33 };
    return r;
}

static inline DM1_V1_SpellAreaRectPc34
dm1_v1_spell_caster_panel_rect_pc34(void)
{
    DM1_V1_SpellAreaRectPc34 r = { 233, 42, 87, 8 };
    return r;
}

static inline DM1_V1_SpellAreaRectPc34
dm1_v1_spell_caster_tab_rect_pc34(void)
{
    DM1_V1_SpellAreaRectPc34 r = { 233, 42, 45, 8 };
    return r;
}

static inline int
dm1_v1_spell_available_symbol_parent_zone_id_pc34(int symbol_index)
{
    if (symbol_index < 0 || symbol_index >= 6) return 0;
    return DM1_V1_SPELL_AVAILABLE_SYMBOL_PARENT_ZONE_ID_BASE_PC34 +
           symbol_index;
}

static inline int
dm1_v1_spell_available_symbol_zone_id_pc34(int symbol_index)
{
    if (symbol_index < 0 || symbol_index >= 6) return 0;
    return DM1_V1_SPELL_AVAILABLE_SYMBOL_ZONE_ID_BASE_PC34 + symbol_index;
}

static inline int
dm1_v1_spell_champion_symbol_zone_id_pc34(int symbol_index)
{
    if (symbol_index < 0 || symbol_index >= 4) return 0;
    return DM1_V1_SPELL_CHAMPION_SYMBOL_ZONE_ID_BASE_PC34 + symbol_index;
}

static inline int
dm1_v1_spell_rune_value_pc34(int row, int symbol_index)
{
    if (row < 0 || row >= DM1_V1_SPELL_RUNE_ROW_COUNT_PC34 ||
        symbol_index < 0 ||
        symbol_index >= DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34) {
        return -1;
    }
    /*
     * ReDMCSB: SYMBOL.C F0399 stores Symbols[SymbolStep] as
     * 96 + (SymbolStep * 6) + SymbolIndex.
     */
    return DM1_V1_SPELL_RUNE_VALUE_BASE_PC34 +
           (row * DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34) +
           symbol_index;
}

static inline const char *
dm1_v1_spell_rune_name_pc34(int row, int symbol_index)
{
    static const char *const names
        [DM1_V1_SPELL_RUNE_ROW_COUNT_PC34]
        [DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34] = {
            { "LO",  "UM",  "ON",  "EE",   "PAL", "MON" },
            { "YA",  "VI",  "OH",  "FUL",  "DES", "ZO"  },
            { "VEN", "EW",  "KATH","IR",   "BRO", "GOR" },
            { "KU",  "ROS", "DAIN","NETA", "RA",  "SAR" }
        };
    if (row < 0 || row >= DM1_V1_SPELL_RUNE_ROW_COUNT_PC34 ||
        symbol_index < 0 ||
        symbol_index >= DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34) {
        return 0;
    }
    return names[row][symbol_index];
}

static inline int
dm1_v1_spell_rune_abbrev_pc34(int row, int symbol_index, char out[3])
{
    const char *name;
    if (!out) return 0;
    out[0] = '?';
    out[1] = '?';
    out[2] = '\0';
    name = dm1_v1_spell_rune_name_pc34(row, symbol_index);
    if (!name || !name[0]) return 0;
    out[0] = name[0];
    out[1] = name[1] ? name[1] : ' ';
    return 1;
}

static inline DM1_V1_SpellLabelSourceZonePc34
dm1_v1_spell_label_source_zone_pc34(int selected_line)
{
    DM1_V1_SpellLabelSourceZonePc34 zone = {
        0,
        selected_line ? DM1_V1_SPELL_LABEL_SELECTED_Y_PC34
                      : DM1_V1_SPELL_LABEL_AVAILABLE_Y_PC34,
        DM1_V1_SPELL_LABEL_CELL_W_PC34,
        DM1_V1_SPELL_LABEL_CELL_H_PC34
    };
    return zone;
}

static inline int
dm1_v1_spell_panel_command_allowed_pc34(
    const DM1_V1_SpellPanelStatePc34 *state)
{
    /*
     * ReDMCSB COMMAND.C:2302-2309, 2338, 2367 guards the C100 spell
     * command family with !G0299_ui_CandidateChampionOrdinal. Firestaff's
     * direct keyboard/API spell-panel paths share the same C040 boundary.
     */
    return state && state->active && !state->party_dead &&
           !state->candidate_panel_active;
}

static inline DM1_V1_SpellPanelReceiptPc34
dm1_v1_spell_panel_reject_pc34(const DM1_V1_SpellPanelStatePc34 *state)
{
    DM1_V1_SpellPanelReceiptPc34 receipt;
    receipt.accepted = 0;
    receipt.panel_open = state ? state->panel_open : 0;
    receipt.rune_row = state ? state->rune_row : 0;
    receipt.rune_count = state ? state->rune_count : 0;
    receipt.clear_runes = 0;
    receipt.append_rune = 0;
    receipt.rune_value = -1;
    receipt.rune_symbol_index = -1;
    receipt.rune_name = 0;
    return receipt;
}

static inline DM1_V1_SpellPanelReceiptPc34
dm1_v1_spell_panel_open_pc34(const DM1_V1_SpellPanelStatePc34 *state)
{
    DM1_V1_SpellPanelReceiptPc34 receipt =
        dm1_v1_spell_panel_reject_pc34(state);
    if (!dm1_v1_spell_panel_command_allowed_pc34(state)) return receipt;
    receipt.accepted = 1;
    receipt.panel_open = 1;
    receipt.rune_row = 0;
    receipt.rune_count = 0;
    receipt.clear_runes = 1;
    return receipt;
}

static inline DM1_V1_SpellPanelReceiptPc34
dm1_v1_spell_panel_close_pc34(const DM1_V1_SpellPanelStatePc34 *state)
{
    DM1_V1_SpellPanelReceiptPc34 receipt =
        dm1_v1_spell_panel_reject_pc34(state);
    if (!state || state->candidate_panel_active) return receipt;
    receipt.accepted = 1;
    receipt.panel_open = 0;
    receipt.rune_row = 0;
    receipt.rune_count = 0;
    receipt.clear_runes = 1;
    return receipt;
}

static inline DM1_V1_SpellPanelReceiptPc34
dm1_v1_spell_panel_clear_pc34(const DM1_V1_SpellPanelStatePc34 *state)
{
    DM1_V1_SpellPanelReceiptPc34 receipt =
        dm1_v1_spell_panel_reject_pc34(state);
    if (!state || !state->active || state->candidate_panel_active) {
        return receipt;
    }
    receipt.accepted = 1;
    receipt.panel_open = state->panel_open;
    receipt.rune_row = 0;
    receipt.rune_count = 0;
    receipt.clear_runes = 1;
    return receipt;
}

static inline DM1_V1_SpellPanelReceiptPc34
dm1_v1_spell_panel_enter_rune_pc34(
    const DM1_V1_SpellPanelStatePc34 *state,
    int symbol_index)
{
    DM1_V1_SpellPanelReceiptPc34 receipt =
        dm1_v1_spell_panel_reject_pc34(state);
    int rune_value;
    if (!dm1_v1_spell_panel_command_allowed_pc34(state) ||
        !state->panel_open ||
        state->rune_count >= DM1_V1_SPELL_RUNE_SEQUENCE_MAX_PC34) {
        return receipt;
    }
    rune_value = dm1_v1_spell_rune_value_pc34(state->rune_row, symbol_index);
    if (rune_value < 0) return receipt;

    /*
     * ReDMCSB SYMBOL.C F0399 appends the rune, then advances SymbolStep
     * modulo the four displayed spell rows.
     */
    receipt.accepted = 1;
    receipt.panel_open = 1;
    receipt.rune_row = state->rune_row + 1;
    if (receipt.rune_row >= DM1_V1_SPELL_RUNE_ROW_COUNT_PC34) {
        receipt.rune_row = DM1_V1_SPELL_RUNE_ROW_COUNT_PC34 - 1;
    }
    receipt.rune_count = state->rune_count + 1;
    receipt.append_rune = 1;
    receipt.rune_value = rune_value;
    receipt.rune_symbol_index = symbol_index;
    receipt.rune_name =
        dm1_v1_spell_rune_name_pc34(state->rune_row, symbol_index);
    return receipt;
}

const int *
dm1_v1_box_spell_area_table_pc34(void);

int
dm1_v1_box_spell_area_size_pc34(void);

int
dm1_v1_box_spell_area_get_pc34(int component, int *out_value);

int
dm1_v1_box_spell_area_x_pc34(void);

int
dm1_v1_box_spell_area_y_pc34(void);

int
dm1_v1_box_spell_area_w_pc34(void);

int
dm1_v1_box_spell_area_h_pc34(void);

int
dm1_v1_box_spell_area_run_pc34(
    DM1_V1_BoxSpellAreaResultPc34 *out);

#endif
