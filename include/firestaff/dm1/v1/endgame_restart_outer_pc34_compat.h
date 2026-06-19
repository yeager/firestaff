#ifndef FIRESTAFF_DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0013_ai_Graphic562_Box_Endgame_Restart_Outer[4].
 *
 * G0013 is the {X, Y, W, H} byte-coordinate sub-rectangle used by ENDGAME.C to fill the Restart Outer rect. Init value {103, 217, 145, 159}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEndgameRestartOuterResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs103;
    int yIs217;
    int wIs145;
    int hIs159;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEndgameRestartOuterResultPc34;

const int *
dm1_v1_endgame_restart_outer_table_pc34(void);

int
dm1_v1_endgame_restart_outer_size_pc34(void);

int
dm1_v1_endgame_restart_outer_get_pc34(int component, int *out_value);

int
dm1_v1_endgame_restart_outer_x_pc34(void);

int
dm1_v1_endgame_restart_outer_y_pc34(void);

int
dm1_v1_endgame_restart_outer_w_pc34(void);

int
dm1_v1_endgame_restart_outer_h_pc34(void);

int
dm1_v1_endgame_restart_outer_run_pc34(
    DM1_V1_BoxEndgameRestartOuterResultPc34 *out);

#endif
