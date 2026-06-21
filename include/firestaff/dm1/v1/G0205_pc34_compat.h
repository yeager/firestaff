#ifndef FIRESTAFF_DM1_V1_G0205_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0205_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0205_aaauc_Graphic558_WallOrnamentCoordinateSets[8][13][6].
 *
 * 8 sets × 13 frames × 6 fields = 624 bytes (wall ornament coords).
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1047.
 */

#define DM1_V1_G0205_PC34_COMPAT_SIZE 624

typedef struct DM1_V1_G0205ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0205_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0205ResultPc34;

const unsigned char *
dm1_v1_g0205_table_pc34(void);

int
dm1_v1_g0205_size_pc34(void);

int
dm1_v1_g0205_get_pc34(int set_index, int frame_index, int field_index);

int
dm1_v1_g0205_run_pc34(
    DM1_V1_G0205ResultPc34 *out);

#endif
