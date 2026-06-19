#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_PANEL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_PANEL_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0032_ai_Graphic562_Box_Panel[4].
 *
 * G0032 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * PANEL.C and CHEST.C/REVIVE.C F0344_INVENTORY_DrawPanel to blit
 * the champion-panel backdrop graphic. The panel is the central
 * rectangular UI surface inside the champion-panel region. PC 3.4
 * init (DATA.C:314 + DATA.C:1031): { 80, 223, 52, 124 }.
 *
 * Read sites (this gate is the panel-rect contract; the source-
 * locked palette-name switch is in PANEL.C):
 * - PANEL.C:967 / 1140 / 1582 / 1600 / 1611 - blit C023/C020/C025/
 *   C040 panel graphics into G0032's box with byte width C072 and
 *   various colors (RED, DARK_GREEN, CYAN, etc.).
 * - CHEST.C - blit C025_GRAPHIC_PANEL_OPEN_CHEST into G0032.
 * - REVIVE.C - blit C027_GRAPHIC_PANEL_RENAME_CHAMPION into G0032.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0032 panel-backdrop blit rectangle.
 */

typedef struct DM1_V1_ChampionPanelBoxPanelResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs80;
    int yIs223;
    int wIs52;
    int hIs124;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
    int heightLargerThanWidth;
} DM1_V1_ChampionPanelBoxPanelResultPc34;

const int *
dm1_v1_champion_panel_box_panel_table_pc34(void);

int
dm1_v1_champion_panel_box_panel_get_pc34(int component, int *out_value);

int
dm1_v1_champion_panel_box_panel_x_pc34(void);

int
dm1_v1_champion_panel_box_panel_y_pc34(void);

int
dm1_v1_champion_panel_box_panel_w_pc34(void);

int
dm1_v1_champion_panel_box_panel_h_pc34(void);

int
dm1_v1_champion_panel_box_panel_run_pc34(
    DM1_V1_ChampionPanelBoxPanelResultPc34 *out);

#endif