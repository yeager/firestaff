#ifndef FIRESTAFF_DM1_V1_INVENTORY_PANEL_HAND_STATUS_BOX_ROUTE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_PANEL_HAND_STATUS_BOX_ROUTE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Focused regression for the DM1 V1 inventory-panel status hand slot box
 * routing. The original game treats slot box indices 0..7 as the per-champion
 * ready/action hand buttons in the champion status row above the inventory
 * panel; indices >= 8 are owned by the inventory itself. This header models
 * the early F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox branch that
 * only depends on the slot box, party champion count, currently open
 * inventory champion, candidate champion flow, and per-champion current
 * health. It does not claim real-asset parity, real mouse, or any
 * chest/route behavior past the F0302:677-684 boundary. */

enum {
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_FIRST = 0,
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_LAST = 7,
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_COUNT = 8,
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_PARTY_LIMIT = M11_MAX_CHAMPIONS,
    DM1_V1_IPHSBR_INVENTORY_FIRST_SLOT_BOX = 8,
    DM1_V1_IPHSBR_THING_END = 0xFFFE,
    DM1_V1_IPHSBR_THING_NONE = 0,
    DM1_V1_IPHSBR_REJECTED = -1,
    DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT = 0x7001,
    DM1_V1_IPHSBR_HEAD_OBJECT = 0x7010,
    DM1_V1_IPHSBR_CHEST_OBJECT = 0x6601,
    DM1_V1_IPHSBR_SCROLL_OBJECT = 0x7701
};

typedef struct {
    int contractOnly;
    int statusSlotBoxFirst;
    int statusSlotBoxLast;
    int statusSlotBoxCount;
    int partyLimit;
    int inventoryFirstSlotBox;
    int thingEnd;
    int thingNone;
    int rejected;
    int foodWaterPoisonedObject;
    int headObject;
    int chestObject;
    int scrollObject;
    const char* f0302Anchor;
    const char* f0292Anchor;
    const char* chamdrawReadyHandAnchor;
    const char* chamdrawActionHandAnchor;
    const char* defsSlotBoxInventoryFirstAnchor;
    const char* defsHandSlotIndexAnchor;
    const char* scope;
} DM1_V1_InventoryPanelHandStatusBoxRouteSpecPc34;

typedef struct {
    int slotBoxIndex;
    int expectedChampionIndex;
    int expectedPc34SourceSlot;
    int expectedResolved;
    int resolvedChampionIndex;
    int resolvedPc34SourceSlot;
    int resolvedReturn;
    int leaderHandObjectBefore;
    int mouseItemTypeBefore;
    int slotItemTypeBefore;
    int clickResult;
    int leaderHandObjectAfter;
    int mouseItemTypeAfter;
    int slotItemTypeAfter;
    int chestObjectIconUnchangedInStatusBox;
    int slotBoxBelongsToStatusRow;
} DM1_V1_InventoryPanelHandStatusBoxRouteCasePc34;

typedef struct {
    int contractOnly;
    int healthyChampionCount;
    int totalAssertions;
    DM1_V1_InventoryPanelHandStatusBoxRouteCasePc34
        slotBoxes[DM1_V1_IPHSBR_STATUS_SLOT_BOX_COUNT];
    int negativeSlotBoxReturn;
    int negativeSlotBoxOutChampionIndex;
    int negativeSlotBoxOutPc34SourceSlot;
    int overlargeSlotBoxReturn;
    int overlargeSlotBoxOutChampionIndex;
    int overlargeSlotBoxOutPc34SourceSlot;
    int negativePartyCountReturn;
    int negativePartyCountOutChampionIndex;
    int negativePartyCountOutPc34SourceSlot;
    int overlargePartyCountReturn;
    int overlargePartyCountOutChampionIndex;
    int overlargePartyCountOutPc34SourceSlot;
    int nullHealthReturn;
    int nullHealthOutChampionIndex;
    int nullHealthOutPc34SourceSlot;
    int candidateChampionRejected;
    int candidateChampionOutChampionIndex;
    int candidateChampionOutPc34SourceSlot;
    int slotbox4ChampionAbovePartyReturn;
    int slotbox4ChampionAbovePartyOutChampionIndex;
    int slotbox4ChampionAbovePartyOutPc34SourceSlot;
    int inventoryChampion1Slotbox0Return;
    int inventoryChampion1Slotbox0OutChampionIndex;
    int inventoryChampion1Slotbox0OutPc34SourceSlot;
    int deadChampion0Return;
    int deadChampion0OutChampionIndex;
    int deadChampion0OutPc34SourceSlot;
    int slotbox3ReducesToChampion1ActionHand;
    int slotbox4ReducesToChampion2ReadyHand;
    int slotbox7ReducesToChampion3ActionHand;
    int slotbox0ReducesToChampion0ReadyHand;
    int inventoryFirstSlotBox;
    int clickOnDeadChampionLeavesMouseIntact;
    int deadChampionMouseItemTypeAfter;
    int deadChampionSlotItemTypeAfter;
    int clickOnAliveChampionMovesObject;
} DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34;

const char*
dm1_v1_inventory_panel_hand_status_box_route_source_evidence_pc34(void);
const DM1_V1_InventoryPanelHandStatusBoxRouteSpecPc34*
dm1_v1_inventory_panel_hand_status_box_route_spec_pc34(void);
int dm1_v1_inventory_panel_hand_status_box_route_pc34(
    DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_PANEL_HAND_STATUS_BOX_ROUTE_PC34_COMPAT_H */
