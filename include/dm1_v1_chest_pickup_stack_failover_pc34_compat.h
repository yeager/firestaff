#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_STACK_FAILOVER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_STACK_FAILOVER_PC34_COMPAT_H

/*
 * DM1 V1 chest pickup stack failover regression gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-32 keeps an already-open chest from being rematerialized.
 * - CHEST.C F0333:53-67 dispatches the visible C537..C544 chest slots.
 * - CHEST.C F0334:117-132 rewires non-empty visible chest slots on close.
 * - OBJECT.C F0032/F0033:121-212 classifies object type/icons for scrolls
 *   and charge-bearing weapon variants used as stackable sentinels here.
 * - AMMO.C F0294:54-79 classifies quiver ammunition compatibility.
 * - BLITMASK.C F0133:30-33 redraws masked hand/icon pixels after changes.
 */

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_STACK_FAILOVER_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_STACK_FAILOVER_CHEST_THING = 0x7B50,
    DM1_PC34_CHEST_STACK_FAILOVER_BOLT = 0x5B01,
    DM1_PC34_CHEST_STACK_FAILOVER_SCROLL = 0x5B02,
    DM1_PC34_CHEST_STACK_FAILOVER_NON_STACK = 0x5B03,
    DM1_PC34_CHEST_STACK_FAILOVER_SENTINEL = 0x5B04,
    DM1_PC34_CHEST_STACK_FAILOVER_PACKED = 0x5B40,
    DM1_PC34_CHEST_STACK_FAILOVER_FREE_A =
        DM1_PC34_SLOT_BACKPACK_LINE2_2,
    DM1_PC34_CHEST_STACK_FAILOVER_FREE_B =
        DM1_PC34_SLOT_BACKPACK_LINE2_3
};

enum {
    DM1_PC34_CHEST_STACK_KIND_NONE = 0,
    DM1_PC34_CHEST_STACK_KIND_BOLT = 1,
    DM1_PC34_CHEST_STACK_KIND_SCROLL = 2
};

enum {
    DM1_PC34_CHEST_STACK_OUTCOME_REJECTED = 0,
    DM1_PC34_CHEST_STACK_OUTCOME_TO_HAND = 1,
    DM1_PC34_CHEST_STACK_OUTCOME_MERGED_HAND = 2,
    DM1_PC34_CHEST_STACK_OUTCOME_ROUTED_FREE_SLOT = 3
};

typedef struct {
    const char* f0333AlreadyOpenAnchor;
    const char* f0333DispatchAnchor;
    const char* f0334RewireAnchor;
    const char* objectClassifyAnchor;
    const char* ammoQuiverAnchor;
    const char* blitmaskHandRedrawAnchor;
    const char* sourceSummary;
    int expectedCaseCount;
    int expectedFreeInventorySlots;
    int targetChestPc34Slot;
    int sentinelChestPc34Slot;
} DM1_V1_ChestPickupStackFailoverSpecPc34;

typedef struct {
    int setupResult;
    int openResult;
    int openThing;
    int handBusyBefore;
    int handBeforeType;
    int handBeforeWeight;
    int handBeforeCharges;
    int handBeforeStackKind;
    int chestBeforeType;
    int chestBeforeWeight;
    int chestBeforeCharges;
    int chestBeforeStackKind;
    int sameStackBefore;
    int firstFreeSlot;
    int secondFreeSlot;
    int freeSlotCountBefore;
    int pickupResult;
    int outcome;
    int routedSlot;
    int routedSlotBeforeType;
    int routedSlotAfterType;
    int routedSlotAfterWeight;
    int routedSlotAfterCharges;
    int secondFreeSlotAfterType;
    int handAfterType;
    int handAfterWeight;
    int handAfterCharges;
    int handAfterStackKind;
    int chestAfterType;
    int chestAfterWeight;
    int chestAfterCharges;
    int sentinelBeforeType;
    int sentinelAfterType;
    int closeCount;
    int closedFirstType;
    int closedContainsPickedType;
    int closedContainsHeldType;
    int visibleWeightAfterPickup;
    int loadBeforePickup;
    int loadAfterPickup;
    int handPreserved;
    int pickedRemovedFromChest;
    int pickedPreservedSomewhere;
    int sentinelPreserved;
    int secondFreeSlotStayedFree;
} DM1_V1_ChestPickupStackFailoverCasePc34;

typedef struct {
    DM1_V1_ChestPickupStackFailoverCasePc34 nonStackHand;
    DM1_V1_ChestPickupStackFailoverCasePc34 sameStackHand;
    DM1_V1_ChestPickupStackFailoverCasePc34 differentStackHand;
    DM1_V1_ChestPickupStackFailoverCasePc34 emptyHand;
} DM1_V1_ChestPickupStackFailoverProbePc34;

const char*
dm1_v1_chest_pickup_stack_failover_source_evidence_pc34(void);
const DM1_V1_ChestPickupStackFailoverSpecPc34*
dm1_v1_chest_pickup_stack_failover_spec_pc34(void);
const DM1_V1_ChestPickupStackFailoverProbePc34*
dm1_v1_chest_pickup_stack_failover_last_probe_pc34(void);
int dm1_v1_chest_pickup_stack_failover_stack_kind_pc34(
    const M11_Item* item);
int dm1_v1_chest_pickup_stack_failover_click_pc34(
    M11_InventoryState* state,
    int champ,
    int chestSlotIndex,
    int* routedPc34SlotOut);
int dm1_v1_chest_pickup_stack_failover_run(int* passed, int* failed);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PICKUP_STACK_FAILOVER_PC34_COMPAT_H */
