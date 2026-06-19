#ifndef FIRESTAFF_DM1_V1_PALETTE_CREDITS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTE_CREDITS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0019_aui_Graphic562_Palette_Credits[16].
 *
 * G0019 is the 16-color palette used for the Credits screen fade.
 * Each entry is a 12-bit RGB color (4 bits per channel) packed as
 * a uint16_t. PC 3.4 init (DATA.C:210):
 *   { 0x009, 0x0AA, 0xFF6, 0x840, 0xFF8, 0x000, 0x080, 0xA00,
 *     0xC84, 0xFFA, 0xF84, 0xFC0, 0xFA0, 0x000, 0x620, 0xFFC }
 *
 * Atari ST has a different palette (DATA.C:211 comment line).
 *
 * Read sites:
 * - ENDGAME.C:680  - F0436_STARTEND_FadeToPalette(G0019_...)
 * - ENTRANCE.C:1061 - F0436_STARTEND_FadeToPalette(G0019_...)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0019 credits-screen palette.
 */

#define DM1_V1_PALETTE_CREDITS_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteCreditsResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_CREDITS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntry0x009;
    int lastEntry0xFFC;
    int allValues12Bit;
    int entry0IsBlack;
    int entry5IsBlack;
    int entry13IsBlack;
    int entry15IsWhite;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_PaletteCreditsResultPc34;

const unsigned int *
dm1_v1_palette_credits_table_pc34(void);

int
dm1_v1_palette_credits_size_pc34(void);

int
dm1_v1_palette_credits_pc34(int palette_index);

int
dm1_v1_palette_credits_run_pc34(
    DM1_V1_PaletteCreditsResultPc34 *out);

#endif