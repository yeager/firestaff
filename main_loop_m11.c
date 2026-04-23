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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
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
     * content dimensions (1280x720 for modern, 480x270 for legacy).
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
                    return gameView && gameView->active
                               ? M12_MENU_INPUT_STRAFE_LEFT
                               : M12_MENU_INPUT_LEFT;
                case SDLK_D:
                    return gameView && gameView->active
                               ? M12_MENU_INPUT_STRAFE_RIGHT
                               : M12_MENU_INPUT_RIGHT;
                case SDLK_W:
                    return M12_MENU_INPUT_UP;
                case SDLK_S:
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
                    return gameView && gameView->active
                               ? M12_MENU_INPUT_STRAFE_LEFT
                               : M12_MENU_INPUT_LEFT;
                case SDLK_D:
                    return gameView && gameView->active
                               ? M12_MENU_INPUT_STRAFE_RIGHT
                               : M12_MENU_INPUT_RIGHT;
                case SDLK_W:
                    return M12_MENU_INPUT_UP;
                case SDLK_S:
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
    int useModern = m11_legacy_menu_requested() ? 0 : 1;
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

    M12_StartupMenu_InitWithDataDir(&menuState, o->dataDir);
    M11_GameView_Init(&gameView);
    M11_ApplyStartupMenuRuntime(&menuState);
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
            M11_ApplyStartupMenuRuntime(&menuState);
            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
            /* If the click flipped us into game-options and the user
             * next triggers launch, we still need the game-view open
             * path below, which requires an explicit ACCEPT. No harm
             * done here — continue to let polling drive the rest. */
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
                int launchHandled = 0;
                if ((input == M12_MENU_INPUT_ACCEPT || input == M12_MENU_INPUT_RIGHT) &&
                    menuState.view == M12_MENU_VIEW_MAIN) {
                    launchHandled = 1;
                    if (M11_GameView_OpenSelectedMenuEntry(&gameView, &menuState)) {
                        /* Keep V1 in-game colors at the original bright base
                         * palette so launcher palette settings do not leak
                         * into gameplay rendering. */
                        (void)M11_Render_SetPaletteLevel(0);
                        idleAccumulatorMs = 0;
                        M11_GameView_Draw(&gameView,
                                          M11_Render_GetFramebuffer(),
                                          M11_FB_WIDTH,
                                          M11_FB_HEIGHT);
                    } else {
                        const M12_MenuEntry* selected = M12_StartupMenu_GetEntry(&menuState,
                                                                                 menuState.selectedIndex);
                        if (!selected || selected->kind != M12_MENU_ENTRY_GAME || !selected->available) {
                            launchHandled = 0;
                        } else {
                            menuState.view = M12_MENU_VIEW_MESSAGE;
                            menuState.messageLine1 = "DUNGEON LOAD FAILED";
                            menuState.messageLine2 = "CHECK DUNGEON.DAT";
                            menuState.messageLine3 = "ESC RETURNS TO MENU";
                            m11_draw_launcher(&menuState, launcherFramebuffer, modernRgba, useModern);
                        }
                    }
                }
                if (!launchHandled) {
                    if (input == M12_MENU_INPUT_ACTION ||
                        input == M12_MENU_INPUT_CYCLE_CHAMPION ||
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
