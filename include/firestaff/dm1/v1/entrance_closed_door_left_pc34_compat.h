#ifndef FIRESTAFF_DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0010_ai_Graphic562_Box_Entrance_ClosedDoorLeft[4].
 *
 * G0010 is the {X, Y, W, H} byte-coordinate sub-rectangle used by ENTRANCE.C to blit the closed-door-left entrance graphic. Init value {0, 104, 30, 190}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEntranceClosedDoorLeftResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs104;
    int wIs30;
    int hIs190;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEntranceClosedDoorLeftResultPc34;

const int *
dm1_v1_entrance_closed_door_left_table_pc34(void);

int
dm1_v1_entrance_closed_door_left_size_pc34(void);

int
dm1_v1_entrance_closed_door_left_get_pc34(int component, int *out_value);

int
dm1_v1_entrance_closed_door_left_x_pc34(void);

int
dm1_v1_entrance_closed_door_left_y_pc34(void);

int
dm1_v1_entrance_closed_door_left_w_pc34(void);

int
dm1_v1_entrance_closed_door_left_h_pc34(void);

int
dm1_v1_entrance_closed_door_left_run_pc34(
    DM1_V1_BoxEntranceClosedDoorLeftResultPc34 *out);

#endif
