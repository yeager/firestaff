#ifndef DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_RAPID_WINDOW_TICKS_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_DEFAULT_CANDIDATE_PC34_COMPAT 4u

typedef enum Dm1V1MirrorCandidateDoubleOpenCloseEventKindPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT = 4,
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_DEADZONE_CLICK_PC34_COMPAT = 5
} Dm1V1MirrorCandidateDoubleOpenCloseEventKindPc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat {
    int kind;
    int tick;
    unsigned int candidateChampionOrdinal;
    int x;
    int y;
    int buttonMask;
} Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat {
    int active;
    int partyChampionCount;
    int preC040PartyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int selectedCandidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    unsigned int liveChampionOrdinal;
    int liveChampionHealth;
    int leaderIndex;
    unsigned int leaderHandThingOrdinal;
    int frontD1cMirrorChampionOrdinal;
    int c040PanelOpen;
    int c040PanelPixelsDrawn;
    int mirrorRouteArmed;
    int lastOpenTick;
    int openDispatchCount;
    int closeDispatchCount;
    int candidateAppendCount;
    int duplicateOpenSuppressedCount;
    int duplicateCloseSuppressedCount;
    int deadzoneSuppressedCount;
    int iconRefreshSuppressedCount;
    int resurrectGateProbeCount;
    int reincarnateGateProbeCount;
    int queueDispatchCount;
    int sideEffectFinalizeCount;
} Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat {
    int eventsProcessed;
    int rapidWindowTicks;
    int finalPanelOpen;
    int finalPanelPixelsDrawn;
    int finalPartyChampionCount;
    int finalCandidateChampionOrdinal;
    int finalSelectedCandidateChampionOrdinal;
    int finalInventoryChampionOrdinal;
    int finalLiveChampionHealth;
    int finalLeaderIndex;
    int finalFrontD1cMirrorChampionOrdinal;
    int openDispatchCount;
    int closeDispatchCount;
    int candidateAppendCount;
    int duplicateOpenSuppressedCount;
    int duplicateCloseSuppressedCount;
    int deadzoneSuppressedCount;
    int iconRefreshSuppressedCount;
    int resurrectGateProbeCount;
    int reincarnateGateProbeCount;
    int queueDispatchCount;
    int sideEffectFinalizeCount;
    int openedAtMostOncePerLivePanel;
    int closedAtMostOncePerLivePanel;
    int candidateSelectionPreserved;
    int liveChampionPreserved;
    int leaderHandPreserved;
    int actionGateBlockedWhileOpen;
    int actionGateOpenAfterClose;
} Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseSpecPc34Compat {
    const char *name;
    int rapidWindowTicks;
    const char *contractMarker;
    const char *sourceEvidence;
} Dm1V1MirrorCandidateDoubleOpenCloseSpecPc34Compat;

extern const Dm1V1MirrorCandidateDoubleOpenCloseSpecPc34Compat
    DM1_V1_MirrorCandidateDoubleOpenCloseSpecPc34Compat;

void DM1_V1_MirrorCandidateDoubleOpenClose_InitPc34Compat(
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state);

int DM1_V1_MirrorCandidateDoubleOpenClose_DispatchPc34Compat(
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state,
    const Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat *events,
    unsigned int eventCount,
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat *outResult);

const Dm1V1MirrorCandidateDoubleOpenCloseSpecPc34Compat *
DM1_V1_MirrorCandidateDoubleOpenClose_SpecPc34Compat(void);

const char *DM1_V1_MirrorCandidateDoubleOpenClose_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
