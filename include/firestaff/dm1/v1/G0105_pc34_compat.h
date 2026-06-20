#ifndef FIRESTAFF_DM1_V1_G0105_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0105_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0105_auc_Graphic558_[4].
 */

#define DM1_V1_G0105_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_G0105ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0105_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0105ResultPc34;

const int *
dm1_v1_g0105_table_pc34(void);

int
dm1_v1_g0105_size_pc34(void);

int
dm1_v1_g0105_get_pc34(int value_index);

int
dm1_v1_g0105_run_pc34(
    DM1_V1_G0105ResultPc34 *out);

#endif
