#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_OBJECT_DESCRIPTION_CIRCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_OBJECT_DESCRIPTION_CIRCLE_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0034_ai_Graphic562_Box_ObjectDescriptionCircle[4].
 *
 * G0034 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * PANEL.C F0344_INVENTORY_DrawPanel to blit the object-description
 * circle graphic (a small circular indicator on the champion panel
 * that highlights the currently-selected inventory object).
 *
 * Init value (DATA.C:316 + DATA.C:1033): { 105, 136, 53, 79 }.
 *
 * Read site:
 * - PANEL.C:1141 F0344_INVENTORY_DrawPanel — blit
 *   C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE into G0034's box with
 *   byte width C016_BYTE_WIDTH and color C12_COLOR_DARKEST_GRAY.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0034 object-description-circle blit rectangle.
 */

typedef struct DM1_V1_ChampionPanelBoxObjectDescriptionCircleResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs105;
    int yIs136;
    int wIs53;
    int hIs79;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
    int heightLargerThanWidth;
} DM1_V1_ChampionPanelBoxObjectDescriptionCircleResultPc34;

const int *
dm1_v1_champion_panel_box_object_description_circle_table_pc34(void);

int
dm1_v1_champion_panel_box_object_description_circle_get_pc34(int component, int *out_value);

int
dm1_v1_champion_panel_box_object_description_circle_x_pc34(void);

int
dm1_v1_champion_panel_box_object_description_circle_y_pc34(void);

int
dm1_v1_champion_panel_box_object_description_circle_w_pc34(void);

int
dm1_v1_champion_panel_box_object_description_circle_h_pc34(void);

int
dm1_v1_champion_panel_box_object_description_circle_run_pc34(
    DM1_V1_ChampionPanelBoxObjectDescriptionCircleResultPc34 *out);

#endif