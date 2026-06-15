#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_WHILE_PARTY_ROTATE_IN_PROGRESS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_WHILE_PARTY_ROTATE_IN_PROGRESS_PC34_COMPAT_H

/*
 * DM1 V1 chest pickup while party rotate is in progress runtime gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 materializes the open chest into G0425/C537..C544.
 * - CHEST.C F0334:113-132 closes by relinking visible non-empty G0425 slots.
 * - CHAMPION.C F0284:93-131 rotates the party and every champion cell/dir.
 * - CHAMPION.C F0297:243-298 preserves leader-hand thing identity/weight.
 * - CHAMPION.C F0298:270-298 removes the current leader-hand thing.
 * - CHAMPION.C F0300:511-515 clears C30+ slots through G0425.
 * - CHAMPION.C F0301:606-614 writes C30+ slots through G0425.
 * - CHAMPION.C F0302:662-714 routes C537..C544 clicks to C30+ slots.
 * - PANEL.C F0344/F0345/F0352 and COMMAND.C F0359:1985-1990 provide the
 *   panel/command route that feeds the slot click.
 * - DEFS.H:2088 C30, G0425/G0426/G0423/G0305/M070/M516, C537..C544.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_ROTATE_PICKUP_CHAMPION_COUNT = 2,
    DM1_PC34_ROTATE_PICKUP_OLD_LEADER = 0,
    DM1_PC34_ROTATE_PICKUP_NEW_LEADER = 1,
    DM1_PC34_ROTATE_PICKUP_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_ROTATE_PICKUP_PICKED_SLOT_INDEX = 0,
    DM1_PC34_ROTATE_PICKUP_PICKED_PC34_SLOT = DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_ROTATE_PICKUP_PICKED_ZONE = 537,
    DM1_PC34_ROTATE_PICKUP_LAST_ZONE = 544,
    DM1_PC34_ROTATE_PICKUP_SLOT_BOX_C38 = 38,
    DM1_PC34_ROTATE_PICKUP_COMMAND_C040 = 40,
    DM1_PC34_ROTATE_PICKUP_PANEL_M568 = 568,
    DM1_PC34_ROTATE_PICKUP_CHEST_THING = 0x7350,
    DM1_PC34_ROTATE_PICKUP_FIRST_ITEM = 0x7360,
    DM1_PC34_ROTATE_PICKUP_FIRST_CHARGES = 31,
    DM1_PC34_ROTATE_PICKUP_FIRST_QUANTITY = 4
};

typedef enum {
    DM1_PC34_ROTATE_PICKUP_STEP_SETUP_OPEN = 0,
    DM1_PC34_ROTATE_PICKUP_STEP_PARTY_ROTATE_TRIGGER = 1,
    DM1_PC34_ROTATE_PICKUP_STEP_C537_POINTER_QUEUE = 2,
    DM1_PC34_ROTATE_PICKUP_STEP_PARTY_ROTATE_COMMIT = 3,
    DM1_PC34_ROTATE_PICKUP_STEP_CHEST_CLOSE_REWRITE = 4
} DM1_V1_ChestPickupWhilePartyRotateStepPc34;

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
    const char* panelAnchor;
    const char* commandAnchor;
    const char* defsAnchor;
    int oldLeaderIndex;
    int newLeaderIndex;
    int pickedZone;
    int pickedPc34Slot;
    int pickedSlotBox;
    int expectedCloseCount;
} DM1_V1_ChestPickupWhilePartyRotateSpecPc34;

typedef struct {
    int stepTrace[5];
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

    int pointerZone;
    int pointerPanelCommand;
    int pointerSlotBox;
    int pointerPc34Slot;
    int pointerRouteQueued;
    int queueStepBetweenTriggerAndCommit;
    int queuedAgainstChampion;
    int queuedAgainstOpenChestThing;
    int queuedThingType;
    int queuedThingCharges;
    int queuedThingQuantity;
    int queuedThingWeight;
    int queuedThingAllowedSlots;

    int rotateBufferTypeBeforeQueue[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int rotateBufferChargesBeforeQueue[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int rotateBufferQuantityBeforeQueue[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int rotateBufferTypeAfterQueue[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int rotateBufferChargesAfterQueue[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int rotateBufferQuantityAfterQueue[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int rotateBufferPreservedByQueue;

    int newLeaderOpenResult;
    int commitPickupResult;
    int commitLandedAgainstNewLeader;
    int newLeaderOpenChestThingAfterCommit;
    int newLeaderHandTypeAfterCommit;
    int newLeaderHandChargesAfterCommit;
    int newLeaderHandQuantityAfterCommit;
    int newLeaderHandWeightAfterCommit;
    int pickedMetadataPreserved;
    int pickedSlotEmptyAfterCommit;
    int visibleCountAfterCommit;
    int visibleTypesAfterCommit[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int visibleChargesAfterCommit[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int visibleQuantitiesAfterCommit[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];

    int closeCount;
    int closeAgainstChampion;
    int closeAgainstNewLeader;
    int closedTypes[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int closedCharges[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int closedQuantities[DM1_PC34_ROTATE_PICKUP_SLOT_COUNT];
    int closeRewroteVisibleChainAgainstNewLeader;
    int closedPickedThingAbsent;

    int totalPickedCopiesAfterClose;
    uint32_t deterministicHash;
} DM1_V1_ChestPickupWhilePartyRotateProbePc34;

const char*
dm1_v1_chest_pickup_while_party_rotate_in_progress_source_evidence_pc34(void);
const DM1_V1_ChestPickupWhilePartyRotateSpecPc34*
dm1_v1_chest_pickup_while_party_rotate_in_progress_spec_pc34(void);
int dm1_v1_chest_pickup_while_party_rotate_in_progress_run_pc34(
    DM1_V1_ChestPickupWhilePartyRotateProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_PICKUP_WHILE_PARTY_ROTATE_IN_PROGRESS_PC34_COMPAT_H */
