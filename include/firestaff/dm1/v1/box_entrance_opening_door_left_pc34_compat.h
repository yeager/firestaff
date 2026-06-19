#ifndef FIRESTAFF_DM1_V1_BOX_ENTRANCE_OPENING_DOOR_LEFT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_ENTRANCE_OPENING_DOOR_LEFT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0007_ai_Graphic562_Box_Entrance_OpeningDoorLeft[4].
 *
 * G0007 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * ENTRANCE.C F0132_VIDEO_Blit to draw the entrance door-opening
 * animation frame (left half). Init value (DATA.C:133 + DATA.C:553):
 * { 0, 100, 0, 160 }.
 *
 * Read sites:
 * - ENTRANCE.C:149 M769_BOX_RIGHT(G0007) -= 4 — shrink the right edge
 *   of the left-door box each animation tick.
 * - ENTRANCE.C:189-195 F0132_VIDEO_Blit(L1394_ppuc_Bitmap_
 *   EntranceDoorAnimationSteps[L1393_ui_AnimationStep & 0x0003],
 *   L1394_ppuc_Bitmap_EntranceDoorAnimationSteps[9], G0007, ...) —
 *   blit the entrance door animation step into the left-door box.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834/835/836/837 (Graphics.dat init-table
 * gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15). This gate
 * is a non-mirror-candidate contract for the G0007 entrance-opening-
 * door-left box.
 */

#define DM1_V1_BOX_ENTRANCE_OPENING_DOOR_LEFT_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEntranceOpeningDoorLeftResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_ENTRANCE_OPENING_DOOR_LEFT_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs100;
    int wIs0;
    int hIs160;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEntranceOpeningDoorLeftResultPc34;

const int *
dm1_v1_box_entrance_opening_door_left_table_pc34(void);

int
dm1_v1_box_entrance_opening_door_left_size_pc34(void);

int
dm1_v1_box_entrance_opening_door_left_get_pc34(int component, int *out_value);

int
dm1_v1_box_entrance_opening_door_left_x_pc34(void);

int
dm1_v1_box_entrance_opening_door_left_y_pc34(void);

int
dm1_v1_box_entrance_opening_door_left_w_pc34(void);

int
dm1_v1_box_entrance_opening_door_left_h_pc34(void);

int
dm1_v1_box_entrance_opening_door_left_run_pc34(
    DM1_V1_BoxEntranceOpeningDoorLeftResultPc34 *out);

#endif