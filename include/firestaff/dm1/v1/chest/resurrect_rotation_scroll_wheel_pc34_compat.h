#ifndef FIRESTAFF_DM1_V1_CHEST_RESURRECT_ROTATION_SCROLL_WHEEL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_RESURRECT_ROTATION_SCROLL_WHEEL_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel regression for a non-leader open G0426 chest while
 * the C028 resurrect-confirm panel close and a leader rotation are queued.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens G0426 and materializes C537..C544 in G0425.
 * - CHEST.C F0334:113-132 closes G0426 and relinks the C537..C544 chain.
 * - CHAMPION.C F0297/F0298:243-298 own the C030 leader hand.
 * - CHAMPION.C F0301/F0302:606-714 route C30+ chest slots through G0425.
 * - COMMAND.C F0359:1452-1662 queues mouse slot commands.
 * - COMMAND.C F0361:1709-1813 writes keyboard/wheel-like queued commands.
 * - COMMAND.C F0380:2045-2178 drains queued turn and C028..C065 commands.
 * - IO.C F0077/F0078:1102-1122 brackets mouse/screen update state.
 * - REVIVE.C F0280/F0282 and PANEL.C F0349/F0350/F0351 own the C028/C029
 *   resurrect-confirm panel route.
 * - DEFS.H:267 C030, 790 C10_SLOT_NECK, 2088 C10_COLOR_FLESH, 810 C30,
 *   1876 C38, 3906-3913 C537..C544, 5878 G0425, and 5881 G0426.
 *
 * Non-duplicative marker: pass775 targets the C028 resurrect-confirm close
 * route plus a queued leader rotation before allowing the C540 scroll-wheel
 * swap. It is not pass768 stale same-champion close, pass771 non-leader-open
 * drop-during-rotation, pass772 C045 food/water accept, or the C040 panel
 * priority click/rotation slices.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CRR_SW_CHAMPION_COUNT = 2,
    DM1_PC34_CRR_SW_OLD_LEADER = 0,
    DM1_PC34_CRR_SW_NON_LEADER_OPEN = 1,
    DM1_PC34_CRR_SW_NEW_LEADER = 1,
    DM1_PC34_CRR_SW_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CRR_SW_TARGET_SLOT_INDEX = 3,
    DM1_PC34_CRR_SW_TARGET_ZONE = 540,
    DM1_PC34_CRR_SW_TARGET_SLOT_BOX = 41,
    DM1_PC34_CRR_SW_TARGET_PC34_SLOT = DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_CRR_SW_TARGET_COMMAND = 61,
    DM1_PC34_CRR_SW_C028_ROUTE = 28,
    DM1_PC34_CRR_SW_C029_ROUTE = 29,
    DM1_PC34_CRR_SW_C160_RESURRECT = 160,
    DM1_PC34_CRR_SW_C162_CANCEL_CLOSE = 162,
    DM1_PC34_CRR_SW_CHEST_THING = 0x7750,
    DM1_PC34_CRR_SW_FIRST_STABLE_ITEM = 0x7760,
    DM1_PC34_CRR_SW_TARGET_SLOT_ITEM = 0x77A3,
    DM1_PC34_CRR_SW_HAND_ITEM = 0x77D5,
    DM1_PC34_CRR_SW_FIRST_CHARGES = 51,
    DM1_PC34_CRR_SW_FIRST_QUANTITY = 7,
    DM1_PC34_CRR_SW_TARGET_SLOT_CHARGES = 91,
    DM1_PC34_CRR_SW_TARGET_SLOT_QUANTITY = 13,
    DM1_PC34_CRR_SW_HAND_CHARGES = 37,
    DM1_PC34_CRR_SW_HAND_QUANTITY = 5,
    DM1_PC34_CRR_SW_HAND_WEIGHT = 17
};

typedef enum {
    DM1_PC34_CRR_SW_STEP_OPEN_NON_LEADER_CHEST = 0,
    DM1_PC34_CRR_SW_STEP_OPEN_C028_PANEL = 1,
    DM1_PC34_CRR_SW_STEP_QUEUE_C028_CLOSE_AND_ROTATION = 2,
    DM1_PC34_CRR_SW_STEP_REJECT_WHEEL_C028_LIVE = 3,
    DM1_PC34_CRR_SW_STEP_DRAIN_C028_CLOSE = 4,
    DM1_PC34_CRR_SW_STEP_REJECT_WHEEL_ROTATION_QUEUED = 5,
    DM1_PC34_CRR_SW_STEP_DRAIN_ROTATION = 6,
    DM1_PC34_CRR_SW_STEP_ACCEPT_WHEEL_C540_SWAP = 7
} DM1_V1_ChestResurrectRotationScrollWheelStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297HandAnchor;
    const char* f0298HandAnchor;
    const char* f0301SlotAnchor;
    const char* f0302DispatchAnchor;
    const char* f0359QueueAnchor;
    const char* f0361WheelQueueAnchor;
    const char* f0380DrainAnchor;
    const char* f0077EnableAnchor;
    const char* f0078DisableAnchor;
    const char* reviveAnchor;
    const char* panelAnchor;
    const char* defsAnchor;
    const char* nonDuplicateMarker;
    int oldLeaderIndex;
    int nonLeaderOpenIndex;
    int newLeaderIndex;
    int c028PanelRoute;
    int c029PanelRoute;
    int targetZone;
    int targetSlotBox;
    int targetPc34Slot;
    int targetCommand;
} DM1_V1_ChestResurrectRotationScrollWheelSpecPc34;

typedef struct {
    int stepTrace[8];
    int stepCount;
    int runtimeRegression;

    int openResult;
    int openChampionBefore;
    int openChestThingBefore;
    int panelChestBeforeC028;
    int c028PanelLiveBeforeQueue;
    int c028PanelRoute;
    int c029PanelRoute;
    int candidateOrdinalBeforeQueue;
    int candidateCommandBeforeQueue;
    int leaderBeforeQueue;
    int handTypeBeforeQueue;
    int handChargesBeforeQueue;
    int handQuantityBeforeQueue;
    int targetSlotTypeBeforeQueue;
    int targetSlotChargesBeforeQueue;
    int targetSlotQuantityBeforeQueue;
    int visibleTypesBefore[DM1_PC34_CRR_SW_SLOT_COUNT];
    int visibleChargesBefore[DM1_PC34_CRR_SW_SLOT_COUNT];
    int visibleQuantitiesBefore[DM1_PC34_CRR_SW_SLOT_COUNT];
    int c537ToC544VisibleBefore;

    int c028CloseQueued;
    int rotationQueued;
    int queuedOldLeader;
    int queuedNewLeader;
    int queuedOpenChampion;
    int commandQueueDepthAfterQueue;
    int queuedZone;
    int queuedSlotBox;
    int queuedPc34Slot;
    int queuedCommand;

    int c028LiveRejectAttempted;
    int c028LiveRejectResult;
    int c028LiveRejectReason;
    int f0077ObservedAfterC028Reject;
    int f0078ObservedAfterC028Reject;
    int mouseDepthAfterC028Reject;
    int commandQueueDepthAfterC028Reject;
    int openChestThingAfterC028Reject;
    int panelAfterC028Reject;
    int targetSlotTypeAfterC028Reject;
    int handTypeAfterC028Reject;
    int chainStableAfterC028Reject;
    int g0426StableAfterC028Reject;

    int c028CloseDrained;
    int c028PanelLiveAfterClose;
    int candidateOrdinalAfterClose;
    int panelAfterC028Close;
    int commandQueueDepthAfterC028Close;
    int rotationStillQueuedAfterC028Close;

    int rotationQueuedRejectAttempted;
    int rotationQueuedRejectResult;
    int rotationQueuedRejectReason;
    int commandQueueDepthAfterRotationReject;
    int openChestThingAfterRotationReject;
    int targetSlotTypeAfterRotationReject;
    int handTypeAfterRotationReject;
    int chainStableAfterRotationReject;
    int g0426StableAfterRotationReject;

    int rotationDrained;
    int leaderAfterRotationDrain;
    int openChampionAfterRotationDrain;
    int commandQueueDepthAfterRotationDrain;
    int handTypeAfterRotationDrain;
    int panelAfterRotationDrain;
    int openChestThingAfterRotationDrain;

    int wheelAcceptedAfterCloseAndRotation;
    int f0302DispatchCountAfterAccept;
    int commandQueueDepthAfterAccept;
    int targetSlotTypeAfterAccept;
    int targetSlotChargesAfterAccept;
    int targetSlotQuantityAfterAccept;
    int handTypeAfterAccept;
    int handChargesAfterAccept;
    int handQuantityAfterAccept;
    int openChestThingAfterAccept;
    int c537ToC544ChainCoherentAfterAccept;
    int c028CloseThenRotationThenSwap;
    int f0077F0078Balanced;

    int visibleTypesAfterAccept[DM1_PC34_CRR_SW_SLOT_COUNT];
    int visibleChargesAfterAccept[DM1_PC34_CRR_SW_SLOT_COUNT];
    int visibleQuantitiesAfterAccept[DM1_PC34_CRR_SW_SLOT_COUNT];

    int noPass768CloseRace;
    int noPass771DropDuringRotation;
    int noPass772FoodWaterAccept;
    int noC040PanelPriorityRotationClick;
    int noDropDuringRotationNonLeaderOpen;

    uint32_t deterministicHash;
} DM1_V1_ChestResurrectRotationScrollWheelProbePc34;

const char*
dm1_v1_chest_resurrect_rotation_scroll_wheel_source_evidence_pc34(void);
const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34*
dm1_v1_chest_resurrect_rotation_scroll_wheel_spec_pc34(void);
int dm1_v1_chest_resurrect_rotation_scroll_wheel_run_pc34(
    DM1_V1_ChestResurrectRotationScrollWheelProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_RESURRECT_ROTATION_SCROLL_WHEEL_PC34_COMPAT_H */
