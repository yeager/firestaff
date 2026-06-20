#ifndef FIRESTAFF_DM1_V1_G0497_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0497_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0497_auc_Graphic560_ActionExperienceGain[44] (PC 3.4 EN).
 *
 * G0497 is the 44-entry action experience gain table (per action index).
 * Each entry is the experience points gained when the action is used
 * successfully. PC 3.4 EN uses the MEDIA728 branch values for BLOW HORN (1),
 * HEAL (5), CALM (1), BRANDISH (3) — the MEDIA359 branch (PC 3.4 default)
 * uses 0 for these.
 *
 * Read sites: MENU.C F0452 + F0412.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1037.
 */

#define DM1_V1_G0497_PC34_COMPAT_SIZE 44

typedef struct DM1_V1_G0497ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0497_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int entry0IsZero;
    int entry1IsBlock8;
    int allValuesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0497ResultPc34;

const unsigned char *
dm1_v1_g0497_table_pc34(void);

int
dm1_v1_g0497_size_pc34(void);

int
dm1_v1_g0497_get_pc34(int entry_index);

int
dm1_v1_g0497_run_pc34(
    DM1_V1_G0497ResultPc34 *out);

#endif
