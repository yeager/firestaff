#ifndef FIRESTAFF_DM1_V1_G0208_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0208_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0208_aaauc_Graphic558_DoorButtonCoordinateSets[1][4][6].
 *
 * G0208 is the 1-set × 4-frame × 6-field door-button coordinate
 * table for the dungeon viewport (D3R, D3C, D2C, D1C frames).
 *
 * Read sites: DUNVIEW.C F0099/F0654.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1044.
 */

#define DM1_V1_G0208_PC34_COMPAT_SIZE 24

typedef struct DM1_V1_G0208ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0208_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0208ResultPc34;

const unsigned char *
dm1_v1_g0208_table_pc34(void);

int
dm1_v1_g0208_size_pc34(void);

int
dm1_v1_g0208_get_pc34(int frame_index, int field_index);

int
dm1_v1_g0208_run_pc34(
    DM1_V1_G0208ResultPc34 *out);

#endif
