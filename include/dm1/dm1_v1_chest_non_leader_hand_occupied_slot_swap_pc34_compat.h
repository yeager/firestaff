#ifndef FIRESTAFF_DM1_V1_CHEST_NON_LEADER_HAND_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_NON_LEADER_HAND_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT = 8,
    DM1_PC34_CHEST_NON_LEADER_SWAP_NONE = 0xFFFF,
    DM1_PC34_CHEST_NON_LEADER_SWAP_END_OF_LIST = 0xFFFE,
    DM1_PC34_CHEST_NON_LEADER_SWAP_C30_BASE = 30,
    DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ORDINAL = 1,
    DM1_PC34_CHEST_NON_LEADER_SWAP_INVENTORY_ORDINAL = 1,
    DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ORDINAL = 2,
    DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_SLOT_BOX = 2,
    DM1_PC34_CHEST_NON_LEADER_SWAP_TARGET_INDEX = 4,
    DM1_PC34_CHEST_NON_LEADER_SWAP_COMPACTED_INDEX = 2,
    DM1_PC34_CHEST_NON_LEADER_SWAP_OPEN_CHEST = 0x7100,
    DM1_PC34_CHEST_NON_LEADER_SWAP_REOPEN_CHEST = 0x7101,
    DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_A = 0x7200,
    DM1_PC34_CHEST_NON_LEADER_SWAP_SEED_B = 0x7202,
    DM1_PC34_CHEST_NON_LEADER_SWAP_NON_LEADER_ITEM = 0x7302,
    DM1_PC34_CHEST_NON_LEADER_SWAP_LEADER_ITEM = 0x7401
};

typedef struct {
    int thing;
    int weight;
    int allowedSlots;
} DM1_V1_ChestNonLeaderHandSwapItemPc34;

typedef struct {
    int contractOnly;
    int thingNone;
    int endOfList;
    int c30BaseSlot;
    int leaderOrdinal;
    int inventoryChampionOrdinal;
    int nonLeaderOrdinal;
    int nonLeaderSourceSlotBox;
    int targetChestIndex;
    int compactedChestIndex;
    DM1_V1_ChestNonLeaderHandSwapItemPc34 nonLeaderHandItem;
    DM1_V1_ChestNonLeaderHandSwapItemPc34 leaderHandItem;
    const char* chestCloseAnchor;
    const char* leaderPutAnchor;
    const char* leaderRemoveAnchor;
    const char* slotClearAnchor;
    const char* slotWriteAnchor;
    const char* occupiedSwapAnchor;
    const char* objectIconAnchor;
    const char* blitMaskAnchor;
    const char* defsAnchor;
} DM1_V1_ChestNonLeaderHandSwapSpecPc34;

typedef struct {
    int sourceSlotBox;
    int championIndexResolved;
    int sourceHandSlotResolved;
    int initialLeaderHand;
    int initialNonLeaderHand;
    int initialChestTypes[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT];

    int pickupClickResult;
    int leaderHandAfterNonLeaderPickup;
    int nonLeaderHandAfterPickup;
    int c0ffffClearAppliedToSourceHand;

    int emptyTargetClickResult;
    int targetChestSlotAfterNonLeaderDrop;
    int leaderHandAfterNonLeaderDrop;
    int nonLeaderHandAfterNonLeaderDrop;
    int targetStillOccupiedBeforeClose;

    int leaderHandBeforeClose;
    int closeCount;
    int closeContainerHead;
    int closeEndSentinel;
    int closedTypes[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT];
    int closedContainsNonLeaderItem;
    int closedNonLeaderItemIndex;
    int closePreservedLeaderHandPointer;
    int targetClearedOnlyByCloseRewrite;

    int reopenResult;
    int reopenedTypes[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT];
    int reopenedNonLeaderItemIndex;
    int leaderHandBeforeOccupiedSwap;
    int nonLeaderHandBeforeOccupiedSwap;

    int occupiedSwapClickResult;
    int c30ClearSlotIndex;
    int c30ValueAfterClear;
    int c30ValueAfterWrite;
    int leaderHandAfterOccupiedSwap;
    int nonLeaderHandAfterOccupiedSwap;
    int swapUsedLeaderPointer;
    int swapDidNotRestoreNonLeaderHand;
    int c0ffffClearDidNotSurviveTarget;
    int adjacentSlot0AfterSwap;
    int adjacentSlot1AfterSwap;
    int adjacentSlot3AfterSwap;

    int finalCloseCount;
    int finalClosedTypes[DM1_PC34_CHEST_NON_LEADER_SWAP_SLOT_COUNT];
    int finalClosedContainsLeaderItem;
    int finalClosedContainsNonLeaderItem;
} DM1_V1_ChestNonLeaderHandSwapProbePc34;

extern const DM1_V1_ChestNonLeaderHandSwapSpecPc34
    dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34_spec;

const char*
dm1_v1_chest_non_leader_hand_occupied_slot_swap_source_evidence_pc34(void);
const DM1_V1_ChestNonLeaderHandSwapSpecPc34*
dm1_v1_chest_non_leader_hand_occupied_slot_swap_spec_pc34(void);
int dm1_v1_chest_non_leader_hand_occupied_slot_swap_pc34(
    DM1_V1_ChestNonLeaderHandSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_NON_LEADER_HAND_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H */
