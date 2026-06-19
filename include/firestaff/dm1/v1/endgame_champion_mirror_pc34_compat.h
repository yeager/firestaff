#ifndef FIRESTAFF_DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0015_ai_Graphic562_Box_Endgame_ChampionMirror[4].
 *
 * G0015 is the {X, Y, W, H} byte-coordinate sub-rectangle used by ENDGAME.C to blit the Champion Mirror endgame graphic. Init value {11, 74, 7, 49}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEndgameChampionMirrorResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs11;
    int yIs74;
    int wIs7;
    int hIs49;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxEndgameChampionMirrorResultPc34;

const int *
dm1_v1_endgame_champion_mirror_table_pc34(void);

int
dm1_v1_endgame_champion_mirror_size_pc34(void);

int
dm1_v1_endgame_champion_mirror_get_pc34(int component, int *out_value);

int
dm1_v1_endgame_champion_mirror_x_pc34(void);

int
dm1_v1_endgame_champion_mirror_y_pc34(void);

int
dm1_v1_endgame_champion_mirror_w_pc34(void);

int
dm1_v1_endgame_champion_mirror_h_pc34(void);

int
dm1_v1_endgame_champion_mirror_run_pc34(
    DM1_V1_BoxEndgameChampionMirrorResultPc34 *out);

#endif
