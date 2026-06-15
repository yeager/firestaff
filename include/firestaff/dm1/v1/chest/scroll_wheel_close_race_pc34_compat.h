#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_CLOSE_RACE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_CLOSE_RACE_PC34_COMPAT_H

/*
 * DM1 V1 chest close while a scroll-wheel slot command and leader rotation are
 * queued.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens G0426 and copies linked objects to C537..C544.
 * - CHEST.C F0334:113-132 clears G0426 and rewrites visible G0425 slots.
 * - CHAMPION.C F0297/F0298:243-298 own the global C030 leader hand.
 * - CHAMPION.C F0301/F0302:606-714 route C30+ chest slots through G0425.
 * - COMMAND.C F0359:1452-1662 queues mouse commands.
 * - COMMAND.C F0380:2045-2178 drains queued slot-box and rotation commands.
 * - IO.C F0077/F0078:1102-1122 brackets mouse/screen update state.
 * - DEFS.H:267 C030, 790 C10_SLOT_NECK, 2088 C10_COLOR_FLESH, 810 C30,
 *   1876 C38, 3906-3913 C537..C544, 5878 G0425, 5881 G0426, and 3909 C540.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_SCROLL_CLOSE_CHAMPION_COUNT = 2,
    DM1_PC34_SCROLL_CLOSE_OLD_LEADER = 0,
    DM1_PC34_SCROLL_CLOSE_NEW_LEADER = 1,
    DM1_PC34_SCROLL_CLOSE_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_SCROLL_CLOSE_VISIBLE_COUNT = 6,
    DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_INDEX = 3,
    DM1_PC34_SCROLL_CLOSE_QUEUE_ZONE = 540,
    DM1_PC34_SCROLL_CLOSE_QUEUE_SLOT_BOX = 41,
    DM1_PC34_SCROLL_CLOSE_QUEUE_PC34_SLOT = DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_SCROLL_CLOSE_QUEUE_COMMAND = 61,
    DM1_PC34_SCROLL_CLOSE_CHEST_THING = 0x7680,
    DM1_PC34_SCROLL_CLOSE_FIRST_ITEM = 0x7690,
    DM1_PC34_SCROLL_CLOSE_FIRST_CHARGES = 31,
    DM1_PC34_SCROLL_CLOSE_FIRST_QUANTITY = 4,
    DM1_PC34_SCROLL_CLOSE_LEADER_HAND_ITEM = 0x76C1,
    DM1_PC34_SCROLL_CLOSE_LEADER_HAND_WEIGHT = 5,
    DM1_PC34_SCROLL_CLOSE_LEADER_HAND_CHARGES = 12,
    DM1_PC34_SCROLL_CLOSE_PANEL_CHEST = DM1_PC34_PANEL_CHEST,
    DM1_PC34_SCROLL_CLOSE_PANEL_FOOD = DM1_PC34_PANEL_FOOD_WATER_POISONED
};

typedef enum {
    DM1_PC34_SCROLL_CLOSE_STEP_OPEN_CHEST = 0,
    DM1_PC34_SCROLL_CLOSE_STEP_QUEUE_SCROLL = 1,
    DM1_PC34_SCROLL_CLOSE_STEP_CLOSE_CHEST = 2,
    DM1_PC34_SCROLL_CLOSE_STEP_DRAIN_STALE_SCROLL = 3,
    DM1_PC34_SCROLL_CLOSE_STEP_ROTATE_LEADER = 4,
    DM1_PC34_SCROLL_CLOSE_STEP_MOUSE_BALANCED = 5
} DM1_V1_ChestScrollWheelCloseRaceStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297HandAnchor;
    const char* f0298HandAnchor;
    const char* f0301SlotAnchor;
    const char* f0302DispatchAnchor;
    const char* f0359QueueAnchor;
    const char* f0380DrainAnchor;
    const char* f0077EnableAnchor;
    const char* f0078DisableAnchor;
    const char* defsAnchor;
    const char* nonDuplicateMarker;
    int oldLeaderIndex;
    int newLeaderIndex;
    int queuedZone;
    int queuedSlotBox;
    int queuedPc34Slot;
    int queuedCommand;
    int expectedCloseCount;
} DM1_V1_ChestScrollWheelCloseRaceSpecPc34;

typedef struct {
    int stepTrace[6];
    int stepCount;
    int runtimeRegression;

    int openResult;
    int openChestThingBeforeQueue;
    int panelBeforeQueue;
    int leaderBeforeQueue;
    int oldLeaderHandTypeBeforeQueue;
    int oldLeaderHandWeightBeforeQueue;
    int oldLeaderHandChargesBeforeQueue;
    int newLeaderHandTypeBeforeQueue;

    int scrollQueued;
    int rotationQueued;
    int queuedChampion;
    int queuedNewLeader;
    int queuedZone;
    int queuedSlotBox;
    int queuedPc34Slot;
    int queuedCommand;
    int commandQueueDepthAfterQueue;
    int mouseUpdateDepthAfterQueue;
    int f0077Observed;
    int f0078Pending;

    int visibleTypesBeforeClose[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int visibleChargesBeforeClose[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int visibleQuantitiesBeforeClose[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int queuedSlotTypeBeforeClose;
    int c537ToC544MaterializedBeforeClose;

    int closeCount;
    int openChestThingAfterClose;
    int panelAfterCloseRoute;
    int commandQueueDepthAfterClose;
    int mouseUpdateDepthAfterClose;
    int leaderAfterClose;
    int oldLeaderHandTypeAfterClose;
    int oldLeaderHandWeightAfterClose;
    int oldLeaderHandChargesAfterClose;
    int closedTypes[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int closedCharges[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int closedQuantities[DM1_PC34_SCROLL_CLOSE_SLOT_COUNT];
    int closedChainPreserved;
    int queuedSlotPreservedInClosedChain;

    int staleScrollDrainAttempted;
    int staleScrollRejected;
    int commandQueueDepthAfterStaleDrain;
    int mouseUpdateDepthAfterStaleDrain;
    int openChestThingAfterStaleDrain;
    int oldLeaderHandTypeAfterStaleDrain;
    int oldLeaderHandWeightAfterStaleDrain;
    int queuedSlotNotPulledAfterClose;
    int noChestSlotMutationAfterClose;

    int rotationConsumed;
    int commandQueueDepthAfterRotate;
    int leaderAfterRotate;
    int oldLeaderOpenChestAfterRotate;
    int newLeaderOpenChestAfterRotate;
    int oldLeaderHandTypeAfterRotate;
    int newLeaderHandTypeAfterRotate;
    int newLeaderHandWeightAfterRotate;
    int newLeaderHandChargesAfterRotate;
    int leaderHandCoherentAfterRotate;
    int f0077F0078Balanced;
    int panelRemainsClosedAfterRotate;

    int noPickupDropPath;
    int noDepositPath;
    int noReopenPath;
    int noMirrorCandidatePath;
    int noResurrectPendingPath;
    int noOccupiedSlotSwapPath;
    int noPartialMaskPath;
    int noDifferentChestOpenPath;
    int noSaveLoadTeleporterPath;
    int noCapacityEncumbrancePath;

    uint32_t deterministicHash;
} DM1_V1_ChestScrollWheelCloseRaceProbePc34;

const char*
dm1_v1_chest_scroll_wheel_close_race_source_evidence_pc34(void);
const DM1_V1_ChestScrollWheelCloseRaceSpecPc34*
dm1_v1_chest_scroll_wheel_close_race_spec_pc34(void);
int dm1_v1_chest_scroll_wheel_close_race_run_pc34(
    DM1_V1_ChestScrollWheelCloseRaceProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_CLOSE_RACE_PC34_COMPAT_H */
