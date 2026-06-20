#ifndef FIRESTAFF_DM1_V1_G0163_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0163_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0163_aauc_Graphic558_Frame_Walls[12][8].
 *
 * G0163 is the 12-row × 8-column wall-frame coordinate table for the
 * dungeon-viewport wall rendering (rows 0-11 correspond to view-square
 * positions D0C, D0L, D0R, D1C, D1L, D1R, D2C, D2L, D2R, D3C, D3L, D3R).
 * Each entry is {X1, X2, Y1, Y2, ByteWidth, Height, X, Y}.
 *
 * Read sites: DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap +
 * F0113_DUNGEONVIEW_DrawField.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1024.
 */

#define DM1_V1_G0163_PC34_COMPAT_ROWS 12
#define DM1_V1_G0163_PC34_COMPAT_COLS 8
#define DM1_V1_G0163_PC34_COMPAT_SIZE 96

typedef struct DM1_V1_G0163ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0163_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int rowD0CD3CValid;        /* {0, 223, 0, 135, 0, 0, 0, 0} for D0C/D3C variants */
    int rowD1CD2CValid;
    int rowD3CD0CValid;        /* D0C row: {0, 223, 0, 135, 0, 0, 0, 0} */
    int allRowsInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0163ResultPc34;

const unsigned char *
dm1_v1_g0163_table_pc34(void);

int
dm1_v1_g0163_size_pc34(void);

int
dm1_v1_g0163_get_pc34(int row_index, int col_index);

int
dm1_v1_g0163_run_pc34(
    DM1_V1_G0163ResultPc34 *out);

#endif
