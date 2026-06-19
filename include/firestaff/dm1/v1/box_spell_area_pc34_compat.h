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