#ifndef FIRESTAFF_DM1_V1_CHEST_MULTI_CHAMPION_CLOSE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_MULTI_CHAMPION_CLOSE_PC34_COMPAT_H

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_MULTI_CHAMPION_COUNT = 3,
    DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_MULTI_CHAMPION_STAFF_OF_CLAWS_INFO = 27
};

typedef struct {
    const char* contractMarker;
    int championCount;
    int c537Pc34Slot;
    int c540Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int staffOfClawsAllowedSlots;
} DM1_V1_ChestMultiChampionCloseSpecPc34;

typedef struct {
    int contractOnly;
    int championCount;
    int c537Mask;
    int c540Mask;
    int c544Mask;
    int staffOfClawsAllowedSlots;
    int staffRejectedFromChest;
    int staffAcceptedInQuiver;

    int targetPc34Slots[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int targetSlotIndexes[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int linkedCounts[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int baseLoads[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];

    int openResults[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int openThings[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int visibleWeightsAfterOpen[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int loadsAfterOpen[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int movementTicksAfterOpen[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];

    int handBeforeTypes[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int handBeforeWeights[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int displacedTypes[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int displacedWeights[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int clickResults[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int handAfterTypes[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int slotAfterTypes[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int visibleWeightsAfterClick[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int loadsAfterClick[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int movementTicksAfterClick[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];

    int closeCounts[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int closeContainerSnapshots[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int openThingsAfterClose[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int loadsAfterClose[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int movementTicksAfterClose[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int replacementClosedAtTarget[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int displacedAbsentFromClosed[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT];
    int closedTypes[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT]
                   [DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT];
    int closedWeights[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT]
                     [DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT];
} DM1_V1_ChestMultiChampionCloseProbePc34;

const char* dm1_v1_chest_multi_champion_close_source_evidence_pc34(void);
const DM1_V1_ChestMultiChampionCloseSpecPc34*
dm1_v1_chest_multi_champion_close_spec_pc34(void);
int dm1_v1_chest_multi_champion_close_run_pc34(
    DM1_V1_ChestMultiChampionCloseProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_MULTI_CHAMPION_CLOSE_PC34_COMPAT_H */
