#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_FOOD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_FOOD_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0035_ai_Graphic562_Box_Food[4].
 *
 * G0035 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * PANEL.C F0344_INVENTORY_DrawPanel to blit the food status label
 * onto the champion panel.
 *
 * Init value (DATA.C:317): { 112, 159, 60, 68 }.
 *
 * Read sites:
 * - PANEL.C:1585 F0344_INVENTORY_DrawPanel — blit C030_GRAPHIC_FOOD_LABEL
 *   into G0035's box with byte width C024 (English localization).
 * - PANEL.C:1589 — same with byte width C032 (German localization).
 * - PANEL.C:1593 — same with byte width C048 (French localization).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index),
 * pass803 (ordered-cells), pass804 (charge-count-to-torch-type).
 * This gate is a non-mirror-candidate contract for the G0035
 * food-label blit rectangle.
 */

typedef struct DM1_V1_ChampionPanelBoxFoodResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs112;
    int yIs159;
    int wIs60;
    int hIs68;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
    int withinPanelLeftMargin;
} DM1_V1_ChampionPanelBoxFoodResultPc34;

const int *
dm1_v1_champion_panel_box_food_table_pc34(void);

int
dm1_v1_champion_panel_box_food_get_pc34(int component, int *out_value);

int
dm1_v1_champion_panel_box_food_x_pc34(void);

int
dm1_v1_champion_panel_box_food_y_pc34(void);

int
dm1_v1_champion_panel_box_food_w_pc34(void);

int
dm1_v1_champion_panel_box_food_h_pc34(void);

int
dm1_v1_champion_panel_box_food_run_pc34(
    DM1_V1_ChampionPanelBoxFoodResultPc34 *out);

#endif