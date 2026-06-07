#ifndef FIRESTAFF_DM1_V1_CHEST_ROUND_TRIP_HAND_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_ROUND_TRIP_HAND_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_ROUND_TRIP_CHEST_THING = 0x6C70,
    DM1_PC34_CHEST_ROUND_TRIP_REOPEN_THING = 0x6C71,
    DM1_PC34_CHEST_ROUND_TRIP_DAGGER = 0xC537,
    DM1_PC34_CHEST_ROUND_TRIP_TORCH = 0xC538,
    DM1_PC34_CHEST_ROUND_TRIP_BASE_ITEM = 0xC175,
    DM1_PC34_CHEST_ROUND_TRIP_DAGGER_WEIGHT = 4,
    DM1_PC34_CHEST_ROUND_TRIP_TORCH_WEIGHT = 4,
    DM1_PC34_CHEST_ROUND_TRIP_BASE_WEIGHT = 19,
    DM1_PC34_CHEST_ROUND_TRIP_LINKED_INPUT_COUNT = 2
};

typedef struct {
    int itemType;
    int weight;
    int allowedSlots;
} DM1_V1_ChestRoundTripHandSwapItemPc34;

typedef struct {
    int contractOnly;
    int c537Slot;
    int c538Slot;
    int c544Slot;
    int visibleSlotCount;
    int linkedInputCount;
    int hiddenTailClaimed;
    DM1_V1_ChestRoundTripHandSwapItemPc34 dagger;
    DM1_V1_ChestRoundTripHandSwapItemPc34 torch;
    DM1_V1_ChestRoundTripHandSwapItemPc34 baseItem;
    const char* f0333Anchor;
    const char* f0334Anchor;
    const char* f0297Anchor;
    const char* f0300Anchor;
    const char* f0301Anchor;
    const char* sourceSummary;
} DM1_V1_ChestRoundTripHandSwapSpecPc34;

typedef struct {
    int openResult;
    int openThing;
    int openedTypes[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int openedWeights[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int openedVisibleCount;
    int openedHasDaggerAtC537;
    int openedHasTorchAtC538;
    int openedOnlyVisibleSlots;
    int openVisibleWeight;
    int openContainerWeight;
    int openContainerBaseContribution;
    int loadBeforeOpen;
    int loadAfterOpen;
    int loadDeltaAfterOpen;
} DM1_V1_ChestRoundTripOpenPc34;

typedef struct {
    int firstClickResult;
    int leaderHandAfterFirstType;
    int leaderHandAfterFirstWeight;
    int leaderHandAfterFirstCanEnterChest;
    int afterFirstTypes[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int afterFirstWeights[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int afterFirstVisibleCount;
    int afterFirstC537StillDagger;
    int afterFirstC538Empty;
    int afterFirstVisibleWeight;
    int afterFirstContainerWeight;
    int afterFirstContainerBaseContribution;
    int loadAfterFirstSwap;
    int loadDeltaAfterFirstSwap;
    int effectiveLoadAfterFirstSwap;
    int effectiveLoadDeltaAfterFirstSwap;
} DM1_V1_ChestRoundTripFirstSwapPc34;

typedef struct {
    int secondClickResult;
    int leaderHandAfterSecondType;
    int leaderHandAfterSecondWeight;
    int leaderHandAfterSecondCanEnterChest;
    int afterSecondTypes[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int afterSecondWeights[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int afterSecondVisibleCount;
    int afterSecondC537Torch;
    int afterSecondC538Empty;
    int afterSecondDaggerOnlyInLeaderHand;
    int afterSecondTorchOnlyInChest;
    int afterSecondVisibleWeight;
    int afterSecondContainerWeight;
    int afterSecondContainerBaseContribution;
    int loadAfterSecondSwap;
    int loadDeltaAfterSecondSwap;
    int effectiveLoadAfterSecondSwap;
    int effectiveLoadDeltaAfterSecondSwap;
} DM1_V1_ChestRoundTripSecondSwapPc34;

typedef struct {
    int closeCount;
    int closeClearsOpenChest;
    int closedTypes[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int closedWeights[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int closedVisibleCount;
    int closedC537Torch;
    int closedC538Empty;
    int closedDaggerExcludedBecauseLeaderHand;
    int closeContainerWeightSnapshot;
    int closeContainerBaseContribution;
    int closeContainerWeightAfter;
    int loadAfterClose;
    int loadDeltaAfterClose;
    int leaderHandAfterCloseType;
    int leaderHandAfterCloseWeight;
} DM1_V1_ChestRoundTripClosePc34;

typedef struct {
    int reopenResult;
    int reopenThing;
    int reopenedTypes[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int reopenedWeights[DM1_PC34_CHEST_ROUND_TRIP_SLOT_COUNT];
    int reopenedVisibleCount;
    int reopenedC537Torch;
    int reopenedC538Empty;
    int reopenedDaggerStillLeaderHand;
    int reopenedTorchPreserved;
    int reopenedOriginalObjectIdentitiesPreserved;
    int reopenedVisibleWeight;
    int reopenedContainerWeight;
    int reopenedContainerBaseContribution;
    int loadAfterReopen;
    int loadDeltaAfterReopen;
    int leaderHandAfterReopenType;
    int leaderHandAfterReopenWeight;
} DM1_V1_ChestRoundTripReopenPc34;

typedef struct {
    int contractOnly;
    int baseItemSetResult;
    int hiddenTailClaimed;
    int originalDaggerId;
    int originalTorchId;
    DM1_V1_ChestRoundTripOpenPc34 open;
    DM1_V1_ChestRoundTripFirstSwapPc34 firstSwap;
    DM1_V1_ChestRoundTripSecondSwapPc34 secondSwap;
    DM1_V1_ChestRoundTripClosePc34 close;
    DM1_V1_ChestRoundTripReopenPc34 reopen;
} DM1_V1_ChestRoundTripHandSwapProbePc34;

const char* dm1_v1_chest_round_trip_hand_swap_source_evidence_pc34(void);
const DM1_V1_ChestRoundTripHandSwapSpecPc34*
dm1_v1_chest_round_trip_hand_swap_spec_pc34(void);
const DM1_V1_ChestRoundTripHandSwapProbePc34*
dm1_v1_chest_round_trip_hand_swap_last_probe_pc34(void);
int dm1_v1_chest_round_trip_hand_swap_run(int* passed, int* failed);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_ROUND_TRIP_HAND_SWAP_PC34_COMPAT_H */
