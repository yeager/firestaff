#ifndef FIRESTAFF_DM1_V1_BOX_SCREEN_RIGHT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_SCREEN_RIGHT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0062_ai_Graphic562_Box_ScreenRight[4].
 *
 * G0062 is the 4-int {L, R, T, B} box for the right
 * screen area (the inventory/spell/action panel column).
 * PC 3.4 init = { 224, 319, 33, 169 }. Read sites:
 * M524_FillScreenBox + F0132_VIDEO_Blit for the right panel.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-863.
 */

#define DM1_V1_BOX_SCREEN_RIGHT_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BOX_SCREEN_RIGHTResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_SCREEN_RIGHT_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int leftOk;
    int rightOk;
    int topOk;
    int bottomOk;
    int leftLtRight;
    int topLtBottom;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_BOX_SCREEN_RIGHTResultPc34;

const int *
dm1_v1_box_screen_right_table_pc34(void);

int
dm1_v1_box_screen_right_size_pc34(void);

int
dm1_v1_box_screen_right_get_pc34(int value_index);

int
dm1_v1_box_screen_right_run_pc34(
    DM1_V1_BOX_SCREEN_RIGHTResultPc34 *out);

#endif
