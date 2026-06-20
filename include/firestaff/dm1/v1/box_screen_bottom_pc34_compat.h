#ifndef FIRESTAFF_DM1_V1_BOX_SCREEN_BOTTOM_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_SCREEN_BOTTOM_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0063_ai_Graphic562_Box_ScreenBottom[4].
 *
 * G0063 is the 4-int {L, R, T, B} box for the bottom
 * screen area (the message-line strip below the dungeon viewport).
 * PC 3.4 init = { 0, 319, 169, 199 }. Read sites:
 * M524_FillScreenBox + F0132_VIDEO_Blit for the bottom message strip.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-863.
 */

#define DM1_V1_BOX_SCREEN_BOTTOM_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BOX_SCREEN_BOTTOMResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_SCREEN_BOTTOM_PC34_COMPAT_SIZE];
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
} DM1_V1_BOX_SCREEN_BOTTOMResultPc34;

const int *
dm1_v1_box_screen_bottom_table_pc34(void);

int
dm1_v1_box_screen_bottom_size_pc34(void);

int
dm1_v1_box_screen_bottom_get_pc34(int value_index);

int
dm1_v1_box_screen_bottom_run_pc34(
    DM1_V1_BOX_SCREEN_BOTTOMResultPc34 *out);

#endif
