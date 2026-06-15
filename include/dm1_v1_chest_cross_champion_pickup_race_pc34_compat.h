#ifndef FIRESTAFF_DM1_V1_CHEST_CROSS_CHAMPION_PICKUP_RACE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_CROSS_CHAMPION_PICKUP_RACE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_PICKUP_RACE_CHAMPION_COUNT = 2,
    DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX = 2,
    DM1_PC34_CHEST_PICKUP_RACE_PC34_SLOT =
        DM1_PC34_SLOT_CHEST_1 + DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX,
    DM1_PC34_CHEST_PICKUP_RACE_FIRST_CHAMPION = 0,
    DM1_PC34_CHEST_PICKUP_RACE_SECOND_CHAMPION = 1,
    DM1_PC34_CHEST_PICKUP_RACE_CHEST_THING = 0x7A10,
    DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM = 0x7B10,
    DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM =
        DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM +
        DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX
};

typedef struct {
    const char* contractMarker;
    int championCount;
    int chestSlotCount;
    int raceSlotIndex;
    int racePc34Slot;
    int chestThing;
    int pickedItem;
} DM1_V1_ChestCrossChampionPickupRaceSpecPc34;

typedef struct {
    int contractOnly;
    int openResult;
    int openThing;
    int initialVisibleCount;
    int initialRaceSlotType;
    int initialRaceSlotWeight;
    int initialTypes[DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT];

    int firstClickResult;
    int firstHandAfterType;
    int firstHandAfterWeight;
    int firstRaceSlotAfterType;
    int firstChampionWonPickup;

    int secondClickResult;
    int secondHandAfterType;
    int secondRaceSlotAfterType;
    int secondNoopPath;
    int raceSlotStableAfterSecondClick;

    int winnerCount;
    int pickedItemCopyCountAfterRace;
    int visibleCountAfterRace;
    int afterRaceTypes[DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT];

    int closeCount;
    int closedTypes[DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT];
    int pickedItemAbsentFromClosedLinks;
    int closeCompactedEmptyRaceSlot;

    int reopenResult;
    int reopenedVisibleCount;
    int reopenedTypes[DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT];
    int pickedItemAbsentAfterReopen;
    int reopenedOrderCompacted;
} DM1_V1_ChestCrossChampionPickupRaceProbePc34;

const char* dm1_v1_chest_cross_champion_pickup_race_source_evidence_pc34(void);
const DM1_V1_ChestCrossChampionPickupRaceSpecPc34*
dm1_v1_chest_cross_champion_pickup_race_spec_pc34(void);
int dm1_v1_chest_cross_champion_pickup_race_run_pc34(
    DM1_V1_ChestCrossChampionPickupRaceProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_CROSS_CHAMPION_PICKUP_RACE_PC34_COMPAT_H */
