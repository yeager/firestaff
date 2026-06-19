#ifndef FIRESTAFF_DM1_V1_PALETTE_CHANGES_NO_CHANGES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTE_CHANGES_NO_CHANGES_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0017_auc_Graphic562_PaletteChanges_NoChanges[16].
 *
 * G0017 is the 16-byte "no-op palette-changes" identity table used
 * by F0129_VIDEO_BlitShrinkWithPaletteChanges and friends. When a
 * caller wants to blit a bitmap without remapping any palette
 * indices, it passes G0017 as the PaletteChanges array (each byte
 * maps src_palette[i] -> dst_palette[i] for i = 0..15).
 *
 * PC 3.4 init (DATA.C:136): { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
 *                            12, 13, 14, 15 }  (true identity).
 *
 * Atari init (DATA.C:590): { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90,
 *                          100, 110, 120, 130, 140, 150 }  (10-step
 * ramp; Atari's 9-bit DAC palette remap).
 *
 * Read sites:
 * - ACTIDRAW.C:160 — creature action icon blit (no palette change)
 * - BLTSHRNK.C:530 — F0129_VIDEO_BlitShrinkWithPaletteChanges identity path
 * - DUNVIEW.C:4518 — explosion aspect blit (smoke/no-change branch)
 * - DUNVIEW.C:5973 — projectile aspect blit (no-change branch)
 * - STARTUP2.C:822 — G0075_apuc_PaletteChanges_Projectile[2]/[3] = G0017
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/801/
 * 802/803/804/805/806 (Graphics.dat init-table gates batches 1+2).
 * This gate is a non-mirror-candidate contract for the G0017
 * identity palette-changes table.
 */

#define DM1_V1_PALETTE_CHANGES_NO_CHANGES_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteChangesNoChangesResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_CHANGES_NO_CHANGES_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntry0;
    int lastEntry15;
    int allValuesInRange0to255;
    int isIdentityPermutation;
    int monotonicStrictlyIncreasing;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
    int isSameAsIdentityHelper;
} DM1_V1_PaletteChangesNoChangesResultPc34;

const unsigned char *
dm1_v1_palette_changes_no_changes_table_pc34(void);

int
dm1_v1_palette_changes_no_changes_size_pc34(void);

int
dm1_v1_palette_changes_no_changes_pc34(int palette_index);

int
dm1_v1_palette_changes_no_changes_is_identity_pc34(void);

int
dm1_v1_palette_changes_no_changes_run_pc34(
    DM1_V1_PaletteChangesNoChangesResultPc34 *out);

#endif