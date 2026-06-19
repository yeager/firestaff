#ifndef FIRESTAFF_DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0002_ai_Graphic562_Box_MovementArrows[4].
 *
 * G0002 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * MENUDRAW.C to draw the movement-arrows graphic on the champion
 * panel. Init value (DATA.C:123 + DATA.C:543): { 224, 319, 124,
 * 168 }.
 *
 * Read sites:
 * - MENUDRAW.C:13 M520_F0021_MAIN_BlitToScreen(C013_GRAPHIC_MOVEMENT_
 *   ARROWS, G0002, C048_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY, 45)
 *   — blit the movement-arrows graphic.
 * - PANEL.C:2369 + STARTUP2.C:369 F0136_VIDEO_HatchScreenBox(G0002,
 *   C00_COLOR_BLACK) — hatch the movement-arrows region.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831 (Graphics.dat init-table gates batches 1+2+3+4+5+6+
 * 7+8+9+10). This gate is a non-mirror-candidate contract for
 * the G0002 movement-arrows box.
 */

#define DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxMovementArrowsResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs224;
    int yIs319;
    int wIs124;
    int hIs168;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxMovementArrowsResultPc34;

const int *
dm1_v1_box_movement_arrows_table_pc34(void);

int
dm1_v1_box_movement_arrows_size_pc34(void);

int
dm1_v1_box_movement_arrows_get_pc34(int component, int *out_value);

int
dm1_v1_box_movement_arrows_x_pc34(void);

int
dm1_v1_box_movement_arrows_y_pc34(void);

int
dm1_v1_box_movement_arrows_w_pc34(void);

int
dm1_v1_box_movement_arrows_h_pc34(void);

int
dm1_v1_box_movement_arrows_run_pc34(
    DM1_V1_BoxMovementArrowsResultPc34 *out);

#endif