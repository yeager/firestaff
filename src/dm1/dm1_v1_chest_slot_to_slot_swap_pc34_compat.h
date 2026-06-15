#ifndef FIRESTAFF_DM1_V1_CHEST_SLOT_TO_SLOT_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SLOT_TO_SLOT_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SOURCE_INDEX = 0,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_DEST_INDEX = 4,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_EMPTY_INDEX_A = 6,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_EMPTY_INDEX_B = 7,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SOURCE_PC34_SLOT = DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_DEST_PC34_SLOT = DM1_PC34_SLOT_CHEST_5,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CHEST_THING = 0x7B30,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_A = 4100,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ITEM_B = 4200,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_1 = 4301,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_2 = 4302,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_FILLER_3 = 4303,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_LEADER_SENTINEL = 4400,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_INVALID_HAND_ITEM = 4500,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_A =
        DM1_PC34_ALLOWED_CONTAINER | DM1_PC34_ALLOWED_POUCH,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_ALLOWED_B =
        DM1_PC34_ALLOWED_CONTAINER | DM1_PC34_ALLOWED_QUIVER_LINE1,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_INVALID_ALLOWED =
        DM1_PC34_ALLOWED_HEAD,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_C10_TRANSPARENT_COLOR = 10,
    DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_CONTRACT_ASSERTION_BUDGET = 80
};

typedef struct {
    const char* contractMarker;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int sourcePc34Slot;
    int destinationPc34Slot;
    int sourceIndex;
    int destinationIndex;
    int emptyIndexA;
    int emptyIndexB;
    int chestSlotMask;
    int pouchMask;
    int quiverLine1Mask;
    int backpackMask;
    int transparentColorC10;
    int assertionBudget;
} DM1_V1_ChestSlotToSlotSwapSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int noAssetParityClaim;
    int chestThing;
    int openResult;
    int openThing;
    int sameOpenNoopResult;
    int sameOpenThing;

    int sourcePc34Slot;
    int destinationPc34Slot;
    int sourceIndex;
    int destinationIndex;
    int emptyIndexA;
    int emptyIndexB;
    int chestSlotMaskSource;
    int chestSlotMaskDestination;
    int pouchMask;
    int quiverLine1Mask;
    int backpackMask;
    int itemAMaskBefore;
    int itemBMaskBefore;
    int itemAContainerOverlap;
    int itemBContainerOverlap;

    int leaderHandBefore;
    int leaderHandAfterSwap;
    int leaderHandAfterEmptyNoop;
    int leaderHandAfterInvalidClick;
    int leaderHandAfterClose;
    int leaderHandAfterReopen;
    int leaderHandUntouchedBySwap;
    int noLeaderHandLeak;

    int loadBeforeSwap;
    int loadAfterSwap;
    int loadAfterEmptyNoop;
    int loadAfterInvalidClick;
    int loadAfterClose;
    int loadAfterReopen;
    int loadUnchangedBySwap;
    int loadUnchangedByNoop;
    int loadUnchangedByInvalidClick;
    int reopenedLoadMatchesOpenLoad;

    int selectedSourceResult;
    int selectedSourceIndex;
    int selectedThing;
    int selectedDestinationResult;
    int emptySourceClickResult;
    int emptyDestinationClickResult;
    int invalidClickResult;
    int invalidCanEquip;
    int invalidSlotUnchanged;
    int emptySlotsNoop;
    int swapCompleted;
    int sourceEmptyDuringDrag;
    int sourceReceivesB;
    int destinationReceivesA;
    int itemAAllowedMaskPreserved;
    int itemBAllowedMaskPreserved;
    int viewPreservedAfterSwap;
    int iconRefreshAcknowledged;
    int maskedBlitTransparentC10;
    int objectTypeIconLookupAcknowledged;
    int ammoCompatibilityUnaffected;

    int beforeTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int beforeAllowed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int duringDragTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int afterSwapTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int afterSwapAllowed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int afterEmptyNoopTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int afterInvalidTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int closedTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int closedAllowed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int reopenedTypes[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int reopenedAllowed[DM1_PC34_CHEST_SLOT_TO_SLOT_SWAP_SLOT_COUNT];
    int closeCount;
    int reopenResult;
    int reopenedOpenThing;
    int openChestClearedAfterClose;
    int closeRewritePreservedVisibleOrder;
    int reopenPreservedSlotToSlotSwap;
    int noDuplicateObjectIds;
    int nonEmptyCountBefore;
    int nonEmptyCountAfterSwap;
    int nonEmptyCountAfterClose;
    int nonEmptyCountAfterReopen;
} DM1_V1_ChestSlotToSlotSwapProbePc34;

extern const DM1_V1_ChestSlotToSlotSwapSpecPc34
    dm1_v1_chest_slot_to_slot_swap_pc34_spec;

const char* dm1_v1_chest_slot_to_slot_swap_source_evidence_pc34(void);
const DM1_V1_ChestSlotToSlotSwapSpecPc34*
dm1_v1_chest_slot_to_slot_swap_spec_pc34(void);
int dm1_v1_chest_slot_to_slot_swap_run_pc34(
    DM1_V1_ChestSlotToSlotSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SLOT_TO_SLOT_SWAP_PC34_COMPAT_H */
