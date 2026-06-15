#ifndef FIRESTAFF_DM1_V1_CHEST_REOPEN_AFTER_LEADER_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_REOPEN_AFTER_LEADER_ROTATION_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT = 6,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_MAX_LINK = 9,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_PARTY_COUNT = 4,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LOG_CAPACITY = 8,

    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_BASIC = 0,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_FULL_HAND = 1,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_DOUBLE_ROTATE = 2,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_HIDDEN_TAIL_HAND = 3,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_CLOSE_FULL_HAND = 4,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_EMPTY_NOOP = 5,

    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE = 0xFFFF,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CHEST_A = 0x7201,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CHEST_B = 0x7202,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_AMULET = 0x1A31,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_SHIELD = 0x1A32,
    DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL = 0x1F08
};

typedef enum {
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_NONE = 0,
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_OPEN_CHEST_A,
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_CLOSE_CHEST_A,
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER,
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_OPEN_CHEST_B,
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_CLOSE_CHEST_B,
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER_BACK,
    M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_REOPEN_CHEST_B
} M11_GameView_ChestReopenAfterLeaderRotationActionPc34;

typedef struct {
    M11_GameView_ChestReopenAfterLeaderRotationActionPc34 action;
    int chestThing;
    int openChestThing;
    int leaderOrdinal;
    int inventoryChampionOrdinal;
    int leaderHandThing;
} M11_GameView_ChestReopenAfterLeaderRotationLogEntryPc34;

typedef struct {
    int count;
    M11_GameView_ChestReopenAfterLeaderRotationLogEntryPc34
        entries[DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LOG_CAPACITY];
} M11_GameView_ChestReopenAfterLeaderRotationActionLogPc34;

typedef struct {
    int chestAThing;
    int chestBThing;
    int partyChampionCount;
    int partyRosterOrdinals[
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_PARTY_COUNT];
    int originalLeaderOrdinal;
    int originalInventoryChampionOrdinal;
    int currentLeaderOrdinalAfterRotation;
    int leaderHandThing;
    int chestAVisibleSlots[
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT];
    int chestBVisibleSlots[
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT];
    int chestBHiddenTail;
} M11_GameView_ChestReopenAfterLeaderRotationContextPc34;

typedef struct {
    int chestALinkHead;
    int chestBLinkHead;
    int leaderHandAfter;
    int slotsAfterRotation[
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT];
    int visibleSlotOrderOnReopen[
        DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT];
    int finalOpenChestThing;
    int finalLeaderOrdinal;
    int finalInventoryChampionOrdinal;
    int rotationCount;
    int closeCountChestA;
    int closeCountChestB;
    int reopenedVisibleCount;
    int hiddenTailAfter;
    int noDetachedC30PlusOccupant;
    int leaderHandIdentityPreserved;
    int chestBVisibleOrderPreserved;
    int hiddenTailStaysWithLeader;
    int fullLeaderHandStillFull;
    int emptyLeaderHandNoopPreserved;
} M11_GameView_ChestReopenAfterLeaderRotationExpectedPc34;

typedef struct {
    int caseIndex;
    const char* caseName;
    M11_GameView_ChestReopenAfterLeaderRotationContextPc34 context;
    M11_GameView_ChestReopenAfterLeaderRotationActionLogPc34 actionLog;
    M11_GameView_ChestReopenAfterLeaderRotationExpectedPc34 expected;
} M11_GameView_ChestReopenAfterLeaderRotationCasePc34;

typedef struct {
    int sourceLockedContractOnly;
    int c0xFFFFThingNone;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int caseCount;
    M11_GameView_ChestReopenAfterLeaderRotationCasePc34
        cases[DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT];
} M11_GameView_ChestReopenAfterLeaderRotationProbePc34;

const char*
M11_GameView_ChestReopenAfterLeaderRotationSourceEvidencePc34(void);
const char* M11_GameView_ChestReopenAfterLeaderRotationCaseNamePc34(
    int caseIndex);
int M11_GameView_ChestReopenAfterLeaderRotationBuildCasePc34(
    int caseIndex,
    M11_GameView_ChestReopenAfterLeaderRotationCasePc34* out);
int M11_GameView_ChestReopenAfterLeaderRotationRunPc34(
    M11_GameView_ChestReopenAfterLeaderRotationProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
