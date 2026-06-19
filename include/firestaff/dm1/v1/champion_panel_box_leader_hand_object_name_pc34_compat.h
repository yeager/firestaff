#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_LEADER_HAND_OBJECT_NAME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_BOX_LEADER_HAND_OBJECT_NAME_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0028_ai_Graphic562_Box_LeaderHandObjectName[4].
 *
 * G0028 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * OBJECT.C to draw the leader-hand object name black-fill rect.
 *
 * Init value (DATA.C:262 + DATA.C:923): { 233, 319, 33, 38 }.
 *
 * Read site:
 * - OBJECT.C:281 - M524_FillScreenBox(G0028, C00_COLOR_BLACK) draws
 *   the black backdrop for the leader-hand object name text.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0028 leader-hand-object-name black-fill
 * rectangle.
 */

typedef struct DM1_V1_ChampionPanelBoxLeaderHandObjectNameResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs233;
    int yIs319;
    int wIs33;
    int hIs38;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
} DM1_V1_ChampionPanelBoxLeaderHandObjectNameResultPc34;

const int *
dm1_v1_champion_panel_box_leader_hand_object_name_table_pc34(void);

int
dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(int component, int *out_value);

int
dm1_v1_champion_panel_box_leader_hand_object_name_x_pc34(void);

int
dm1_v1_champion_panel_box_leader_hand_object_name_y_pc34(void);

int
dm1_v1_champion_panel_box_leader_hand_object_name_w_pc34(void);

int
dm1_v1_champion_panel_box_leader_hand_object_name_h_pc34(void);

int
dm1_v1_champion_panel_box_leader_hand_object_name_run_pc34(
    DM1_V1_ChampionPanelBoxLeaderHandObjectNameResultPc34 *out);

#endif