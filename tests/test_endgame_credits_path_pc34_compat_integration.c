#include <stdio.h>
#include "endgame_credits_path_pc34_compat.h"

int main(void) {
    const EndgameCreditsPathCompat* c = endgame_credits_path_GetContract();
    unsigned int ok = endgame_credits_path_GetInvariant();
    printf("probe=firestaff_endgame_credits_path\n");
    printf("sourceEvidence=%s\n", endgame_credits_path_GetEvidence());
    if (c) {
        printf("fadesCreditsPalette=%u\n", c->fadesCreditsPalette);
        printf("discardsInputBeforeWait=%u\n", c->discardsInputBeforeWait);
        printf("waitsForKeyboardOrMouseInput=%u\n",
               c->waitsForKeyboardOrMouseInput);
        printf("reenterRequiresRestartAllowed=%u\n",
               c->reenterRestartRequiresRestartAllowed);
        printf("reenterRequiresDoNotDrawCreditsOnly=%u\n",
               c->reenterRestartRequiresDoNotDrawCreditsOnly);
        printf("reenterClearsWaitBeforeDrawingRestart=%u\n",
               c->reenterRestartClearsWaitBeforeDrawingRestart);
    }
    printf("reenter00=%u\n",
           endgame_credits_path_ShouldReenterRestartAfterCredits(0u, 0u));
    printf("reenter10=%u\n",
           endgame_credits_path_ShouldReenterRestartAfterCredits(1u, 0u));
    printf("reenter01=%u\n",
           endgame_credits_path_ShouldReenterRestartAfterCredits(0u, 1u));
    printf("reenter11=%u\n",
           endgame_credits_path_ShouldReenterRestartAfterCredits(1u, 1u));
    printf("endgameCreditsPathInvariantOk=%u\n", ok);
    return ok ? 0 : 1;
}
