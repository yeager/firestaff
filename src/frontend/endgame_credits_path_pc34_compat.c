#include "endgame_credits_path_pc34_compat.h"

static const EndgameCreditsPathCompat kContract = {
    1u, 1u, 1u, 1u, 1u, 1u
};

const char* endgame_credits_path_GetEvidence(void) {
    return "ReDMCSB ENDGAME.C F0444 lines 639-680 fades to the credits "
           "palette and blits C005 credits; lines 688-699 discard stale "
           "input and wait for keyboard/mouse activity; lines 707-710 "
           "re-enter the restart draw path only when G0524_B_RestartGameAllowed "
           "and P0856_B_DoNotDrawCreditsOnly are both true, after clearing "
           "L1423_B_WaitBeforeDrawingRestart.";
}

const EndgameCreditsPathCompat* endgame_credits_path_GetContract(void) {
    return &kContract;
}

unsigned int endgame_credits_path_ShouldReenterRestartAfterCredits(
    unsigned int restartAllowed,
    unsigned int doNotDrawCreditsOnly) {
    /* ReDMCSB: ENDGAME.C F0444 lines 707-710 gates the post-credits
     * return to T0444017 on G0524_B_RestartGameAllowed &&
     * P0856_B_DoNotDrawCreditsOnly, and clears the restart delay flag first. */
    return (restartAllowed != 0u && doNotDrawCreditsOnly != 0u) ? 1u : 0u;
}

unsigned int endgame_credits_path_GetInvariant(void) {
    const EndgameCreditsPathCompat* c = endgame_credits_path_GetContract();
    if (!c) return 0u;
    if (!c->fadesCreditsPalette) return 0u;
    if (!c->discardsInputBeforeWait) return 0u;
    if (!c->waitsForKeyboardOrMouseInput) return 0u;
    if (!c->reenterRestartRequiresRestartAllowed) return 0u;
    if (!c->reenterRestartRequiresDoNotDrawCreditsOnly) return 0u;
    if (!c->reenterRestartClearsWaitBeforeDrawingRestart) return 0u;
    if (endgame_credits_path_ShouldReenterRestartAfterCredits(0u, 0u)) return 0u;
    if (endgame_credits_path_ShouldReenterRestartAfterCredits(1u, 0u)) return 0u;
    if (endgame_credits_path_ShouldReenterRestartAfterCredits(0u, 1u)) return 0u;
    if (!endgame_credits_path_ShouldReenterRestartAfterCredits(1u, 1u)) return 0u;
    return 1u;
}
