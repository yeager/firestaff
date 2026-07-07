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

/*
 * ReDMCSB: CASTER.C F0394 draws C009 at the spell-area screen origin,
 * SPELDRAW.C F0393 owns the champion caster tab row, and DEFS.H exposes
 * C013/C221/C224 plus C009/C011.  The raw G0000 screen box remains
 * 224..319,42..74 for full clears/hatching; the visible PC spell-area
 * background is the native 87x25 graphic anchored at x=233,y=42, while
 * COMMAND.C routes the C100 click family through the taller 87x33 box.
 */
enum {
    DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 = 9,
    DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 = 11,
    DM1_V1_SPELL_AREA_ZONE_ID_PC34 = 13,
    DM1_V1_SPELL_CASTER_PANEL_ZONE_ID_PC34 = 221,
    DM1_V1_SPELL_CASTER_TAB_ZONE_ID_PC34 = 224,
    DM1_V1_SPELL_AREA_CAST_ZONE_ID_PC34 = 252,
    DM1_V1_SPELL_AREA_RECANT_ZONE_ID_PC34 = 254,
    DM1_V1_SPELL_AVAILABLE_SYMBOL_PARENT_ZONE_ID_BASE_PC34 = 245,
    DM1_V1_SPELL_AVAILABLE_SYMBOL_ZONE_ID_BASE_PC34 = 255,
    DM1_V1_SPELL_CHAMPION_SYMBOL_ZONE_ID_BASE_PC34 = 261
};

static inline DM1_V1_SpellAreaRectPc34
dm1_v1_spell_area_graphic_rect_pc34(void)
{
    DM1_V1_SpellAreaRectPc34 r = { 233, 42, 87, 25 };
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
