#ifndef FIRESTAFF_DM1_V1_CHEST_REOPEN_CROSS_CHAMPION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_REOPEN_CROSS_CHAMPION_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_REOPEN_CROSS_LEADER_A = 0,
    DM1_PC34_CHEST_REOPEN_CROSS_LEADER_B = 1,
    DM1_PC34_CHEST_REOPEN_CROSS_PARTY_COUNT = 2,
    DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_REOPEN_CROSS_A_COUNT = 3,
    DM1_PC34_CHEST_REOPEN_CROSS_B_COUNT = 4,
    DM1_PC34_CHEST_REOPEN_CROSS_A_FIRST = 4100,
    DM1_PC34_CHEST_REOPEN_CROSS_B_FIRST = 5100,
    DM1_PC34_CHEST_REOPEN_CROSS_A_BASE_ITEM = 0x4A01,
    DM1_PC34_CHEST_REOPEN_CROSS_B_BASE_ITEM = 0x4B01,
    DM1_PC34_CHEST_REOPEN_CROSS_HAND_ITEM = 0x4C01,
    DM1_PC34_CHEST_REOPEN_CROSS_HAND_WEIGHT = 13,
    DM1_PC34_CHEST_REOPEN_CROSS_A_THING = 0x7C10,
    DM1_PC34_CHEST_REOPEN_CROSS_B_THING = 0x7C20
};

typedef struct {
    const char* contractMarker;
    int leaderAIndex;
    int leaderBIndex;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int chestAFirstItem;
    int chestBFirstItem;
    int leaderHandItem;
    int leaderHandWeight;
} DM1_V1_ChestReopenCrossChampionSpecPc34;

typedef struct {
    int contractOnly;
    int leaderAIndex;
    int leaderBIndex;
    int chestAThing;
    int chestBThing;

    int leaderABaseSetResult;
    int leaderBBaseSetResult;
    int leaderABaseLoad;
    int leaderBBaseLoad;

    int leaderAHandSetupResult;
    int leaderAHandBeforeSwitchType;
    int leaderAHandBeforeSwitchWeight;
    int leaderAHandAfterSwitchType;
    int leaderAHandClearedAfterSwitch;
    int leaderBHandAfterSwitchType;
    int leaderBHandAfterSwitchWeight;
    int leaderBHandOccupiedAfterSwitch;

    int chestAOpenResult;
    int chestAOpenThing;
    int chestAOpenedTypes[DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT];
    int chestAOpenedVisibleWeight;
    int leaderAPanelLoadAfterOpen;

    int chestACloseCount;
    int chestAClosedTypes[DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT];
    int chestACloseContainerSnapshot;
    int leaderAPanelLoadAfterClose;

    int leaderSwitchResult;
    int leaderSwitchPrevious;
    int leaderSwitchNew;
    int leaderAStateLoadAfterSwitch;
    int leaderBStateLoadAfterSwitch;
    int leaderBPanelLoadAfterSwitch;

    int chestBReopenResult;
    int chestBOpenThing;
    int chestBReopenedTypes[DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT];
    int chestBReopenedVisibleCount;
    int chestBReopenedVisibleWeight;
    int chestBOrderMatchesLeaderB;
    int chestBOrderLeaksLeaderA;
    int leaderBPanelLoadAfterReopen;
    int leaderBReopenLoadDelta;
    int reopenDoesNotDoubleCountLinkWeights;

    int chestBCloseCount;
    int chestBClosedTypes[DM1_PC34_CHEST_REOPEN_CROSS_SLOT_COUNT];
    int chestBCloseContainerSnapshot;
    int leaderBPanelLoadAfterClose;
    int containerSnapshotWeightConsistent;
} DM1_V1_ChestReopenCrossChampionProbePc34;

extern const DM1_V1_ChestReopenCrossChampionSpecPc34
    dm1_v1_chest_reopen_cross_champion_pc34_spec;

const char* M11_GameView_ChestReopenCrossChampionSourceEvidencePc34(void);
const DM1_V1_ChestReopenCrossChampionSpecPc34*
M11_GameView_ChestReopenCrossChampionSpecPc34(void);
int M11_GameView_ChestReopenCrossChampionRunPc34(
    DM1_V1_ChestReopenCrossChampionProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_REOPEN_CROSS_CHAMPION_PC34_COMPAT_H */
