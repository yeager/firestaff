#ifndef FIRESTAFF_DM1_V1_G0192_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0192_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0192_auc_Graphic558_AlcoveOrnamentIndices[3] (alcove ornament type indices).
 *
 * G0192 is the 3-entry ornament/init table for AlcoveOrnamentIndices.
 *
 * Read sites: DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap +
 * F0113_DUNGEONVIEW_DrawField.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1038.
 */

#define DM1_V1_G0192_PC34_COMPAT_SIZE 3

typedef struct DM1_V1_G0192ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0192_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0192ResultPc34;

const unsigned char *
dm1_v1_g0192_table_pc34(void);

int
dm1_v1_g0192_size_pc34(void);

int
dm1_v1_g0192_get_pc34(int entry_index);

int
dm1_v1_g0192_run_pc34(
    DM1_V1_G0192ResultPc34 *out);

#endif
