#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_DROP_DURING_ROTATION_NON_LEADER_OPEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_DROP_DURING_ROTATION_NON_LEADER_OPEN_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel drop while leader rotation is queued and a
 * non-leader inventory owns the live open chest panel.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens G0426 and materializes C537..C544 in G0425.
 * - CHEST.C F0334:113-132 would clear G0426 and relink visible G0425 slots;
 *   this gate asserts that path is not taken.
 * - CHAMPION.C F0297/F0298:243-298 own the global C030 leader hand.
 * - CHAMPION.C F0301/F0302:606-714 route C30+ chest slots through G0425.
 * - COMMAND.C F0359:1452-1662 queues mouse commands.
 * - COMMAND.C F0380:2045-2178 drains the queued C540 slot command before the
 *   queued leader rotation in this race.
 * - IO.C F0077/F0078:1102-1122 brackets mouse/screen update state.
 * - DEFS.H:790 C10_SLOT_NECK, 810 C30, 2088 C10_COLOR_FLESH,
 *   3906-3913 C537..C544, and C540.
 *
 * Non-duplicative marker: pass771 covers drop-while-rotation with a non-leader
 * open G0426 panel and post-rotation hand continuity; it is not pass768 close
 * race, pickup/drop, drop-onto-open-slot, deposit-during-rotation,
 * resurrect-pending pickup, candidate-live close, open-during-pending,
 * close/open with full leader hand, or any mirror/chest close regression.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_SW_DROP_ROT_NLO_CHAMPION_COUNT = 2,
    DM1_PC34_SW_DROP_ROT_NLO_OLD_LEADER = 0,
    DM1_PC34_SW_DROP_ROT_NLO_NON_LEADER_OPEN = 1,
    DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER = 1,
    DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_INDEX = 3,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_ZONE = 540,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_SLOT_BOX = 41,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_PC34_SLOT = DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_SW_DROP_ROT_NLO_TARGET_COMMAND = 61,
    DM1_PC34_SW_DROP_ROT_NLO_CHEST_THING = 0x7710,
    DM1_PC34_SW_DROP_ROT_NLO_FIRST_STABLE_ITEM = 0x7720,
    DM1_PC34_SW_DROP_ROT_NLO_DROP_ITEM = 0x77D0,
    DM1_PC34_SW_DROP_ROT_NLO_NEW_LEADER_HAND_ITEM = 0x77E1,
    DM1_PC34_SW_DROP_ROT_NLO_FIRST_CHARGES = 41,
    DM1_PC34_SW_DROP_ROT_NLO_FIRST_QUANTITY = 5,
    DM1_PC34_SW_DROP_ROT_NLO_DROP_CHARGES = 73,
    DM1_PC34_SW_DROP_ROT_NLO_DROP_QUANTITY = 11,
    DM1_PC34_SW_DROP_ROT_NLO_DROP_WEIGHT = 19,
    DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_CHARGES = 29,
    DM1_PC34_SW_DROP_ROT_NLO_NEW_HAND_WEIGHT = 7
};

typedef enum {
    DM1_PC34_SW_DROP_ROT_NLO_STEP_OPEN_NON_LEADER_CHEST = 0,
    DM1_PC34_SW_DROP_ROT_NLO_STEP_QUEUE_DROP_AND_ROTATION = 1,
    DM1_PC34_SW_DROP_ROT_NLO_STEP_F0380_DROP_C540 = 2,
    DM1_PC34_SW_DROP_ROT_NLO_STEP_F0380_ROTATE_LEADER = 3,
    DM1_PC34_SW_DROP_ROT_NLO_STEP_ASSERT_STILL_OPEN = 4
} DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseNegativeAnchor;
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
    int nonLeaderOpenIndex;
    int newLeaderIndex;
    int targetZone;
    int targetSlotBox;
    int targetPc34Slot;
    int targetCommand;
} DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34;

typedef struct {
    int stepTrace[5];
    int stepCount;
    int runtimeRegression;

    int openResult;
    int openChampionBefore;
    int openChestThingBefore;
    int panelContentBefore;
    int leaderBeforeQueue;
    int oldLeaderHandTypeBefore;
    int oldLeaderHandWeightBefore;
    int oldLeaderHandChargesBefore;
    int oldLeaderHandQuantityBefore;
    int newLeaderHandTypeBefore;
    int newLeaderHandWeightBefore;
    int newLeaderHandChargesBefore;

    int visibleTypesBefore[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int visibleChargesBefore[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int visibleQuantitiesBefore[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int c540EmptyBeforeDrop;
    int c537ToC544VisibleBefore;

    int dropQueued;
    int rotationQueued;
    int queuedChampion;
    int queuedOpenChampion;
    int queuedNewLeader;
    int queuedZone;
    int queuedSlotBox;
    int queuedPc34Slot;
    int queuedCommand;
    int commandQueueDepthAfterQueue;

    int f0077Observed;
    int f0078ObservedAfterDrop;
    int mouseUpdateDepthAfterDrop;
    int dropDrainFirst;
    int dropClickResult;
    int commandQueueDepthAfterDrop;
    int openChestThingAfterDrop;
    int panelContentAfterDrop;
    int oldLeaderHandTypeAfterDrop;
    int newLeaderHandTypeAfterDrop;
    int c540TypeAfterDrop;
    int c540WeightAfterDrop;
    int c540ChargesAfterDrop;
    int c540QuantityAfterDrop;
    int c540DropPersistedBeforeRotate;

    int rotationConsumed;
    int commandQueueDepthAfterRotate;
    int leaderAfterRotate;
    int openChampionAfterRotate;
    int newLeaderOpenChestThingAfterRotate;
    int panelContentAfterRotate;
    int oldLeaderHandTypeAfterRotate;
    int newLeaderHandTypeAfterRotate;
    int newLeaderHandWeightAfterRotate;
    int newLeaderHandChargesAfterRotate;
    int oldLeaderHandEmptyAfterRotate;
    int newLeaderHandPreservedAfterRotate;

    int visibleTypesAfterRotate[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int visibleChargesAfterRotate[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int visibleQuantitiesAfterRotate[DM1_PC34_SW_DROP_ROT_NLO_SLOT_COUNT];
    int c540TypeAfterRotate;
    int c540QuantityAfterRotate;
    int c540StillVisibleAfterRotate;
    int c537ToC544ChainCoherentAfterRotate;
    int chestNeverClosed;
    int closeCount;
    int f0077F0078Balanced;

    int noPass768CloseRace;
    int noScrollWheelPickupDrop;
    int noScrollWheelDropOntoOpenChestSlot;
    int noChestDepositDuringLeaderRotation;
    int noPickupDuringResurrectPendingNonLeader;
    int noChestCloseWhileCandidateLiveNonLeader;
    int noChestOpenDuringPending;
    int noChestCloseWithFullLeaderHand;
    int noChestOpenWithFullLeaderHand;

    uint32_t deterministicHash;
} DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34;

const char*
dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_source_evidence_pc34(void);
const DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenSpecPc34*
dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_spec_pc34(void);
int dm1_v1_chest_scroll_wheel_drop_during_rotation_non_leader_open_run_pc34(
    DM1_V1_ChestScrollWheelDropDuringRotationNonLeaderOpenProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_DROP_DURING_ROTATION_NON_LEADER_OPEN_PC34_COMPAT_H */
