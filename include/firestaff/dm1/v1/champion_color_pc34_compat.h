#ifndef FIRESTAFF_DM1_V1_CHAMPION_COLOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_COLOR_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0046_auc_Graphic562_ChampionColor[4].
 *
 * G0046 assigns each of the 4 champions a unique text color used
 * for leader/follower rendering, message-area text, and champion-
 * icon fill. Init value (DATA.C:423 + DATA.C:1095): { 7, 11, 8, 14 }.
 *
 * The 4 values are 4-bit EGA-style palette indices:
 *   champion 0 (leader)      = color 7  (LIGHT_GRAY)
 *   champion 1 (1st follower)= color 11 (LIGHT_CYAN)
 *   champion 2              = color 8  (LIGHT_RED)
 *   champion 3              = color 14 (LIGHT_YELLOW)
 *
 * Read sites:
 * - CHAMDRAW.C:48/51/60 - champion icon bitmap fill
 * - CHAMDRAW.C:300/342 - champion-portrait screen box fill
 * - CHAMDRAW.C:1022 - champion-icons area fill
 * - CHAMPION.C:986/1016/1052 - champion name text color
 * - REVIVE.C:868/872/887 - champion name text color in resurrect
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807 (Graphics.dat init-table gates
 * batches 1+2+3). This gate is a non-mirror-candidate contract
 * for the G0046 champion-color assignment table.
 */

#define DM1_V1_CHAMPION_COLOR_PC34_COMPAT_COUNT 4

typedef struct DM1_V1_ChampionColorResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_CHAMPION_COLOR_PC34_COMPAT_COUNT];
    int tableMatchesDeclaration;
    int leaderColorIs7;
    int firstFollowerColorIs11;
    int allColorsDistinct;
    int allColorsInRange0to15;
    int lookupFunctionInRange;
    int lookupOutOfRangeReturnsZero;
    int dispatchByChampionIndexCorrect;
} DM1_V1_ChampionColorResultPc34;

const unsigned char *
dm1_v1_champion_color_table_pc34(void);

int
dm1_v1_champion_color_size_pc34(void);

int
dm1_v1_champion_color_pc34(int champion_index);

int
dm1_v1_champion_color_leader_pc34(void);

int
dm1_v1_champion_color_run_pc34(
    DM1_V1_ChampionColorResultPc34 *out);

#endif