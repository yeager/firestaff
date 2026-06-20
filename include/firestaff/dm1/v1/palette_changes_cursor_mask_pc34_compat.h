#ifndef FIRESTAFF_DM1_V1_PALETTECHANGESCURSORMASK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTECHANGESCURSORMASK_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G4010_auc_PaletteChanges_CursorMask[16].
 *
 * G4010 is the 16-entry palette-change table used by
 * F0129_VIDEO_BlitShrinkWithPaletteChanges for the dark-cursor mask
 * blit path. PC 3.4 init = {15, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0}. The first 4 entries are non-zero (palette indices
 * 15, 15), the rest are 0 (preserves the source pixels for the
 * mask-blend operation).
 *
 * Read site: IO.C:2144/2151 (F0129_VIDEO_BlitShrinkWithPaletteChanges
 * with G4010 as the palette-change table).
 *
 * Disjoint from all pass784+ non-mirror-candidate contract gates.
 */

#define DM1_V1_PALETTE_CHANGES_CURSOR_MASK_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteChangesCursorMaskResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_CHANGES_CURSOR_MASK_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int entry0Palette15;
    int entry1Zero;
    int entry2Zero;
    int entry3Zero;
    int entry4Palette15;
    int entry5Through15Zero;
    int allEntriesInByteRange;
    int exactlyTwoNonZeroEntries;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_PaletteChangesCursorMaskResultPc34;

const unsigned char *
dm1_v1_palette_changes_cursor_mask_table_pc34(void);

int
dm1_v1_palette_changes_cursor_mask_size_pc34(void);

int
dm1_v1_palette_changes_cursor_mask_get_pc34(int entry_index);

int
dm1_v1_palette_changes_cursor_mask_run_pc34(
    DM1_V1_PaletteChangesCursorMaskResultPc34 *out);

#endif