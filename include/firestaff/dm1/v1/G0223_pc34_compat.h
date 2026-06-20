#ifndef FIRESTAFF_DM1_V1_G0223_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0223_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0223_aac_Graphic558_ShiftSets[3][8] (PC 3.4 EN MEDIA365 branch, int16_t).
 *
 * G0223 is the 3-row x 8-column int16_t shift-set table for creature
 * viewport animation (D0 back, D1 back, D2 back offsets).
 *
 * Read sites: DUNVIEW.C F0099/F0654.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1043.
 */

#define DM1_V1_G0223_PC34_COMPAT_SIZE 24

typedef struct DM1_V1_G0223ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0223_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allInInt16Range;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0223ResultPc34;

const unsigned char *
dm1_v1_g0223_table_pc34(void);

int
dm1_v1_g0223_size_pc34(void);

int
dm1_v1_g0223_get_pc34(int row_index, int col_index);

int
dm1_v1_g0223_run_pc34(
    DM1_V1_G0223ResultPc34 *out);

#endif
