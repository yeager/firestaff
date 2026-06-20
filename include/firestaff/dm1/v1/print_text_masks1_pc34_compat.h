#ifndef FIRESTAFF_DM1_V1_PRINT_TEXT_MASKS1_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PRINT_TEXT_MASKS1_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0065_al_Graphic562_PrintTextMasks1[4].
 *
 * G0065 is the 4-entry 32-bit text-print mask table used by
 * F0040_TEXT_Print to mask the destination bitmap per character pixel
 * (alternating pixels per stride to support the Atari ST 4bpp
 * packed-pixel layout). PC 3.4 init = {
 *   0x7FFF7FFF, 0x3FFF3FFF, 0x1FFF1FFF, 0x0FFF0FFF
 * }.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-863.
 */

#define DM1_V1_PRINT_TEXT_MASKS1_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_PRINT_TEXT_MASKS1ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PRINT_TEXT_MASKS1_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_PRINT_TEXT_MASKS1ResultPc34;

const unsigned int *
dm1_v1_print_text_masks1_table_pc34(void);

int
dm1_v1_print_text_masks1_size_pc34(void);

unsigned int
dm1_v1_print_text_masks1_get_pc34(int mask_index);

int
dm1_v1_print_text_masks1_run_pc34(
    DM1_V1_PRINT_TEXT_MASKS1ResultPc34 *out);

#endif
