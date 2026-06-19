#ifndef FIRESTAFF_DM1_V1_PALETTE_ENTRANCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTE_ENTRANCE_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0020_aui_Graphic562_Palette_Entrance[16].
 *
 * G0020 is the 16-color palette used for the Entrance screen fade.
 * Each entry is a 12-bit RGB color (4 bits per channel) packed as
 * a uint16_t. PC 3.4 init (DATA.C:213):
 *   { 0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0,
 *     0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF }
 *
 * Atari ST init (DATA.C:792): different palette values (see comment).
 *
 * Read site:
 * - ENTRANCE.C:595 F0436_STARTEND_FadeToPalette(G0020) — the entrance
 *   sequence fades from black to this palette as the player enters
 *   the dungeon.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819
 * (Graphics.dat init-table gates batches 1+2+3+4+5+6+7). This gate is
 * a non-mirror-candidate contract for the G0020 entrance-screen
 * palette.
 */

#define DM1_V1_PALETTE_ENTRANCE_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteEntranceResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_ENTRANCE_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntry0x000;
    int lastEntry0xFFF;
    int allValues12Bit;
    int entry0IsBlack;
    int entry15IsWhite;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_PaletteEntranceResultPc34;

const unsigned int *
dm1_v1_palette_entrance_table_pc34(void);

int
dm1_v1_palette_entrance_size_pc34(void);

int
dm1_v1_palette_entrance_pc34(int palette_index);

int
dm1_v1_palette_entrance_run_pc34(
    DM1_V1_PaletteEntranceResultPc34 *out);

#endif