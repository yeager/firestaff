#ifndef FIRESTAFF_DM1_V1_CHEST_DESTINATION_GUARD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_DESTINATION_GUARD_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_DESTINATION_GUARD_NONE = -1,
    DM1_PC34_CHEST_DESTINATION_GUARD_SLOT_COUNT = 8,
    DM1_PC34_CHEST_DESTINATION_GUARD_FIRST_SLOT = 30,
    DM1_PC34_CHEST_DESTINATION_GUARD_LAST_SLOT = 37,
    DM1_PC34_CHEST_DESTINATION_GUARD_ITEM = 0x5201,
    DM1_PC34_CHEST_DESTINATION_GUARD_EXISTING_ITEM = 0x5202,
    DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST = 0x7C50,
    DM1_PC34_CHEST_DESTINATION_GUARD_CLOSED_CHEST = 0,
    DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE = -1,
    DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_CORRIDOR = 1,
    DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_TELEPORTER = 5,
    DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_FAKEWALL = 6
};

typedef enum {
    DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_INTERNAL = 0,
    DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_CORRIDOR = 1,
    DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_TELEPORTER = 5,
    DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_FAKEWALL = 6
} DM1_V1_ChestDestinationGuardDestinationPc34;

typedef struct {
    const char* contractMarker;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int cm1MapXNotOnASquare;
    int teleporterElement;
    int fakewallElement;
    int corridorElement;
} DM1_V1_ChestDestinationGuardSpecPc34;

typedef struct {
    const char* contractScope;
    const char* redmcsbChestOpen;
    const char* redmcsbChestClose;
    const char* redmcsbChampionSlotRoute;
    const char* redmcsbDungeonLink;
    const char* redmcsbDungeonAspect;
    const char* redmcsbDefs;
    const char* disjointCoverage;
} DM1_V1_ChestDestinationGuardEvidencePc34;

typedef struct {
    int result;
    int rejected;
    int stateHashBefore;
    int stateHashAfter;
    int stateStable;
    int leaderHandAfter;
    int destinationSlotAfter;
    int openChestThingAfter;
    int dungeonLinkCountAfter;
    int squareFirstThingAfter;
} DM1_V1_ChestDestinationGuardAttemptPc34;

typedef struct {
    int contract_only;
    int no_real_asset_data;
    int openChestThing;
    int leaderHandItem;
    int targetChestSlot;
    int occupiedSentinel;
    int initialDungeonLinkCount;
    int initialSquareFirstThing;

    DM1_V1_ChestDestinationGuardAttemptPc34 teleporterAttempt;
    DM1_V1_ChestDestinationGuardAttemptPc34 fakewallAttempt;
    DM1_V1_ChestDestinationGuardAttemptPc34 corridorSquareAttempt;
    DM1_V1_ChestDestinationGuardAttemptPc34 closedChestAttempt;
    DM1_V1_ChestDestinationGuardAttemptPc34 internalChestAttempt;

    int internalStoredItem;
    int internalLeaderHandEmpty;
    int internalNoDungeonLink;
    int internalOpenChestStable;
    int rejectedAttemptsStable;
} DM1_V1_ChestDestinationGuardProbePc34;

const DM1_V1_ChestDestinationGuardEvidencePc34*
M11_GameView_ChestDestinationGuardEvidencePc34(void);
const DM1_V1_ChestDestinationGuardSpecPc34*
M11_GameView_ChestDestinationGuardSpecPc34(void);
int M11_GameView_ChestDestinationGuardRunPc34(
    DM1_V1_ChestDestinationGuardProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_DESTINATION_GUARD_PC34_COMPAT_H */
