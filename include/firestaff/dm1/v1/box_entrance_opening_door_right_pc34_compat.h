#ifndef FIRESTAFF_DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0008_ai_Graphic562_Box_Entrance_OpeningDoorRight[4].
 *
 * G0008 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * ENTRANCE.C F0132_VIDEO_Blit to draw the entrance door-opening
 * animation frame (right half). Init value (DATA.C:135 + DATA.C:555):
 * { 109, 231, 0, 160 }.
 *
 * Read sites:
 * - ENTRANCE.C:150 M768_BOX_LEFT(G0008) += 4 — expand the left edge
 *   of the right-door box each animation tick.
 * - ENTRANCE.C:190/192/195 F0132_VIDEO_Blit(L1394_ppuc_Bitmap_
 *   EntranceDoorAnimationSteps[L1393_ui_AnimationStep & 0x0003],
 *   L1394_ppuc_Bitmap_EntranceDoorAnimationSteps[9], G0008, ...) —
 *   blit the entrance door animation step into the right-door box.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834/835/836/837/838 (Graphics.dat init-table
 * gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16). This gate
 * is a non-mirror-candidate contract for the G0008 entrance-
 * opening-door-right box.
 */

#define DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEntranceOpeningDoorRightResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs109;
    int yIs231;
    int wIs0;
    int hIs160;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEntranceOpeningDoorRightResultPc34;

const int *
dm1_v1_box_entrance_opening_door_right_table_pc34(void);

int
dm1_v1_box_entrance_opening_door_right_size_pc34(void);

int
dm1_v1_box_entrance_opening_door_right_get_pc34(int component, int *out_value);

int
dm1_v1_box_entrance_opening_door_right_x_pc34(void);

int
dm1_v1_box_entrance_opening_door_right_y_pc34(void);

int
dm1_v1_box_entrance_opening_door_right_w_pc34(void);

int
dm1_v1_box_entrance_opening_door_right_h_pc34(void);

int
dm1_v1_box_entrance_opening_door_right_run_pc34(
    DM1_V1_BoxEntranceOpeningDoorRightResultPc34 *out);

#endif