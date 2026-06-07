#ifndef FIRESTAFF_DM1_V1_INVENTORY_HAND_SWAP_WITH_CHEST_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_HAND_SWAP_WITH_CHEST_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_HAND_SWAP_WITH_CHEST_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_HAND_SWAP_WITH_CHEST_MAIN_THING = 0x7C30,
    DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_THING = 0x7C31,
    DM1_PC34_HAND_SWAP_WITH_CHEST_FIRST_ITEM = 3100,
    DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_HAND_ITEM = 3200,
    DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_FIRST_ITEM = 3300,
    DM1_PC34_HAND_SWAP_WITH_CHEST_OTHER_HAND_ITEM = 3400,
    DM1_PC34_HAND_SWAP_WITH_CHEST_STACK_CHARGES = 3,
    DM1_PC34_HAND_SWAP_WITH_CHEST_EMPTY_INDEX = 1,
    DM1_PC34_HAND_SWAP_WITH_CHEST_FULL_INDEX = 4
};

typedef struct {
    M11_InventoryState* inventory;
    int championIndex;
    int actionHandIndex;
    int expectedOpenChestThing;
} DM1_V1_InventoryHandSwapWithChestHandPc34;

typedef struct {
    int chestSlotIndex;
} DM1_V1_InventoryHandSwapWithChestSlotPc34;

typedef struct {
    int accepted;
    int openChestThingBefore;
    int pc34ChestSlot;
    int visibleWeightBefore;
    int visibleWeightAfter;
    M11_Item handBefore;
    M11_Item handAfter;
    M11_Item originalChestOccupant;
    M11_Item chestSlotAfter;
    int originalChestOccupantReturnedToHand;
    int previousHandStoredInChestSlot;
    int chestSlotCleared;
    int incompatibleRejected;
} DM1_V1_InventoryHandSwapWithChestResultPc34;

typedef struct {
    const char* contractMarker;
    int c09SlotBoxInventoryActionHand;
    int c145IconContainerChestOpen;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int c30ChestFirstSlot;
    int c37ChestLastSlot;
    int chestSlotMask;
    int actionHandIndex;
    int stackingNotApplicable;
} DM1_V1_InventoryHandSwapWithChestSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int mainOpenResult;
    int mainOpenThing;
    int emptySwapResult;
    int emptyHandBefore;
    int emptyHandAfter;
    int emptyOriginalChestOccupant;
    int emptyChestSlotAfter;
    int emptyClosedCount;
    int emptyReopenResult;
    int emptyReopenedSlotContainsNextVisible;
    int emptyReopenedOriginalStillInHand;

    int fullHandSetupResult;
    int fullSwapResult;
    int fullHandBefore;
    int fullOriginalChestOccupant;
    int fullHandAfter;
    int fullChestSlotAfter;
    int fullOriginalReturnedToHand;
    int fullPreviousHandStoredInSlot;
    int fullClosedCount;
    int fullReopenResult;
    int fullReopenedSlotContainsPreviousHand;

    int restoreSwapResult;
    int restoreReturnedPreviousHand;
    int restoreSlotOriginalOccupant;
    int restoreClosedCount;
    int restoreReopenResult;
    int restoreReopenedSlotOriginalOccupant;

    int incompatibleSwapResult;
    int incompatibleRejected;
    int incompatibleHandAfter;
    int incompatibleSlotAfter;

    int otherChampionOpenResult;
    int otherChampionSwapResult;
    int otherChampionHandAfter;
    int otherChampionSlotAfter;
    int mainChampionUnaffectedSlot;
    int nonActionHandRejected;
    int staleChestRejected;
    int chargesPreserved;
    int stackingNotApplicable;
    int noDuplicateTrackedItems;
} DM1_V1_InventoryHandSwapWithChestProbePc34;

extern const DM1_V1_InventoryHandSwapWithChestSpecPc34
    dm1_v1_inventory_hand_swap_with_chest_pc34_spec;

const char*
dm1_v1_inventory_hand_swap_with_chest_source_evidence_pc34(void);
const DM1_V1_InventoryHandSwapWithChestSpecPc34*
dm1_v1_inventory_hand_swap_with_chest_spec_pc34(void);
int M11_V1_Inventory_HandSwapWithChest_ApplyPc34(
    const DM1_V1_InventoryHandSwapWithChestHandPc34* hand,
    const DM1_V1_InventoryHandSwapWithChestSlotPc34* chestSlot,
    DM1_V1_InventoryHandSwapWithChestResultPc34* out);
int M11_V1_Inventory_HandSwapWithChest_RunPc34(
    DM1_V1_InventoryHandSwapWithChestProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_HAND_SWAP_WITH_CHEST_PC34_COMPAT_H */
