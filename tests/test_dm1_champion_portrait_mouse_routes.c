/* DM1 V1/V2 champion portrait and name click regression. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", message); \
        ++failures; \
    } \
} while (0)

static int source_pixel_center(int source_coord,
                               int source_extent,
                               int presented_extent) {
    const int low = (int)(((long long)source_coord * presented_extent +
                           source_extent - 1) / source_extent);
    const int high = (int)((((long long)(source_coord + 1) *
                             presented_extent) - 1) / source_extent);
    return (low + high) / 2;
}

static void seed_party(M11_GameViewState *state, int presentation_mode) {
    int slot;

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->presentationMode = presentation_mode;
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

static void test_v1_hidpi_clicks(void) {
    const int window_w = 1780;
    const int window_h = 992;
    const int drawable_w = 3560;
    const int drawable_h = 1984;
    M11_GameViewState state;
    int rect_x = 0;
    int rect_y = 0;
    int rect_w = 0;
    int rect_h = 0;
    int slot;

    seed_party(&state, M12_PRESENTATION_V1_ORIGINAL);
    CHECK(M11_Render_ComputeDrawablePresentationRect(
              window_w, window_h, drawable_w, drawable_h,
              M11_FB_WIDTH, M11_FB_HEIGHT, M11_SCALE_FIT, 0,
              M11_DISPLAY_ASPECT_CONTENT,
              &rect_x, &rect_y, &rect_w, &rect_h) == M11_RENDER_OK,
          "Mac HiDPI drawable presentation rectangle computes");

    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        const int source_x = slot * 69 + 20;
        const int source_y = 14;
        const int drawable_x = rect_x + source_pixel_center(
            source_x, M11_FB_WIDTH, rect_w);
        const int drawable_y = rect_y + source_pixel_center(
            source_y, M11_FB_HEIGHT, rect_h);
        const int window_x = (int)(((long long)drawable_x * window_w) /
                                   drawable_w);
        const int window_y = (int)(((long long)drawable_y * window_h) /
                                   drawable_h);
        int framebuffer_x = -1;
        int framebuffer_y = -1;

        CHECK(M11_Render_MapPointToDrawableFramebuffer(
                  window_x, window_y, window_w, window_h,
                  drawable_w, drawable_h, M11_FB_WIDTH, M11_FB_HEIGHT,
                  M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT,
                  &framebuffer_x, &framebuffer_y) == 1,
              "Mac HiDPI portrait point maps to source framebuffer");
        CHECK(framebuffer_x >= slot * 69 && framebuffer_x <= slot * 69 + 42 &&
                  framebuffer_y >= 0 && framebuffer_y <= 28,
              "Mac HiDPI point lands inside the correct C151..C154 zone");
        CHECK(M11_GameView_HandlePointerButton(
                  &state, framebuffer_x, framebuffer_y,
                  M11_DM1_MOUSE_MASK_LEFT) == M11_GAME_INPUT_REDRAW,
              "Mac HiDPI champion name click dispatches");
        CHECK(state.world.party.activeChampionIndex == slot,
              "Mac HiDPI champion name selects its champion");
        CHECK(state.inventoryPanelActive == 0,
              "Mac HiDPI status click does not open inventory");
    }
}

int main(void) {
    M11_GameViewState state;
    int slot;

    seed_party(&state, M12_PRESENTATION_V22_MODERN);
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

    /* PC34 V1 keeps C012..C015 at x=0/69/138/207, with C007..C010
     * occupying the adjacent 24-pixel bar regions. The reported Mac/PC
     * failure is on this original source-faithful layout, so exercise the
     * same production input handler for every champion name/portrait zone. */
    seed_party(&state, M12_PRESENTATION_V1_ORIGINAL);
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        const int nameX = slot * 69 + 20;

        CHECK(M11_GameView_HandlePointerButton(
                  &state, nameX, 14, M11_DM1_MOUSE_MASK_LEFT) ==
                  M11_GAME_INPUT_REDRAW,
              "visible V1 champion name click redraws");
        CHECK(state.world.party.activeChampionIndex == slot,
              "visible V1 champion name selects its champion");
        CHECK(state.inventoryPanelActive == 0,
              "V1 champion status click does not open inventory");
    }

    test_v1_hidpi_clicks();
    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM1 V1/V2 champion portrait input routes, including Mac HiDPI");
    return 0;
}
