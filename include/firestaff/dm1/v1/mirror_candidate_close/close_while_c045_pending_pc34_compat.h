#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_WHILE_C045_PENDING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_WHILE_C045_PENDING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34 4
#define DM1_V1_MC_CLOSE_C045_PENDING_TRACE_COUNT_PC34 10
#define DM1_V1_MC_CLOSE_C045_PENDING_NONE_PC34 0xffffu

typedef struct {
    const char *revivePublishAnchor;
    const char *reviveCloseAnchor;
    const char *panelFoodWaterAnchor;
    const char *panelResurrectAnchor;
    const char *commandQueueAnchor;
    const char *commandPendingAnchor;
    const char *commandPanelRouteAnchor;
    const char *commandDrainAnchor;
    const char *championHandAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
} Dm1V1MirrorCandidateCloseWhileC045PendingEvidencePc34;

typedef struct {
    int contractOnly;
    int noGameDataRequired;
    uint32_t seed;
    int partyChampionCount;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int candidateChampionOrdinal;
    int g0299CandidateOrdinal;
    int c040PanelOpen;
    int c045FoodWaterAcceptPending;
    int c160CloseClickQueued;
    int pendingClickStoredWhileLocked;
    int panelContent;
    int panelGraphic;
    int queuedC045Command;
    int queuedCloseCommand;
    uint16_t c045FoodThing;
    uint16_t leaderHandThing;
    int candidateChainOrdinals[DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34];
    int partyChainOrdinals[DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34];
    int commandQueueDepth;
    int f0280PublishCount;
    int f0282CloseClearCount;
    int f0298RemoveLeaderHandCount;
    int f0344FoodWaterReadCount;
    int f0345FoodWaterDrawCount;
    int f0346ResurrectDrawCount;
    int f0359QueueWriteCount;
    int f0360PendingReplayCount;
    int f0378PanelRouteCount;
    int f0380DrainCount;
    int staleC045RejectCount;
    int candidateRemovedFromChain;
    int candidateSensorDisabled;
    int foodConsumed;
    int closeClearedPendingC045;
    int trace[DM1_V1_MC_CLOSE_C045_PENDING_TRACE_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterStaleC045Hash;
    uint32_t afterPendingReplayHash;
    uint32_t afterCloseHash;
} Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34;

typedef struct {
    int accepted;
    int c045PendingAtStart;
    int c160CloseArrivedWhileLocked;
    int staleC045RejectedBeforeClose;
    int candidatePreservedUntilClose;
    int pendingClickReplayed;
    int closeDispatchedThroughC040Panel;
    int g0299ClearedByClose;
    int c045PendingClearedByClose;
    int candidateRemovedFromChain;
    int foodNotConsumed;
    int leaderHandStable;
    int noLeaderHandRemoval;
    int queueDrained;
    int panelClosedAfterC160;
    int sourceAnchorsPresent;
    int guardRejectsNullState;
    int guardRejectsNullResult;
    int guardRejectsNoCandidate;
    int guardRejectsWrongPanel;
    int guardRejectsNoC045Pending;
    int guardRejectsNoCloseClick;
    int g0299Before;
    int g0299AfterStaleC045;
    int g0299AfterClose;
    uint16_t leaderHandBefore;
    uint16_t leaderHandAfter;
    int candidateChainBefore[DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34];
    int candidateChainAfterStaleC045
        [DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34];
    int candidateChainAfterClose[DM1_V1_MC_CLOSE_C045_PENDING_CHAIN_COUNT_PC34];
    int trace[DM1_V1_MC_CLOSE_C045_PENDING_TRACE_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterStaleC045Hash;
    uint32_t afterPendingReplayHash;
    uint32_t afterCloseHash;
    uint32_t hash;
} Dm1V1MirrorCandidateCloseWhileC045PendingResultPc34;

void dm1_v1_mirror_candidate_close_while_c045_pending_init_pc34(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state,
    uint32_t seed);

int dm1_v1_mirror_candidate_close_while_c045_pending_run_pc34(
    Dm1V1MirrorCandidateCloseWhileC045PendingStatePc34 *state,
    Dm1V1MirrorCandidateCloseWhileC045PendingResultPc34 *result);

const Dm1V1MirrorCandidateCloseWhileC045PendingEvidencePc34 *
dm1_v1_mirror_candidate_close_while_c045_pending_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_close_while_c045_pending_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
