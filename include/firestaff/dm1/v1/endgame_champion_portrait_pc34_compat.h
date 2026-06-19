#ifndef FIRESTAFF_DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0016_ai_Graphic562_Box_Endgame_ChampionPortrait[4].
 *
 * G0016 is the {X, Y, W, H} byte-coordinate sub-rectangle used by ENDGAME.C to blit the Champion Portrait endgame graphic. Init value {27, 58, 13, 41}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEndgameChampionPortraitResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs27;
    int yIs58;
    int wIs13;
    int hIs41;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEndgameChampionPortraitResultPc34;

const int *
dm1_v1_endgame_champion_portrait_table_pc34(void);

int
dm1_v1_endgame_champion_portrait_size_pc34(void);

int
dm1_v1_endgame_champion_portrait_get_pc34(int component, int *out_value);

int
dm1_v1_endgame_champion_portrait_x_pc34(void);

int
dm1_v1_endgame_champion_portrait_y_pc34(void);

int
dm1_v1_endgame_champion_portrait_w_pc34(void);

int
dm1_v1_endgame_champion_portrait_h_pc34(void);

int
dm1_v1_endgame_champion_portrait_run_pc34(
    DM1_V1_BoxEndgameChampionPortraitResultPc34 *out);

#endif
