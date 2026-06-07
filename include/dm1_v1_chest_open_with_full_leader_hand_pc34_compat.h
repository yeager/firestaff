#ifndef FIRESTAFF_DM1_V1_CHEST_OPEN_WITH_FULL_LEADER_HAND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OPEN_WITH_FULL_LEADER_HAND_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_OPEN_FULL_HAND_C537_SLOT = DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C538_SLOT = DM1_PC34_SLOT_CHEST_2,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C539_SLOT = DM1_PC34_SLOT_CHEST_3,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C540_SLOT = DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C541_SLOT = DM1_PC34_SLOT_CHEST_5,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C542_SLOT = DM1_PC34_SLOT_CHEST_6,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C543_SLOT = DM1_PC34_SLOT_CHEST_7,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C544_SLOT = DM1_PC34_SLOT_CHEST_8,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C537_INDEX = 0,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C538_INDEX = 1,
    DM1_PC34_CHEST_OPEN_FULL_HAND_C540_INDEX = 3,
    DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_OPEN_FULL_HAND_CONTAINER_BASE_WEIGHT = 50,
    DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE = 0x0040,
    DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_WEIGHT = 9
};

typedef struct {
    int m11ItemItemType;
    int m11ItemWeight;
    int m11ItemCharges;
    int m11ItemCursed;
    int m11ItemIdentified;
    int m11ItemAllowedSlots;
} DM1_V1_ChestOpenFullLeaderHandItemPc34;

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
    int partyChampionCount;
    int chestACellX;
    int chestACellY;
    int chestBCellX;
    int chestBCellY;
    DM1_V1_ChestOpenFullLeaderHandItemPc34 leaderHelmet;
    DM1_V1_ChestOpenFullLeaderHandItemPc34 chestAFirstItem;
    DM1_V1_ChestOpenFullLeaderHandItemPc34 chestBFirstItem;
} DM1_V1_ChestOpenFullLeaderHandSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int partyChampionCount;

    int chestAThing;
    int chestBThing;
    int chestACellX;
    int chestACellY;
    int chestBCellX;
    int chestBCellY;

    int leaderHandSetupResult;
    int leaderHandBeforeOpenType;
    int leaderHandBeforeOpenWeight;
    int leaderHandBeforeOpenAllowedSlots;

    int chestAClosedHeadBefore;
    int chestAClosedTailBefore;
    int chestAClosedWeightBefore;
    int chestAClosedHeadAfterChestBOpen;
    int chestAClosedTailAfterChestBOpen;
    int chestAClosedWeightAfterChestBOpen;
    int chestAClosedStateIntactAfterChestBOpen;

    int chestBClosedHeadBefore;
    int chestBClosedTailBefore;
    int chestBClosedWeightBefore;
    int chestBClosedHeadAfterOpen;
    int chestBClosedTailAfterOpen;
    int chestBClosedWeightAfterOpen;
    int chestBClosedStateIntactAfterOpen;

    int chestBOpenResult;
    int chestBOpenThing;
    int chestBOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT];
    int chestBOpenWeights[DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT];
    int chestBVisibleWeightAfterOpen;
    int chestBContainerWeightAfterOpen;
    int chestBC537TypeAfterOpen;
    int chestBC540TypeAfterOpen;
    int chestBContainsLeaderHelmetAfterOpen;
    int chestBFirstItemInLeaderHandAfterOpen;

    int leaderHandAfterChestBOpenType;
    int leaderHandAfterChestBOpenWeight;
    int leaderHandAfterChestBOpenAllowedSlots;
    int leaderHandEvictedByChestBOpen;
    int leaderHandDuplicatedIntoC540ByChestBOpen;

    int case2LeaderHandSetupResult;
    int case2LeaderBeforeChestAOpen;
    int case2ChestAOpenResult;
    int case2ChestAOpenThing;
    int case2LeaderAfterChestAOpen;
    int case2ChestAOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT];
    int case2ChestAContainsLeaderHelmetAfterOpen;
    int case2ChestAC537TypeAfterOpen;
    int case2ChestAC538TypeAfterOpen;
    int case2ChestAVisibleWeightAfterOpen;
    int case2ChestAContainerWeightAfterOpen;

    int case2OpenChestBReplacingReturn;
    int case2ChestBOpenThing;
    int case2LeaderAfterChestBOpen;
    int case2PreviousChestAClosedCount;
    int case2PreviousChestAClosedHead;
    int case2PreviousChestAClosedTail;
    int case2PreviousChestAClosedWeight;
    int case2ChestBOpenTypes[DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT];
    int case2ChestBVisibleWeightAfterOpen;
    int case2ChestBContainerWeightAfterOpen;
    int case2ChestBC537TypeAfterOpen;
    int case2ChestBC538TypeAfterOpen;
    int case2ChestBC540TypeAfterOpen;
    int case2ChestBContainsChestAItem;
    int case2ChestBContainsLeaderHelmet;
    int case2ChestBFirstItemInLeaderHand;
    int case2LeaderDuplicatedIntoC540;
} DM1_V1_ChestOpenFullLeaderHandProbePc34;

extern const DM1_V1_ChestOpenFullLeaderHandSpecPc34
    dm1_v1_chest_open_with_full_leader_hand_pc34_spec;

const char* M11_GameView_ChestOpenWithFullLeaderHandSourceEvidencePc34(void);
const DM1_V1_ChestOpenFullLeaderHandSpecPc34*
M11_GameView_ChestOpenWithFullLeaderHandSpecPc34(void);
int M11_GameView_ChestOpenWithFullLeaderHandRuntimeGatePc34(
    DM1_V1_ChestOpenFullLeaderHandProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OPEN_WITH_FULL_LEADER_HAND_PC34_COMPAT_H */
