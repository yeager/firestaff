#ifndef FIRESTAFF_DM1_V1_G0224_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0224_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0224_aaaauc_Graphic558_CreatureCoordinateSets[3][11][5][2].
 *
 * 3 sets × 11 frames × 5 coords × 2 fields (X, Y) = 330 bytes.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1048.
 */

#define DM1_V1_G0224_PC34_COMPAT_SIZE 330

typedef struct DM1_V1_G0224ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0224_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0224ResultPc34;

const unsigned char *
dm1_v1_g0224_table_pc34(void);

int
dm1_v1_g0224_size_pc34(void);

int
dm1_v1_g0224_get_pc34(int set_index, int frame_index, int coord_index, int field_index);

int
dm1_v1_g0224_run_pc34(
    DM1_V1_G0224ResultPc34 *out);

#endif
