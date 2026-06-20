#ifndef FIRESTAFF_DM1_V1_BOXCHAMPIONICONS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXCHAMPIONICONS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0054_ai_Graphic562_Box_ChampionIcons[16].
 *
 * G0054 is the 4-box array (16 int16_t = 4 boxes × {L, R, T, B}) for
 * the on-screen champion-icon rectangles. PC 3.4 init = {
 *   box 0: {281, 299,  0, 13},
 *   box 1: {301, 319,  0, 13},
 *   box 2: {301, 319, 15, 28},
 *   box 3: {281, 299, 15, 28}
 * }. Read sites: CHAMDRAW.C:830/1022/1025/1028 + CHAMPION.C:1656 +
 * IO.C:2433/2619/2677 (F0132_VIDEO_Blit + M524_FillScreenBox).
 *
 * Each box is {L, R, T, B} where L<R (horizontal extent) and T<B
 * (vertical extent). Box width is 18 pixels (R-L), height is 13 or
 * 13 (B-T) — the icons are 18×13/15 bpp-padded 4bpp bitmaps.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-859.
 */

#define DM1_V1_BOX_CHAMPION_ICONS_PC34_COMPAT_BOX_COUNT 4
#define DM1_V1_BOX_CHAMPION_ICONS_PC34_COMPAT_BOX_VALUES 4
#define DM1_V1_BOX_CHAMPION_ICONS_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_BoxChampionIconsResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_CHAMPION_ICONS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int box0Left281Right299;
    int box1Left301Right319;
    int box2Left301Right319Top15;
    int box3Left281Right299Top15;
    int allXInByteRange;
    int allYInByteRange;
    int allWidthsInRange;
    int allHeightsInRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_BoxChampionIconsResultPc34;

const int *
dm1_v1_box_champion_icons_table_pc34(void);

int
dm1_v1_box_champion_icons_size_pc34(void);

int
dm1_v1_box_champion_icons_get_pc34(int box_index, int value_index);

int
dm1_v1_box_champion_icons_box_count_pc34(void);

int
dm1_v1_box_champion_icons_run_pc34(
    DM1_V1_BoxChampionIconsResultPc34 *out);

#endif