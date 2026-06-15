#ifndef FIRESTAFF_DM1_V1_CHEST_CLOSE_WHILE_PARTY_ROTATE_PICKUP_PENDING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_CLOSE_WHILE_PARTY_ROTATE_PICKUP_PENDING_PC34_COMPAT_H

/*
 * DM1 V1 chest close while party rotation and C537 pickup are pending.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-76 materializes G0426 into G0425/C537..C544.
 * - CHEST.C F0334:113-132 clears G0426 and relinks non-empty G0425 slots.
 * - CHAMPION.C F0284:93-131 rotates party direction/cell state.
 * - CHAMPION.C F0297:243-268/F0298:270-298 own leader-hand object state.
 * - CHAMPION.C F0300:511-515 clears C30+ slots through G0425.
 * - CHAMPION.C F0301:606-614 writes C30+ slots through G0425.
 * - CHAMPION.C F0302:662-714 routes C537..C544 slot-box clicks.
 * - COMMAND.C F0359:1973-1983 and F0380:2045-2152 feed panel/pending
 *   click work around queued turn commands.
 * - PANEL.C:7-13 owns G0423/G0425/G0426 panel globals.
 * - DEFS.H:810,1878,3001-3008,3906-3913,5876-5881 defines C30, M070,
 *   M568/M569, C537..C544, G0423/G0425/G0426.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CLOSE_ROTATE_PICKUP_CHAMPION_COUNT = 2,
    DM1_PC34_CLOSE_ROTATE_PICKUP_OLD_LEADER = 0,
    DM1_PC34_CLOSE_ROTATE_PICKUP_NEW_LEADER = 1,
    DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_SLOT_INDEX = 0,
    DM1_PC34_CLOSE_ROTATE_PICKUP_PICKED_ZONE = 537,
    DM1_PC34_CLOSE_ROTATE_PICKUP_LAST_ZONE = 544,
    DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_BOX_C38 = 38,
    DM1_PC34_CLOSE_ROTATE_PICKUP_PANEL_M569 = 569,
    DM1_PC34_CLOSE_ROTATE_PICKUP_RESURRECT_PANEL_M568 = 568,
    DM1_PC34_CLOSE_ROTATE_PICKUP_COMMAND_TURN_RIGHT = 2,
    DM1_PC34_CLOSE_ROTATE_PICKUP_COMMAND_C040 = 40,
    DM1_PC34_CLOSE_ROTATE_PICKUP_CHEST_THING = 0x7370,
    DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_ITEM = 0x7380,
    DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_CHARGES = 41,
    DM1_PC34_CLOSE_ROTATE_PICKUP_FIRST_QUANTITY = 5
};

typedef enum {
    DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_OPEN = 0,
    DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_ROTATE_TRIGGER = 1,
    DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_QUEUE_C537 = 2,
    DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_BEGIN_PICKUP = 3,
    DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_CLOSE_G0426 = 4,
    DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_ROTATE_COMMIT = 5,
    DM1_PC34_CLOSE_ROTATE_PICKUP_STEP_STALE_PICKUP_REJECT = 6
} DM1_V1_ChestCloseWhilePartyRotatePickupStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0284RotateAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* commandF0359Anchor;
    const char* commandF0380Anchor;
    const char* panelAnchor;
    const char* defsAnchor;
    int expectedCloseCount;
} DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34;

typedef struct {
    int stepTrace[7];
    int stepCount;
    int sourceLockedContractOnly;

    int openResult;
    int openChestThingBeforeRotate;
    int leaderBeforeRotate;
    int leaderAfterTrigger;
    int leaderAfterCommit;
    int partyDirectionBefore;
    int partyDirectionAfterTrigger;
    int partyDirectionAfterCommit;
    int oldLeaderDirectionAfterCommit;
    int newLeaderDirectionAfterCommit;

    int queuedCommand;
    int queuedPanel;
    int queuedZone;
    int queuedSlotBox;
    int queuedPc34Slot;
    int queuedAgainstChampion;
    int queuedAgainstOpenChestThing;
    int queuedThingType;
    int queuedThingCharges;
    int queuedThingQuantity;
    int queuedThingWeight;
    int queuedThingAllowedSlots;

    int pickupBeginResult;
    int pickedSlotEmptyBeforeClose;
    int leaderHandTypeBeforeClose;
    int leaderHandChargesBeforeClose;
    int leaderHandQuantityBeforeClose;
    int leaderHandWeightBeforeClose;

    int closeCount;
    int openChestThingAfterClose;
    int closedTypes[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    int closedCharges[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    int closedQuantities[DM1_PC34_CLOSE_ROTATE_PICKUP_SLOT_COUNT];
    int closeSkippedPickedSlot;
    int closeCompactedTail;

    int stalePickupResultAfterClose;
    int newLeaderOpenChestThingAfterCommit;
    int latePickupRejectedAgainstClosedG0426;
    int pickedCopiesInClosedChain;
    int pickedCopiesIncludingHand;
    int totalVisibleAfterClose;
    uint32_t deterministicHash;
} DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34;

const char*
dm1_v1_chest_close_while_party_rotate_pickup_pending_source_evidence_pc34(void);
const DM1_V1_ChestCloseWhilePartyRotatePickupSpecPc34*
dm1_v1_chest_close_while_party_rotate_pickup_pending_spec_pc34(void);
int dm1_v1_chest_close_while_party_rotate_pickup_pending_run_pc34(
    DM1_V1_ChestCloseWhilePartyRotatePickupProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
