#ifndef DM1_V1_ENDGAME_PRESENTATION_PC34_COMPAT_H
#define DM1_V1_ENDGAME_PRESENTATION_PC34_COMPAT_H

/* ReDMCSB: ENDGAME.C F0444 lines 440-590 and F0446 lines 939-961. */
typedef enum DM1_V1_EndgamePresentationActionPc34 {
    DM1_V1_ENDGAME_PRESENTATION_ACTION_NONE_PC34 = 0,
    DM1_V1_ENDGAME_PRESENTATION_ACTION_RESTART_PC34,
    DM1_V1_ENDGAME_PRESENTATION_ACTION_RETURN_TO_MENU_PC34
} DM1_V1_EndgamePresentationActionPc34;

typedef struct DM1_V1_EndgamePresentationInputPc34 {
    int gameWon;
    int finalHandoffReady;
    int endgameCalledWithTrue;
    int finalDelayTicks;
    int fuseDelayTicks;
    int fuseDelayRemainingTicks;
    int textMessageDelayTicks;
    int restartAllowed;
    int pointerX;
    int pointerY;
    int pointerPressed;
    int backRequested;
} DM1_V1_EndgamePresentationInputPc34;

typedef struct DM1_V1_EndgamePresentationDecisionPc34 {
    int presentationReady;
    int controlsVisible;
    DM1_V1_EndgamePresentationActionPc34 action;
} DM1_V1_EndgamePresentationDecisionPc34;

typedef struct DM1_V1_EndgameFinalPresentationInputPc34 {
    int gameWon;
    int finalPresentationReady;
    int endgameCalledWithTrue;
    int restartAllowed;
    int replayCursor;
    int replayEventCount;
    int replayFrameRemainingTicks;
    int fuseDelayRemainingTicks;
    int textMessageDelayTicks;
    int finalDelayTicks;
    int requestedMusicTrackId;
    int expectedVictoryMusicId;
    int musicPlayRequestCount;
    int assetsAvailable;
    int f0444MaterialBound;
    int theEndGraphicId;
    int championMirrorGraphicId;
    int championPortraitsGraphicId;
    int theEndX;
    int theEndY;
    int theEndW;
    int theEndH;
    int creditsPaletteSize;
    int creditsPaletteFirstEntry;
    int creditsPaletteLastEntry;
} DM1_V1_EndgameFinalPresentationInputPc34;

typedef struct DM1_V1_EndgameFinalPresentationReceiptPc34 {
    int valid;
    int finalScreenReady;
    int gameplayInputBlocked;
    int saveTerminalStateLocked;
    int controlsVisible;
    int f0445ReplayDrained;
    int finalDelayDrained;
    int textMessagesCompleted;
    int originalMusicRequested;
    int originalGraphicsDatRoute;
    int f0444MaterialBound;
    int creditsPaletteRoute;
    int theEndGraphicId;
    int championMirrorGraphicId;
    int championPortraitsGraphicId;
    int theEndX;
    int theEndY;
    int theEndW;
    int theEndH;
    int creditsPaletteSize;
    int creditsPaletteFirstEntry;
    int creditsPaletteLastEntry;
    const char* sourceEvidence;
} DM1_V1_EndgameFinalPresentationReceiptPc34;

void dm1_v1_endgame_presentation_decide_pc34(
    const DM1_V1_EndgamePresentationInputPc34* input,
    DM1_V1_EndgamePresentationDecisionPc34* output);

int dm1_v1_endgame_final_presentation_receipt_pc34(
    const DM1_V1_EndgameFinalPresentationInputPc34* input,
    DM1_V1_EndgameFinalPresentationReceiptPc34* output);

const char* dm1_v1_endgame_presentation_evidence_pc34(void);

#endif
