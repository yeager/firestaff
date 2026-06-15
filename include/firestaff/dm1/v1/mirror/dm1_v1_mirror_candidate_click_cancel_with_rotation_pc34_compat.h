#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_WITH_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_WITH_ROTATION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_CC_ROT_PARTY_COUNT_PC34 4
#define DM1_V1_MC_CC_ROT_CHAIN_COUNT_PC34 3
#define DM1_V1_MC_CC_ROT_NONE_PC34 (-1)

typedef struct {
    const char *championPanelRedrawAnchor;
    const char *leaderHandAnchor;
    const char *championSlotAnchor;
    const char *reviveAnchor;
    const char *commandAnchor;
    const char *panelAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
} Dm1V1MirrorCandidateClickCancelWithRotationEvidencePc34Compat;

typedef struct {
    int championOrdinal;
    int alive;
    int leader;
    int statusBoxZone;
    int championIconZone;
    int c040ChainLinked;
} Dm1V1MirrorCandidateClickCancelWithRotationChampionPc34Compat;

typedef struct {
    int contractOnly;
    int partyChampionCount;
    int leaderIndex;
    int pendingLeaderIndex;
    int rotationInFlight;
    int inventoryChampionOrdinal;
    int leaderHandThing;
    int leaderHandEmpty;
    int mousePointerHidden;
    int c040PanelOpen;
    int panelContent;
    int panelGraphic;
    int candidateOwnerIndex;
    int candidateChainIndex;
    int candidateChainCount;
    int candidateChainOrdinals[DM1_V1_MC_CC_ROT_CHAIN_COUNT_PC34];
    int g0299CandidateOrdinal;
    int selectedCandidateOrdinal;
    int selectedCandidateCommitted;
    int c040RedrawState;
    int c040RedrawOrder[6];
    int f0280PublishCount;
    int f0282CancelCount;
    int f0296HidePointerCount;
    int f0296ShowPointerCount;
    int f0301LeaderWriteCount;
    int f0302RotationDispatchCount;
    int f0359SelectCount;
    int f0359CancelCount;
    int panelF0344Count;
    int panelF0345Count;
    int panelF0352Count;
    int panelF0354Count;
    int resurrectPendingCount;
    uint32_t chainHash;
    uint32_t stateHash;
    Dm1V1MirrorCandidateClickCancelWithRotationChampionPc34Compat
        champions[DM1_V1_MC_CC_ROT_PARTY_COUNT_PC34];
} Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat;

typedef struct {
    int accepted;
    int sameTickSequence;
    int selectConsumed;
    int cancelConsumed;
    int rotationConsumed;
    int initialLeaderIndex;
    int finalLeaderIndex;
    int pendingLeaderIndexBefore;
    int pendingLeaderIndexAfter;
    int g0299BeforeClick;
    int g0299AfterClick;
    int g0299AfterCancel;
    int g0299AfterRotation;
    int selectedCandidateAfterClick;
    int selectedCandidateAfterCancel;
    int selectedCandidateCommittedAfterCancel;
    int c040PanelOpenBefore;
    int c040PanelOpenAfterCancel;
    int c040RedrawStateBefore;
    int c040RedrawStateAfterCancel;
    int c040RedrawStateAfterRotation;
    int oldLeaderOwnsCandidateChainAfter;
    int newLeaderInheritedCandidate;
    int chainCountBefore;
    int chainCountAfter;
    int chainIndexBefore;
    int chainIndexAfter;
    int candidateChainPreserved;
    int noResurrectPendingStarted;
    int noSaveLoadOrTeleporterPath;
    int f0302WasBlockedWhileG0299BeforeCancel;
    int f0302AllowedAfterCancel;
    int f0301LeaderWriteCount;
    int f0296PointerHideShowBalanced;
    int panelRedrawReturnedToNoCandidate;
    int statusBoxRedrawUsesNewLeader;
    int championIconRedrawUsesNewLeader;
    int sourceLockAnchorsPresent;
    uint32_t beforeHash;
    uint32_t afterClickHash;
    uint32_t afterCancelHash;
    uint32_t afterRotationHash;
    uint32_t deterministicHash;
} Dm1V1MirrorCandidateClickCancelWithRotationResultPc34Compat;

void dm1_v1_mirror_candidate_click_cancel_with_rotation_init_pc34(
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state);

int dm1_v1_mirror_candidate_click_cancel_with_rotation_run_pc34(
    Dm1V1MirrorCandidateClickCancelWithRotationStatePc34Compat *state,
    Dm1V1MirrorCandidateClickCancelWithRotationResultPc34Compat *result);

const Dm1V1MirrorCandidateClickCancelWithRotationEvidencePc34Compat *
dm1_v1_mirror_candidate_click_cancel_with_rotation_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_click_cancel_with_rotation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
