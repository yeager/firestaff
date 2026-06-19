#ifndef FIRESTAFF_DM1_V1_PALETTE_INDEX_TO_LIGHT_AMOUNT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTE_INDEX_TO_LIGHT_AMOUNT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0040_ai_Graphic562_PaletteIndexToLightAmount[6].
 *
 * G0040 is the 6-entry palette-index-to-light-amount threshold table
 * used by PANEL.C F0337_INVENTORY_SetDungeonViewPalette. The values
 * {99, 75, 50, 25, 1, 0} are walked top-down by the source:
 *
 *   PaletteIndex = 0;
 *   while (*G0040++ > TotalLightAmount) PaletteIndex++;
 *
 * So palette 0 = brightest (TotalLightAmount >= 99), palette 5 =
 * darkest (TotalLightAmount = 0). TotalLightAmount = 99 picks
 * palette 0 (first match: 99 <= 99). TotalLightAmount = 100 walks
 * past all 6 (always <= 99..0 thresholds) and selects palette 5
 * because the loop exits before *G0040 reaches 0... wait, let's
 * read the source carefully:
 *
 *   PANEL.C:419-423 — AL1040_pi_LightAmount = G0040;
 *   if (TotalLightAmount > 0) {
 *       PaletteIndex = 0;
 *       while (*AL1040_pi_LightAmount++ > TotalLightAmount)
 *           PaletteIndex++;
 *   } else {
 *       PaletteIndex = 5;
 *   }
 *
 * For TotalLightAmount in [100, INF): 99 > 100? no -> PaletteIndex=0
 * For TotalLightAmount = 99: 99 > 99? no -> PaletteIndex=0
 * For TotalLightAmount = 76: 99 > 76? yes (PI=1), 75 > 76? no -> PI=1
 * For TotalLightAmount = 0: dark branch -> PI=5
 * For TotalLightAmount = 1: 99 > 1? yes (PI=1), 75 > 1? yes (PI=2),
 *                            50 > 1? yes (PI=3), 25 > 1? yes (PI=4),
 *                            1 > 1? no -> PI=4
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power). This gate is a non-mirror-
 * candidate contract for the G0040 palette-index threshold table.
 */

#define DM1_V1_PALETTE_INDEX_TABLE_PC34_COMPAT_SIZE 6

typedef struct DM1_V1_PaletteIndexToLightAmountResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTE_INDEX_TABLE_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntry99;
    int lastEntry0;
    int monotonicallyNonIncreasing;
    int allWithinRange0_99;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
    int selectPaletteIndexBrightestFor99Plus;
    int selectPaletteIndexDarkestFor0;
    int selectPaletteIndexBoundariesCorrect;
    int selectPaletteIndexBoundaryTests;
} DM1_V1_PaletteIndexToLightAmountResultPc34;

const int *
dm1_v1_palette_index_to_light_amount_table_pc34(void);

int
dm1_v1_palette_index_to_light_amount_size_pc34(void);

int
dm1_v1_palette_index_to_light_amount_pc34(int palette_index);

int
dm1_v1_palette_index_to_light_amount_select_pc34(int total_light_amount);

int
dm1_v1_palette_index_to_light_amount_brightest_index_pc34(void);

int
dm1_v1_palette_index_to_light_amount_darkest_index_pc34(void);

int
dm1_v1_palette_index_to_light_amount_brightest_threshold_pc34(void);

int
dm1_v1_palette_index_to_light_amount_run_pc34(
    DM1_V1_PaletteIndexToLightAmountResultPc34 *out);

#endif