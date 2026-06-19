#ifndef FIRESTAFF_DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0012_ai_Graphic562_Box_Endgame_TheEnd[4].
 *
 * G0012 is the {X, Y, W, H} byte-coordinate sub-rectangle used by ENDGAME.C to blit The End endgame graphic. Init value {120, 199, 95, 108}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEndgameTheEndResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs120;
    int yIs199;
    int wIs95;
    int hIs108;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEndgameTheEndResultPc34;

const int *
dm1_v1_endgame_the_end_table_pc34(void);

int
dm1_v1_endgame_the_end_size_pc34(void);

int
dm1_v1_endgame_the_end_get_pc34(int component, int *out_value);

int
dm1_v1_endgame_the_end_x_pc34(void);

int
dm1_v1_endgame_the_end_y_pc34(void);

int
dm1_v1_endgame_the_end_w_pc34(void);

int
dm1_v1_endgame_the_end_h_pc34(void);

int
dm1_v1_endgame_the_end_run_pc34(
    DM1_V1_BoxEndgameTheEndResultPc34 *out);

#endif
