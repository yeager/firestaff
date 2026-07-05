/*
 * main_loop_m11.c — M11 Phase A stub.
 *
 * Opens a window via render_sdl_m11, presents a black framebuffer, pumps
 * events until either the user quits or the configured duration elapses,
 * then shuts down.
 */

#include "main_loop_m11.h"
#include "config_m12.h"

#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"
#include "menu_hit_m12.h"
#include "m11_game_view.h"
#include "firestaff_po_loader.h"
#include "firestaff_accessibility.h"
#include "audio_sdl_m11.h"
#include "render_sdl_m11.h"
#include "m11_qol_runtime.h"
#include "dm1_v1_minimap_pc34_compat.h"
#include "dm1_v1_automap_pc34_compat.h"
#include "dm1_v1_combat_log_pc34_compat.h"
#include "title_frontend_v1.h"
#include "asset_status_m12.h"
#include "fs_portable_compat.h"
#include "dm1_v1_vblank_timing.h"
#include "entrance_frontend_pc34_compat.h"
#include "entrance_mouse_routes_pc34_compat.h"
#include "csb_v1_keyboard_commands_pc34_compat.h"
#include "input_remap_m11.h"
#include "gamepad_config_m12.h"
#include "vga_palette_pc34_compat.h"
#include "swsh_frontend_pc34_compat.h"
#include "screenshot_m11.h"
#include "swsh_intro_pathfinder_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#define SDLK_A SDLK_a
#define SDLK_C SDLK_c
#define SDLK_D SDLK_d
#define SDLK_E SDLK_e
#define SDLK_G SDLK_g
#define SDLK_I SDLK_i
#define SDLK_M SDLK_m
#define SDLK_P SDLK_p
#define SDLK_Q SDLK_q
#define SDLK_R SDLK_r
#define SDLK_S SDLK_s
#define SDLK_U SDLK_u
#define SDLK_V SDLK_v
#define SDLK_W SDLK_w
#define SDLK_X SDLK_x
#endif

enum {
    M11_LAUNCHER_FB_WIDTH = 480,
    M11_LAUNCHER_FB_HEIGHT = 270,
    M11_LAUNCHER_MODERN_WIDTH = M12_MODERN_MENU_NATIVE_WIDTH,
    M11_LAUNCHER_MODERN_HEIGHT = M12_MODERN_MENU_NATIVE_HEIGHT
};

/* Runtime switch: when the environment variable FIRESTAFF_LEGACY_MENU
 * is set to a non-zero value we fall back to the original
 * palette-indexed startup menu renderer. This keeps a safe escape hatch
 * for anyone who depends on the legacy 480x270 output. */
static int m11_legacy_menu_requested(void) {
    const char* val = getenv("FIRESTAFF_LEGACY_MENU");
    if (!val || val[0] == '\0') return 0;
    if (val[0] == '0' && val[1] == '\0') return 0;
    return 1;
}

static int m11_should_use_modern_launcher(const M12_StartupMenuState* menuState) {
    if (m11_legacy_menu_requested()) {
        return 0;
    }
    /* The startup menu is Firestaff's shared product front door for every
     * presentation mode, including V1 original.  V1 parity begins after the
     * user launches a game: TITLE/entrance/Hall-of-Champions sequencing must
     * not force the launcher itself back to the old sparse indexed renderer.
     * FIRESTAFF_LEGACY_MENU remains the explicit escape hatch. */
    return menuState != NULL;
}

static void m11_draw_launcher_legacy(const M12_StartupMenuState* menuState,
                                     unsigned char* launcherFramebuffer) {
    if (!menuState || !launcherFramebuffer) {
        return;
    }
    M12_StartupMenu_Draw(menuState,
                         launcherFramebuffer,
                         M11_LAUNCHER_FB_WIDTH,
                         M11_LAUNCHER_FB_HEIGHT);
}

static void m11_draw_launcher_modern(const M12_StartupMenuState* menuState,
                                     unsigned char* modernRgba) {
    if (!menuState || !modernRgba) {
        return;
    }
    M12_ModernMenu_Render(menuState,
                          modernRgba,
                          M11_LAUNCHER_MODERN_WIDTH,
                          M11_LAUNCHER_MODERN_HEIGHT);
}

static void m11_draw_launcher(const M12_StartupMenuState* menuState,
                              unsigned char* launcherFramebuffer,
                              unsigned char* modernRgba,
                              int useModern) {
    if (useModern && modernRgba) {
        m11_draw_launcher_modern(menuState, modernRgba);
    } else if (launcherFramebuffer) {
        m11_draw_launcher_legacy(menuState, launcherFramebuffer);
    }
}

static int m11_present_launcher(unsigned char* launcherFramebuffer,
                                unsigned char* modernRgba,
                                int useModern) {
    if (useModern && modernRgba) {
        return M11_Render_PresentRGBA(modernRgba,
                                      M11_LAUNCHER_MODERN_WIDTH,
                                      M11_LAUNCHER_MODERN_HEIGHT);
    }
    return M11_Render_PresentIndexed(launcherFramebuffer,
                                     M11_LAUNCHER_FB_WIDTH,
                                     M11_LAUNCHER_FB_HEIGHT);
}

int M11_GameView_PresentationIndexedScale(int presentationMode) {
    return presentationMode == M12_PRESENTATION_V20_FILTERED ? 2 : 1;
}

int M11_GameView_PresentationTarget(int presentationMode,
                                    int presentationWidth,
                                    int presentationHeight,
                                    int* outW,
                                    int* outH) {
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;
    if (presentationMode == M12_PRESENTATION_V20_FILTERED) {
        targetW = M11_FB_WIDTH * 2;
        targetH = M11_FB_HEIGHT * 2;
    } else if (M12_PresentationMode_AllowsResolutionChoice(presentationMode) &&
               presentationWidth > 0 &&
               presentationHeight > 0) {
        targetW = presentationWidth;
        targetH = presentationHeight;
    }
    if (outW) {
        *outW = targetW;
    }
    if (outH) {
        *outH = targetH;
    }
    return targetW != M11_FB_WIDTH || targetH != M11_FB_HEIGHT;
}

int M11_ResolveGameScaleFilterForPresentation(int presentationMode,
                                              int requestedScaleFilter) {
    if (presentationMode == M12_PRESENTATION_V1_ORIGINAL ||
        presentationMode == M12_PRESENTATION_V20_FILTERED) {
        return M11_SCALE_FILTER_NEAREST;
    }
    if (requestedScaleFilter != M11_SCALE_FILTER_NEAREST &&
        requestedScaleFilter != M11_SCALE_FILTER_LINEAR) {
        return M11_SCALE_FILTER_NEAREST;
    }
    return requestedScaleFilter;
}

static void m11_map_presented_game_point_to_source(const M11_GameViewState* gameView,
                                                   int* x,
                                                   int* y) {
    if (!gameView) {
        return;
    }
    (void)M11_MapPresentedGamePointToSourceForPresentation(gameView->presentationMode,
                                                           gameView->presentationWidth,
                                                           gameView->presentationHeight,
                                                           x,
                                                           y);
}

static int m11_present_game_frame(const M11_GameViewState* gameView) {
    int scale = M11_GameView_PresentationIndexedScale(
        gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL);
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;
    int requestedFilter = M11_Render_GetScaleFilter();
    int effectiveFilter = M11_ResolveGameScaleFilterForPresentation(
        gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
        requestedFilter);
    int restoreFilter = 0;
    int result;
    /* ReDMCSB DUNVIEW.C:3619-3638 draws DM1 inscriptions as hard-edged
     * M648 8x8 glyphs into the 320x200 viewport.  If the launcher's global
     * scaling filter is LINEAR, SDL smooths those glyphs during window
     * presentation and the wall text becomes unreadable.  V1 original mode
     * therefore presents with nearest-neighbor regardless of the enhanced-mode
     * filter setting.  V2.0 still shows the same source glyphs through the
     * 320x200 -> 640x400 indexed path, so it also keeps nearest presentation;
     * V2.1/V2.2 keep honoring the user setting. */
    if (effectiveFilter != requestedFilter) {
        M11_Render_SetScaleFilter(effectiveFilter);
        restoreFilter = 1;
    }
    if (scale > 1) {
        result = M11_Render_PresentScaledIndexed(M11_Render_GetFramebuffer(),
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 scale);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        return result;
    }
    if (M11_GameView_PresentationTarget(gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
                                         gameView ? gameView->presentationWidth : 0,
                                         gameView ? gameView->presentationHeight : 0,
                                         &targetW,
                                         &targetH)) {
        result = M11_Render_PresentIndexedToResolution(M11_Render_GetFramebuffer(),
                                                       M11_FB_WIDTH,
                                                       M11_FB_HEIGHT,
                                                       targetW,
                                                       targetH);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        return result;
    }
    result = M11_Render_Present();
    if (restoreFilter) {
        M11_Render_SetScaleFilter(requestedFilter);
    }
    return result;
}

void M11_ApplyStartupMenuRuntime(M12_StartupMenuState* menuState) {
    int requestedWindowMode;
    int actualWindowMode;
    if (!menuState) {
        return;
    }
    requestedWindowMode = menuState->settings.windowModeIndex;
    actualWindowMode = M11_Render_SyncWindowModeFromWindow();
    /* If the user maximized the OS window directly while the saved setting
     * still says WINDOWED, do not shrink the window just because the launcher
     * is switching between menu/game/settings surfaces. Keep the observed
     * maximized/fullscreen mode as the runtime setting outside the settings
     * editor; inside settings, an explicit WINDOWED selection must still be
     * able to restore the window. */
    if (menuState->view != M12_MENU_VIEW_SETTINGS &&
        requestedWindowMode == M11_WINDOW_MODE_WINDOWED &&
        (actualWindowMode == M11_WINDOW_MODE_MAXIMIZED ||
         actualWindowMode == M11_WINDOW_MODE_FULLSCREEN)) {
        requestedWindowMode = actualWindowMode;
        menuState->settings.windowModeIndex = actualWindowMode;
    }
    if (M11_Render_GetPaletteLevel() != M12_StartupMenu_GetRenderPaletteLevel(menuState)) {
        M11_Render_SetPaletteLevel(M12_StartupMenu_GetRenderPaletteLevel(menuState));
    }
    if (M11_Render_GetWindowMode() != requestedWindowMode) {
        M11_Render_SetWindowMode(requestedWindowMode);
    }
    if (M11_Render_GetScaleMode() != menuState->settings.scaleModeIndex) {
        M11_Render_SetScaleMode(menuState->settings.scaleModeIndex);
    }
    if (M11_Render_GetDisplayAspectMode() != menuState->settings.displayAspectMode) {
        M11_Render_SetDisplayAspectMode(menuState->settings.displayAspectMode);
    }
    if (M11_Render_GetIntegerScaling() != menuState->settings.integerScaling) {
        M11_Render_SetIntegerScaling(menuState->settings.integerScaling);
    }
    if (M11_Render_GetScaleFilter() != menuState->settings.scalingFilterIndex) {
        M11_Render_SetScaleFilter(menuState->settings.scalingFilterIndex);
    }
    if (M11_Render_GetVSync() != menuState->settings.vsyncIndex) {
        M11_Render_SetVSync(menuState->settings.vsyncIndex);
    }
}

static int m11_is_default_window_size(int width, int height) {
    return width == 960 && height == 540;
}

static void m11_apply_persisted_window_size(M11_PhaseA_Options* opts) {
    M12_Config config;
    if (!opts || !m11_is_default_window_size(opts->windowWidth, opts->windowHeight)) {
        return;
    }
    M12_Config_Load(&config, opts->dataDir);
    if (config.windowWidth > 0 && config.windowHeight > 0) {
        opts->windowWidth = config.windowWidth;
        opts->windowHeight = config.windowHeight;
    }
}

static void m11_sync_and_save_window_size(M12_StartupMenuState* menuState) {
    if (!menuState || !M11_Render_IsInitialized()) {
        return;
    }
    menuState->settings.windowWidth = M11_Render_GetWindowWidth();
    menuState->settings.windowHeight = M11_Render_GetWindowHeight();
    M12_StartupMenu_SaveConfig(menuState);
}


static int m11_find_title_dat_for_intro(const M12_StartupMenuState* menuState,
                                        char* outPath,
                                        size_t outPathBytes) {
    const char* envPath;
    const char* dataDir;
    char candidate[FSP_PATH_MAX];
    char parent[FSP_PATH_MAX];
    const M12_AssetVersionStatus* dm1v;
    size_t i;
    static const char* suffixes[] = {
        "TITLE",
        "TITLE.DAT",
        "dm1/TITLE",
        "dm1/TITLE.DAT",
        "DungeonMasterPC34/TITLE",
        "DungeonMasterPC34/TITLE.DAT",
        "DungeonMasterPC34Multilingual/TITLE",
        "DungeonMasterPC34Multilingual/TITLE.DAT",
        "dm-pc34/DungeonMasterPC34/TITLE",
        "dm-pc34/DungeonMasterPC34/TITLE.DAT",
        "dm-pc34/DungeonMasterPC34Multilingual/TITLE",
        "dm-pc34/DungeonMasterPC34Multilingual/TITLE.DAT"
    };
    static const char* homeSuffixes[] = {
        ".firestaff/data/TITLE",
        ".firestaff/data/dm1/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34Multilingual/TITLE"
    };
    char titleErr[160];

    if (!outPath || outPathBytes == 0U) {
        return 0;
    }
    outPath[0] = '\0';
    titleErr[0] = '\0';

    envPath = getenv("FIRESTAFF_TITLE_DAT");
    if (envPath && envPath[0] != '\0' && V1_Title_IsCanonicalPc34Title(envPath, titleErr, sizeof(titleErr))) {
        snprintf(outPath, outPathBytes, "%s", envPath);
        return 1;
    }

    if (menuState) {
        for (i = 0U; i < M12_AssetStatus_GetVersionCount("dm1"); ++i) {
            dm1v = M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1", i);
            if (dm1v && dm1v->matched && FSP_ParentDir(parent, sizeof(parent), dm1v->matchedPath)) {
                if (FSP_JoinPath(candidate, sizeof(candidate), parent, "TITLE") &&
                    V1_Title_IsCanonicalPc34Title(candidate, titleErr, sizeof(titleErr))) {
                    snprintf(outPath, outPathBytes, "%s", candidate);
                    return 1;
                }
                if (FSP_JoinPath(candidate, sizeof(candidate), parent, "TITLE.DAT") &&
                    V1_Title_IsCanonicalPc34Title(candidate, titleErr, sizeof(titleErr))) {
                    snprintf(outPath, outPathBytes, "%s", candidate);
                    return 1;
                }
                /* DM1 PC 3.4: TITLE lives beside DATA/, not inside it.
                 * DUNGEON.DAT is in .../DungeonMasterPC34/DATA/DUNGEON.DAT
                 * TITLE is at    .../DungeonMasterPC34/TITLE
                 * So check the grandparent (parent of DATA/). */
                {
                    char grandparent[FSP_PATH_MAX];
                    if (FSP_ParentDir(grandparent, sizeof(grandparent), parent)) {
                        if (FSP_JoinPath(candidate, sizeof(candidate), grandparent, "TITLE") &&
                            V1_Title_IsCanonicalPc34Title(candidate, titleErr, sizeof(titleErr))) {
                            snprintf(outPath, outPathBytes, "%s", candidate);
                            return 1;
                        }
                        if (FSP_JoinPath(candidate, sizeof(candidate), grandparent, "TITLE.DAT") &&
                            V1_Title_IsCanonicalPc34Title(candidate, titleErr, sizeof(titleErr))) {
                            snprintf(outPath, outPathBytes, "%s", candidate);
                            return 1;
                        }
                    }
                }
            }
        }
    }

    dataDir = menuState ? M12_AssetStatus_GetDataDir(&menuState->assetStatus) : NULL;
    if (!dataDir || dataDir[0] == '\0') {
        dataDir = ".";
    }
    for (i = 0U; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (FSP_JoinPath(candidate, sizeof(candidate), dataDir, suffixes[i]) &&
            V1_Title_IsCanonicalPc34Title(candidate, titleErr, sizeof(titleErr))) {
            snprintf(outPath, outPathBytes, "%s", candidate);
            return 1;
        }
    }

    /* N2/Mac original-data fallback: V1 original mode must not silently skip
     * the ReDMCSB TITLE path just because GRAPHICS.DAT/DUNGEON.DAT were found
     * through the asset catalog while TITLE lives beside the canonical local
     * DM1 anchors.  This mirrors the verified N2 layout and also works on a
     * developer Mac if the same OpenClaw original-data tree is present. */
    dataDir = getenv("HOME");
    if (dataDir && dataDir[0] != '\0') {
        for (i = 0U; i < sizeof(homeSuffixes) / sizeof(homeSuffixes[0]); ++i) {
            if (FSP_JoinPath(candidate, sizeof(candidate), dataDir, homeSuffixes[i]) &&
                V1_Title_IsCanonicalPc34Title(candidate, titleErr, sizeof(titleErr))) {
                snprintf(outPath, outPathBytes, "%s", candidate);
                return 1;
            }
        }
    }
    return 0;
}

static void m11_unpack_title_4bpp_to_indexed(const unsigned char* packed4bpp,
                                             unsigned char* indexed) {
    unsigned int y;
    unsigned int x;
    for (y = 0U; y < (unsigned int)M11_FB_HEIGHT; ++y) {
        const unsigned char* src = packed4bpp + y * 160U;
        unsigned char* dst = indexed + y * (unsigned int)M11_FB_WIDTH;
        for (x = 0U; x < (unsigned int)M11_FB_WIDTH; x += 2U) {
            unsigned char b = src[x >> 1];
            dst[x] = (unsigned char)((b >> 4) & 0x0fU);
            dst[x + 1U] = (unsigned char)(b & 0x0fU);
        }
    }
}

static void m11_fill_rect_indexed(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  int x,
                                  int y,
                                  int w,
                                  int h,
                                  unsigned char color) {
    int yy;
    if (!framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0 || w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > framebufferWidth) w = framebufferWidth - x;
    if (y + h > framebufferHeight) h = framebufferHeight - y;
    if (w <= 0 || h <= 0) return;
    for (yy = 0; yy < h; ++yy) {
        memset(framebuffer + (size_t)(y + yy) * (size_t)framebufferWidth + (size_t)x, color, (size_t)w);
    }
}


static int m11_draw_entrance_screen_asset(M11_GameViewState* gameView,
                                          unsigned char* framebuffer) {
    const M11_AssetSlot* entranceScreen;
    if (!gameView || !framebuffer || !gameView->assetsAvailable) {
        return 0;
    }
    entranceScreen = M11_AssetLoader_Load(&gameView->assetLoader, 4U);
    if (!entranceScreen || entranceScreen->width != 320U || entranceScreen->height != 200U) {
        return 0;
    }
    M11_AssetLoader_Blit(entranceScreen,
                         framebuffer,
                         M11_FB_WIDTH,
                         M11_FB_HEIGHT,
                         0,
                         0,
                         -1);
    return 1;
}


static int m11_draw_entrance_closed_doors_asset(M11_GameViewState* gameView,
                                                unsigned char* framebuffer) {
    const M11_AssetSlot* leftDoor;
    const M11_AssetSlot* rightDoor;
    if (!gameView || !framebuffer || !gameView->assetsAvailable) {
        return 0;
    }
    leftDoor = M11_AssetLoader_Load(&gameView->assetLoader, 2U);
    rightDoor = M11_AssetLoader_Load(&gameView->assetLoader, 3U);
    if (!leftDoor || !rightDoor || leftDoor->height < 161U || rightDoor->height < 161U) {
        return 0;
    }
    /* ReDMCSB DATA.C PC boxes: closed left {0,104,28,188},
     * closed right {105,231,28,188}; ENTRANCE.C:574-579 blits C002/C003
     * over C004 before the command wait / door opening. */
    M11_AssetLoader_BlitRegion(leftDoor, 0, 0, 105, 161,
                               framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                               0, 28, -1);
    M11_AssetLoader_BlitRegion(rightDoor, 0, 0, 127, 161,
                               framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                               105, 28, -1);
    return 1;
}

typedef enum {
    M11_ENTRANCE_COMMAND_QUIT = -1,
    M11_ENTRANCE_COMMAND_NONE = 0,
    M11_ENTRANCE_COMMAND_ENTER = 1,
    M11_ENTRANCE_COMMAND_RESUME = 2,
    M11_ENTRANCE_COMMAND_CREDITS = 3
} M11_EntranceCommand;

static int m11_draw_entrance_credits_asset(M11_GameViewState* gameView,
                                           unsigned char* framebuffer) {
    const M11_AssetSlot* credits;
    if (!gameView || !framebuffer || !gameView->assetsAvailable) {
        return 0;
    }
    credits = M11_AssetLoader_Load(&gameView->assetLoader, 5U);
    if (!credits || credits->width != 320U || credits->height != 200U) {
        return 0;
    }
    M11_AssetLoader_Blit(credits,
                         framebuffer,
                         M11_FB_WIDTH,
                         M11_FB_HEIGHT,
                         0,
                         0,
                         -1);
    return 1;
}

static int m11_wait_for_entrance_credits_done(void) {
    unsigned int ticks;
    SDL_Event ev;
    /* ReDMCSB ENTRANCE.C:1012-1091 F0442 sets L1406=1800, discards stale
     * keyboard input, then waits one delay/vblank per tick until any
     * keyboard or mouse input is present before returning to the entrance
     * command loop. */
    while (SDL_PollEvent(&ev)) {
        (void)ev;
    }
    for (ticks = 0U; ticks < 1800U; ++ticks) {
        while (SDL_PollEvent(&ev)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            if (ev.type == SDL_EVENT_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            if (ev.type == SDL_EVENT_KEY_DOWN ||
                ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                ev.type == SDL_EVENT_FINGER_DOWN) {
                return M11_ENTRANCE_COMMAND_NONE;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#else
            if (ev.type == SDL_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            if (ev.type == SDL_KEYDOWN ||
                ev.type == SDL_MOUSEBUTTONDOWN ||
                ev.type == SDL_FINGERDOWN) {
                return M11_ENTRANCE_COMMAND_NONE;
            }
            if (ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#endif
        }
        SDL_Delay(20);
    }
    return M11_ENTRANCE_COMMAND_NONE;
}

static int m11_show_redmcsb_entrance_credits(M11_GameViewState* gameView,
                                             unsigned char* framebuffer) {
    int waitResult;
    if (!gameView || !framebuffer) return M11_ENTRANCE_COMMAND_NONE;
    if (!m11_draw_entrance_credits_asset(gameView, framebuffer)) {
        memset(framebuffer, 0, (size_t)M11_FB_BYTES);
        m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                              0, 0, M11_FB_WIDTH, M11_FB_HEIGHT, 1);
        m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                              36, 88, 248, 24, 15);
        m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                              40, 92, 240, 16, 0);
    }
    M11_Render_PresentIndexedWithSpecialPalette(framebuffer,
                                                M11_FB_WIDTH,
                                                M11_FB_HEIGHT,
                                                VGA_PALETTE_PC34_SPECIAL_CREDITS);
    waitResult = m11_wait_for_entrance_credits_done();
    return waitResult;
}

static int m11_draw_entrance_opening_doors_asset(M11_GameViewState* gameView,
                                                 unsigned char* framebuffer,
                                                 const unsigned char* dungeonFrame,
                                                 const EntranceCompatDoorStep* door) {
    const M11_AssetSlot* entranceScreen;
    const M11_AssetSlot* leftDoor;
    const M11_AssetSlot* rightDoor;
    EntranceCompatCompositePixels pixels;
    if (!gameView || !framebuffer || !dungeonFrame || !door || !gameView->assetsAvailable) {
        return 0;
    }
    entranceScreen = M11_AssetLoader_Load(&gameView->assetLoader, 4U);
    leftDoor = M11_AssetLoader_Load(&gameView->assetLoader, 2U);
    rightDoor = M11_AssetLoader_Load(&gameView->assetLoader, 3U);
    if (!entranceScreen || !leftDoor || !rightDoor) {
        return 0;
    }
    memset(&pixels, 0, sizeof(pixels));
    pixels.entranceScreen = entranceScreen->pixels;
    pixels.entranceWidth = entranceScreen->width;
    pixels.entranceHeight = entranceScreen->height;
    pixels.dungeonFrame = dungeonFrame;
    pixels.dungeonFrameWidth = M11_FB_WIDTH;
    pixels.dungeonFrameHeight = M11_FB_HEIGHT;
    pixels.leftDoor = leftDoor->pixels;
    pixels.leftDoorWidth = leftDoor->width;
    pixels.leftDoorHeight = leftDoor->height;
    pixels.rightDoor = rightDoor->pixels;
    pixels.rightDoorWidth = rightDoor->width;
    pixels.rightDoorHeight = rightDoor->height;
    return ENTRANCE_Compat_CompositeDoorOpeningFrame(framebuffer,
                                                     M11_FB_WIDTH,
                                                     M11_FB_HEIGHT,
                                                     &pixels,
                                                     door);
}

static void m11_draw_entrance_door_panel(unsigned char* framebuffer,
                                         int x,
                                         int y,
                                         int w,
                                         int h,
                                         unsigned char fill) {
    if (!framebuffer || w <= 0 || h <= 0) return;
    m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, x, y, w, h, fill);
    m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, x, y, w, 1, 13);
    m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, x, y + h - 1, w, 1, 0);
    m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, x, y, 1, h, 13);
    m11_fill_rect_indexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, x + w - 1, y, 1, h, 0);
}

static M11_EntranceCommand m11_wait_for_redmcsb_entrance_command(int autoEnterAfterMs);

int M11_Entrance_DispatchSourceLockedPointerCommand(int framebufferX,
                                                    int framebufferY,
                                                    unsigned int buttonMask) {
    EntranceMouseRouteCompat route;
    if (!ENTRANCE_Compat_HitTestMouseRoute(framebufferX, framebufferY, buttonMask, &route)) {
        return M11_ENTRANCE_RUNTIME_COMMAND_NONE;
    }
    return (int)route.commandId;
}

int M11_Entrance_DispatchSourceLockedKeyCommand(int keyCode) {
    /* ReDMCSB ENTRANCE.C:850-883 PC/F20 path checks raw keyboard input in the
     * entrance wait loop and maps carriage return to C001_MODE_LOAD_DUNGEON.
     * Keep Space inert because the PC/F20 source path does not activate the
     * entrance with Space. */
    switch (keyCode) {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        return M11_ENTRANCE_RUNTIME_COMMAND_ENTER_DUNGEON;
    case SDLK_ESCAPE:
    case SDLK_Q:
        return M11_ENTRANCE_RUNTIME_COMMAND_QUIT;
    default:
        return M11_ENTRANCE_RUNTIME_COMMAND_NONE;
    }
}

int M11_Entrance_ResolveDm1ResumeSavePath(const char* sourceId,
                                          int quickResumeAvailable,
                                          const char* quickResumeGameId,
                                          const char* quickResumeSavePath,
                                          char* outPath,
                                          size_t outPathBytes) {
    const char* sid;
    int rc;
    if (!outPath || outPathBytes == 0U) {
        return 0;
    }
    outPath[0] = '\0';
    sid = (sourceId && sourceId[0] != '\0') ? sourceId : "dm1";
    if (quickResumeAvailable &&
        quickResumeGameId && strcmp(quickResumeGameId, "dm1") == 0 &&
        quickResumeSavePath && quickResumeSavePath[0] != '\0') {
        rc = snprintf(outPath, outPathBytes, "%s", quickResumeSavePath);
        return rc > 0 && rc < (int)outPathBytes;
    }
    /* ReDMCSB COMMAND.C M566 enters saved-game load from the entrance.  This
     * fallback preserves Firestaff's pre-existing source-id quicksave name
     * for users who have no launcher-resolved quick resume path. */
    rc = snprintf(outPath, outPathBytes, "firestaff-%s-dm1save.sav", sid);
    return rc > 0 && rc < (int)outPathBytes;
}

static M11_EntranceCommand m11_entrance_command_path_from_source_command(int commandId) {
    switch (commandId) {
    case M11_ENTRANCE_RUNTIME_COMMAND_ENTER_DUNGEON:
    case M11_ENTRANCE_RUNTIME_COMMAND_ENTER_BONUS_DUNGEON:
        return M11_ENTRANCE_COMMAND_ENTER;
    case M11_ENTRANCE_RUNTIME_COMMAND_RESUME:
        return M11_ENTRANCE_COMMAND_RESUME;
    case M11_ENTRANCE_RUNTIME_COMMAND_DRAW_CREDITS:
        return M11_ENTRANCE_COMMAND_CREDITS;
    case M11_ENTRANCE_RUNTIME_COMMAND_QUIT:
        return M11_ENTRANCE_COMMAND_QUIT;
    default:
        return M11_ENTRANCE_COMMAND_NONE;
    }
}

int M11_Entrance_ShouldAutoEnterForTimeout(int allowHeadlessTimeout,
                                           int autoEnterAfterMs,
                                           uint64_t elapsedMs) {
    if (!allowHeadlessTimeout) {
        return 0;
    }
    if (autoEnterAfterMs > 0 && elapsedMs > (uint64_t)autoEnterAfterMs) {
        return 1;
    }
    return elapsedMs > 5000U;
}

static int m11_play_redmcsb_entrance_transition(M11_GameViewState* gameView, int autoEnterAfterMs) {
    unsigned char* framebuffer;
    unsigned char* dungeonFrame;
    unsigned int sourceStep;
    if (!gameView || !gameView->active) return 0;
    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) return 0;
    dungeonFrame = (unsigned char*)malloc((size_t)M11_FB_BYTES);
    if (!dungeonFrame) return 0;

    M11_GameView_Draw(gameView, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    memcpy(dungeonFrame, framebuffer, (size_t)M11_FB_BYTES);

    /* Pre-fill framebuffer with the entrance screen (C004 + closed doors)
     * before the loop begins.  Source-lock: ENTRANCE.C draws the entrance
     * screen and doors *before* entering the VBlank wait loop (lines 446-
     * 579).  Without this pre-fill the first PresentIndexed call (inside
     * the loop, sourceStep==1, DRAW_MICRO_DUNGEON → else branch) would
     * present the uninitialized dungeon viewport as the first visible
     * frame — an ugly flash between the TITLE intro and the entrance.
     * The loop's first event (DRAW_MICRO_DUNGEON) falls through to the
     * else branch and memcpy's the dungeon frame, but the pre-draw above
     * ensures that if the first Present is called before any loop body
     * executes, the screen shows the entrance screen, never the dungeon.
     * This also mirrors the ReDMCSB source order: draw C004+doors first,
     * then micro-dungeon, then fade/curtain, then wait-for-input loop.
     */
    if (m11_draw_entrance_screen_asset(gameView, framebuffer)) {
        (void)m11_draw_entrance_closed_doors_asset(gameView, framebuffer);
    } else {
        /* Palette-fill fallback: draw the entrance screen background
         * and closed door panels so the first present is not a dungeon
         * viewport. */
        memset(framebuffer, 0, (size_t)M11_FB_BYTES);
        m11_draw_entrance_door_panel(framebuffer, 0, 28, 101, 161, 5);
        m11_draw_entrance_door_panel(framebuffer, 109, 28, 123, 161, 5);
    }

    /* ReDMCSB ENTRANCE.C source-lock:
     * - F0441_STARTEND_ProcessEntrance() waits in entrance mode until C200.
     * - ENTRANCE.C:935 delays 20 ticks before opening.
     * - F0438_STARTEND_OpenEntranceDoors() runs 31 one-VBlank steps.
     * - ENTRANCE.C:149-231 moves the left/right door boxes by 4px/step
     *   from DATA.C source boxes left {0,100,0,160}, right {109,231,0,160}.
     * This runtime transition uses the source schedule/boxes here; C004 and
     * C002/C003 are blitted from GRAPHICS.DAT when available, with palette-fill
     * fallback preserving timing/geometry if assets are missing. */
    for (sourceStep = 1U; sourceStep <= ENTRANCE_Compat_GetSourceAnimationStepCount(); ++sourceStep) {
        EntranceCompatSourceAnimationStep step;
        if (!ENTRANCE_Compat_GetSourceAnimationStep(sourceStep, &step)) break;

        if (step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK) {
            memset(framebuffer, 0, (size_t)M11_FB_BYTES);
        } else if (step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN ||
                   step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT ||
                   step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND ||
                   step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY) {
            if (m11_draw_entrance_screen_asset(gameView, framebuffer)) {
                (void)m11_draw_entrance_closed_doors_asset(gameView, framebuffer);
            } else {
                memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
                m11_draw_entrance_door_panel(framebuffer, 0, 28, 101, 161, 5);
                m11_draw_entrance_door_panel(framebuffer, 109, 28, 123, 161, 5);
            }
        } else if (step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP) {
            EntranceCompatDoorStep door;
            if (ENTRANCE_Compat_GetDoorAnimationStep(sourceStep - 6U, &door)) {
                if (!m11_draw_entrance_opening_doors_asset(gameView, framebuffer, dungeonFrame, &door)) {
                    memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
                    if (door.leftBoxW > 0U) {
                        m11_draw_entrance_door_panel(framebuffer,
                                                     (int)door.leftBoxX,
                                                     28 + (int)door.leftBoxY,
                                                     (int)door.leftBoxW,
                                                     (int)door.leftBoxH,
                                                     5);
                    }
                    if (door.rightBoxW > 0U) {
                        m11_draw_entrance_door_panel(framebuffer,
                                                     (int)door.rightBoxX,
                                                     28 + (int)door.rightBoxY,
                                                     (int)door.rightBoxW,
                                                     (int)door.rightBoxH,
                                                     5);
                    }
                }
            }
        } else {
            memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
        }

        M11_Render_PresentIndexedWithSpecialPalette(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT, VGA_PALETTE_PC34_SPECIAL_ENTRANCE);
        if (step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT) {
            M11_EntranceCommand cmd = m11_wait_for_redmcsb_entrance_command(autoEnterAfterMs);
            if (cmd == M11_ENTRANCE_COMMAND_QUIT) {
                free(dungeonFrame);
                return M11_ENTRANCE_COMMAND_QUIT;
            }
            if (cmd == M11_ENTRANCE_COMMAND_RESUME) {
                free(dungeonFrame);
                return M11_ENTRANCE_COMMAND_RESUME;
            }
            if (cmd == M11_ENTRANCE_COMMAND_CREDITS) {
                int creditsResult =
                    m11_show_redmcsb_entrance_credits(gameView, framebuffer);
                if (creditsResult == M11_ENTRANCE_COMMAND_QUIT) {
                    free(dungeonFrame);
                    return M11_ENTRANCE_COMMAND_QUIT;
                }
                sourceStep = 0U;
                continue;
            }
        }
        {
            unsigned int delayMs = ENTRANCE_Compat_GetRuntimeDelayMs(&step);
            if (delayMs > 0U) {
                SDL_Delay(delayMs);
            }
        }
        if (M11_Render_PumpEvents()) break;
    }
    memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
    M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    free(dungeonFrame);
    return 1;
}

static M11_EntranceCommand m11_entrance_route_framebuffer_pointer(int fbX,
                                                                  int fbY,
                                                                  unsigned int buttonMask) {
    return m11_entrance_command_path_from_source_command(
        M11_Entrance_DispatchSourceLockedPointerCommand(fbX, fbY, buttonMask));
}

static M11_EntranceCommand m11_entrance_route_window_pointer(int windowX,
                                                            int windowY,
                                                            unsigned int buttonMask) {
    int fbX = 0;
    int fbY = 0;
    if (!M11_Render_MapWindowToFramebuffer(windowX, windowY, &fbX, &fbY)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    return m11_entrance_route_framebuffer_pointer(fbX, fbY, buttonMask);
}

static M11_EntranceCommand m11_entrance_route_normalized_touch(float normalizedX,
                                                              float normalizedY) {
    int windowW = M11_Render_GetWindowWidth();
    int windowH = M11_Render_GetWindowHeight();
    int windowX;
    int windowY;
    if (windowW <= 0 || windowH <= 0) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    if (normalizedX < 0.0f || normalizedX > 1.0f ||
        normalizedY < 0.0f || normalizedY > 1.0f) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    windowX = (int)(normalizedX * (float)windowW);
    windowY = (int)(normalizedY * (float)windowH);
    if (windowX >= windowW) windowX = windowW - 1;
    if (windowY >= windowH) windowY = windowH - 1;
    return m11_entrance_route_window_pointer(windowX,
                                             windowY,
                                             ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT);
}

static M11_EntranceCommand m11_wait_for_redmcsb_entrance_command(int autoEnterAfterMs) {
    /* ReDMCSB ENTRANCE.C:850-883 redraws the entrance, discards previous
     * input, then waits in the entrance command loop until a fresh command
     * changes G0298_B_NewGame away from C099_MODE_WAITING_ON_ENTRANCE.
     * Do the same at the SDL boundary: drain the launch key/button that got
     * us here, then require a new Enter/Space/click before the doors open. */
    Uint64 started;
    int allowHeadlessTimeout = 0;
    int drained = 0;
    SDL_Event ev;
    const char* videoDriver = getenv("SDL_VIDEODRIVER");
    if ((videoDriver && strcmp(videoDriver, "dummy") == 0) || getenv("FIRESTAFF_AUTOTEST")) {
        allowHeadlessTimeout = 1;
    }

    while (SDL_PollEvent(&ev)) {
        drained += 1;
    }
    (void)drained;
    started = SDL_GetTicks();

    for (;;) {
        while (SDL_PollEvent(&ev)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            if (ev.type == SDL_EVENT_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            /* ReDMCSB ENTRANCE.C:850-883 accepts a fresh Return while waiting
             * at the entrance; Space remains inert on the PC/F20 path. */
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                M11_EntranceCommand keyCommand =
                    m11_entrance_command_path_from_source_command(
                        M11_Entrance_DispatchSourceLockedKeyCommand((int)ev.key.key));
                if (keyCommand != M11_ENTRANCE_COMMAND_NONE) return keyCommand;
            }
            if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (ev.button.button != SDL_BUTTON_LEFT) continue;
                {
                    M11_EntranceCommand pointerCommand =
                        m11_entrance_route_window_pointer((int)ev.button.x,
                                                          (int)ev.button.y,
                                                          ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT);
                    if (pointerCommand != M11_ENTRANCE_COMMAND_NONE) return pointerCommand;
                }
                continue;
            }
            if (ev.type == SDL_EVENT_FINGER_DOWN) {
                M11_EntranceCommand touchCommand =
                    m11_entrance_route_normalized_touch(ev.tfinger.x, ev.tfinger.y);
                if (touchCommand != M11_ENTRANCE_COMMAND_NONE) return touchCommand;
                continue;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#else
            if (ev.type == SDL_QUIT) return M11_ENTRANCE_COMMAND_QUIT;
            /* ReDMCSB ENTRANCE.C:850-883 accepts a fresh Return while waiting
             * at the entrance; Space remains inert on the PC/F20 path. */
            if (ev.type == SDL_KEYDOWN) {
                M11_EntranceCommand keyCommand =
                    m11_entrance_command_path_from_source_command(
                        M11_Entrance_DispatchSourceLockedKeyCommand((int)ev.key.keysym.sym));
                if (keyCommand != M11_ENTRANCE_COMMAND_NONE) return keyCommand;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                if (ev.button.button != SDL_BUTTON_LEFT) continue;
                {
                    M11_EntranceCommand pointerCommand =
                        m11_entrance_route_window_pointer(ev.button.x,
                                                          ev.button.y,
                                                          ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT);
                    if (pointerCommand != M11_ENTRANCE_COMMAND_NONE) return pointerCommand;
                }
                continue;
            }
            if (ev.type == SDL_FINGERDOWN) {
                M11_EntranceCommand touchCommand =
                    m11_entrance_route_normalized_touch(ev.tfinger.x, ev.tfinger.y);
                if (touchCommand != M11_ENTRANCE_COMMAND_NONE) return touchCommand;
                continue;
            }
            if (ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#endif
        }

        /* Scripted/headless probes cannot send a second physical command.
         * ReDMCSB ENTRANCE.C waits at C099_MODE_WAITING_ON_ENTRANCE until a
         * fresh command arrives; the launcher click/key that got us here was
         * drained above and must not count as that command.  Therefore timeout
         * auto-enter is strictly a dummy-video/autotest escape hatch, never an
         * interactive runtime behavior. */
        if (M11_Entrance_ShouldAutoEnterForTimeout(allowHeadlessTimeout,
                                                   autoEnterAfterMs,
                                                   (uint64_t)(SDL_GetTicks() - started))) {
            return M11_ENTRANCE_COMMAND_ENTER;
        }
        SDL_Delay(16);
    }
}

/* Play the FTL swoosh palette animation. ReDMCSB SWSH.C: static logo on black palette,
 * then V0901006_PaletteCommands lights colors sequentially via Setcolor()/Vsync.
 * ESC/Enter/click skips. Skipped when --game was used (direct launch skips full intro). */
static void m11_swsh_indexed_to_rgba(const unsigned char* indexed,
                                     unsigned char* rgba,
                                     const unsigned char palette[16][3]) {
    unsigned int i;
    if (!indexed || !rgba || !palette) return;
    for (i = 0U; i < (unsigned int)M11_FB_BYTES; ++i) {
        unsigned char idx = indexed[i] & 0x0Fu;
        rgba[i * 4U + 0U] = palette[idx][0];
        rgba[i * 4U + 1U] = palette[idx][1];
        rgba[i * 4U + 2U] = palette[idx][2];
        rgba[i * 4U + 3U] = 0xFFu;
    }
}

/* Unpack a 4-bit-per-pixel Atari ST low-res buffer (160 bytes per row,
 * 32000 bytes total for 320x200) into a 1-byte-per-pixel indexed
 * framebuffer (320 bytes per row, 64000 bytes total).
 *
 * ReDMCSB SWSHGDAT.C FTL logo is 4bpp-packed in the IMG1 source. The
 * SWSH_Compat_ExpandLogoToBitmap decode path emits 4bpp-packed pixels
 * (matching the original Atari ST Physbase layout). The FTL swoosh
 * renderer treats `screenFb` as 1-byte-per-pixel, so the packed buffer
 * must be unpacked before indexed->RGBA conversion. Without this,
 * pixels 160..319 of every row stay zero (logo only renders in the
 * left half) and even pixels 0..159 show alternating nibbles (visible
 * vertical stripes). ReDMCSB TITLE.C PC/F20 port uses the same Atari
 * ST logo layout; see m11_unpack_title_4bpp_to_indexed for the same
 * pattern used by the DM title animation.
 *
 * Source-lock: Atari ST VDI / LINEA: F0080_vq_extend / F0086_vs_clip
 * low-res Physbase layout is 4 planes of (width+1)/2 bytes per row.
 */
static void m11_swsh_unpack_4bpp_to_indexed(const unsigned char* packed,
                                           unsigned char* indexed) {
    unsigned int y;
    unsigned int x;
    if (!packed || !indexed) return;
    for (y = 0U; y < (unsigned int)M11_FB_HEIGHT; ++y) {
        const unsigned char* src = packed + y * 160U;
        unsigned char* dst = indexed + y * (unsigned int)M11_FB_WIDTH;
        for (x = 0U; x < (unsigned int)M11_FB_WIDTH; x += 2U) {
            unsigned char b = src[x >> 1];
            dst[x]         = (unsigned char)((b >> 4) & 0x0Fu);
            dst[x + 1U]    = (unsigned char)(b & 0x0Fu);
        }
    }
}

static int m11_delay_ms_with_intro_event_pump(unsigned int delayMs) {
    Uint64 start;
    if (delayMs == 0U) {
        return M11_Render_PumpEvents();
    }
    start = SDL_GetTicks();
    while ((SDL_GetTicks() - start) < (Uint64)delayMs) {
        Uint64 elapsed;
        unsigned int remaining;
        if (M11_Render_PumpEvents()) {
            return 1;
        }
        elapsed = SDL_GetTicks() - start;
        if (elapsed >= (Uint64)delayMs) {
            break;
        }
        remaining = delayMs - (unsigned int)elapsed;
        SDL_Delay(remaining > 10U ? 10U : remaining);
    }
    return 0;
}

static void m11_play_ftl_swoosh_if_available(const M12_StartupMenuState* menuState,
                                              const char* dataDir,
                                              int skipSwoosh) {
    char logoPath[FSP_PATH_MAX];
    unsigned char* logoImg = NULL;
    unsigned char* screenFbPacked = NULL;
    unsigned char* screenFbIndexed = NULL;
    unsigned char* screenRgba = NULL;
    FILE* f = NULL; long fsize = 0;
    SWSH_CompatLogoPayload logoPayload;
    unsigned char swshPalette[16][3];
    if (skipSwoosh) return;
    memset(&logoPayload, 0, sizeof(logoPayload));
    if (!M11_SWSH_Intro_FindLogoPath(menuState, dataDir, logoPath, sizeof(logoPath))) return;
    f = fopen(logoPath, "rb"); if (!f) return;
    fseek(f, 0, SEEK_END); fsize = ftell(f); fseek(f, 0, SEEK_SET);
    logoImg = (unsigned char*)malloc((size_t)fsize);
    /* Atari ST low-res FTL logo: 4bpp packed, 160 bytes/row, 32000 bytes total.
     * Use a dedicated packed buffer for the SWSH_Compat_ExpandLogoToBitmap
     * output (was previously aliased onto screenFb which is 1bpp). */
    screenFbPacked  = (unsigned char*)malloc((size_t)(M11_FB_WIDTH * M11_FB_HEIGHT) / 2U);
    /* 1-byte-per-pixel indexed framebuffer that m11_swsh_indexed_to_rgba reads. */
    screenFbIndexed = (unsigned char*)calloc(1, (size_t)M11_FB_BYTES);
    screenRgba      = (unsigned char*)malloc((size_t)M11_FB_BYTES * 4U);
    if (!logoImg || !screenFbPacked || !screenFbIndexed || !screenRgba) goto cleanup;
    if (fread(logoImg, 1, (size_t)fsize, f) != (size_t)fsize) goto cleanup;
    if (!SWSH_Compat_FindLogoImagePayloadEx(logoImg, (unsigned int)fsize, &logoPayload)) goto cleanup;
    SWSH_Compat_ExpandLogoToBitmap(logoPayload.payload, screenFbPacked);
    /* BUG-PASS841-FIX: pass841 — the FTL swoosh logo was rendered as a
     * half-blank vertically-striped image because the runtime treated
     * the 4bpp-packed Atari ST low-res output of SWSH_Compat_ExpandLogoToBitmap
     * as a 1-byte-per-pixel indexed framebuffer. Unpack 4bpp->1bpp before
     * any palette/RGBA conversion. Without this, pixels 160..319 of every
     * row stay zero and even pixels 0..159 show alternating nibbles. */
    m11_swsh_unpack_4bpp_to_indexed(screenFbPacked, screenFbIndexed);

    /* ReDMCSB SWSH.C F2255:2975-3039 / DRAWVIEW.C G8162-G8171:
     * the PC/F20E path applies each Swoosh palette row before waiting
     * for its source delay. Present each palette mutation immediately
     * so the logo does not collapse several source colors into one
     * modern frame. */
    memset(swshPalette, 0, sizeof(swshPalette));
    {
      unsigned int sourceStep;
      m11_swsh_indexed_to_rgba(screenFbIndexed, screenRgba, swshPalette);
      M11_Render_PresentRGBA(screenRgba, M11_FB_WIDTH, M11_FB_HEIGHT);
      if (m11_delay_ms_with_intro_event_pump(
              SWSH_Compat_GetRuntimeInitialLogoHoldMs())) {
          goto cleanup;
      }
      for (sourceStep = 1U; sourceStep <= SWSH_Compat_GetSourceAnimationStepCount(); ++sourceStep) {
          SWSH_CompatSourceAnimationStep step;
          if (M11_Render_PumpEvents()) break;
          if (!SWSH_Compat_GetSourceAnimationStep(sourceStep, &step)) break;
          if (step.kind == SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR) {
              SWSH_Compat_ConvertPcSwooshRgbWordToRgb8(step.colorValue,
                                                       swshPalette[step.colorIndex & 0x0FU]);
              m11_swsh_indexed_to_rgba(screenFbIndexed, screenRgba, swshPalette);
              M11_Render_PresentRGBA(screenRgba, M11_FB_WIDTH, M11_FB_HEIGHT);
          } else if (step.kind == SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS) {
              /* ReDMCSB SWSH.C:33-37: each Vsync wait is one 50 Hz vertical
               * blank (~20 ms).  Use wall-clock timing so high-refresh displays
               * (e.g. MacBook Pro 120 Hz ProMotion) do not race through the
               * palette animation faster than the original Atari ST rate.
               * The previous dead-code `paletteDirty` flag was removed:
               * the WAIT_VBLANKS branch never had a palette to "re-render"
               * because every SET_PALETTE_COLOR step renders its own frame. */
              if (m11_delay_ms_with_intro_event_pump(
                      SWSH_Compat_GetRuntimeDelayMsForVblankCount(
                          step.vblankCount))) {
                  break;
              }
          } else if (step.kind == SWSH_COMPAT_SOURCE_EVENT_RUN_START_PROGRAM) {
              /* No palette was queued between the previous SET_PALETTE_COLOR
               * and now (the dead paletteDirty=1 path was never reachable). */
          }
      }
      (void)m11_delay_ms_with_intro_event_pump(
          SWSH_Compat_GetRuntimeFinalHoldMs()); }
cleanup:
    SWSH_Compat_ReleaseLogoImagePayload(&logoPayload);
    if (logoImg) free(logoImg);
    if (screenFbPacked) free(screenFbPacked);
    if (screenFbIndexed) free(screenFbIndexed);
    if (screenRgba) free(screenRgba);
    if (f) fclose(f);
}

static int m11_play_redmcsb_title_graphic_intro_if_available(M11_GameViewState* gameView,
                                                              int* outPlayedAnyFrame) {
    const M11_AssetSlot* titleGraphic;
    unsigned char* framebuffer;
    V1_TitleFrontendSourceTiming timing;
    M11_AudioState titleAudio;
    int titleAudioInitialized = 0;
    unsigned int sourceStep;

    if (outPlayedAnyFrame) {
        *outPlayedAnyFrame = 0;
    }
    if (!gameView || !gameView->assetsAvailable) {
        return 0;
    }
    titleGraphic = M11_AssetLoader_Load(&gameView->assetLoader, 1U);
    {
        V1_TitleFrontendRuntimeSourceDecision sourceDecision =
            V1_TitleFrontend_SelectRuntimeSource(
                titleGraphic != NULL,
                titleGraphic ? titleGraphic->width : 0U,
                titleGraphic ? titleGraphic->height : 0U,
                0);
        if (sourceDecision.source != V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001) {
            return 0;
        }
    }
    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) {
        return 0;
    }
    timing = V1_TitleFrontend_GetSourceTimingEvidence();

    memset(&titleAudio, 0, sizeof(titleAudio));
    if (M11_Audio_Init(&titleAudio)) {
        titleAudioInitialized = 1;
        (void)M11_Audio_PlayTitleMusic(&titleAudio);
    }

    memset(framebuffer, 0, (size_t)M11_FB_BYTES);

    /* ReDMCSB TITLE.C F0437 PC/F20 source-lock:
     * - TITLE.C:309 loads/decompresses C001_GRAPHIC_TITLE.
     * - TITLE.C:319-324 blits PRESENTS from source y=137 to 0,90..105.
     * - TITLE.C:340-360 builds 18 shrinked 320x80 title bitmaps.
     * - TITLE.C:385-387 waits VBlank and blits those bitmaps in reverse
     *   order (small 48x12 at 136,74 through full 320x80 at 0,40).
     * - TITLE.C:395-402 waits twice, then blits MASTER from source y=80
     *   to 0,118..174 with black transparency, before the final guard.
     *
     * Firestaff previously drove this handoff from the decoded TITLE.DAT
     * animation bank.  That bank is a separate PC title-file animation; the
     * ReDMCSB startup TITLE routine uses GRAPHICS.DAT graphic C001 instead.
     */
    for (sourceStep = 1U; sourceStep <= V1_TitleFrontend_GetSourceAnimationStepCount(); ++sourceStep) {
        V1_TitleFrontendSourceAnimationStep step;
        int stepPalette;
        if (!V1_TitleFrontend_GetSourceAnimationStep(sourceStep, &step)) {
            break;
        }
        if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS) {
            memset(framebuffer, 0, (size_t)M11_FB_BYTES);
            M11_AssetLoader_BlitRegion(titleGraphic,
                                       0, 137, 320, 16,
                                       framebuffer,
                                       M11_FB_WIDTH,
                                       M11_FB_HEIGHT,
                                       0, 90,
                                       -1);
        } else if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT) {
            memset(framebuffer, 0, (size_t)M11_FB_BYTES);
            M11_AssetLoader_BlitSubRectScaled(titleGraphic,
                                              framebuffer,
                                              M11_FB_WIDTH,
                                              M11_FB_HEIGHT,
                                              (int)step.x,
                                              (int)step.y,
                                              (int)step.width,
                                              (int)step.height,
                                              0,
                                              0,
                                              320,
                                              80,
                                              -1);
        } else if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT) {
            M11_AssetLoader_BlitRegion(titleGraphic,
                                       0, 80, 320, 57,
                                       framebuffer,
                                       M11_FB_WIDTH,
                                       M11_FB_HEIGHT,
                                       0, 118,
                                       0);
        } else {
            if (m11_delay_ms_with_intro_event_pump(
                    V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing))) {
                break;
            }
            continue;
        }

        /* ReDMCSB TITLE.C F0437 PC/F20: PRESENTS uses C12_PRESENTS
         * (VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS), ZOOM and STRIKES
         * BACK use the merged C13_DUNGEON + C14_MASTER palette
         * (VGA_PALETTE_PC34_SPECIAL_TITLE).  The helper
         * V1_TitleFrontend_GetStepPalette is the single source of
         * truth for that mapping; v2.7.4 always used
         * VGA_PALETTE_PC34_SPECIAL_TITLE for every step and painted
         * the "PRESENTS" word red instead of plain white. */
        (void)V1_TitleFrontend_GetStepPalette(step.kind, &stepPalette);
        if (M11_Render_PresentIndexedWithSpecialPalette(framebuffer,
                                                        M11_FB_WIDTH,
                                                        M11_FB_HEIGHT,
                                                        stepPalette) != M11_RENDER_OK) {
            break;
        }
        if (outPlayedAnyFrame) {
            *outPlayedAnyFrame = 1;
        }
        if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS) {
            if (m11_delay_ms_with_intro_event_pump(
                    V1_TitleFrontend_GetRuntimePresentsHoldDelayMs(&timing))) {
                break;
            }
        } else {
            if (m11_delay_ms_with_intro_event_pump(
                    V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing))) {
                break;
            }
        }
    }
    /* ReDMCSB TITLE.C:395-409 leaves two post-zoom VBlanks plus the final
     * BUG0_71 guard before STARTUP1.C advances into the entrance.  The
     * TITLE.DAT fallback below already observes this; keep the GRAPHICS.DAT
     * C001 runtime path on the same source cadence. */
    if (!m11_delay_ms_with_intro_event_pump(
            V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing))) {
        (void)m11_delay_ms_with_intro_event_pump(
            V1_TitleFrontend_GetRuntimeC001CadencePadDelayMs(&timing));
    }
    if (titleAudioInitialized) {
        M11_Audio_Shutdown(&titleAudio);
    }
    return outPlayedAnyFrame ? *outPlayedAnyFrame : 1;
}

static void m11_play_redmcsb_title_intro_if_available(const M12_StartupMenuState* menuState,
                                                      M11_GameViewState* gameView,
                                                      int* outPlayedAnyFrame) {
    char titlePath[FSP_PATH_MAX];
    unsigned char* packedStorage;
    unsigned char* packedScreen;
    unsigned char* indexedScreen;
    char err[160];
    unsigned int step;
    V1_TitleFrontendSourceTiming timing;
    M11_AudioState titleAudio;
    int titleAudioInitialized = 0;

    if (outPlayedAnyFrame) {
        *outPlayedAnyFrame = 0;
    }
    if (m11_play_redmcsb_title_graphic_intro_if_available(gameView, outPlayedAnyFrame)) {
        return;
    }
    if (!m11_find_title_dat_for_intro(menuState, titlePath, sizeof(titlePath))) {
        fprintf(stderr,
                "Firestaff V1 original TITLE intro skipped: no GRAPHICS.DAT C001 title graphic "
                "or DM PC 3.4 TITLE fallback file found; set FIRESTAFF_TITLE_DAT or install "
                "the canonical original-data anchor at "
                "$HOME/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE.\n");
        return;
    }
    packedStorage = (unsigned char*)calloc(1U, 4U + 32000U);
    indexedScreen = (unsigned char*)malloc((size_t)M11_FB_BYTES);
    if (!packedStorage || !indexedScreen) {
        free(packedStorage);
        free(indexedScreen);
        return;
    }
    packedScreen = packedStorage + 4U;
    timing = V1_TitleFrontend_GetSourceTimingEvidence();

    memset(&titleAudio, 0, sizeof(titleAudio));
    if (M11_Audio_Init(&titleAudio)) {
        titleAudioInitialized = 1;
        (void)M11_Audio_PlayTitleMusic(&titleAudio);
    }

    /* ReDMCSB TITLE.C PC/F20 source-lock:
     *   TITLE.C:319-324 draws PRESENTS from the decompressed title graphic.
     *   TITLE.C:340-360 builds 18 shrinked title bitmaps; TITLE.C:385-387
     *               waits M526_WaitVerticalBlank() before each reverse-order zoom blit.
     *   TITLE.C:395-402 waits two more VBlanks and draws STRIKES BACK.
     *   TITLE.C:409 adds the final guard before the next screen.
     * Runtime normally uses GRAPHICS.DAT C001 above.  If that bitmap is not
     * available, keep the hash-locked TITLE.DAT bank as a last-resort visible
     * fallback rather than skipping straight to the entrance. */
    for (step = 1U; step <= V1_TITLE_DAT_FRAME_MAX; ++step) {
        V1_TitleFrontendSequenceDecision d = V1_TitleFrontend_DecideSequenceStep(step);
        V1_TitleFrontendRenderResult renderResult;
        int stepPalette;
        memset(packedStorage, 0, 4U + 32000U);
        memset(indexedScreen, 0, (size_t)M11_FB_BYTES);
        memset(&renderResult, 0, sizeof(renderResult));
        err[0] = '\0';
        if (!V1_TitleFrontend_RenderFrameToScreen(titlePath,
                                                  d.renderFrameOrdinal,
                                                  packedScreen,
                                                  &renderResult,
                                                  err,
                                                  sizeof(err))) {
            fprintf(stderr,
                    "Firestaff V1 original TITLE intro stopped: failed to render frame %u from %s: %s\n",
                    d.renderFrameOrdinal,
                    titlePath,
                    err[0] ? err : "unknown TITLE decode error");
            break;
        }
        m11_unpack_title_4bpp_to_indexed(packedScreen, indexedScreen);
        /* TITLE.DAT is the bank-of-frames fallback used when the
         * GRAPHICS.DAT C001 graphic is not available.  Keep its palette
         * choice behind the same ReDMCSB TITLE.C source-lock helper as
         * the normal GRAPHICS.DAT path; the runtime must not hard-code a
         * different interpretation of the C12_PRESENTS -> C13_DUNGEON +
         * C14_MASTER switch. */
        (void)V1_TitleFrontend_GetFallbackFramePalette(renderResult.paletteOrdinal,
                                                       &stepPalette);
        if (M11_Render_PresentIndexedWithSpecialPalette(indexedScreen,
                                                        M11_FB_WIDTH,
                                                        M11_FB_HEIGHT,
                                                        stepPalette) != M11_RENDER_OK) {
            fprintf(stderr,
                    "Firestaff V1 original TITLE intro stopped: renderer failed to present frame %u\n",
                    d.renderFrameOrdinal);
            break;
        }
        if (outPlayedAnyFrame) {
            *outPlayedAnyFrame = 1;
        }
        /* ReDMCSB TITLE.C:201-214 gates the zoom on vertical blanks, then
         * TITLE.C:251 adds a final BUG0_71 guard so fast machines do not
         * smash straight into the entrance screen.  Bind the runtime delay
         * through the TITLE frontend helper so the observable handoff cadence
         * remains tied to the source timing evidence. */
        if (m11_delay_ms_with_intro_event_pump(
                V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing))) {
            break;
        }
    }
    (void)m11_delay_ms_with_intro_event_pump(
        V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing));
    if (titleAudioInitialized) {
        M11_Audio_Shutdown(&titleAudio);
    }
    free(packedStorage);
    free(indexedScreen);
}

static int m11_open_requested_launch(M11_GameViewState* gameView,
                                     M12_StartupMenuState* menuState,
                                     uint32_t* idleAccumulatorMs,
                                     const char* dataDir) {
    int titleIntroPlayed = 0;
    if (!gameView || !menuState || !menuState->launchRequested) {
        return 0;
    }
    {
        /* ReDMCSB startup source-lock: MAIN/STARTEND enters F0437_STARTEND_DrawTitle() before
         * F0441_STARTEND_ProcessEntrance().  Firestaff has a modern launcher front door, so
         * the original TITLE animation and title-song/swoosh cue must run at the
         * launcher->DM1 handoff, before the game view opens and before the entrance
         * transition.
         * This is a DM1 source-order rule, not a V1-only renderer feature:
         * Entrance is mandatory for every DM1 presentation mode, and TITLE must
         * be the visible handoff immediately before it.  CSB/DM2/Nexus keep
         * their own intro paths. */


        const M12_MenuEntry* launchEntry = M12_StartupMenu_GetEntry(
            menuState, menuState->activatedIndex);
        if (launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "dm1") == 0) {
            /* ReDMCSB: FTL swoosh (SWSH.C) before TITLE per original boot order.
             * Pass the menu state so the FTL/SWSH finder can locate SWOOSH next
             * to the matched GRAPHICS.DAT, the user-supplied data dir, or the
             * canonical $HOME OpenClaw original-games anchors. */
            M11_Render_RaiseWindow();
            m11_play_ftl_swoosh_if_available(menuState, dataDir, 0);
            /* ReDMCSB source order is SWSH.C -> STARTUP1.C:143 ->
             * TITLE.C F0437.  The SWSH path presents caller-owned RGBA
             * frames, while TITLE.C presents indexed C001 pixels through the
             * C12/C13/C14 VGA palette tables.  Recreate the SDL streaming
             * texture at this one-time handoff so SDL3/Metal does not carry
             * stale true-colour texture state into the indexed title
             * animation on Apple Silicon. */
            M11_Render_DiscardPresentationTexture();
        }
        /* CSB has its own title/entrance sequence.  ReDMCSB ENTRANCE.C
         * F0806 builds the CSB entrance micro-dungeon with C28_ENTRANCE_CSB
         * palette and switches to C001_MODE_LOAD_DUNGEON when the player
         * dismisses the entrance.  CSB does not use the DM1 FTL swoosh.
         * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441, 857-883. */
        else if (launchEntry && launchEntry->gameId &&
                 strcmp(launchEntry->gameId, "csb") == 0) {
            /* CSB title/entrance plays after M11_GameView_OpenSelectedMenuEntry
             * below, using the CSB runtime boot state.  No FTL swoosh needed. */
        }
        /* Theron's Quest has no source -- no intro needed. */
    }
    if (M11_GameView_OpenSelectedMenuEntry(gameView, menuState)) {
        menuState->launchRequested = 0;
        (void)M11_Render_SetPaletteLevel(0);
        if (idleAccumulatorMs) {
            *idleAccumulatorMs = 0;
        }
        if (!titleIntroPlayed && strcmp(gameView->sourceId, "dm1") == 0) {
            /* ReDMCSB STARTEND still orders F0437_STARTEND_DrawTitle before
             * F0441_STARTEND_ProcessEntrance.  Play it after the launch spec
             * has resolved so GRAPHICS.DAT C001 is available, but still
             * before entrance/gameplay is shown. */
            M11_Render_RaiseWindow();
            m11_play_redmcsb_title_intro_if_available(menuState, gameView, &titleIntroPlayed);
        }
        /* ReDMCSB: the entrance screen always plays for DM1 regardless
         * of presentation mode. F0441_STARTEND_ProcessEntrance is the
         * mandatory gate between title and gameplay. Other games keep
         * their own handoff paths; Theron's Quest starts directly from
         * its Track 02 runtime image. */
        if (strcmp(gameView->sourceId, "dm1") == 0) {
            int entranceResult = m11_play_redmcsb_entrance_transition(gameView, 1200);
            if (entranceResult == M11_ENTRANCE_COMMAND_QUIT) {
                gameView->active = 0;
                return 1;
            }
            if (entranceResult == M11_ENTRANCE_COMMAND_RESUME) {
                /* ReDMCSB COMMAND.C M566: RESUME loads the saved game.
                 * Prefer the launcher's already validated DM1 quick-resume
                 * path, then fall back to Firestaff's historical source-id
                 * save filename. */
                char savePath[512];
                int usedBackup = 0;
                if (M11_Entrance_ResolveDm1ResumeSavePath(
                        gameView->sourceId,
                        menuState->quickResumeAvailable,
                        menuState->quickResumeGameId,
                        menuState->quickResumeSavePath,
                        savePath,
                        sizeof(savePath)) &&
                    M11_GameView_LoadDm1SavePath(gameView, savePath, &usedBackup)) {
                    gameView->active = 1;
                    fprintf(stderr, "RESUME: loaded save from %s%s\n", savePath,
                            usedBackup ? " backup" : "");
                } else {
                    fprintf(stderr, "RESUME: no save found at %s, starting new game\n",
                            savePath[0] ? savePath : "(unresolved)");
                }
            } else if (!entranceResult) {
                /* Non-fatal: skip entrance animation but continue to game.
                 * Previously this aborted back to menu, causing the black
                 * viewport bug when TITLE.DAT decode failed. */
                fprintf(stderr, "entrance transition skipped (non-fatal)\n");
            }
        }
        M11_GameView_Draw(gameView,
                          M11_Render_GetFramebuffer(),
                          M11_FB_WIDTH,
                          M11_FB_HEIGHT);
        return 1;
    }
    menuState->launchRequested = 0;
    menuState->view = M12_MENU_VIEW_MESSAGE;
    menuState->messageLine1 = "DUNGEON LOAD FAILED";
    menuState->messageLine2 = "CHECK DUNGEON.DAT";
    menuState->messageLine3 = "ESC RETURNS TO MENU";
    return 0;
}

static int m11_restart_current_launch(M11_GameViewState* gameView,
                                      M12_StartupMenuState* menuState,
                                      uint32_t* idleAccumulatorMs,
                                      const char* dataDir) {
    if (!gameView || !menuState) {
        return 0;
    }
    /* ReDMCSB ENDGAME.C F0444 lines 568-590 observes
     * G0523_B_RestartGameRequested after queue processing and returns to
     * the top-level load/start path.  Firestaff mirrors that as a full
     * teardown followed by the same selected-entry launch handoff used by
     * the modern launcher. */
    M11_GameView_Shutdown(gameView);
    M11_GameView_Init(gameView);
    menuState->launchRequested = 1;
    return m11_open_requested_launch(gameView, menuState, idleAccumulatorMs, dataDir);
}

static int m11_prepare_direct_launch(M12_StartupMenuState* menuState,
                                     const char* gameId) {
    int i;
    int entryCount;
    if (!menuState || !gameId || gameId[0] == '\0') {
        return 0;
    }
    entryCount = M12_StartupMenu_GetEntryCount();
    for (i = 0; i < entryCount; ++i) {
        const M12_MenuEntry* entry = M12_StartupMenu_GetEntry(menuState, i);
        if (entry &&
            entry->kind == M12_MENU_ENTRY_GAME &&
            entry->gameId &&
            strcmp(entry->gameId, gameId) == 0) {
            if (!entry->available ||
                !M12_AssetStatus_GameAvailable(&menuState->assetStatus, gameId)) {
                return 0;
            }
            menuState->selectedIndex = i;
            menuState->activatedIndex = i;
            menuState->launchRequested = 1;
            menuState->quickResumeLaunchRequested = 0;
            return 1;
        }
    }
    return 0;
}

void M11_PhaseA_SetDefaultOptions(M11_PhaseA_Options* opts) {
    if (!opts) {
        return;
    }
    opts->windowWidth    = 960;
    opts->windowHeight   = 540;
    opts->scaleMode      = M11_SCALE_FIT;
    opts->durationMs     = -1;
    opts->presentEveryMs = 16;
    opts->script         = NULL;
    opts->dataDir        = NULL;
    opts->gameId         = NULL;
    opts->directLaunch   = 0;
}


static void m11_write_autotest_runtime_probe(const char* path,
                                             int launchedEver,
                                             const M11_GameViewState* gameView,
                                             int inputRedrawDrawCount,
                                             int inputRedrawAfterViewportDirtyCount,
                                             int lastInputRedrawAfterViewportDirty) {
    FILE* f;
    if (!path || path[0] == '\0') {
        return;
    }
    f = fopen(path, "w");
    if (!f) {
        return;
    }
    fprintf(f,
            "{\n"
            "  \"schema\": \"firestaff_m11_autotest_runtime_probe.v1\",\n"
            "  \"launchedEver\": %d,\n"
            "  \"active\": %d,\n"
            "  \"title\": \"%s\",\n"
            "  \"sourceId\": \"%s\",\n"
            "  \"presentation\": {\"mode\": %d, \"width\": %d, \"height\": %d},\n"
            "  \"lastAction\": \"%s\",\n"
            "  \"lastOutcome\": \"%s\",\n"
            "  \"gameTick\": %u,\n"
            "  \"party\": {\"mapIndex\": %d, \"mapX\": %d, \"mapY\": %d, \"direction\": %d, \"championCount\": %d},\n"
            "  \"pipeline\": {\"dequeued\": %d, \"command\": %d, \"turnApplied\": %d, \"stepApplied\": %d, \"movementBlocked\": %d, \"anyMovementOccurred\": %d, \"anyTurnOccurred\": %d, \"viewportDirty\": %d},\n"
            "  \"redraw\": {\"inputRedrawDrawCount\": %d, \"inputRedrawAfterViewportDirtyCount\": %d, \"lastInputRedrawAfterViewportDirty\": %d}\n"
            "}\n",
            launchedEver,
            gameView ? gameView->active : 0,
            gameView ? gameView->title : "",
            gameView ? gameView->sourceId : "",
            gameView ? gameView->presentationMode : -1,
            gameView ? gameView->presentationWidth : 0,
            gameView ? gameView->presentationHeight : 0,
            gameView ? gameView->lastAction : "",
            gameView ? gameView->lastOutcome : "",
            gameView ? (unsigned int)gameView->world.gameTick : 0U,
            gameView ? gameView->world.party.mapIndex : -1,
            gameView ? gameView->world.party.mapX : -1,
            gameView ? gameView->world.party.mapY : -1,
            gameView ? gameView->world.party.direction : -1,
            gameView ? gameView->world.party.championCount : -1,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.queue.dequeued : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.queue.command : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.turnApplied : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.stepApplied : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.core.movementBlocked : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.anyMovementOccurred : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.anyTurnOccurred : 0,
            gameView ? gameView->lastDm1V1MovementPipelineResult.viewportDirty : 0,
            inputRedrawDrawCount,
            inputRedrawAfterViewportDirtyCount,
            lastInputRedrawAfterViewportDirty);
    fclose(f);
}

static void m11_write_autotest_screenshot(const char* outputDir) {
    char outPath[1024];
    if (!outputDir || outputDir[0] == '\0') {
        return;
    }
    if (!M11_Screenshot_CaptureCurrent(outputDir, outPath, (int)sizeof(outPath))) {
        fprintf(stderr, "firestaff: autotest screenshot capture failed: %s\n", outputDir);
        return;
    }
    fprintf(stderr, "AUTOTEST SCREENSHOT: %s\n", outPath);
}

static void m11_write_autotest_presented_screenshot(const char* outputDir) {
    char outPath[1024];
    if (!outputDir || outputDir[0] == '\0') {
        return;
    }
    if (!M11_Screenshot_CapturePresentedRGBA(outputDir, outPath, (int)sizeof(outPath))) {
        fprintf(stderr, "firestaff: autotest presented screenshot capture failed: %s\n", outputDir);
        return;
    }
    fprintf(stderr, "AUTOTEST PRESENTED SCREENSHOT: %s\n", outPath);
}

static M12_MenuInput m11_map_script_token(const char* token, size_t len) {
    if (!token || len == 0U) {
        return M12_MENU_INPUT_NONE;
    }
    if ((len == 2U && strncmp(token, "up", len) == 0) ||
        (len == 1U && strncmp(token, "u", len) == 0)) {
        return M12_MENU_INPUT_UP;
    }
    if ((len == 4U && strncmp(token, "down", len) == 0) ||
        (len == 1U && strncmp(token, "d", len) == 0)) {
        return M12_MENU_INPUT_DOWN;
    }
    if ((len == 11U && strncmp(token, "strafe-left", len) == 0) ||
        (len == 2U && strncmp(token, "sl", len) == 0)) {
        return M12_MENU_INPUT_STRAFE_LEFT;
    }
    if ((len == 12U && strncmp(token, "strafe-right", len) == 0) ||
        (len == 2U && strncmp(token, "sr", len) == 0)) {
        return M12_MENU_INPUT_STRAFE_RIGHT;
    }
    /* v2.8.x: script-token `left` keeps its historical turn-left
     * semantics so existing probe and replay-script code (pass373's
     * script, firestaff_m11_wall_collision_capture_probe,
     * firestaff_m11_turn_viewport_orientation_probe, etc.) keeps
     * working unchanged.  The user's keyboard-mapping request is
     * honoured at the SDL scancode layer: SDLK_LEFT maps to
     * STRAFE_LEFT (which feeds the gameplay pipeline through
     * M12_MENU_INPUT_STRAFE_LEFT, not M12_MENU_INPUT_LEFT).  Use
     * `strafe-left` or `sl` for the strafe-left replay token. */
    if ((len == 4U && strncmp(token, "left", len) == 0) ||
        (len == 1U && strncmp(token, "l", len) == 0)) {
        return M12_MENU_INPUT_TURN_LEFT;
    }
    /* v2.8.x: script-token `right` keeps its historical turn-right
     * semantics so pass373's `enter,down,down,down,down,down,down,enter,right`
     * script (and similar replay scripts in probe code) still drives
     * a turn.  Use `strafe-right` or `sr` for the strafe-right token
     * that the SDL scancode handler produces when SDLK_RIGHT is
     * pressed.  Same for `left` / `l` aliasing turn-left.  The
     * gameplay pipeline switch (m11_dm1_v1_pipeline_command_for_input
     * + m11_apply_tick) treats M12_MENU_INPUT_LEFT/RIGHT as turn so
     * script-token and probe-code paths both reach the same DM1 V1
     * command id (TURN_LEFT/RIGHT). */
    if ((len == 5U && strncmp(token, "right", len) == 0) ||
        (len == 1U && strncmp(token, "r", len) == 0)) {
        return M12_MENU_INPUT_TURN_RIGHT;
    }
    if ((len == 9U && strncmp(token, "turn-left", len) == 0) ||
        (len == 2U && strncmp(token, "tl", len) == 0) ||
        (len == 4U && strncmp(token, "home", len) == 0)) {
        return M12_MENU_INPUT_TURN_LEFT;
    }
    if ((len == 10U && strncmp(token, "turn-right", len) == 0) ||
        (len == 2U && strncmp(token, "tr", len) == 0) ||
        (len == 3U && strncmp(token, "end", len) == 0)) {
        return M12_MENU_INPUT_TURN_RIGHT;
    }
    if ((len == 5U && strncmp(token, "enter", len) == 0) ||
        (len == 6U && strncmp(token, "return", len) == 0)) {
        return M12_MENU_INPUT_ACCEPT;
    }
    if ((len == 5U && strncmp(token, "space", len) == 0) ||
        (len == 3U && strncmp(token, "act", len) == 0)) {
        return M12_MENU_INPUT_ACTION;
    }
    if ((len == 3U && strncmp(token, "tab", len) == 0) ||
        (len == 5U && strncmp(token, "champ", len) == 0)) {
        return M12_MENU_INPUT_CYCLE_CHAMPION;
    }
    if ((len == 3U && strncmp(token, "esc", len) == 0) ||
        (len == 6U && strncmp(token, "escape", len) == 0) ||
        (len == 4U && strncmp(token, "back", len) == 0)) {
        return M12_MENU_INPUT_BACK;
    }
    if (len == 4U && strncmp(token, "rest", len) == 0) {
        return M12_MENU_INPUT_REST_TOGGLE;
    }
    if ((len == 6U && strncmp(token, "stairs", len) == 0) ||
        (len == 7U && strncmp(token, "descend", len) == 0)) {
        return M12_MENU_INPUT_USE_STAIRS;
    }
    if ((len == 4U && strncmp(token, "grab", len) == 0) ||
        (len == 6U && strncmp(token, "pickup", len) == 0) ||
        (len == 1U && strncmp(token, "g", len) == 0)) {
        return M12_MENU_INPUT_PICKUP_ITEM;
    }
    if ((len == 4U && strncmp(token, "drop", len) == 0) ||
        (len == 3U && strncmp(token, "put", len) == 0) ||
        (len == 1U && strncmp(token, "p", len) == 0)) {
        return M12_MENU_INPUT_DROP_ITEM;
    }
    if (len == 5U && strncmp(token, "rune1", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_1;
    if (len == 5U && strncmp(token, "rune2", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_2;
    if (len == 5U && strncmp(token, "rune3", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_3;
    if (len == 5U && strncmp(token, "rune4", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_4;
    if (len == 5U && strncmp(token, "rune5", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_5;
    if (len == 5U && strncmp(token, "rune6", len) == 0) return M12_MENU_INPUT_SPELL_RUNE_6;
    if ((len == 4U && strncmp(token, "cast", len) == 0) ||
        (len == 5U && strncmp(token, "spell", len) == 0)) {
        return M12_MENU_INPUT_SPELL_CAST;
    }
    if (len == 5U && strncmp(token, "clear", len) == 0) return M12_MENU_INPUT_SPELL_CLEAR;
    if ((len == 3U && strncmp(token, "use", len) == 0) ||
        (len == 5U && strncmp(token, "drink", len) == 0) ||
        (len == 3U && strncmp(token, "eat", len) == 0)) {
        return M12_MENU_INPUT_USE_ITEM;
    }
    return M12_MENU_INPUT_NONE;
}

static int m11_script_keycode_from_name(const char* name) {
    if (!name || name[0] == '\0') {
        return 0;
    }
    if (strcmp(name, "up") == 0) return SDLK_UP;
    if (strcmp(name, "down") == 0) return SDLK_DOWN;
    if (strcmp(name, "left") == 0) return SDLK_LEFT;
    if (strcmp(name, "right") == 0) return SDLK_RIGHT;
    if (strcmp(name, "enter") == 0 || strcmp(name, "return") == 0) return SDLK_RETURN;
    if (strcmp(name, "kp-enter") == 0) return SDLK_KP_ENTER;
    if (strcmp(name, "kp-1") == 0 || strcmp(name, "kp1") == 0) return SDLK_KP_1;
    if (strcmp(name, "kp-2") == 0 || strcmp(name, "kp2") == 0) return SDLK_KP_2;
    if (strcmp(name, "kp-3") == 0 || strcmp(name, "kp3") == 0) return SDLK_KP_3;
    if (strcmp(name, "kp-4") == 0 || strcmp(name, "kp4") == 0) return SDLK_KP_4;
    if (strcmp(name, "kp-5") == 0 || strcmp(name, "kp5") == 0) return SDLK_KP_5;
    if (strcmp(name, "kp-6") == 0 || strcmp(name, "kp6") == 0) return SDLK_KP_6;
    if (strcmp(name, "space") == 0) return SDLK_SPACE;
    if (strcmp(name, "tab") == 0) return SDLK_TAB;
    if (strcmp(name, "esc") == 0 || strcmp(name, "escape") == 0) return SDLK_ESCAPE;
    if (strcmp(name, "f5") == 0) return SDLK_F5;
    if (strcmp(name, "f9") == 0) return SDLK_F9;
    if (strcmp(name, "f10") == 0) return SDLK_F10;
    if (strcmp(name, "f11") == 0) return SDLK_F11;
    if (name[1] == '\0') {
        switch (name[0]) {
            case 'a': return SDLK_A;
            case 'c': return SDLK_C;
            case 'd': return SDLK_D;
            case 'e': return SDLK_E;
            case 'g': return SDLK_G;
            case 'i': return SDLK_I;
            case 'm': return SDLK_M;
            case 'p': return SDLK_P;
            case 'q': return SDLK_Q;
            case 'r': return SDLK_R;
            case 's': return SDLK_S;
            case 'u': return SDLK_U;
            case 'v': return SDLK_V;
            case 'w': return SDLK_W;
            case 'x': return SDLK_X;
            case '1': return SDLK_1;
            case '2': return SDLK_2;
            case '3': return SDLK_3;
            case '4': return SDLK_4;
            case '5': return SDLK_5;
            case '6': return SDLK_6;
            default: break;
        }
    }
    return 0x7fffffff;
}

static int m11_game_view_is_csb(const M11_GameViewState* gameView) {
    return gameView && gameView->active && strcmp(gameView->sourceId, "csb") == 0;
}

static int m11_game_view_is_dm1(const M11_GameViewState* gameView) {
    return gameView && gameView->active && strcmp(gameView->sourceId, "dm1") == 0;
}

static int m11_csb_sdl_key_to_menu_input(int key, int ctrlDown, M12_MenuInput* outInput) {
    CsbV1KeyboardKeyPc34Compat csbKey = CSB_V1_KEYBOARD_KEY_NONE;
    switch (key) {
        case SDLK_F1: csbKey = CSB_V1_KEYBOARD_KEY_F1; break;
        case SDLK_F2: csbKey = CSB_V1_KEYBOARD_KEY_F2; break;
        case SDLK_F3: csbKey = CSB_V1_KEYBOARD_KEY_F3; break;
        case SDLK_F4: csbKey = CSB_V1_KEYBOARD_KEY_F4; break;
        case SDLK_ESCAPE: csbKey = CSB_V1_KEYBOARD_KEY_ESCAPE; break;
        case SDLK_RETURN: csbKey = CSB_V1_KEYBOARD_KEY_RETURN; break;
        case SDLK_KP_ENTER: csbKey = CSB_V1_KEYBOARD_KEY_ENTER; break;
#if SDL_VERSION_ATLEAST(3, 0, 0)
        case SDLK_S:
#else
        case SDLK_s:
#endif
            csbKey = CSB_V1_KEYBOARD_KEY_S;
            break;
        case SDLK_INSERT: csbKey = CSB_V1_KEYBOARD_KEY_INSERT; break;
        case SDLK_UP: csbKey = CSB_V1_KEYBOARD_KEY_UP; break;
        case SDLK_HOME: csbKey = CSB_V1_KEYBOARD_KEY_CLR_HOME; break;
        case SDLK_LEFT: csbKey = CSB_V1_KEYBOARD_KEY_LEFT; break;
        case SDLK_DOWN: csbKey = CSB_V1_KEYBOARD_KEY_DOWN; break;
        case SDLK_RIGHT: csbKey = CSB_V1_KEYBOARD_KEY_RIGHT; break;
        default:
            return 0;
    }
    return CSB_V1_KeyboardCommandToMenuInputPc34Compat(csbKey, ctrlDown, outInput);
}

static int m11_push_script_event_token(const char* token, size_t len) {
    char buffer[128];
    SDL_Event ev;
    int x = 0;
    int y = 0;
    if (!token || len == 0U || len >= sizeof(buffer)) {
        return 0;
    }
    memcpy(buffer, token, len);
    buffer[len] = '\0';
    memset(&ev, 0, sizeof(ev));

    if (strncmp(buffer, "key:", 4) == 0) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_KEY_DOWN;
        ev.key.key = (SDL_Keycode)m11_script_keycode_from_name(buffer + 4);
#else
        ev.type = SDL_KEYDOWN;
        ev.key.keysym.sym = (SDL_Keycode)m11_script_keycode_from_name(buffer + 4);
#endif
        SDL_PushEvent(&ev);
        return 1;
    }
    if (sscanf(buffer, "click:%d:%d", &x, &y) == 2) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        ev.button.button = SDL_BUTTON_LEFT;
        ev.button.x = (float)x;
        ev.button.y = (float)y;
#else
        ev.type = SDL_MOUSEBUTTONDOWN;
        ev.button.button = SDL_BUTTON_LEFT;
        ev.button.x = x;
        ev.button.y = y;
#endif
        SDL_PushEvent(&ev);
        return 1;
    }
    if (sscanf(buffer, "move:%d:%d", &x, &y) == 2) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        ev.type = SDL_EVENT_MOUSE_MOTION;
        ev.motion.x = (float)x;
        ev.motion.y = (float)y;
#else
        ev.type = SDL_MOUSEMOTION;
        ev.motion.x = x;
        ev.motion.y = y;
#endif
        SDL_PushEvent(&ev);
        return 1;
    }
    return 0;
}

static int m11_input_action_is_motion(int action) {
    switch (action) {
        case M11_ACTION_MOVE_FORWARD:
        case M11_ACTION_MOVE_BACKWARD:
        case M11_ACTION_TURN_LEFT:
        case M11_ACTION_TURN_RIGHT:
        case M11_ACTION_STRAFE_LEFT:
        case M11_ACTION_STRAFE_RIGHT:
            return 1;
        default:
            return 0;
    }
}

static M12_MenuInput m11_menu_input_for_m11_action(int action) {
    switch (action) {
        case M11_ACTION_MOVE_FORWARD:
            return M12_MENU_INPUT_UP;
        case M11_ACTION_MOVE_BACKWARD:
            return M12_MENU_INPUT_DOWN;
        case M11_ACTION_TURN_LEFT:
            return M12_MENU_INPUT_TURN_LEFT;
        case M11_ACTION_TURN_RIGHT:
            return M12_MENU_INPUT_TURN_RIGHT;
        case M11_ACTION_STRAFE_LEFT:
            return M12_MENU_INPUT_STRAFE_LEFT;
        case M11_ACTION_STRAFE_RIGHT:
            return M12_MENU_INPUT_STRAFE_RIGHT;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

static M12_MenuInput m11_dm1_v1_fixed_turn_input_from_scancode(SDL_Scancode scancode) {
    /* ReDMCSB: COMMAND.C 677-684 maps PC34 keypad turn keys to C001/C002;
     * Firestaff's DM1 V1 keyboard convention also exposes Q/E + Home/End as
     * turn aliases.  Resolve these before persisted keymaps so stale pre-v2.8
     * bindings cannot route Q/E into cooldown-gated strafe commands. */
    switch (scancode) {
        case SDL_SCANCODE_HOME:
        case SDL_SCANCODE_Q:
        case SDL_SCANCODE_KP_4:
            return M12_MENU_INPUT_TURN_LEFT;
        case SDL_SCANCODE_END:
        case SDL_SCANCODE_E:
        case SDL_SCANCODE_KP_6:
            return M12_MENU_INPUT_TURN_RIGHT;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

static int m11_dm1_v1_input_is_immediate_turn(M12_MenuInput input) {
    /* ReDMCSB: CLIKMENU.C F0365 lines 142-180 sets G0321 true as soon
     * as C001/C002 turn is dispatched; COMMAND.C F0380 lines 2095-2100
     * only holds C003..C006 movement while G0310/G0311 cooldowns are
     * active.  Keep Q/E/Home/End/KP turn taps out of the delayed VBlank
     * pending queue so single taps rotate immediately like the source. */
    return input == M12_MENU_INPUT_TURN_LEFT ||
           input == M12_MENU_INPUT_TURN_RIGHT ||
           input == M12_MENU_INPUT_LEFT ||
           input == M12_MENU_INPUT_RIGHT;
}

static M12_MenuInput m11_motion_input_from_scancode(SDL_Scancode scancode) {
    int action = M11_Input_ActionForScancode(scancode);
    if (!m11_input_action_is_motion(action)) {
        switch (scancode) {
            case SDL_SCANCODE_KP_5:
                return M12_MENU_INPUT_UP;
            case SDL_SCANCODE_KP_2:
                return M12_MENU_INPUT_DOWN;
            case SDL_SCANCODE_KP_1:
                return M12_MENU_INPUT_STRAFE_LEFT;
            case SDL_SCANCODE_KP_3:
                return M12_MENU_INPUT_STRAFE_RIGHT;
            case SDL_SCANCODE_KP_4:
                return M12_MENU_INPUT_TURN_LEFT;
            case SDL_SCANCODE_KP_6:
                return M12_MENU_INPUT_TURN_RIGHT;
            default:
                return M12_MENU_INPUT_NONE;
        }
    }
    return m11_menu_input_for_m11_action(action);
}

static M12_MenuInput m11_menu_input_for_m12_gamepad_action(M12_InputAction action,
                                                           int gameplayActive) {
    switch (action) {
        case M12_ACTION_MOVE_FORWARD:
            return M12_MENU_INPUT_UP;
        case M12_ACTION_MOVE_BACKWARD:
            return M12_MENU_INPUT_DOWN;
        case M12_ACTION_TURN_LEFT:
            return gameplayActive ? M12_MENU_INPUT_TURN_LEFT : M12_MENU_INPUT_LEFT;
        case M12_ACTION_TURN_RIGHT:
            return gameplayActive ? M12_MENU_INPUT_TURN_RIGHT : M12_MENU_INPUT_RIGHT;
        case M12_ACTION_STRAFE_LEFT:
            return gameplayActive ? M12_MENU_INPUT_STRAFE_LEFT : M12_MENU_INPUT_LEFT;
        case M12_ACTION_STRAFE_RIGHT:
            return gameplayActive ? M12_MENU_INPUT_STRAFE_RIGHT : M12_MENU_INPUT_RIGHT;
        case M12_ACTION_ACCEPT:
            return M12_MENU_INPUT_ACCEPT;
        case M12_ACTION_BACK:
            return M12_MENU_INPUT_BACK;
        case M12_ACTION_ACTION:
            return M12_MENU_INPUT_ACTION;
        case M12_ACTION_CYCLE_CHAMPION:
            return M12_MENU_INPUT_CYCLE_CHAMPION;
        case M12_ACTION_REST_TOGGLE:
            return M12_MENU_INPUT_REST_TOGGLE;
        case M12_ACTION_USE_STAIRS:
            return M12_MENU_INPUT_USE_STAIRS;
        case M12_ACTION_PICKUP_ITEM:
            return M12_MENU_INPUT_PICKUP_ITEM;
        case M12_ACTION_DROP_ITEM:
            return M12_MENU_INPUT_DROP_ITEM;
        case M12_ACTION_SPELL_RUNE_1:
            return M12_MENU_INPUT_SPELL_RUNE_1;
        case M12_ACTION_SPELL_RUNE_2:
            return M12_MENU_INPUT_SPELL_RUNE_2;
        case M12_ACTION_SPELL_RUNE_3:
            return M12_MENU_INPUT_SPELL_RUNE_3;
        case M12_ACTION_SPELL_RUNE_4:
            return M12_MENU_INPUT_SPELL_RUNE_4;
        case M12_ACTION_SPELL_RUNE_5:
            return M12_MENU_INPUT_SPELL_RUNE_5;
        case M12_ACTION_SPELL_RUNE_6:
            return M12_MENU_INPUT_SPELL_RUNE_6;
        case M12_ACTION_SPELL_CAST:
            return M12_MENU_INPUT_SPELL_CAST;
        case M12_ACTION_SPELL_CLEAR:
            return M12_MENU_INPUT_SPELL_CLEAR;
        case M12_ACTION_USE_ITEM:
            return M12_MENU_INPUT_ACTION;
        case M12_ACTION_MAP_TOGGLE:
            return M12_MENU_INPUT_MAP_TOGGLE;
        case M12_ACTION_INVENTORY_TOGGLE:
            return M12_MENU_INPUT_INVENTORY_TOGGLE;
        case M12_ACTION_QUICK_SAVE:
            return M12_MENU_INPUT_SAVE_GAME;
        default:
            return M12_MENU_INPUT_NONE;
    }
}

static M12_MenuInput m11_gamepad_button_input(const M12_GamepadMap* map,
                                              SDL_GamepadButton button,
                                              int gameplayActive) {
    M12_InputAction action;
    if (!map || !map->enabled) return M12_MENU_INPUT_NONE;
    action = M12_GamepadMap_ActionForButton(map, button);
    if (action == M12_ACTION_COUNT) return M12_MENU_INPUT_NONE;
    return m11_menu_input_for_m12_gamepad_action(action, gameplayActive);
}

static M12_MenuInput m11_gamepad_axis_input(const M12_GamepadMap* map,
                                            SDL_GamepadAxis axis,
                                            int rawValue,
                                            int gameplayActive) {
    const M12_GamepadAxisConfig* cfg;
    int value;
    if (!map || !map->enabled) return M12_MENU_INPUT_NONE;
    cfg = M12_GamepadMap_GetAxisConfig(map, axis);
    if (!cfg) return M12_MENU_INPUT_NONE;
    value = M12_GamepadAxis_Process(cfg, rawValue);
    if (value > -16000 && value < 16000) return M12_MENU_INPUT_NONE;

    if (cfg->role == M12_AXIS_ROLE_MOVE) {
        if (axis == SDL_GAMEPAD_AXIS_LEFTY || axis == SDL_GAMEPAD_AXIS_RIGHTY) {
            return value < 0 ? M12_MENU_INPUT_UP : M12_MENU_INPUT_DOWN;
        }
        if (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX) {
            if (gameplayActive) {
                return value < 0 ? M12_MENU_INPUT_STRAFE_LEFT
                                 : M12_MENU_INPUT_STRAFE_RIGHT;
            }
            return value < 0 ? M12_MENU_INPUT_LEFT : M12_MENU_INPUT_RIGHT;
        }
    } else if (cfg->role == M12_AXIS_ROLE_TURN) {
        if (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX) {
            if (gameplayActive) {
                return value < 0 ? M12_MENU_INPUT_TURN_LEFT
                                 : M12_MENU_INPUT_TURN_RIGHT;
            }
            return value < 0 ? M12_MENU_INPUT_LEFT : M12_MENU_INPUT_RIGHT;
        }
    }
    return M12_MENU_INPUT_NONE;
}

static M12_MenuInput m11_held_motion_input_from_gamepad(
    const M11_GameViewState* gameView,
    const M12_GamepadStatus* gamepadStatus,
    const M12_GamepadMap* gamepadMap) {
    static const SDL_GamepadButton preferredButtons[] = {
        SDL_GAMEPAD_BUTTON_DPAD_UP,
        SDL_GAMEPAD_BUTTON_DPAD_DOWN,
        SDL_GAMEPAD_BUTTON_DPAD_LEFT,
        SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
        SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
        SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER
    };
    static const SDL_GamepadAxis preferredAxes[] = {
        SDL_GAMEPAD_AXIS_LEFTY,
        SDL_GAMEPAD_AXIS_LEFTX,
        SDL_GAMEPAD_AXIS_RIGHTX
    };
    size_t i;
    int gameplayActive = gameView && gameView->active;
    if (!gamepadStatus || !gamepadStatus->handle ||
        !gamepadMap || !gamepadMap->enabled) {
        return M12_MENU_INPUT_NONE;
    }
    for (i = 0; i < sizeof(preferredButtons) / sizeof(preferredButtons[0]); ++i) {
        SDL_GamepadButton button = preferredButtons[i];
        if (SDL_GetGamepadButton(gamepadStatus->handle, button)) {
            M12_MenuInput input =
                m11_gamepad_button_input(gamepadMap, button, gameplayActive);
            if (input != M12_MENU_INPUT_NONE) return input;
        }
    }
    for (i = 0; i < sizeof(preferredAxes) / sizeof(preferredAxes[0]); ++i) {
        SDL_GamepadAxis axis = preferredAxes[i];
        M12_MenuInput input =
            m11_gamepad_axis_input(gamepadMap,
                                   axis,
                                   SDL_GetGamepadAxis(gamepadStatus->handle, axis),
                                   gameplayActive);
        if (input != M12_MENU_INPUT_NONE) return input;
    }
    return M12_MENU_INPUT_NONE;
}

static M12_MenuInput m11_held_motion_input_from_keyboard(const M11_GameViewState* gameView) {
    int count = 0;
    int i;
#if SDL_VERSION_ATLEAST(3, 0, 0)
    const bool* keys = SDL_GetKeyboardState(&count);
#else
    const Uint8* keys = SDL_GetKeyboardState(&count);
#endif
    static const SDL_Scancode preferred[] = {
        SDL_SCANCODE_UP,
        SDL_SCANCODE_W,
        SDL_SCANCODE_DOWN,
        SDL_SCANCODE_S,
        SDL_SCANCODE_LEFT,
        SDL_SCANCODE_A,
        SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_D,
        SDL_SCANCODE_HOME,
        SDL_SCANCODE_Q,
        SDL_SCANCODE_END,
        SDL_SCANCODE_E,
        SDL_SCANCODE_KP_5,
        SDL_SCANCODE_KP_2,
        SDL_SCANCODE_KP_1,
        SDL_SCANCODE_KP_3,
        SDL_SCANCODE_KP_4,
        SDL_SCANCODE_KP_6
    };

    if (!keys || count <= 0) {
        return M12_MENU_INPUT_NONE;
    }
    for (i = 0; i < (int)(sizeof(preferred) / sizeof(preferred[0])); i++) {
        int sc = (int)preferred[i];
        if (sc >= 0 && sc < count && keys[sc]) {
            M12_MenuInput input =
                m11_game_view_is_dm1(gameView)
                    ? m11_dm1_v1_fixed_turn_input_from_scancode(preferred[i])
                    : M12_MENU_INPUT_NONE;
            if (input == M12_MENU_INPUT_NONE) {
                input = m11_motion_input_from_scancode(preferred[i]);
            }
            if (input != M12_MENU_INPUT_NONE) {
                return input;
            }
        }
    }
    return M12_MENU_INPUT_NONE;
}

static M12_MenuInput m11_next_script_input(const char** cursor) {
    const char* start;
    const char* end;
    if (!cursor || !*cursor) {
        return M12_MENU_INPUT_NONE;
    }
    start = *cursor;
    while (*start == ' ' || *start == ',') {
        ++start;
    }
    if (*start == '\0') {
        *cursor = start;
        return M12_MENU_INPUT_NONE;
    }
    end = start;
    while (*end != '\0' && *end != ',') {
        ++end;
    }
    *cursor = end;
    if (m11_push_script_event_token(start, (size_t)(end - start))) {
        return M12_MENU_INPUT_NONE;
    }
    return m11_map_script_token(start, (size_t)(end - start));
}

/* Result of polling a single pump. `menuPointerChanged` is set to 1
 * when a launcher mouse event mutated the menu state (the caller
 * should redraw the launcher). */
typedef struct {
    M12_MenuInput menuInput;
    int menuPointerChanged;
    int useModernLauncher;
} M11_PumpResult;

static int m11_map_window_to_launcher(int wx, int wy,
                                      int useModern,
                                      int* outX, int* outY) {
    int fbX = 0;
    int fbY = 0;
    if (!M11_Render_MapWindowToFramebuffer(wx, wy, &fbX, &fbY)) {
        return 0;
    }
    /* MapWindowToFramebuffer already maps into the current presented
     * content dimensions (1920x1080 for modern, 480x270 for legacy).
     * For the modern path we return the coords unchanged; for legacy
     * there is no mouse UI so we skip. */
    (void)useModern;
    if (outX) *outX = fbX;
    if (outY) *outY = fbY;
    return 1;
}

static int m11_dm1_rename_text_input_active(const M11_GameViewState* gameView) {
    return gameView && gameView->active &&
           gameView->candidateMirrorPanelActive &&
           gameView->candidateMirrorRenameActive;
}

static M11_GameInputResult
m11_dm1_rename_apply_ascii(M11_GameViewState* gameView, int ch) {
    if (!m11_dm1_rename_text_input_active(gameView)) {
        return M11_GAME_INPUT_IGNORED;
    }
    /* ReDMCSB REVIVE.C F0281:515-529 appends the selected A-Z/space/
     * punctuation character; the Firestaff gate keeps the same accepted
     * ASCII set and rejects other text-input bytes. */
    return M11_GameView_ApplyMirrorCandidateRenameAscii(gameView, ch)
               ? M11_GAME_INPUT_REDRAW
               : M11_GAME_INPUT_IGNORED;
}

static int m11_dm1_rename_consume_text_input(M11_GameViewState* gameView,
                                             const char* text,
                                             M11_GameInputResult* outResult) {
    int changed = 0;
    const unsigned char* p = (const unsigned char*)text;
    if (!m11_dm1_rename_text_input_active(gameView)) {
        return 0;
    }
    while (p && *p) {
        unsigned char ch = *p++;
        if (ch < 0x80U &&
            m11_dm1_rename_apply_ascii(gameView, (int)ch) ==
                M11_GAME_INPUT_REDRAW) {
            changed = 1;
        }
    }
    if (changed && outResult) {
        *outResult = M11_GAME_INPUT_REDRAW;
    }
    return 1;
}

static M11_GameInputResult
m11_dm1_rename_handle_keydown(M11_GameViewState* gameView,
                              int key,
                              int keypadEnterKey) {
    if (!m11_dm1_rename_text_input_active(gameView)) {
        return M11_GAME_INPUT_IGNORED;
    }
    /* ReDMCSB REVIVE.C F0281:535-545 uses Return to move from name to
     * title, F0281:549-567 uses backspace within the active field, and
     * F0282:806-808 enters F0281 from C161.  Consume all other keydown
     * events here so SDL_TEXTINPUT, not the movement/shortcut mapper,
     * owns printable rename characters while the panel is active. */
    if (key == SDLK_BACKSPACE || key == SDLK_ESCAPE) {
        return M11_GameView_ApplyMirrorCandidateRenameCommand(
                   gameView,
                   DM1_V1_RESURRECTION_RENAME_UI_COMMAND_BACKSPACE_PC34_COMPAT)
                   ? M11_GAME_INPUT_REDRAW
                   : M11_GAME_INPUT_IGNORED;
    }
    if (key == SDLK_RETURN || key == keypadEnterKey) {
        if (gameView->candidateMirrorRename.fieldMode ==
            DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT) {
            return m11_dm1_rename_apply_ascii(gameView, '\r');
        }
        return M11_GameView_ApplyMirrorCandidateRenameCommand(
                   gameView,
                   DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT)
                   ? M11_GAME_INPUT_REDRAW
                   : M11_GAME_INPUT_IGNORED;
    }
    return M11_GAME_INPUT_IGNORED;
}

static M12_MenuInput m11_poll_menu_input(M11_GameViewState* gameView,
                                         M12_StartupMenuState* menuState,
                                         const M12_GamepadMap* gamepadMap,
                                         M12_GamepadStatus* gamepadStatus,
                                         int useModernLauncher,
                                         M11_GameInputResult* gameViewResult,
                                         int* quitRequested,
                                         int* menuPointerChanged) {
    SDL_Event ev;
    int mappedX;
    int mappedY;
    if (gameViewResult) {
        *gameViewResult = M11_GAME_INPUT_IGNORED;
    }
    if (menuPointerChanged) {
        *menuPointerChanged = 0;
    }
    while (SDL_PollEvent(&ev)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        if (ev.type == SDL_EVENT_QUIT) {
            if (quitRequested) {
                *quitRequested = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
            ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
            M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            if (menuState) {
                menuState->settings.windowWidth = M11_Render_GetWindowWidth();
                menuState->settings.windowHeight = M11_Render_GetWindowHeight();
            }
            if (gameView && gameView->active && gameViewResult) {
                *gameViewResult = M11_GAME_INPUT_REDRAW;
            }
            continue;
        }
        if (ev.type == SDL_EVENT_GAMEPAD_ADDED ||
            ev.type == SDL_EVENT_GAMEPAD_REMOVED) {
            if (gamepadStatus) {
                M12_GamepadStatus_Update(gamepadStatus);
            }
            continue;
        }
        if (ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            M12_MenuInput gpadInput =
                m11_gamepad_button_input(gamepadMap,
                                         (SDL_GamepadButton)ev.gbutton.button,
                                         gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if (ev.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
            M12_MenuInput gpadInput =
                m11_gamepad_axis_input(gamepadMap,
                                       (SDL_GamepadAxis)ev.gaxis.axis,
                                       ev.gaxis.value,
                                       gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_MOTION &&
            gameView && gameView->active) {
            if (gameViewResult &&
                M11_Render_MapWindowToFramebuffer((int)ev.motion.x,
                                                  (int)ev.motion.y,
                                                  &mappedX,
                                                  &mappedY)) {
                m11_map_presented_game_point_to_source(gameView,
                                                       &mappedX,
                                                       &mappedY);
                *gameViewResult = M11_GameView_HandlePointerMove(
                    gameView,
                    mappedX,
                    mappedY);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_MOTION &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active)) {
            int lx, ly;
            if (m11_map_window_to_launcher((int)ev.motion.x, (int)ev.motion.y,
                                           1, &lx, &ly)) {
                M12_ModernMenu_HandlePointer(menuState, lx, ly, 0, NULL);
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            gameView && gameView->active &&
            (ev.button.button == SDL_BUTTON_LEFT || ev.button.button == SDL_BUTTON_RIGHT)) {
            if (gameViewResult &&
                M11_Render_MapWindowToFramebuffer((int)ev.button.x,
                                                  (int)ev.button.y,
                                                  &mappedX,
                                                  &mappedY)) {
                m11_map_presented_game_point_to_source(gameView, &mappedX, &mappedY);
                *gameViewResult = M11_GameView_HandlePointerButton(
                    gameView,
                    mappedX,
                    mappedY,
                    ev.button.button == SDL_BUTTON_RIGHT
                        ? M11_DM1_MOUSE_MASK_RIGHT
                        : M11_DM1_MOUSE_MASK_LEFT);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active) &&
            ev.button.button == SDL_BUTTON_LEFT) {
            int lx, ly;
            if (m11_map_window_to_launcher((int)ev.button.x, (int)ev.button.y,
                                           1, &lx, &ly)) {
                int changed = M12_ModernMenu_HandlePointer(menuState,
                                                           lx, ly, 1, NULL);
                if (changed && menuPointerChanged) {
                    *menuPointerChanged = 1;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_TEXT_INPUT &&
            m11_dm1_rename_consume_text_input(gameView,
                                              ev.text.text,
                                              gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_KEY_DOWN) {
            if (m11_dm1_rename_text_input_active(gameView)) {
                M11_GameInputResult renameResult =
                    m11_dm1_rename_handle_keydown(gameView,
                                                  (int)ev.key.key,
                                                  SDLK_KP_ENTER);
                if (gameViewResult) {
                    *gameViewResult = renameResult;
                }
                return M12_MENU_INPUT_NONE;
            }
            if (m11_game_view_is_csb(gameView)) {
                M12_MenuInput csbInput = M12_MENU_INPUT_NONE;
                if (m11_csb_sdl_key_to_menu_input((int)ev.key.key,
                                                  (ev.key.mod & SDL_KMOD_CTRL) != 0,
                                                  &csbInput)) {
                    return csbInput;
                }
            }
            if (gameView && gameView->active) {
                M12_MenuInput mappedInput = M12_MENU_INPUT_NONE;
                if ((ev.key.mod & SDL_KMOD_CTRL) && ev.key.scancode == SDL_SCANCODE_S) {
                    return M12_MENU_INPUT_SAVE_GAME;
                }
                if (m11_game_view_is_dm1(gameView)) {
                    mappedInput =
                        m11_dm1_v1_fixed_turn_input_from_scancode(ev.key.scancode);
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                }
                mappedInput = m11_motion_input_from_scancode(ev.key.scancode);
                if (mappedInput != M12_MENU_INPUT_NONE) {
                    return mappedInput;
                }
            }
            switch (ev.key.key) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                /* ReDMCSB PC-34/I34E source lock: COMMAND.C:677-684 maps
                 * keypad scancodes 0x4B/0x4C/0x4D/0x4F/0x50/0x51 to
                 * turn-left/forward/turn-right/strafe-left/back/strafe-right;
                 * IO2.C:47-59 normalizes shifted arrow scancodes into the
                 * same command-table codes. SDL reports NumLock-on keypad
                 * keys as SDLK_KP_N, so route them explicitly before the
                 * generic q/e/wasd convenience aliases. */
                case SDLK_KP_5:
                    return M12_MENU_INPUT_UP;
                case SDLK_KP_2:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_KP_1:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_KP_3:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                /* v2.8.x: arrow Left/Right now mean strafe (matches
                 * the original DM1 PC 3.4 convention; see also the
                 * user's keyboard-mapping request).  Q/E + Home/End +
                 * KP_4/KP_6 produce the turn-left/turn-right input
                 * tokens.  KP_4/KP_6 stay turn-left/turn-right
                 * because that's what COMMAND.C:677-684 maps them to
                 * on the original PC 3.4 keyboard (source-locked by
                 * test_dm1_v1_input_command_queue_pc34_compat). */
                case SDLK_LEFT:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_RIGHT:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_KP_4:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_KP_6:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_Q:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_E:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_HOME:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_END:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_A:
                    /* v2.9.x: WASD mirrors the arrow keys unconditionally.
                     * Per the user's request, WASD should always work
                     * exactly like arrow keys for navigation. The
                     * wasdMovementEnabled settings toggle is preserved
                     * as legacy state for backward-compatible config
                     * files but is no longer consulted here -- the
                     * settings menu entry has been removed and a
                     * migration warning is logged once on first launch
                     * if the user had previously disabled it. */
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_D:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_W:
                    return M12_MENU_INPUT_UP;
                case SDLK_S:
                    if (ev.key.mod & SDL_KMOD_CTRL) {
                        if (gameView && gameView->active)
                            return M12_MENU_INPUT_SAVE_GAME;
                        return M12_MENU_INPUT_NONE;
                    }
                    return M12_MENU_INPUT_DOWN;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
                case SDLK_SPACE:
                    return M12_MENU_INPUT_ACTION;
                case SDLK_TAB:
                    return M12_MENU_INPUT_CYCLE_CHAMPION;
                case SDLK_F5: {
                    /* F5 = quick save */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickSave(gameView)) {
                            fprintf(stderr, "SAVE: quicksave written\n");
                        } else {
                            fprintf(stderr, "SAVE FAILED: quicksave rejected\n");
                        }
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F9: {
                    /* F9 = quick load */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickLoad(gameView)) {
                            fprintf(stderr, "LOAD: quicksave restored\n");
                            if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                        } else {
                            fprintf(stderr, "LOAD FAILED: quicksave rejected\n");
                        }
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F6: {
                    int m = M11_QolRuntime_CycleSpeedMultiplier();
                    fprintf(stderr, "QoL: game speed %d%% (%.1fx)\n", m, m / 100.0);
                    if (gameViewResult && gameView && gameView->active) {
                        *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F7:
                    /* F7 = toggle dungeon minimap overlay */
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleMinimap();
                        fprintf(stderr, "QoL: minimap %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_F8:
                    /* F8 = export auto-map for current level
                     * (Ctrl+M alias: ReDMCSB M already triggers a
                     * fullscreen map overlay so we keep that key for
                     * the V1 chrome and add a dedicated export key.) */
                    if (gameView && gameView->active) {
                        int ex = DM1_AutoMap_ExportCurrentLevel(gameView);
                        fprintf(stderr, "QoL: auto-map export %s\n",
                                ex ? "ok" : "FAILED");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_L:
                    /* L = toggle combat log overlay */
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleCombatLog();
                        fprintf(stderr, "QoL: combat log %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_R:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_REST_TOGGLE;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_X:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_USE_STAIRS;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_G:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_PICKUP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_P:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_DROP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_1:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_1;
                    return M12_MENU_INPUT_NONE;
                case SDLK_2:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_2;
                    return M12_MENU_INPUT_NONE;
                case SDLK_3:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_3;
                    return M12_MENU_INPUT_NONE;
                case SDLK_4:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_4;
                    return M12_MENU_INPUT_NONE;
                case SDLK_5:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_5;
                    return M12_MENU_INPUT_NONE;
                case SDLK_6:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_6;
                    return M12_MENU_INPUT_NONE;
                case SDLK_C:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CAST;
                    return M12_MENU_INPUT_NONE;
                case SDLK_V:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CLEAR;
                    return M12_MENU_INPUT_NONE;
                case SDLK_U:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_USE_ITEM;
                    return M12_MENU_INPUT_NONE;
                case SDLK_M:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_MAP_TOGGLE;
                    return M12_MENU_INPUT_NONE;
                case SDLK_I:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_INVENTORY_TOGGLE;
                    return M12_MENU_INPUT_NONE;
                case SDLK_F10:
                    M11_Render_CycleScaleMode();
                    return M12_MENU_INPUT_NONE;
                case SDLK_F11:
                    M11_Render_ToggleFullscreen();
                    return M12_MENU_INPUT_NONE;
                default:
                    break;
            }
        }
#else
        if (ev.type == SDL_QUIT) {
            if (quitRequested) {
                *quitRequested = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_WINDOWEVENT &&
            ev.window.event == SDL_WINDOWEVENT_RESIZED) {
            M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            if (menuState) {
                menuState->settings.windowWidth = M11_Render_GetWindowWidth();
                menuState->settings.windowHeight = M11_Render_GetWindowHeight();
            }
            continue;
        }
        if (ev.type == SDL_CONTROLLERDEVICEADDED ||
            ev.type == SDL_CONTROLLERDEVICEREMOVED) {
            if (gamepadStatus) {
                M12_GamepadStatus_Update(gamepadStatus);
            }
            continue;
        }
        if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
            M12_MenuInput gpadInput =
                m11_gamepad_button_input(gamepadMap,
                                         (SDL_GamepadButton)ev.cbutton.button,
                                         gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if (ev.type == SDL_CONTROLLERAXISMOTION) {
            M12_MenuInput gpadInput =
                m11_gamepad_axis_input(gamepadMap,
                                       (SDL_GamepadAxis)ev.caxis.axis,
                                       ev.caxis.value,
                                       gameView && gameView->active);
            if (gpadInput != M12_MENU_INPUT_NONE) {
                return gpadInput;
            }
            continue;
        }
        if (ev.type == SDL_MOUSEMOTION &&
            gameView && gameView->active) {
            if (gameViewResult &&
                M11_Render_MapWindowToFramebuffer(ev.motion.x,
                                                  ev.motion.y,
                                                  &mappedX,
                                                  &mappedY)) {
                m11_map_presented_game_point_to_source(gameView,
                                                       &mappedX,
                                                       &mappedY);
                *gameViewResult = M11_GameView_HandlePointerMove(
                    gameView,
                    mappedX,
                    mappedY);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_MOUSEMOTION &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active)) {
            int lx, ly;
            if (m11_map_window_to_launcher(ev.motion.x, ev.motion.y,
                                           1, &lx, &ly)) {
                M12_ModernMenu_HandlePointer(menuState, lx, ly, 0, NULL);
            }
            continue;
        }
        if (ev.type == SDL_MOUSEBUTTONDOWN &&
            gameView && gameView->active &&
            (ev.button.button == SDL_BUTTON_LEFT || ev.button.button == SDL_BUTTON_RIGHT)) {
            if (gameViewResult &&
                M11_Render_MapWindowToFramebuffer(ev.button.x,
                                                  ev.button.y,
                                                  &mappedX,
                                                  &mappedY)) {
                m11_map_presented_game_point_to_source(gameView, &mappedX, &mappedY);
                *gameViewResult = M11_GameView_HandlePointerButton(
                    gameView,
                    mappedX,
                    mappedY,
                    ev.button.button == SDL_BUTTON_RIGHT
                        ? M11_DM1_MOUSE_MASK_RIGHT
                        : M11_DM1_MOUSE_MASK_LEFT);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_MOUSEBUTTONDOWN &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active) &&
            ev.button.button == SDL_BUTTON_LEFT) {
            int lx, ly;
            if (m11_map_window_to_launcher(ev.button.x, ev.button.y,
                                           1, &lx, &ly)) {
                int changed = M12_ModernMenu_HandlePointer(menuState,
                                                           lx, ly, 1, NULL);
                if (changed && menuPointerChanged) {
                    *menuPointerChanged = 1;
                }
            }
            continue;
        }
        if (ev.type == SDL_TEXTINPUT &&
            m11_dm1_rename_consume_text_input(gameView,
                                              ev.text.text,
                                              gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_KEYDOWN) {
            if (m11_dm1_rename_text_input_active(gameView)) {
                M11_GameInputResult renameResult =
                    m11_dm1_rename_handle_keydown(gameView,
                                                  (int)ev.key.keysym.sym,
                                                  SDLK_KP_ENTER);
                if (gameViewResult) {
                    *gameViewResult = renameResult;
                }
                return M12_MENU_INPUT_NONE;
            }
            if (m11_game_view_is_csb(gameView)) {
                M12_MenuInput csbInput = M12_MENU_INPUT_NONE;
                if (m11_csb_sdl_key_to_menu_input((int)ev.key.keysym.sym,
                                                  (ev.key.keysym.mod & KMOD_CTRL) != 0,
                                                  &csbInput)) {
                    return csbInput;
                }
            }
            if (gameView && gameView->active) {
                M12_MenuInput mappedInput = M12_MENU_INPUT_NONE;
                if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.scancode == SDL_SCANCODE_S) {
                    return M12_MENU_INPUT_SAVE_GAME;
                }
                if (m11_game_view_is_dm1(gameView)) {
                    mappedInput =
                        m11_dm1_v1_fixed_turn_input_from_scancode(ev.key.keysym.scancode);
                    if (mappedInput != M12_MENU_INPUT_NONE) {
                        return mappedInput;
                    }
                }
                mappedInput = m11_motion_input_from_scancode(ev.key.keysym.scancode);
                if (mappedInput != M12_MENU_INPUT_NONE) {
                    return mappedInput;
                }
            }
            switch (ev.key.keysym.sym) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                /* ReDMCSB PC-34/I34E source lock: COMMAND.C:677-684 maps
                 * keypad scancodes 0x4B/0x4C/0x4D/0x4F/0x50/0x51 to
                 * turn-left/forward/turn-right/strafe-left/back/strafe-right;
                 * IO2.C:47-59 normalizes shifted arrow scancodes into the
                 * same command-table codes. SDL reports NumLock-on keypad
                 * keys as SDLK_KP_N, so route them explicitly before the
                 * generic q/e/wasd convenience aliases. */
                case SDLK_KP_5:
                    return M12_MENU_INPUT_UP;
                case SDLK_KP_2:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_KP_1:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_KP_3:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                /* v2.8.x: arrow Left/Right now mean strafe (matches
                 * the original DM1 PC 3.4 convention; see also the
                 * user's keyboard-mapping request).  Q/E + Home/End +
                 * KP_4/KP_6 produce the turn-left/turn-right input
                 * tokens.  KP_4/KP_6 stay turn-left/turn-right
                 * because that's what COMMAND.C:677-684 maps them to
                 * on the original PC 3.4 keyboard (source-locked by
                 * test_dm1_v1_input_command_queue_pc34_compat). */
                case SDLK_LEFT:
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_RIGHT:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_KP_4:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_KP_6:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_Q:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_E:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_HOME:
                    return M12_MENU_INPUT_TURN_LEFT;
                case SDLK_END:
                    return M12_MENU_INPUT_TURN_RIGHT;
                case SDLK_A:
                    /* v2.9.x: see the SDL3 branch above; WASD is now
                     * unconditional and mirrors the arrow keys. */
                    return M12_MENU_INPUT_STRAFE_LEFT;
                case SDLK_D:
                    return M12_MENU_INPUT_STRAFE_RIGHT;
                case SDLK_W:
                    return M12_MENU_INPUT_UP;
                case SDLK_S:
                    if (ev.key.keysym.mod & KMOD_CTRL) {
                        if (gameView && gameView->active)
                            return M12_MENU_INPUT_SAVE_GAME;
                        return M12_MENU_INPUT_NONE;
                    }
                    return M12_MENU_INPUT_DOWN;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
                case SDLK_SPACE:
                    return M12_MENU_INPUT_ACTION;
                case SDLK_TAB:
                    return M12_MENU_INPUT_CYCLE_CHAMPION;
                case SDLK_F5: {
                    /* F5 = quick save */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickSave(gameView)) {
                            fprintf(stderr, "SAVE: quicksave written\n");
                        } else {
                            fprintf(stderr, "SAVE FAILED: quicksave rejected\n");
                        }
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F9: {
                    /* F9 = quick load */
                    if (gameView && gameView->active) {
                        if (M11_GameView_QuickLoad(gameView)) {
                            fprintf(stderr, "LOAD: quicksave restored\n");
                            if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                        } else {
                            fprintf(stderr, "LOAD FAILED: quicksave rejected\n");
                        }
                    }
                    return M12_MENU_INPUT_NONE;
                }
                case SDLK_F7:
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleMinimap();
                        fprintf(stderr, "QoL: minimap %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_F8:
                    if (gameView && gameView->active) {
                        int ex = DM1_AutoMap_ExportCurrentLevel(gameView);
                        fprintf(stderr, "QoL: auto-map export %s\n",
                                ex ? "ok" : "FAILED");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_l:
                    if (gameView && gameView->active) {
                        int on = M11_QolRuntime_ToggleCombatLog();
                        fprintf(stderr, "QoL: combat log %s\n", on ? "on" : "off");
                        if (gameViewResult) *gameViewResult = M11_GAME_INPUT_REDRAW;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_r:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_REST_TOGGLE;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_x:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_USE_STAIRS;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_g:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_PICKUP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_p:
                    if (gameView && gameView->active) {
                        return M12_MENU_INPUT_DROP_ITEM;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_1:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_1;
                    return M12_MENU_INPUT_NONE;
                case SDLK_2:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_2;
                    return M12_MENU_INPUT_NONE;
                case SDLK_3:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_3;
                    return M12_MENU_INPUT_NONE;
                case SDLK_4:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_4;
                    return M12_MENU_INPUT_NONE;
                case SDLK_5:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_5;
                    return M12_MENU_INPUT_NONE;
                case SDLK_6:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_RUNE_6;
                    return M12_MENU_INPUT_NONE;
                case SDLK_c:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CAST;
                    return M12_MENU_INPUT_NONE;
                case SDLK_v:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_SPELL_CLEAR;
                    return M12_MENU_INPUT_NONE;
                case SDLK_u:
                    if (gameView && gameView->active)
                        return M12_MENU_INPUT_USE_ITEM;
                    return M12_MENU_INPUT_NONE;
                case SDLK_F10:
                    M11_Render_CycleScaleMode();
                    return M12_MENU_INPUT_NONE;
                case SDLK_F11:
                    M11_Render_ToggleFullscreen();
                    return M12_MENU_INPUT_NONE;
                default:
                    break;
            }
        }
#endif
    }
    return M12_MENU_INPUT_NONE;
}

int M11_PhaseA_Run(const M11_PhaseA_Options* opts) {
    M11_PhaseA_Options defaults;
    M11_PhaseA_SetDefaultOptions(&defaults);
    M11_PhaseA_Options runtimeOptions;
    const M11_PhaseA_Options* o;
    runtimeOptions = opts ? *opts : defaults;
    m11_apply_persisted_window_size(&runtimeOptions);
    /* Enable the disk-backed automation manifest only on request. It writes
     * ~/.firestaff/accessibility.json atomically from M11_GameView_Draw();
     * leaving it always-on made normal gameplay do per-frame filesystem I/O. */
    if (getenv("FS_ACCESSIBILITY")) {
        fs_ax_set_enabled(1);
    }
    o = &runtimeOptions;
    M12_StartupMenuState menuState;
    M11_GameViewState gameView;
    const char* scriptCursor = o->script;
    unsigned char* launcherFramebuffer = NULL;
    unsigned char* modernRgba = NULL;
    int useModern = 0;
    int quitRequested = 0;
    uint32_t idleAccumulatorMs = 0;
    M12_GamepadMap gamepadMap;
    M12_GamepadStatus gamepadStatus;
    enum { M11_DM1_V1_PENDING_MOTION_CAPACITY = 7 };
    M12_MenuInput pendingDm1V1MotionInputs[M11_DM1_V1_PENDING_MOTION_CAPACITY];
    int pendingDm1V1MotionHead = 0;
    int pendingDm1V1MotionCount = 0;

    int rc = M11_Render_Init(o->windowWidth, o->windowHeight, o->scaleMode);
    if (rc != M11_RENDER_OK) {
        return rc;
    }

    launcherFramebuffer = (unsigned char*)calloc((size_t)M11_LAUNCHER_FB_WIDTH,
                                                 (size_t)M11_LAUNCHER_FB_HEIGHT);
    if (!launcherFramebuffer) {
        M11_Render_Shutdown();
        return M11_RENDER_ERR_TEXTURE;
    }
    M12_StartupMenu_InitWithDataDir(&menuState, o->dataDir, o->gameId);
    menuState.settings.windowWidth = M11_Render_GetWindowWidth();
    menuState.settings.windowHeight = M11_Render_GetWindowHeight();
    useModern = m11_should_use_modern_launcher(&menuState);
    if (useModern) {
        modernRgba = (unsigned char*)calloc((size_t)M11_LAUNCHER_MODERN_WIDTH *
                                                (size_t)M11_LAUNCHER_MODERN_HEIGHT,
                                            4U);
        if (!modernRgba) {
            /* Fall back to legacy renderer on allocation failure rather
             * than aborting the launcher. */
            useModern = 0;
        }
    }
    /* Load PO translations based on system language.
     * M12_Config_GetAutoLanguageIndex: 0=en, 1=sv, 2=fr */
    {
        const char* langCodes[] = {"en", "sv", "fr"};
        const char* langCode = "en";
        int langIdx = M12_Config_GetAutoLanguageIndex();
        if (langIdx >= 0 && langIdx < 3) langCode = langCodes[langIdx];
        {
            char poPath[512];
            snprintf(poPath, sizeof(poPath), "%s/po/startup-menu.%s.po",
                     o->dataDir ? o->dataDir : ".", langCode);
            if (fs_po_load(poPath) <= 0) {
                char relPath[128];
                snprintf(relPath, sizeof(relPath), "po/startup-menu.%s.po", langCode);
                fs_po_load(relPath);
            }
        }
    }
    M11_GameView_Init(&gameView);
    M12_GamepadMap_SetDefaults(&gamepadMap);
    (void)M12_GamepadMap_Load(&gamepadMap);
    memset(&gamepadStatus, 0, sizeof(gamepadStatus));
    M12_GamepadStatus_Update(&gamepadStatus);
    int launchedEver = 0;
    int inputRedrawDrawCount = 0;
    int inputRedrawAfterViewportDirtyCount = 0;
    int lastInputRedrawAfterViewportDirty = 0;
    int exitAfterLaunch = getenv("FIRESTAFF_EXIT_AFTER_LAUNCH") != NULL;
    int failIfNoLaunch = getenv("FIRESTAFF_FAIL_IF_NO_LAUNCH") != NULL;
    int runRc = 0;
    M11_ApplyStartupMenuRuntime(&menuState);
    {
        M12_Config qolCfg;
        M12_Config_Load(&qolCfg, o->dataDir);
        M11_QolRuntime_InitFromConfig(&qolCfg);
    }

    /* Compute deadlines using millisecond ticks. SDL_GetTicks returns
       Uint64 in SDL3 and Uint32 in SDL2. Both are fine for our math. */
#if SDL_VERSION_ATLEAST(3, 0, 0)
    Uint64 start = SDL_GetTicks();
    Uint64 now = start;
    Uint64 lastLoopTick = start;
    const Uint64 duration = (Uint64)(o->durationMs < 0 ? 0 : o->durationMs);
    const Uint64 interval = (Uint64)(o->presentEveryMs < 1
                                         ? 1
                                         : o->presentEveryMs);
    Uint64 gameTickInterval = 200; /* DM1 V1 authentic PAL: 10 VBlanks * 20ms = 200ms (VBLANK.C:F0577, GAMELOOP.C:F0002) */
#else
    Uint32 start = SDL_GetTicks();
    Uint32 now = start;
    Uint32 lastLoopTick = start;
    const Uint32 duration = (Uint32)(o->durationMs < 0 ? 0 : o->durationMs);
    const Uint32 interval = (Uint32)(o->presentEveryMs < 1
                                         ? 1
                                         : o->presentEveryMs);
    Uint32 gameTickInterval = 200; /* DM1 V1 authentic PAL: 10 VBlanks * 20ms = 200ms (VBLANK.C:F0577, GAMELOOP.C:F0002) */
#endif

    /* Always present at least once so the window actually has content. */
    if (o->directLaunch) {
        if (!m11_prepare_direct_launch(&menuState, o->gameId)) {
            fprintf(stderr, "firestaff: game unavailable for --game: %s\n",
                    o->gameId ? o->gameId : "(null)");
            runRc = 2;
            goto cleanup;
        }
        /* CLI direct launch bypasses only the M12 menu. The launch still
         * enters through M11_GameView_OpenSelectedMenuEntry(), so DM1 keeps
         * the ReDMCSB TITLE/ENTRANCE order (TITLE.C F0437 before
         * ENTRANCE.C F0441). */
        if (!m11_open_requested_launch(&gameView, &menuState, &idleAccumulatorMs, o->dataDir)) {
            fprintf(stderr, "firestaff: direct launch failed for --game %s\n", o->gameId);
            runRc = 3;
            goto cleanup;
        }
        launchedEver = 1;
        if (exitAfterLaunch) {
            goto cleanup;
        }
    } else {
        m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
    }
    if (gameView.active) {
        M11_Render_Present();
    } else {
        m11_present_launcher(launcherFramebuffer, modernRgba, useModern);
    }
    int gameFrameNeedsPresent = 0;

    while (o->durationMs < 0 || (now - start) < duration) {
        M12_MenuInput input = M12_MENU_INPUT_NONE;
        M11_GameInputResult pointerResult = M11_GAME_INPUT_IGNORED;
        uint32_t tickBeforeEvents = gameView.world.gameTick;
        uint32_t tickBeforeInput = gameView.world.gameTick;

        {
            int speedMul = M11_QolRuntime_GetSpeedMultiplier();
            if (speedMul < 50)  speedMul = 50;
            if (speedMul > 400) speedMul = 400;
            /* Original 200ms tick divided by (multiplier/100).
             * 0.5x -> 400ms, 1x -> 200ms, 1.5x -> 133ms, 2x -> 100ms. */
#if SDL_VERSION_ATLEAST(3, 0, 0)
            gameTickInterval = (Uint64)((200 * 100 + speedMul / 2) / speedMul);
#else
            gameTickInterval = (Uint32)((200 * 100 + speedMul / 2) / speedMul);
#endif
            if (gameTickInterval < 1) gameTickInterval = 1;
        }
        now = SDL_GetTicks();
        if (gameView.active) {
            uint32_t loopDeltaMs = (uint32_t)(now - lastLoopTick);
            idleAccumulatorMs += loopDeltaMs;
            /* DM1 V1: feed elapsed time to VBlank simulation */
            DM1_V1_VBlankTiming_Update(&gameView.vblankTiming, loopDeltaMs);
            /* Session timer runtime handoff: tick the in-game runtime
             * once per ~1 second of active gameplay.  We round down
             * to whole seconds so the tick boundary is deterministic
             * across host framerates.  The runtime enforces its own
             * Off-mode + post-limit no-op semantics, so this loop is
             * safe even when the user has the Session Timer set to
             * Off or when the FORCED_PAUSE latch is already set. */
            if (loopDeltaMs >= 1000) {
                SessionTimerRuntimeEvent stEvent =
                    M11_GameView_TickSessionTimer(&gameView,
                                                  (int)(loopDeltaMs / 1000));
                (void)stEvent;
            }
        } else {
            idleAccumulatorMs = 0;
        }
        lastLoopTick = now;

        if (scriptCursor && *scriptCursor != '\0') {
            input = m11_next_script_input(&scriptCursor);
        }
        int menuPointerChanged = 0;
        if (input == M12_MENU_INPUT_NONE) {
            input = m11_poll_menu_input(&gameView,
                                        &menuState,
                                        &gamepadMap,
                                        &gamepadStatus,
                                        useModern,
                                        &pointerResult,
                                        &quitRequested,
                                        &menuPointerChanged);
        }
        if (quitRequested) {
            break;
        }
        if (menuPointerChanged && !gameView.active) {
            if (menuState.shouldExit) {
                break;
            }
            if (m11_open_requested_launch(&gameView, &menuState, &idleAccumulatorMs, o->dataDir)) {
                launchedEver = 1;
                if (gameView.active) {
                    gameFrameNeedsPresent = 1;
                }
                if (exitAfterLaunch) {
                    break;
                }
                continue;
            }
            M11_ApplyStartupMenuRuntime(&menuState);
            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
        }
        if (pointerResult != M11_GAME_INPUT_IGNORED) {
            if (pointerResult == M11_GAME_INPUT_RETURN_TO_MENU) {
                M11_GameView_Shutdown(&gameView);
                M11_GameView_Init(&gameView);
                pendingDm1V1MotionHead = 0;
                pendingDm1V1MotionCount = 0;
                idleAccumulatorMs = 0;
                M11_ApplyStartupMenuRuntime(&menuState);
                m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            } else if (pointerResult == M11_GAME_INPUT_RESTART_GAME) {
                pendingDm1V1MotionHead = 0;
                pendingDm1V1MotionCount = 0;
                if (m11_restart_current_launch(&gameView,
                                                &menuState,
                                                &idleAccumulatorMs,
                                                o->dataDir) &&
                    gameView.active) {
                    launchedEver = 1;
                    gameFrameNeedsPresent = 1;
                    if (exitAfterLaunch) {
                        break;
                    }
                } else {
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                }
            } else if (pointerResult == M11_GAME_INPUT_REDRAW) {
                M11_GameView_Draw(&gameView,
                                  M11_Render_GetFramebuffer(),
                                  M11_FB_WIDTH,
                                  M11_FB_HEIGHT);
                gameFrameNeedsPresent = 1;
                if (gameView.world.gameTick != tickBeforeEvents) {
                    idleAccumulatorMs = 0;
                }
            }
        }
        if (input == M12_MENU_INPUT_NONE &&
            pointerResult == M11_GAME_INPUT_IGNORED &&
            m11_game_view_is_dm1(&gameView) &&
            M11_GameView_Dm1V1SourceTickReadyForInput(&gameView)) {
            if (pendingDm1V1MotionCount > 0) {
                input = pendingDm1V1MotionInputs[pendingDm1V1MotionHead];
                pendingDm1V1MotionHead =
                    (pendingDm1V1MotionHead + 1) % M11_DM1_V1_PENDING_MOTION_CAPACITY;
                pendingDm1V1MotionCount--;
            } else {
                input = m11_held_motion_input_from_keyboard(&gameView);
                if (input == M12_MENU_INPUT_NONE) {
                    input = m11_held_motion_input_from_gamepad(&gameView,
                                                               &gamepadStatus,
                                                               &gamepadMap);
                }
            }
        }
        if (input != M12_MENU_INPUT_NONE) {
            tickBeforeInput = gameView.world.gameTick;
            if (gameView.active) {
                M11_GameInputResult result = M11_GAME_INPUT_IGNORED;
                if (M11_GameView_InputConsumesDm1V1SourceTick(&gameView, input) &&
                    !m11_dm1_v1_input_is_immediate_turn(input) &&
                    !M11_GameView_Dm1V1SourceTickReadyForInput(&gameView)) {
                    /* ReDMCSB COMMAND.C F0359/F0361 queues key commands while
                     * GAMELOOP.C waits for G0321.  COMMAND.C F0361 lines
                     * 1744-1768 admits up to C5 queued keyboard commands
                     * while reserving two queue slots; mirror that bounded
                     * pending shape here so quick Q/E/Home/End/keypad turn
                     * taps are not overwritten before the vblank gate opens. */
                    if (pendingDm1V1MotionCount < M11_DM1_V1_PENDING_MOTION_CAPACITY) {
                        int tail = (pendingDm1V1MotionHead + pendingDm1V1MotionCount) %
                                   M11_DM1_V1_PENDING_MOTION_CAPACITY;
                        pendingDm1V1MotionInputs[tail] = input;
                        pendingDm1V1MotionCount++;
                    }
                    input = M12_MENU_INPUT_NONE;
                } else {
                    result = M11_GameView_HandleInput(&gameView, input);
                }
                if (result == M11_GAME_INPUT_RETURN_TO_MENU) {
                    M11_GameView_Shutdown(&gameView);
                    M11_GameView_Init(&gameView);
                    pendingDm1V1MotionHead = 0;
                    pendingDm1V1MotionCount = 0;
                    idleAccumulatorMs = 0;
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                } else if (result == M11_GAME_INPUT_RESTART_GAME) {
                    pendingDm1V1MotionHead = 0;
                    pendingDm1V1MotionCount = 0;
                    if (m11_restart_current_launch(&gameView,
                                                    &menuState,
                                                    &idleAccumulatorMs,
                                                    o->dataDir) &&
                        gameView.active) {
                        launchedEver = 1;
                        gameFrameNeedsPresent = 1;
                        if (exitAfterLaunch) {
                            break;
                        }
                    } else {
                        M11_ApplyStartupMenuRuntime(&menuState);
                        m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                    }
                } else if (result == M11_GAME_INPUT_REDRAW) {
                    int redrawWasAfterViewportDirty =
                        gameView.lastDm1V1MovementPipelineResult.viewportDirty;
                    M11_GameView_Draw(&gameView,
                                      M11_Render_GetFramebuffer(),
                                      M11_FB_WIDTH,
                                      M11_FB_HEIGHT);
                    gameFrameNeedsPresent = 1;
                    inputRedrawDrawCount++;
                    if (redrawWasAfterViewportDirty) {
                        inputRedrawAfterViewportDirtyCount++;
                    }
                    lastInputRedrawAfterViewportDirty = redrawWasAfterViewportDirty;
                    if (gameView.world.gameTick != tickBeforeInput) {
                        idleAccumulatorMs = 0;
                    }
                }
            } else {
                int launchHandled = 0;
                /* Do not short-circuit Enter/Right on the top-level launcher into
                 * M11_GameView_OpenSelectedMenuEntry(). ReDMCSB waits at the entrance
                 * command surface (ENTRANCE.C:739-747 installs entrance input;
                 * COMMAND.C:2438-2451 handles Enter/Resume) and only the explicit
                 * launch row in M12 should request runtime handoff. */
                if (!launchHandled) {
                    if (input == M12_MENU_INPUT_CYCLE_CHAMPION ||
                        input == M12_MENU_INPUT_STRAFE_LEFT ||
                        input == M12_MENU_INPUT_STRAFE_RIGHT ||
                        input == M12_MENU_INPUT_PICKUP_ITEM ||
                        input == M12_MENU_INPUT_DROP_ITEM) {
                        input = M12_MENU_INPUT_NONE;
                    }
                    M12_StartupMenu_HandleInput(&menuState, input);
                    if (menuState.shouldExit) {
                        break;
                    }
                    if (m11_open_requested_launch(&gameView, &menuState, &idleAccumulatorMs, o->dataDir)) {
                        launchedEver = 1;
                        if (gameView.active) {
                            gameFrameNeedsPresent = 1;
                        }
                        if (exitAfterLaunch) {
                            break;
                        }
                        continue;
                    }
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                }
            }
        }
        while (gameView.active && idleAccumulatorMs >= (uint32_t)gameTickInterval) {
            if (M11_GameView_AdvanceIdleTick(&gameView) == M11_GAME_INPUT_REDRAW) {
                M11_GameView_Draw(&gameView,
                                  M11_Render_GetFramebuffer(),
                                  M11_FB_WIDTH,
                                  M11_FB_HEIGHT);
                gameFrameNeedsPresent = 1;
            }
            idleAccumulatorMs -= (uint32_t)gameTickInterval;
        }
        if (gameView.active) {
            /* Static DM1 V1 frames do not need a 60 Hz full redraw/present.
             * Resurrecting a champion enables the full HUD/champion render
             * path, so keep CPU bounded by presenting only after input,
             * resize, launch, restart, or a source tick dirties the frame. */
            if (gameFrameNeedsPresent) {
                DM1_AutoMap_RecordVisit(&gameView);
                DM1_Minimap_Render(&gameView,
                                   M11_Render_GetFramebuffer(),
                                   M11_FB_WIDTH, M11_FB_HEIGHT);
                DM1_CombatLog_Render(&gameView,
                                     M11_Render_GetFramebuffer(),
                                     M11_FB_WIDTH, M11_FB_HEIGHT);
                m11_present_game_frame(&gameView);
                gameFrameNeedsPresent = 0;
            }
        } else {
            /* Redraw the launcher every tick so animations (pulse,
             * hover) remain alive even without input. */
            menuState.frameTick += 1U;
            if (M12_StartupMenu_Update(&menuState)) {
                M11_ApplyStartupMenuRuntime(&menuState);
            }
            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            m11_present_launcher(launcherFramebuffer, modernRgba, useModern);
        }
        SDL_Delay((Uint32)interval);
        now = SDL_GetTicks();
    }

    if (failIfNoLaunch && !launchedEver) {
        fprintf(stderr, "firestaff: launch smoke failed: no launch reached before exit\n");
        runRc = 3;
    }
cleanup:
    M12_StartupMenu_Destroy(&menuState);
    m11_write_autotest_screenshot(getenv("FIRESTAFF_AUTOTEST_SCREENSHOT_DIR"));
    m11_write_autotest_presented_screenshot(getenv("FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR"));
    m11_write_autotest_runtime_probe(getenv("FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON"),
                                     launchedEver,
                                     &gameView,
                                     inputRedrawDrawCount,
                                     inputRedrawAfterViewportDirtyCount,
                                     lastInputRedrawAfterViewportDirty);
    m11_sync_and_save_window_size(&menuState);
    M12_GamepadStatus_Close(&gamepadStatus);
    M11_GameView_Shutdown(&gameView);
    free(launcherFramebuffer);
    if (modernRgba) {
        free(modernRgba);
    }
    M11_Render_Shutdown();
    return runRc;
}
