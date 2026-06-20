#ifndef FIRESTAFF_DM1_V1_G0207_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0207_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0207_aaauc_Graphic558_FloorOrnamentCoordinateSets[4][3][6]
 * (g0207 — floor ornament coordinate sets).
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1045.
 */

#define DM1_V1_G0207_PC34_COMPAT_SIZE 72

typedef struct DM1_V1_G0207ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0207_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0207ResultPc34;

const unsigned char *
dm1_v1_g0207_table_pc34(void);

int
dm1_v1_g0207_size_pc34(void);

int
dm1_v1_g0207_get_pc34(int set_index, int frame_index, int field_index);

int
dm1_v1_g0207_run_pc34(
    DM1_V1_G0207ResultPc34 *out);

#endif
