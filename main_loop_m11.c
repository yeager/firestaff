/*
 * main_loop_m11.c — M11 Phase A stub.
 *
 * Opens a window via render_sdl_m11, presents a black framebuffer, pumps
 * events until either the user quits or the configured duration elapses,
 * then shuts down.
 */

#include "main_loop_m11.h"

#include "menu_startup_m12.h"
#include "m11_game_view.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#endif

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
    opts->windowWidth    = 640;
    opts->windowHeight   = 400;
    opts->scaleMode      = M11_SCALE_2X;
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

static M12_MenuInput m11_poll_menu_input(M11_GameViewState* gameView,
                                         M11_GameInputResult* gameViewResult,
                                         int* quitRequested) {
    SDL_Event ev;
    int mappedX;
    int mappedY;
    if (gameViewResult) {
        *gameViewResult = M11_GAME_INPUT_IGNORED;
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
    int quitRequested = 0;
    uint32_t idleAccumulatorMs = 0;

    int rc = M11_Render_Init(o->windowWidth, o->windowHeight, o->scaleMode);
    if (rc != M11_RENDER_OK) {
        return rc;
    }

    M12_StartupMenu_InitWithDataDir(&menuState, o->dataDir);
    M11_GameView_Init(&gameView);
    M11_ApplyStartupMenuRuntime(&menuState);
    M12_StartupMenu_Draw(&menuState,
                         M11_Render_GetFramebuffer(),
                         M11_FB_WIDTH,
                         M11_FB_HEIGHT);

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
    M11_Render_Present();

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
        if (input == M12_MENU_INPUT_NONE) {
            input = m11_poll_menu_input(&gameView, &pointerResult, &quitRequested);
        }
        if (quitRequested) {
            break;
        }
        if (pointerResult != M11_GAME_INPUT_IGNORED) {
            if (pointerResult == M11_GAME_INPUT_RETURN_TO_MENU) {
                M11_GameView_Shutdown(&gameView);
                M11_GameView_Init(&gameView);
                idleAccumulatorMs = 0;
                M11_ApplyStartupMenuRuntime(&menuState);
                M12_StartupMenu_Draw(&menuState,
                                     M11_Render_GetFramebuffer(),
                                     M11_FB_WIDTH,
                                     M11_FB_HEIGHT);
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
                    M12_StartupMenu_Draw(&menuState,
                                         M11_Render_GetFramebuffer(),
                                         M11_FB_WIDTH,
                                         M11_FB_HEIGHT);
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
                            M12_StartupMenu_Draw(&menuState,
                                                 M11_Render_GetFramebuffer(),
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT);
                        }
                    }
                }
                if (!launchHandled) {
                    if (input == M12_MENU_INPUT_ACTION ||
                        input == M12_MENU_INPUT_CYCLE_CHAMPION ||
                        input == M12_MENU_INPUT_STRAFE_LEFT ||
                        input == M12_MENU_INPUT_STRAFE_RIGHT) {
                        input = M12_MENU_INPUT_NONE;
                    }
                    M12_StartupMenu_HandleInput(&menuState, input);
                    if (menuState.shouldExit) {
                        break;
                    }
                    M11_ApplyStartupMenuRuntime(&menuState);
                    M12_StartupMenu_Draw(&menuState,
                                         M11_Render_GetFramebuffer(),
                                         M11_FB_WIDTH,
                                         M11_FB_HEIGHT);
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
        M11_Render_Present();
        SDL_Delay((Uint32)interval);
        now = SDL_GetTicks();
    }

    M11_GameView_Shutdown(&gameView);
    M11_Render_Shutdown();
    return 0;
}
