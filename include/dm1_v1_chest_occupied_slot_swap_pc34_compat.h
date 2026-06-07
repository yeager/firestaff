#ifndef FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_OCCUPIED_SWAP_CASE_COUNT = 2,
    DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_CASE = 0,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_CASE = 1,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_SOURCE_INDEX = 2,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_DEST_PC34_SLOT =
        DM1_PC34_SLOT_BACKPACK_LINE1_2,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_SOURCE_INDEX = 0,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_INDEX = 0,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_CHEST_FIRST = 1100,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_POTION = 1102,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_HIDDEN_TAIL = 1108,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_REPLACEMENT = 1200,
    DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_DEST_OCCUPANT = 1210,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_FIRST = 2100,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON = 2100,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_HIDDEN_TAIL = 2103,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_REPLACEMENT = 2200,
    DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_OCCUPANT = 2210
};

typedef struct {
    int itemType;
    int weight;
    int allowedSlots;
} DM1_V1_ChestOccupiedSlotSwapItemPc34;

typedef struct {
    const char* contractMarker;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int caseCount;
    int backpackSourceIndex;
    int backpackDestinationPc34Slot;
    int chestBSourceIndex;
    int chestBDestinationIndex;
    DM1_V1_ChestOccupiedSlotSwapItemPc34 backpackPotion;
    DM1_V1_ChestOccupiedSlotSwapItemPc34 chestAWeapon;
} DM1_V1_ChestOccupiedSlotSwapSpecPc34;

typedef struct {
    int sourceChestThing;
    int destinationChestThing;
    int sourceSlotIndex;
    int destinationPc34Slot;
    int destinationChestSlotIndex;

    int worldHashBeforeResult;
    unsigned int worldHashBefore;
    int worldHashAfterResult;
    unsigned int worldHashAfter;
    int worldHashUnchanged;

    int leaderHandBefore;
    int leaderHandBeforeReinsert;
    int leaderHandAfterReinsert;
    int leaderHandStable;

    int sourceOpenResult;
    int sourceOpenThing;
    int sourceVisibleCountBefore;
    int sourceHiddenTailInput;
    int originalHead;
    int originalTail;
    int originalVisibleTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];

    int sourceSwapClickResult;
    int swappedObjectType;
    int sourceSlotAfterSourceSwap;
    int leaderHandAfterSourceSwap;
    int sourceReplacementType;
    int sourceReplacementStoredAtOriginalIndex;
    int afterSourceSwapTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];

    int destinationSwapClickResult;
    int destinationOriginalOccupant;
    int destinationAfterSwapType;
    int leaderHandAfterDestinationSwap;
    int swappedObjectInDestinationSlot;
    int destinationOccupantMovedToLeader;
    int afterDestinationSwapTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];

    int destinationReturnClickResult;
    int destinationAfterReturnType;
    int leaderHandAfterDestinationReturn;
    int destinationOccupantRestored;
    int swappedObjectReadyForReinsert;

    int sourceReopenResult;
    int sourceReinsertClickResult;
    int sourceSlotAfterReinsert;
    int replacementReturnedToLeader;
    int finalVisibleCount;
    int finalHead;
    int finalTail;
    int finalHeadMembershipCount;
    int finalVisibleTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];

    int sourceCloseCount;
    int closedTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int hiddenTailClosed;
    int sourceReopenAfterCloseResult;
    int reopenedVisibleCount;
    int reopenedTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int hiddenTailReopened;

    int destinationCloseCount;
    int destinationClosedTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];

    int visibleHeadUnchanged;
    int visibleTailUnchanged;
    int visibleOrderUnchanged;
    int reopenedOrderUnchanged;
    int noDuplicateObjectIds;
    int noEvictions;
} DM1_V1_ChestOccupiedSlotSwapCasePc34;

typedef struct {
    int sourceLockedContractOnly;
    DM1_V1_ChestOccupiedSlotSwapCasePc34
        cases[DM1_PC34_CHEST_OCCUPIED_SWAP_CASE_COUNT];
} DM1_V1_ChestOccupiedSlotSwapProbePc34;

extern const DM1_V1_ChestOccupiedSlotSwapSpecPc34
    dm1_v1_chest_occupied_slot_swap_pc34_spec;

const char* dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(void);
const DM1_V1_ChestOccupiedSlotSwapSpecPc34*
dm1_v1_chest_occupied_slot_swap_spec_pc34(void);
int dm1_v1_chest_occupied_slot_swap_pc34(
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H */
