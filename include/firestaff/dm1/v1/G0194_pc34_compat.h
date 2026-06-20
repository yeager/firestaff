#ifndef FIRESTAFF_DM1_V1_G0194_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0194_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices[60].
 *
 * G0194 is the 60-entry wall-ornament coordinate-set index table for
 * the dungeon viewport (each entry corresponds to one wall ornament
 * type, mapping to its FRAME coordinate set in G0205).
 *
 * Read sites: DUNVIEW.C F0113/F0100.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1038.
 */

#define DM1_V1_G0194_PC34_COMPAT_SIZE 60

typedef struct DM1_V1_G0194ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0194_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0194ResultPc34;

const unsigned char *
dm1_v1_g0194_table_pc34(void);

int
dm1_v1_g0194_size_pc34(void);

int
dm1_v1_g0194_get_pc34(int entry_index);

int
dm1_v1_g0194_run_pc34(
    DM1_V1_G0194ResultPc34 *out);

#endif
