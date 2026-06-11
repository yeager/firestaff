#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_ENCUMBRANCE_PARTIAL_STACK_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_ENCUMBRANCE_PARTIAL_STACK_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_K = 3,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER = 0,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TARGET = 1,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CHAMPION_COUNT = 2,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CAPACITY_N = 100,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BASE_LOAD = 40,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_CHEST_THING = 0x6720,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_BOTTOM = 0x6721,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_MIDDLE = 0x6722,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_TOP = 0x6723,
    DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_LEADER_HAND = 0x6724
};

typedef struct {
    int result;
    int blocked;
    int championIndex;
    int storagePc34Slot;
    int capacityN;
    int loadBefore;
    int itemWeight;
    int candidateLoad;
    int loadAfter;
    int stackCountBefore;
    int stackCountAfter;
    int topTypeBefore;
    int topTypeAfter;
    int bottomTypeBefore;
    int bottomTypeAfter;
    int pickupType;
    int pickupWasBottom;
    int nextPickupSeesCount;
    int nextPickupSeesBottomType;
    int nextPickupSeesTopType;
    int visibleWeightBefore;
    int visibleWeightAfter;
    int storageTypeAfter;
    int storageWeightAfter;
    int stackUnchangedAfterBlock;
    int leaderHandTypeBefore;
    int leaderHandTypeAfter;
    int slotTypesBefore[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
    int slotWeightsBefore[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
    int slotTypesAfter[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
    int slotWeightsAfter[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
} DM1_V1_ChestPickupEncumbrancePartialStackEventPc34;

typedef struct {
    int setupResult;
    int championCount;
    int leaderIndex;
    int targetChampionIndex;
    int targetIsNonLeader;
    int capacityN;
    int stackK;
    int openResult;
    int openChestThing;
    int initialCarriedLoad;
    int initialStackCount;
    int initialBottomType;
    int initialTopType;
    int initialVisibleWeight;
    int initialLeaderHandType;
    int initialTargetHandType;
    DM1_V1_ChestPickupEncumbrancePartialStackEventPc34 firstPickup;
    DM1_V1_ChestPickupEncumbrancePartialStackEventPc34 blockedPickup;
    int firstStoragePc34SlotTypeAfter;
    int secondStoragePc34SlotTypeAfter;
    int cancelResult;
    int cancelClosedCount;
    int cancelRemainingStackCount;
    int cancelRemainingBottomType;
    int cancelRemainingTopType;
    int cancelRemainingVisibleWeight;
    int cancelLeaderHandType;
    int cancelLeaderHandWeight;
    int cancelLeaderHandPreserved;
    int cancelTargetHandType;
    int cancelTargetHandPreserved;
    int cancelRemainingStackMatches;
    int cancelPickedItemStillStored;
    int cancelBlockedSlotStillEmpty;
    int cancelClosedTypes[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
    int cancelClosedWeights[DM1_PC34_CHEST_PICKUP_ENC_PARTIAL_STACK_SLOT_COUNT];
} DM1_V1_ChestPickupEncumbrancePartialStackProbePc34;

const char*
dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_source_evidence_pc34(
    void);
int dm1_v1_chest_pickup_encumbrance_partial_stack_runtime_run_pc34(
    DM1_V1_ChestPickupEncumbrancePartialStackProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PICKUP_ENCUMBRANCE_PARTIAL_STACK_RUNTIME_PC34_COMPAT_H */
