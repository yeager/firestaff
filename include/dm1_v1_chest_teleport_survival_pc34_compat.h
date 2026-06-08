#ifndef FIRESTAFF_DM1_V1_CHEST_TELEPORT_SURVIVAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_TELEPORT_SURVIVAL_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT = 3,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT = 4,

    DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE = 0xFFFF,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_END_OF_LIST = 0xFFFE,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A = 4,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B = 7,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A = 0x7701,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST = 0x4401,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_THING = 0x6610,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_WEIGHT = 9,

    DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_OPEN_A = 0,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_TELEPORT_B = 1,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_TELEPORT_A = 2,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_CLOSE_A = 3,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_REOPEN_A = 4,
    DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_COUNT = 5
};

typedef struct {
    const char* chestF0333OpenMaterialization;
    const char* chestF0334CloseRewrite;
    const char* championF0297LeaderHandPut;
    const char* championF0298LeaderHandRemove;
    const char* championF0300ChestSlotClear;
    const char* championF0301ChestSlotWrite;
    const char* championF0302OccupiedSlotSwap;
    const char* dungeonF0163LinkAppend;
    const char* dungeonF0173F0174MapSet;
    const char* movesensF0267TeleportLevelChange;
    const char* objectF0033IconIndex;
    const char* blitmaskF0133PresentationRoute;
    const char* defsSentinelsAndSlots;
} M11_GameView_ChestTeleportSurvivalAnchorsPc34;

typedef struct {
    const char* phaseName;
    int currentMapIndex;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int openChestThing;
    int chestThing;
    int chestOwningMapIndex;
    int chestResolvedOnCurrentMap;
    int g0425Slots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT];
    int chestLinkThings[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT];
    int chestVisibleCount;
    int chestLinkHead;
    int chestLinkTail;
    int leaderHandThing;
    int leaderHandWeight;
    int leaderHandIconIndex;
    int leaderHandNameRefreshed;
    int leaderHandPointerStable;
    int leaderHandRemoveCount;
    int leaderHandPutCount;
    int leaderLoad;
    int aliveChampionCount;
    int championHealth[DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT];
    int mapSetCount;
    int teleportCount;
    int closeRewriteCount;
    int closeRecompactCount;
    int closeClearedOpenChest;
    int closeWithoutOpenEarlyReturnCount;
    int objectIconLookupCount;
    int blitRouteCount;
    int c30SlotClearWriteObserved;
} M11_GameView_ChestTeleportSurvivalSnapshotPc34;

typedef struct {
    int beforeMapIndex;
    int afterMapIndex;
    int closeAttemptedOnMap;
    int openChestThingAfterCloseAttempt;
    int g0425SlotsAfterCloseAttempt[
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT];
    int chestLinkThingsAfterCloseAttempt[
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT];
    int closeRewriteCount;
    int closeWithoutOpenEarlyReturnCount;
    int chestResolvedOnCurrentMap;
    int leaderHandThingAfterCloseAttempt;
    int leaderHandWeightAfterCloseAttempt;
    int preservedOpenChestOnForeignMap;
    int preservedG0425OnForeignMap;
    int preservedLeaderHandOnForeignMap;
} M11_GameView_ChestTeleportSurvivalNegativePc34;

typedef struct {
    int sourceLockedContractOnly;
    int c0xFFFFThingNone;
    int c0xFFFEThingEndOfList;
    int c30ChestSlotBase;
    int c37ChestSlotLast;
    int g0425SlotCount;
    int partyChampionCount;
    int allChampionsAlive;
    int leaderOrdinal;
    int inventoryChampionOrdinal;
    int chestThing;
    int chestMapIndex;
    int teleportDestinationMapIndex;
    int initialChestItems[DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT];
    int initialChestWeights[DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT];
    int initialLeaderHandThing;
    int initialLeaderHandWeight;
    M11_GameView_ChestTeleportSurvivalAnchorsPc34 anchors;
    M11_GameView_ChestTeleportSurvivalSnapshotPc34
        snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_COUNT];
    M11_GameView_ChestTeleportSurvivalNegativePc34 negative;
} M11_GameView_ChestTeleportSurvivalProbePc34;

const char* M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(void);
int M11_GameView_ChestTeleportSurvivalRunPc34(
    M11_GameView_ChestTeleportSurvivalProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
