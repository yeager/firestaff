#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_POISONED_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_POISONED_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0037_ai_Graphic562_Box_Poisoned[4].
 *
 * G0037 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * PANEL.C F0344_INVENTORY_DrawPanel to blit the "POISONED" status
 * label onto the champion panel when the champion has an active
 * poison event.
 *
 * Init value (DATA.C:319 + DATA.C:1046): { 112, 207, 105, 119 }.
 *
 * Read site:
 * - PANEL.C:1603 F0344_INVENTORY_DrawPanel — if
 *   Champion->PoisonEventCount != 0, blit C032_GRAPHIC_POISONED_LABEL
 *   into G0037's box with byte width C048.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index),
 * pass803 (ordered-cells), pass804 (charge-count-to-torch-type),
 * pass805 (champion-panel-box-food). This gate is a non-mirror-
 * candidate contract for the G0037 poisoned-label blit rectangle.
 */

typedef struct DM1_V1_ChampionPanelBoxPoisonedResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs112;
    int yIs207;
    int wIs105;
    int hIs119;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
    int withinPanelLeftMargin;
    int poisonedBelowFoodLabel;
} DM1_V1_ChampionPanelBoxPoisonedResultPc34;

const int *
dm1_v1_champion_panel_box_poisoned_table_pc34(void);

int
dm1_v1_champion_panel_box_poisoned_get_pc34(int component, int *out_value);

int
dm1_v1_champion_panel_box_poisoned_x_pc34(void);

int
dm1_v1_champion_panel_box_poisoned_y_pc34(void);

int
dm1_v1_champion_panel_box_poisoned_w_pc34(void);

int
dm1_v1_champion_panel_box_poisoned_h_pc34(void);

int
dm1_v1_champion_panel_box_poisoned_run_pc34(
    DM1_V1_ChampionPanelBoxPoisonedResultPc34 *out);

#endif