#include "m11_game_view.h"
#include "dm1_v1_endgame_system_pc34_compat.h"

#include <stdio.h>

static int failures;

static void check(const char* name, int condition) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

static void bind_decoded_pc34_endgame_material(M11_GameViewState* state) {
    static const unsigned char the_end[] = { 1, 2, 3, 4 };
    static const unsigned char mirror[] = { 5, 6, 7, 8 };
    static const unsigned char portraits[] = { 9, 10, 11, 12 };
    static const unsigned char credits[] = { 13, 14, 15, 1 };
    const unsigned char* const pixels[] = {
        the_end, mirror, portraits, credits
    };
    const unsigned int graphic_ids[] = { 6u, 346u, 26u, 5u };
    int index;

    state->assetLoader.initialized = 1;
    state->assetLoader.fileState = state;
    state->assetLoader.runtimeState = state;
    state->assetLoader.cacheUsed = 4;
    for (index = 0; index < 4; ++index) {
        state->assetLoader.cache[index].loaded = 1;
        state->assetLoader.cache[index].graphicIndex = graphic_ids[index];
        state->assetLoader.cache[index].width = 2u;
        state->assetLoader.cache[index].height = 2u;
        state->assetLoader.cache[index].pixels = (unsigned char*)pixels[index];
    }
}

int main(void) {
    M11_GameViewState state;
    DM1_V1_EndgameFinalPresentationReceiptPc34 receipt;

    M11_GameView_Init(&state);
    state.active = 1;
    state.gameWon = 1;
    state.world.gameWon = 1;
    state.endgameCalledWithTrue = 1;
    state.endgameFinalHandoffReady = 1;
    state.endgameRestartAllowed = 0;
    state.endgameFuseSequenceReplayCursor = 45;
    state.endgameFuseSequenceReplayEventCount = 45;
    state.endgameFuseSequenceFrameReplayRemainingTicks = 0;
    state.endgameFuseSequenceDelayRemainingTicks = 0;
    state.endgameTextMessageDelayTicks = 1560;
    state.endgameFinalDelayTicks = DM1_Endgame_GetEndingParams()->finalDelayTicks;
    state.audioState.lastMusicTrackId =
        DM1_Endgame_GetEndingParams()->victoryMusicId;
    state.audioState.titleMusicPlayRequestCount = 1;

    state.assetsAvailable = 0;
    check("receipt builds without assets",
          M11_GameView_BuildEndgameFinalPresentationReceipt(
              &state, &receipt) == 1);
    check("missing assets keep receipt invalid",
          !receipt.valid && !receipt.originalGraphicsDatRoute);
    check("terminal gameplay is still blocked",
          receipt.gameplayInputBlocked);
    check("source music request is preserved",
          receipt.originalMusicRequested);

    state.assetsAvailable = 1;
    check("receipt builds with assets",
          M11_GameView_BuildEndgameFinalPresentationReceipt(
              &state, &receipt) == 1);
    check("asset-ready flag alone cannot validate final route", !receipt.valid);

    bind_decoded_pc34_endgame_material(&state);
    check("receipt builds with decoded PC34 endgame material",
          M11_GameView_BuildEndgameFinalPresentationReceipt(
              &state, &receipt) == 1);
    check("receipt validates terminal route", receipt.valid);
    check("final screen is ready", receipt.finalScreenReady);
    check("save/runtime state is terminal", receipt.saveTerminalStateLocked);
    check("restart controls stay hidden after F0446",
          !receipt.controlsVisible);
    check("F0445 replay drained", receipt.f0445ReplayDrained);
    check("final delay drained", receipt.finalDelayDrained);
    check("text message delays completed", receipt.textMessagesCompleted);
    check("C006 THE END graphic used", receipt.theEndGraphicId == 6);
    check("C346 champion mirror graphic used",
          receipt.championMirrorGraphicId == 346);
    check("C026 portrait atlas used", receipt.championPortraitsGraphicId == 26);
    check("G0012 THE END rect used",
          receipt.theEndX == 120 && receipt.theEndY == 95 &&
          receipt.theEndW == 80 && receipt.theEndH == 14);
    check("G0019 credits palette endpoints used",
          receipt.creditsPaletteSize == 16 &&
          receipt.creditsPaletteFirstEntry == 0x009 &&
          receipt.creditsPaletteLastEntry == 0xFFC);

    return failures ? 1 : 0;
}
