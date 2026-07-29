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
#include "startup_intro_m12.h"
#include "firestaff_version.h"
#include "menu_hit_m12.h"
#include "m11_game_view.h"
#include "firestaff_po_loader.h"
#include "firestaff_accessibility.h"
#include "firestaff_retroachievements.h"
#include "audio_sdl_m11.h"
#include "render_sdl_m11.h"
#include "dm1_v2_startup_title_filter_handoff_pc34.h"
#include "csb_v2_filter_config_pc34.h"
#include "m11_qol_runtime.h"
#include "dm1_v1_minimap_pc34_compat.h"
#include "dm1_v1_automap_pc34_compat.h"
#include "dm1_v1_combat_log_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "title_frontend_v1.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
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
#include "v1_swsh_intro_pathfinder_pc34_compat.h"
#include "v1_title_intro_pathfinder_pc34_compat.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v20_startup_presentation_timing_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "csb_v22_finished_art_material_gate_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
#include "csb_v1_boot.h"
#include "csb_v1_f0908_f0909_f0910_swsh_sound_pc34_compat.h"

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

uint32_t M11_GameView_IdleTickIntervalMs(const M11_GameViewState* gameView,
                                         int speedMultiplier) {
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT &&
        (gameView->csbState.startup_title_active ||
         gameView->csbState.startup_entrance_active)) {
        return 20u;
    }

    if (speedMultiplier < 50) speedMultiplier = 50;
    if (speedMultiplier > 400) speedMultiplier = 400;
    return (uint32_t)((200 * 100 + speedMultiplier / 2) / speedMultiplier);
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

int M11_ResolveScaleModeForWindowMode(int requestedScaleMode, int windowMode);
static int m11_running_from_macos_app_bundle(void);
static int m11_map_window_pointer_to_game_source(const M11_GameViewState* gameView,
                                                 int windowX,
                                                 int windowY,
                                                 int* outX,
                                                 int* outY);
static int m11_draw_entrance_credits_asset(M11_GameViewState* gameView,
                                           unsigned char* framebuffer) {
    (void)gameView;
    (void)framebuffer;
    return 0;
}
static int DM1_V1_Entrance_FullStartRenderReceiptHostReadyPc34Compat(
    const DM1_V1_EntranceFullStartRenderReceiptPc34* receipt) {
    return receipt && receipt->realAssetCaptureProof &&
           receipt->requiresGraphicsDat && receipt->noHostRenderInference;
}
static int DM1_V1_InputMenuTokenUsesHeldRepeatPc34Compat(int token) {
    return token == M12_MENU_INPUT_LEFT || token == M12_MENU_INPUT_RIGHT ||
           token == M12_MENU_INPUT_UP || token == M12_MENU_INPUT_DOWN ||
           token == M12_MENU_INPUT_STRAFE_LEFT ||
           token == M12_MENU_INPUT_STRAFE_RIGHT;
}
static int M12_StartupMenu_PrepareSelectedGameLaunch(
        M12_StartupMenuState* menuState) {
    return menuState != NULL;
}

static void m11_set_launch_failed_message(M12_StartupMenuState* menuState) {
    const M12_MenuEntry* entry = NULL;
    const char* gameId = NULL;
    if (!menuState) {
        return;
    }
    if (menuState->activatedIndex >= 0) {
        entry = M12_StartupMenu_GetEntry(menuState, menuState->activatedIndex);
    }
    if (!entry && menuState->selectedIndex >= 0) {
        entry = M12_StartupMenu_GetEntry(menuState, menuState->selectedIndex);
    }
    gameId = entry ? entry->gameId : NULL;
    menuState->launchRequested = 0;
    menuState->view = M12_MENU_VIEW_MESSAGE;
    if (gameId && strcmp(gameId, "nexus") == 0) {
        menuState->messageLine1 = "NEXUS LOAD FAILED";
        menuState->messageLine2 = "CHECK ISO/BIN OR EXTRACTED FILES";
    } else if (gameId && strcmp(gameId, "theron") == 0) {
        /* This generic launcher path covers both invalid media and a valid
         * Track 02 whose later original graphics route is still unavailable.
         * The message must not imply that a hash-verified CUE/BIN is corrupt.
         */
        menuState->messageLine1 = "THERON STARTUP FAILED";
        menuState->messageLine2 = "VERIFY CUE/BIN AND STARTUP DETAILS";
    } else if (gameId && strcmp(gameId, "dm2") == 0) {
        menuState->messageLine1 = "DM2 LOAD FAILED";
        menuState->messageLine2 = "CHECK GRAPHICS/DUNGEON DATA";
    } else if (gameId && strcmp(gameId, "csb") == 0) {
        menuState->messageLine1 = "CSB LOAD FAILED";
        menuState->messageLine2 = "CHECK GRAPHICS/DUNGEON DATA";
    } else {
        menuState->messageLine1 = "DUNGEON LOAD FAILED";
        menuState->messageLine2 = "CHECK DUNGEON.DAT";
    }
    menuState->messageLine3 = "ESC RETURNS TO MENU";
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

static int m11_play_firestaff_startup_intro(void) {
    unsigned char* rgba;
    char resourcePath[1024];
    const char* basePath;
    Uint64 started;
    rgba = (unsigned char*)malloc((size_t)M12_STARTUP_INTRO_WIDTH *
                                  (size_t)M12_STARTUP_INTRO_HEIGHT * 4U);
    if (!rgba) return 0;
    (void)M12_StartupIntro_LoadBackground("assets/branding/firestaff-startup-intro.ppm");
    basePath = SDL_GetBasePath();
    if (basePath) {
        snprintf(resourcePath, sizeof(resourcePath), "%sfirestaff-startup-intro.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        snprintf(resourcePath, sizeof(resourcePath), "%s../Resources/firestaff-startup-intro.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        snprintf(resourcePath, sizeof(resourcePath), "%s../share/firestaff/firestaff-startup-intro.ppm", basePath);
        (void)M12_StartupIntro_LoadBackground(resourcePath);
        SDL_free((void*)basePath);
    }
    (void)M12_StartupIntro_LoadBackground("/usr/share/firestaff/firestaff-startup-intro.ppm");
    started = SDL_GetTicks();
    while ((SDL_GetTicks() - started) < M12_STARTUP_INTRO_DURATION_MS) {
        Uint64 elapsed = SDL_GetTicks() - started;
        M12_StartupIntro_Render(rgba,
                                M12_STARTUP_INTRO_WIDTH,
                                M12_STARTUP_INTRO_HEIGHT,
                                (uint32_t)elapsed,
                                M12_STARTUP_INTRO_DURATION_MS,
                                FIRESTAFF_VERSION_NUMBER);
        (void)M11_Render_PresentRGBA(rgba,
                                     M12_STARTUP_INTRO_WIDTH,
                                     M12_STARTUP_INTRO_HEIGHT);
        if (M11_Render_PumpEvents()) {
            free(rgba);
            return 1;
        }
        SDL_Delay(33U);
    }
    free(rgba);
    return 0;
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

static int m11_map_window_pointer_to_game_source(
    const M11_GameViewState* gameView,
    int windowX,
    int windowY,
    int* outX,
    int* outY)
{
    int framebufferX;
    int framebufferY;

    if (!gameView || !outX || !outY ||
        !M11_Render_MapWindowToFramebuffer(windowX, windowY,
                                           &framebufferX, &framebufferY)) {
        return 0;
    }
    /* V1_ORIGINAL returns 0 (pass-through): framebuffer coords ARE
     * source coords (320x200).  Only fail on NULL pointers. */
    (void)M11_MapPresentedGamePointToSourceForPresentation(
            gameView->presentationMode,
            gameView->presentationWidth,
            gameView->presentationHeight,
            &framebufferX,
            &framebufferY);
    *outX = framebufferX;
    *outY = framebufferY;
    return 1;
}

static int m11_dm1_v20_presentation_active(const M11_GameViewState* gameView) {
    return gameView &&
        gameView->presentationMode == M12_PRESENTATION_V20_FILTERED &&
        (gameView->sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG ||
         gameView->sourceKind == M11_GAME_SOURCE_CUSTOM_DUNGEON ||
         gameView->sourceKind == M11_GAME_SOURCE_DIRECT_DUNGEON);
}

static int m11_present_game_frame(const M11_GameViewState* gameView,
                                  const unsigned char** outPresentedFrame) {
    int scale = M11_GameView_PresentationIndexedScale(
        gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL);
    int specialPalette =
        M11_GameView_GetPresentationSpecialPalette(gameView);
    int targetW = M11_FB_WIDTH;
    int targetH = M11_FB_HEIGHT;
    int requestedFilter = M11_Render_GetScaleFilter();
    int effectiveFilter = M11_ResolveGameScaleFilterForPresentation(
        gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
        requestedFilter);
    int restoreFilter = 0;
    int result;
    int dm1_v20_active = m11_dm1_v20_presentation_active(gameView);
    const unsigned char* presented_frame = M11_Render_GetFramebuffer();
    int csb_v20_active = gameView &&
        gameView->presentationMode == M12_PRESENTATION_V20_FILTERED &&
        gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT;

    if (outPresentedFrame) {
        *outPresentedFrame = NULL;
    }
    if (csb_v20_active && presented_frame) {
        static unsigned char csb_v20_scratch[M11_FB_BYTES];
        memcpy(csb_v20_scratch, presented_frame, sizeof(csb_v20_scratch));
        (void)csb_v2_filter_chain_apply_indexed(csb_v20_scratch,
                                                 M11_FB_WIDTH, M11_FB_HEIGHT);
        presented_frame = csb_v20_scratch;
    }

    /* M12 persists V2.0 preferences globally, but those post-filters are
     * only valid for the DM1 V2.0 framebuffer route. */
    M11_Render_SetV2PresentationActive(dm1_v20_active);
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
    if (specialPalette >= 0) {
        if (gameView &&
            gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
            (void)M11_GameView_PresentationTarget(
                gameView->presentationMode,
                gameView->presentationWidth,
                gameView->presentationHeight,
                &targetW,
                &targetH);
            result = M11_Render_PresentEpxIndexedToResolutionWithSpecialPalette(
                presented_frame, M11_FB_WIDTH, M11_FB_HEIGHT,
                targetW, targetH, specialPalette);
            if (restoreFilter) {
                M11_Render_SetScaleFilter(requestedFilter);
            }
            if (result == M11_RENDER_OK && outPresentedFrame) {
                *outPresentedFrame = presented_frame;
            }
            return result == M11_RENDER_OK;
        }
        if (M11_GameView_PresentationTarget(
                gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
                gameView ? gameView->presentationWidth : 0,
                gameView ? gameView->presentationHeight : 0,
                &targetW,
                &targetH)) {
            result = M11_Render_PresentIndexedToResolutionWithSpecialPalette(
                presented_frame,
                M11_FB_WIDTH,
                M11_FB_HEIGHT,
                targetW,
                targetH,
                specialPalette);
        } else {
            result = M11_Render_PresentIndexedWithSpecialPalette(
                presented_frame,
                M11_FB_WIDTH,
                M11_FB_HEIGHT,
                specialPalette);
        }
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    if (gameView &&
        gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
        (void)M11_GameView_PresentationTarget(gameView->presentationMode,
                                               gameView->presentationWidth,
                                               gameView->presentationHeight,
                                               &targetW,
                                               &targetH);
        result = M11_Render_PresentEpxIndexedToResolution(presented_frame,
                                                           M11_FB_WIDTH,
                                                           M11_FB_HEIGHT,
                                                           targetW,
                                                           targetH);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    if (scale > 1) {
        result = M11_Render_PresentScaledIndexed(presented_frame,
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 scale);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    if (M11_GameView_PresentationTarget(gameView ? gameView->presentationMode : M12_PRESENTATION_V1_ORIGINAL,
                                         gameView ? gameView->presentationWidth : 0,
                                         gameView ? gameView->presentationHeight : 0,
                                         &targetW,
                                         &targetH)) {
        result = M11_Render_PresentIndexedToResolution(presented_frame,
                                                       M11_FB_WIDTH,
                                                       M11_FB_HEIGHT,
                                                       targetW,
                                                       targetH);
        if (restoreFilter) {
            M11_Render_SetScaleFilter(requestedFilter);
        }
        if (result == M11_RENDER_OK && outPresentedFrame) {
            *outPresentedFrame = presented_frame;
        }
        return result == M11_RENDER_OK;
    }
    result = M11_Render_Present();
    if (restoreFilter) {
        M11_Render_SetScaleFilter(requestedFilter);
    }
    if (result == M11_RENDER_OK && outPresentedFrame) {
        *outPresentedFrame = presented_frame;
    }
    return result == M11_RENDER_OK;
}

static void m11_record_csb_presented_frame(M11_GameViewState *gameView)
{
    int width = 0;
    int height = 0;
    const unsigned char *rgba;

    if (!gameView || gameView->sourceKind != M11_GAME_SOURCE_CSB_BOOT) {
        return;
    }
    (void)M11_Render_GetPresentedRGBA(&width, &height);
    M11_GameView_RecordCSBPresentedIndexedFrame(
        gameView,
        M11_Render_GetFramebuffer(),
        M11_FB_WIDTH,
        M11_FB_HEIGHT,
        m11_running_from_macos_app_bundle(),
        width > 0 && height > 0);
}

static void m11_publish_dm1_hoc_presented_capture_to_m12(
    const M11_GameViewState* gameView,
    M12_StartupMenuState* menuState) {
    M11_BootProbeReceipt boot;
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34 export_receipt;
    M12_DM1HoCPresentedCaptureReceipt capture;

    if (!gameView || !menuState ||
        !M11_GameView_GetBootProbeReceipt(gameView, &boot) ||
        strcmp(boot.sourceId, "dm1") != 0) {
        return;
    }

    memset(&export_receipt, 0, sizeof(export_receipt));
    if (!dm1_v1_startup_hoc_presented_capture_host_export_from_boot_summary_pc34(
            &boot.dm1HoCBootSummary,
            &export_receipt) ||
        !export_receipt.ready) {
        return;
    }

    memset(&capture, 0, sizeof(capture));
    capture.handled = export_receipt.handled;
    capture.presentedCaptureReady = export_receipt.presented_capture_ready;
    capture.hostWindowPresent = export_receipt.host_window_present;
    capture.capturedFromMacWindow = export_receipt.captured_from_mac_window;
    capture.capturedFromReleaseApp = export_receipt.captured_from_release_app;
    capture.width = export_receipt.width;
    capture.height = export_receipt.height;
    capture.byteCount = export_receipt.byte_count;
    capture.framebufferHash = export_receipt.framebuffer_hash;
    capture.consumerMask = export_receipt.consumer_mask;
    capture.chainHash = export_receipt.chain_hash;
    (void)M12_StartupMenu_SetDM1HoCPresentedCaptureReceipt(menuState,
                                                           &capture);
}

static int m11_running_from_macos_app_bundle(void)
{
#ifdef __APPLE__
    const char *base_path = SDL_GetBasePath();
    int running_from_bundle =
        base_path && strstr(base_path, ".app/Contents/MacOS/") != NULL;
#if !SDL_VERSION_ATLEAST(3, 0, 0)
    /* SDL2 allocates this string; SDL3 returns a library-owned const path. */
    SDL_free((void *)base_path);
#endif
    return running_from_bundle;
#else
    return 0;
#endif
}

/* Opt-in evidence capture for the actual post-present SDL surface. The
 * capture is downstream of the source-raster and RGBA palette gates; it
 * never promotes a screenshot into source evidence. One file per source
 * palette phase avoids writing one capture for every startup VBlank. */
static void m11_capture_csb_presented_source_phase(int special_palette)
{
    const char *capture_dir = getenv("FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR");
    static unsigned int captured_palette_mask;
    char output_path[1024];
    unsigned int palette_bit;

    if (!capture_dir || !capture_dir[0] || special_palette < 0 ||
        special_palette >= (int)(sizeof(captured_palette_mask) * 8u)) {
        return;
    }
    palette_bit = 1u << (unsigned int)special_palette;
    if (captured_palette_mask & palette_bit) {
        return;
    }
    if (!M11_Screenshot_CapturePresentedRGBA(capture_dir, output_path,
                                              (int)sizeof(output_path))) {
        fprintf(stderr, "firestaff: CSB presented capture failed: %s\n",
                capture_dir);
        return;
    }
    captured_palette_mask |= palette_bit;
    fprintf(stderr, "CSB PRESENTED SOURCE CAPTURE: palette=%d %s\n",
            special_palette, output_path);
}

/* CSB V2.0 has two source-preserving presentation passes: indexed cleanup
 * happens before palette conversion in m11_present_game_frame(), while CRT
 * scanlines operate on the final RGBA surface. Keep the second pass here,
 * after the normal M11 present, so it uses the actual target dimensions and
 * never mutates the source-owned indexed framebuffer or its palette receipt. */
static int m11_present_csb_v20_rgba_filter_if_enabled(
    const M11_GameViewState *gameView,
    const unsigned char **out_rgba,
    int *out_width,
    int *out_height)
{
    const unsigned char *presented_rgba;
    unsigned char *filtered_rgba;
    const CSB_V2_FilterConfig *filter_config;
    size_t bytes;
    int width = 0;
    int height = 0;

    if (!gameView ||
        gameView->sourceKind != M11_GAME_SOURCE_CSB_BOOT ||
        gameView->presentationMode != M12_PRESENTATION_V20_FILTERED) {
        return 1;
    }
    filter_config = csb_v2_filter_config_get();
    if (!filter_config || !filter_config->crtScanlinesEnabled) {
        return 1;
    }
    presented_rgba = M11_Render_GetPresentedRGBA(&width, &height);
    if (!presented_rgba || width <= 0 || height <= 0 ||
        (size_t)width > (size_t)-1 / (size_t)height ||
        (size_t)width * (size_t)height > (size_t)-1 / 4u) {
        return 0;
    }
    bytes = (size_t)width * (size_t)height * 4u;
    filtered_rgba = (unsigned char *)malloc(bytes);
    if (!filtered_rgba) {
        return 0;
    }
    memcpy(filtered_rgba, presented_rgba, bytes);
    if (csb_v2_filter_chain_apply_rgba(filtered_rgba, width, height) > 0 &&
        M11_Render_PresentRGBA(filtered_rgba, width, height) != M11_RENDER_OK) {
        free(filtered_rgba);
        return 0;
    }
    free(filtered_rgba);
    if (out_rgba) {
        *out_rgba = M11_Render_GetPresentedRGBA(&width, &height);
    }
    if (out_width) {
        *out_width = width;
    }
    if (out_height) {
        *out_height = height;
    }
    return 1;
}

/* ReDMCSB/CSBWin startup hands source-owned pages to the host before input.
 * Capture may observe only a completed SDL presentation, never a pre-upload
 * framebuffer or a merely allocated window. */
static int m11_present_game_frame_and_publish_startup_capture(
    const M11_GameViewState* gameView,
    M12_StartupMenuState* menuState) {
    const unsigned char* presented_frame = NULL;
    const unsigned char* presented_rgba = NULL;
    int presented_width = 0;
    int presented_height = 0;
    int mac_window_capture_ready = 0;
    int csb_source_output_matches = 0;
    int csb_special_palette = -1;

    if (!m11_present_game_frame(gameView, &presented_frame)) {
        return 0;
    }
    /* M11_Render_GetPresentedRGBA is populated in the renderer immediately
     * before SDL_RenderPresent.  Do not promote a CSB startup receipt when
     * that actual host buffer is absent, even if SDL still has a window. */
    presented_rgba = M11_Render_GetPresentedRGBA(&presented_width,
                                                  &presented_height);
    if (!m11_present_csb_v20_rgba_filter_if_enabled(gameView,
                                                    &presented_rgba,
                                                    &presented_width,
                                                    &presented_height)) {
        return 0;
    }
#ifdef __APPLE__
    mac_window_capture_ready =
        M11_Render_GetWindow() != NULL && presented_rgba != NULL &&
        presented_width > 0 && presented_height > 0;
#endif
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT &&
        presented_frame) {
        csb_special_palette =
             M11_GameView_GetPresentationSpecialPalette(gameView);
        csb_source_output_matches = csb_special_palette >= 0 &&
            M11_GameView_CSBPresentedFrameMatchesCurrentSource(
                gameView, presented_frame, M11_FB_WIDTH, M11_FB_HEIGHT,
                csb_special_palette) &&
            M11_Render_PresentedIndexedSpecialMatches(
                presented_frame, M11_FB_WIDTH, M11_FB_HEIGHT,
                csb_special_palette);
    }
    if (gameView && gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT &&
        presented_frame && presented_rgba && presented_width > 0 &&
        presented_height > 0 && csb_source_output_matches) {
        M11_GameView_RecordCSBPresentedIndexedFrame(
            (M11_GameViewState *)gameView,
            presented_frame,
            M11_FB_WIDTH,
            M11_FB_HEIGHT,
            m11_running_from_macos_app_bundle(),
            mac_window_capture_ready
        );
        m11_capture_csb_presented_source_phase(csb_special_palette);
    }
    m11_publish_dm1_hoc_presented_capture_to_m12(gameView, menuState);
    return 1;
}

void M11_ApplyStartupMenuRuntime(M12_StartupMenuState* menuState) {
    int requestedWindowMode;
    int requestedScaleMode;
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
    requestedScaleMode = menuState->settings.scaleModeIndex;
    requestedScaleMode = M11_ResolveScaleModeForWindowMode(requestedScaleMode,
                                                            requestedWindowMode);
    if (requestedWindowMode != M11_WINDOW_MODE_WINDOWED &&
        menuState->settings.scaleModeIndex != requestedScaleMode) {
        menuState->settings.scaleModeIndex = requestedScaleMode;
    }
    if (M11_Render_GetPaletteLevel() != M12_StartupMenu_GetRenderPaletteLevel(menuState)) {
        M11_Render_SetPaletteLevel(M12_StartupMenu_GetRenderPaletteLevel(menuState));
    }
    if (M11_Render_GetWindowMode() != requestedWindowMode) {
        M11_Render_SetWindowMode(requestedWindowMode);
    }
    if (M11_Render_GetScaleMode() != requestedScaleMode) {
        M11_Render_SetScaleMode(requestedScaleMode);
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

static void m11_sync_runtime_graphics_popup_to_menu(
    const M11_GameViewState* gameView,
    M12_StartupMenuState* menuState)
{
    M12_Config config;
    int slot = 0;
    if (!gameView || !menuState) return;
    if (gameView->sourceKind == M11_GAME_SOURCE_CSB_BOOT) slot = 1;
    else if (gameView->sourceKind == M11_GAME_SOURCE_DM2_BOOT) slot = 2;
    else if (gameView->sourceKind == M11_GAME_SOURCE_NEXUS_DGN) slot = 3;
    else if (gameView->sourceKind == M11_GAME_SOURCE_THERON_TRACK02) slot = 4;
    M12_Config_Load(&config, NULL);
    menuState->settings.graphicsIndex = config.graphicsIndex;
    menuState->settings.windowModeIndex = config.windowModeIndex;
    menuState->settings.scaleModeIndex = config.scaleModeIndex;
    menuState->settings.displayAspectMode = config.displayAspectMode;
    menuState->settings.integerScaling = config.integerScaling;
    menuState->settings.scalingFilterIndex = config.scalingFilterIndex;
    menuState->settings.vsyncIndex = config.vsyncIndex;
    menuState->settings.showFpsOverlay = config.showFpsOverlay ? 1 : 0;
    /* M12_SaveConfig serializes these fields from menuState.  Keep that
     * shadow copy current after a live M11 edit so returning to the launcher
     * cannot write pre-popup V2 values back over the saved config. */
    menuState->settings.dm1V2SmoothingEnabled = config.dm1V2SmoothingEnabled;
    menuState->settings.dm1V2DynamicLightingEnabled = config.dm1V2DynamicLightingEnabled;
    menuState->settings.dm1V2SmoothTurnPanEnabled = config.dm1V2SmoothTurnPanEnabled;
    menuState->settings.dm1V2CrtScanlinesEnabled = config.dm1V2CrtScanlinesEnabled;
    menuState->settings.dm1V2CrtScanlineStrength = config.dm1V2CrtScanlineStrength;
    menuState->settings.dm1V2PaletteCorrectionEnabled = config.dm1V2PaletteCorrectionEnabled;
    menuState->settings.dm1V2PaletteGamma = config.dm1V2PaletteGamma;
    menuState->settings.dm1V2PaletteBrightness = config.dm1V2PaletteBrightness;
    menuState->settings.dm1V2PaletteContrast = config.dm1V2PaletteContrast;
    menuState->settings.dm1V2DitherCleanupEnabled = config.dm1V2DitherCleanupEnabled;
    menuState->settings.dm1V2SharpeningEnabled = config.dm1V2SharpeningEnabled;
    menuState->settings.dm1V2SharpeningStrength = config.dm1V2SharpeningStrength;
    menuState->settings.dm1V2PhosphorPersistenceEnabled = config.dm1V2PhosphorPersistenceEnabled;
    menuState->settings.dm1V2PhosphorDecay = config.dm1V2PhosphorDecay;
    menuState->settings.dm1V2ColorPreset = config.dm1V2ColorPreset;
    menuState->settings.dm1V2PixelGridEnabled = config.dm1V2PixelGridEnabled;
    menuState->settings.dm1V2PixelGridIntensity = config.dm1V2PixelGridIntensity;
    menuState->settings.dm1V2MotionBlurEnabled = config.dm1V2MotionBlurEnabled;
    menuState->settings.dm1V2MotionBlurStrength = config.dm1V2MotionBlurStrength;
    menuState->gameOptions[slot].presentationModeIndex = config.graphicsIndex;
    menuState->gameOptions[slot].aspectRatio = config.gameAspectRatio[slot];
    menuState->gameOptions[slot].resolution = config.gameResolution[slot];
}

int M11_ResolveScaleModeForWindowMode(int requestedScaleMode, int windowMode) {
    /* A fixed 1x--4x framebuffer inside a maximized Cocoa window leaves
     * DM1's 320x200 V1 frame looking like a thumbnail. M11 is the shared
     * host for all game views, so maximized/fullscreen presentation uses FIT
     * unless the user explicitly selected STRETCH. Windowed mode retains
     * fixed-scale choices for pixel inspection. */
    if (windowMode != M11_WINDOW_MODE_WINDOWED &&
        requestedScaleMode >= M11_SCALE_1X &&
        requestedScaleMode <= M11_SCALE_4X) {
        return M11_SCALE_FIT;
    }
    return requestedScaleMode;
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

static void m11_sync_startup_text_input(const M12_StartupMenuState* menuState,
                                        int launcherOwnsInput,
                                        int* hostTextInputActive) {
    M12_StartupTextInputHostReceipt receipt;
    int active;
    if (!hostTextInputActive) {
        return;
    }
    active = *hostTextInputActive;
    if (!launcherOwnsInput) {
        if (active) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
            (void)SDL_StopTextInput(M11_Render_GetWindow());
#else
            SDL_StopTextInput();
#endif
            *hostTextInputActive = 0;
        }
        return;
    }
    if (!M12_StartupMenu_TextInputHostReceipt(menuState, active, &receipt)) {
        return;
    }
    if (receipt.startTextInput) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        if (SDL_StartTextInput(M11_Render_GetWindow())) {
            *hostTextInputActive = 1;
        }
#else
        SDL_StartTextInput();
        *hostTextInputActive = 1;
#endif
    } else if (receipt.stopTextInput) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        (void)SDL_StopTextInput(M11_Render_GetWindow());
#else
        SDL_StopTextInput();
#endif
        *hostTextInputActive = 0;
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
    unsigned int ordinal;
    if (!gameView || !framebuffer || !gameView->assetsAvailable) {
        return 0;
    }
    for (ordinal = 1U; ordinal <= 2U; ++ordinal) {
        EntranceCompatClosedDoorBlit blit;
        const M11_AssetSlot* door;
        if (!ENTRANCE_Compat_GetClosedDoorBlit(ordinal, &blit)) return 0;
        door = M11_AssetLoader_Load(&gameView->assetLoader, blit.assetId);
        if (!door ||
            door->width < blit.srcX + blit.width ||
            door->height < blit.srcY + blit.height) {
            return 0;
        }
        M11_AssetLoader_BlitRegion(door,
                                   (int)blit.srcX,
                                   (int)blit.srcY,
                                   (int)blit.width,
                                   (int)blit.height,
                                   framebuffer,
                                   M11_FB_WIDTH,
                                   M11_FB_HEIGHT,
                                   (int)blit.dstX,
                                   (int)blit.dstY,
                                   blit.transparentColor);
    }
    return 1;
}

typedef enum {
    M11_ENTRANCE_COMMAND_QUIT = ENTRANCE_COMPAT_COMMAND_PATH_QUIT,
    M11_ENTRANCE_COMMAND_NONE = ENTRANCE_COMPAT_COMMAND_PATH_NONE,
    M11_ENTRANCE_COMMAND_ENTER = ENTRANCE_COMPAT_COMMAND_PATH_ENTER,
    M11_ENTRANCE_COMMAND_RESUME = ENTRANCE_COMPAT_COMMAND_PATH_RESUME,
    M11_ENTRANCE_COMMAND_CREDITS = ENTRANCE_COMPAT_COMMAND_PATH_CREDITS
} M11_EntranceCommand;

static int g_m11_intro_delay_fast_forward = 0;

static unsigned int m11_v20_startup_remaining_delay_ms(
    unsigned int source_delay_ms,
    Uint64 presentation_started_ms)
{
    if (!dm1_v2_presentation_mode_is_v20() ||
        presentation_started_ms == 0U) {
        return source_delay_ms;
    }
    return dm1_v20_startup_presentation_remaining_delay_ms_pc34(
        source_delay_ms, SDL_GetTicks() - presentation_started_ms);
}

static int m11_wait_for_entrance_credits_done(unsigned int wait_ticks,
                                              unsigned int vblank_delay_ms,
                                              Uint64 presentation_started_ms) {
    unsigned int ticks;
    SDL_Event ev;
    const int v20TimingActive = dm1_v2_presentation_mode_is_v20();
    const Uint64 sourceDeadlineMs = presentation_started_ms +
        (Uint64)wait_ticks * (Uint64)vblank_delay_ms;
    /* ReDMCSB ENTRANCE.C:1012-1091 F0442 sets L1406=1800, discards stale
     * keyboard input, then waits one delay/vblank per tick until any
     * keyboard or mouse input is present before returning to the entrance
     * command loop. */
    while (SDL_PollEvent(&ev)) {
        (void)ev;
    }
    for (ticks = 0U; ticks < ENTRANCE_Compat_GetCreditsWaitTicks(); ++ticks) {
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
        if (v20TimingActive) {
            const Uint64 nowMs = SDL_GetTicks();
            const Uint64 remainingMs = nowMs >= sourceDeadlineMs
                ? 0U : sourceDeadlineMs - nowMs;
            if (remainingMs == 0U) {
                return M11_ENTRANCE_COMMAND_NONE;
            }
            SDL_Delay((unsigned int)(remainingMs < vblank_delay_ms
                ? remainingMs : vblank_delay_ms));
        } else {
            SDL_Delay(vblank_delay_ms);
        }
    }
    return M11_ENTRANCE_COMMAND_NONE;
}

static int m11_show_redmcsb_entrance_credits(M11_GameViewState* gameView,
                                             unsigned char* framebuffer,
                                             const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                 media_receipt,
                                             DM1_V1_StartupEntranceCreditsPresentationCommand_PC34*
                                                 out_command) {
    const M11_AssetSlot* credits;
    DM1_V1_StartupEntranceCreditsPresentationCommand_PC34 command;
    Uint64 presentationStartedMs;
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
    credits = M11_AssetLoader_Load(&gameView->assetLoader, 5U);
    if (!credits || !dm1_v1_startup_entrance_credits_presentation_command_pc34(
                        media_receipt, credits->pixels, credits->width,
                        credits->height, &command)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
    M11_AssetLoader_Blit(credits, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                         0, 0, -1);
    presentationStartedMs = SDL_GetTicks();
    M11_Render_PresentIndexedWithSpecialPalette(framebuffer,
                                                M11_FB_WIDTH,
                                                M11_FB_HEIGHT,
                                                command.special_palette);
    waitResult = m11_wait_for_entrance_credits_done(command.credits_wait_ticks,
                                                    command.vblank_delay_ms,
                                                    presentationStartedMs);
    *out_command = command;
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

static M11_EntranceCommand m11_wait_for_redmcsb_entrance_command(int autoEnterAfterMs);
static int m11_delay_ms_with_intro_event_pump(unsigned int delayMs);
static M12_MenuInput m11_next_script_input(const char** cursor);
static M12_MenuInput m11_map_script_token(const char* token, size_t len);
static int m11_push_script_event_token(const char* token, size_t len);
static int m11_script_event_token_is_valid(const char* token, size_t len);
static int m11_game_view_is_dm1(const M11_GameViewState* gameView);
static int m11_apply_boot_probe_event_token(M11_GameViewState* gameView,
                                            const char* token,
                                            size_t len,
                                            M11_GameInputResult* outResult);
static int m11_boot_probe_expected_source_kind(const char* gameId,
                                               M11_GameSourceKind* outKind);

static EntranceCompatKey m11_entrance_compat_key_from_sdl_key(int keyCode) {
    switch (keyCode) {
    case SDLK_RETURN:
        return ENTRANCE_COMPAT_KEY_RETURN;
    case SDLK_KP_ENTER:
        return ENTRANCE_COMPAT_KEY_KEYPAD_RETURN;
    case SDLK_ESCAPE:
        return ENTRANCE_COMPAT_KEY_ESCAPE;
    case SDLK_Q:
        return ENTRANCE_COMPAT_KEY_Q;
    case SDLK_SPACE:
        return ENTRANCE_COMPAT_KEY_SPACE;
    default:
        return ENTRANCE_COMPAT_KEY_OTHER;
    }
}

static int m11_entrance_dispatch_source_locked_key_command(int keyCode) {
    return ENTRANCE_Compat_DispatchKeyCommand(m11_entrance_compat_key_from_sdl_key(keyCode));
}

static M11_EntranceCommand m11_entrance_command_path_from_source_command(int commandId) {
    return (M11_EntranceCommand)ENTRANCE_Compat_CommandPathFromSourceCommand(commandId);
}

static int m11_play_redmcsb_entrance_transition(
    M11_GameViewState* gameView,
    int autoEnterAfterMs,
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entranceReceipt,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* mediaReceipt) {
    unsigned char* framebuffer;
    unsigned char* dungeonFrame;
    unsigned int sourceStep;
    int entrancePalette;
    if (!gameView || !gameView->active || !mediaReceipt ||
        !dm1_v1_startup_entrance_timing_receipt_valid_pc34(mediaReceipt)) {
        return 0;
    }
    /* Startup presentation runs before the ordinary frame loop selects the
     * V2.0 lane. Keep its original special-palette frames in that same lane. */
    M11_Render_SetV2PresentationActive(
        m11_dm1_v20_presentation_active(gameView));
    entrancePalette = mediaReceipt->entrance_palette;
    if (entrancePalette != VGA_PALETTE_PC34_SPECIAL_ENTRANCE) {
        return 0;
    }
    if (!DM1_V1_Entrance_FullStartRenderReceiptHostReadyPc34Compat(
            entranceReceipt)) {
        return 0;
    }
    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) return 0;
    dungeonFrame = (unsigned char*)malloc((size_t)M11_FB_BYTES);
    if (!dungeonFrame) return 0;

    M11_GameView_Draw(gameView, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    memcpy(dungeonFrame, framebuffer, (size_t)M11_FB_BYTES);

    /* ENTRANCE.C F0439 first composites the micro-dungeon, curtains it to
     * black, then draws C004/C002/C003.  Do not pre-present closed doors:
     * doing so inserted an extra frame before that source sequence and made
     * the title-to-Entrance transition flash abruptly. */

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
        DM1_V1_StartupEntranceRenderAudioCommand_PC34 command;
        Uint64 presentationStartedMs = 0U;
        if (!ENTRANCE_Compat_GetSourceAnimationStep(sourceStep, &step)) break;
        memset(&command, 0, sizeof(command));
        if (!dm1_v1_startup_entrance_render_audio_command_pc34(
                mediaReceipt,
                sourceStep,
                (int)step.kind,
                step.delayTicks,
                step.vblankLoopCount,
                &command)) {
            free(dungeonFrame);
            return 0;
        }

        if (command.render_kind ==
            DM1_V1_STARTUP_ENTRANCE_RENDER_FADE_BLACK_PC34) {
            memset(framebuffer, 0, (size_t)M11_FB_BYTES);
        } else if (command.render_kind ==
                   DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34) {
            if (!m11_draw_entrance_screen_asset(gameView, framebuffer) ||
                !m11_draw_entrance_closed_doors_asset(gameView, framebuffer)) {
                free(dungeonFrame);
                return 0;
            }
        } else if (command.render_kind ==
                   DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34) {
            EntranceCompatDoorStep door;
            if (command.door_geometry_ready) {
                memset(&door, 0, sizeof(door));
                door.animationStep = command.door_animation_step;
                door.soundRattle = (unsigned int)(command.play_door_rattle_sound ? 1 : 0);
                door.vblankBeforeCopy = 1U;
                door.leftBoxX = command.door_left_box_x;
                door.leftBoxY = command.door_left_box_y;
                door.leftBoxW = command.door_left_box_w;
                door.leftBoxH = command.door_left_box_h;
                door.rightBoxX = command.door_right_box_x;
                door.rightBoxY = command.door_right_box_y;
                door.rightBoxW = command.door_right_box_w;
                door.rightBoxH = command.door_right_box_h;
                door.leftSourceX = command.door_left_source_x;
                door.rightSourceX = command.door_right_source_x;
                if (command.audio_request_ready) {
                    (void)M11_Audio_EmitSourceSoundIndex(
                        &gameView->audioState, command.audio_sound_index);
                }
                if (!m11_draw_entrance_opening_doors_asset(
                        gameView,
                        framebuffer,
                        dungeonFrame,
                        &door)) {
                    free(dungeonFrame);
                    return 0;
                }
            }
        } else {
            memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
        }

        if (command.present_entrance_palette) {
            if (command.entrance_palette != entrancePalette ||
                command.entrance_palette_fingerprint !=
                    mediaReceipt->entrance_palette_fingerprint) {
                free(dungeonFrame);
                return 0;
            }
            presentationStartedMs = SDL_GetTicks();
            M11_Render_PresentIndexedWithSpecialPalette(framebuffer,
                                                        M11_FB_WIDTH,
                                                        M11_FB_HEIGHT,
                                                        command.entrance_palette);
        }
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
                    m11_show_redmcsb_entrance_credits(
                        gameView, framebuffer, mediaReceipt, NULL);
                if (creditsResult == M11_ENTRANCE_COMMAND_QUIT) {
                    free(dungeonFrame);
                    return M11_ENTRANCE_COMMAND_QUIT;
                }
                sourceStep = 0U;
                continue;
            }
        }
        {
            unsigned int delayMs = m11_v20_startup_remaining_delay_ms(
                command.delay_ms, presentationStartedMs);
            if (delayMs > 0U) {
                (void)m11_delay_ms_with_intro_event_pump(delayMs);
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
    return (M11_EntranceCommand)ENTRANCE_Compat_CommandPathFromPointerCommand(fbX,
                                                                              fbY,
                                                                              buttonMask);
}

/* ReDMCSB ENTRANCE.C -> COMMAND.C pointer route exported for the focused
 * HiDPI gate. The event loop uses the same framebuffer-space helper.
 * Restored after the worktree merge drift (newest author state ca0ab2a51). */
int M11_Entrance_DispatchSourceLockedPointerCommand(int framebufferX,
                                                    int framebufferY,
                                                    unsigned int buttonMask) {
    return ENTRANCE_Compat_DispatchMouseRouteCommand(framebufferX,
                                                      framebufferY,
                                                      buttonMask);
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
    if (!ENTRANCE_Compat_NormalizedTouchToWindowPoint(windowW,
                                                      windowH,
                                                      normalizedX,
                                                      normalizedY,
                                                      &windowX,
                                                      &windowY)) {
        return M11_ENTRANCE_COMMAND_NONE;
    }
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
    /* A probe has no second physical C200 command.  Keep the interactive
     * entrance source-faithful, while ensuring dummy-video boot verification
     * completes even when its caller has no explicit timeout policy. */
    if (allowHeadlessTimeout && autoEnterAfterMs <= 0) {
        autoEnterAfterMs = 1;
    }

    while (SDL_PollEvent(&ev)) {
        drained += 1;
    }
    (void)drained;
    if (g_m11_intro_delay_fast_forward) {
        return M11_ENTRANCE_COMMAND_ENTER;
    }
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
                        m11_entrance_dispatch_source_locked_key_command((int)ev.key.key));
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
                        m11_entrance_dispatch_source_locked_key_command((int)ev.key.keysym.sym));
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
        if (ENTRANCE_Compat_ShouldAutoEnterForTimeout(
                allowHeadlessTimeout,
                autoEnterAfterMs,
                (unsigned long long)(SDL_GetTicks() - started))) {
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
 * ST logo layout; TITLE.DAT fallback frames use the corresponding
 * V1_TitleFrontend_Unpack4bppScreenToIndexed path.
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
    if (g_m11_intro_delay_fast_forward) {
        return M11_Render_PumpEvents();
    }
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

/* ReDMCSB NECIO.C F0022 lines 3592-3609 and TITLE.C F0437 lines 319-409:
 * DM1 startup presents SWSH palette waits and TITLE C001 palette/timing in
 * source order.  Keep production rendering on the DM1 receipt so M11 does
 * not infer host cadence independently from the source-locked startup route. */
static unsigned int m11_startup_media_swsh_wait_ms(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media,
    unsigned int vblankCount) {
    if (!media || !media->handled || media->swsh_vblank_ms == 0U) {
        return SWSH_Compat_GetRuntimeDelayMsForVblankCount(vblankCount);
    }
    if (vblankCount > (0xffffffffU / media->swsh_vblank_ms)) {
        return SWSH_Compat_GetRuntimeDelayMsForVblankCount(vblankCount);
    }
    return vblankCount * media->swsh_vblank_ms;
}

static void m11_play_ftl_swoosh_for_game_if_available(
                                              const M12_StartupMenuState* menuState,
                                              const char* dataDir,
                                              const char* gameId,
                                              int skipSwoosh,
                                              const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                  dm1MediaReceipt) {
    char logoPath[FSP_PATH_MAX];
    unsigned char* logoImg = NULL;
    unsigned char* screenFbPacked = NULL;
    unsigned char* screenFbIndexed = NULL;
    unsigned char* screenRgba = NULL;
    FILE* f = NULL; long fsize = 0;
    SWSH_CompatLogoPayload logoPayload;
    unsigned char swshPalette[16][3];
    M11_AudioState swshAudio;
    const unsigned char* dosoundProgram = NULL;
    unsigned int dosoundProgramBytes = 0U;
    int swshAudioInitialized = 0;
    int csbSwshAudioInitialized = 0;
    CSB_V1_BootProfile csbBoot;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 dm1Media;
    int hasDm1Media = 0;
    if (skipSwoosh) return;
    memset(&logoPayload, 0, sizeof(logoPayload));
    memset(&swshAudio, 0, sizeof(swshAudio));
    memset(&csbBoot, 0, sizeof(csbBoot));
    memset(&dm1Media, 0, sizeof(dm1Media));
    if (dm1MediaReceipt && dm1MediaReceipt->handled) {
        dm1Media = *dm1MediaReceipt;
        hasDm1Media = 1;
    } else if (gameId && strcmp(gameId, "dm1") == 0) {
        hasDm1Media =
            dm1_v1_startup_full_graphics_media_receipt_for_source_pc34(
                gameId,
                &dm1Media);
    }
    if (!V1_SWSH_Intro_FindLogoPathForGame(menuState,
                                            dataDir,
                                            gameId,
                                            logoPath,
                                            sizeof(logoPath))) return;
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
      /* SWSH.C:10-14 starts the immutable V0901005 Dosound program before
       * it mutates a single palette register.  It is deliberately scoped to
       * DM1: CSB owns a different F0908 DMA source path.  A media receipt
       * whose cadence is not the PC34 PAL 20 ms VBlank cannot authorize this
       * PSG program, so leave audio silent rather than inventing a host cue. */
      if (gameId && strcmp(gameId, "dm1") == 0 &&
          (!hasDm1Media ||
           dm1Media.swsh_vblank_ms == SWSH_COMPAT_RUNTIME_VBLANK_MS) &&
          M11_Audio_Init(&swshAudio)) {
          dosoundProgram = SWSH_Compat_GetPc34DosoundProgram(
              &dosoundProgramBytes);
          if (dosoundProgram &&
              M11_Audio_PlayDm1SwshDosoundProgram(
                  &swshAudio,
                  dosoundProgram,
                  (int)dosoundProgramBytes,
                  hasDm1Media ? dm1Media.swsh_vblank_ms :
                                SWSH_COMPAT_RUNTIME_VBLANK_MS)) {
              swshAudioInitialized = 1;
          } else {
              M11_Audio_Shutdown(&swshAudio);
          }
      }
      /* CSB PC3.4 has its own SWSHSND.C F0908 DMA sample.  The audio
       * transport already accepts that exact source format, but the old M11
       * prelude only initialized the DM1 PSG branch.  Scan the selected CSB
       * root through the normal hash-first boot profile and queue only the
       * authenticated 9,078-byte source buffer.  Missing media remains
       * silent: no DM1 cue or generated substitute is permitted. */
      if (gameId && strcmp(gameId, "csb") == 0 && dataDir && dataDir[0]) {
          csb_v1_boot_profile_init(&csbBoot);
          if (csb_v1_boot_scan_assets(&csbBoot, dataDir) == 0 &&
              csbBoot.swoosh_source_bound && M11_Audio_Init(&swshAudio)) {
              if (M11_Audio_PlayCsbSwshPcm(
                      &swshAudio,
                      csbBoot.swoosh_source_bytes,
                      (int)sizeof(csbBoot.swoosh_source_bytes),
                      CSB_V1_SWSH_F0908_SOUND_PERIOD_PC34,
                      csbBoot.swoosh_source_fnv1a)) {
                  csbSwshAudioInitialized = 1;
              } else {
                  M11_Audio_Shutdown(&swshAudio);
              }
          }
      }
      m11_swsh_indexed_to_rgba(screenFbIndexed, screenRgba, swshPalette);
      M11_Render_PresentRGBA(screenRgba, M11_FB_WIDTH, M11_FB_HEIGHT);
      if (m11_delay_ms_with_intro_event_pump(
              hasDm1Media ? dm1Media.swsh_initial_logo_hold_ms :
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
                      hasDm1Media ?
                          m11_startup_media_swsh_wait_ms(&dm1Media, step.vblankCount) :
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
          hasDm1Media ? dm1Media.swsh_final_hold_ms :
                        SWSH_Compat_GetRuntimeFinalHoldMs()); }
cleanup:
    if (swshAudioInitialized || csbSwshAudioInitialized) {
        M11_Audio_Shutdown(&swshAudio);
    }
    SWSH_Compat_ReleaseLogoImagePayload(&logoPayload);
    if (logoImg) free(logoImg);
    if (screenFbPacked) free(screenFbPacked);
    if (screenFbIndexed) free(screenFbIndexed);
    if (screenRgba) free(screenRgba);
    if (f) fclose(f);
}

static void m11_play_ftl_swoosh_if_available(const M12_StartupMenuState* menuState,
                                              const char* dataDir,
                                              int skipSwoosh,
                                              const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                  dm1MediaReceipt) {
    m11_play_ftl_swoosh_for_game_if_available(menuState,
                                              dataDir,
                                              "dm1",
                                              skipSwoosh,
                                              dm1MediaReceipt);
}

static int m11_play_redmcsb_title_graphic_intro_if_available(
    const M12_StartupMenuState* menuState,
    M11_GameViewState* gameView,
    const char* sourceId,
    int* outPlayedAnyFrame,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* dm1MediaReceipt) {
    const M11_AssetSlot* titleGraphic;
    unsigned char* framebuffer;
    V1_TitleFrontendSourceTiming timing;
    M11_AudioState titleAudio;
    int titleAudioInitialized = 0;
    int titlePalette = -1;
    Uint64 presentationStartedMs = 0U;
    unsigned int sourceStep;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 dm1Media;
    DM1_V1_StartupTitleRuntimeAssetReceipt_PC34 titleAssetReceipt;
    DM1_V1_StartupTitleSourceHandoffReceipt_PC34 titleSourceHandoff;
    DM1_V1_StartupTitlePresentationCommand_PC34 command;
    char titleDatPath[FSP_PATH_MAX];
    const char* titleDatProvenancePath = NULL;
    int hasDm1Media;

    if (outPlayedAnyFrame) {
        *outPlayedAnyFrame = 0;
    }
    /* ReDMCSB TITLE.C has separate PC/F20 and CSB/A31 implementations.
     * C001 plus C12/C13/C14 is the PC/F20 DM1 contract only; never let a
     * caller selecting CSB consume this DM1 asset/palette route. */
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(sourceId) ||
        !gameView || !gameView->assetsAvailable) {
        return 0;
    }
    /* TITLE.C renders before m11_present_game_frame() can activate V2.0. */
    M11_Render_SetV2PresentationActive(
        m11_dm1_v20_presentation_active(gameView));
    titleGraphic = M11_AssetLoader_Load(&gameView->assetLoader, 1U);
    {
        DM1_V1_StartupTitleRuntimeSourceReceipt_PC34 sourceReceipt;
        if (!dm1_v1_startup_title_runtime_source_receipt_pc34(
                "dm1",
                titleGraphic != NULL,
                titleGraphic ? titleGraphic->width : 0U,
                titleGraphic ? titleGraphic->height : 0U,
                0,
                &sourceReceipt) ||
            !sourceReceipt.handled ||
            sourceReceipt.selected_runtime_source !=
                (int)V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001) {
            return 0;
        }
    }
    framebuffer = M11_Render_GetFramebuffer();
    if (!framebuffer) {
        return 0;
    }
    timing = V1_TitleFrontend_GetSourceTimingEvidence();
    memset(&dm1Media, 0, sizeof(dm1Media));
    if (dm1MediaReceipt && dm1MediaReceipt->handled) {
        dm1Media = *dm1MediaReceipt;
        hasDm1Media = 1;
    } else {
        hasDm1Media =
            dm1_v1_startup_full_graphics_media_receipt_for_source_pc34(
                "dm1",
                &dm1Media);
    }

    memset(&titleAudio, 0, sizeof(titleAudio));
    if (M11_Audio_Init(&titleAudio)) {
        titleAudioInitialized = 1;
        (void)M11_Audio_PlayTitleMusic(&titleAudio);
    }

    memset(framebuffer, 0, (size_t)M11_FB_BYTES);
    memset(&titleAssetReceipt, 0, sizeof(titleAssetReceipt));
    if (!dm1_v1_startup_title_runtime_asset_receipt_pc34(
            "dm1",
            titleGraphic ? titleGraphic->pixels : NULL,
            titleGraphic ? titleGraphic->width : 0U,
            titleGraphic ? titleGraphic->height : 0U,
            &titleAssetReceipt)) {
        memset(&titleAssetReceipt, 0, sizeof(titleAssetReceipt));
    }
    titleDatPath[0] = '\0';
    if (V1_TitleIntro_FindTitleDatPath(menuState, NULL, titleDatPath,
                                       sizeof(titleDatPath))) {
        titleDatProvenancePath = titleDatPath;
    }
    memset(&titleSourceHandoff, 0, sizeof(titleSourceHandoff));
    if (!dm1_v1_startup_title_source_handoff_receipt_pc34(
            "dm1", titleDatProvenancePath,
            titleGraphic ? titleGraphic->pixels : NULL,
            titleGraphic ? titleGraphic->width : 0U,
            titleGraphic ? titleGraphic->height : 0U,
            &titleSourceHandoff) ||
        !dm1_v1_startup_title_source_handoff_valid_pc34(
            &titleSourceHandoff)) {
        return 0;
    }

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
        V1_TitleFrontendC001BlitPlan blitPlan;
        int stepPalette;
        if (!V1_TitleFrontend_GetSourceAnimationStep(sourceStep, &step)) {
            break;
        }
        if (!V1_TitleFrontend_GetC001BlitPlanForStep(&step, &blitPlan)) {
            break;
        }
        memset(&command, 0, sizeof(command));
        if (!dm1_v1_startup_title_presentation_command_pc34(
                &dm1Media, &titleAssetReceipt, sourceStep, &command)) {
            break;
        }
        if (!V1_TitleFrontend_GetStepPalette(step.kind, &stepPalette) ||
            stepPalette != command.special_palette) {
            break;
        }
        {
            DM1_V2_StartupTitleFilterHandoffReceiptPc34 filterReceipt;
            const int paletteValid = command.special_palette >= 0;

            (void)dm1_v2_startup_title_filter_handoff_pc34(
                sourceId,
                gameView->presentationMode,
                command.source_timing_receipt_consumed,
                paletteValid,
                &filterReceipt);
            M11_Render_SetV2PresentationActive(filterReceipt.filters_active);
        }
        if (command.clear_before_present) {
            memset(framebuffer, 0, (size_t)M11_FB_BYTES);
        }
        /* ReDMCSB TITLE.C F0437:362-387: the C13/C14 palette is live
         * before the first zoom VBlank, not after it.  Present the
         * cleared indexed surface to make the hardware-equivalent palette
         * latch visible before waiting. */
        if (command.palette_before_pre_present_delay &&
            titlePalette != stepPalette) {
            presentationStartedMs = SDL_GetTicks();
            if (M11_Render_PresentIndexedWithSpecialPalette(framebuffer,
                                                            M11_FB_WIDTH,
                                                            M11_FB_HEIGHT,
                                                            stepPalette) != M11_RENDER_OK) {
                break;
            }
        }
        /* ReDMCSB TITLE.C F0437:385-387 waits before each prepared
         * zoom bitmap is blitted. Steps 20, 21, and 23 model the two
         * post-zoom waits and the final guard individually, so do not add
         * an aggregate delay after this loop. */
        if (command.pre_present_delay_ms > 0U &&
            m11_delay_ms_with_intro_event_pump(
                m11_v20_startup_remaining_delay_ms(
                    command.pre_present_delay_ms, presentationStartedMs))) {
            break;
        }
        presentationStartedMs = 0U;
        if (!command.present_frame) continue;
        if (blitPlan.kind == V1_TITLE_FRONTEND_C001_BLIT_REGION) {
            M11_AssetLoader_BlitRegion(titleGraphic,
                                       (int)blitPlan.srcX,
                                       (int)blitPlan.srcY,
                                       (int)blitPlan.srcW,
                                       (int)blitPlan.srcH,
                                       framebuffer,
                                       M11_FB_WIDTH,
                                       M11_FB_HEIGHT,
                                       (int)blitPlan.dstX,
                                       (int)blitPlan.dstY,
                                       blitPlan.transparentColor);
        } else if (blitPlan.kind == V1_TITLE_FRONTEND_C001_BLIT_SCALED_REGION) {
            M11_AssetLoader_BlitSubRectScaled(titleGraphic,
                                              framebuffer,
                                              M11_FB_WIDTH,
                                              M11_FB_HEIGHT,
                                              (int)blitPlan.dstX,
                                              (int)blitPlan.dstY,
                                              (int)blitPlan.dstW,
                                              (int)blitPlan.dstH,
                                              (int)blitPlan.srcX,
                                              (int)blitPlan.srcY,
                                              (int)blitPlan.srcW,
                                              (int)blitPlan.srcH,
                                              blitPlan.transparentColor);
        } else {
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
        presentationStartedMs = SDL_GetTicks();
        if (M11_Render_PresentIndexedWithSpecialPalette(framebuffer,
                                                        M11_FB_WIDTH,
                                                        M11_FB_HEIGHT,
                                                        stepPalette) != M11_RENDER_OK) {
            break;
        }
        if (outPlayedAnyFrame) {
            *outPlayedAnyFrame = 1;
        }
        if (command.post_present_delay_ms > 0U &&
            m11_delay_ms_with_intro_event_pump(
                m11_v20_startup_remaining_delay_ms(
                    command.post_present_delay_ms, presentationStartedMs))) {
            break;
        }
        if (command.post_present_delay_ms > 0U) {
            presentationStartedMs = 0U;
        }
    }
    if (titleAudioInitialized) {
        M11_Audio_Shutdown(&titleAudio);
    }
    return outPlayedAnyFrame ? *outPlayedAnyFrame : 1;
}

static void m11_play_redmcsb_title_intro_if_available(const M12_StartupMenuState* menuState,
                                                      M11_GameViewState* gameView,
                                                      const char* sourceId,
                                                      int* outPlayedAnyFrame,
                                                      const DM1_V1_StartupFullGraphicsMediaReceipt_PC34*
                                                          dm1MediaReceipt) {
    char titlePath[FSP_PATH_MAX];
    unsigned char* packedStorage;
    unsigned char* packedScreen;
    unsigned char* indexedScreen;
    char err[160];
    unsigned int step;
    V1_TitleFrontendSourceTiming timing;
    M11_AudioState titleAudio;
    int titleAudioInitialized = 0;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 dm1Media;
    int hasDm1Media;

    if (outPlayedAnyFrame) {
        *outPlayedAnyFrame = 0;
    }
    /* TITLE.C:309-409 is the DM1 PC/F20 branch. CSB enters the distinct
     * A31 branch at TITLE.C:412 and must use its own title implementation. */
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(sourceId)) {
        return;
    }
    if (m11_play_redmcsb_title_graphic_intro_if_available(menuState, gameView,
                                                          sourceId,
                                                          outPlayedAnyFrame,
                                                          dm1MediaReceipt)) {
        return;
    }
    if (!V1_TitleIntro_FindTitleDatPath(menuState, NULL, titlePath, sizeof(titlePath))) {
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
    memset(&dm1Media, 0, sizeof(dm1Media));
    if (dm1MediaReceipt && dm1MediaReceipt->handled) {
        dm1Media = *dm1MediaReceipt;
        hasDm1Media = 1;
    } else {
        hasDm1Media =
            dm1_v1_startup_full_graphics_media_receipt_for_source_pc34(
                "dm1",
                &dm1Media);
    }

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
        (void)V1_TitleFrontend_Unpack4bppScreenToIndexed(packedScreen,
                                                         M11_FB_WIDTH,
                                                         M11_FB_HEIGHT,
                                                         indexedScreen,
                                                         M11_FB_WIDTH);
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
                hasDm1Media ? dm1Media.title_zoom_frame_delay_ms :
                              V1_TitleFrontend_GetRuntimeFrameDelayMs(&timing))) {
            break;
        }
    }
    (void)m11_delay_ms_with_intro_event_pump(
        hasDm1Media ? dm1Media.title_post_zoom_guard_ms :
                      V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&timing));
    if (titleAudioInitialized) {
        M11_Audio_Shutdown(&titleAudio);
    }
    free(packedStorage);
    free(indexedScreen);
}

typedef struct M11_DM1StartupHandoffContext {
    M12_StartupMenuState* menuState;
    M11_GameViewState* gameView;
    uint32_t* idleAccumulatorMs;
    const char* dataDir;
    int bootProbe;
    DM1_V1_StartupHandoffPreludePlan_PC34 activePreludePlan;
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 activePostLaunchPlan;
    int activePreludePlanValid;
    int activePostLaunchPlanValid;
} M11_DM1StartupHandoffContext;

static int m11_dm1_handoff_report_source_order_failure(void* user,
                                                       const char* evidence) {
    (void)user;
    fprintf(stderr,
            "DM1 startup source-order guard failed: %s\n",
            evidence ? evidence : "");
    return 1;
}

static int m11_dm1_handoff_raise_window(void* user) {
    (void)user;
    M11_Render_RaiseWindow();
    return 1;
}

static int m11_dm1_handoff_begin_prelude_plan(
    void* user,
    const DM1_V1_StartupHandoffPreludePlan_PC34* plan) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !plan) {
        return 0;
    }
    ctx->activePreludePlan = *plan;
    ctx->activePreludePlanValid = 1;
    return 1;
}

static int m11_dm1_handoff_end_prelude_plan(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx) {
        return 0;
    }
    memset(&ctx->activePreludePlan, 0, sizeof(ctx->activePreludePlan));
    ctx->activePreludePlanValid = 0;
    return 1;
}

static int m11_dm1_handoff_begin_post_launch_plan(
    void* user,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* plan) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !plan) {
        return 0;
    }
    ctx->activePostLaunchPlan = *plan;
    ctx->activePostLaunchPlanValid = 1;
    return 1;
}

static int m11_dm1_handoff_end_post_launch_plan(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx) {
        return 0;
    }
    memset(&ctx->activePostLaunchPlan, 0, sizeof(ctx->activePostLaunchPlan));
    ctx->activePostLaunchPlanValid = 0;
    return 1;
}

static int m11_dm1_handoff_play_swsh(void* user,
                                     const char* game_id,
                                     int preserve_audio) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media = NULL;
    (void)game_id;
    if (ctx && ctx->activePreludePlanValid &&
        ctx->activePreludePlan.media_receipt.handled) {
        media = &ctx->activePreludePlan.media_receipt;
    }
    m11_play_ftl_swoosh_if_available(ctx ? ctx->menuState : NULL,
                                     ctx ? ctx->dataDir : NULL,
                                     preserve_audio,
                                     media);
    return 1;
}

static int m11_dm1_handoff_discard_presentation_texture(void* user) {
    (void)user;
    M11_Render_DiscardPresentationTexture();
    return 1;
}

static int m11_dm1_handoff_play_title(void* user,
                                      const char* source_id,
                                      int* out_played_any_frame) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media = NULL;
    if (!ctx ||
        !dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        return 0;
    }
    if (ctx->activePostLaunchPlanValid &&
        ctx->activePostLaunchPlan.media_receipt.handled) {
        media = &ctx->activePostLaunchPlan.media_receipt;
    }
    m11_play_redmcsb_title_intro_if_available(ctx->menuState,
                                              ctx->gameView,
                                              source_id,
                                              out_played_any_frame,
                                              media);
    return 1;
}

static int m11_dm1_handoff_play_entrance(void* user,
                                         const char* source_id,
                                         int auto_enter_after_ms,
                                         int* out_entrance_command) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entrance = NULL;
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media = NULL;
    int command;
    (void)source_id;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    if (!ctx->activePostLaunchPlanValid ||
        !ctx->activePostLaunchPlan.entrance_full_start_receipt.valid ||
        ctx->activePostLaunchPlan.entrance_auto_enter_ms != auto_enter_after_ms ||
        !dm1_v1_startup_entrance_timing_receipt_valid_pc34(
            &ctx->activePostLaunchPlan.media_receipt)) {
        return 0;
    }
    entrance = &ctx->activePostLaunchPlan.entrance_full_start_receipt;
    media = &ctx->activePostLaunchPlan.media_receipt;
    command = m11_play_redmcsb_entrance_transition(ctx->gameView,
                                                   auto_enter_after_ms,
                                                   entrance,
                                                   media);
    if (out_entrance_command) {
        *out_entrance_command = command;
    }
    return 1;
}

static int m11_dm1_host_set_game_active(void* user, int active) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    ctx->gameView->active = active;
    return 1;
}

static int m11_dm1_host_resolve_resume_save_path(void* user,
                                                 const char* source_id,
                                                 char* out_path,
                                                 int out_path_size) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->menuState) {
        return 0;
    }
    return ENTRANCE_Compat_ResolveDm1ResumeSavePath(
        source_id,
        ctx->menuState->quickResumeAvailable,
        ctx->menuState->quickResumeGameId,
        ctx->menuState->quickResumeSavePath,
        out_path,
        (size_t)out_path_size);
}

static int m11_dm1_host_load_resume_save_path(void* user,
                                              const char* save_path,
                                              int* out_used_backup) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    return M11_GameView_LoadDm1SavePath(ctx->gameView,
                                        save_path,
                                        out_used_backup);
}

static int m11_dm1_host_log_resume_loaded(void* user,
                                          const char* save_path,
                                          int used_backup) {
    (void)user;
    fprintf(stderr, "RESUME: loaded save from %s%s\n", save_path,
            used_backup ? " backup" : "");
    return 1;
}

static int m11_dm1_host_log_resume_missing(void* user, const char* save_path) {
    (void)user;
    fprintf(stderr, "RESUME: no save found at %s, starting new game\n",
            save_path ? save_path : "(unresolved)");
    return 1;
}

static int m11_dm1_host_log_entrance_skipped(void* user) {
    (void)user;
    fprintf(stderr, "entrance transition skipped (non-fatal)\n");
    return 1;
}

static int m11_dm1_selected_launch_open(void* user,
                                        char* out_source_id,
                                        int out_source_id_size) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView || !ctx->menuState) {
        return 0;
    }
    if (!M11_GameView_OpenSelectedMenuEntry(ctx->gameView, ctx->menuState)) {
        return 0;
    }
    if (out_source_id && out_source_id_size > 0) {
        snprintf(out_source_id,
                 (size_t)out_source_id_size,
                 "%s",
                 ctx->gameView->sourceId);
    }
    return 1;
}

static int m11_dm1_selected_launch_after_open(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->menuState) {
        return 0;
    }
    ctx->menuState->launchRequested = 0;
    (void)M11_Render_SetPaletteLevel(0);
    if (ctx->idleAccumulatorMs) {
        *ctx->idleAccumulatorMs = 0;
    }
    return 1;
}

static int m11_dm1_selected_launch_draw_opened(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->gameView) {
        return 0;
    }
    M11_GameView_Draw(ctx->gameView,
                      M11_Render_GetFramebuffer(),
                      M11_FB_WIDTH,
                      M11_FB_HEIGHT);
    return 1;
}

static int m11_dm1_selected_launch_mark_failed(void* user) {
    M11_DM1StartupHandoffContext* ctx = (M11_DM1StartupHandoffContext*)user;
    if (!ctx || !ctx->menuState) {
        return 0;
    }
    m11_set_launch_failed_message(ctx->menuState);
    return 1;
}

static int m11_open_requested_launch(M11_GameViewState* gameView,
                                     M12_StartupMenuState* menuState,
                                     uint32_t* idleAccumulatorMs,
                                     const char* dataDir,
                                     int bootProbe) {
    M11_DM1StartupHandoffContext dm1HandoffContext;
    DM1_V1_StartupHandoffCallbacks_PC34 dm1HandoffCallbacks;
    DM1_V1_StartupHostCallbacks_PC34 dm1HostCallbacks;
    DM1_V1_StartupSelectedLaunchCallbacks_PC34 dm1SelectedLaunchCallbacks;
    DM1_V1_StartupSelectedLaunchResult_PC34 dm1SelectedLaunchResult;
    DM1_V1_StartupSelectedLaunchRouteFacts_PC34 dm1RouteFacts;
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34 dm1RouteReceipt;
    const M12_MenuEntry* launchEntry;
    if (!gameView || !menuState || !menuState->launchRequested) {
        return 0;
    }
    if (!M12_StartupMenu_PrepareSelectedGameLaunch(menuState)) {
        return 0;
    }
    memset(&dm1HandoffContext, 0, sizeof(dm1HandoffContext));
    memset(&dm1HandoffCallbacks, 0, sizeof(dm1HandoffCallbacks));
    memset(&dm1HostCallbacks, 0, sizeof(dm1HostCallbacks));
    memset(&dm1SelectedLaunchCallbacks, 0, sizeof(dm1SelectedLaunchCallbacks));
    memset(&dm1SelectedLaunchResult, 0, sizeof(dm1SelectedLaunchResult));
    memset(&dm1RouteFacts, 0, sizeof(dm1RouteFacts));
    memset(&dm1RouteReceipt, 0, sizeof(dm1RouteReceipt));
    dm1HandoffContext.menuState = menuState;
    dm1HandoffContext.gameView = gameView;
    dm1HandoffContext.dataDir = dataDir;
    dm1HandoffContext.idleAccumulatorMs = idleAccumulatorMs;
    dm1HandoffContext.bootProbe = bootProbe ? 1 : 0;
    dm1HandoffCallbacks.user = &dm1HandoffContext;
    dm1HandoffCallbacks.begin_prelude_plan =
        m11_dm1_handoff_begin_prelude_plan;
    dm1HandoffCallbacks.end_prelude_plan =
        m11_dm1_handoff_end_prelude_plan;
    dm1HandoffCallbacks.begin_post_launch_plan =
        m11_dm1_handoff_begin_post_launch_plan;
    dm1HandoffCallbacks.end_post_launch_plan =
        m11_dm1_handoff_end_post_launch_plan;
    dm1HandoffCallbacks.report_source_order_failure =
        m11_dm1_handoff_report_source_order_failure;
    dm1HandoffCallbacks.raise_window = m11_dm1_handoff_raise_window;
    dm1HandoffCallbacks.play_swsh = m11_dm1_handoff_play_swsh;
    dm1HandoffCallbacks.discard_presentation_texture =
        m11_dm1_handoff_discard_presentation_texture;
    dm1HandoffCallbacks.play_title = m11_dm1_handoff_play_title;
    dm1HandoffCallbacks.play_entrance = m11_dm1_handoff_play_entrance;
    dm1HostCallbacks.user = &dm1HandoffContext;
    dm1HostCallbacks.set_game_active = m11_dm1_host_set_game_active;
    dm1HostCallbacks.resolve_resume_save_path =
        m11_dm1_host_resolve_resume_save_path;
    dm1HostCallbacks.load_resume_save_path =
        m11_dm1_host_load_resume_save_path;
    dm1HostCallbacks.log_resume_loaded = m11_dm1_host_log_resume_loaded;
    dm1HostCallbacks.log_resume_missing = m11_dm1_host_log_resume_missing;
    dm1HostCallbacks.log_entrance_skipped = m11_dm1_host_log_entrance_skipped;
    dm1SelectedLaunchCallbacks.user = &dm1HandoffContext;
    dm1SelectedLaunchCallbacks.handoff_callbacks = &dm1HandoffCallbacks;
    dm1SelectedLaunchCallbacks.host_callbacks = &dm1HostCallbacks;
    dm1SelectedLaunchCallbacks.open_selected_entry = m11_dm1_selected_launch_open;
    dm1SelectedLaunchCallbacks.after_open = m11_dm1_selected_launch_after_open;
    dm1SelectedLaunchCallbacks.draw_opened = m11_dm1_selected_launch_draw_opened;
    dm1SelectedLaunchCallbacks.mark_launch_failed =
        m11_dm1_selected_launch_mark_failed;
    launchEntry = M12_StartupMenu_GetEntry(menuState, menuState->activatedIndex);
    dm1RouteFacts.selected_game_id =
        (launchEntry && launchEntry->gameId) ? launchEntry->gameId : NULL;
    if (!dm1_v1_startup_selected_launch_route_receipt_pc34(
            &dm1RouteFacts,
            &dm1RouteReceipt)) {
        m11_set_launch_failed_message(menuState);
        return 0;
    }
    if (dm1RouteReceipt.use_dm1_transaction) {
        int oldFastForward = g_m11_intro_delay_fast_forward;
        g_m11_intro_delay_fast_forward = bootProbe ? 1 : oldFastForward;
        if (!dm1_v1_startup_execute_selected_launch_transaction_pc34(
                launchEntry->gameId,
                &dm1SelectedLaunchCallbacks,
                &dm1SelectedLaunchResult)) {
            g_m11_intro_delay_fast_forward = oldFastForward;
            m11_set_launch_failed_message(menuState);
            return 0;
        }
        g_m11_intro_delay_fast_forward = oldFastForward;
        if (dm1SelectedLaunchResult.runtime_handoff_receipt.handled) {
            return (dm1SelectedLaunchResult.runtime_handoff_receipt
                        .runtime_first_frame_ready ||
                    dm1SelectedLaunchResult.runtime_handoff_receipt
                        .return_to_launcher)
                       ? 1
                       : 0;
        }
        return dm1SelectedLaunchResult.opened ? 1 : 0;
    }
    {
        /* CSB has its own title/entrance sequence after the common FTL/SWSH
         * prelude.  ReDMCSB SWSH.C runs the FTL logo before the started
         * program hands off to TITLE/ENTRANCE; Firestaff keeps CSB title and
         * entrance in M11 but still needs the SWSH prelude. */
        if (launchEntry && launchEntry->gameId &&
            strcmp(launchEntry->gameId, "csb") == 0) {
            M11_Render_RaiseWindow();
            m11_play_ftl_swoosh_for_game_if_available(menuState,
                                                       dataDir,
                                                       "csb",
                                                       0,
                                                       NULL);
            M11_Render_DiscardPresentationTexture();
        }
        /* Theron's Quest has no source -- no intro needed. */
    }
    if (M11_GameView_OpenSelectedMenuEntry(gameView, menuState)) {
        menuState->launchRequested = 0;
        (void)M11_Render_SetPaletteLevel(0);
        if (idleAccumulatorMs) {
            *idleAccumulatorMs = 0;
        }
        M11_GameView_Draw(gameView,
                          M11_Render_GetFramebuffer(),
                          M11_FB_WIDTH,
                          M11_FB_HEIGHT);
        return 1;
    }
    m11_set_launch_failed_message(menuState);
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
    return m11_open_requested_launch(gameView,
                                     menuState,
                                     idleAccumulatorMs,
                                     dataDir,
                                     0);
}

int M11_PrepareDirectLaunchForGame(M12_StartupMenuState* menuState,
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
            M12_LaunchIntent intent;
            if (!M12_AssetStatus_GameAvailable(&menuState->assetStatus, gameId)) {
                return 0;
            }
            menuState->selectedIndex = i;
            menuState->activatedIndex = i;
            menuState->launchRequested = 1;
            menuState->quickResumeLaunchRequested = 0;
            /* A direct CLI launch still goes through the M12 launch intent,
             * whose generic V2.2 gate quite rightly refuses CSB unless every
             * reviewed CSB material is installed.  The CSB presentation
             * runtime already has the supported V2.2 -> V2.1 resolution for
             * this exact case.  Consume it before asking M12 for the intent,
             * so --game csb --presentation-mode v22 starts in V2.1 rather
             * than reporting the verified game data as unavailable. */
            if (menuState->gameOptions[i].presentationModeIndex ==
                M12_PRESENTATION_V22_MODERN) {
                int resolveToV21 = 0;
                if (strcmp(gameId, "csb") == 0) {
                    const char* dataDir = M12_AssetStatus_GetRuntimeDataDir(
                        &menuState->assetStatus, gameId);
                    csb_v22_set_manifest_path(dataDir);
                    csb_v22_famg_set_manifest_path(dataDir);
                    csb_v2_presentation_mode_set_m12(
                        M12_PRESENTATION_V22_MODERN);
                    resolveToV21 = csb_v2_presentation_mode_get() ==
                        CSB_V2_PM_V21_UPSCALED;
                } else if (strcmp(gameId, "dm1") != 0 &&
                           strcmp(gameId, "nexus") != 0) {
                    /* M12 only admits a native V2.2 pack for DM1/Nexus.
                     * A persisted global V2.2 choice must not make an
                     * otherwise verified direct launch of another game fail. */
                    resolveToV21 = 1;
                }
                if (resolveToV21) {
                    menuState->settings.graphicsIndex =
                        M12_PRESENTATION_V21_UPSCALED;
                    menuState->gameOptions[i].presentationModeIndex =
                        M12_PRESENTATION_V21_UPSCALED;
                }
            }
            if (gameId && strcmp(gameId, "theron") == 0) {
                const M12_AssetVersionStatus* version =
                    M12_AssetStatus_GetFirstMatchedVersion(&menuState->assetStatus, gameId);
                if (version && version->matchedPath[0] != '\0' &&
                    version->matchedMd5[0] != '\0') {
                    M12_StartupMenu_ScanTheronCampaignMedia(
                        menuState, version->matchedPath, version->matchedMd5, NULL);
                }
            }
            intent = M12_StartupMenu_GetLaunchIntent(menuState);
            if (!intent.valid) {
                menuState->launchRequested = 0;
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

static int m11_boot_probe_expected_source_kind(const char* gameId,
                                               M11_GameSourceKind* outKind) {
    if (!gameId || !outKind) {
        return 0;
    }
    if (strcmp(gameId, "dm1") == 0) {
        *outKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
        return 1;
    }
    if (strcmp(gameId, "csb") == 0) {
        *outKind = M11_GAME_SOURCE_CSB_BOOT;
        return 1;
    }
    if (strcmp(gameId, "dm2") == 0) {
        *outKind = M11_GAME_SOURCE_DM2_BOOT;
        return 1;
    }
    if (strcmp(gameId, "nexus") == 0) {
        *outKind = M11_GAME_SOURCE_NEXUS_DGN;
        return 1;
    }
    if (strcmp(gameId, "theron") == 0) {
        *outKind = M11_GAME_SOURCE_THERON_TRACK02;
        return 1;
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
    opts->presentationModeOverride = -1;
    opts->durationMs     = -1;
    opts->presentEveryMs = 16;
    opts->script         = NULL;
    opts->dataDir        = NULL;
    opts->gameId         = NULL;
    opts->directLaunch   = 0;
    opts->bootProbe      = 0;
    opts->bootProbeFrames = 0;
    opts->bootProbeExpectPhase = NULL;
    opts->bootProbeExpectRuntime = 0;
    opts->bootProbeExpectParty = 0;
    opts->bootProbeExpectPartyX = -1;
    opts->bootProbeExpectPartyY = -1;
    opts->bootProbeExpectPartyDir = -1;
    opts->bootProbeExpectChampions = 0;
    opts->bootProbeExpectChampionCount = -1;
    opts->bootProbeExpectLevelLoaded = -1;
    opts->bootProbeExpectAssetMd5 = NULL;
    opts->bootProbeExpectMap = 0;
    opts->bootProbeExpectMapIndex = -1;
    opts->bootProbeExpectRuntimeTickMin = -1;
    opts->bootProbeExpectRuntimeTickMax = -1;
    opts->bootProbeExpectStartupActive = -1;
    opts->bootProbeExpectStartupFrameMin = -1;
    opts->bootProbeExpectStartupFrameMax = -1;
    opts->bootProbeExpectStartupAnimation = NULL;
    opts->bootProbeExpectStartupAnimationActive = -1;
    opts->bootProbeExpectTitleFrameMin = -1;
    opts->bootProbeExpectTitleFrameMax = -1;
    opts->bootProbeExpectTitleFrameBoundary = -1;
    opts->bootProbeExpectTitleReady = -1;
    opts->bootProbeExpectDm1HoCFullGraphics = 0;
    opts->bootProbeExpectDm1HoCReleaseAppCapture = 0;
    opts->retroAchievementsEnabled = 0;
    opts->showFpsOverlay = 0;
    opts->retroAchievementsHardcore = 1;
    opts->retroAchievementsUser = NULL;
    opts->retroAchievementsToken = NULL;
}

static void m11_phase_a_advance_boot_probe_frames(M11_GameViewState* gameView,
                                                  int frameCount) {
    int i;
    if (!gameView || !gameView->active || frameCount <= 0) {
        return;
    }
    for (i = 0; i < frameCount; ++i) {
        M11_GameInputResult result = M11_GameView_AdvanceIdleTick(gameView);
        if (result == M11_GAME_INPUT_REDRAW || i == frameCount - 1) {
            M11_GameView_Draw(gameView,
                              M11_Render_GetFramebuffer(),
                              M11_FB_WIDTH,
                              M11_FB_HEIGHT);
        }
    }
}

static int m11_script_next_token(const char** cursor,
                                 const char** outStart,
                                 size_t* outLen) {
    const char* start;
    const char* end;
    if (outStart) {
        *outStart = NULL;
    }
    if (outLen) {
        *outLen = 0U;
    }
    if (!cursor || !*cursor) {
        return 0;
    }
    start = *cursor;
    while (*start == ' ' || *start == ',') {
        ++start;
    }
    if (*start == '\0') {
        *cursor = start;
        return 0;
    }
    end = start;
    while (*end != '\0' && *end != ',') {
        ++end;
    }
    *cursor = end;
    if (outStart) {
        *outStart = start;
    }
    if (outLen) {
        *outLen = (size_t)(end - start);
    }
    return 1;
}

static int m11_boot_probe_script_wait_frames(const char* token, size_t len) {
    unsigned long value = 0UL;
    size_t pos = 0U;
    if (!token || len == 0U) {
        return -1;
    }
    if (len >= 4U && strncmp(token, "wait", 4U) == 0) {
        pos = 4U;
        if (pos < len && token[pos] == ':') {
            ++pos;
        }
    } else if (len >= 6U && strncmp(token, "frames", 6U) == 0) {
        pos = 6U;
        if (pos < len && token[pos] == ':') {
            ++pos;
        }
    } else {
        return -1;
    }
    if (pos >= len) {
        return -1;
    }
    while (pos < len) {
        if (token[pos] < '0' || token[pos] > '9') {
            return -1;
        }
        value = value * 10UL + (unsigned long)(token[pos] - '0');
        if (value > 100000UL) {
            value = 100000UL;
        }
        ++pos;
    }
    return (int)value;
}

static int m11_phase_a_apply_boot_probe_script(M11_GameViewState* gameView,
                                               const char* script,
                                               int framesAfterInput,
                                               int* outWaitFrames) {
    const char* cursor = script;
    int applied = 0;
    int waited = 0;
    if (outWaitFrames) {
        *outWaitFrames = 0;
    }
    if (!gameView || !gameView->active || !script || script[0] == '\0') {
        return 0;
    }
    while (cursor && *cursor != '\0') {
        const char* token = NULL;
        size_t tokenLen = 0U;
        int waitFrames;
        M12_MenuInput input;
        M11_GameInputResult result;
        if (!m11_script_next_token(&cursor, &token, &tokenLen)) {
            break;
        }
        waitFrames = m11_boot_probe_script_wait_frames(token, tokenLen);
        if (waitFrames >= 0) {
            m11_phase_a_advance_boot_probe_frames(gameView, waitFrames);
            waited += waitFrames;
            continue;
        }
        if (m11_apply_boot_probe_event_token(gameView, token, tokenLen, &result)) {
            applied++;
            if (result == M11_GAME_INPUT_REDRAW) {
                M11_GameView_Draw(gameView,
                                  M11_Render_GetFramebuffer(),
                                  M11_FB_WIDTH,
                                  M11_FB_HEIGHT);
            }
            if (result == M11_GAME_INPUT_RETURN_TO_MENU ||
                result == M11_GAME_INPUT_RESTART_GAME) {
                break;
            }
            m11_phase_a_advance_boot_probe_frames(gameView, framesAfterInput);
            waited += framesAfterInput > 0 ? framesAfterInput : 0;
            continue;
        }
        input = m11_map_script_token(token, tokenLen);
        if (input == M12_MENU_INPUT_NONE) {
            continue;
        }
        result = M11_GameView_HandleInput(gameView, input);
        applied++;
        if (result == M11_GAME_INPUT_REDRAW) {
            M11_GameView_Draw(gameView,
                              M11_Render_GetFramebuffer(),
                              M11_FB_WIDTH,
                              M11_FB_HEIGHT);
        }
        if (result == M11_GAME_INPUT_RETURN_TO_MENU ||
            result == M11_GAME_INPUT_RESTART_GAME) {
            break;
        }
        m11_phase_a_advance_boot_probe_frames(gameView, framesAfterInput);
        waited += framesAfterInput > 0 ? framesAfterInput : 0;
    }
    if (outWaitFrames) {
        *outWaitFrames = waited;
    }
    return applied;
}

static void m11_phase_a_print_boot_probe_receipt(
    const M11_GameViewState* gameView,
    const M12_StartupMenuState* menuState,
    const char* gameId,
    int advancedFrames,
    int scriptInputs,
    int scriptFrames) {
    M11_BootProbeReceipt receipt;
    const char* runtimeDir = "";
    if (menuState && gameId && gameId[0] != '\0') {
        runtimeDir = M12_AssetStatus_GetRuntimeDataDir(&menuState->assetStatus,
                                                       gameId);
    }
    if (!runtimeDir) {
        runtimeDir = "";
    }
    if (!M11_GameView_GetBootProbeReceipt(gameView, &receipt)) {
        fprintf(stderr,
                "FIRESTAFF BOOT PROBE READY: gameId=%s sourceKind=%d sourceId=%s dataDir=%s frames=%d inputs=%d scriptFrames=%d\n",
                gameId ? gameId : "",
                gameView ? (int)gameView->sourceKind : 0,
                gameView ? gameView->sourceId : "",
                runtimeDir,
                advancedFrames,
                scriptInputs,
                scriptFrames);
        return;
    }
    {
        DM1_V1_StartupHoCBootProbeLogReceipt_PC34 dm1Log;
        memset(&dm1Log, 0, sizeof(dm1Log));
        dm1_v1_startup_hoc_boot_probe_log_receipt_pc34(
            &receipt.dm1HoCBootSummary,
            &dm1Log);
    fprintf(stderr,
            "FIRESTAFF BOOT PROBE READY: gameId=%s sourceKind=%d sourceId=%s assetMd5=%s dataDir=%s frames=%d inputs=%d scriptFrames=%d presentationMode=%d presentation=%dx%d phase=%s startupActive=%d startupFrame=%d startupAnimation=%s startupAnimationActive=%d titleFrame=%d titleFrameMax=%d titleReady=%d levelLoaded=%d map=%d party=%d,%d,%d champions=%d runtimeTick=%d dm1WorldTick=%u startedFromLauncher=%d introBypassed=%d %s\n",
            gameId ? gameId : "",
            (int)receipt.sourceKind,
            receipt.sourceId,
            receipt.bootAssetMd5,
            runtimeDir,
            advancedFrames,
            scriptInputs,
            scriptFrames,
            receipt.presentationMode,
            receipt.presentationWidth,
            receipt.presentationHeight,
            receipt.startupPhase,
            receipt.startupActive,
            receipt.startupFrame,
            receipt.startupAnimation,
            receipt.startupAnimationActive,
            receipt.startupTitleFrame,
            receipt.startupTitleFrameMax,
            receipt.startupTitleReady,
            receipt.levelLoaded,
            receipt.mapIndex,
            receipt.partyX,
            receipt.partyY,
            receipt.partyDir,
            receipt.championCount,
            receipt.runtimeTick,
            (unsigned int)receipt.dm1WorldTick,
            receipt.startedFromLauncher,
            receipt.dm1StartupIntroBypassed,
            dm1Log.fields[0] ? dm1Log.fields : "dm1HoCBootSummary=missing");
    }
}

static int m11_boot_probe_expected_phase_is_runtime(const char *phase) {
    size_t len;
    const char *suffix = "-runtime";
    size_t suffix_len = 8U;
    if (!phase) {
        return 0;
    }
    len = strlen(phase);
    if (len == 7U && strcmp(phase, "runtime") == 0) {
        return 1;
    }
    return len > suffix_len &&
           strcmp(phase + len - suffix_len, suffix) == 0;
}

static int m11_boot_probe_runtime_receipt_ready(
    const M11_BootProbeReceipt *receipt)
{
    return receipt &&
           receipt->active &&
           !receipt->startupActive &&
           receipt->levelLoaded;
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

/* V2.1's EPX surface is the user-visible image. Capturing the indexed
 * source here would produce a valid-looking but unscaled BMP. */
static void m11_capture_user_screenshot(const M11_GameViewState* gameView,
                                        const M12_StartupMenuState* menuState) {
    const char* outputDir = NULL;
    char outPath[1024];
    int captured;

    if (!gameView || !gameView->active) {
        return;
    }
    if (menuState && menuState->settings.screenshotPath[0] != '\0') {
        outputDir = menuState->settings.screenshotPath;
    }
    if (m11_game_view_is_dm1(gameView) &&
        gameView->presentationMode == M12_PRESENTATION_V21_UPSCALED) {
        /* V2.1 only enlarges DM1's original indexed graphics.  Do not turn
         * an asset-free fallback frame into a user-facing capture receipt. */
        if (!gameView->assetsAvailable || !gameView->assetLoader.fileState) {
            fprintf(stderr,
                    "SCREENSHOT FAILED: DM1 V2.1 requires loaded original assets\n");
            return;
        }
        captured = M11_Screenshot_CapturePresentedRGBA(outputDir,
                                                        outPath,
                                                        (int)sizeof(outPath));
    } else {
        captured = M11_Screenshot_CaptureCurrent(outputDir,
                                                  outPath,
                                                  (int)sizeof(outPath));
    }
    if (captured) {
        fprintf(stderr, "SCREENSHOT: %s\n", outPath);
    } else {
        fprintf(stderr, "SCREENSHOT FAILED\n");
    }
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
        (len == 6U && strncmp(token, "action", len) == 0) ||
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
    return gameView &&
           DM1_V1_InputSourceIsActivePc34Compat(gameView->active,
                                                gameView->sourceId);
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

/* ReDMCSB COMMAND.C:579-610, MEDIA707_I34E/I34M.  Keep the original
 * interface keys ahead of Firestaff-only shortcuts in the live host route. */
static int m11_dm1_sdl_key_to_menu_input(int key, int ctrlDown, int shiftDown,
                                         M12_MenuInput* outInput) {
    if (!outInput) {
        return 0;
    }
    if (key == SDLK_ESCAPE) {
        /* DM1 uses Escape for the LOADSAVE.C return/quit route.  C147/C148
         * freeze is CSB-only, so routing DM1 through FREEZE_TOGGLE left the
         * game on a misleading frozen status instead of its source dialog. */
        *outInput = M12_MENU_INPUT_BACK;
        return 1;
    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
    if (ctrlDown && key == SDLK_S) {
#else
    if (ctrlDown && key == SDLK_s) {
#endif
        *outInput = M12_MENU_INPUT_SAVE_GAME;
        return 1;
    }
    if (!shiftDown) {
        switch (key) {
            case SDLK_1: *outInput = M12_MENU_INPUT_CHAMPION_1_INVENTORY; return 1;
            case SDLK_2: *outInput = M12_MENU_INPUT_CHAMPION_2_INVENTORY; return 1;
            case SDLK_3: *outInput = M12_MENU_INPUT_CHAMPION_3_INVENTORY; return 1;
            case SDLK_4: *outInput = M12_MENU_INPUT_CHAMPION_4_INVENTORY; return 1;
            default: break;
        }
    }
    return 0;
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

static int m11_script_event_token_is_valid(const char* token, size_t len) {
    char buffer[128];
    int x = 0;
    int y = 0;
    if (!token || len == 0U || len >= sizeof(buffer)) {
        return 0;
    }
    memcpy(buffer, token, len);
    buffer[len] = '\0';
    if (strncmp(buffer, "key:", 4) == 0) {
        return buffer[4] != '\0' &&
               m11_script_keycode_from_name(buffer + 4) != 0x7fffffff;
    }
    if (sscanf(buffer, "click:%d:%d", &x, &y) == 2) {
        return 1;
    }
    if (sscanf(buffer, "move:%d:%d", &x, &y) == 2) {
        return 1;
    }
    return 0;
}

static M12_MenuInput m11_boot_probe_key_input(const M11_GameViewState* gameView,
                                              int keycode) {
    M12_MenuInput csbInput = M12_MENU_INPUT_NONE;
    if (m11_game_view_is_csb(gameView) &&
        m11_csb_sdl_key_to_menu_input(keycode, 0, &csbInput)) {
        return csbInput;
    }
    switch (keycode) {
        case SDLK_UP: return M12_MENU_INPUT_UP;
        case SDLK_DOWN: return M12_MENU_INPUT_DOWN;
        case SDLK_LEFT: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_RIGHT: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_KP_5: return M12_MENU_INPUT_UP;
        case SDLK_KP_2: return M12_MENU_INPUT_DOWN;
        case SDLK_KP_1: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_KP_3: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_KP_4: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_KP_6: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_Q: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_E: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_HOME: return M12_MENU_INPUT_TURN_LEFT;
        case SDLK_END: return M12_MENU_INPUT_TURN_RIGHT;
        case SDLK_A: return M12_MENU_INPUT_STRAFE_LEFT;
        case SDLK_D: return M12_MENU_INPUT_STRAFE_RIGHT;
        case SDLK_W: return M12_MENU_INPUT_UP;
        case SDLK_S: return M12_MENU_INPUT_DOWN;
        case SDLK_RETURN:
        case SDLK_KP_ENTER: return M12_MENU_INPUT_ACCEPT;
        case SDLK_ESCAPE: return M12_MENU_INPUT_BACK;
        case SDLK_SPACE: return M12_MENU_INPUT_ACTION;
        case SDLK_TAB: return M12_MENU_INPUT_CYCLE_CHAMPION;
        case SDLK_R: return gameView && gameView->active ? M12_MENU_INPUT_REST_TOGGLE : M12_MENU_INPUT_NONE;
        case SDLK_X: return gameView && gameView->active ? M12_MENU_INPUT_USE_STAIRS : M12_MENU_INPUT_NONE;
        case SDLK_G: return gameView && gameView->active ? M12_MENU_INPUT_PICKUP_ITEM : M12_MENU_INPUT_NONE;
        case SDLK_P: return gameView && gameView->active ? M12_MENU_INPUT_DROP_ITEM : M12_MENU_INPUT_NONE;
        case SDLK_1: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_1 : M12_MENU_INPUT_NONE;
        case SDLK_2: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_2 : M12_MENU_INPUT_NONE;
        case SDLK_3: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_3 : M12_MENU_INPUT_NONE;
        case SDLK_4: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_4 : M12_MENU_INPUT_NONE;
        case SDLK_5: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_5 : M12_MENU_INPUT_NONE;
        case SDLK_6: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_RUNE_6 : M12_MENU_INPUT_NONE;
        case SDLK_C: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_CAST : M12_MENU_INPUT_NONE;
        case SDLK_V: return gameView && gameView->active ? M12_MENU_INPUT_SPELL_CLEAR : M12_MENU_INPUT_NONE;
        case SDLK_U: return gameView && gameView->active ? M12_MENU_INPUT_USE_ITEM : M12_MENU_INPUT_NONE;
        case SDLK_M: return gameView && gameView->active ? M12_MENU_INPUT_MAP_TOGGLE : M12_MENU_INPUT_NONE;
        case SDLK_I: return gameView && gameView->active ? M12_MENU_INPUT_INVENTORY_TOGGLE : M12_MENU_INPUT_NONE;
        default: return M12_MENU_INPUT_NONE;
    }
}

static int m11_apply_boot_probe_event_token(M11_GameViewState* gameView,
                                            const char* token,
                                            size_t len,
                                            M11_GameInputResult* outResult) {
    char buffer[128];
    int x = 0;
    int y = 0;
    if (outResult) {
        *outResult = M11_GAME_INPUT_IGNORED;
    }
    if (!gameView || !gameView->active ||
        !token || len == 0U || len >= sizeof(buffer)) {
        return 0;
    }
    memcpy(buffer, token, len);
    buffer[len] = '\0';
    if (strncmp(buffer, "key:", 4) == 0) {
        int keycode = m11_script_keycode_from_name(buffer + 4);
        M12_MenuInput input = m11_boot_probe_key_input(gameView, keycode);
        if (keycode == SDLK_F5) {
            if (outResult) {
                *outResult = M11_GameView_QuickSave(gameView)
                    ? M11_GAME_INPUT_REDRAW : M11_GAME_INPUT_IGNORED;
            } else {
                (void)M11_GameView_QuickSave(gameView);
            }
            return 1;
        }
        if (input == M12_MENU_INPUT_NONE) {
            return 0;
        }
        if (outResult) {
            *outResult = M11_GameView_HandleInput(gameView, input);
        } else {
            (void)M11_GameView_HandleInput(gameView, input);
        }
        return 1;
    }
    if (sscanf(buffer, "click:%d:%d", &x, &y) == 2) {
        /* The normal SDL event loop has already presented the current frame
         * before it maps a window point through its content rectangle.  A
         * boot-probe advances game state without drawing, which left the
         * renderer reporting its stale 320x200 V1 content size for V2.0/V2.1
         * clicks.  Present once here so the probe exercises the identical
         * 640x400/upscaled input geometry as a live CSB window. */
        if (!m11_present_game_frame(gameView, NULL)) {
            return 1;
        }
        if (!m11_map_window_pointer_to_game_source(gameView, x, y, &x, &y)) {
            return 1;
        }
        if (outResult) {
            *outResult = M11_GameView_HandlePointerButton(
                gameView, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        } else {
            (void)M11_GameView_HandlePointerButton(
                gameView, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        }
        return 1;
    }
    if (sscanf(buffer, "move:%d:%d", &x, &y) == 2) {
        if (!m11_present_game_frame(gameView, NULL)) {
            return 1;
        }
        if (!m11_map_window_pointer_to_game_source(gameView, x, y, &x, &y)) {
            return 1;
        }
        if (outResult) {
            *outResult = M11_GameView_HandlePointerMove(gameView, x, y);
        } else {
            (void)M11_GameView_HandlePointerMove(gameView, x, y);
        }
        return 1;
    }
    return 0;
}

int M11_BootProbeScript_Validate(const char* script,
                                 char* firstInvalidOut,
                                 size_t firstInvalidOutSize) {
    const char* cursor = script;
    int invalid = 0;
    if (firstInvalidOut && firstInvalidOutSize > 0U) {
        firstInvalidOut[0] = '\0';
    }
    if (!script || script[0] == '\0') {
        return 0;
    }
    while (cursor && *cursor != '\0') {
        const char* token = NULL;
        size_t tokenLen = 0U;
        int recognized = 0;
        if (!m11_script_next_token(&cursor, &token, &tokenLen)) {
            break;
        }
        if (m11_boot_probe_script_wait_frames(token, tokenLen) >= 0 ||
            m11_script_event_token_is_valid(token, tokenLen) ||
            m11_map_script_token(token, tokenLen) != M12_MENU_INPUT_NONE) {
            recognized = 1;
        }
        if (!recognized) {
            if (invalid == 0 && firstInvalidOut && firstInvalidOutSize > 0U) {
                size_t copyLen = tokenLen;
                if (copyLen >= firstInvalidOutSize) {
                    copyLen = firstInvalidOutSize - 1U;
                }
                memcpy(firstInvalidOut, token, copyLen);
                firstInvalidOut[copyLen] = '\0';
            }
            ++invalid;
        }
    }
    return invalid;
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

M12_MenuInput M11_DM1V1_NavigationInputFromScancode(int scancode) {
    /* ReDMCSB COMMAND.C:677-684 maps the PC34 navigation keys into the
     * same C068..C073 movement-arrow commands used by mouse clicks. Keep
     * the host keyboard route on those tokens so presentation can highlight
     * the exact clicked arrow zone. */
    switch (scancode) {
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_KP_5:
            return M12_MENU_INPUT_UP;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_KP_2:
            return M12_MENU_INPUT_DOWN;
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_KP_1:
            return M12_MENU_INPUT_STRAFE_LEFT;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_KP_3:
            return M12_MENU_INPUT_STRAFE_RIGHT;
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
    return DM1_V1_InputMenuTokenIsImmediateTurnPc34Compat((int)input);
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

M12_MenuInput M11_GamepadActionToMenuInput(M12_InputAction action,
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
    return M11_GamepadActionToMenuInput(action, gameplayActive);
}

M12_MenuInput M11_GamepadAxisToMenuInput(SDL_GamepadAxis axis,
                                         M12_AxisRole role,
                                         int processedValue,
                                         int gameplayActive) {
    if (processedValue > -16000 && processedValue < 16000) {
        return M12_MENU_INPUT_NONE;
    }

    /* ReDMCSB COMMAND.C F0358/F0359 consumes the same source-locked
     * navigation commands regardless of host device. Keep controller
     * directions on the tokens used by keyboard and click-arrow receipts. */
    if (role == M12_AXIS_ROLE_MOVE) {
        if (axis == SDL_GAMEPAD_AXIS_LEFTY || axis == SDL_GAMEPAD_AXIS_RIGHTY) {
            return processedValue < 0 ? M12_MENU_INPUT_UP : M12_MENU_INPUT_DOWN;
        }
        if (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX) {
            if (gameplayActive) {
                return processedValue < 0 ? M12_MENU_INPUT_STRAFE_LEFT
                                          : M12_MENU_INPUT_STRAFE_RIGHT;
            }
            return processedValue < 0 ? M12_MENU_INPUT_LEFT : M12_MENU_INPUT_RIGHT;
        }
    } else if (role == M12_AXIS_ROLE_TURN) {
        if (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX) {
            if (gameplayActive) {
                return processedValue < 0 ? M12_MENU_INPUT_TURN_LEFT
                                          : M12_MENU_INPUT_TURN_RIGHT;
            }
            return processedValue < 0 ? M12_MENU_INPUT_LEFT : M12_MENU_INPUT_RIGHT;
        }
    }
    return M12_MENU_INPUT_NONE;
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
    return M11_GamepadAxisToMenuInput(axis, cfg->role, value, gameplayActive);
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
                    ? M11_DM1V1_NavigationInputFromScancode((int)preferred[i])
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
    return gameView &&
           dm1_v1_resurrection_rename_ui_gate_host_active_pc34(
               gameView->active,
               gameView->candidateMirrorPanelActive,
               gameView->candidateMirrorRenameActive);
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
    while (p && *p) {
        unsigned char ch = *p++;
        DM1_V1_ResurrectionRenameUiHostTextByteDecisionPc34Compat decision;
        if (!dm1_v1_resurrection_rename_ui_gate_host_text_byte_decision_pc34(
                gameView ? gameView->active : 0,
                gameView ? gameView->candidateMirrorPanelActive : 0,
                gameView ? gameView->candidateMirrorRenameActive : 0,
                (int)ch,
                &decision)) {
            return 0;
        }
        if (decision.useAscii &&
            m11_dm1_rename_apply_ascii(gameView, decision.ascii) ==
                M11_GAME_INPUT_REDRAW) {
            changed = 1;
        }
    }
    if (changed && outResult) {
        *outResult = M11_GAME_INPUT_REDRAW;
    }
    return 1;
}

static int m11_drain_retroachievements_events(Firestaff_RA_Runtime* runtime,
                                              M11_GameViewState* gameView) {
    Firestaff_RA_Event event;
    int pushed = 0;

    if (!runtime || !gameView) {
        return 0;
    }
    while (firestaff_ra_poll_event(runtime, &event)) {
        if (firestaff_ra_overlay_push_event(&gameView->retroAchievementsOverlay,
                                            runtime,
                                            &event)) {
            pushed = 1;
        }
    }
    return pushed;
}

static M11_GameInputResult
m11_dm1_rename_handle_keydown(M11_GameViewState* gameView,
                              int key,
                              int keypadEnterKey) {
    DM1_V1_ResurrectionRenameUiHostKeyDecisionPc34Compat decision;
    int hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_OTHER_PC34_COMPAT;
    /* ReDMCSB REVIVE.C F0281:535-545 uses Return to move from name to
     * title, F0281:549-567 uses backspace within the active field, and
     * F0282:806-808 enters F0281 from C161.  Consume all other keydown
     * events here so SDL_TEXTINPUT, not the movement/shortcut mapper,
     * owns printable rename characters while the panel is active. */
    if (key == SDLK_BACKSPACE) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_BACKSPACE_PC34_COMPAT;
    } else if (key == SDLK_ESCAPE) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_ESCAPE_PC34_COMPAT;
    } else if (key == SDLK_RETURN) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_RETURN_PC34_COMPAT;
    } else if (key == keypadEnterKey) {
        hostKey = DM1_V1_RESURRECTION_RENAME_UI_HOST_KEY_KEYPAD_RETURN_PC34_COMPAT;
    }
    if (!dm1_v1_resurrection_rename_ui_gate_host_keydown_route_pc34(
            gameView ? gameView->active : 0,
            gameView ? gameView->candidateMirrorPanelActive : 0,
            gameView ? gameView->candidateMirrorRenameActive : 0,
            gameView ? &gameView->candidateMirrorRename : NULL,
            hostKey,
            &decision)) {
        return M11_GAME_INPUT_IGNORED;
    }
    if (decision.useAscii) {
        return m11_dm1_rename_apply_ascii(gameView, decision.ascii);
    }
    if (decision.useCommand) {
        return M11_GameView_ApplyMirrorCandidateRenameCommand(
                   gameView, decision.command)
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
                /* An OS maximize can arrive while a DM1 game is active.
                 * Reapply the host presentation policy immediately so the
                 * next pointer event maps through the same FIT rect M11
                 * presents, rather than a stale 1x--4x rectangle. */
                M11_ApplyStartupMenuRuntime(menuState);
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
                m11_map_window_pointer_to_game_source(gameView,
                                                       (int)ev.motion.x,
                                                       (int)ev.motion.y,
                                                       &mappedX,
                                                       &mappedY)) {
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
                m11_map_window_pointer_to_game_source(gameView,
                                                       (int)ev.button.x,
                                                       (int)ev.button.y,
                                                       &mappedX,
                                                       &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointerButton(
                    gameView,
                    mappedX,
                    mappedY,
                    ev.button.button == SDL_BUTTON_RIGHT
                        ? DM1_V1_MOUSE_MASK_RIGHT_PC34
                        : DM1_V1_MOUSE_MASK_LEFT_PC34);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP &&
            gameView && gameView->active &&
            ev.button.button == SDL_BUTTON_LEFT) {
            if (gameViewResult &&
                m11_map_window_pointer_to_game_source(gameView,
                                                       (int)ev.button.x,
                                                       (int)ev.button.y,
                                                       &mappedX,
                                                       &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointerButtonRelease(
                    gameView, mappedX, mappedY, DM1_V1_MOUSE_MASK_LEFT_PC34);
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
        /* SDL3 is the macOS/Steam Deck path.  Keep launcher text editing
         * ahead of the game rename route so RetroAchievements credentials
         * receive both hardware-keyboard and virtual-keyboard text events. */
        if (ev.type == SDL_EVENT_TEXT_INPUT &&
            menuState && useModernLauncher &&
            (!gameView || !gameView->active) &&
            M12_StartupMenu_ConsumeTextInput(menuState, ev.text.text)) {
            if (menuPointerChanged) {
                *menuPointerChanged = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_TEXT_INPUT &&
            m11_dm1_rename_consume_text_input(gameView,
                                              ev.text.text,
                                              gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_EVENT_KEY_DOWN) {
            if (menuState && useModernLauncher &&
                (!gameView || !gameView->active) &&
                M12_StartupMenu_TextEditActive(menuState)) {
                if ((ev.key.mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) &&
                    ev.key.key == SDLK_V) {
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard) {
                        (void)M12_StartupMenu_ConsumeTextInput(menuState,
                                                                clipboard);
                        SDL_free(clipboard);
                    }
                    if (menuPointerChanged) {
                        *menuPointerChanged = 1;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                switch (ev.key.key) {
                    case SDLK_BACKSPACE:
                        (void)M12_StartupMenu_TextEditBackspace(menuState);
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        (void)M12_StartupMenu_TextEditCommit(menuState);
                        break;
                    case SDLK_ESCAPE:
                        (void)M12_StartupMenu_TextEditCancel(menuState);
                        break;
                    default:
                        return M12_MENU_INPUT_NONE;
                }
                if (menuPointerChanged) {
                    *menuPointerChanged = 1;
                }
                return M12_MENU_INPUT_NONE;
            }
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
            if (gameView && gameView->active &&
                m11_game_view_is_dm1(gameView) && ev.key.repeat) {
                M12_MenuInput repeatInput =
                    M11_DM1V1_NavigationInputFromScancode((int)ev.key.scancode);
                if (repeatInput == M12_MENU_INPUT_NONE) {
                    repeatInput =
                        m11_motion_input_from_scancode(ev.key.scancode);
                }
                if (DM1_V1_InputMenuTokenUsesHeldRepeatPc34Compat(
                        (int)repeatInput)) {
                    /* ReDMCSB COMMAND.C F0361 -> F0380 consumes one
                     * keyboard command at a source boundary.  Ignore the
                     * OS autorepeat copy; m11_held_motion_input_from_keyboard
                     * will issue the next held command when that boundary
                     * opens, including Q/E and arrow-key feedback routes. */
                    return M12_MENU_INPUT_NONE;
                }
            }
            if (gameView && gameView->active) {
                M12_MenuInput mappedInput = M12_MENU_INPUT_NONE;
                if (m11_game_view_is_dm1(gameView)) {
                    if (m11_dm1_sdl_key_to_menu_input(
                            (int)ev.key.key,
                            (ev.key.mod & SDL_KMOD_CTRL) != 0,
                            (ev.key.mod & SDL_KMOD_SHIFT) != 0,
                            &mappedInput)) {
                        return mappedInput;
                    }
                    mappedInput =
                        M11_DM1V1_NavigationInputFromScancode((int)ev.key.scancode);
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
                        int ex = DM1_V1_AutoMap_ExportCurrentLevelPc34Compat(gameView);
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
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_1;
                    return M12_MENU_INPUT_NONE;
                case SDLK_2:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_2;
                    return M12_MENU_INPUT_NONE;
                case SDLK_3:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_3;
                    return M12_MENU_INPUT_NONE;
                case SDLK_4:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_4;
                    return M12_MENU_INPUT_NONE;
                case SDLK_5:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_5;
                    return M12_MENU_INPUT_NONE;
                case SDLK_6:
                    if (gameView && gameView->active && (ev.key.mod & SDL_KMOD_SHIFT))
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
                    return (gameView && gameView->active)
                        ? M12_MENU_INPUT_GRAPHICS_POPUP
                        : M12_MENU_INPUT_NONE;
                case SDLK_F11:
                    M11_Render_ToggleFullscreen();
                    return M12_MENU_INPUT_NONE;
                case SDLK_F12:
                    m11_capture_user_screenshot(gameView, menuState);
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
                M11_ApplyStartupMenuRuntime(menuState);
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
                m11_map_window_pointer_to_game_source(gameView,
                                                       ev.motion.x,
                                                       ev.motion.y,
                                                       &mappedX,
                                                       &mappedY)) {
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
                m11_map_window_pointer_to_game_source(gameView,
                                                       ev.button.x,
                                                       ev.button.y,
                                                       &mappedX,
                                                       &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointerButton(
                    gameView,
                    mappedX,
                    mappedY,
                    ev.button.button == SDL_BUTTON_RIGHT
                        ? DM1_V1_MOUSE_MASK_RIGHT_PC34
                        : DM1_V1_MOUSE_MASK_LEFT_PC34);
                if (*gameViewResult != M11_GAME_INPUT_IGNORED) {
                    return M12_MENU_INPUT_NONE;
                }
            }
            continue;
        }
        if (ev.type == SDL_MOUSEBUTTONUP &&
            gameView && gameView->active &&
            ev.button.button == SDL_BUTTON_LEFT) {
            if (gameViewResult &&
                m11_map_window_pointer_to_game_source(gameView,
                                                       ev.button.x,
                                                       ev.button.y,
                                                       &mappedX,
                                                       &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointerButtonRelease(
                    gameView, mappedX, mappedY, DM1_V1_MOUSE_MASK_LEFT_PC34);
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
            menuState && useModernLauncher &&
            (!gameView || !gameView->active) &&
            M12_StartupMenu_ConsumeTextInput(menuState, ev.text.text)) {
            if (menuPointerChanged) {
                *menuPointerChanged = 1;
            }
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_TEXTINPUT &&
            m11_dm1_rename_consume_text_input(gameView,
                                              ev.text.text,
                                              gameViewResult)) {
            return M12_MENU_INPUT_NONE;
        }
        if (ev.type == SDL_KEYDOWN) {
            if (menuState && useModernLauncher &&
                (!gameView || !gameView->active) &&
                M12_StartupMenu_TextEditActive(menuState)) {
                if ((ev.key.keysym.mod & (KMOD_CTRL | KMOD_GUI)) &&
                    ev.key.keysym.sym == SDLK_V) {
                    char* clipboard = SDL_GetClipboardText();
                    if (clipboard) {
                        (void)M12_StartupMenu_ConsumeTextInput(menuState,
                                                                clipboard);
                        SDL_free(clipboard);
                    }
                    if (menuPointerChanged) {
                        *menuPointerChanged = 1;
                    }
                    return M12_MENU_INPUT_NONE;
                }
                switch (ev.key.keysym.sym) {
                    case SDLK_BACKSPACE:
                        (void)M12_StartupMenu_TextEditBackspace(menuState);
                        if (menuPointerChanged) {
                            *menuPointerChanged = 1;
                        }
                        return M12_MENU_INPUT_NONE;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        (void)M12_StartupMenu_TextEditCommit(menuState);
                        if (menuPointerChanged) {
                            *menuPointerChanged = 1;
                        }
                        return M12_MENU_INPUT_NONE;
                    case SDLK_ESCAPE:
                        (void)M12_StartupMenu_TextEditCancel(menuState);
                        if (menuPointerChanged) {
                            *menuPointerChanged = 1;
                        }
                        return M12_MENU_INPUT_NONE;
                    default:
                        return M12_MENU_INPUT_NONE;
                }
            }
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
            if (gameView && gameView->active &&
                m11_game_view_is_dm1(gameView) && ev.key.repeat) {
                M12_MenuInput repeatInput =
                    M11_DM1V1_NavigationInputFromScancode((int)ev.key.keysym.scancode);
                if (repeatInput == M12_MENU_INPUT_NONE) {
                    repeatInput =
                        m11_motion_input_from_scancode(ev.key.keysym.scancode);
                }
                if (DM1_V1_InputMenuTokenUsesHeldRepeatPc34Compat(
                        (int)repeatInput)) {
                    /* See the SDL3 branch above: held-key polling owns
                     * DM1's paced repeat after F0361/F0380. */
                    return M12_MENU_INPUT_NONE;
                }
            }
            if (gameView && gameView->active) {
                M12_MenuInput mappedInput = M12_MENU_INPUT_NONE;
                if (m11_game_view_is_dm1(gameView)) {
                    if (m11_dm1_sdl_key_to_menu_input(
                            (int)ev.key.keysym.sym,
                            (ev.key.keysym.mod & KMOD_CTRL) != 0,
                            (ev.key.keysym.mod & KMOD_SHIFT) != 0,
                            &mappedInput)) {
                        return mappedInput;
                    }
                    mappedInput =
                        M11_DM1V1_NavigationInputFromScancode((int)ev.key.keysym.scancode);
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
                        int ex = DM1_V1_AutoMap_ExportCurrentLevelPc34Compat(gameView);
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
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_1;
                    return M12_MENU_INPUT_NONE;
                case SDLK_2:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_2;
                    return M12_MENU_INPUT_NONE;
                case SDLK_3:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_3;
                    return M12_MENU_INPUT_NONE;
                case SDLK_4:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_4;
                    return M12_MENU_INPUT_NONE;
                case SDLK_5:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
                        return M12_MENU_INPUT_SPELL_RUNE_5;
                    return M12_MENU_INPUT_NONE;
                case SDLK_6:
                    if (gameView && gameView->active && (ev.key.keysym.mod & KMOD_SHIFT))
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
                    return (gameView && gameView->active)
                        ? M12_MENU_INPUT_GRAPHICS_POPUP
                        : M12_MENU_INPUT_NONE;
                case SDLK_F11:
                    M11_Render_ToggleFullscreen();
                    return M12_MENU_INPUT_NONE;
                case SDLK_F12:
                    m11_capture_user_screenshot(gameView, menuState);
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
    if (runtimeOptions.bootProbe && (!runtimeOptions.gameId || runtimeOptions.gameId[0] == '\0')) {
        fprintf(stderr, "firestaff: --boot-probe requires --game <id>\n");
        return 2;
    }
    if (runtimeOptions.bootProbe) {
        char invalidToken[64];
        int invalidCount = M11_BootProbeScript_Validate(runtimeOptions.script,
                                                        invalidToken,
                                                        sizeof(invalidToken));
        if (invalidCount > 0) {
            fprintf(stderr,
                    "firestaff: --boot-probe script contains %d invalid token(s); first invalid token '%s'\n",
                    invalidCount,
                    invalidToken);
            return 5;
        }
        runtimeOptions.directLaunch = 1;
    }
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
    struct Dm1V1PendingMotionQueuePc34Compat pendingDm1V1MotionQueue;
    Firestaff_RA_Runtime raRuntime;

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
    {
        M12_StartupMenuInitOptions menuInitOptions;
        memset(&menuInitOptions, 0, sizeof(menuInitOptions));
        menuInitOptions.skipScreenshotGalleryScan = o->bootProbe ? 1 : 0;
        /* A caller-supplied game-data path is authoritative, including a
         * .7z/.zip/.iso container.  Boot probes still avoid the unrelated
         * screenshot walk, but must not quietly discard archive contents and
         * fall back to a different installed release. */
        menuInitOptions.looseFilesOnlyAssetScan =
            (o->bootProbe && (!o->dataDir || !o->dataDir[0])) ? 1 : 0;
        M12_StartupMenu_InitWithOptions(&menuState,
                                        o->dataDir,
                                        o->gameId,
                                        &menuInitOptions);
    }
    menuState.settings.windowWidth = M11_Render_GetWindowWidth();
    menuState.settings.windowHeight = M11_Render_GetWindowHeight();
    if (o->presentationModeOverride >= M12_PRESENTATION_V1_ORIGINAL &&
        o->presentationModeOverride < M12_PRESENTATION_MODE_COUNT) {
        int game_slot = 0;
        if (o->gameId && strcmp(o->gameId, "csb") == 0) game_slot = 1;
        else if (o->gameId && strcmp(o->gameId, "dm2") == 0) game_slot = 2;
        else if (o->gameId && strcmp(o->gameId, "nexus") == 0) game_slot = 3;
        else if (o->gameId && strcmp(o->gameId, "theron") == 0) game_slot = 4;
        menuState.settings.graphicsIndex = o->presentationModeOverride;
        menuState.gameOptions[game_slot].presentationModeIndex =
            o->presentationModeOverride;
    }
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
    /* Boot probes exercise the launch/runtime contract and emit a
     * machine-readable receipt; they do not draw translated launcher text.
     * Keep that path independent of filesystem-backed PO catalogs so a
     * headless DM1 entrance/HoC probe cannot be delayed by unrelated UI I/O. */
    if (!o->bootProbe) {
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
    gameView.fpsOverlayEnabled = menuState.settings.showFpsOverlay ? 1 : 0;
    if (o->showFpsOverlay) {
        gameView.fpsOverlayEnabled = 1;
    }
    DM1_V1_PendingMotionQueue_InitPc34Compat(&pendingDm1V1MotionQueue);
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
    int startupTextInputActive = 0;
    if (!o->bootProbe && !o->directLaunch && o->durationMs < 0 &&
        getenv("FIRESTAFF_SKIP_INTRO") == NULL &&
        m11_play_firestaff_startup_intro()) {
        goto cleanup;
    }
    M11_ApplyStartupMenuRuntime(&menuState);
    firestaff_ra_runtime_init(&raRuntime);
    if (o->retroAchievementsEnabled ||
        menuState.settings.retroAchievementsEnabled) {
        Firestaff_RA_Config raConfig;
        char redactedToken[16];
        firestaff_ra_config_init(&raConfig);
        raConfig.enabled = 1;
        raConfig.hardcore = o->retroAchievementsEnabled
                                ? (o->retroAchievementsHardcore ? 1 : 0)
                                : (menuState.settings.retroAchievementsHardcore ? 1 : 0);
        firestaff_ra_set_credentials(&raConfig,
                                     o->retroAchievementsUser
                                         ? o->retroAchievementsUser
                                         : (menuState.settings.retroAchievementsUsername[0]
                                                ? menuState.settings.retroAchievementsUsername
                                                : getenv("FIRESTAFF_RA_USER")),
                                     o->retroAchievementsToken
                                         ? o->retroAchievementsToken
                                         : (menuState.settings.retroAchievementsToken[0]
                                                ? menuState.settings.retroAchievementsToken
                                                : getenv("FIRESTAFF_RA_TOKEN")));
        snprintf(raConfig.endpoint,
                 sizeof(raConfig.endpoint),
                 "%s",
                 o->retroAchievementsEndpoint
                     ? o->retroAchievementsEndpoint
                     : (menuState.settings.retroAchievementsEndpoint[0]
                            ? menuState.settings.retroAchievementsEndpoint
                            : "https://retroachievements.org"));
        raRuntime.backend_available = 1;
        firestaff_ra_runtime_apply_config(&raRuntime, &raConfig);
        firestaff_ra_redact_token(raConfig.api_token,
                                  redactedToken,
                                  sizeof(redactedToken));
        fprintf(stderr,
                "RetroAchievements: %s user=%s token=%s hardcore=%d\n",
                firestaff_ra_status_label(firestaff_ra_status(&raRuntime)),
                raConfig.username[0] ? raConfig.username : "(none)",
                redactedToken[0] ? redactedToken : "(none)",
                raConfig.hardcore);
        (void)m11_drain_retroachievements_events(&raRuntime, &gameView);
    }
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
        if (!M11_PrepareDirectLaunchForGame(&menuState, o->gameId)) {
            fprintf(stderr, "firestaff: game unavailable for --game: %s\n",
                    o->gameId ? o->gameId : "(null)");
            runRc = 2;
            goto cleanup;
        }
        /* CLI direct launch bypasses only the M12 menu. The launch still
         * enters through M11_GameView_OpenSelectedMenuEntry(), so DM1 keeps
         * the ReDMCSB TITLE/ENTRANCE order (TITLE.C F0437 before
         * ENTRANCE.C F0441). */
        if (!m11_open_requested_launch(&gameView,
                                       &menuState,
                                       &idleAccumulatorMs,
                                       o->dataDir,
                                       o->bootProbe)) {
            fprintf(stderr, "firestaff: direct launch failed for --game %s\n", o->gameId);
            runRc = 3;
            goto cleanup;
        }
        launchedEver = 1;
        if (o->bootProbe) {
            int frames = o->bootProbeFrames < 0 ? 0 : o->bootProbeFrames;
            int scriptInputs;
            int scriptFrames = 0;
            M11_BootProbeReceipt receipt;
            DM1_V1_StartupSelectedBootProbeFacts_PC34 selectedFacts;
            DM1_V1_StartupSelectedBootProbeReceipt_PC34 selectedReceipt;
            DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34 selectedKindFacts;
            DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34 selectedKindReceipt;
            M11_GameSourceKind expectedSourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
            m11_phase_a_advance_boot_probe_frames(&gameView, frames);
            scriptInputs = m11_phase_a_apply_boot_probe_script(&gameView,
                                                               o->script,
                                                               frames,
                                                               &scriptFrames);
            m11_phase_a_print_boot_probe_receipt(&gameView,
                                                 &menuState,
                                                 o->gameId,
                                                 frames,
                                                 scriptInputs,
                                                 scriptFrames);
            memset(&receipt, 0, sizeof(receipt));
            memset(&selectedFacts, 0, sizeof(selectedFacts));
            memset(&selectedReceipt, 0, sizeof(selectedReceipt));
            memset(&selectedKindFacts, 0, sizeof(selectedKindFacts));
            memset(&selectedKindReceipt, 0, sizeof(selectedKindReceipt));
            if (!M11_GameView_GetBootProbeReceipt(&gameView, &receipt)) {
                fprintf(stderr,
                        "firestaff: boot-probe could not read runtime receipt for --game %s\n",
                        o->gameId ? o->gameId : "");
                runRc = 4;
                goto boot_probe_terminal_exit;
            }
            if ((selectedFacts.expected_game_id = o->gameId,
                 selectedFacts.actual_source_id = receipt.sourceId,
                 selectedFacts.active = receipt.active,
                 selectedFacts.started_from_launcher =
                     receipt.startedFromLauncher,
                 selectedFacts.intro_bypassed =
                     receipt.dm1StartupIntroBypassed,
                 !dm1_v1_startup_selected_boot_probe_receipt_pc34(
                     &selectedFacts,
                     &selectedReceipt)) ||
                !selectedReceipt.valid ||
                (selectedKindFacts.expected_game_id = o->gameId,
                 selectedKindFacts.actual_source_kind = (int)receipt.sourceKind,
                 selectedKindFacts.dm1_builtin_source_kind =
                     (int)M11_GAME_SOURCE_BUILTIN_CATALOG,
                 !dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
                     &selectedKindFacts,
                     &selectedKindReceipt)) ||
                (selectedKindReceipt.handled &&
                 !selectedKindReceipt.valid) ||
                (!selectedKindReceipt.handled &&
                 (!m11_boot_probe_expected_source_kind(o->gameId,
                                                       &expectedSourceKind) ||
                  receipt.sourceKind != expectedSourceKind))) {
                if (selectedKindReceipt.handled) {
                    expectedSourceKind =
                        (M11_GameSourceKind)selectedKindReceipt.expected_source_kind;
                }
                fprintf(stderr,
                        "firestaff: boot-probe expected selected-entry source '%s' kind=%d but got active=%d sourceId='%s' kind=%d startedFromLauncher=%d introBypassed=%d\n",
                        o->gameId ? o->gameId : "",
                        (int)expectedSourceKind,
                        receipt.active,
                        receipt.sourceId,
                        (int)receipt.sourceKind,
                        receipt.startedFromLauncher,
                        receipt.dm1StartupIntroBypassed);
                runRc = 4;
            }
            if (o->bootProbeExpectPhase && o->bootProbeExpectPhase[0] != '\0') {
                if (strcmp(receipt.startupPhase, o->bootProbeExpectPhase) != 0) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected phase '%s' but got '%s'\n",
                            o->bootProbeExpectPhase,
                            receipt.startupPhase);
                    runRc = 4;
                } else if (m11_boot_probe_expected_phase_is_runtime(
                               o->bootProbeExpectPhase) &&
                           !m11_boot_probe_runtime_receipt_ready(&receipt)) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime-ready phase '%s' but receipt has active=%d startupActive=%d levelLoaded=%d\n",
                            o->bootProbeExpectPhase,
                            receipt.active,
                            receipt.startupActive,
                            receipt.levelLoaded);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectRuntime) {
                if (receipt.startupActive ||
                    !receipt.levelLoaded) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime handoff but got phase='%s' startupActive=%d levelLoaded=%d\n",
                            receipt.startupPhase,
                            receipt.startupActive,
                            receipt.levelLoaded);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectParty) {
                if (receipt.partyX != o->bootProbeExpectPartyX ||
                    receipt.partyY != o->bootProbeExpectPartyY ||
                    receipt.partyDir != o->bootProbeExpectPartyDir) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected party %d,%d,%d but got %d,%d,%d\n",
                            o->bootProbeExpectPartyX,
                            o->bootProbeExpectPartyY,
                            o->bootProbeExpectPartyDir,
                            receipt.partyX,
                            receipt.partyY,
                            receipt.partyDir);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectChampions) {
                if (receipt.championCount != o->bootProbeExpectChampionCount) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected champions %d but got %d\n",
                            o->bootProbeExpectChampionCount,
                            receipt.championCount);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectLevelLoaded >= 0) {
                if (receipt.levelLoaded != o->bootProbeExpectLevelLoaded) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected levelLoaded=%d but got %d\n",
                            o->bootProbeExpectLevelLoaded,
                            receipt.levelLoaded);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectAssetMd5 &&
                o->bootProbeExpectAssetMd5[0] != '\0') {
                if (strcmp(receipt.bootAssetMd5,
                           o->bootProbeExpectAssetMd5) != 0) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected asset md5 '%s' but got '%s'\n",
                            o->bootProbeExpectAssetMd5,
                            receipt.bootAssetMd5);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectMap) {
                if (receipt.mapIndex != o->bootProbeExpectMapIndex) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected map %d but got %d\n",
                            o->bootProbeExpectMapIndex,
                            receipt.mapIndex);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectRuntimeTickMin >= 0) {
                if (receipt.runtimeTick < o->bootProbeExpectRuntimeTickMin) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime tick >= %d but got %d\n",
                            o->bootProbeExpectRuntimeTickMin,
                            receipt.runtimeTick);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectRuntimeTickMax >= 0) {
                if (receipt.runtimeTick > o->bootProbeExpectRuntimeTickMax) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected runtime tick <= %d but got %d\n",
                            o->bootProbeExpectRuntimeTickMax,
                            receipt.runtimeTick);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupActive >= 0) {
                if (receipt.startupActive != o->bootProbeExpectStartupActive) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startupActive=%d but got %d\n",
                            o->bootProbeExpectStartupActive,
                            receipt.startupActive);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupFrameMin >= 0) {
                if (receipt.startupFrame < o->bootProbeExpectStartupFrameMin) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startup frame >= %d but got %d\n",
                            o->bootProbeExpectStartupFrameMin,
                            receipt.startupFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupFrameMax >= 0) {
                if (receipt.startupFrame > o->bootProbeExpectStartupFrameMax) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startup frame <= %d but got %d\n",
                            o->bootProbeExpectStartupFrameMax,
                            receipt.startupFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupAnimation &&
                o->bootProbeExpectStartupAnimation[0] != '\0') {
                if (strcmp(receipt.startupAnimation,
                           o->bootProbeExpectStartupAnimation) != 0) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startup animation '%s' but got '%s'\n",
                            o->bootProbeExpectStartupAnimation,
                            receipt.startupAnimation);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectStartupAnimationActive >= 0) {
                if (receipt.startupAnimationActive !=
                        o->bootProbeExpectStartupAnimationActive) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected startupAnimationActive=%d but got %d\n",
                            o->bootProbeExpectStartupAnimationActive,
                            receipt.startupAnimationActive);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleFrameMin >= 0) {
                if (receipt.startupTitleFrame < o->bootProbeExpectTitleFrameMin) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected title frame >= %d but got %d\n",
                            o->bootProbeExpectTitleFrameMin,
                            receipt.startupTitleFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleFrameMax >= 0) {
                if (receipt.startupTitleFrame > o->bootProbeExpectTitleFrameMax) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected title frame <= %d but got %d\n",
                            o->bootProbeExpectTitleFrameMax,
                            receipt.startupTitleFrame);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleFrameBoundary >= 0) {
                if (receipt.startupTitleFrameMax !=
                        o->bootProbeExpectTitleFrameBoundary) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected title frame boundary %d but got %d\n",
                            o->bootProbeExpectTitleFrameBoundary,
                            receipt.startupTitleFrameMax);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectTitleReady >= 0) {
                if (receipt.startupTitleReady != o->bootProbeExpectTitleReady) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected titleReady=%d but got %d\n",
                            o->bootProbeExpectTitleReady,
                            receipt.startupTitleReady);
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectDm1HoCFullGraphics) {
                DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 expectation;
                memset(&expectation, 0, sizeof(expectation));
                if (!dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
                        &receipt.dm1HoCBootSummary,
                        DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_COMPLETE_SUPPORT_PC34,
                        &expectation) ||
                    !expectation.ready) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected DM1 HoC complete support but got %s\n",
                            expectation.diagnostic[0] ?
                                expectation.diagnostic :
                                "missing DM1 expectation diagnostic");
                    runRc = 4;
                }
            }
            if (o->bootProbeExpectDm1HoCReleaseAppCapture) {
                DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 expectation;
                memset(&expectation, 0, sizeof(expectation));
                if (!dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
                        &receipt.dm1HoCBootSummary,
                        DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_RELEASE_APP_CAPTURE_PC34,
                        &expectation) ||
                    !expectation.ready) {
                    fprintf(stderr,
                            "firestaff: boot-probe expected DM1 HoC release-app host capture but got %s\n",
                            expectation.diagnostic[0] ?
                                expectation.diagnostic :
                                "missing DM1 expectation diagnostic");
                    runRc = 4;
                }
            }
            goto boot_probe_terminal_exit;
        }
        if (exitAfterLaunch) {
            goto cleanup;
        }
    } else {
        m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
    }
    if (gameView.active) {
        /* ReDMCSB ENTRANCE.C F0797/F0441 hands its opened Hall frame to
         * DRAWVIEW before input.  The initial HoC frame must use the same
         * M11 presentation path as every later materialized DM1 frame so
         * V1 palette/nearest scaling and V2 targets are applied before the
         * M12 presented-capture consumer records the PC34 receipt.  Calling
         * bare Render_Present() here bypassed that contract on first launch. */
        (void)m11_present_game_frame_and_publish_startup_capture(
            &gameView, &menuState);
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
            /* Normal gameplay uses the original 200 ms source tick divided
             * by the configured speed. CSB startup instead advances one
             * original VBlank per source tick. */
#if SDL_VERSION_ATLEAST(3, 0, 0)
            gameTickInterval = (Uint64)M11_GameView_IdleTickIntervalMs(
                &gameView, speedMul);
#else
            gameTickInterval = (Uint32)M11_GameView_IdleTickIntervalMs(
                &gameView, speedMul);
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
            firestaff_ra_overlay_tick(&gameView.retroAchievementsOverlay,
                                      (int)loopDeltaMs);
            if (gameView.retroAchievementsOverlay.active_valid ||
                gameView.retroAchievementsOverlay.queue_count > 0) {
                gameFrameNeedsPresent = 1;
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
        m11_sync_startup_text_input(&menuState,
                                    useModern && !gameView.active,
                                    &startupTextInputActive);
        if (quitRequested) {
            break;
        }
        if (menuPointerChanged && !gameView.active) {
            if (menuState.shouldExit) {
                break;
            }
            if (m11_open_requested_launch(&gameView,
                                          &menuState,
                                          &idleAccumulatorMs,
                                          o->dataDir,
                                          0)) {
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
                DM1_V1_PendingMotionQueue_ClearPc34Compat(
                    &pendingDm1V1MotionQueue);
                idleAccumulatorMs = 0;
                M11_ApplyStartupMenuRuntime(&menuState);
                m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            } else if (pointerResult == M11_GAME_INPUT_RESTART_GAME) {
                DM1_V1_PendingMotionQueue_ClearPc34Compat(
                    &pendingDm1V1MotionQueue);
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
                if (gameView.graphicsPopupActive) {
                    m11_sync_runtime_graphics_popup_to_menu(&gameView, &menuState);
                }
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
            {
                int pendingInput = M12_MENU_INPUT_NONE;
                if (DM1_V1_PendingMotionQueue_PopPc34Compat(
                        &pendingDm1V1MotionQueue, &pendingInput)) {
                    input = (M12_MenuInput)pendingInput;
                }
            }
            if (input == M12_MENU_INPUT_NONE) {
                input = m11_held_motion_input_from_keyboard(&gameView);
            }
            if (input == M12_MENU_INPUT_NONE) {
                input = m11_held_motion_input_from_gamepad(&gameView,
                                                           &gamepadStatus,
                                                           &gamepadMap);
            }
        }
        if (input != M12_MENU_INPUT_NONE) {
            tickBeforeInput = gameView.world.gameTick;
            if (gameView.active) {
                M11_GameInputResult result = M11_GAME_INPUT_IGNORED;
                if (!gameView.graphicsPopupActive &&
                    input != M12_MENU_INPUT_GRAPHICS_POPUP &&
                    M11_GameView_InputConsumesDm1V1SourceTick(&gameView, input) &&
                    !m11_dm1_v1_input_is_immediate_turn(input) &&
                    !M11_GameView_Dm1V1SourceTickReadyForInput(&gameView)) {
                    /* ReDMCSB COMMAND.C F0359/F0361 queues key commands while
                     * GAMELOOP.C waits for G0321.  COMMAND.C F0361 lines
                     * 1744-1768 admits up to C5 queued keyboard commands
                     * while reserving two queue slots; mirror that bounded
                     * pending shape here so quick Q/E/Home/End/keypad turn
                     * taps are not overwritten before the vblank gate opens. */
                    (void)DM1_V1_PendingMotionQueue_PushPc34Compat(
                        &pendingDm1V1MotionQueue, (int)input);
                    input = M12_MENU_INPUT_NONE;
                } else {
                    result = M11_GameView_HandleInput(&gameView, input);
                }
                if (result == M11_GAME_INPUT_RETURN_TO_MENU) {
                    M11_GameView_Shutdown(&gameView);
                    M11_GameView_Init(&gameView);
                    DM1_V1_PendingMotionQueue_ClearPc34Compat(
                        &pendingDm1V1MotionQueue);
                    idleAccumulatorMs = 0;
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                } else if (result == M11_GAME_INPUT_RESTART_GAME) {
                    DM1_V1_PendingMotionQueue_ClearPc34Compat(
                        &pendingDm1V1MotionQueue);
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
                    if (gameView.graphicsPopupActive ||
                        input == M12_MENU_INPUT_GRAPHICS_POPUP) {
                        m11_sync_runtime_graphics_popup_to_menu(&gameView, &menuState);
                    }
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
                    m11_sync_startup_text_input(&menuState,
                                                useModern && !gameView.active,
                                                &startupTextInputActive);
                    if (menuState.shouldExit) {
                        break;
                    }
                    if (m11_open_requested_launch(&gameView,
                                                  &menuState,
                                                  &idleAccumulatorMs,
                                                  o->dataDir,
                                                  0)) {
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
                DM1_V1_AutoMap_RecordVisitPc34Compat(&gameView);
                DM1_V1_Minimap_RenderPc34Compat(&gameView,
                                   M11_Render_GetFramebuffer(),
                                   M11_FB_WIDTH, M11_FB_HEIGHT);
                DM1_CombatLog_Render(&gameView,
                                     M11_Render_GetFramebuffer(),
                                     M11_FB_WIDTH, M11_FB_HEIGHT);
                M11_GameView_DrawGraphicsPopup(&gameView,
                                                M11_Render_GetFramebuffer(),
                                                M11_FB_WIDTH, M11_FB_HEIGHT);
                M11_GameView_RecordPresentedFrame(&gameView, SDL_GetTicks());
                M11_GameView_DrawFpsOverlay(&gameView,
                                             M11_Render_GetFramebuffer(),
                                             M11_FB_WIDTH, M11_FB_HEIGHT);
                (void)m11_present_game_frame_and_publish_startup_capture(
                    &gameView, &menuState);
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
    m11_sync_startup_text_input(&menuState, 0, &startupTextInputActive);
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

boot_probe_terminal_exit:
    /* Process-terminal probe path: the printed boot receipt is the contract.
     * Do not enter live runtime/menu teardown here; DM1 has already reached
     * runtime and some shutdown paths are intentionally game-loop-owned.
     * The SDL renderer, however, is owned by M11_Render alone, so shut it
     * down: without this a second in-process M11_PhaseA_Run boot probe
     * (multi-game gates) dies on M11_RENDER_ERR_ALREADY_INIT. */
    M11_Render_Shutdown();
    free(launcherFramebuffer);
    if (modernRgba) {
        free(modernRgba);
    }
    return runRc;
}
