#ifndef FIRESTAFF_DM1_V1_CHEST_MID_CLOSE_HAND_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_MID_CLOSE_HAND_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_MID_CLOSE_PREFIX_COUNT = 3,
    DM1_PC34_CHEST_MID_CLOSE_ALREADY_PROCESSED_INDEX = 1,
    DM1_PC34_CHEST_MID_CLOSE_FUTURE_INDEX = 4,
    DM1_PC34_CHEST_MID_CLOSE_CHEST_THING = 0x7D00,
    DM1_PC34_CHEST_MID_CLOSE_FIRST_ITEM = 0x7D10,
    DM1_PC34_CHEST_MID_CLOSE_REPLACEMENT_ITEM = 0x7D90,
    DM1_PC34_CHEST_MID_CLOSE_REOPEN_THING = 0x7D01
};

typedef struct {
    const char* contractMarker;
    const char* chosenBecause;
    int contractOnly;
    int chestSlotCount;
    int prefixCount;
    int alreadyProcessedIndex;
    int futureIndex;
    int futurePc34Slot;
    int alreadyProcessedPc34Slot;
    int chestThing;
    int firstItemType;
    int futureOriginalItemType;
    int replacementItemType;
} DM1_V1_ChestMidCloseHandSwapSpecPc34;

typedef struct {
    int contractOnly;

    int openResult;
    int openThing;
    int openVisibleCount;
    int openedTypes[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    int openedWeights[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    int openedOrderMatchesInput;
    int objectIconPathCited;
    int blitMaskIsPresentationOnly;

    int leaderHandBeforeCloseType;
    int leaderHandBeforeCloseWeight;
    int leaderHandBeforeCloseAllowedSlots;
    int replacementCanEnterChest;

    int closeBeginResult;
    int openThingBeforeCloseBegin;
    int openThingAfterCloseBegin;
    int containerSlotClearedToEnd;
    int prefixProcessedCount;
    int prefixClosedCount;
    int prefixClosedTypes[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    int prefixSlotsCleared[DM1_PC34_CHEST_MID_CLOSE_PREFIX_COUNT];
    int prefixLastLinkedType;
    int futureSlotBeforeSwapType;
    int futureSlotBeforeSwapWeight;

    int midSwapResult;
    int midSwapPc34Slot;
    int midSwapLeaderBeforeType;
    int midSwapSlotBeforeType;
    int midSwapLeaderAfterType;
    int midSwapSlotAfterType;
    int midSwapLeaderRemovedCall;
    int midSwapSlotRemovedCall;
    int midSwapPutDisplacedInLeaderCall;
    int midSwapPutReplacementInSlotCall;
    int futureOriginalAbsentFromSlotAfterSwap;

    int closeCount;
    int closedTypes[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    int closedWeights[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    int replacementLinkedAtFuturePosition;
    int displacedAbsentFromClosedLinks;
    int closedOrderUsesCurrentFutureSlot;
    int futureSlotClearedByClose;
    int allSlotsEmptyAfterClose;
    int leaderHandAfterCloseType;
    int leaderHandAfterCloseWeight;

    int reopenResult;
    int reopenThing;
    int reopenedVisibleCount;
    int reopenedTypes[DM1_PC34_CHEST_MID_CLOSE_SLOT_COUNT];
    int reopenedOrderMatchesClosed;
    int replacementVisibleAfterReopen;
    int displacedAbsentAfterReopen;

    int alreadyProcessedSwapResult;
    int alreadyProcessedPc34Slot;
    int alreadyProcessedLeaderBeforeType;
    int alreadyProcessedSlotBeforeLateWriteType;
    int alreadyProcessedSlotAfterLateWriteType;
    int alreadyProcessedCloseCount;
    int alreadyProcessedReplacementLinked;
    int alreadyProcessedLateWriteNotRevisited;
    int alreadyProcessedStaleSlotAfterCloseType;
    int alreadyProcessedLeaderAfterCloseType;
} DM1_V1_ChestMidCloseHandSwapProbePc34;

const char* dm1_v1_chest_mid_close_hand_swap_source_evidence_pc34(void);
const DM1_V1_ChestMidCloseHandSwapSpecPc34*
dm1_v1_chest_mid_close_hand_swap_spec_pc34(void);
int dm1_v1_chest_mid_close_hand_swap_run_pc34(
    DM1_V1_ChestMidCloseHandSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_MID_CLOSE_HAND_SWAP_PC34_COMPAT_H */
