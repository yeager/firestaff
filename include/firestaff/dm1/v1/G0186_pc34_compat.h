#ifndef FIRESTAFF_DM1_V1_G0186_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0186_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0186_s_Graphic558_Frames_Door_D1C (DOOR_FRAMES struct).
 *
 * DOOR_FRAMES = {ClosedOrDestroyed[8], Vertical[3][8],
 *                LeftHorizontal[3][8], RightHorizontal[3][8]}.
 * Total = 80 bytes per gate.
 *
 * Read sites: DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap +
 * F0113_DUNGEONVIEW_DrawField.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1026.
 */

#define DM1_V1_G0186_PC34_COMPAT_SIZE 80

typedef struct DM1_V1_G0186ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0186_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int closedOrDestroyedValid;
    int verticalFramesValid;
    int leftHorizontalFramesValid;
    int rightHorizontalFramesValid;
    int allBytesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0186ResultPc34;

const unsigned char *
dm1_v1_g0186_table_pc34(void);

int
dm1_v1_g0186_size_pc34(void);

int
dm1_v1_g0186_get_pc34(int frame_index, int value_index);

int
dm1_v1_g0186_run_pc34(
    DM1_V1_G0186ResultPc34 *out);

#endif
