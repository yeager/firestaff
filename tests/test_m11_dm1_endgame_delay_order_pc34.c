#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static M11_GameInputResult advance_edges(M11_GameViewState* state,
                                         uint32_t edges) {
    uint64_t needed = (uint64_t)edges * 1000u - state->endgameVblankPhase;
    uint32_t elapsed = (uint32_t)((needed + 69u) / 70u);
    return M11_GameView_AdvanceDm1Pc34EndgameClockMs(state, elapsed);
}

int main(void) {
    M11_GameViewState state;
    uint32_t gameTick;
    int i;

    memset(&state, 0, sizeof(state));
    state.active = 1;
    state.gameWon = 1;
    state.world.gameWon = 1;
    state.world.gameTick = 1234;
    state.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;
    snprintf(state.sourceId, sizeof(state.sourceId), "dm1");
    DM1_V1_VBlankTiming_ConfigurePc34(&state.vblankTiming);
    state.endgameFuseSequenceFrameReplayRemainingTicks = 3;
    state.endgameFuseSequenceReplayEventCount = 3;
    state.endgameFuseSequenceReplayTypes[0] = M11_ENDGAME_F0445_EVENT_SETUP;
    state.endgameFuseSequenceReplayTypes[1] = M11_ENDGAME_F0445_EVENT_TEXT_MESSAGE;
    state.endgameFuseSequenceReplayTypes[2] = M11_ENDGAME_F0445_EVENT_TEXT_MESSAGE;
    state.endgameFuseSequenceReplayDelayTicks[1] = 780;
    state.endgameFuseSequenceReplayDelayTicks[2] = 780;
    state.endgameFinalDelayTicks = 600;
    state.endgameFuseSequenceDelayTicks = 2160;
    state.endgameFuseSequenceDelayRemainingTicks = 2160;
    gameTick = state.world.gameTick;

    CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_IGNORED);
    CHECK(state.endgameFuseSequenceReplayCursor == 0);
    CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceReplayCursor == 1);
    CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceReplayCursor == 2);
    CHECK(state.endgameFinalDelayPendingTicks == 780);

    for (i = 0; i < 780; ++i)
        CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceReplayCursor == 2);
    CHECK(state.endgameFuseSequenceDelayRemainingTicks == 1380);
    CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceReplayCursor == 3);
    CHECK(state.endgameFinalDelayPendingTicks == 780);

    for (i = 0; i < 780; ++i)
        CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceDelayRemainingTicks == 600);
    for (i = 0; i < 600; ++i)
        CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameFuseSequenceDelayRemainingTicks == 0);
    CHECK(state.endgameFinalHandoffReady == 1);
    CHECK(state.endgameF0444PresentationPhase == 1);
    CHECK(state.world.gameTick == gameTick);
    CHECK(M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
          M11_GAME_INPUT_IGNORED);
    CHECK(state.endgameF0444PresentationPhase == 1);

    /* I34E T0444017 holds THE END for exactly 300 VGA VBlank delays. */
    state.endgameF0444PresentationPhase = 2;
    state.endgameTheEndDelayRemainingTicks = 300;
    CHECK(M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_IGNORED);
    CHECK(state.endgameTheEndDelayRemainingTicks == 300);
    CHECK(advance_edges(&state, 299) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameF0444PresentationPhase == 2);
    CHECK(state.endgameTheEndDelayRemainingTicks == 1);
    CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameTheEndDelayRemainingTicks == 0);
    CHECK(state.endgameF0444PresentationPhase == 3);
    CHECK(state.world.gameTick == gameTick);

    state.endgameF0444PresentationPhase = 2;
    state.endgameRestartAllowed = 1;
    state.endgameTheEndDelayRemainingTicks = 300;
    CHECK(advance_edges(&state, 300) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameTheEndDelayRemainingTicks == 0);
    CHECK(state.endgameF0444PresentationPhase == 2);
    CHECK(state.endgameRestartWaitRemainingTicks == 899);
    CHECK(advance_edges(&state, 898) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameRestartWaitRemainingTicks == 1);
    CHECK(state.endgameF0444PresentationPhase == 2);
    CHECK(advance_edges(&state, 1) == M11_GAME_INPUT_REDRAW);
    CHECK(state.endgameRestartWaitRemainingTicks == 0);
    CHECK(state.endgameF0444PresentationPhase == 3);
    CHECK(state.world.gameTick == gameTick);

    /* 70 rational edges per second: 899 source waits finish in ~12.85 s. */
    CHECK((uint64_t)12850u * 70u / 1000u >= 899u);
    puts("PASS: PC34 DM1 endgame delays advance on source VGA VBlank cadence");
    return 0;
}
