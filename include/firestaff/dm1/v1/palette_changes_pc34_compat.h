#ifndef FIRESTAFF_DM1_V1_PALETTECHANGES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTECHANGES_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G4011_auc_PaletteChanges[16].
 *
 * G4011 is the 16-entry palette-change table used by
 * F0129_VIDEO_BlitShrinkWithPaletteChanges to remap the source
 * bitmap's palette indices during cursor + champion-icon blits.
 * PC 3.4 init = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 0, 2, 1, 0}
 * (palette indices 15, 14, ..., 4 = downshift, 0, 2, 1, 0 = 
 * inverse-downshift for indices 12-15).
 *
 * Read sites: IO.C:2145/2152/2454 (cursor + champion-icon blit path).
 *
 * Disjoint from all pass784+ non-mirror-candidate contract gates.
 */

#define DM1_V1_PALETTE_CHANGES_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteChangesResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_CHANGES_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int entry0Palette15;
    int entry1Palette14;
    int entry11Palette4;
    int entry12Palette0;
    int entry13Palette2;
    int entry14Palette1;
    int entry15Palette0;
    int allEntriesInByteRange;
    int exactlyTwoZeroEntries;
    int entry12And15Collide;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_PaletteChangesResultPc34;

const unsigned char *
dm1_v1_palette_changes_table_pc34(void);

int
dm1_v1_palette_changes_size_pc34(void);

int
dm1_v1_palette_changes_get_pc34(int entry_index);

int
dm1_v1_palette_changes_run_pc34(
    DM1_V1_PaletteChangesResultPc34 *out);

#endif