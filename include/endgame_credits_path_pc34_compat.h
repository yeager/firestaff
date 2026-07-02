#ifndef ENDGAME_CREDITS_PATH_PC34_COMPAT_H
#define ENDGAME_CREDITS_PATH_PC34_COMPAT_H

typedef struct EndgameCreditsPathCompat {
    unsigned int fadesCreditsPalette;
    unsigned int discardsInputBeforeWait;
    unsigned int waitsForKeyboardOrMouseInput;
    unsigned int reenterRestartRequiresRestartAllowed;
    unsigned int reenterRestartRequiresDoNotDrawCreditsOnly;
    unsigned int reenterRestartClearsWaitBeforeDrawingRestart;
} EndgameCreditsPathCompat;

const char* endgame_credits_path_GetEvidence(void);
const EndgameCreditsPathCompat* endgame_credits_path_GetContract(void);
unsigned int endgame_credits_path_ShouldReenterRestartAfterCredits(
    unsigned int restartAllowed,
    unsigned int doNotDrawCreditsOnly);
unsigned int endgame_credits_path_GetInvariant(void);

#endif
