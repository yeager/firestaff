#ifndef MENU_DATA_ROOT_POLICY_M12_H
#define MENU_DATA_ROOT_POLICY_M12_H

/* A selected media root is authoritative once it admits a playable game.
 * A --game request may recover from an unrelated configured root, but only
 * when the candidate actually admits that requested game. */
static inline int M12_MenuDataRoot_ShouldUseCandidate(
    int explicitRoot, int selectedReadyCount, int candidateReadyCount,
    int requestedGame, int selectedRequestedReady, int candidateRequestedReady) {
    if (explicitRoot || candidateReadyCount <= 0) return 0;
    if (requestedGame) return !selectedRequestedReady && candidateRequestedReady;
    return selectedReadyCount <= 0;
}

#endif
