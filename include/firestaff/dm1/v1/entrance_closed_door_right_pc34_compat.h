#ifndef FIRESTAFF_DM1_V1_BOXENTRANCECLOSEDDOORRIGHT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXENTRANCECLOSEDDOORRIGHT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0011_ai_Graphic562_Box_Entrance_ClosedDoorRight[4].
 *
 * G0011 is the {X, Y, W, H} byte-coordinate sub-rectangle used by ENTRANCE.C to blit the closed-door-right entrance graphic. Init value {105, 231, 30, 190}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXENTRANCECLOSEDDOORRIGHT_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEntranceClosedDoorRightResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXENTRANCECLOSEDDOORRIGHT_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs105;
    int yIs231;
    int wIs30;
    int hIs190;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEntranceClosedDoorRightResultPc34;

const int *
dm1_v1_entrance_closed_door_right_table_pc34(void);

int
dm1_v1_entrance_closed_door_right_size_pc34(void);

int
dm1_v1_entrance_closed_door_right_get_pc34(int component, int *out_value);

int
dm1_v1_entrance_closed_door_right_x_pc34(void);

int
dm1_v1_entrance_closed_door_right_y_pc34(void);

int
dm1_v1_entrance_closed_door_right_w_pc34(void);

int
dm1_v1_entrance_closed_door_right_h_pc34(void);

int
dm1_v1_entrance_closed_door_right_run_pc34(
    DM1_V1_BoxEntranceClosedDoorRightResultPc34 *out);

#endif
