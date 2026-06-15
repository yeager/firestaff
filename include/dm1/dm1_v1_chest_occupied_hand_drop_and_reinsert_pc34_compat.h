#ifndef FIRESTAFF_DM1_V1_CHEST_OCCUPIED_HAND_DROP_AND_REINSERT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OCCUPIED_HAND_DROP_AND_REINSERT_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX = 3,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX = 5,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_COMPACTED_HAND_INDEX = 4,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_THING = 0x7D40,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_REOPEN_THING = 0x7D41,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT0_ITEM = 0x7300,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT1_ITEM = 0x7301,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT2_ITEM = 0x7302,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM = 0x7333,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT4_ITEM = 0x7304,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM = 0x7555,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_ONLY_CONTAINER = 0x7600,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_TOO_HEAVY_ITEM = 0x7700,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_MAX_ACCEPTED_WEIGHT = 100,
    DM1_PC34_CHEST_OCCUPIED_HAND_DROP_TOO_HEAVY_WEIGHT = 150,
    DM1_PC34_ICON_CONTAINER_CHEST_OPEN_PC34 = 145
};

typedef struct {
    int itemType;
    int weight;
    int allowedSlots;
} DM1_V1_ChestOccupiedHandDropItemPc34;

typedef struct {
    int contract_only;
    int slot3Index;
    int slot5Index;
    int compactedFormerHandIndex;
    int closeEyeIcon;
    int maxAcceptedWeight;
    DM1_V1_ChestOccupiedHandDropItemPc34 leaderHandItem;
    DM1_V1_ChestOccupiedHandDropItemPc34 slot3Item;
    DM1_V1_ChestOccupiedHandDropItemPc34 chestOnlyContainer;
    DM1_V1_ChestOccupiedHandDropItemPc34 tooHeavyItem;
    const char* f0333Anchor;
    const char* f0334Anchor;
    const char* f0300Anchor;
    const char* f0301Anchor;
    const char* f0302Anchor;
    const char* dataSlotMaskAnchor;
    const char* objectInfoAnchor;
    const char* f0140Anchor;
    const char* unavailableDestroyChestAnchor;
    const char* sourceSummary;
} DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34;

typedef struct {
    int openResult;
    int openThing;
    int openedTypes[DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT];
    int slot3BeforeDrop;
    int slot5BeforeDrop;
    int leaderHandBeforeDrop;
    int leaderHandCanEnterSlot5;

    int emptySlot5ClickResult;
    int slot5AfterHandDrop;
    int leaderHandAfterHandDrop;
    int slot3AfterHandDrop;

    int occupiedSlot3ClickResult;
    int slot3AfterPickup;
    int slot5AfterPickup;
    int leaderHandAfterSlot3Pickup;

    int closeCount;
    int closedTypes[DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT];
    int closedContainsFormerHand;
    int closedContainsOriginalSlot3;
    int closedSkipsEmptySlot3;
    int closeClearsOpenChest;

    int reopenResult;
    int reopenThing;
    int reopenedTypes[DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT_COUNT];
    int reopenedFormerHandIndex;
    int reopenedContainsFormerHand;
    int reopenedContainsOriginalSlot3;
    int originalSlot5EmptyAfterCompaction;
    int leaderHandAfterReopen;

    int postReopenSwapClickResult;
    int leaderHandAfterPostReopenSwap;
    int compactedSlotAfterPostReopenSwap;
    int originalSlot3UnaffectedByPostReopenSwap;

    int secondCycleClickResult;
    int leaderHandAfterSecondCycle;
    int compactedSlotAfterSecondCycle;
    int originalSlot3UnaffectedBySecondCycle;
} DM1_V1_ChestOccupiedHandDropRuntimePc34;

typedef struct {
    int incompatibleCanEnterChestSlot;
    int incompatibleClickResult;
    int incompatibleLeaderHandBefore;
    int incompatibleLeaderHandAfter;
    int incompatibleSlot5Before;
    int incompatibleSlot5After;
} DM1_V1_ChestOccupiedHandDropAllowedSlotsRejectPc34;

typedef struct {
    int heavyCanEnterChestSlot;
    int heavyRejectedByWeightGate;
    int heavyClickResult;
    int heavyLeaderHandBefore;
    int heavyLeaderHandAfter;
    int heavySlot5Before;
    int heavySlot5After;
    int maxAcceptedWeight;
    int heavyWeight;
} DM1_V1_ChestOccupiedHandDropWeightRejectPc34;

typedef struct {
    int contract_only;
    DM1_V1_ChestOccupiedHandDropRuntimePc34 runtime;
    DM1_V1_ChestOccupiedHandDropAllowedSlotsRejectPc34 allowedReject;
    DM1_V1_ChestOccupiedHandDropWeightRejectPc34 weightReject;
} DM1_V1_ChestOccupiedHandDropAndReinsertProbePc34;

extern const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34
    dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34_spec;

const char*
dm1_v1_chest_occupied_hand_drop_and_reinsert_source_evidence_pc34(void);
const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34*
dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34(void);
int dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34(
    DM1_V1_ChestOccupiedHandDropAndReinsertProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OCCUPIED_HAND_DROP_AND_REINSERT_PC34_COMPAT_H */
