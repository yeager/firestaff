#ifndef FIRESTAFF_DM1_V1_BOXSCREENTOP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXSCREENTOP_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0061_ai_Graphic562_Box_ScreenTop[4] = {0, 319, 0, 32}.
 *
 * G0061 is the 4-int {L, R, T, B} box for the top status bar area
 * (above the dungeon viewport). PC 3.4 init = {0, 319, 0, 32}.
 * Read sites: VIDEO_Blit + M524_FillScreenBox for the status row
 * (champion icons, action icons).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-863.
 */

#define DM1_V1_BOX_SCREEN_TOP_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxScreenTopResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_SCREEN_TOP_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int left0;
    int right319;
    int top0;
    int bottom32;
    int leftLtRight;
    int topLtBottom;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_BoxScreenTopResultPc34;

const int *
dm1_v1_box_screen_top_table_pc34(void);

int
dm1_v1_box_screen_top_size_pc34(void);

int
dm1_v1_box_screen_top_get_pc34(int value_index);

int
dm1_v1_box_screen_top_run_pc34(
    DM1_V1_BoxScreenTopResultPc34 *out);

#endif