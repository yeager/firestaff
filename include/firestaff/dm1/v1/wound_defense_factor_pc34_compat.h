#ifndef FIRESTAFF_DM1_V1_WOUNDDEFENSEFACTOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_WOUNDDEFENSEFACTOR_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0050_auc_Graphic562_WoundDefenseFactor[6].
 *
 * G0050 is the 6-entry wound-defense-factor table indexed by
 * wound position (HEAD=0, TORSO=1, LEGS=2, ARMS=3, FEET=4,
 * HAND=5). PC 3.4 init = {5, 5, 4, 6, 3, 1}. Read sites:
 * CHAMPION.C:1346 (F0312_CHAMPION_GetStrength * G0050[P0653_ui_WoundIndex]).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-852.
 */

#define DM1_V1_WOUND_DEFENSE_FACTOR_PC34_COMPAT_SIZE 6

typedef struct DM1_V1_WoundDefenseFactorResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_WOUND_DEFENSE_FACTOR_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int headFactor5;
    int torsoFactor5;
    int legsFactor4;
    int armsFactor6;
    int feetFactor3;
    int handFactor1;
    int allFactorsPositive;
    int allFactorsInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_WoundDefenseFactorResultPc34;

const unsigned char *
dm1_v1_wound_defense_factor_table_pc34(void);

int
dm1_v1_wound_defense_factor_size_pc34(void);

int
dm1_v1_wound_defense_factor_get_pc34(int wound_index);

int
dm1_v1_wound_defense_factor_run_pc34(
    DM1_V1_WoundDefenseFactorResultPc34 *out);

#endif