#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); \
        return 1; \
    } \
} while (0)

int main(void) {
    M11_GameViewState state;
    uint32_t gameTick;
    int i;

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.gameWon = 1;
    state.world.gameWon = 1;
    state.world.gameTick = 1234;
    state.endgameFuseSequenceFrameReplayRemainingTicks = 3;
    state.endgameFuseSequenceReplayEventCount = 3;
    state.endgameFuseSequenceReplayTypes[0] = M11_ENDGAME_F0445_EVENT_SETUP;
    state.endgameFuseSequenceReplayTypes[1] =
        M11_ENDGAME_F0445_EVENT_TEXT_MESSAGE;
    state.endgameFuseSequenceReplayTypes[2] =
        M11_ENDGAME_F0445_EVENT_TEXT_MESSAGE;
    state.endgameFuseSequenceReplayDelayTicks[1] = 780;
    state.endgameFuseSequenceReplayDelayTicks[2] = 780;
    state.endgameFinalDelayTicks = 600;
    state.endgameFuseSequenceDelayTicks = 2160;
    state.endgameFuseSequenceDelayRemainingTicks = 2160;
    gameTick = state.world.gameTick;

    CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceReplayCursor == 1);
    CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceReplayCursor == 2);
    CHECK(state.endgameFinalDelayPendingTicks == 780);

    for (i = 0; i < 780; ++i) {
        CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    CHECK(state.endgameFuseSequenceReplayCursor == 2);
    CHECK(state.endgameFuseSequenceDelayRemainingTicks == 1380);
    CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceReplayCursor == 3);
    CHECK(state.endgameFinalDelayPendingTicks == 780);

    for (i = 0; i < 780; ++i) {
        CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    CHECK(state.endgameFuseSequenceDelayRemainingTicks == 600);
    for (i = 0; i < 600; ++i) {
        CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    CHECK(state.endgameFuseSequenceDelayRemainingTicks == 0);
    CHECK(state.endgameFinalHandoffReady == 1);
    CHECK(state.endgameF0444PresentationPhase == 1);
    CHECK(state.world.gameTick == gameTick);
    CHECK(M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
          M11_GAME_INPUT_IGNORED);
    CHECK(state.endgameF0444PresentationPhase == 1);

    /* ENDGAME.C I34E T0444017 holds C3/THE END for 300 delay ticks,
     * then falls through to C005 credits because victory disabled restart. */
    state.endgameF0444PresentationPhase = 2;
    state.endgameTheEndDelayRemainingTicks = 300;
    for (i = 0; i < 299; ++i) {
        CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
        CHECK(state.endgameF0444PresentationPhase == 2);
    }
    CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameTheEndDelayRemainingTicks == 0);
    CHECK(state.endgameF0444PresentationPhase == 3);
    CHECK(state.world.gameTick == gameTick);

    state.endgameF0444PresentationPhase = 2;
    state.endgameRestartAllowed = 1;
    state.endgameTheEndDelayRemainingTicks = 300;
    for (i = 0; i < 300; ++i) {
        CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW);
    }
    CHECK(state.endgameTheEndDelayRemainingTicks == 0);
    CHECK(state.endgameF0444PresentationPhase == 2);
    CHECK(state.world.gameTick == gameTick);
    puts("PASS: DM1 F0446 delays and F0444 THE END hold preserve source order");
    return 0;
}
