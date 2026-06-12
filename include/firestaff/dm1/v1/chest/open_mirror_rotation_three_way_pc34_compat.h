#ifndef FIRESTAFF_DM1_V1_CHEST_OPEN_MIRROR_ROTATION_THREE_WAY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OPEN_MIRROR_ROTATION_THREE_WAY_PC34_COMPAT_H

/*
 * Contract-only DM1 V1 runtime regression model for a C540 chest wheel swap
 * while a non-leader owns G0426, a different champion has a live C040
 * resurrect/reincarnate candidate chain, and a leader rotation is queued
 * behind the wheel event.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens G0426 and materializes C537..C544 in G0425.
 * - CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 slots.
 * - CHAMPION.C F0297:243-298 and F0298:270-298 own leader-hand lifetime.
 * - CHAMPION.C F0300:511-515 clears C30+ G0425 slots.
 * - CHAMPION.C F0301:606-614 writes C30+ G0425 slots.
 * - CHAMPION.C F0302:662-714 routes occupied C537..C544 slot clicks.
 * - COMMAND.C F0359:1452-1662 queues mouse commands.
 * - COMMAND.C F0361:1709-1813 writes keyboard/wheel-like queue commands.
 * - COMMAND.C F0380:2045-2178 drains exactly one queued command.
 * - IO.C F0077/F0078:1102-1122 brackets mouse/screen update state.
 * - REVIVE.C F0280:124-132 publishes G0299/C040 candidate state.
 * - REVIVE.C F0282:744-806 clears G0299 and the candidate chain.
 * - DUNGEON.C F0163:1796-1837 relinks item lists fed by G0425.
 * - DEFS.H binds C30/G0425/G0426/G0423/G0305/M070/M516/C040,
 *   C160..C162, C537..C544, and C159.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_COMR3_CHAMPION_COUNT = 3,
    DM1_PC34_COMR3_LEADER_BEFORE = 0,
    DM1_PC34_COMR3_OPEN_NON_LEADER = 1,
    DM1_PC34_COMR3_CANDIDATE_CHAMPION = 2,
    DM1_PC34_COMR3_LEADER_AFTER_ROTATION = 1,
    DM1_PC34_COMR3_CANDIDATE_ORDINAL = 3,
    DM1_PC34_COMR3_INVENTORY_ORDINAL = 2,
    DM1_PC34_COMR3_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_COMR3_TARGET_SLOT_INDEX = 3,
    DM1_PC34_COMR3_TARGET_ZONE = 540,
    DM1_PC34_COMR3_TARGET_SLOT_BOX = 41,
    DM1_PC34_COMR3_TARGET_PC34_SLOT = DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_COMR3_TARGET_COMMAND = 61,
    DM1_PC34_COMR3_ROTATION_COMMAND = 17,
    DM1_PC34_COMR3_C160_RESURRECT = 160,
    DM1_PC34_COMR3_C161_REINCARNATE = 161,
    DM1_PC34_COMR3_C162_CANCEL = 162,
    DM1_PC34_COMR3_C040_GRAPHIC = 40,
    DM1_PC34_COMR3_CHEST_THING = 0x7790,
    DM1_PC34_COMR3_FIRST_STABLE_ITEM = 0x77B0,
    DM1_PC34_COMR3_TARGET_SLOT_ITEM = 0x77C3,
    DM1_PC34_COMR3_NON_LEADER_HAND_ITEM = 0x77D4,
    DM1_PC34_COMR3_CANDIDATE_HAND_ITEM = 0x77E2,
    DM1_PC34_COMR3_FIRST_CHARGES = 61,
    DM1_PC34_COMR3_FIRST_QUANTITY = 4,
    DM1_PC34_COMR3_TARGET_SLOT_CHARGES = 97,
    DM1_PC34_COMR3_TARGET_SLOT_QUANTITY = 12,
    DM1_PC34_COMR3_HAND_CHARGES = 43,
    DM1_PC34_COMR3_HAND_QUANTITY = 8,
    DM1_PC34_COMR3_HAND_WEIGHT = 15
};

typedef enum {
    DM1_PC34_COMR3_STEP_DEFAULT_STATE = 0,
    DM1_PC34_COMR3_STEP_QUEUE_WHEEL_AND_ROTATION = 1,
    DM1_PC34_COMR3_STEP_DRAIN_WHEEL_C540 = 2,
    DM1_PC34_COMR3_STEP_DRAIN_ROTATION = 3,
    DM1_PC34_COMR3_STEP_ASSERT_C040_STILL_LIVE = 4
} DM1_V1_ChestOpenMirrorRotationThreeWayStepPc34;

typedef struct {
    int type;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34;

typedef struct {
    int command;
    int championIndex;
    int zone;
    int slotBox;
    int pc34Slot;
} DM1_V1_ChestOpenMirrorRotationThreeWayCommandPc34;

typedef struct {
    int contractOnly;
    int assetFree;
    int noDosPixelParityClaim;

    M11_InventoryState inventory;
    int currentLeaderIndex;
    int inventoryChampionOrdinal;
    int partyChampionCount;
    int g0426OpenChestThing;
    int g0423InventoryChampionOrdinal;

    int g0299CandidateOrdinal;
    int c040CandidateChampionIndex;
    int c040PanelLive;
    int c040Graphic;
    int c160Command;
    int c161Command;
    int c162Command;
    int candidateChainOrdinals[3];
    int candidateHandQueueDepth;
    DM1_V1_ChestOpenMirrorRotationThreeWayItemPc34 candidateHandQueueItem;

    int chestQuantities[DM1_PC34_COMR3_CHAMPION_COUNT]
                       [DM1_PC34_COMR3_SLOT_COUNT];
    int handQuantities[DM1_PC34_COMR3_CHAMPION_COUNT];
    int commandQueueDepth;
    DM1_V1_ChestOpenMirrorRotationThreeWayCommandPc34 commandQueue[4];
    int wheelQueued;
    int leaderRotationQueued;

    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutHandCount;
    int f0298RemoveHandCount;
    int f0300ClearC30Count;
    int f0301WriteC30Count;
    int f0302DispatchCount;
    int f0359QueueWriteCount;
    int f0361WheelQueueWriteCount;
    int f0380DrainCount;
    int f0077EnableCount;
    int f0078DisableCount;
    int f0280PublishCount;
    int f0282ClearCount;
    int f0163RelinkCount;

    int trace[5];
    int traceCount;
} DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297HandAnchor;
    const char* f0298HandAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* f0359QueueAnchor;
    const char* f0361WheelQueueAnchor;
    const char* f0380DrainAnchor;
    const char* f0077EnableAnchor;
    const char* f0078DisableAnchor;
    const char* f0280PublishAnchor;
    const char* f0282ClearAnchor;
    const char* f0163RelinkAnchor;
    const char* defsAnchor;
    const char* nonDuplicateMarker;
} DM1_V1_ChestOpenMirrorRotationThreeWaySpecPc34;

typedef struct {
    int runtimeRegression;
    int stepTrace[5];
    int stepCount;

    int leaderBefore;
    int nonLeaderOpenChampion;
    int candidateChampionIndex;
    int candidateOrdinalBefore;
    int candidateChainBefore[3];
    int candidateHandQueueDepthBefore;
    int candidateHandQueueItemBefore;
    int openChestThingBefore;
    int g0426OpenBefore;
    int panelContentBefore;
    int handTypeBefore;
    int handQuantityBefore;
    int targetSlotTypeBefore;
    int targetSlotQuantityBefore;
    int visibleTypesBefore[DM1_PC34_COMR3_SLOT_COUNT];
    int visibleQuantitiesBefore[DM1_PC34_COMR3_SLOT_COUNT];

    int wheelQueued;
    int rotationQueued;
    int commandQueueDepthAfterQueue;
    int queuedWheelCommand;
    int queuedRotationCommand;
    int queuedWheelBeforeRotation;

    int wheelDrainResult;
    int wheelDrainedBeforeRotation;
    int f0077F0078BalancedAfterWheel;
    int f0302DispatchCountAfterWheel;
    int f0300ClearCountAfterWheel;
    int f0301WriteCountAfterWheel;
    int handTypeAfterWheel;
    int handQuantityAfterWheel;
    int c540TypeAfterWheel;
    int c540QuantityAfterWheel;
    int commandQueueDepthAfterWheel;
    int rotationStillQueuedAfterWheel;
    int candidateOrdinalAfterWheel;
    int candidateHandQueueDepthAfterWheel;
    int candidateHandQueueItemAfterWheel;
    int candidateChainStableAfterWheel;
    int g0299StableAfterWheel;
    int g0426StableAfterWheel;
    int panelStillChestAfterWheel;

    int rotationDrainResult;
    int leaderAfterRotation;
    int commandQueueDepthAfterRotation;
    int candidateOrdinalAfterRotation;
    int candidateHandQueueDepthAfterRotation;
    int candidateHandQueueItemAfterRotation;
    int candidateChainStableAfterRotation;
    int g0426StableAfterRotation;
    int panelStillChestAfterRotation;
    int handTypeAfterRotation;
    int c540TypeAfterRotation;

    int f0282NeverDrainedCandidate;
    int chestNeverClosed;
    int c537ToC544ChainCoherentAfterRotation;
    int noAssetRead;
    int noPixelParityClaim;
    int nonDuplicateThreeWay;

    uint32_t deterministicHash;
} DM1_V1_ChestOpenMirrorRotationThreeWayProbePc34;

const char*
dm1_v1_chest_open_mirror_rotation_three_way_source_evidence_pc34(void);
const DM1_V1_ChestOpenMirrorRotationThreeWaySpecPc34*
dm1_v1_chest_open_mirror_rotation_three_way_spec_pc34(void);
DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34
dm1_v1_chest_open_mirror_rotation_three_way_default_state_pc34(void);
uint32_t dm1_v1_chest_open_mirror_rotation_three_way_hash_state_pc34(
    const DM1_V1_ChestOpenMirrorRotationThreeWayStatePc34* state);
int dm1_v1_chest_open_mirror_rotation_three_way_run_pc34(
    DM1_V1_ChestOpenMirrorRotationThreeWayProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
