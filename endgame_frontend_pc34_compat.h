#ifndef REDMCSB_ENDGAME_FRONTEND_PC34_COMPAT_H
#define REDMCSB_ENDGAME_FRONTEND_PC34_COMPAT_H

void ENDGAME_Compat_ExpandCreditsToScreenBitmap(const unsigned char* graphic, unsigned char* screenBitmap);

typedef enum EndgameCompatSourceEventKind {
    ENDGAME_COMPAT_SOURCE_EVENT_OPTIONAL_LOSS_SCREAM_DELAY = 0,
    ENDGAME_COMPAT_SOURCE_EVENT_LOAD_END_ASSETS = 1,
    ENDGAME_COMPAT_SOURCE_EVENT_FADE_TO_BLACK = 2,
    ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_RENDER = 3,
    ENDGAME_COMPAT_SOURCE_EVENT_CHAMPION_SUMMARY_WAIT_INPUT = 4,
    ENDGAME_COMPAT_SOURCE_EVENT_THE_END_BLIT = 5,
    ENDGAME_COMPAT_SOURCE_EVENT_THE_END_PALETTE_FADE = 6,
    ENDGAME_COMPAT_SOURCE_EVENT_RESTART_DELAY = 7,
    ENDGAME_COMPAT_SOURCE_EVENT_RESTART_BUTTONS_RENDER = 8,
    ENDGAME_COMPAT_SOURCE_EVENT_RESTART_WAIT = 9,
    ENDGAME_COMPAT_SOURCE_EVENT_CREDITS_FADE = 10
} EndgameCompatSourceEventKind;

typedef struct EndgameCompatSourceAnimationStep {
    unsigned int sourceStepOrdinal;
    EndgameCompatSourceEventKind kind;
    unsigned int delayTicks;
    unsigned int vblankLoopCount;
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    const char* sourceLineEvidence;
} EndgameCompatSourceAnimationStep;

unsigned int ENDGAME_Compat_GetSourceAnimationStepCount(void);
int ENDGAME_Compat_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                          EndgameCompatSourceAnimationStep* outStep);
const char* ENDGAME_Compat_GetSourceAnimationEvidence(void);

#endif
