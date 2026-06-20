#ifndef FIRESTAFF_DM1_V1_PALETTETOPANDBOTTOMSCREEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTETOPANDBOTTOMSCREEN_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for global init var
 * G0347_aui_Palette_TopAndBottomScreen[16].
 *
 * G0347 is the 16-entry VGA palette for the top status bar + bottom
 * message-line strip of the screen. PC 3.4 EN init = {0x000, 0x666,
 * 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0, 0xF00, 0xFA0, 0xC86,
 * 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF}.
 *
 * Read sites: BASE.C:759 (G0347 load into registers D0-D7) +
 * DIALOG.C:866 (F0436_STARTEND_FadeToPalette) +
 * DRAWVIEW.C:790 (F0565_VIEWPORT_SetPalette).
 *
 * Disjoint from all pass784+ non-mirror-candidate contract gates.
 */

#define DM1_V1_PALETTE_TOP_AND_BOTTOM_SCREEN_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteTopAndBottomScreenResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_TOP_AND_BOTTOM_SCREEN_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int entry0Black;
    int entry1DarkGray;
    int entry15BrightWhite;
    int allEntriesInByteRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_PaletteTopAndBottomScreenResultPc34;

const unsigned int *
dm1_v1_palette_top_and_bottom_screen_table_pc34(void);

int
dm1_v1_palette_top_and_bottom_screen_size_pc34(void);

int
dm1_v1_palette_top_and_bottom_screen_get_pc34(int entry_index);

int
dm1_v1_palette_top_and_bottom_screen_run_pc34(
    DM1_V1_PaletteTopAndBottomScreenResultPc34 *out);

#endif