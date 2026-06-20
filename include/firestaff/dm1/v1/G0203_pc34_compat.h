#ifndef FIRESTAFF_DM1_V1_G0203_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0203_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0203_auc_Graphic558_InscriptionLineY[4].
 *
 * G0203 is the 4-entry inscription-line Y-coordinate table for
 * 1-4 text lines (per text-line count, the Y position of the
 * inscription text). Source-locked against DUNVIEW.C:480/1049.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1041.
 */

#define DM1_V1_G0203_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_G0203ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0203_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0203ResultPc34;

const unsigned char *
dm1_v1_g0203_table_pc34(void);

int
dm1_v1_g0203_size_pc34(void);

int
dm1_v1_g0203_get_pc34(int entry_index);

int
dm1_v1_g0203_run_pc34(
    DM1_V1_G0203ResultPc34 *out);

#endif
