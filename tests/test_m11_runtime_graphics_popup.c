#include "m11_game_view.h"
#include "m11_qol_runtime.h"
#include "config_m12.h"
#include "render_sdl_m11.h"
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_texture_upscale_pc34.h"

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

    /* Every visible F10 control has a mouse path.  Exercise the tabs, a
     * value row and the close button through the same source-space pointer
     * dispatcher used by the SDL frontend. */
    result = M11_GameView_HandlePointerButton(&state, 220, 34,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 1);
    result = M11_GameView_HandlePointerButton(&state, 220, 58,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupSelectedRow == 1);
    /* Mouse controls are bidirectional: clicking the selected value with
     * Button II is the pointer equivalent of the keyboard LEFT key. */
    result = M11_GameView_HandlePointerButton(&state, 180, 34,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 0);
    result = M11_GameView_HandlePointerButton(&state, 220, 58,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupSelectedRow == 1);
    result = M11_GameView_HandlePointerButton(&state, 220, 58,
                                               DM1_V1_MOUSE_MASK_RIGHT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.scaleModeIndex == M11_SCALE_4X);
    /* Keep the remainder of this long regression on its documented FIT
     * baseline; the assertion above has already proved the pointer route. */
    config.scaleModeIndex = M11_SCALE_FIT;
    assert(M12_Config_Save(&config) == 1);
    result = M11_GameView_HandlePointerButton(&state, 306, 14,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);

    result = M11_GameView_HandleInput(&state,
                                      M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 1);
    assert(state.graphicsPopupPage == 0);
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
    assert(state.presentationMode == M12_PRESENTATION_V21_UPSCALED);

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
    assert(config.graphicsIndex == M12_PRESENTATION_V21_UPSCALED);
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
     * The new CHEATS page is still available, so cycle past it before
     * returning to presentation; prove enhancement rows cannot alter the
     * original-data presentation. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 3);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 0);
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

    /* CSB V2.2 cannot be opened by a test-generated manifest or host PNG
     * placeholders. The production renderer accepts only future original CSB
     * decoded material; until then the F10 route stays on V1/V2.1. */

    /* CSB owns a separate V2 filter contract. The in-game F10 panel must
     * persist and apply CSB's filter settings, not mutate DM1's settings
     * while claiming that it changed the active CSB renderer. */
    config.dm1V2CrtScanlinesEnabled = 0;
    config.dm1V2PaletteCorrectionEnabled = 0;
    config.dm1V2DitherCleanupEnabled = 0;
    config.csbV2CrtScanlinesEnabled = 0;
    config.csbV2CrtScanlineStrength = 35;
    config.csbV2PaletteCorrectionEnabled = 0;
    config.csbV2DitherCleanupEnabled = 0;
    config.csbV2ScalePercent = 200;
    config.csbV2BilinearEnabled = 0;
    assert(M12_Config_Save(&config) == 1);
    state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    state.presentationMode = M12_PRESENTATION_V21_UPSCALED;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.csbV2CrtScanlinesEnabled == 1);
    assert(config.dm1V2CrtScanlinesEnabled == 0);
    assert(csb_v2_filter_config_get()->crtScanlinesEnabled == 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.csbV2CrtScanlineStrength == 36);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.csbV2PaletteCorrectionEnabled == 1);
    assert(csb_v2_filter_config_get()->paletteCorrectionEnabled == 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.csbV2DitherCleanupEnabled == 1);
    assert(csb_v2_filter_config_get()->ditherCleanupEnabled == 1);
    /* The remaining CSB-only rows are real V2.1 presentation controls.
     * They must update the independent CSB EPX runtime, not merely persist
     * decoration in M12's config. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.csbV2ScalePercent == 400);
    assert(csb_v2_upscale_get_scale() == 4);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.csbV2BilinearEnabled == 1);
    assert(csb_v2_upscale_get_bilinear() == 1);

    /* The fourth F10 page is backed by the same real cheat settings exposed
     * by the launcher. It must work in the live CSB slot without affecting
     * DM1's slot, and speed must reach the live scheduler immediately. */
    config.gameCheatsEnabled[0] = 0;
    config.gameSpeed[0] = 1;
    config.gameCheatsEnabled[1] = 0;
    config.gameSpeed[1] = 1;
    config.gameSpeedMultiplier = 100;
    assert(M12_Config_Save(&config) == 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 2);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 3);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.gameCheatsEnabled[1] == 1);
    assert(config.gameCheatsEnabled[0] == 0);
    assert(M11_QolRuntime_GetSpeedMultiplier() == 100);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.gameSpeed[1] == 2);
    assert(config.gameSpeedMultiplier == 150);
    assert(M11_QolRuntime_GetSpeedMultiplier() == 150);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);

    /* Every non-DM1 runtime source owns the same modal F10 contract.  Keep
     * this as a source-kind matrix so a new game cannot accidentally inherit
     * the popup only through the DM1 built-in catalog path.  The clicks use
     * the real popup coordinates and therefore cover the mouse path too. */
    {
        static const M11_GameSourceKind otherSources[] = {
            M11_GAME_SOURCE_CSB_BOOT,
            M11_GAME_SOURCE_DM2_BOOT,
            M11_GAME_SOURCE_THERON_TRACK02,
            M11_GAME_SOURCE_NEXUS_DGN
        };
        static const int otherSlots[] = { 1, 2, 4, 3 };
        size_t sourceIndex;
        config.gameCheatsEnabled[1] = 0;
        config.gameCheatsEnabled[2] = 0;
        config.gameCheatsEnabled[3] = 0;
        config.gameCheatsEnabled[4] = 0;
        config.gameSpeed[1] = 1;
        config.gameSpeed[2] = 1;
        config.gameSpeed[3] = 1;
        config.gameSpeed[4] = 1;
        assert(M12_Config_Save(&config) == 1);
        for (sourceIndex = 0;
             sourceIndex < sizeof(otherSources) / sizeof(otherSources[0]);
             ++sourceIndex) {
            int slot = otherSlots[sourceIndex];
            state.sourceKind = otherSources[sourceIndex];
            state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
            state.graphicsPopupActive = 0;
            state.graphicsPopupPage = 0;
            state.graphicsPopupSelectedRow = 0;

            result = M11_GameView_HandleInput(
                &state, M12_MENU_INPUT_GRAPHICS_POPUP);
            assert(result == M11_GAME_INPUT_REDRAW);
            assert(state.graphicsPopupActive == 1);
            result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
            assert(result == M11_GAME_INPUT_REDRAW);
            assert(state.presentationMode == M12_PRESENTATION_V21_UPSCALED);

            /* Click CH, then click the selected CHEATS and SPEED rows. */
            result = M11_GameView_HandlePointerButton(
                &state, 286, 31, DM1_V1_MOUSE_MASK_LEFT_PC34);
            assert(result == M11_GAME_INPUT_REDRAW);
            assert(state.graphicsPopupPage == 3);
            result = M11_GameView_HandlePointerButton(
                &state, 180, 48, DM1_V1_MOUSE_MASK_LEFT_PC34);
            assert(result == M11_GAME_INPUT_REDRAW);
            assert(M12_Config_Load(&config, NULL) == 1);
            assert(config.gameCheatsEnabled[slot] == 1);
            result = M11_GameView_HandlePointerButton(
                &state, 180, 58, DM1_V1_MOUSE_MASK_LEFT_PC34);
            assert(result == M11_GAME_INPUT_REDRAW);
            result = M11_GameView_HandlePointerButton(
                &state, 180, 58, DM1_V1_MOUSE_MASK_LEFT_PC34);
            assert(result == M11_GAME_INPUT_REDRAW);
            assert(M12_Config_Load(&config, NULL) == 1);
            assert(config.gameSpeed[slot] == 2);
            assert(M11_QolRuntime_GetSpeedMultiplier() == 150);
            result = M11_GameView_HandlePointerButton(
                &state, 300, 20, DM1_V1_MOUSE_MASK_LEFT_PC34);
            assert(result == M11_GAME_INPUT_REDRAW);
            assert(state.graphicsPopupActive == 0);
        }
    }

    /* Source-specific filter ownership: Theron must use its own admitted
     * settings bridge, while DM2/Nexus must not silently mutate DM1 values
     * through a generic host filter path. */
    config.dm1V2CrtScanlinesEnabled = 0;
    config.theronV2CrtScanlinesEnabled = 0;
    config.theronV2ScalePercent = 200;
    assert(M12_Config_Save(&config) == 1);
    state.sourceKind = M11_GAME_SOURCE_THERON_TRACK02;
    state.presentationMode = M12_PRESENTATION_V20_FILTERED;
    state.graphicsPopupActive = 0;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.theronV2CrtScanlinesEnabled == 1);
    assert(config.dm1V2CrtScanlinesEnabled == 0);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.theronV2ScalePercent == 400);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    assert(result == M11_GAME_INPUT_REDRAW);

    config.dm1V2CrtScanlinesEnabled = 0;
    state.sourceKind = M11_GAME_SOURCE_DM2_BOOT;
    state.presentationMode = M12_PRESENTATION_V20_FILTERED;
    state.graphicsPopupActive = 0;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_GRAPHICS_POPUP);
    assert(result == M11_GAME_INPUT_REDRAW);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_CYCLE_CHAMPION);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupPage == 1);
    result = M11_GameView_HandlePointerButton(&state, 180, 48,
                                               DM1_V1_MOUSE_MASK_LEFT_PC34);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(M12_Config_Load(&config, NULL) == 1);
    assert(config.dm1V2CrtScanlinesEnabled == 0);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    assert(result == M11_GAME_INPUT_REDRAW);

    puts("m11 runtime graphics popup: ok");
    return 0;
}
