#ifndef FIRESTAFF_DM1_V1_CHEST_HAND_SWAP_NO_DUPLICATE_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_HAND_SWAP_NO_DUPLICATE_GATE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 chest <-> action-hand swap no-duplicate / no-loss chain gate.
 *
 * Source-locked against ReDMCSB:
 *   CHEST.C    F0333:30-67 + 70-74 (F0333_INVENTORY_OpenAndDrawChest) - G0426 +
 *              G0425_aT_ChestSlots / C537..C544
 *   CHEST.C    F0334:113-132 (F0334_INVENTORY_CloseChest) - close-loop
 *              relink through L1026_B_ProcessFirstChestSlot and
 *              F0163_DUNGEON_LinkThingToList
 *   CHAMPION.C F0297:243-268 (F0297_CHAMPION_PutObjectInLeaderHand) - put
 *              into leader hand and refresh load
 *   CHAMPION.C F0298:270-298 (F0298_CHAMPION_GetObjectRemovedFromLeaderHand) -
 *              remove from leader hand and refresh load
 *   CHAMPION.C F0300:511-584 (F0300_CHAMPION_GetObjectRemovedFromSlot) -
 *              G0425_aT_ChestSlots remove path for C30+
 *   CHAMPION.C F0301:606-660 (F0301_CHAMPION_AddObjectInSlot) - G0425 write
 *              path for C30+
 *   CHAMPION.C F0302:662-713 (F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox)
 *              - leader-hand <-> C30+ slot swap dispatcher; this is the
 *              "panel refresh orphan" path described in the BUG0_39
 *              CHANGE7_27_FIX history for the C30..C37 chest slots
 *   PANEL.C    F0347:1639-1691 - C09_SLOT_BOX_INVENTORY_ACTION_HAND click
 *              route to F0302
 *   DEFS.H     C08..C72 (status box slot indices), C30..C37 (chest slot
 *              mask), M569_PANEL_CHEST, M070_HAND_SLOT_INDEX, G0425,
 *              G0426, C0xFFFF_THING_NONE, C0xFFFE_THING_ENDOFLIST
 *
 * Contract:
 *   For every scenario (empty-hand pickup, full-hand drop, full swap, no-op
 *   empty/empty, full-hand/occupied-slot into the SAME item), the multiset
 *   union of (action-hand mouse item + 8 chest slot items) is preserved
 *   across the swap. The chain has no duplicate thing ids and no losses:
 *   each non-zero itemType appears the same number of times before and
 *   after the click. This pins the F0302:700-710 remove/put sequence so
 *   that a mid-click F0297 -> F0292 panel refresh cannot leak an object
 *   out of the visible chain.
 *
 * Disjoint from:
 *   - pass797 auto-chest action-hand swap during close (press-eye close)
 *   - pass706 chest slot-to-slot swap (chest-slot <-> chest-slot)
 *   - chest_occupied_slot_swap (occupied C538 swap, leader->non-leader
 *     hand receiver)
 *   - hand_swap_with_chest (per-slot click results, no chain multiset
 *     union)
 *   - chest_mid_close_hand_swap (mid-close manual F0334 swap)
 *   - chest_round_trip_hand_swap (open/close round-trip C537/C538)
 *   - c040_cancel_reopen_pickup and other mirror-candidate C040 lanes
 *   - scroll-wheel pickup / drop lanes
 *
 * This module is contract-only: it does not claim real-asset pixel parity
 * for the leader hand or chest panel.
 */

enum {
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_COUNT = 1,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE =
        DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_COUNT +
        DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_CHEST_THING = 0x7D80,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_REOPEN_THING = 0x7D81,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_ITEM = 5000,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_ITEM = 5001,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_ITEM = 5002,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_ITEM = 5003,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C541_ITEM = 5004,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM = 5100,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_HAND = 5002,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_CHEST = 5002,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_EMPTY_HAND_NONE = 0,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_NONE_SENTINEL = 0xFFFF,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_END_SENTINEL = 0xFFFE,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_ACTION_HAND_PC34_SLOT =
        DM1_PC34_SLOT_ACTION_HAND,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_2,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_3,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C541_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_5,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C542_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_6,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C543_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_7,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C544_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_8,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_INDEX = 0,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_INDEX = 1,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_INDEX = 2,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_INDEX = 3,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C541_INDEX = 4,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C542_INDEX = 5,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C543_INDEX = 6,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C544_INDEX = 7,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_HAND_TO_SLOT = 3,
    DM1_PC34_CHEST_HAND_SWAP_NO_DUP_ASSERTION_BUDGET = 96
};

typedef struct {
    const char* contractMarker;
    const char* disjointnessMarker;
    int unionSize;
    int slotCount;
    int actionHandPc34Slot;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestThing;
    int reopenThing;
    int c537Item;
    int c538Item;
    int c539Item;
    int c540Item;
    int c541Item;
    int handItem;
    int sameTypeHand;
    int sameTypeChest;
    int noneSentinel;
    int endSentinel;
    int actionHandMaskContainer;
    int assertionBudget;
} DM1_V1_ChestHandSwapNoDuplicateGateSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int disjointFromPass797;
    int disjointFromPass706;
    int disjointFromHandSwapWithChest;
    int disjointFromChestRoundTrip;
    int disjointFromScrollWheel;
    int disjointFromC040Mirror;

    int chestOpenResult;
    int chestOpenThing;
    int reopenResult;
    int reopenThing;
    int closedCount;
    int reopenedCount;

    int handTypeBeforeEmptyPickup;
    int handTypeAfterEmptyPickup;
    int handWeightAfterEmptyPickup;
    int slotTypeAfterEmptyPickup;
    int slotIndexEmptyPickup;
    int clickAcceptedEmptyPickup;
    int nonEmptyCountAfterEmptyPickup;
    int unionStableAfterEmptyPickup;
    int noDuplicateAfterEmptyPickup;

    int handTypeBeforeFullDrop;
    int handTypeAfterFullDrop;
    int handWeightAfterFullDrop;
    int slotTypeAfterFullDrop;
    int slotIndexFullDrop;
    int clickAcceptedFullDrop;
    int nonEmptyCountAfterFullDrop;
    int unionStableAfterFullDrop;
    int noDuplicateAfterFullDrop;

    int handTypeBeforeFullSwap;
    int handTypeAfterFullSwap;
    int handWeightAfterFullSwap;
    int slotTypeAfterFullSwap;
    int slotIndexFullSwap;
    int clickAcceptedFullSwap;
    int nonEmptyCountAfterFullSwap;
    int unionStableAfterFullSwap;
    int noDuplicateAfterFullSwap;

    int handTypeBeforeSameTypeClick;
    int handTypeAfterSameTypeClick;
    int slotTypeAfterSameTypeClick;
    int clickAcceptedSameTypeClick;
    int unionStableAfterSameTypeClick;
    int noDuplicateAfterSameTypeClick;
    int slotCountBeforeSameTypeClick;
    int slotCountAfterSameTypeClick;

    int handTypeBeforeEmptyEmptyNoop;
    int handTypeAfterEmptyEmptyNoop;
    int clickAcceptedEmptyEmptyNoop;
    int unionStableAfterEmptyEmptyNoop;
    int noDuplicateAfterEmptyEmptyNoop;

    int preClickUnionTypes[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
    int postSwapUnionTypes[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
    int postDropUnionTypes[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];
    int postSameTypeUnionTypes[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE];

    int preClickNonZeroCountEmptyPickup;
    int preClickNonZeroCountFullDrop;
    int preClickNonZeroCountFullSwap;
    int preClickNonZeroCountSameType;
    int preClickNonZeroCountEmptyEmptyNoop;
    int preClickNonZeroCountRoundTrip;
    int postEmptyPickupNonZeroCount;
    int postFullDropNonZeroCount;
    int postFullSwapNonZeroCount;
    int postSameTypeNonZeroCount;
    int postEmptyEmptyNoopNonZeroCount;

    int closedTypes[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT];
    int closedHandType;
    int reopenedTypes[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT];
    int reopenedHandType;

    int unionStableAcrossCloseReopen;
    int noDuplicateAcrossCloseReopen;
    int fullHandLoadedAfterReopen;
    int noOrphanItemAcrossRoundTrip;
} DM1_V1_ChestHandSwapNoDuplicateGateProbePc34;

extern const DM1_V1_ChestHandSwapNoDuplicateGateSpecPc34
    dm1_v1_chest_hand_swap_no_duplicate_gate_pc34_spec;

const char*
dm1_v1_chest_hand_swap_no_duplicate_gate_source_evidence_pc34(void);
const DM1_V1_ChestHandSwapNoDuplicateGateSpecPc34*
dm1_v1_chest_hand_swap_no_duplicate_gate_spec_pc34(void);
int dm1_v1_chest_hand_swap_no_duplicate_gate_run_pc34(
    DM1_V1_ChestHandSwapNoDuplicateGateProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_HAND_SWAP_NO_DUPLICATE_GATE_PC34_COMPAT_H */
