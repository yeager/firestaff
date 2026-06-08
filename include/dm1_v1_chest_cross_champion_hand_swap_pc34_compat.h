#ifndef FIRESTAFF_DM1_V1_CHEST_CROSS_CHAMPION_HAND_SWAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_CROSS_CHAMPION_HAND_SWAP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT = 6,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_PARTY_COUNT = 4,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_LOG_CAPACITY = 10,

    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE = 0xFFFF,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_END_OF_LIST = 0xFFFE,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CHEST_THING = 0x7300,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_FIRST_CHEST_ITEM = 0x6100,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CHAMPION_A_HAND = 0x6A01,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CHAMPION_B_HAND = 0x6B01,

    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_FULL = 0,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SPARSE_FIVE = 1,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SAME_OPEN_NOOP = 2,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_DOUBLE_ROTATION = 3,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_INVENTORY_A = 4,
    DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_WEIGHTED = 5
};

typedef enum {
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_NONE = 0,
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_OPEN_CHEST,
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_SAME_OPEN_NOOP,
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_PICKUP_A_HAND,
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_ROTATE_LEADER,
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_SWAP_WITH_B_HAND,
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_CLOSE_CHEST,
    M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_REOPEN_CHEST
} M11_GameView_ChestCrossChampionHandSwapActionPc34;

typedef struct {
    M11_GameView_ChestCrossChampionHandSwapActionPc34 action;
    int openChestThing;
    int leaderOrdinal;
    int inventoryChampionOrdinal;
    int leaderHandThing;
    int championAHandThing;
    int championBHandThing;
} M11_GameView_ChestCrossChampionHandSwapLogEntryPc34;

typedef struct {
    int count;
    M11_GameView_ChestCrossChampionHandSwapLogEntryPc34
        entries[DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_LOG_CAPACITY];
} M11_GameView_ChestCrossChampionHandSwapActionLogPc34;

typedef struct {
    int caseIndex;
    const char* caseName;
    int chestThing;
    int linkedCount;
    int originalLeaderOrdinal;
    int rotatedLeaderOrdinal;
    int inventoryChampionOrdinal;
    int championAOrdinal;
    int championBOrdinal;
    int mapX;
    int mapY;
    int facing;
    int chestLinkedThings[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int chestLinkedWeights[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int championAInitialHandThing;
    int championBInitialHandThing;
    int championAInitialHandWeight;
    int championBInitialHandWeight;
    int sameOpenBeforeSwap;
    int rotateTwiceWhileOpen;
} M11_GameView_ChestCrossChampionHandSwapContextPc34;

typedef struct {
    int openResult;
    int sameOpenNoopResult;
    int openChestThingAfterOpen;
    int visibleCountAfterOpen;
    int openedSlotThings[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int openedSlotWeights[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];

    int pickupAResult;
    int leaderHandAfterPickup;
    int championAHandAfterPickup;
    int rotationCountWhileOpen;
    int leaderOrdinalAfterRotation;
    int inventoryChampionOrdinalDuringRotation;
    int openChestThingDuringRotation;
    int mapXAfterRotation;
    int mapYAfterRotation;
    int facingAfterRotation;
    int portraitSwitchCount;
    int redrawCadenceCount;

    int swapWithBResult;
    int leaderHandAfterSwap;
    int championAHandAfterSwap;
    int championBHandAfterSwap;
    int removedLeaderHandCall;
    int removedChampionBSlotCall;
    int putBObjectInLeaderHandCall;
    int putAObjectInChampionBSlotCall;

    int closeResult;
    int closeCount;
    int closedLinkHead;
    int closedLinkTail;
    int closedLinkThings[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int closedLinkWeights[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int slotsClearedAfterClose[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int openChestThingAfterClose;

    int reopenResult;
    int openChestThingAfterReopen;
    int reopenedVisibleCount;
    int reopenedSlotThings[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];
    int reopenedSlotWeights[
        DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT];

    int linkOrderPreserved;
    int g0425ViewMatchesClosedLinks;
    int leaderHandIdentityPreserved;
    int inventoryChampionIdentityPreserved;
    int inventoryChampionHandStateCoherent;
    int crossChampionSwapCoherent;
    int dungeonPositionInvariant;
    int noDetachedC30PlusOccupant;
    int sparseClearCoherent;
    int sameOpenNoopPreserved;
    int blitRoutingPresentationOnly;
    int c10ColorFleshKnown;
} M11_GameView_ChestCrossChampionHandSwapExpectedPc34;

typedef struct {
    M11_GameView_ChestCrossChampionHandSwapContextPc34 context;
    M11_GameView_ChestCrossChampionHandSwapActionLogPc34 actionLog;
    M11_GameView_ChestCrossChampionHandSwapExpectedPc34 expected;
} M11_GameView_ChestCrossChampionHandSwapCasePc34;

typedef struct {
    int sourceLockedRuntimeGate;
    int c10ColorFlesh;
    int c30ChestSlotBase;
    int g0425SlotCount;
    int g0426NoneSentinel;
    int g0423InventoryChampionOrdinalStart;
    int g0305PartyChampionCount;
    int caseCount;
    M11_GameView_ChestCrossChampionHandSwapCasePc34
        cases[DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT];
} M11_GameView_ChestCrossChampionHandSwapProbePc34;

const char*
M11_GameView_ChestCrossChampionHandSwapSourceEvidencePc34(void);
const char* M11_GameView_ChestCrossChampionHandSwapCaseNamePc34(
    int caseIndex);
int M11_GameView_ChestCrossChampionHandSwapBuildCasePc34(
    int caseIndex,
    M11_GameView_ChestCrossChampionHandSwapCasePc34* out);
int M11_GameView_ChestCrossChampionHandSwapRunPc34(
    M11_GameView_ChestCrossChampionHandSwapProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
