#ifndef FIRESTAFF_DM1_V1_CHEST_ACTION_HAND_OWNER_CHANGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_ACTION_HAND_OWNER_CHANGE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 chest action-hand owner-change runtime regression.
 *
 * Lane:
 *   "Action hand owner change" - when the chest-owning champion's action
 *   hand item type transitions from CONTAINER (C09_THING_TYPE_CONTAINER)
 *   to a NON-CONTAINER (e.g. C04_THING_TYPE_WEAPON, C03_THING_TYPE_ARMOR,
 *   C0_THING_TYPE_GENERIC), the PANEL.C F0347 panel redraw that follows
 *   the F0302 slot-box click (CHAMPION.C F0302:702-712) must:
 *     1. close the live G0426 chest through CHEST.C F0334:113-132,
 *     2. preserve the visible G0425[0..7] items through the close,
 *     3. re-route the panel from M569_PANEL_CHEST to
 *        M565_PANEL_FOOD_WATER_POISONED because the new action hand
 *        item is not a container, and
 *     4. leave G0426 = NONE and G0425 cleared after the close.
 *
 * The reverse transition (NON-CONTAINER -> CONTAINER) is a separate
 * lane: a CONTAINER action hand landing after the close forces a fresh
 * F0333 reopen, which re-materializes the first eight linked items
 * and rewrites panelContent = M569_PANEL_CHEST. That reopen is the
 * companion piece covered by m11_inventory_open_chest_replacing_current
 * on the close-fresh path; this gate focuses on the
 *   CONTAINER -> NON-CONTAINER
 * action hand owner change while a G0426 chest is live, the precise
 * PANEL.C F0347:1639-1691 close-chest-first branch.
 *
 * ReDMCSB anchors:
 *   CHEST.C F0333:30-67     - open + same-open guard + G0426_T_OpenChest
 *                             write + M569_PANEL_CHEST assignment
 *                             (CHANGE7_27_FIX).
 *   CHEST.C F0334:113-132   - close path: clear G0426, rewrite G0425
 *                             into L1023/L1024 via F0163.
 *   CHAMPION.C F0297:243-268 - leader-hand put.
 *   CHAMPION.C F0298:270-298 - leader-hand remove.
 *   CHAMPION.C F0300:511-515 - slot remove through F0299.
 *   CHAMPION.C F0301:606-614 - slot add through F0299.
 *   CHAMPION.C F0302:662-714 - C537..C544 slot-box click dispatch.
 *   PANEL.C F0347:1639-1691 - close-chest-first redraw + G0424 panel
 *                             content re-route keyed by inventory
 *                             champion's action hand item type.
 *   DEFS.H                   - C08/C09/C30/C38/C537..C544/G0423/G0424
 *                             /G0425/G0426/M565/M569/M643
 *                             and C0..C9 thing-type enumeration.
 *
 * Source-locked contract-only marker; no real-asset or original-DOS
 * pixel parity claim. The visible-only contract is pinned to the
 * F0334:113-132 close path with a maximum of 8 visible items; the
 * hidden tail beyond the 8th item is intentionally out of scope.
 */

enum {
    DM1_PC34_CAOC_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CAOC_CHAMPION_COUNT = 4,
    DM1_PC34_CAOC_LEADER = 0,
    DM1_PC34_CAOC_INVENTORY_OWNER = 1,
    DM1_PC34_CAOC_OTHER_CHAMPION = 2,
    DM1_PC34_CAOC_DEAD_CHAMPION = 3,

    DM1_PC34_CAOC_OPEN_CHEST_THING = 0x6AC1,
    DM1_PC34_CAOC_FIRST_VISIBLE_ITEM = 0x7701,
    DM1_PC34_CAOC_FIRST_VISIBLE_WEIGHT = 21,
    DM1_PC34_CAOC_FIRST_VISIBLE_CHARGES = 7,

    DM1_PC34_CAOC_CONTAINER_ITEM = 0x6601,
    DM1_PC34_CAOC_CONTAINER_WEIGHT = 30,
    DM1_PC34_CAOC_CONTAINER_CHARGES = 0,

    DM1_PC34_CAOC_NON_CONTAINER_ITEM = 0x0501,
    DM1_PC34_CAOC_NON_CONTAINER_WEIGHT = 8,
    DM1_PC34_CAOC_NON_CONTAINER_CHARGES = 99,

    DM1_PC34_CAOC_THING_TYPE_GENERIC = 0,
    DM1_PC34_CAOC_THING_TYPE_WEAPON = 5,
    DM1_PC34_CAOC_THING_TYPE_CONTAINER = 9,

    DM1_PC34_CAOC_PANEL_FOOD_WATER_POISONED = 1,
    DM1_PC34_CAOC_PANEL_SCROLL = 5,
    DM1_PC34_CAOC_PANEL_CHEST = 6,
    DM1_PC34_CAOC_PANEL_INVENTORY = 0,

    DM1_PC34_CAOC_C537_ZONE = 537,
    DM1_PC34_CAOC_C544_ZONE = 544,
    DM1_PC34_CAOC_C037_SLOT_BOX = 37,
    DM1_PC34_CAOC_C044_SLOT_BOX = 44,

    DM1_PC34_CAOC_C0xFFFE_ENDOFLIST = 0xFFFE
};

typedef struct {
    int sourceLockedContractOnly;
    int assetFree;
    const char* disjointnessNote;
    const char* chestOpenAnchor;
    const char* chestCloseAnchor;
    const char* championHandPutAnchor;
    const char* championHandRemoveAnchor;
    const char* championSlotRemoveAnchor;
    const char* championSlotAddAnchor;
    const char* championSlotBoxClickAnchor;
    const char* panelRedrawAnchor;
    const char* defsAnchor;
    const char* thingTypeAnchor;
} DM1_V1_ChestActionHandOwnerChangeSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int assetFree;

    int leader;
    int inventoryOwner;
    int otherChampion;
    int deadChampion;
    int partyChampionCount;

    int openChestThing;
    int openResult;
    int openPanelContentBefore;
    int openPanelContentAfter;
    int openChampionBefore;
    int openChampionAfter;

    int initialActionHandItem;
    int initialActionHandAllowedSlots;
    int initialActionHandThingType;
    int initialPanelContent;

    int newActionHandItem;
    int newActionHandAllowedSlots;
    int newActionHandThingType;

    int visibleBeforeSlot0Type;
    int visibleBeforeSlot0Weight;
    int visibleBeforeSlot0Charges;
    int visibleBeforeSlot3Type;
    int visibleBeforeSlot3Weight;
    int visibleBeforeSlot3Charges;
    int visibleBeforeSlot7Type;
    int visibleBeforeSlot7Weight;
    int visibleBeforeSlot7Charges;
    int visibleCountBefore;

    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveC030Count;
    int f0301AddC030Count;
    int f0302SlotBoxClickCount;
    int f0347PanelRedrawCount;
    int f0299ObjectModifierApplyCount;
    int f0292DrawStateCount;

    int closedSlot0Type;
    int closedSlot0Weight;
    int closedSlot0Charges;
    int closedSlot3Type;
    int closedSlot3Weight;
    int closedSlot3Charges;
    int closedSlot7Type;
    int closedSlot7Weight;
    int closedSlot7Charges;
    int closedVisibleItemCount;
    int closedChainMatchesVisible;

    int actionHandTypeAfter;
    int actionHandAllowedSlotsAfter;
    int actionHandWeightAfter;
    int actionHandThingTypeAfter;
    int openChestThingAfterClose;
    int panelContentAfterClose;
    int panelContentReRoutedToFood;
    int panelContentDidNotStayAtChest;
    int panelContentDidNotStayAtScroll;

    int otherChampionActionHandBefore;
    int otherChampionActionHandAfter;
    int deadChampionActionHandBefore;
    int deadChampionActionHandAfter;
    int leaderHandBefore;
    int leaderHandAfter;

    int contractOnlyAssertions;
    int contractOnlyFailures;

    uint32_t deterministicHash;
} DM1_V1_ChestActionHandOwnerChangeProbePc34;

const char*
dm1_v1_chest_action_hand_owner_change_source_evidence_pc34(void);

const DM1_V1_ChestActionHandOwnerChangeSpecPc34*
dm1_v1_chest_action_hand_owner_change_spec_pc34(void);

int dm1_v1_chest_action_hand_owner_change_run_pc34(
    DM1_V1_ChestActionHandOwnerChangeProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_ACTION_HAND_OWNER_CHANGE_PC34_COMPAT_H */
