#ifndef FIRESTAFF_DM1_V1_CHEST_REOPEN_THEN_SWAP_LEADER_HAND_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_REOPEN_THEN_SWAP_LEADER_HAND_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_REOPEN_SWAP_MAX_LINKED = 9,
    DM1_PC34_CHEST_REOPEN_SWAP_A_FIRST = 2300,
    DM1_PC34_CHEST_REOPEN_SWAP_A_HIDDEN_TAIL = 2308,
    DM1_PC34_CHEST_REOPEN_SWAP_A_THING = 0x7B30,
    DM1_PC34_CHEST_REOPEN_SWAP_A_REOPEN_THING = 0x7B31,
    DM1_PC34_CHEST_REOPEN_SWAP_B_THING = 0x7B32,
    DM1_PC34_CHEST_REOPEN_SWAP_B_DEST_INDEX = 0
};

typedef struct {
    const char* contractMarker;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int maxLinkedInputCount;
    int chestBDestinationIndex;
    int chestAVisibleFirstItemType;
    int chestAHiddenTailItemType;
} DM1_V1_ChestReopenThenSwapLeaderHandSpecPc34;

typedef struct {
    int contract_only;
    int chestAThing;
    int chestBThing;

    int worldHashBeforeResult;
    unsigned int worldHashBefore;
    int worldHashAfterCloseResult;
    unsigned int worldHashAfterClose;
    int worldHashAfterFinalResult;
    unsigned int worldHashAfterFinal;
    int chestAWorldHashStableAfterClose;
    int chestAWorldHashStableFinal;

    int chestAOpenResult;
    int chestAOpenThing;
    int chestAOpenedVisibleCount;
    int chestAOpenedTypes[DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT];
    int chestAOpenedOrderMatchesInput;

    int leaderHandSetupResult;
    int leaderHandBeforeClose;
    int leaderHandCanEnterChestB;

    int chestACloseCount;
    int chestAClosedTypes[DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT];
    int chestAHiddenTailReturnedByClose;
    int leaderHandAfterClose;
    int leaderHandStableAcrossClose;

    int chestAReopenResult;
    int chestAReopenThing;
    int chestAReopenedVisibleCount;
    int chestAReopenedTypes[DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT];
    int chestAReopenedOrderMatchesClose;
    int chestAHiddenTailVisibleAfterReopen;
    int leaderHandAfterReopen;
    int leaderHandStableAcrossReopen;

    int chestACloseWhileOpeningBCount;
    int chestAClosedWhileOpeningBTypes[DM1_PC34_CHEST_REOPEN_SWAP_SLOT_COUNT];
    int chestAHiddenTailReturnedWhileOpeningB;
    int chestBOpenThing;

    int chestBPlaceClickResult;
    int chestBDestinationAfterPlace;
    int leaderHandAfterPlace;
    int leaderHandEmptyAfterPlace;
    int hiddenTailStoredInChestB;
    int chestAHiddenTailStillEmpty;
} DM1_V1_ChestReopenThenSwapLeaderHandProbePc34;

extern const DM1_V1_ChestReopenThenSwapLeaderHandSpecPc34
    dm1_v1_chest_reopen_then_swap_leader_hand_pc34_spec;

const char*
M11_GameView_ChestReopenThenSwapLeaderHandSourceEvidencePc34(void);
const DM1_V1_ChestReopenThenSwapLeaderHandSpecPc34*
M11_GameView_ChestReopenThenSwapLeaderHandSpecPc34(void);
int M11_GameView_ChestReopenThenSwapLeaderHandRunPc34(
    DM1_V1_ChestReopenThenSwapLeaderHandProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_REOPEN_THEN_SWAP_LEADER_HAND_PC34_COMPAT_H */
