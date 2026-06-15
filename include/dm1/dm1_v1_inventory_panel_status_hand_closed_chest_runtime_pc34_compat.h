#ifndef FIRESTAFF_DM1_V1_INVENTORY_PANEL_STATUS_HAND_CLOSED_CHEST_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_PANEL_STATUS_HAND_CLOSED_CHEST_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Focused regression for the DM1 V1 inventory-panel status hand slot box
 * 0..7 dispatch while the chest is in the closed state.
 *
 * The complementary open-chest slice lives in
 * dm1_v1_inventory_panel_status_hand_open_chest_runtime_pc34_compat.h and
 * asserts that status hand clicks through CHAMPION.C F0302:677-684 do not
 * disturb an open chest (G0426_T_OpenChest != 0, panel = M569_PANEL_CHEST,
 * C144 -> C145 action-hand icon swap). This header models the closed-chest
 * complement that no other gate covers together:
 *
 *   1. CHEST.C F0334:112-133 has run and cleared G0426_T_OpenChest,
 *      G0425_aT_ChestSlots[0..7] (all C0xFFFF_THING_NONE),
 *      G0424_i_PanelContent (rolled back to PANEL_INVENTORY / FOOD_WATER),
 *      and the C144 -> C145 action-hand icon binding in CHAMDRAW.C F0291
 *      (icon now C144 again).
 *   2. CHAMPION.C F0302:677-684 still resolves every status hand slot box
 *      0..7 to (championIndex = slotBoxIndex >> 1,
 *               slotIndex = M070_HAND_SLOT_INDEX(slotBox)).
 *   3. The F0302:685-710 leader-hand / slot swap path runs normally and
 *      never writes G0425 / G0426 / M569_PANEL_CHEST and never flips the
 *      C144 -> C145 action-hand icon.
 *   4. The F0302:677-684 reject rules (candidate champion flow, currently
 *      open inventory champion, dead champion, above-party champion) all
 *      still apply in the closed-chest state.
 *   5. The F0077 / F0078 mouse screen-update bracketing balances across
 *      the click and the F0292 redraw. The C033 / C034 / C035 slot-box
 *      graphic binding is unchanged from the open-chest baseline.
 *
 * The test is contract-only synthetic. It does not claim real-asset
 * pixel parity, real mouse, or any chest / route behavior past the
 * F0302:677-710 + F0333:43-67 + F0334:112-133 + F0291 boundary. */

enum {
    DM1_V1_IPHSCC_PARTY_LIMIT = 4,
    DM1_V1_IPHSCC_LEADER_HAND_SCROLL = 0x7701,
    DM1_V1_IPHSCC_CLOSED_CHEST_DAGGER = 0x0501,
    DM1_V1_IPHSCC_CLOSED_ICON = 144,
    DM1_V1_IPHSCC_OPEN_ICON = 145,
    DM1_V1_IPHSCC_NONE_THING = 0xFFFF,
    DM1_V1_IPHSCC_STATUS_SLOT_BOX_FIRST = 0,
    DM1_V1_IPHSCC_STATUS_SLOT_BOX_LAST = 7,
    DM1_V1_IPHSCC_STATUS_SLOT_BOX_COUNT = 8,
    DM1_V1_IPHSCC_CHEST_SLOT_COUNT = 8,
    DM1_V1_IPHSCC_EXPECTED_HASH = 0x32F59547u
};

typedef struct {
    int contractOnly;
    const char* f0302Anchor;
    const char* f0333Anchor;
    const char* f0334Anchor;
    const char* f0291Anchor;
    const char* defsAnchor;
    const char* scope;
} DM1_V1_InventoryPanelStatusHandClosedChestSpecPc34;

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
    int g0425AllZeroAfterClick;
    int leaderHandObjectBefore;
    int leaderHandObjectAfter;
} DM1_V1_InventoryPanelStatusHandClosedChestRowPc34;

typedef struct {
    int contractOnly;
    int panelContentBeforeClick;
    int panelContentAfterAllClicks;
    int actionHandIconBefore;
    int actionHandIconAfterAllClicks;
    int openChestThingBefore;
    int openChestThingAfterAllClicks;
    int g0425AllZeroBefore;
    int g0425AllZeroAfterAllClicks;
    int f0077F0078BalancedAcrossClick;
    int f0334ClearsG0426OnInit;
    int f0334ClearsG0425OnInit;
    int f0334LeavesPanelInventoryOnInit;
    int f0334LeavesActionHandIconClosedOnInit;
    DM1_V1_InventoryPanelStatusHandClosedChestRowPc34
        rows[DM1_V1_IPHSCC_STATUS_SLOT_BOX_COUNT];
    int candidateChampionRejectedWithChestClosed;
    int deadChampionRejectedWithChestClosed;
    int slotbox6AbovePartyRejectedWithChestClosed;
    int slotbox0OpenInventoryChampionRejectedWithChestClosed;
    int slotbox0ResolvesChampion0ReadyHandWithChestClosed;
    int slotbox3ResolvesChampion1ActionHandWithChestClosed;
    int slotbox7ResolvesChampion3ActionHandWithChestClosed;
    int clickOnChampion0ReadyHandSwapsLeaderHand;
    int clickOnChampion1ActionHandSwapsLeaderHand;
    int clickOnChampion3ActionHandSwapsLeaderHand;
    int closedActionHandStaysClosedIconAfterStatusClick;
    int panelContentStaysInventoryAfterStatusClick;
    int g0425StaysZeroAcrossStatusClick;
    int openChestThingStaysZeroAcrossStatusClick;
    int slotbox0ClosedChestSelectsChampion0ReadyHand;
    int slotbox3ClosedChestSelectsChampion1ActionHand;
    int slotbox7ClosedChestSelectsChampion3ActionHand;
    int totalAssertions;
    unsigned int deterministicHash;
} DM1_V1_InventoryPanelStatusHandClosedChestProbePc34;

const char*
dm1_v1_inventory_panel_status_hand_closed_chest_source_evidence_pc34(void);
const DM1_V1_InventoryPanelStatusHandClosedChestSpecPc34*
dm1_v1_inventory_panel_status_hand_closed_chest_spec_pc34(void);
int dm1_v1_inventory_panel_status_hand_closed_chest_pc34(
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_PANEL_STATUS_HAND_CLOSED_CHEST_RUNTIME_PC34_COMPAT_H */
