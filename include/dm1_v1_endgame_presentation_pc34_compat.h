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

void dm1_v1_endgame_presentation_decide_pc34(
    const DM1_V1_EndgamePresentationInputPc34* input,
    DM1_V1_EndgamePresentationDecisionPc34* output);

const char* dm1_v1_endgame_presentation_evidence_pc34(void);

#endif
