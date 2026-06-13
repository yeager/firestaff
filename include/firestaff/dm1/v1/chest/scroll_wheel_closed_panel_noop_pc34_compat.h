#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_CLOSED_PANEL_NOOP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_CLOSED_PANEL_NOOP_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel click while the inventory panel is already in
 * the closed/inventory state (G0426_T_OpenChest == 0, G0424_i_PanelContent ==
 * DM1_PC34_PANEL_INVENTORY). The scroll-wheel C540 click that COMMAND.C
 * F0359/F0380 queues for the chest-slot must be rejected as a no-op without
 * mutating the leader hand, G0425_aT_ChestSlots, G0426, or the panel content.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-32 returns before the open path when the requested chest
 *   is already G0426; F0333:53-76 only materializes G0425 when the open path
 *   actually fires.
 * - CHEST.C F0334:113-132 close/relink only runs when G0426 is non-zero, so
 *   a closed panel means F0334 returns without rewriting G0425 slots.
 * - CHAMPION.C F0297:243-268 put object in C030 leader hand.
 * - CHAMPION.C F0298:270-298 remove object from C030 leader hand.
 * - CHAMPION.C F0301:606-614 routes C30+ chest slots through G0425 only.
 * - CHAMPION.C F0302:662-714 dispatches C537..C544 slot-box commands only
 *   when G0425 holds a non-empty entry; empty leader hand + empty slot is
 *   rejected at lines 688-695.
 * - COMMAND.C F0359:1452-1662 queues mouse slot commands without draining.
 * - COMMAND.C F0380:2045-2178 drains queued slot-box commands.
 * - IO.C F0077:1113-1122 / F0078:1102-1111 bracket mouse screen updates.
 * - DEFS.H:267 C030, 1876 C38, 2088 C10_COLOR_FLESH, 3906-3913 C537..C544,
 *   5878/5881 G0425/G0426, 3005-3008 M569_PANEL_CHEST, 3909 C540.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_SCROLL_CLOSED_PANEL_CHAMPION_COUNT = 2,
    DM1_PC34_SCROLL_CLOSED_PANEL_LEADER = 0,
    DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER = 1,
    DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_SLOT_INDEX = 2,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_PC34_SLOT = DM1_PC34_SLOT_CHEST_3,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_ZONE = 540,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_SLOT_BOX = 32,
    DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT = 3,
    DM1_PC34_SCROLL_CLOSED_PANEL_SETTLE_TICKS = 32,
    DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_ITEM = 0,
    DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_WEIGHT = 0,
    DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_CHARGES = 0,
    DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_ITEM = 0x76F0,
    DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_WEIGHT = 11,
    DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_CHARGES = 3,
    DM1_PC34_SCROLL_CLOSED_PANEL_REJECT_REASON_NO_G0426 = 0x4750
};

typedef enum {
    DM1_PC34_SCROLL_CLOSED_PANEL_STEP_INIT_CLOSED_STATE = 0,
    DM1_PC34_SCROLL_CLOSED_PANEL_STEP_SNAPSHOT_BEFORE_WHEEL = 1,
    DM1_PC34_SCROLL_CLOSED_PANEL_STEP_QUEUE_WHEEL_CLICK = 2,
    DM1_PC34_SCROLL_CLOSED_PANEL_STEP_DRAIN_WHEEL_CLICK = 3,
    DM1_PC34_SCROLL_CLOSED_PANEL_STEP_REPLAY_WHEEL_TICKS = 4,
    DM1_PC34_SCROLL_CLOSED_PANEL_STEP_SETTLE_INVENTORY = 5
} DM1_V1_ChestScrollWheelClosedPanelNoopStepPc34;

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
    int leaderIndex;
    int nonLeaderIndex;
    int targetSlotIndex;
    int targetZone;
    int targetSlotBox;
    int targetPc34Slot;
    int wheelTickCount;
    int settleTicks;
} DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34;

typedef struct {
    int stepTrace[8];
    int stepCount;
    int runtimeRegression;

    int setupResult;
    int initialPanelContent;
    int initialLeaderG0426;
    int initialNonLeaderG0426;
    int initialLeaderHandType;
    int initialLeaderHandWeight;
    int initialLeaderHandCharges;
    int initialNonLeaderHandType;
    int initialNonLeaderHandWeight;
    int initialG0425Types[DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT];
    int initialG0425Charges[DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT];
    int initialG0425AllZero;
    int initialLoadLeader;
    int initialLoadNonLeader;

    int wheelTicksIssued;
    int wheelTicksRejected;
    int commandQueueDepthAfterIssue;
    int mouseUpdateDepthAfterIssue;
    int f0077Observed;
    int f0078Observed;
    int rejectReasonNoG0426;

    int clickResults[DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT];
    int clickAllRejected;
    int finalClickReturn;

    int panelContentAfterDrain;
    int leaderG0426AfterDrain;
    int nonLeaderG0426AfterDrain;
    int leaderHandTypeAfterDrain;
    int leaderHandWeightAfterDrain;
    int leaderHandChargesAfterDrain;
    int nonLeaderHandTypeAfterDrain;
    int nonLeaderHandWeightAfterDrain;
    int g0425TypesAfterDrain[DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT];
    int g0425ChargesAfterDrain[DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT];
    int g0425AllZeroAfterDrain;
    int loadLeaderAfterDrain;
    int loadNonLeaderAfterDrain;
    int panelStayedInventory;
    int leaderHandStable;
    int g0426StayedZero;
    int g0425ManifestUnchanged;

    int f0077F0078Balanced;
    int mouseUpdateDepthAfterDrain;
    int commandQueueDepthAfterDrain;
    int queueNotMutatedByDrain;
    int mouseNotMutatedByDrain;
    int mouseF0078NotEmitted;

    int panelContentAfterSettle;
    int leaderHandTypeAfterSettle;
    int g0425AllZeroAfterSettle;
    int leaderG0426AfterSettle;
    int stableAcrossSettle;
    int leaderBackpackNotMutated;
    int nonLeaderBackpackStable;

    int noF0333Open;
    int noF0334Close;
    int noF0297Put;
    int noF0298Remove;
    int noF0301SlotWrite;
    int noF0302SlotDispatch;
    int noF0380Drain;
    int noPanelRouteFlip;
    int noC30InLeaderHand;
    int noResurrectPending;
    int noMirrorCandidate;
    int noTeleporterSaveLoad;
    int noDifferentChestOpen;
    int noLeaderRotation;
    int noCapacityEncumbrance;
    int noPartyResize;

    uint32_t deterministicHash;
} DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34;

const char*
dm1_v1_chest_scroll_wheel_closed_panel_noop_source_evidence_pc34(void);
const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34*
dm1_v1_chest_scroll_wheel_closed_panel_noop_spec_pc34(void);
int dm1_v1_chest_scroll_wheel_closed_panel_noop_run_pc34(
    DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_CLOSED_PANEL_NOOP_PC34_COMPAT_H */
