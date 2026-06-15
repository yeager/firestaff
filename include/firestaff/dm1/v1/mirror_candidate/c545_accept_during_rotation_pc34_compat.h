#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C545_ACCEPT_DURING_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C545_ACCEPT_DURING_ROTATION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34 4
#define DM1_V1_MC_C545_ACCEPT_ROTATE_TRACE_COUNT_PC34 10
#define DM1_V1_MC_C545_ACCEPT_ROTATE_PARTY_COUNT_PC34 4
#define DM1_V1_MC_C545_ACCEPT_ROTATE_NONE_PC34 0xffffu

typedef struct {
    const char *revivePublishAnchor;
    const char *reviveAcceptAnchor;
    const char *panelFoodWaterAnchor;
    const char *panelC545Anchor;
    const char *championHandAnchor;
    const char *championSlotAnchor;
    const char *commandQueueAnchor;
    const char *commandPanelRouteAnchor;
    const char *commandWheelQueueAnchor;
    const char *commandDrainAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
} Dm1V1MirrorCandidateC545AcceptDuringRotationEvidencePc34;

typedef struct {
    int ordinal;
    int alive;
    int leader;
    int chainLinked;
    uint16_t handThing;
} Dm1V1MirrorCandidateC545AcceptDuringRotationChampionPc34;

typedef struct {
    int contractOnly;
    int noDosPixelParityClaim;
    int partyChampionCount;
    int leaderIndex;
    int queuedLeaderIndex;
    int inventoryChampionOrdinal;
    int candidateChampionOrdinal;
    int c040CandidateIndex;
    int candidateChainOrdinals[DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    int championChainOrdinals[DM1_V1_MC_C545_ACCEPT_ROTATE_PARTY_COUNT_PC34];
    int g0299CandidateOrdinal;
    int c045CandidateOpen;
    int c040PanelOpen;
    int c040CloseQueued;
    int c040CloseCompleted;
    int leaderRotationQueued;
    int leaderRotationDrained;
    int panelContent;
    int panelGraphic;
    int c545Zone;
    uint32_t c545PanelPixel;
    uint32_t c040RedrawState;
    int c545AcceptCommand;
    int f0280PublishCount;
    int f0282AcceptClearCount;
    int f0297LeaderHandCount;
    int f0298LeaderHandCount;
    int f0301SlotAddCount;
    int f0302SlotDispatchCount;
    int f0344FoodWaterReadCount;
    int f0345FoodWaterDrawCount;
    int f0351PanelDrawCount;
    int f0352PanelPressCount;
    int f0353PanelRestoreCount;
    int f0359QueueWriteCount;
    int f0361WheelQueueWriteCount;
    int f0378PanelRouteCount;
    int f0380DrainCount;
    int f0368SetLeaderCount;
    int c040GateRejectCount;
    int rotationGateRejectCount;
    int candidateSensorDisabled;
    int candidateRemovedFromChain;
    int acceptedAfterBothGates;
    int commandQueueDepth;
    int trace[DM1_V1_MC_C545_ACCEPT_ROTATE_TRACE_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterRejectedC040Hash;
    uint32_t afterCloseRejectedRotationHash;
    uint32_t afterAcceptHash;
    Dm1V1MirrorCandidateC545AcceptDuringRotationChampionPc34
        champions[DM1_V1_MC_C545_ACCEPT_ROTATE_PARTY_COUNT_PC34];
} Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34;

typedef struct {
    int accepted;
    int c545AcceptRoute;
    int c040GateRequired;
    int rotationGateRequired;
    int rejectedBeforeC040Close;
    int rejectedBeforeRotationDrain;
    int stableThroughRejectedC545;
    int stableUntilBothGates;
    int c040CloseCompletedBeforeAccept;
    int rotationDrainedBeforeAccept;
    int acceptAfterGatesSucceeded;
    int g0299PreservedBeforeGates;
    int g0299ClearedAfterAccept;
    int c040CandidateIndexPreserved;
    int c040RedrawStatePreserved;
    int championChainPreservedBeforeGates;
    int c545PanelPixelPreservedBeforeGates;
    int candidateRemovedFromChain;
    int leaderRotationCompleted;
    int leaderHandCoherentAfterRotation;
    int sourceAnchorsPresent;
    int guardRejectsNullState;
    int guardRejectsNullResult;
    int guardRejectsNonContract;
    int guardRejectsNoCandidate;
    int guardRejectsWrongPanel;
    int guardRejectsNoRotation;
    int guardRejectsNoCloseQueued;
    int leaderBefore;
    int leaderAfter;
    int g0299Before;
    int g0299AfterRejectedC040;
    int g0299AfterRejectedRotation;
    int g0299AfterAccept;
    int c040CandidateIndexBefore;
    int c040CandidateIndexAfterRejectedC040;
    int c040CandidateIndexAfterRejectedRotation;
    uint32_t c040RedrawStateBefore;
    uint32_t c040RedrawStateAfterRejectedC040;
    uint32_t c040RedrawStateAfterRejectedRotation;
    uint32_t c545PanelPixelBefore;
    uint32_t c545PanelPixelAfterRejectedC040;
    uint32_t c545PanelPixelAfterRejectedRotation;
    int candidateChainBefore[DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    int candidateChainAfterRejectedC040
        [DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    int candidateChainAfterRejectedRotation
        [DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    int candidateChainAfterAccept[DM1_V1_MC_C545_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    int trace[DM1_V1_MC_C545_ACCEPT_ROTATE_TRACE_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterRejectedC040Hash;
    uint32_t afterCloseRejectedRotationHash;
    uint32_t afterAcceptHash;
    uint32_t hash;
} Dm1V1MirrorCandidateC545AcceptDuringRotationResultPc34;

void dm1_v1_mirror_candidate_c545_accept_during_rotation_init_pc34(
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state);

int dm1_v1_mirror_candidate_c545_accept_during_rotation_run_pc34(
    Dm1V1MirrorCandidateC545AcceptDuringRotationStatePc34 *state,
    Dm1V1MirrorCandidateC545AcceptDuringRotationResultPc34 *result);

const Dm1V1MirrorCandidateC545AcceptDuringRotationEvidencePc34 *
dm1_v1_mirror_candidate_c545_accept_during_rotation_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_c545_accept_during_rotation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
