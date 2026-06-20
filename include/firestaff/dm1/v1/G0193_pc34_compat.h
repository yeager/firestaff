#ifndef FIRESTAFF_DM1_V1_G0193_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0193_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0193_auc_Graphic558_FountainOrnamentIndices[1] (fountain ornament type indices).
 *
 * G0193 is the 1-entry ornament/init table for FountainOrnamentIndices.
 *
 * Read sites: DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap +
 * F0113_DUNGEONVIEW_DrawField.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1038.
 */

#define DM1_V1_G0193_PC34_COMPAT_SIZE 1

typedef struct DM1_V1_G0193ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0193_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0193ResultPc34;

const unsigned char *
dm1_v1_g0193_table_pc34(void);

int
dm1_v1_g0193_size_pc34(void);

int
dm1_v1_g0193_get_pc34(int entry_index);

int
dm1_v1_g0193_run_pc34(
    DM1_V1_G0193ResultPc34 *out);

#endif
