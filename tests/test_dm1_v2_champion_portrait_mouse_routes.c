/*
 * DM1 V2 champion portrait input regression.
 *
 * V2's composed four-slot HUD is laid out at x=12 with a 77-pixel stride.
 * The visible portrait/name area must select its champion, following the
 * PC34 C012..C015 status-box command. It cannot depend on V1 source x=0/69/138/207.
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
    state->presentationMode = M12_PRESENTATION_V22_MODERN;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    snprintf(state->sourceId, sizeof(state->sourceId), "dm1");
    state->inventoryPanelActive = 0;
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
        const int nameX = 12 + slot * 77 + 40;

        CHECK(M11_GameView_HandlePointerButton(
                  &state, nameX, 14, M11_DM1_MOUSE_MASK_LEFT) ==
                  M11_GAME_INPUT_REDRAW,
              "visible V2 champion name click redraws");
        CHECK(state.world.party.activeChampionIndex == slot,
              "visible V2 champion name selects its champion");
        CHECK(state.inventoryPanelActive == 0,
              "champion status click does not open inventory");
    }

    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM1 V2 champion portrait input routes");
    return 0;
}
