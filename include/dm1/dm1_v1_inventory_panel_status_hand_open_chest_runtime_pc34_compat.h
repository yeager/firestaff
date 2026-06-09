#ifndef FIRESTAFF_DM1_V1_INVENTORY_PANEL_STATUS_HAND_OPEN_CHEST_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_PANEL_STATUS_HAND_OPEN_CHEST_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Focused regression for the DM1 V1 inventory-panel status hand slot box
 * 0..7 reduction when the inventory panel is open AND a chest is open in
 * the panel.
 *
 * The original game dispatches status hand slot box 0..7 clicks through
 * CHAMPION.C F0302:677-684, which routes the click to a per-champion
 * ready/action hand before any F0302:685-710 swap runs. The inventory
 * panel's open-chest state is owned by CHEST.C F0333/F0334 and only
 * affects the C537..C544 chest slot boxes plus the C144 -> C145
 * action-hand icon swap (CHAMDRAW.C F0291). The two code paths must
 * remain orthogonal: opening a chest must not steal a status hand slot
 * box click, and a status hand slot box click must not close a chest.
 *
 * This header models the intersection by:
 *   1. resolving every status hand slot box 0..7 to its per-champion
 *      ready/action hand (F0302:677-684 boundary)
 *   2. exercising the open-chest action-hand icon swap (F0333:43-46
 *      + F0291) and the close-time clear of v1OpenChestThing
 *   3. confirming that the F0302:677-684 rejection rules (candidate
 *      champion flow, currently open inventory champion, dead champion)
 *      still apply while a chest is open
 *   4. confirming that the resolved champion's mouse-item swap is
 *      observable across the open/close cycle
 *   5. confirming that F0333 sets the PC 3.4 G0424 panel content to
 *      M569_PANEL_CHEST even when the requested chest is already open
 *
 * The test is contract-only synthetic. It does not claim real-asset
 * pixel parity, real mouse, or any chest/route behavior past the
 * F0333:43-67 + F0334:112-133 + F0302:677-684 + F0291 boundary. */

enum {
    DM1_V1_IPHSOC_PARTY_LIMIT = 4,
    DM1_V1_IPHSOC_LEADER_HAND_SCROLL = 0x7701,
    DM1_V1_IPHSOC_ACTION_HAND_CHEST = 0x6601,
    DM1_V1_IPHSOC_DAGGER = 0x0501,
    DM1_V1_IPHSOC_CLOSED_ICON = 144,
    DM1_V1_IPHSOC_OPEN_ICON = 145,
    DM1_V1_IPHSOC_STATUS_SLOT_BOX_FIRST = 0,
    DM1_V1_IPHSOC_STATUS_SLOT_BOX_LAST = 7,
    DM1_V1_IPHSOC_STATUS_SLOT_BOX_COUNT = 8
};

typedef struct {
    int contractOnly;
    const char* f0302Anchor;
    const char* f0333Anchor;
    const char* f0334Anchor;
    const char* f0347Anchor;
    const char* f0291Anchor;
    const char* defsAnchor;
    const char* scope;
} DM1_V1_InventoryPanelStatusHandOpenChestSpecPc34;

typedef struct {
    int slotBoxIndex;
    int expectedChampionIndex;
    int expectedPc34SourceSlot;
    int resolvedReturn;
    int resolvedChampionIndex;
    int resolvedPc34SourceSlot;
    int clickResult;
    int mouseItemTypeBefore;
    int mouseItemTypeAfter;
    int slotItemTypeBefore;
    int slotItemTypeAfter;
    int openChestThingBeforeClick;
    int openChestThingAfterClick;
    int openChestThingPreservedByClick;
    int leaderHandObjectBefore;
    int leaderHandObjectAfter;
} DM1_V1_InventoryPanelStatusHandOpenChestRowPc34;

typedef struct {
    int contractOnly;
    int chestOpenBefore;
    int chestOpenAfter;
    int actionHandIconBefore;
    int actionHandIconAfterOpen;
    int actionHandIconAfterClose;
    int openChestThingBefore;
    int openChestThingAfterOpen;
    int openChestThingAfterClose;
    int panelContentBeforeOpen;
    int panelContentAfterOpen;
    int panelContentAfterSameOpen;
    int panelContentAfterClose;
    int panelContentAfterCloseRedraw;
    int sameOpenChestResult;
    int leaderHandThingAfter;
    DM1_V1_InventoryPanelStatusHandOpenChestRowPc34
        rows[DM1_V1_IPHSOC_STATUS_SLOT_BOX_COUNT];
    int candidateChampionRejectedWithChestOpen;
    int deadChampionRejectedWithChestOpen;
    int slotbox4AbovePartyRejectedWithChestOpen;
    int slotbox0OpenInventoryChampionRejectedWithChestOpen;
    int slotbox0ResolvesChampion0ReadyHandWithChestOpen;
    int slotbox3ResolvesChampion1ActionHandWithChestOpen;
    int slotbox7ResolvesChampion3ActionHandWithChestOpen;
    int clickOnChampion0ReadyHandSwapsLeaderHand;
    int clickOnChampion1ActionHandSwapsLeaderHand;
    int clickOnChampion3ActionHandSwapsLeaderHand;
    int chestStillOpenAfterChampion0ReadyHandClick;
    int chestStillOpenAfterChampion3ActionHandClick;
    int chestStillOpenAfterCandidateChampionReject;
    int chestStillOpenAfterDeadChampionReject;
    int openChestThingAfterAllClicks;
    int readyHandClickLeavesActionHandChestInPlace;
    int readyHandClickKeepsOpenActionHandIcon;
    int actionHandClickMovesOpenChestToLeaderHand;
    int actionHandClickReplacesActionHandWithLeaderHandObject;
    int actionHandClickDropsActionHandIconToClosed;
    int actionHandClickPreservesOpenChestThing;
    int totalAssertions;
} DM1_V1_InventoryPanelStatusHandOpenChestProbePc34;

const char*
dm1_v1_inventory_panel_status_hand_open_chest_source_evidence_pc34(void);
const DM1_V1_InventoryPanelStatusHandOpenChestSpecPc34*
dm1_v1_inventory_panel_status_hand_open_chest_spec_pc34(void);
int dm1_v1_inventory_panel_status_hand_open_chest_pc34(
    DM1_V1_InventoryPanelStatusHandOpenChestProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_PANEL_STATUS_HAND_OPEN_CHEST_RUNTIME_PC34_COMPAT_H */
