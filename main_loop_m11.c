/*
 * main_loop_m11.c — M11 Phase A stub.
 *
 * Opens a window via render_sdl_m11, presents a black framebuffer, pumps
 * events until either the user quits or the configured duration elapses,
 * then shuts down.
 */

#include "main_loop_m11.h"

#include "menu_startup_m12.h"
#include "menu_startup_render_modern_m12.h"
#include "menu_hit_m12.h"
#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "title_frontend_v1.h"
#include "asset_status_m12.h"
#include "fs_portable_compat.h"
#include "entrance_frontend_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
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

void M11_ApplyStartupMenuRuntime(const M12_StartupMenuState* menuState) {
    if (!menuState) {
        return;
    }
    M11_Render_SetPaletteLevel(M12_StartupMenu_GetRenderPaletteLevel(menuState));
    M11_Render_SetWindowMode(menuState->settings.windowModeIndex);
    M11_Render_SetScaleMode(menuState->settings.scaleModeIndex);
    M11_Render_SetIntegerScaling(menuState->settings.integerScaling);
    M11_Render_SetScaleFilter(menuState->settings.scalingFilterIndex);
    M11_Render_SetVSync(menuState->settings.vsyncIndex);
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
        "DungeonMasterPC34/TITLE",
        "DungeonMasterPC34Multilingual/TITLE",
        "dm-pc34/DungeonMasterPC34/TITLE",
        "dm-pc34/DungeonMasterPC34Multilingual/TITLE"
    };

    if (!outPath || outPathBytes == 0U) {
        return 0;
    }
    outPath[0] = '\0';

    envPath = getenv("FIRESTAFF_TITLE_DAT");
    if (envPath && envPath[0] != '\0' && FSP_FileExists(envPath)) {
        snprintf(outPath, outPathBytes, "%s", envPath);
        return 1;
    }

    if (menuState) {
        for (i = 0U; i < M12_AssetStatus_GetVersionCount("dm1"); ++i) {
            dm1v = M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1", i);
            if (dm1v && dm1v->matched && FSP_ParentDir(parent, sizeof(parent), dm1v->matchedPath)) {
                if (FSP_JoinPath(candidate, sizeof(candidate), parent, "TITLE") &&
                    FSP_FileExists(candidate)) {
                    snprintf(outPath, outPathBytes, "%s", candidate);
                    return 1;
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
            FSP_FileExists(candidate)) {
            snprintf(outPath, outPathBytes, "%s", candidate);
            return 1;
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

static int m11_draw_entrance_opening_doors_asset(M11_GameViewState* gameView,
                                                 unsigned char* framebuffer,
                                                 const EntranceCompatDoorStep* door) {
    const M11_AssetSlot* leftDoor;
    const M11_AssetSlot* rightDoor;
    int drew = 0;
    if (!gameView || !framebuffer || !door || !gameView->assetsAvailable) {
        return 0;
    }
    leftDoor = M11_AssetLoader_Load(&gameView->assetLoader, 2U);
    rightDoor = M11_AssetLoader_Load(&gameView->assetLoader, 3U);
    if (!leftDoor || !rightDoor) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C:189-231 blits source door strips from the
     * precomputed C002/C003 animation-step bitmaps using the DATA.C opening
     * boxes.  The compat schedule already exposes those boxes/source X values. */
    if (door->leftBoxW > 0U && leftDoor->height >= door->leftBoxH &&
        leftDoor->width >= door->leftSourceX + door->leftBoxW) {
        M11_AssetLoader_BlitRegion(leftDoor,
                                   (int)door->leftSourceX, 0,
                                   (int)door->leftBoxW, (int)door->leftBoxH,
                                   framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                                   (int)door->leftBoxX, 28 + (int)door->leftBoxY,
                                   -1);
        drew = 1;
    }
    if (door->rightBoxW > 0U && rightDoor->height >= door->rightBoxH &&
        rightDoor->width >= door->rightSourceX + door->rightBoxW) {
        M11_AssetLoader_BlitRegion(rightDoor,
                                   (int)door->rightSourceX, 0,
                                   (int)door->rightBoxW, (int)door->rightBoxH,
                                   framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT,
                                   (int)door->rightBoxX, 28 + (int)door->rightBoxY,
                                   -1);
        drew = 1;
    }
    return drew;
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

static int m11_wait_for_redmcsb_entrance_command(void);

static int m11_play_redmcsb_entrance_transition(M11_GameViewState* gameView) {
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
            memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
            if (ENTRANCE_Compat_GetDoorAnimationStep(sourceStep - 6U, &door)) {
                if (!m11_draw_entrance_opening_doors_asset(gameView, framebuffer, &door)) {
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

        M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
        if (step.kind == ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT) {
            if (!m11_wait_for_redmcsb_entrance_command()) {
                free(dungeonFrame);
                return 0;
            }
        }
        if (step.delayTicks >= 20U) {
            SDL_Delay(330);
        } else {
            SDL_Delay(step.vblankLoopCount ? 16 : 33);
        }
        if (M11_Render_PumpEvents()) break;
    }
    memcpy(framebuffer, dungeonFrame, (size_t)M11_FB_BYTES);
    M11_Render_PresentIndexed(framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    free(dungeonFrame);
    return 1;
}

static int m11_wait_for_redmcsb_entrance_command(void) {
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
            if (ev.type == SDL_EVENT_QUIT) return 0;
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                if (ev.key.key == SDLK_ESCAPE || ev.key.key == SDLK_Q) return 0;
                if (ev.key.key == SDLK_RETURN || ev.key.key == SDLK_KP_ENTER ||
                    ev.key.key == SDLK_SPACE) return 1;
            }
            if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN) return 1;
            if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#else
            if (ev.type == SDL_QUIT) return 0;
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE || ev.key.keysym.sym == SDLK_Q) return 0;
                if (ev.key.keysym.sym == SDLK_RETURN || ev.key.keysym.sym == SDLK_KP_ENTER ||
                    ev.key.keysym.sym == SDLK_SPACE) return 1;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN) return 1;
            if (ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                M11_Render_HandleResize(ev.window.data1, ev.window.data2);
            }
#endif
        }

        /* Scripted/headless probes cannot send a second physical command.
         * Keep the real app faithful by waiting indefinitely, but avoid
         * deadlocks under the SDL dummy driver / explicit autotest mode. */
        if (allowHeadlessTimeout && SDL_GetTicks() - started > 5000U) {
            return 1;
        }
        SDL_Delay(16);
    }
}

static void m11_play_redmcsb_title_intro_if_available(const M12_StartupMenuState* menuState,
                                                      int* outPlayedAnyFrame) {
    char titlePath[FSP_PATH_MAX];
    unsigned char* packedStorage;
    unsigned char* packedScreen;
    unsigned char* indexedScreen;
    char err[160];
    unsigned int step;
    V1_TitleFrontendSourceTiming timing;

    if (outPlayedAnyFrame) {
        *outPlayedAnyFrame = 0;
    }
    if (!m11_find_title_dat_for_intro(menuState, titlePath, sizeof(titlePath))) {
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
    (void)timing;

    /* ReDMCSB TITLE.C source-lock:
     *   TITLE.C:430 draws PRESENTS.
     *   TITLE.C:456 waits M526_WaitVerticalBlank() before each reverse-order
     *               zoom blit to C425_ZONE_TITLE_CHAOS.
     *   TITLE.C:460 delays 20 ticks.
     *   TITLE.C:461 draws STRIKES BACK.
     *   TITLE.C:463 final delay/guard before the next screen.
     * Runtime uses the already decoded original 53-frame TITLE bank as the
     * visible source and presents it before the launcher, instead of skipping
     * straight to the menu. */
    for (step = 1U; step <= V1_TITLE_DAT_FRAME_MAX; ++step) {
        V1_TitleFrontendSequenceDecision d = V1_TitleFrontend_DecideSequenceStep(step);
        memset(packedStorage, 0, 4U + 32000U);
        memset(indexedScreen, 0, (size_t)M11_FB_BYTES);
        err[0] = '\0';
        if (!V1_TitleFrontend_RenderFrameToScreen(titlePath,
                                                  d.renderFrameOrdinal,
                                                  packedScreen,
                                                  NULL,
                                                  err,
                                                  sizeof(err))) {
            break;
        }
        m11_unpack_title_4bpp_to_indexed(packedScreen, indexedScreen);
        if (outPlayedAnyFrame) {
            *outPlayedAnyFrame = 1;
        }
        M11_Render_PresentIndexed(indexedScreen, M11_FB_WIDTH, M11_FB_HEIGHT);
        /* ReDMCSB TITLE.C:201-214 gates the zoom on vertical blanks, then
         * TITLE.C:251 adds a final BUG0_71 guard so fast machines do not
         * smash straight into the entrance screen.  The decoded TITLE.DAT
         * bank has more visible frames than the source zoom loop, so use a
         * deliberate 50 ms presentation cadence here instead of racing the
         * launcher at full frame speed. */
        SDL_Delay(50);
        if (M11_Render_PumpEvents()) {
            break;
        }
    }
    SDL_Delay(500);
    free(packedStorage);
    free(indexedScreen);
}

static int m11_open_requested_launch(M11_GameViewState* gameView,
                                     M12_StartupMenuState* menuState,
                                     uint32_t* idleAccumulatorMs) {
    if (!gameView || !menuState || !menuState->launchRequested) {
        return 0;
    }
    if (M11_GameView_OpenSelectedMenuEntry(gameView, menuState)) {
        menuState->launchRequested = 0;
        (void)M11_Render_SetPaletteLevel(0);
        if (idleAccumulatorMs) {
            *idleAccumulatorMs = 0;
        }
        if (M12_StartupMenu_GetPresentationMode(menuState) == M12_PRESENTATION_V1_ORIGINAL) {
            if (!m11_play_redmcsb_entrance_transition(gameView)) {
                M11_GameView_Shutdown(gameView);
                M11_GameView_Init(gameView);
                return 1;
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
    if ((len == 4U && strncmp(token, "left", len) == 0) ||
        (len == 1U && strncmp(token, "l", len) == 0)) {
        return M12_MENU_INPUT_LEFT;
    }
    if ((len == 5U && strncmp(token, "right", len) == 0) ||
        (len == 1U && strncmp(token, "r", len) == 0)) {
        return M12_MENU_INPUT_RIGHT;
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

static M12_MenuInput m11_poll_menu_input(M11_GameViewState* gameView,
                                         M12_StartupMenuState* menuState,
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
            gameView && gameView->active && ev.button.button == SDL_BUTTON_LEFT) {
            if (gameViewResult &&
                M11_Render_MapWindowToFramebuffer((int)ev.button.x,
                                                  (int)ev.button.y,
                                                  &mappedX,
                                                  &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointer(
                    gameView,
                    mappedX,
                    mappedY,
                    1);
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
        if (ev.type == SDL_EVENT_KEY_DOWN) {
            switch (ev.key.key) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_LEFT:
                case SDLK_Q:
                    return M12_MENU_INPUT_LEFT;
                case SDLK_RIGHT:
                case SDLK_E:
                    return M12_MENU_INPUT_RIGHT;
                case SDLK_A:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_LEFT;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_D:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_RIGHT;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_W:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_UP;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_S:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_DOWN;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
                case SDLK_SPACE:
                    return M12_MENU_INPUT_ACTION;
                case SDLK_TAB:
                    return M12_MENU_INPUT_CYCLE_CHAMPION;
                case SDLK_F5:
                    if (gameView && gameView->active && M11_GameView_QuickSave(gameView)) {
                        if (gameViewResult) {
                            *gameViewResult = M11_GAME_INPUT_REDRAW;
                        }
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_F9:
                    if (gameView && gameView->active && M11_GameView_QuickLoad(gameView)) {
                        if (gameViewResult) {
                            *gameViewResult = M11_GAME_INPUT_REDRAW;
                        }
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
            gameView && gameView->active && ev.button.button == SDL_BUTTON_LEFT) {
            if (gameViewResult &&
                M11_Render_MapWindowToFramebuffer(ev.button.x,
                                                  ev.button.y,
                                                  &mappedX,
                                                  &mappedY)) {
                *gameViewResult = M11_GameView_HandlePointer(
                    gameView,
                    mappedX,
                    mappedY,
                    1);
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
        if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_LEFT:
                case SDLK_Q:
                    return M12_MENU_INPUT_LEFT;
                case SDLK_RIGHT:
                case SDLK_E:
                    return M12_MENU_INPUT_RIGHT;
                case SDLK_A:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_LEFT;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_D:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_RIGHT;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_W:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_UP;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_S:
                    if (menuState && menuState->settings.wasdMovementEnabled) {
                        return M12_MENU_INPUT_DOWN;
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
                case SDLK_SPACE:
                    return M12_MENU_INPUT_ACTION;
                case SDLK_TAB:
                    return M12_MENU_INPUT_CYCLE_CHAMPION;
                case SDLK_F5:
                    if (gameView && gameView->active && M11_GameView_QuickSave(gameView)) {
                        if (gameViewResult) {
                            *gameViewResult = M11_GAME_INPUT_REDRAW;
                        }
                    }
                    return M12_MENU_INPUT_NONE;
                case SDLK_F9:
                    if (gameView && gameView->active && M11_GameView_QuickLoad(gameView)) {
                        if (gameViewResult) {
                            *gameViewResult = M11_GAME_INPUT_REDRAW;
                        }
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
    const M11_PhaseA_Options* o = opts ? opts : &defaults;
    M12_StartupMenuState menuState;
    M11_GameViewState gameView;
    const char* scriptCursor = o->script;
    unsigned char* launcherFramebuffer = NULL;
    unsigned char* modernRgba = NULL;
    int useModern = 0;
    int quitRequested = 0;
    uint32_t idleAccumulatorMs = 0;

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
    M12_StartupMenu_InitWithDataDir(&menuState, o->dataDir);
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
    M11_GameView_Init(&gameView);
    M11_ApplyStartupMenuRuntime(&menuState);
    {
        int titleIntroPlayed = 0;
        if (M12_StartupMenu_GetPresentationMode(&menuState) == M12_PRESENTATION_V1_ORIGINAL) {
            m11_play_redmcsb_title_intro_if_available(&menuState, &titleIntroPlayed);
        }
        (void)titleIntroPlayed;
    }
    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);

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
    const Uint64 gameTickInterval = 166;
#else
    Uint32 start = SDL_GetTicks();
    Uint32 now = start;
    Uint32 lastLoopTick = start;
    const Uint32 duration = (Uint32)(o->durationMs < 0 ? 0 : o->durationMs);
    const Uint32 interval = (Uint32)(o->presentEveryMs < 1
                                         ? 1
                                         : o->presentEveryMs);
    const Uint32 gameTickInterval = 166;
#endif

    /* Always present at least once so the window actually has content. */
    m11_present_launcher(launcherFramebuffer, modernRgba, useModern);

    while (o->durationMs < 0 || (now - start) < duration) {
        M12_MenuInput input = M12_MENU_INPUT_NONE;
        M11_GameInputResult pointerResult = M11_GAME_INPUT_IGNORED;
        uint32_t tickBeforeEvents = gameView.world.gameTick;
        uint32_t tickBeforeInput = gameView.world.gameTick;

        now = SDL_GetTicks();
        if (gameView.active) {
            idleAccumulatorMs += (uint32_t)(now - lastLoopTick);
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
            if (m11_open_requested_launch(&gameView, &menuState, &idleAccumulatorMs)) {
                continue;
            }
            M11_ApplyStartupMenuRuntime(&menuState);
            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
        }
        if (pointerResult != M11_GAME_INPUT_IGNORED) {
            if (pointerResult == M11_GAME_INPUT_RETURN_TO_MENU) {
                M11_GameView_Shutdown(&gameView);
                M11_GameView_Init(&gameView);
                idleAccumulatorMs = 0;
                M11_ApplyStartupMenuRuntime(&menuState);
                m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            } else if (pointerResult == M11_GAME_INPUT_REDRAW) {
                M11_GameView_Draw(&gameView,
                                  M11_Render_GetFramebuffer(),
                                  M11_FB_WIDTH,
                                  M11_FB_HEIGHT);
                if (gameView.world.gameTick != tickBeforeEvents) {
                    idleAccumulatorMs = 0;
                }
            }
        }
        if (input != M12_MENU_INPUT_NONE) {
            tickBeforeInput = gameView.world.gameTick;
            if (gameView.active) {
                M11_GameInputResult result = M11_GameView_HandleInput(&gameView, input);
                if (result == M11_GAME_INPUT_RETURN_TO_MENU) {
                    M11_GameView_Shutdown(&gameView);
                    M11_GameView_Init(&gameView);
                    idleAccumulatorMs = 0;
                    M11_ApplyStartupMenuRuntime(&menuState);
                    m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                } else if (result == M11_GAME_INPUT_REDRAW) {
                    M11_GameView_Draw(&gameView,
                                      M11_Render_GetFramebuffer(),
                                      M11_FB_WIDTH,
                                      M11_FB_HEIGHT);
                    if (gameView.world.gameTick != tickBeforeInput) {
                        idleAccumulatorMs = 0;
                    }
                }
            } else {
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
                if (m11_open_requested_launch(&gameView, &menuState, &idleAccumulatorMs)) {
                    continue;
                }
                M11_ApplyStartupMenuRuntime(&menuState);
                m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            }
        }
        while (gameView.active && idleAccumulatorMs >= (uint32_t)gameTickInterval) {
            if (M11_GameView_AdvanceIdleTick(&gameView) == M11_GAME_INPUT_REDRAW) {
                M11_GameView_Draw(&gameView,
                                  M11_Render_GetFramebuffer(),
                                  M11_FB_WIDTH,
                                  M11_FB_HEIGHT);
            }
            idleAccumulatorMs -= (uint32_t)gameTickInterval;
        }
        if (gameView.active) {
            M11_Render_Present();
        } else {
            /* Redraw the launcher every tick so animations (pulse,
             * hover) remain alive even without input. */
            menuState.frameTick += 1U;
            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            m11_present_launcher(launcherFramebuffer, modernRgba, useModern);
        }
        SDL_Delay((Uint32)interval);
        now = SDL_GetTicks();
    }

    M11_GameView_Shutdown(&gameView);
    free(launcherFramebuffer);
    if (modernRgba) {
        free(modernRgba);
    }
    M11_Render_Shutdown();
    return 0;
}
