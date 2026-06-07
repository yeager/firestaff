#ifndef DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C040_PANEL_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_PANEL_CONTENT_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C160_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C161_PC34_COMPAT 161
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_C162_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_STATUS_BOX_0_PC34_COMPAT 12
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_INVENTORY_0_PC34_COMPAT 7
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_CLOSE_INVENTORY_PC34_COMPAT 11
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_SPELL_AREA_PC34_COMPAT 100
#define DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_ACTION_AREA_PC34_COMPAT 111

typedef struct Dm1V1MirrorCandidateNoPendingResurrectChampionPc34Compat {
    unsigned int championOrdinal;
    int currentHealth;
    int portraitOrdinal;
    int present;
    int rearmed;
} Dm1V1MirrorCandidateNoPendingResurrectChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat {
    int active;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int g0305PartyChampionCount;
    unsigned int inventoryChampionOrdinal;
    int inventoryPanelOpen;
    int panelContent;
    int c040PanelOpen;
    int c040PanelPixelsDrawn;
    unsigned int c040PanelPixelHash;
    int leaderIndex;
    int leaderHandEmpty;
    int mirrorRouteOpen;
    int frontD1cMirrorChampionOrdinal;
    int championRearmCount;
    int f0282ResurrectCallCount;
    int f0282ReincarnateCallCount;
    int f0282CancelCallCount;
    int magicCasterChampionIndex;
    Dm1V1MirrorCandidateNoPendingResurrectChampionPc34Compat
        champions[DM1_V1_MIRROR_CANDIDATE_NO_PENDING_RESURRECT_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat;

typedef struct Dm1V1MirrorCandidateNoPendingResurrectEvidencePc34Compat {
    const char *candidatePublishAnchor;
    const char *f0282CancelAnchor;
    const char *f0282ConfirmAnchor;
    const char *commandGateAnchor;
    const char *panelEmptyHandAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidateNoPendingResurrectEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat {
    const Dm1V1MirrorCandidateNoPendingResurrectEvidencePc34Compat *evidence;
    int command;
    int consumed;
    int validPanelCommand;
    int panelRouteOpen;
    int ignoredWrongCommand;
    int ignoredLeaderHandFull;
    int ignoredNoPendingCandidate;
    int wouldReachF0282;
    int noF0282Called;
    int resurrectCallPreserved;
    int cancelCallPreserved;
    unsigned int g0299Before;
    unsigned int g0299After;
    unsigned int g0305Before;
    unsigned int g0305After;
    unsigned int inventoryChampionOrdinalBefore;
    unsigned int inventoryChampionOrdinalAfter;
    int inventoryPanelOpenBefore;
    int inventoryPanelOpenAfter;
    int panelContentBefore;
    int panelContentAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    int c040PanelPixelsBefore;
    int c040PanelPixelsAfter;
    unsigned int c040PanelPixelHashBefore;
    unsigned int c040PanelPixelHashAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int mirrorRouteOpenBefore;
    int mirrorRouteOpenAfter;
    int frontD1cMirrorChampionOrdinalBefore;
    int frontD1cMirrorChampionOrdinalAfter;
    int championRearmCountBefore;
    int championRearmCountAfter;
    int f0282ResurrectCallCountBefore;
    int f0282ResurrectCallCountAfter;
    int f0282ReincarnateCallCountBefore;
    int f0282ReincarnateCallCountAfter;
    int f0282CancelCallCountBefore;
    int f0282CancelCallCountAfter;
    int g0299Preserved;
    int g0305Preserved;
    int c040PanelPreserved;
    int inventoryPreserved;
    int mirrorRoutePreserved;
    int noChampionRearmed;
} Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat;

typedef struct Dm1V1MirrorCandidateNoPendingResurrectGateResultPc34Compat {
    int command;
    int blockedByG0299;
    int statusBoxAllowed;
    int inventoryAllowed;
    int spellAreaAllowed;
    int actionAreaAllowed;
} Dm1V1MirrorCandidateNoPendingResurrectGateResultPc34Compat;

void DM1_V1_MirrorCandidateNoPendingResurrect_InitPc34Compat(
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state);

int DM1_V1_MirrorCandidateNoPendingResurrect_ProcessPanelCommandPc34Compat(
    Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateNoPendingResurrectResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateNoPendingResurrect_CanDispatchCommandPc34Compat(
    const Dm1V1MirrorCandidateNoPendingResurrectStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateNoPendingResurrectGateResultPc34Compat *outResult);

const Dm1V1MirrorCandidateNoPendingResurrectEvidencePc34Compat *
DM1_V1_MirrorCandidateNoPendingResurrect_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
