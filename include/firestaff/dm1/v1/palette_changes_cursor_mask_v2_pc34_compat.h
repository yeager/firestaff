#ifndef FIRESTAFF_DM1_V1_PALETTECHANGESCURSORMASKV2_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTECHANGESCURSORMASKV2_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G4013_auc_PaletteChanges_CursorMask[16].
 *
 * G4013 is the variant-2 cursor palette-change table used by
 * F0129_VIDEO_BlitShrinkWithPaletteChanges for the second-style cursor
 * mask blit (the "alternative" dark-cursor path used by the
 * F0129_VIDEO_BlitShrinkWithPaletteChanges call in IO.C:2151/2453).
 * PC 3.4 init = {15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0,
 * 15, 15, 15} — 15 non-zero palette indices (everywhere except
 * entry 12) which forms a near-uniform mask-blend.
 *
 * Read sites: IO.C:2151 (F0129_VIDEO_BlitShrinkWithPaletteChanges
 * with G4013 as the cursor palette-change table for the second
 * cursor style).
 *
 * Disjoint from all pass784+ non-mirror-candidate contract gates.
 */

#define DM1_V1_PALETTE_CHANGES_CURSOR_MASK_V2_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteChangesCursorMaskV2ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_CHANGES_CURSOR_MASK_V2_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int entry0Through11AllPalette15;
    int entry12Palette0;
    int entry13Through15Palette15;
    int allEntriesInByteRange;
    int exactlyOneZeroEntry;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_PaletteChangesCursorMaskV2ResultPc34;

const unsigned char *
dm1_v1_palette_changes_cursor_mask_v2_table_pc34(void);

int
dm1_v1_palette_changes_cursor_mask_v2_size_pc34(void);

int
dm1_v1_palette_changes_cursor_mask_v2_get_pc34(int entry_index);

int
dm1_v1_palette_changes_cursor_mask_v2_run_pc34(
    DM1_V1_PaletteChangesCursorMaskV2ResultPc34 *out);

#endif