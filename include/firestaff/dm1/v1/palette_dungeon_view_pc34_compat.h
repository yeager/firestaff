#ifndef FIRESTAFF_DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0021_aaui_Graphic562_Palette_DungeonView[6][16].
 *
 * G0021 is the 6-row × 16-col palette table used by DRAWVIEW.C to
 * pick a dungeon-view palette based on the current light amount
 * (G0304_i_DungeonViewPaletteIndex 0..5 = brightest to darkest).
 * Each entry is a 12-bit RGB color (4 bits per channel) packed as
 * a uint16_t.
 *
 * PC 3.4 init (DATA.C:218-224, 6 rows of 16):
 *   row 0 (brightest, full color): { 0x000, 0x666, 0x888, 0x620,
 *                                   0x0CC, 0x840, 0x080, 0x0C0,
 *                                   0xF00, 0xFA0, 0xC86, 0xFF0,
 *                                   0x444, 0xAAA, 0x00F, 0xFFF }
 *   row 1: { 0x000, 0x444, 0x666, 0x620, 0x0CC, 0x820, 0x060, 0x0A0,
 *           0xC00, 0x000, 0x000, 0xFC0, 0x222, 0x888, 0x00C, 0xCCC }
 *   row 2: { 0x000, 0x222, 0x444, 0x420, 0x0CC, 0x620, 0x040, 0x080,
 *           0xA00, 0x000, 0x000, 0xFA0, 0x000, 0x666, 0x00A, 0xAAA }
 *   row 3: { 0x000, 0x000, 0x222, 0x200, 0x0CC, 0x420, 0x020, 0x060,
 *           0x800, 0x000, 0x000, 0xC80, 0x000, 0x444, 0x008, 0x888 }
 *   row 4: { 0x000, 0x000, 0x000, 0x000, 0x0CC, 0x200, 0x000, 0x040,
 *           0x600, 0x000, 0x000, 0xA60, 0x000, 0x222, 0x006, 0x666 }
 *   row 5 (darkest, almost black): { 0x000, 0x000, 0x000, 0x000,
 *           0x0CC, 0x000, 0x000, 0x020, 0x400, 0x000, 0x000, 0x640,
 *           0x000, 0x000, 0x004, 0x444 }
 *
 * Atari ST init (DATA.C:833-848): dimmer values.
 *
 * Read sites:
 * - BASE.C:968 - address-of for F0007_MAIN_CopyBytes (palette copying)
 * - DRAWVIEW.C:744/776/880/1040 - F0007_MAIN_CopyBytes(G0021[Idx], ...)
 *   selects the current palette based on G0304_i_DungeonViewPaletteIndex
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820
 * (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8). This gate
 * is a non-mirror-candidate contract for the G0021 dungeon-view
 * palette.
 */

#define DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_ROWS 6
#define DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_COLS 16
#define DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_SIZE \
    (DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_ROWS * \
     DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_COLS)

typedef struct DM1_V1_PaletteDungeonViewResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_DUNGEON_VIEW_PC34_COMPAT_SIZE];
    int tableRows;
    int tableCols;
    int tableMatchesDeclaration;
    int firstEntry0x000;
    int lastEntry0x444;
    int allValues12Bit;
    int row0IsBrightest;
    int row5IsDarkest;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_PaletteDungeonViewResultPc34;

const unsigned int *
dm1_v1_palette_dungeon_view_table_pc34(void);

int
dm1_v1_palette_dungeon_view_rows_pc34(void);

int
dm1_v1_palette_dungeon_view_cols_pc34(void);

int
dm1_v1_palette_dungeon_view_pc34(int row, int col);

int
dm1_v1_palette_dungeon_view_run_pc34(
    DM1_V1_PaletteDungeonViewResultPc34 *out);

#endif