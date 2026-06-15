#ifndef FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C537_ORDINAL = 537,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C538_ORDINAL = 538,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C539_ORDINAL = 539,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C544_ORDINAL = 544,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX = 1,
    DM1_PC34_CHEST_OCCUPIED_SWAP_OPEN_CHEST_THING = 0x7C38,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C537_STACK = 4101,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK = 4102,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C539_STACK = 4103,
    DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK = 4202,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C537_WEIGHT = 3,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C538_WEIGHT = 5,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C539_WEIGHT = 2,
    DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_WEIGHT = 4,
    DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT = 7,
    DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT = 2
};

typedef struct {
    const char* contractMarker;
    int g0305PartyChampionCount;
    int g0423InventoryChampionOrdinal;
    int c537Ordinal;
    int c538Ordinal;
    int c544Ordinal;
    int c538Pc34Slot;
    int c538G0425Index;
    int m070ReadyHandSlotIndex;
    int m070ActionHandSlotIndex;
    int visibleSlotCount;
    int oldStackType;
    int oldStackCount;
    int leaderStackType;
    int leaderStackCount;
} DM1_V1_ChestOccupiedSlotSwapSpecPc34;

typedef struct {
    M11_InventoryState runtime;
    int g0305PartyChampionCount;
    int g0423InventoryChampionOrdinal;
    int g0426OpenChestThing;
    int m070ReadyHandSlotIndex;
    int m070ActionHandSlotIndex;
    int m516LeaderLoad;
    int m516LeaderSourceEquivalentLoad;
    int g0425Types[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int g0425Weights[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int g0425Counts[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
} DM1_V1_ChestOccupiedSlotSwapRuntimePc34;

typedef struct {
    int initResult;
    int exerciseResult;
    int sourceLockedContractOnly;

    int openChestThingBefore;
    int openChestThingAfterClick;
    int g0305PartyChampionCount;
    int g0423InventoryChampionOrdinal;
    int m070ReadyHandSlotIndex;
    int m070ActionHandSlotIndex;
    int c538Pc34Slot;
    int c538Ordinal;

    int visibleCountBefore;
    int visibleCountAfterClick;
    int closedCount;
    int reopenedCount;
    int reopenResult;

    int beforeTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int beforeWeights[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int beforeCounts[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int afterTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int afterWeights[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int afterCounts[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int closedTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int reopenedTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];

    int leaderHandBeforeType;
    int leaderHandBeforeWeight;
    int leaderHandBeforeCount;
    int leaderHandAfterType;
    int leaderHandAfterWeight;
    int leaderHandAfterCount;

    int c538BeforeType;
    int c538BeforeWeight;
    int c538BeforeCount;
    int c538AfterType;
    int c538AfterWeight;
    int c538AfterCount;

    int replacementAllowedInC538;
    int f0302Accepted;
    int f0300RemovedOccupiedC538;
    int f0297PlacedOldC538InLeaderHand;
    int f0301StoredLeaderObjectInC538;
    int c537StableAfterClick;
    int c539StableAfterClick;
    int oldStackNoLongerInChestAfterClick;
    int replacementNoLongerInLeaderHandAfterClick;
    int sourceEquivalentLoadBefore;
    int sourceEquivalentLoadAfterClick;
    int sourceEquivalentLoadUnchanged;
    int closeClearedG0426;
    int closeRewroteVisibleOnly;
    int reopenPreservedC538Replacement;
} DM1_V1_ChestOccupiedSlotSwapProbePc34;

typedef struct {
    int totalAssertions;
    int passedAssertions;
    int failedAssertions;
} DM1_V1_ChestOccupiedSlotSwapAssertionsPc34;

extern const DM1_V1_ChestOccupiedSlotSwapSpecPc34
    dm1_v1_chest_occupied_slot_swap_pc34_spec;

const char* dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(void);
const DM1_V1_ChestOccupiedSlotSwapSpecPc34*
dm1_v1_chest_occupied_slot_swap_spec_pc34(void);
int dm1_v1_chest_occupied_slot_swap_init_pc34(
    DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state);
int dm1_v1_chest_occupied_slot_swap_exercise_pc34(
    DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state,
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out);
const DM1_V1_ChestOccupiedSlotSwapAssertionsPc34*
dm1_v1_chest_occupied_slot_swap_assertions_pc34(void);
int dm1_v1_chest_occupied_slot_swap_pc34(
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PC34_COMPAT_H */
