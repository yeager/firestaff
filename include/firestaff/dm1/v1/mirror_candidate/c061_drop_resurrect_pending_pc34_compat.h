#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C061_DROP_RESURRECT_PENDING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C061_DROP_RESURRECT_PENDING_PC34_COMPAT_H

/*
 * DM1 V1 runtime regression gate: a queued C061/C540 open-chest drop drains
 * while a C028 resurrect confirmation is already pending and the M568/C040
 * mirror-candidate panel remains live.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens/materializes G0426 into C537..C544/G0425.
 * - CHEST.C F0334:117-132 closes by rewiring visible non-empty G0425 slots.
 * - CHAMPION.C F0297/F0298:243-298 own the global leader-hand object/load.
 * - CHAMPION.C F0300/F0301:511-614 clear/write C30+ slots through G0425.
 * - CHAMPION.C F0302:677-712 routes C537..C544 slot boxes.
 * - REVIVE.C F0280:63-132 installs the live candidate; F0282:744-806 clears
 *   or commits C160..C162, which must not run for this C061 drop.
 * - PANEL.C F0344/F0345/F0346 keep food/water and resurrect panels separate.
 * - COMMAND.C F0359/F0378/F0380 route panel clicks and queued slot commands.
 * - DEFS.H C028/C030/C040/C061/C540/G0299/G0425/G0426.
 *
 * Synthetic, contract-only, deterministic with no game data and no original-DOS
 * pixel claim.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_MC_C061_RES_PARTY_COUNT_PC34 = 4,
    DM1_V1_MC_C061_RES_SLOT_COUNT_PC34 = 8,
    DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34 = 3,
    DM1_V1_MC_C061_RES_TRACE_COUNT_PC34 = 8,
    DM1_V1_MC_C061_RES_LEADER_PC34 = 0,
    DM1_V1_MC_C061_RES_CANDIDATE_OWNER_PC34 = 2,
    DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34 = 3,
    DM1_V1_MC_C061_RES_TARGET_ZONE_PC34 = 540,
    DM1_V1_MC_C061_RES_TARGET_COMMAND_PC34 = 61,
    DM1_V1_MC_C061_RES_C028_COMMAND_PC34 = 28,
    DM1_V1_MC_C061_RES_C040_GRAPHIC_PC34 = 40,
    DM1_V1_MC_C061_RES_M568_PANEL_PC34 = 568,
    DM1_V1_MC_C061_RES_OPEN_CHEST_THING_PC34 = 0x6D61,
    DM1_V1_MC_C061_RES_LEADER_HAND_THING_PC34 = 0xC061,
    DM1_V1_MC_C061_RES_THING_NONE_PC34 = 0xFFFF,
    DM1_V1_MC_C061_RES_SEED_PC34 = 0xC061C028u
};

typedef enum {
    DM1_V1_MC_C061_RES_STEP_OPEN_CHEST_PC34 = 0,
    DM1_V1_MC_C061_RES_STEP_OPEN_C040_PC34 = 1,
    DM1_V1_MC_C061_RES_STEP_C028_PENDING_PC34 = 2,
    DM1_V1_MC_C061_RES_STEP_CAPTURE_C061_PC34 = 3,
    DM1_V1_MC_C061_RES_STEP_DRAIN_C061_PC34 = 4,
    DM1_V1_MC_C061_RES_STEP_ASSERT_PENDING_STABLE_PC34 = 5,
    DM1_V1_MC_C061_RES_STEP_DRY_RUN_CLOSE_PC34 = 6,
    DM1_V1_MC_C061_RES_STEP_ASSERT_STABLE_PC34 = 7
} DM1_V1_MirrorCandidateC061DropResurrectPendingStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297HandAnchor;
    const char* f0298HandAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* f0280CandidateAnchor;
    const char* f0282PanelAnchor;
    const char* f0344FoodWaterAnchor;
    const char* f0345FoodWaterAnchor;
    const char* f0346ResurrectAnchor;
    const char* f0359ClickAnchor;
    const char* f0378PanelRouteAnchor;
    const char* f0380QueueAnchor;
    const char* defsAnchor;
    const char* disjointness;
    uint32_t deterministicSeed;
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
} DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
    int runtimeRegression;
    uint32_t deterministicSeed;
    uint32_t deterministicHash;
    int stepTrace[DM1_V1_MC_C061_RES_TRACE_COUNT_PC34];
    int stepCount;

    int partyChampionCount;
    int leaderIndex;
    int candidateOwnerIndex;
    int candidateOwnerIsLeader;
    int championCurrentHealth[DM1_V1_MC_C061_RES_PARTY_COUNT_PC34];
    int candidateChainBefore[DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34];
    int candidateChainAfter[DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34];
    int candidateChainStable;
    int g0299CandidateBefore;
    int g0299CandidateAfterDrain;
    int candidateGraphicBefore;
    int candidateGraphicAfterDrain;
    int candidateCommandBefore;
    int candidateCommandAfterDrain;
    int c040PanelBeforeDrain;
    int c040PanelAfterDrain;
    int c040PanelStayedLive;

    int c028ResurrectPendingBefore;
    int c028ResurrectPendingAfterDrain;
    int c028Command;
    int f0280CandidateAddCount;
    int f0282CandidateClearCount;
    int f0282ResurrectCommitCount;
    int f0282CancelCount;
    int resurrectConfirmationStayedPending;

    int openChestOwnerIndex;
    int openChestThingBefore;
    int openChestThingAfterDrain;
    int g0426StayedOpenDuringDrain;
    int f0333OpenCount;
    int f0334CloseCountDuringDrain;
    int f0334DryRunCloseCount;
    int closeTailCountAfterDryRun;
    int closeTailTypesAfterDryRun[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];

    int c061Captured;
    int c061Drained;
    int c061Command;
    int c061Zone;
    int c061SlotBox;
    int c061Pc34Slot;
    int commandQueueDepthAfterCapture;
    int commandQueueDepthAfterDrain;
    int f0359CapturedQueuedDrop;
    int f0378PanelRouteCount;
    int f0380DrainCount;
    int f0302DropDispatchCount;
    int c061DidNotRouteToF0282;
    int c061DidNotClearCandidate;

    int leaderHandTypeBefore;
    int leaderHandWeightBefore;
    int leaderHandChargesBefore;
    int leaderHandTypeAfterDrain;
    int leaderHandClearedByDrop;
    int leaderLoadBefore;
    int leaderLoadAfterDrain;
    int leaderLoadDelta;
    int f0298RemovedLeaderHand;
    int f0300ClearCount;
    int f0301WroteC540;
    int f0297PutSlotInLeaderHandCount;

    int g0425TypesBefore[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    int g0425WeightsBefore[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    int g0425ChargesBefore[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    int g0425TypesAfterDrain[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    int g0425WeightsAfterDrain[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    int g0425ChargesAfterDrain[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    int g0425SlotStableExceptTarget[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    int targetSlotEmptyBefore;
    int targetSlotReceivesLeaderHand;
    uint32_t g0425HashBefore;
    uint32_t g0425HashAfterDrain;
    int g0425HashMutatedOnlyByTargetDrop;

    int f0344FoodWaterDrawCount;
    int f0345FoodWaterPanelCount;
    int f0346C040DrawCount;
    int f0355InventoryToggleCount;
    int f0368SetLeaderCount;
    int saveLoadCount;
    int teleporterCount;
    int partyRotateCount;
    int noC160Close;
    int noC045Accept;
    int noLeaderRotation;
    int disjointFromC061LeaderRotation;
    int disjointFromC061CandidateLive;
    int disjointFromC160CloseRotation;
} DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34;

const char*
dm1_v1_mirror_candidate_c061_drop_resurrect_pending_source_evidence_pc34(void);
const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34*
dm1_v1_mirror_candidate_c061_drop_resurrect_pending_spec_pc34(void);
int dm1_v1_mirror_candidate_c061_drop_resurrect_pending_run_pc34(
    DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif
