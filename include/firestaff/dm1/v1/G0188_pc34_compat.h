#ifndef FIRESTAFF_DM1_V1_G0188_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0188_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 558 init var
 * G0188_aauc_Graphic558_FieldAspects[12][8].
 *
 * G0188 is the 12-row × 8-column field-aspect rendering table for the
 * dungeon viewport (rows = view-square positions, cols = {NativeBitmap,
 * BaseStartUnit, TransparentColor, Mask, ByteWidth, Height, X, BitPlaneWordCount}).
 *
 * Read sites: DUNVIEW.C F0007_MAIN_CopyBytes (per-field copy) +
 * F0113_DUNGEONVIEW_DrawField.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1024.
 */

#define DM1_V1_G0188_PC34_COMPAT_ROWS 12
#define DM1_V1_G0188_PC34_COMPAT_COLS 8
#define DM1_V1_G0188_PC34_COMPAT_SIZE 96

typedef struct DM1_V1_G0188ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0188_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int rowD3CTransparentColor;
    int rowD1LMaskByte;
    int rowD0LValid;
    int allRowsInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0188ResultPc34;

const unsigned char *
dm1_v1_g0188_table_pc34(void);

int
dm1_v1_g0188_size_pc34(void);

int
dm1_v1_g0188_get_pc34(int row_index, int col_index);

int
dm1_v1_g0188_run_pc34(
    DM1_V1_G0188ResultPc34 *out);

#endif
