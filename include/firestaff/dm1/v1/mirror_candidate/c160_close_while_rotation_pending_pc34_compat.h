#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C160_CLOSE_WHILE_ROTATION_PENDING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C160_CLOSE_WHILE_ROTATION_PENDING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_C160_ROTATION_PARTY_COUNT_PC34 4
#define DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_MC_C160_ROTATION_TRACE_COUNT_PC34 9

typedef struct {
    const char *panelFoodWaterAnchor;
    const char *panelResurrectAnchor;
    const char *toggleCandidateGateAnchor;
    const char *chestAnchor;
    const char *championRotationAnchor;
    const char *commandClickAnchor;
    const char *commandPanelRouteAnchor;
    const char *commandQueueAnchor;
    const char *clickChampionAnchor;
    const char *defsAnchor;
    const char *closeLandingAnchor;
    const char *nonOverlap;
} Dm1V1MirrorCandidateC160CloseWhileRotationPendingEvidencePc34;

typedef struct {
    int championOrdinal;
    int alive;
    int leader;
    int rotationPending;
    int c040CandidateOwner;
} Dm1V1MirrorCandidateC160CloseWhileRotationPendingChampionPc34;

typedef struct {
    int contractOnly;
    int noGameDataRequired;
    uint32_t seed;
    int partyChampionCount;
    int leaderIndex;
    int pendingLeaderIndex;
    int inventoryChampionOrdinal;
    int g0299CandidateOrdinal;
    int candidateOwnerIndex;
    int candidateChainIndex;
    int c040PanelLive;
    int panelContent;
    int panelGraphic;
    int leaderHandEmpty;
    int f0302RotationInFlight;
    int f0302RotationCommitted;
    int c160CloseClickDispatched;
    int commandQueueDepth;
    int openChestThing;
    int g0426OpenChest;
    int chestSlots[DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34];
    int trace[DM1_V1_MC_C160_ROTATION_TRACE_COUNT_PC34];
    int f0282C160ClearCount;
    int f0282NonC160ClearCount;
    int f0302EnterCount;
    int f0302LeaderCommitCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0345FoodWaterDrawCount;
    int f0346C040DrawCount;
    int f0347PanelDrawCount;
    int f0355ToggleSuppressedByCandidateCount;
    int f0359FreshClickCount;
    int f0360PendingReplayCount;
    int f0367StatusBoxClickCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380DrainCount;
    int f0098FloorCeilingDrawCount;
    int f0326MousePointerRefreshCount;
    int saveLoadCount;
    int teleporterCount;
    int partyRotateCount;
    int c040PanelRerenderDuringCloseCount;
    uint32_t c040PanelPixelHashBefore;
    uint32_t c040PanelPixelHashAfterClose;
    uint32_t chestHashBefore;
    uint32_t chestHashAfter;
    uint32_t beforeHash;
    uint32_t afterF0302PendingHash;
    uint32_t afterCloseHash;
    uint32_t afterRotationCommitHash;
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingChampionPc34
        champions[DM1_V1_MC_C160_ROTATION_PARTY_COUNT_PC34];
} Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34;

typedef struct {
    int accepted;
    int contractOnly;
    int noGameDataRequired;
    int initialLeaderIndex;
    int pendingLeaderIndexBeforeClose;
    int leaderIndexDuringClose;
    int finalLeaderIndex;
    int pendingLeaderIndexAfterClose;
    int pendingLeaderIndexAfterCommit;
    int g0299BeforeClose;
    int g0299AfterClose;
    int candidateOwnerBeforeClose;
    int candidateOwnerAfterClose;
    int c040PanelLiveBeforeClose;
    int c040PanelLiveAfterClose;
    int panelContentBeforeClose;
    int panelContentAfterClose;
    int panelGraphicBeforeClose;
    int panelGraphicAfterClose;
    int commandQueueDepthAfterClose;
    int commandQueueDepthAfterCommit;
    int f0302RotationInFlightBeforeClose;
    int f0302RotationInFlightAfterClose;
    int f0302RotationCommittedAfterClose;
    int f0302RotationCommittedAfterCommit;
    int f0282C160ClearCount;
    int f0282NonC160ClearCount;
    int f0302EnterCount;
    int f0302LeaderCommitCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0345FoodWaterDrawCount;
    int f0346C040DrawCount;
    int f0347PanelDrawCount;
    int f0355ToggleSuppressedByCandidateCount;
    int f0359FreshClickCount;
    int f0360PendingReplayCount;
    int f0367StatusBoxClickCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380DrainCount;
    int f0098FloorCeilingDrawCount;
    int f0326MousePointerRefreshCount;
    int saveLoadCount;
    int teleporterCount;
    int partyRotateCount;
    int c040PanelRerenderDuringCloseCount;
    int c160ClearsG0299DespiteRotationPending;
    int closeBypassesF0355CandidateGate;
    int noC040RerenderDuringClose;
    int rotationCommitsAfterClose;
    int noChestOpenOrClose;
    int noExtraCandidateClear;
    int noSaveLoadTeleporterPartyRotate;
    int closeLandsInDungeonRefresh;
    int c040PanelPixelsStable;
    int chestStatePreserved;
    int sourceLockAnchorsPresent;
    int guardRejectsNullState;
    int guardRejectsNullResult;
    int guardRejectsNoCandidate;
    int guardRejectsNoRotationPending;
    int guardRejectsWrongPanel;
    int chestSlotsBefore[DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34];
    int chestSlotsAfter[DM1_V1_MC_C160_ROTATION_CHEST_SLOT_COUNT_PC34];
    int trace[DM1_V1_MC_C160_ROTATION_TRACE_COUNT_PC34];
    uint32_t c040PanelPixelHashBefore;
    uint32_t c040PanelPixelHashAfterClose;
    uint32_t chestHashBefore;
    uint32_t chestHashAfter;
    uint32_t beforeHash;
    uint32_t afterF0302PendingHash;
    uint32_t afterCloseHash;
    uint32_t afterRotationCommitHash;
    uint32_t deterministicHash;
} Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34;

void dm1_v1_mirror_candidate_c160_close_while_rotation_pending_init_pc34(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state,
    uint32_t seed);

int dm1_v1_mirror_candidate_c160_close_while_rotation_pending_run_pc34(
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingStatePc34 *state,
    Dm1V1MirrorCandidateC160CloseWhileRotationPendingResultPc34 *result);

const Dm1V1MirrorCandidateC160CloseWhileRotationPendingEvidencePc34 *
dm1_v1_mirror_candidate_c160_close_while_rotation_pending_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_c160_close_while_rotation_pending_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif
