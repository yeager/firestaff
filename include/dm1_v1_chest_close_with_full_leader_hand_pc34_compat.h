#ifndef FIRESTAFF_DM1_V1_CHEST_CLOSE_WITH_FULL_LEADER_HAND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_CLOSE_WITH_FULL_LEADER_HAND_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C537_SLOT = DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C538_SLOT = DM1_PC34_SLOT_CHEST_2,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C539_SLOT = DM1_PC34_SLOT_CHEST_3,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C540_SLOT = DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C541_SLOT = DM1_PC34_SLOT_CHEST_5,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C542_SLOT = DM1_PC34_SLOT_CHEST_6,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C543_SLOT = DM1_PC34_SLOT_CHEST_7,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_SLOT = DM1_PC34_SLOT_CHEST_8,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C537_INDEX = 0,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C538_INDEX = 1,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_INDEX = 7,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_CONTAINER_BASE_WEIGHT = 50,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_LEADER_MAX_LOAD = 100,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_BASE_BACKPACK_WEIGHT = 13,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_BACKPACK_ITEM = 700,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE = 0x0040,
    DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_WEIGHT = 9
};

typedef struct {
    int m11ItemItemType;
    int m11ItemWeight;
    int m11ItemCharges;
    int m11ItemCursed;
    int m11ItemIdentified;
    int m11ItemAllowedSlots;
} DM1_V1_ChestCloseFullLeaderHandItemPc34;

typedef struct {
    const char* contractMarker;
    int c537Pc34Slot;
    int c538Pc34Slot;
    int c539Pc34Slot;
    int c540Pc34Slot;
    int c541Pc34Slot;
    int c542Pc34Slot;
    int c543Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int containerBaseWeight;
    int leaderMaxLoad;
    int baseBackpackWeight;
    int chestACellX;
    int chestACellY;
    int chestBCellX;
    int chestBCellY;
    DM1_V1_ChestCloseFullLeaderHandItemPc34 c544Helmet;
    DM1_V1_ChestCloseFullLeaderHandItemPc34 chestBVisibleC538;
} DM1_V1_ChestCloseFullLeaderHandSpecPc34;

typedef struct {
    int setupBaseLoadResult;
    int baseBackpackLoad;
    int sourceLockedContractOnly;

    int chestAThing;
    int chestBThing;
    int chestACellX;
    int chestACellY;
    int chestBCellX;
    int chestBCellY;

    int chestAOpenResult;
    int chestAOpenThing;
    int chestAVisibleWeightAfterOpen;
    int chestAContainerWeightAfterOpen;
    int chestAOpenTypes[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int chestAOpenAllowedSlots[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];

    int c544HelmetCanLeaveChest;
    int leaderHandBeforeC544Click;
    int c544BeforePickupType;
    int c544BeforePickupAllowedSlots;
    int c544ClickResult;
    int leaderHandAfterC544Click;
    int leaderHandAfterC544ClickAllowedSlots;
    int leaderHandFullAfterC544Click;
    int c544AfterPickupType;
    int chestAVisibleWeightAfterPickup;
    int loadAfterC544Pickup;

    int chestACloseCount;
    int chestAContainerWeightSnapshotAtClose;
    int chestAClosedTypes[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int chestAClosedWeights[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int chestAHiddenTailInputType;
    int chestAHiddenTailExcludedOnClose;
    int leaderHandAfterChestAClose;
    int leaderHandWeightAfterChestAClose;
    int leaderHandAllowedSlotsAfterChestAClose;
    int readyHandAfterChestAClose;
    int actionHandAfterChestAClose;
    int backpackAfterChestAClose;
    int loadAfterChestAClose;
    int chestAReadSlotAfterCloseResult;
    int leaderHandFullDuringChestAClose;
    int chestAOpenThingAfterClose;

    int chestBOpenResult;
    int chestBOpenThing;
    int leaderHandBeforeChestBOpen;
    int leaderHandAfterChestBOpen;
    int chestBContainsLeaderHelmet;
    int chestBVisibleWeightAfterOpen;
    int chestBContainerWeightAfterOpen;
    int chestBOpenTypes[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int chestBC538Type;
    int chestBC538IsOwnItem;

    int chestBCloseCount;
    int chestBContainerWeightSnapshotAtClose;
    int chestBClosedTypes[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int chestBClosedWeights[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int leaderHandAfterChestBClose;
    int leaderHandWeightAfterChestBClose;
    int leaderHandAllowedSlotsAfterChestBClose;
    int readyHandAfterChestBClose;
    int actionHandAfterChestBClose;
    int backpackAfterChestBClose;
    int loadAfterChestBClose;
    int chestBReadSlotAfterCloseResult;
    int leaderHandFullDuringChestBClose;
    int chestBOpenThingAfterClose;

    int chestAAfterChestBCloseTypes[DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT];
    int chestAChangedByChestBClose;
    int chestAStateIntactAfterChestBClose;
    int containerAWeightAfterFirstClose;
    int containerAWeightAfterChestBClose;
    int containerAContainerBaseWeightPreserved;
    int containerBWeightFromClosedLinks;
    int containerBContainerBaseWeightComputed;
} DM1_V1_ChestCloseFullLeaderHandProbePc34;

extern const DM1_V1_ChestCloseFullLeaderHandSpecPc34
    dm1_v1_chest_close_with_full_leader_hand_pc34_spec;

const char* dm1_v1_chest_close_with_full_leader_hand_source_evidence_pc34(void);
const DM1_V1_ChestCloseFullLeaderHandSpecPc34*
dm1_v1_chest_close_with_full_leader_hand_spec_pc34(void);
int dm1_v1_chest_close_with_full_leader_hand_pc34(
    DM1_V1_ChestCloseFullLeaderHandProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_CLOSE_WITH_FULL_LEADER_HAND_PC34_COMPAT_H */
