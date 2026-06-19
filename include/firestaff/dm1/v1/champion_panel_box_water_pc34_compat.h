#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_WATER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_WATER_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0036_ai_Graphic562_Box_Water[4].
 *
 * G0036 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * PANEL.C F0344_INVENTORY_DrawPanel to blit the water status label
 * onto the champion panel (paired with the food label G0035).
 *
 * Init value (DATA.C:318 + DATA.C:1036): { 112, 159, 83, 91 }.
 *
 * Read sites:
 * - PANEL.C:1586 F0344_INVENTORY_DrawPanel — blit
 *   C031_GRAPHIC_WATER_LABEL into G0036's box with byte width C024
 *   (English localization).
 * - PANEL.C:1590 — same with byte width C032 (German localization).
 * - PANEL.C:1594 — same with byte width C024 (French localization).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0036 water-label blit rectangle.
 */

typedef struct DM1_V1_ChampionPanelBoxWaterResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs112;
    int yIs159;
    int wIs83;
    int hIs91;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
    int xAlignedWithFoodLabel;
} DM1_V1_ChampionPanelBoxWaterResultPc34;

const int *
dm1_v1_champion_panel_box_water_table_pc34(void);

int
dm1_v1_champion_panel_box_water_get_pc34(int component, int *out_value);

int
dm1_v1_champion_panel_box_water_x_pc34(void);

int
dm1_v1_champion_panel_box_water_y_pc34(void);

int
dm1_v1_champion_panel_box_water_w_pc34(void);

int
dm1_v1_champion_panel_box_water_h_pc34(void);

int
dm1_v1_champion_panel_box_water_run_pc34(
    DM1_V1_ChampionPanelBoxWaterResultPc34 *out);

#endif