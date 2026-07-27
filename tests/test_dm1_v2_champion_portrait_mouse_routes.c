/*
 * DM1 V2 champion portrait input regression.
 *
 * V2's composed four-slot HUD is laid out at x=12 with a 77-pixel stride.
 * The visible portrait/name area must select and open each champion's
 * inventory; it cannot depend on the V1 C007 table at x=0/69/138/207.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", message); \
        ++failures; \
    } \
} while (0)

static void seed_party(M11_GameViewState *state) {
    int slot;

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->presentationMode = M12_PRESENTATION_V21_UPSCALED;
    state->world.party.championCount = CHAMPION_MAX_PARTY;
    state->world.party.activeChampionIndex = 0;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        state->world.party.champions[slot].present = 1;
        state->world.party.champions[slot].hp.current = 100;
        state->world.party.champions[slot].hp.maximum = 100;
    }
}

int main(void) {
    M11_GameViewState state;
    int slot;

    seed_party(&state);
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        const int portraitX = 12 + slot * 77 + 24;

        state.inventoryPanelActive = 0;
        CHECK(M11_GameView_HandlePointerButton(
                  &state, portraitX, 14, M11_DM1_MOUSE_MASK_LEFT) ==
                  M11_GAME_INPUT_REDRAW,
              "visible V2 portrait click redraws");
        CHECK(state.inventoryPanelActive == 1,
              "visible V2 portrait opens inventory");
        CHECK(state.world.party.activeChampionIndex == slot,
              "visible V2 portrait selects its champion");
    }

    CHECK(M11_GameView_HandlePointerButton(
              &state, 12 + 3 * 77 + 24, 14, M11_DM1_MOUSE_MASK_LEFT) ==
              M11_GAME_INPUT_REDRAW,
          "second visible V2 portrait click redraws");
    CHECK(state.inventoryPanelActive == 0,
          "second visible V2 portrait click closes inventory");

    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM1 V2 champion portrait input routes");
    return 0;
}
