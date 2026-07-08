#ifndef FIRESTAFF_DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0002_ai_Graphic562_Box_MovementArrows[4].
 *
 * G0002 is the {X1, X2, Y1, Y2} byte-coordinate sub-rectangle used by
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
#define DM1_V1_MOVEMENT_ARROW_COUNT_PC34 6

enum {
    DM1_V1_MOVEMENT_ARROW_INDEX_TURN_LEFT_PC34 = 0,
    DM1_V1_MOVEMENT_ARROW_INDEX_TURN_RIGHT_PC34 = 1,
    DM1_V1_MOVEMENT_ARROW_INDEX_FORWARD_PC34 = 2,
    DM1_V1_MOVEMENT_ARROW_INDEX_RIGHT_PC34 = 3,
    DM1_V1_MOVEMENT_ARROW_INDEX_BACKWARD_PC34 = 4,
    DM1_V1_MOVEMENT_ARROW_INDEX_LEFT_PC34 = 5
};

enum {
    DM1_V1_MOVEMENT_ARROW_COMMAND_TURN_LEFT_PC34 = 1,
    DM1_V1_MOVEMENT_ARROW_COMMAND_TURN_RIGHT_PC34 = 2,
    DM1_V1_MOVEMENT_ARROW_COMMAND_FORWARD_PC34 = 3,
    DM1_V1_MOVEMENT_ARROW_COMMAND_RIGHT_PC34 = 4,
    DM1_V1_MOVEMENT_ARROW_COMMAND_BACKWARD_PC34 = 5,
    DM1_V1_MOVEMENT_ARROW_COMMAND_LEFT_PC34 = 6
};

enum {
    DM1_V1_MOVEMENT_ARROW_VIS_TURN_LEFT_PC34 = 1 << 0,
    DM1_V1_MOVEMENT_ARROW_VIS_TURN_RIGHT_PC34 = 1 << 1,
    DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34 = 1 << 2,
    DM1_V1_MOVEMENT_ARROW_VIS_RIGHT_PC34 = 1 << 3,
    DM1_V1_MOVEMENT_ARROW_VIS_BACKWARD_PC34 = 1 << 4,
    DM1_V1_MOVEMENT_ARROW_VIS_LEFT_PC34 = 1 << 5,
    DM1_V1_MOVEMENT_ARROW_VIS_TICKS_PC34 = 4
};

typedef struct DM1_V1_MovementArrowRectPc34 {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_MovementArrowRectPc34;

typedef struct DM1_V1_MovementArrowVisualReceiptPc34 {
    int accepted;
    int arrowIndex;
    int visualMask;
    DM1_V1_MovementArrowRectPc34 rect;
    int cueColorKind;
} DM1_V1_MovementArrowVisualReceiptPc34;

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
dm1_v1_movement_arrows_zone_id_pc34(void);

int
dm1_v1_movement_arrows_graphic_id_pc34(void);

int
dm1_v1_movement_arrows_outer_rect_pc34(DM1_V1_MovementArrowRectPc34 *out);

int
dm1_v1_movement_arrows_graphic_rect_pc34(DM1_V1_MovementArrowRectPc34 *out);

int
dm1_v1_movement_arrow_zone_id_pc34(int arrow_index);

int
dm1_v1_movement_arrow_rect_pc34(int arrow_index,
                                DM1_V1_MovementArrowRectPc34 *out);

int
dm1_v1_movement_arrow_visual_mask_for_command_pc34(int command);

int
dm1_v1_movement_arrow_visual_receipt_pc34(
    int visual_mask,
    int arrow_index,
    DM1_V1_MovementArrowVisualReceiptPc34 *out);

int
dm1_v1_box_movement_arrows_run_pc34(
    DM1_V1_BoxMovementArrowsResultPc34 *out);

#endif
