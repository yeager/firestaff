#ifndef FIRESTAFF_DM1_V1_CHEST_C061_DROP_WHILE_LEADER_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_C061_DROP_WHILE_LEADER_ROTATION_PC34_COMPAT_H

/*
 * DM1 V1 runtime regression gate: a C061/C540 chest-slot click is captured
 * while leader rotation is already in flight, with a separate non-leader
 * champion owning the live open chest.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens/materializes G0426 into C537..C544/G0425.
 * - CHEST.C F0334:117-132 closes by rewiring visible non-empty G0425 slots.
 * - CHAMPION.C F0297/F0298:243-298 own the global leader-hand object/load.
 * - CHAMPION.C F0300/F0301:511-614 clear/write C30+ slots through G0425.
 * - CHAMPION.C F0302:677-712 routes C537..C544 slot boxes and hand swaps.
 * - COMMAND.C F0359:1985-1990 keeps panel dispatch separate; F0380:2045-2184
 *   drains queue entries including C061 and rotation.
 * - OBJECT.C F0032/F0033:121-176 resolve object type/icon for slot masks.
 * - DUNGEON.C F0163:1769-1795 is the close-time list append tail.
 * - IO.C F0077/F0078:1102-1122 bracket mouse/screen update suppression.
 * - DEFS.H C30/C061/C540/C037/C038/C039/C537..C544/G0425/G0426/M070/M516.
 *
 * Contract-only, deterministic, no game data and no real-asset pixels.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34 = 0,
    DM1_V1_CHEST_C061_DROP_ROT_NEW_LEADER_PC34 = 1,
    DM1_V1_CHEST_C061_DROP_ROT_OPEN_OWNER_PC34 = 2,
    DM1_V1_CHEST_C061_DROP_ROT_IDLE_CHAMPION_PC34 = 3,
    DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34 = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_INDEX_PC34 = 3,
    DM1_V1_CHEST_C061_DROP_ROT_ACTION_HAND_PC34 = DM1_PC34_SLOT_ACTION_HAND,
    DM1_V1_CHEST_C061_DROP_ROT_TARGET_ZONE_PC34 = 540,
    DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_BOX_PC34 = 41,
    DM1_V1_CHEST_C061_DROP_ROT_TARGET_PC34_SLOT_PC34 = DM1_PC34_SLOT_CHEST_4,
    DM1_V1_CHEST_C061_DROP_ROT_TARGET_COMMAND_PC34 = 61,
    DM1_V1_CHEST_C061_DROP_ROT_PANEL_CHEST_PC34 = DM1_PC34_PANEL_CHEST,
    DM1_V1_CHEST_C061_DROP_ROT_CHEST_THING_PC34 = 0x6A61,
    DM1_V1_CHEST_C061_DROP_ROT_LEADER_ACTION_ITEM_PC34 = 0x6101,
    DM1_V1_CHEST_C061_DROP_ROT_LEADER_HAND_ITEM_PC34 = 0xC061,
    DM1_V1_CHEST_C061_DROP_ROT_DETERMINISTIC_SEED_PC34 = 0xC061F302u
};

typedef enum {
    DM1_V1_CHEST_C061_DROP_ROT_STEP_OPEN_CHEST_PC34 = 0,
    DM1_V1_CHEST_C061_DROP_ROT_STEP_QUEUE_ROTATION_PC34 = 1,
    DM1_V1_CHEST_C061_DROP_ROT_STEP_CAPTURE_C061_PC34 = 2,
    DM1_V1_CHEST_C061_DROP_ROT_STEP_DRAIN_ROTATION_PC34 = 3,
    DM1_V1_CHEST_C061_DROP_ROT_STEP_ASSERT_STABLE_PC34 = 4
} DM1_V1_ChestC061DropWhileLeaderRotationStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297HandAnchor;
    const char* f0298HandAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* f0359PanelAnchor;
    const char* f0380QueueAnchor;
    const char* f0032ObjectAnchor;
    const char* f0033ObjectAnchor;
    const char* f0163AppendAnchor;
    const char* f0077Anchor;
    const char* f0078Anchor;
    const char* defsAnchor;
    const char* disjointness;
    uint32_t deterministicSeed;
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
} DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
    int runtimeRegression;

    uint32_t deterministicSeed;
    uint32_t deterministicHash;
    int stepTrace[5];
    int stepCount;

    int leaderBeforeQueue;
    int leaderAfterRotation;
    int openOwnerBefore;
    int openOwnerAfterRotation;
    int openChestThingBefore;
    int openChestThingAfterRotation;
    int g0426ByteHashBefore;
    int g0426ByteHashAfterRotation;
    int g0426ByteStableAcrossRotation;
    int closeCountDuringRotation;
    int f0334CloseSuppressed;

    int panelBeforeRace;
    int panelAfterCapture;
    int panelAfterRotation;
    int panelRepaintChampionDuringRace;
    int leaderPanelRepaintedDuringRace;
    int newLeaderPanelRepaintedDuringRace;
    int openOwnerPanelRepaintedDuringRace;

    int rotationQueued;
    int c061CapturedWhileRotationQueued;
    int commandQueueDepthAfterCapture;
    int commandQueueDepthAfterRotation;
    int pendingC061AfterRotation;
    int c061AppliedDuringRotation;
    int c061EndsInLeaderActionHand;
    int c061EndsInPendingQueue;
    int c061Command;
    int c061Zone;
    int c061SlotBox;
    int c061Pc34Slot;
    int c061MouseRouteAccepted;

    int leaderHandTypeBefore;
    int leaderHandWeightBefore;
    int leaderHandChargesBefore;
    int leaderHandTypeAfterRotation;
    int leaderHandByteStableAcrossRotation;
    int leaderActionHandTypeBefore;
    int leaderActionHandWeightBefore;
    int leaderActionHandChargesBefore;
    int leaderActionHandTypeAfterRotation;
    int leaderActionHandWeightAfterRotation;
    int leaderActionHandChargesAfterRotation;
    int leaderActionHandByteStableAcrossRotation;
    int leaderActionHandSlotPc34;

    int g0425TypesBefore[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425WeightsBefore[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425ChargesBefore[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425QuantitiesBefore[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425TypesAfterRotation[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425WeightsAfterRotation[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425ChargesAfterRotation[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425QuantitiesAfterRotation[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425SlotByteStable[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int g0425ByteHashBefore;
    int g0425ByteHashAfterRotation;
    int g0425ByteStableAcrossRotation;
    int targetSlotEmptyBefore;
    int targetSlotEmptyAfterRotation;
    int targetSlotNotMutatedByPendingC061;

    int loadBefore[DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34];
    int loadAfterRotation[DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34];
    int loadDelta[DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34];
    int loadByteStable[DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34];
    int allLoadsByteStableAcrossRotation;

    int f0077Observed;
    int f0078Observed;
    int mouseUpdateDepthAfterCapture;
    int mouseUpdateBalanced;
    int objectMaskCheckedByF0032F0033;
    int f0163AppendNotReached;

    int noPass786C040MirrorCandidateDrain;
    int noPass771ScrollWheelDropDuringRotation;
    int noChestCloseWhilePartyRotatePickupPending;
    int noMirrorCandidateC160CloseRotation;
    int noChampionPanelHudFoodWaterRecompute;
} DM1_V1_ChestC061DropWhileLeaderRotationProbePc34;

const char*
dm1_v1_chest_c061_drop_while_leader_rotation_source_evidence_pc34(void);
const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34*
dm1_v1_chest_c061_drop_while_leader_rotation_spec_pc34(void);
int dm1_v1_chest_c061_drop_while_leader_rotation_run_pc34(
    DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_C061_DROP_WHILE_LEADER_ROTATION_PC34_COMPAT_H */
