#ifndef DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TRAVERSAL_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TRAVERSAL_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_CANDIDATE_COUNT_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_THOUGHT_SLOT_COUNT_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT 48
#define DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT (-1)

typedef enum Dm1V1MirrorCandidateThoughtProjectCommandPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C157_PROJECT_PC34_COMPAT = 157,
    DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C158_COMMIT_PC34_COMPAT = 158,
    DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C162_CLOSE_PC34_COMPAT = 162
} Dm1V1MirrorCandidateThoughtProjectCommandPc34Compat;

typedef struct Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat {
    int contractOnly;
    const char *commandKeyDispatchAnchor;
    const char *commandQueueDispatchAnchor;
    const char *commandPanelDispatchAnchor;
    const char *candidatePublishAnchor;
    const char *candidateCloseAnchor;
    const char *thoughtTraversalAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateThoughtProjectCandidatePc34Compat {
    unsigned int championOrdinal;
    char thoughtSlots
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_THOUGHT_SLOT_COUNT_PC34_COMPAT]
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    char committedThought
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
} Dm1V1MirrorCandidateThoughtProjectCandidatePc34Compat;

typedef struct Dm1V1MirrorCandidateThoughtProjectStatePc34Compat {
    int contractOnly;
    int panelOpen;
    int activeCandidateIndex;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int g0420CandidateIdentityOrdinal;
    int pendingThoughtActive;
    int pendingThoughtCandidateIndex;
    int projectedSlotIndex;
    int projectedSlotCount;
    int projectDispatchCount;
    int commitDispatchCount;
    int closeDispatchCount;
    int reopenDispatchCount;
    char statusBoxText
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    char pendingThoughtText
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    Dm1V1MirrorCandidateThoughtProjectCandidatePc34Compat candidates
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_CANDIDATE_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateThoughtProjectStatePc34Compat;

typedef struct Dm1V1MirrorCandidateThoughtProjectResultPc34Compat {
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *evidence;
    Dm1V1MirrorCandidateThoughtProjectCommandPc34Compat command;
    int panelOpenBefore;
    int panelOpenAfter;
    int activeCandidateIndexBefore;
    int activeCandidateIndexAfter;
    unsigned int g0299Before;
    unsigned int g0299After;
    unsigned int g0420Before;
    unsigned int g0420After;
    int pendingActiveBefore;
    int pendingActiveAfter;
    int pendingCandidateBefore;
    int pendingCandidateAfter;
    int projectedSlotIndexBefore;
    int projectedSlotIndexAfter;
    int projectedSlotCountBefore;
    int projectedSlotCountAfter;
    int projectDispatchCountBefore;
    int projectDispatchCountAfter;
    int commitDispatchCountBefore;
    int commitDispatchCountAfter;
    int closeDispatchCountBefore;
    int closeDispatchCountAfter;
    int reopenDispatchCountBefore;
    int reopenDispatchCountAfter;
    char statusBoxBefore
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    char statusBoxAfter
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    char pendingTextBefore
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    char pendingTextAfter
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    char committedBefore
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    char committedAfter
        [DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_TEXT_SIZE_PC34_COMPAT];
    int liveCandidatePreserved;
    int thoughtProjected;
    int thoughtCommitted;
    int emptyThoughtNoOp;
    int firstSlotOnly;
    int projectStateCleared;
    int reopenReset;
} Dm1V1MirrorCandidateThoughtProjectResultPc34Compat;

void DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state);

int DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateThoughtProjectTraversal_CommitPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateThoughtProjectTraversal_SwapCandidatePc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    int candidateIndex);

int DM1_V1_MirrorCandidateThoughtProjectTraversal_ClosePc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateThoughtProjectTraversal_ReopenPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult);

const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *
DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
