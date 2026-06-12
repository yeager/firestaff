#ifndef FIRESTAFF_DM1_V1_CHEST_C040_DROP_DURING_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_C040_DROP_DURING_ROTATION_PC34_COMPAT_H

/*
 * DM1 V1 runtime regression gate: a queued C061/C540 chest-slot drop drains
 * while the visible panel is the live M568/C040 mirror-candidate overlay, then
 * a queued leader rotation drains afterward.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens/materializes G0426 into C537..C544/G0425.
 * - CHEST.C F0334:113-132 would close/relink the visible G0425 chain.
 * - CHAMPION.C F0297/F0298:243-298 own the C030/G4055 leader hand.
 * - CHAMPION.C F0301/F0302:606-714 route C30+ chest slots through G0425.
 * - COMMAND.C F0359:1452-1662 queues mouse commands; F0380:2045-2178
 *   drains queued slot commands, including C061/C540, after panel dispatch.
 * - COMMAND.C F0359/F0380:1985-1990 routes M568/C040 candidate commands only
 *   when the leader hand is empty; this gate keeps the hand full until C061.
 * - REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806 clears it.
 * - PANEL.C F0346/F0347:1619-1657 draws/keeps M568/C040 panel state.
 * - IO.C F0077/F0078:1102-1122 brackets mouse/screen update state.
 * - DEFS.H:338-340 C160..C162, 810-817 C30..C37, 1874-1878 C38,
 *   2200 C040, 3001-3008 M568/M569, 3906-3913 C537..C544,
 *   5694 G0299, 5876-5881 G0423/G0425/G0426.
 *
 * Contract-only, deterministic, no game data and no real-asset pixels.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_C040_DROP_ROT_CHAMPION_COUNT_PC34 = 3,
    DM1_V1_CHEST_C040_DROP_ROT_OLD_LEADER_PC34 = 0,
    DM1_V1_CHEST_C040_DROP_ROT_OPEN_OWNER_PC34 = 1,
    DM1_V1_CHEST_C040_DROP_ROT_NEW_LEADER_PC34 = 2,
    DM1_V1_CHEST_C040_DROP_ROT_CANDIDATE_ORDINAL_PC34 = 3,
    DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34 = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_INDEX_PC34 = 3,
    DM1_V1_CHEST_C040_DROP_ROT_TARGET_ZONE_PC34 = 540,
    DM1_V1_CHEST_C040_DROP_ROT_TARGET_SLOT_BOX_PC34 = 41,
    DM1_V1_CHEST_C040_DROP_ROT_TARGET_PC34_SLOT_PC34 = DM1_PC34_SLOT_CHEST_4,
    DM1_V1_CHEST_C040_DROP_ROT_TARGET_COMMAND_PC34 = 61,
    DM1_V1_CHEST_C040_DROP_ROT_PANEL_C040_PC34 = DM1_PC34_PANEL_RESURRECT_REINCARNATE,
    DM1_V1_CHEST_C040_DROP_ROT_PANEL_CHEST_PC34 = DM1_PC34_PANEL_CHEST,
    DM1_V1_CHEST_C040_DROP_ROT_C040_GRAPHIC_PC34 = 40,
    DM1_V1_CHEST_C040_DROP_ROT_C040_COMMAND_PC34 = 568,
    DM1_V1_CHEST_C040_DROP_ROT_CHEST_COMMAND_PC34 = 569,
    DM1_V1_CHEST_C040_DROP_ROT_DETERMINISTIC_SEED_PC34 = 0xC040C061u,
    DM1_V1_CHEST_C040_DROP_ROT_EXPECTED_RNG_CALLS_PC34 = 37
};

typedef enum {
    DM1_V1_CHEST_C040_DROP_ROT_STEP_OPEN_CHEST_PC34 = 0,
    DM1_V1_CHEST_C040_DROP_ROT_STEP_OPEN_C040_PC34 = 1,
    DM1_V1_CHEST_C040_DROP_ROT_STEP_QUEUE_DROP_ROTATION_PC34 = 2,
    DM1_V1_CHEST_C040_DROP_ROT_STEP_DRAIN_C540_DROP_PC34 = 3,
    DM1_V1_CHEST_C040_DROP_ROT_STEP_DRAIN_ROTATION_PC34 = 4,
    DM1_V1_CHEST_C040_DROP_ROT_STEP_ASSERT_STABLE_PC34 = 5
} DM1_V1_ChestC040DropDuringRotationStepPc34;

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
    const char* f0380C040Anchor;
    const char* f0280CandidateAnchor;
    const char* f0282CandidateAnchor;
    const char* f0346PanelAnchor;
    const char* f0077Anchor;
    const char* f0078Anchor;
    const char* defsAnchor;
    const char* disjointness;
    uint32_t deterministicSeed;
    uint32_t expectedPostResolveSeed;
    int expectedRngCallCount;
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
} DM1_V1_ChestC040DropDuringRotationSpecPc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
    int runtimeRegression;

    uint32_t deterministicSeed;
    uint32_t postResolveSeed;
    int rngCallCount;
    uint32_t deterministicHash;

    int stepTrace[6];
    int stepCount;

    int openOwnerBefore;
    int openChestThingBefore;
    int panelAfterChestOpen;
    int panelAfterC040Open;
    int panelAfterDrop;
    int panelAfterRotate;
    int panelStayedC040;
    int panelHashBeforeDrop;
    int panelHashAfterDrop;
    int panelHashAfterRotate;
    int panelHashStable;
    int c040PanelOpenBefore;
    int c040PanelOpenAfterDrop;
    int c040PanelOpenAfterRotate;
    int c040Graphic;
    int c040Command;
    int candidateOrdinalBefore;
    int candidateOrdinalAfterDrop;
    int candidateOrdinalAfterRotate;
    int candidateStillLiveAfterDrop;
    int candidateStillLiveAfterRotate;
    int c040ClickSuppressedWhileHandFull;
    int f0282ClearCount;

    int leaderBeforeQueue;
    int queuedDrop;
    int queuedRotation;
    int queuedCommand;
    int queuedZone;
    int queuedSlotBox;
    int queuedPc34Slot;
    int commandQueueDepthAfterQueue;
    int dropDrainFirst;
    int commandQueueDepthAfterDrop;
    int commandQueueDepthAfterRotate;

    int oldLeaderHandTypeBefore;
    int oldLeaderHandWeightBefore;
    int oldLeaderHandChargesBefore;
    int oldLeaderHandQuantityBefore;
    int oldLeaderHandTypeAfterDrop;
    int oldLeaderHandTypeAfterRotate;
    int oldLeaderHandEmptyAfterDrop;
    int oldLeaderHandEmptyAfterRotate;
    int newLeaderHandTypeBefore;
    int newLeaderHandTypeAfterRotate;
    int newLeaderHandPreservedAfterRotate;
    int leaderAfterRotate;
    int openOwnerAfterRotate;

    int visibleTypesBefore[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleChargesBefore[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleQuantitiesBefore[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleTypesAfterDrop[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleChargesAfterDrop[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleQuantitiesAfterDrop[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleTypesAfterRotate[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleChargesAfterRotate[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int visibleQuantitiesAfterRotate[DM1_V1_CHEST_C040_DROP_ROT_SLOT_COUNT_PC34];
    int c540EmptyBeforeDrop;
    int c540TypeAfterDrop;
    int c540WeightAfterDrop;
    int c540ChargesAfterDrop;
    int c540QuantityAfterDrop;
    int c540TypeAfterRotate;
    int c540QuantityAfterRotate;
    int c540StillVisibleAfterRotate;
    int chestSlotChainCoherentBefore;
    int chestSlotChainCoherentAfterDrop;
    int chestSlotChainCoherentAfterRotate;
    int chestNeverClosed;
    int closeCount;
    int openChestThingAfterDrop;
    int openChestThingAfterRotate;

    int f0077Observed;
    int f0078Observed;
    int f0077F0078Balanced;
    int mouseUpdateDepthAfterDrop;

    int noPass771PlainDropDuringRotation;
    int noMirrorCandidateC040LiveC545Drop;
    int noMirrorCandidateC040RedrawAfterChestClose;
    int noChestCloseWhileCandidateLive;
    int noChestScrollWheelCloseRace;
    int noChestResurrectRotationScrollWheel;
    int noMirrorCandidateC545AcceptDuringRotation;
    int noMirrorCandidatePanelRedrawAfterInventoryExit;
} DM1_V1_ChestC040DropDuringRotationProbePc34;

const char*
dm1_v1_chest_c040_drop_during_rotation_source_evidence_pc34(void);
const DM1_V1_ChestC040DropDuringRotationSpecPc34*
dm1_v1_chest_c040_drop_during_rotation_spec_pc34(void);
int dm1_v1_chest_c040_drop_during_rotation_run_pc34(
    DM1_V1_ChestC040DropDuringRotationProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_C040_DROP_DURING_ROTATION_PC34_COMPAT_H */
