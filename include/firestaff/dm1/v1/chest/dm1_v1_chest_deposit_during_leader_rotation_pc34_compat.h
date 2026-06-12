#ifndef FIRESTAFF_DM1_V1_CHEST_DEPOSIT_DURING_LEADER_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_DEPOSIT_DURING_LEADER_ROTATION_PC34_COMPAT_H

/*
 * DM1 V1 chest deposit while leader rotation is in flight runtime gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 materializes the open chest into G0425/C537..C544.
 * - CHEST.C F0334:113-132 closes by relinking visible non-empty G0425 slots.
 * - CHAMPION.C F0297:243-298 puts a thing in the global leader hand (C030).
 * - CHAMPION.C F0298:270-298 removes the global leader-hand thing.
 * - CHAMPION.C F0300:511-515 clears C30+ slots through G0425.
 * - CHAMPION.C F0301:606-614 writes C30+ slots through G0425.
 * - CHAMPION.C F0302:662-714 routes occupied C537..C544 slot clicks.
 * - COMMAND.C F0359:1985-1990 consumes the command queue / rotation route.
 * - PANEL.C F0344:1493-1561, F0345:1563-1616, and F0354:2299-2352 redraw
 *   the affected panel/champion state.
 * - DEFS.H C030, C038, C040/C045, C537..C544, G0425/G0426, and C540.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_DEPOSIT_ROTATE_CHAMPION_COUNT = 2,
    DM1_PC34_DEPOSIT_ROTATE_OLD_LEADER = 0,
    DM1_PC34_DEPOSIT_ROTATE_NEW_LEADER = 1,
    DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT = 5,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_INDEX = 5,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_ZONE = 542,
    DM1_PC34_DEPOSIT_ROTATE_FIRST_ZONE = 537,
    DM1_PC34_DEPOSIT_ROTATE_LAST_ZONE = 544,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_PC34_SLOT = DM1_PC34_SLOT_CHEST_6,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_SLOT_BOX = 43,
    DM1_PC34_DEPOSIT_ROTATE_CHEST_THING = 0x7670,
    DM1_PC34_DEPOSIT_ROTATE_FIRST_STABLE_ITEM = 0x7680,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_ITEM = 0x76A5,
    DM1_PC34_DEPOSIT_ROTATE_FIRST_CHARGES = 21,
    DM1_PC34_DEPOSIT_ROTATE_FIRST_QUANTITY = 3,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_CHARGES = 62,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_QUANTITY = 9,
    DM1_PC34_DEPOSIT_ROTATE_TARGET_WEIGHT = 17,
    DM1_PC34_DEPOSIT_ROTATE_PANEL_CHEST = 6,
    DM1_PC34_DEPOSIT_ROTATE_PANEL_RESURRECT = 7,
    DM1_PC34_DEPOSIT_ROTATE_C040_OFF = 0
};

typedef enum {
    DM1_PC34_DEPOSIT_ROTATE_STEP_SETUP_OPEN = 0,
    DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_QUEUED = 1,
    DM1_PC34_DEPOSIT_ROTATE_STEP_C542_DEPOSIT = 2,
    DM1_PC34_DEPOSIT_ROTATE_STEP_ROTATION_CONSUMED = 3,
    DM1_PC34_DEPOSIT_ROTATE_STEP_CLOSE_REWRITE = 4
} DM1_V1_ChestDepositDuringLeaderRotationStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* commandAnchor;
    const char* panelAnchor;
    const char* defsAnchor;
    const char* nonDuplicateMarker;
    int oldLeaderIndex;
    int newLeaderIndex;
    int targetZone;
    int targetPc34Slot;
    int targetSlotBox;
    int stablePrefixCount;
    int resurrectPendingExpected;
} DM1_V1_ChestDepositDuringLeaderRotationSpecPc34;

typedef struct {
    int stepTrace[5];
    int stepCount;
    int sourceLockedContractOnly;

    int openResult;
    int openChestThingBefore;
    int leaderBefore;
    int oldLeaderHandTypeBefore;
    int newLeaderHandTypeBefore;
    int c040ResurrectPendingBefore;
    int panelContentBefore;

    int rotationQueued;
    int queuedChampion;
    int queuedNewLeader;
    int leaderAfterQueue;
    int rotationConsumed;
    int leaderAfterConsume;
    int depositSameTickAsRotation;

    int pointerZone;
    int pointerSlotBox;
    int pointerPc34Slot;
    int depositClickResult;
    int depositAgainstOldLeader;
    int depositAgainstOpenChest;
    int oldLeaderHandTypeAfterDeposit;
    int oldLeaderHandChargesAfterDeposit;
    int oldLeaderHandQuantityAfterDeposit;
    int oldLeaderHandWeightAfterDeposit;

    int stableTypesBefore[DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT];
    int stableChargesBefore[DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT];
    int stableQuantitiesBefore[DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT];
    int stableTypesAfterDeposit[DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT];
    int stableChargesAfterDeposit[DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT];
    int stableQuantitiesAfterDeposit[DM1_PC34_DEPOSIT_ROTATE_STABLE_PREFIX_COUNT];
    int stablePrefixPreservedAfterDeposit;

    int targetTypeBefore;
    int targetChargesBefore;
    int targetQuantityBefore;
    int targetWeightBefore;
    int targetTypeAfterDeposit;
    int targetRemovedFromC542;

    int newLeaderOpenChestThingAfterConsume;
    int newLeaderHandTypeAfterConsume;
    int newLeaderHandChargesAfterConsume;
    int newLeaderHandQuantityAfterConsume;
    int newLeaderHandWeightAfterConsume;
    int newLeaderInheritedDeposit;
    int oldLeaderHandTypeAfterConsume;
    int noResurrectPendingAfterConsume;

    int visibleTypesAfterConsume[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int visibleChargesAfterConsume[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int visibleQuantitiesAfterConsume[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int c537ToC541RemainInG0425;
    int c542EmptyAfterConsume;

    int closeCount;
    int closeAgainstNewLeader;
    int closedTypes[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int closedCharges[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int closedQuantities[DM1_PC34_DEPOSIT_ROTATE_SLOT_COUNT];
    int closedStablePrefixPreserved;
    int closedTargetAbsent;
    int totalTargetCopiesAfterClose;

    uint32_t deterministicHash;
} DM1_V1_ChestDepositDuringLeaderRotationProbePc34;

const char*
dm1_v1_chest_deposit_during_leader_rotation_source_evidence_pc34(void);
const DM1_V1_ChestDepositDuringLeaderRotationSpecPc34*
dm1_v1_chest_deposit_during_leader_rotation_spec_pc34(void);
int dm1_v1_chest_deposit_during_leader_rotation_run_pc34(
    DM1_V1_ChestDepositDuringLeaderRotationProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_DEPOSIT_DURING_LEADER_ROTATION_PC34_COMPAT_H */
