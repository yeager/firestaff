#ifndef FIRESTAFF_DM1_V1_CHEST_DROP_TO_FLOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_DROP_TO_FLOOR_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT = 6,
    DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT = 5,
    DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_CHEST_ITEM = 700,
    DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_HAND_ITEM = 900,
    DM1_PC34_CHEST_DROP_TO_FLOOR_CHEST_THING = 0x7D30,
    DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_X = 12,
    DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_Y = 18
};

typedef struct {
    const char* contractMarker;
    int commandActionHand;
    int firstChestSlot;
    int lastChestSlot;
    int chestSlotCount;
    int initialChestCount;
    int handStackCount;
    int frontMapX;
    int frontMapY;
} DM1_V1_ChestDropToFloorSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int openResult;
    int openThing;
    int initialChestCount;
    int handStackCountBefore;
    int freeChestSlotsBefore;

    int depositAttempts;
    int depositedCount;
    int handStackCountAfterDeposit;
    int overflowCount;
    int floorDropCount;
    int frontMapX;
    int frontMapY;

    int closeCount;
    int closedChestCount;
    int openThingAfterClose;
    int leaderHandEmptyAfterRun;

    int openedTypes[DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT];
    int afterDepositTypes[DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT];
    int closedTypes[DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT];
    int floorTypes[DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT];

    int chestOrderPreserved;
    int depositOrderPreserved;
    int floorOverflowOrderPreserved;
    int overflowAbsentFromChest;
    int handCountReducedByDeposits;
} DM1_V1_ChestDropToFloorProbePc34;

const char* dm1_v1_chest_drop_to_floor_source_evidence_pc34(void);
const DM1_V1_ChestDropToFloorSpecPc34*
dm1_v1_chest_drop_to_floor_spec_pc34(void);
int dm1_v1_chest_drop_to_floor_pc34_compat_run(
    DM1_V1_ChestDropToFloorProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_DROP_TO_FLOOR_PC34_COMPAT_H */
