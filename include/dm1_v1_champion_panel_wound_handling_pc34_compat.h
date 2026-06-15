#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_PC34_COMPAT_H

/*
 * DM1 V1 champion panel wound hand slot-box contract.
 *
 * Source-lock anchors:
 * - ReDMCSB CHAMDRAW.C F0291:632-646 draws body slots C00..C05 through
 *   the slot-box selector and returns C034_GRAPHIC_SLOT_BOX_WOUNDED when
 *   Wounds has the selected slot bit. Lines 648-651 contain the later
 *   acting-action-hand override used by some media branches.
 * - ReDMCSB CHAMDRAW.C F0296:1185-1262 is the changed-object-icon cadence;
 *   lines 1226-1231 revisit status hand slot boxes 0..7 through M070 hand
 *   slot mapping while action-hand icon changes trigger action-icon redraw.
 * - ReDMCSB DEFS.H requested C033/C034/C035 anchor 2186-2188; local source
 *   has C033/C034/C035 at 2193-2195, C195 at 3799, M070 at 1878, and
 *   C211..C218 hand-slot zones at 3800-3807.
 * - ReDMCSB CHAMPION.C F0297:243-298 and F0298 update leader-hand state and
 *   call F0292 draw-state on leader load changes; CHAMPION.C F0302:662-714
 *   maps slot boxes through M070 and ends with F0292 redraw. F0292 is pulled
 *   in by CHAMPION.C:236-237 from CHAMDRAW.C and its wound/action cadence is
 *   at CHAMDRAW.C:937-955.
 * - ReDMCSB PANEL.C F0354:2195-2227 draws the status portrait zone; F0355
 *   champion switching closes/redraws the previous status box at 2316-2322
 *   and draws the new inventory slots/state at 2421-2433.
 *
 * Contract only: this file exposes the C034 wound branch decision and the
 * C033/C035 alternatives. It does not call real M11 graphics and does not
 * claim real-asset bitmap parity.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_CHAMPION_COUNT_PC34 4
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_HAND_COUNT_PC34 2
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_STATUS_BOX_SPACING_PC34 69
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_SLOT_BOX_SIZE_PC34 18
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_READY_HAND_PC34 0
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_ACTION_HAND_PC34 1
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_FIRST_BAR_GRAPH_ZONE_PC34 195
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_FIRST_HAND_ZONE_PC34 211
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_LAST_HAND_ZONE_PC34 218
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_NORMAL_PC34 33
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_WOUNDED_PC34 34
#define DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_ACTING_PC34 35

typedef enum DM1_ChampionPanel_WoundHandSlotBoxBranch {
    DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_NORMAL_PC34 = 0,
    DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_WOUNDED_PC34,
    DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_ACTING_PC34
} DM1_ChampionPanel_WoundHandSlotBoxBranch;

typedef struct DM1_ChampionPanel_WoundHandSlotBoxModel {
    int championIndex;
    int handIndex;
    int bodySlotIndex;
    int slotBoxIndex;
    int zoneId;
    int woundLevel;
    int isWoundAffected;
    int isActingChampion;
    int isActionHand;
    int woundBranchSuppressedByActing;
    int x;
    int y;
    int width;
    int height;
    int graphicId;
    DM1_ChampionPanel_WoundHandSlotBoxBranch branch;
} DM1_ChampionPanel_WoundHandSlotBoxModel;

int DM1_ChampionPanel_BuildWoundHandSlotBoxModel(
    int championIndex, int handIndex, int woundLevel, int isActingChampion,
    DM1_ChampionPanel_WoundHandSlotBoxModel *outModel);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_PC34_COMPAT_H */
