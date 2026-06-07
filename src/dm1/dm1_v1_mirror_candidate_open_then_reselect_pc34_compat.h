#ifndef DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C016_PC34_COMPAT 16
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C159_ZONE_PC34_COMPAT 159
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C161_REINCARNATE_PC34_COMPAT 161
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C162_CANCEL_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_M568_PANEL_PC34_COMPAT 568

typedef struct Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat {
    int contractOnly;
    const char *commandNameRowAnchor;
    const char *commandF0359DispatchAnchor;
    const char *commandPanelMouseAnchor;
    const char *panelF0354ChampionSwitchAnchor;
    const char *chestF0333SameOpenAnchor;
    const char *chestF0334CloseRewriteAnchor;
    const char *reviveF0280NoPendingAnchor;
    const char *reviveF0282ClearAnchor;
    const char *commandGuardAnchor;
    const char *nonOverlapNote;
    const char *contractScope;
} Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateOpenThenReselectChampionPc34Compat {
    unsigned int championOrdinal;
    unsigned int slotFingerprint;
    int present;
    int redrawGeneration;
    int candidateOpenCount;
    int cancelCount;
} Dm1V1MirrorCandidateOpenThenReselectChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat {
    int contractOnly;
    int partyChampionCount;
    int selectedChampionIndex;
    unsigned int g0299CandidateChampionOrdinal;
    int c040PanelOpen;
    int panelContent;
    int c040Graphic;
    int panelOwnerChampionIndex;
    unsigned int panelSlotFingerprint;
    unsigned int leaderHandThing;
    int leaderHandEmpty;
    int c159RowClickCount;
    int f0359PanelDispatchCount;
    int f0354SwitchCount;
    int f0354RedrawCount;
    int f0333SameOpenNoopCount;
    int f0334CloseRewriteCount;
    int f0282CancelCount;
    int noPendingResurrectRejectCount;
    int sameChampionNoopCount;
    int blockedStatusBoxCount;
    int blockedInventoryToggleCount;
    int blockedSpellRuneCount;
    int blockedActionAreaCount;
    int blockedSaveCount;
    int leaderHandPutCount;
    int leaderHandRemoveCount;
    int slotRouteCount;
    int assetLoadCount;
    int pixelParityClaimCount;
    int lastRedrawChampionIndex;
    int lastRedrawGeneration;
    int bRedrawLeakIntoA;
    Dm1V1MirrorCandidateOpenThenReselectChampionPc34Compat champions
        [DM1_V1_MIRROR_CANDIDATE_OPEN_THEN_RESELECT_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat;

typedef struct Dm1V1MirrorCandidateOpenThenReselectResultPc34Compat {
    const Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat *evidence;
    int ok;
    int championAIndex;
    int championBIndex;
    int c159MappedToC016A;
    int c159MappedToC017B;
    int aPanelOpened;
    int bSelectedViaF0354;
    int bPanelReopened;
    int bPanelUsesBSlotState;
    int bCancelClearedPending;
    int reopenedAUsesPreviousAState;
    int noBRedrawLeakedIntoA;
    int handCarryPreserved;
    int redrawCadencePreserved;
    int sameChampionDeadzoneNoop;
    int sameOpenNoopPreserved;
    int noPendingResurrectRejected;
    int closeRewriteRanBeforeBOpen;
    int guardsBlockedWhileG0299;
    int noLeaderHandRoutes;
    int noSlotRoutes;
    int contractOnly;
    int noAssetsOrPixelParity;
    unsigned int leaderHandBefore;
    unsigned int leaderHandAfter;
    unsigned int aSlotBefore;
    unsigned int aSlotAfterReopen;
    unsigned int bSlotBefore;
    unsigned int bSlotAfterReopen;
    unsigned int g0299AfterAOpen;
    unsigned int g0299AfterBOpen;
    unsigned int g0299AfterBCancel;
    unsigned int g0299AfterAReopen;
    int f0354SwitchCountBefore;
    int f0354SwitchCountAfterBSelect;
    int f0354SwitchCountAfterAReselect;
    int f0354RedrawCountBefore;
    int f0354RedrawCountAfterBSelect;
    int f0354RedrawCountAfterAReselect;
    int f0334CloseRewriteCountBefore;
    int f0334CloseRewriteCountAfterBOpen;
    int f0282CancelCountBefore;
    int f0282CancelCountAfterB;
    int blockedStatusBoxCountAfter;
    int blockedInventoryToggleCountAfter;
    int blockedSpellRuneCountAfter;
    int blockedActionAreaCountAfter;
    int blockedSaveCountAfter;
} Dm1V1MirrorCandidateOpenThenReselectResultPc34Compat;

void DM1_V1_MirrorCandidateOpenThenReselect_InitPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state);

int DM1_V1_MirrorCandidateOpenThenReselect_OpenCandidatePc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    int championIndex);

int DM1_V1_MirrorCandidateOpenThenReselect_SelectChampionPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    int championIndex);

int DM1_V1_MirrorCandidateOpenThenReselect_CancelPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state);

int DM1_V1_MirrorCandidateOpenThenReselect_ClickResurrectPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state);

int DM1_V1_MirrorCandidateOpenThenReselect_RunPc34Compat(
    Dm1V1MirrorCandidateOpenThenReselectStatePc34Compat *state,
    Dm1V1MirrorCandidateOpenThenReselectResultPc34Compat *outResult);

const Dm1V1MirrorCandidateOpenThenReselectEvidencePc34Compat *
DM1_V1_MirrorCandidateOpenThenReselect_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
