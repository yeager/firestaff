#ifndef FIRESTAFF_DM1_V1_CHEST_PARTIAL_DROP_TO_FLOOR_WHILE_CHEST_OPEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PARTIAL_DROP_TO_FLOOR_WHILE_CHEST_OPEN_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_PARTIAL_DROP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_PARTIAL_DROP_C30 = DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_PARTIAL_DROP_C537 = 537,
    DM1_PC34_PARTIAL_DROP_C538 = 538,
    DM1_PC34_PARTIAL_DROP_TARGET_INDEX = 1,
    DM1_PC34_PARTIAL_DROP_CHEST_THING = 0x7640,
    DM1_PC34_PARTIAL_DROP_STACK_TYPE = 0x7641,
    DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT = 3,
    DM1_PC34_PARTIAL_DROP_INITIAL_COUNT = 9,
    DM1_PC34_PARTIAL_DROP_UNIT_WEIGHT = 2,
    DM1_PC34_PARTIAL_DROP_FLOOR_X = 14,
    DM1_PC34_PARTIAL_DROP_FLOOR_Y = 22,
    DM1_PC34_PARTIAL_DROP_PANEL_CHEST = DM1_PC34_PANEL_CHEST
};

typedef struct {
    int type;
    int count;
    int weight;
} DM1_V1_ChestPartialDropStackPc34;

typedef struct {
    int sourceLockedContractOnly;
    int openResult;
    int openChestThing;
    int panelAfterOpen;
    int commandPanelChest;
    int commandDispatchC040;
    int sourcePc34Slot;
    int sourceZone;
    int sourceSlotIndex;
    int sourceAllowedByC30Mask;
    int f0300RemovedFromG0425;
    int f0297PutPartialInLeaderHand;
    int f0298RemovedLeaderHandForFloor;
    int f0301PreservedRemainingInG0425;
    int f0133PartialMaskDispatches;
    int f0163FloorLinkCount;
    int floorMapX;
    int floorMapY;
    int floorDropCount;
    int floorType;
    int floorWeight;

    DM1_V1_ChestPartialDropStackPc34 chestBefore[
        DM1_PC34_PARTIAL_DROP_SLOT_COUNT];
    DM1_V1_ChestPartialDropStackPc34 chestAfter[
        DM1_PC34_PARTIAL_DROP_SLOT_COUNT];
    DM1_V1_ChestPartialDropStackPc34 closedChain[
        DM1_PC34_PARTIAL_DROP_SLOT_COUNT];
    DM1_V1_ChestPartialDropStackPc34 leaderHandBefore;
    DM1_V1_ChestPartialDropStackPc34 leaderHandAfterSplit;
    DM1_V1_ChestPartialDropStackPc34 leaderHandAfterFloor;

    int initialStackCount;
    int partialDropCount;
    int remainingStackCount;
    int initialStackWeight;
    int partialDropWeight;
    int remainingStackWeight;
    int openVisibleCountBefore;
    int openVisibleCountAfter;
    int closeCount;
    int closedTargetIndex;
    int chestVisibleChainUpdated;
    int leaderHandUpdated;
    int floorReceivedPartial;
    int closedChainPreservesRemaining;
    int deterministic;
    uint32_t deterministicHash;
} DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34;

const char*
dm1_v1_chest_partial_drop_to_floor_while_chest_open_source_evidence_pc34(
    void);
int dm1_v1_chest_partial_drop_to_floor_while_chest_open_run_pc34(
    DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
