#ifndef FIRESTAFF_DM1_V1_BOX_ENTRANCE_DOORS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_ENTRANCE_DOORS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0009_ai_Graphic562_Box_Entrance_Doors[4].
 *
 * G0009 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * ENTRANCE.C F0132_VIDEO_Blit to draw the entrance doors backdrop
 * (the closed-doors graphic under the door-opening animation).
 * Init value (DATA.C:137 + DATA.C:557): { 0, 231, 0, 160 }.
 * G1076 and G1133 partially replace G0009 for upper/lower half
 * animation steps.
 *
 * Read sites:
 * - ENTRANCE.C:529/538/541/544/547 F0132_VIDEO_Blit
 *   (G0562_apuc_Bitmap_EntranceDoorAnimationSteps[8], G0009, ...)
 *   — blit the entrance doors backdrop animation steps.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834/835/836/837/838/839 (Graphics.dat init-
 * table gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16).
 * This gate is a non-mirror-candidate contract for the G0009
 * entrance-doors box.
 */

#define DM1_V1_BOX_ENTRANCE_DOORS_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEntranceDoorsResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_ENTRANCE_DOORS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs231;
    int wIs0;
    int hIs160;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEntranceDoorsResultPc34;

const int *
dm1_v1_box_entrance_doors_table_pc34(void);

int
dm1_v1_box_entrance_doors_size_pc34(void);

int
dm1_v1_box_entrance_doors_get_pc34(int component, int *out_value);

int
dm1_v1_box_entrance_doors_x_pc34(void);

int
dm1_v1_box_entrance_doors_y_pc34(void);

int
dm1_v1_box_entrance_doors_w_pc34(void);

int
dm1_v1_box_entrance_doors_h_pc34(void);

int
dm1_v1_box_entrance_doors_run_pc34(
    DM1_V1_BoxEntranceDoorsResultPc34 *out);

#endif