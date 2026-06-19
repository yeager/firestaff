#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_ARROW_OR_EYE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_ARROW_OR_EYE_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0033_ai_Graphic562_Box_ArrowOrEye[4].
 *
 * G0033 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * PANEL.C F0344_INVENTORY_DrawPanel to blit the chest-content
 * eye/arrow indicator onto the inventory panel. The blit source
 * is C018_GRAPHIC_ARROW_FOR_CHEST_CONTENT (default) or
 * C019_GRAPHIC_EYE_FOR_OBJECT_DESCRIPTION (when the user is pressing
 * the eye icon).
 *
 * Init value (DATA.C:315 + DATA.C:1032): { 83, 98, 57, 65 }.
 *
 * Read site:
 * - PANEL.C:511 F0344_INVENTORY_DrawPanel — blit the arrow or eye
 *   graphic into G0033's box with byte width C008_BYTE_WIDTH and color
 *   C08_COLOR_RED.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0033 arrow/eye blit rectangle.
 */

typedef struct DM1_V1_ChampionPanelBoxArrowOrEyeResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs83;
    int yIs98;
    int wIs57;
    int hIs65;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
    int withinPanelRegion;
} DM1_V1_ChampionPanelBoxArrowOrEyeResultPc34;

const int *
dm1_v1_champion_panel_box_arrow_or_eye_table_pc34(void);

int
dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(int component, int *out_value);

int
dm1_v1_champion_panel_box_arrow_or_eye_x_pc34(void);

int
dm1_v1_champion_panel_box_arrow_or_eye_y_pc34(void);

int
dm1_v1_champion_panel_box_arrow_or_eye_w_pc34(void);

int
dm1_v1_champion_panel_box_arrow_or_eye_h_pc34(void);

int
dm1_v1_champion_panel_box_arrow_or_eye_run_pc34(
    DM1_V1_ChampionPanelBoxArrowOrEyeResultPc34 *out);

#endif