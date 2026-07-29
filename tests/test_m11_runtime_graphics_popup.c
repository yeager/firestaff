#include "m11_game_view.h"
#include "config_m12.h"
#include "render_sdl_m11.h"
#include "csb_v22_finished_art_material_gate_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
#include "csb_v2_filter_config_pc34.h"
#include "csb_v2_texture_upscale_pc34.h"
#include "fs_portable_compat.h"

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

static void write_csb_v22_finished_fixture(const char* dataDir) {
    static const struct {
        const char* id;
        const char* category;
        int width;
        int height;
    } slots[] = {
        { "wall_dungeon_d0_01", "wall_shapes", 96, 96 },
        { "wall_dungeon_d1_01", "wall_shapes", 96, 96 },
        { "wall_dungeon_d2_01", "wall_shapes", 96, 96 },
        { "door_d0_01", "door_shapes", 64, 96 },
        { "door_d1_01", "door_shapes", 64, 96 },
        { "door_d2_01", "door_shapes", 64, 96 },
        { "floor_plain_d0_01", "floor_shapes", 96, 96 },
        { "floor_plain_d1_01", "floor_shapes", 96, 96 },
        { "floor_plain_d2_01", "floor_shapes", 96, 96 },
        { "floor_cracked_d0_01", "floor_shapes", 96, 96 },
        { "floor_cracked_d1_01", "floor_shapes", 96, 96 },
        { "floor_cracked_d2_01", "floor_shapes", 96, 96 },
        { "floor_mossy_d0_01", "floor_shapes", 96, 96 },
        { "floor_mossy_d1_01", "floor_shapes", 96, 96 },
        { "floor_mossy_d2_01", "floor_shapes", 96, 96 },
        { "floor_pit_01", "floor_shapes", 96, 96 },
        { "floor_stairs_up_01", "floor_shapes", 96, 96 },
        { "floor_stairs_down_01", "floor_shapes", 96, 96 },
        { "ceiling_01", "wall_shapes", 96, 96 },
        { "creature_demon_d0_01", "creature_shapes", 64, 64 },
        { "creature_demon_d1_01", "creature_shapes", 64, 64 },
        { "creature_demon_d2_01", "creature_shapes", 64, 64 },
        { "prison_door_01", "wall_shapes", 64, 96 },
        { "lord_order_01", "wall_shapes", 96, 96 },
        { "chaos_rune_0_01", "chaos_runes", 32, 32 },
        { "chaos_rune_1_01", "chaos_runes", 32, 32 },
        { "chaos_rune_2_01", "chaos_runes", 32, 32 },
        { "chaos_rune_3_01", "chaos_runes", 32, 32 },
        { "dsa_scroll_01", "dsa_scrolls", 32, 32 }
    };
    char root[FSP_PATH_MAX];
    char parent[FSP_PATH_MAX];
    char modern[FSP_PATH_MAX];
    char manifest[FSP_PATH_MAX];
    const char* slash;
    FILE* fp;

    snprintf(root, sizeof(root), "%s", dataDir);
    slash = strrchr(root, '/');
    assert(slash != NULL);
    root[slash - root] = '\0';             /* .../data */
    snprintf(parent, sizeof(parent), "%s", root);
    slash = strrchr(parent, '/');
    assert(slash != NULL);
    parent[slash - parent] = '\0';         /* fixture root */
    snprintf(modern, sizeof(modern), "%s/assets/csb/modern", parent);
    assert(FSP_CreateDirectoryRecursive(modern) == 1);
    snprintf(manifest, sizeof(manifest), "%s/modern_asset_manifest.json", modern);
    fp = fopen(manifest, "wb");
    assert(fp != NULL);
    static const char* const categories[] = {
        "wall_shapes", "floor_shapes", "creature_shapes", "door_shapes",
        "chaos_runes", "dsa_scrolls"
    };
    fputs("{\"manifestVersion\":\"1.0.0\"", fp);
    for (size_t categoryIndex = 0;
         categoryIndex < sizeof(categories) / sizeof(categories[0]);
         ++categoryIndex) {
        int first = 1;
        fprintf(fp, ",\"%s\":[", categories[categoryIndex]);
        for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); ++i) {
            if (strcmp(slots[i].category, categories[categoryIndex]) != 0) continue;
            {
                char category[FSP_PATH_MAX];
                char asset[FSP_PATH_MAX];
                FILE* assetFile;
                snprintf(category, sizeof(category), "%s/%s", modern,
                         slots[i].category);
                assert(FSP_CreateDirectoryRecursive(category) == 1);
                snprintf(asset, sizeof(asset), "%s/%s.png", category,
                         slots[i].id);
                assetFile = fopen(asset, "wb");
                assert(assetFile != NULL);
                fputs("source-derived-fixture", assetFile);
                fclose(assetFile);
                fprintf(fp, "%s{\"id\":\"%s\",\"generator\":\"source_export\","
                            "\"source_file\":\"%s.png\",\"width\":%d,\"height\":%d}",
                        first ? "" : ",", slots[i].id, slots[i].id,
                        slots[i].width, slots[i].height);
                first = 0;
            }
        }
        fputs("]", fp);
    }
    fputs("}", fp);
    fclose(fp);
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

    /* A complete CSB-specific material manifest must expose V2.2 even when
     * M12 has no global (DM1) artpack-installed bit. */
    {
        const char* dataDir = "/tmp/firestaff-csb-v22-popup/data/csb";
        assert(FSP_CreateDirectoryRecursive(dataDir) == 1);
        write_csb_v22_finished_fixture(dataDir);
        csb_v22_famg_set_manifest_path(dataDir);
        csb_v22_set_manifest_path(dataDir);
        assert(csb_v22_famg_is_finished_real() == 1);
        assert(csb_v22_modern_assets_available() == 1);
        config.v22_modern_assets_installed = 0;
        config.graphicsIndex = M12_PRESENTATION_V21_UPSCALED;
        assert(M12_Config_Save(&config) == 1);
        state.presentationMode = M12_PRESENTATION_V21_UPSCALED;
        result = M11_GameView_HandleInput(&state,
                                          M12_MENU_INPUT_GRAPHICS_POPUP);
        assert(result == M11_GAME_INPUT_REDRAW);
        result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_RIGHT);
        assert(result == M11_GAME_INPUT_REDRAW);
        assert(state.presentationMode == M12_PRESENTATION_V22_MODERN);
        result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
        assert(result == M11_GAME_INPUT_REDRAW);
        assert(state.graphicsPopupActive == 0);
    }

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
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    assert(result == M11_GAME_INPUT_REDRAW);
    assert(state.graphicsPopupActive == 0);

    puts("m11 runtime graphics popup: ok");
    return 0;
}
