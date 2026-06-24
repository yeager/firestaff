#ifndef DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_RAPID_WINDOW_TICKS_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_DEFAULT_CANDIDATE_PC34_COMPAT 4u
/* DM1 V1 Hall of Champions C026 atlas geometry per ReDMCSB DEFS.H:821-826.
 * The 256x87 portrait atlas is laid out as 8 columns x 3 rows of 32x29
 * champion portraits. ordinal 13 (WUUF) lives at (col 5, row 1) = source
 * rect (160, 29, 32, 29). The destination D1C wall-ornament viewport cutout
 * is fixed at (96, 35) of size 32x29 per DUNVIEW.C:3913-3928 +
 * DUNVIEW.C:525 G0109 box. This contract is consumed by the
 * HoC champion portrait ordinal 13 WUUF double_click_stability gate
 * (test_dm1_v1_hoc_champion_portrait_13_double_click_stability_pc34_compat). */
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_W_PC34_COMPAT 32
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_H_PC34_COMPAT 29
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_VP_X_PC34_COMPAT 96
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_VP_Y_PC34_COMPAT 35
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_ORDINAL_PC34_COMPAT 13u
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_SRC_X_PC34_COMPAT \
    ((DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_ORDINAL_PC34_COMPAT & 7u) * \
     (unsigned int)DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_W_PC34_COMPAT)
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_SRC_Y_PC34_COMPAT \
    ((DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_ORDINAL_PC34_COMPAT >> 3) * \
     DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_H_PC34_COMPAT)

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
    /* Track the first non-zero candidateChampionOrdinal observed in any
     * open-kind event (RESURRECT_ICON / RESURRECT_HOTKEY / MIRROR_ICON).
     * The candidate_append must preserve this ordinal across close-then-open
     * cycles and rapid double-tap. Filled from the first open event so that
     * callers can drive arbitrary ordinals (e.g. WUUF=13) without depending
     * on the historical DEFAULT_CANDIDATE=4 contract. */
    unsigned int expectedCandidateOrdinal;
    int expectedCandidateOrdinalSeen;
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
    /* Mirrors state->expectedCandidateOrdinal after dispatch. */
    int expectedCandidateOrdinal;
    /* 1 iff selectedCandidateChampionOrdinal matches the first
     * non-zero candidateChampionOrdinal observed in any open event.
     * Closes the HoC ordinal-13 WUUF double_click_stability gate:
     * rapid double-tap and close-then-open must preserve the
     * caller-supplied ordinal (typically 13, not the historical
     * default 4) end-to-end. */
    int candidateOrdinalMatchesExpected;
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
