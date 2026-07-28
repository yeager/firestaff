#include "m11_game_view.h"
#include "config_m12.h"
#include "render_sdl_m11.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static void set_test_home(void) {
#if defined(_WIN32)
    (void)_putenv("APPDATA=.firestaff-runtime-graphics-popup-test");
#else
    char path[] = "/tmp/firestaff-runtime-graphics-popup-XXXXXX";
    char* home = mkdtemp(path);
    (void)home;
    assert(home != NULL);
    assert(setenv("HOME", home, 1) == 0);
    unsetenv("XDG_CONFIG_HOME");
    unsetenv("XDG_DATA_HOME");
#endif
}

int main(void) {
    M11_GameViewState state;
    M12_Config config;
    unsigned char framebuffer[320 * 200];
    M11_GameInputResult result;
    (void)result;
    size_t i;
    int nonzero = 0;
    (void)nonzero;
    int savedScanlineStrength;
    (void)savedScanlineStrength;
    int savedPhosphor;
    (void)savedPhosphor;

    set_test_home();
    M12_Config_SetDefaults(&config);
    config.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    config.scaleModeIndex = M11_SCALE_FIT;
    assert(M12_Config_Save(&config) == 1);
    M11_GameView_Init(&state);
    state.active = 1;
    state.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = CHAMPION_MAX_PARTY;
    for (int slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        state.world.party.champions[slot].present = 1;
        state.world.party.champions[slot].hp.current = 100;
        state.world.party.champions[slot].hp.maximum = 100;
    }

    result = M11_GameView_HandleInput(&state,
                                      M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 1);
    assert(state.graphicsPopupSelectedRow == 0);

    /* The modal backdrop consumes pointer input before world/HUD hit tests. */
    state.world.party.mapX = 7;
    state.world.party.mapY = 9;
    state.world.party.direction = 1;
    result = M11_GameView_HandlePointerButton(&state, 2, 2,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 1);
    assert(state.world.party.mapX == 7);
    assert(state.world.party.mapY == 9);
    assert(state.world.party.direction == 1);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.presentationMode == M12_PRESENTATION_V20_FILTERED);

    /* Close the F10 modal and exercise the V2 HUD that is actually drawn
     * after the live mode change. Champion four's portrait is deliberately
     * outside the old V1 C007..C010 geometry. */
    result = M11_GameView_HandleInput(&state,
                                      M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);
    result = M11_GameView_HandlePointerButton(
        &state, 12 + 3 * 77 + 24, 14, DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.inventoryPanelActive == 1);
    assert(state.world.party.activeChampionIndex == 3);
    state.inventoryPanelActive = 0;
    result = M11_GameView_HandleInput(&state,
                                      M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 1);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupSelectedRow == 1);

    /* Live changes are serialized through the M12 config owner. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.graphicsIndex == M12_PRESENTATION_V20_FILTERED);
    assert(config.scaleModeIndex == M11_SCALE_STRETCH);

    /* FPS is a presentation diagnostic: it is persisted and sampled from
     * actual host presents without touching game/source tick ownership. */
    for (int i = 0; i < 5; ++i) {
        result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
        assert(result == M11_GAME_INPUT_REDRAW);
    }
    assert(state.graphicsPopupSelectedRow == 6);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.fpsOverlayEnabled == 1);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.showFpsOverlay == 1);
    M11_GameView_RecordPresentedFrame(&state, 1000U);
    M11_GameView_RecordPresentedFrame(&state, 1500U);
    assert(state.fpsOverlayValue == 2U);
    memset(framebuffer, 0x5a, sizeof(framebuffer));
    M11_GameView_DrawFpsOverlay(&state, framebuffer, 320, 200);
    assert(framebuffer[4 * 320 + 4] != 0x5a);

    /* Tab is already the shared runtime CYCLE_CHAMPION token; while the
     * graphics panel owns input it advances to the advanced filter page
     * instead of reaching the game. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 1);
    /* Filters update their persisted/live renderer configuration while the
     * panel remains modal. */
    savedScanlineStrength = config.dm1V2CrtScanlineStrength;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.dm1V2CrtScanlinesEnabled == 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.dm1V2CrtScanlineStrength ==
           (savedScanlineStrength + 1) % 101);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 2);
    savedPhosphor = config.dm1V2PhosphorPersistenceEnabled;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.dm1V2PhosphorPersistenceEnabled != savedPhosphor);

    /* V2.2 is omitted completely when no real art pack has been admitted.
     * Cycling from V2.1 therefore returns directly to V1; prove enhancement
     * rows cannot alter that original-data presentation. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 0);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.presentationMode == M12_PRESENTATION_V21_UPSCALED);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    savedScanlineStrength = config.dm1V2CrtScanlinesEnabled;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.dm1V2CrtScanlinesEnabled == savedScanlineStrength);

    /* The compact panel lives at the right edge and must not dim or obscure
     * the live dungeon viewport while the user adjusts a setting. */
    memset(framebuffer, 0x5a, sizeof(framebuffer));
    M11_GameView_DrawGraphicsPopup(&state, framebuffer, 320, 200);
    assert(framebuffer[90 * 320 + 20] == 0x5a);
    assert(framebuffer[20 * 320 + 180] != 0x5a);
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0) {
            nonzero = 1;
            break;
        }
    }
    assert(nonzero == 1);

    result = M11_GameView_HandlePointerButton(&state, 300, 20,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);

    result = M11_GameView_HandleInput(&state,
                                      M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 1);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);

    /* An admitted path alone is not CSB V2.2 material: a DM1 (or otherwise
     * unrelated) .fsart must not expose CSB's modern mode before the CSB
     * manifest/cache gate reports a complete installation. */
    config.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
    /* Simulate a completed modern pack for another game. CSB's own
     * finished-material gate is still absent and must hide V2.2. */
    config.v22_modern_assets_installed = 1;
    snprintf(config.artpackPath, sizeof(config.artpackPath),
             "%s", "/tmp/other-game.fsart");
    assert(M12_Config_Save(&config) == 1);
    state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    state.presentationMode = M12_PRESENTATION_V21_UPSCALED;
    result = M11_GameView_HandleInput(&state,
                                      M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.presentationMode == M12_PRESENTATION_V1_ORIGINAL);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);

    puts("m11 runtime graphics popup: ok");
    return 0;
}
