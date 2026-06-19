#ifndef FIRESTAFF_DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0014_ai_Graphic562_Box_Endgame_Restart_Inner[4].
 *
 * G0014 is the {X, Y, W, H} byte-coordinate sub-rectangle used by ENDGAME.C to fill the Restart Inner rect. Init value {105, 215, 147, 157}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEndgameRestartInnerResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs105;
    int yIs215;
    int wIs147;
    int hIs157;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEndgameRestartInnerResultPc34;

const int *
dm1_v1_endgame_restart_inner_table_pc34(void);

int
dm1_v1_endgame_restart_inner_size_pc34(void);

int
dm1_v1_endgame_restart_inner_get_pc34(int component, int *out_value);

int
dm1_v1_endgame_restart_inner_x_pc34(void);

int
dm1_v1_endgame_restart_inner_y_pc34(void);

int
dm1_v1_endgame_restart_inner_w_pc34(void);

int
dm1_v1_endgame_restart_inner_h_pc34(void);

int
dm1_v1_endgame_restart_inner_run_pc34(
    DM1_V1_BoxEndgameRestartInnerResultPc34 *out);

#endif
