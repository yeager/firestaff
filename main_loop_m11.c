/*
 * main_loop_m11.c — M11 Phase A stub.
 *
 * Opens a window via render_sdl_m11, presents a black framebuffer, pumps
 * events until either the user quits or the configured duration elapses,
 * then shuts down.
 */

#include "main_loop_m11.h"

#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#endif

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
    if ((len == 5U && strncmp(token, "enter", len) == 0) ||
        (len == 6U && strncmp(token, "return", len) == 0)) {
        return M12_MENU_INPUT_ACCEPT;
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

static M12_MenuInput m11_poll_menu_input(int* quitRequested) {
    SDL_Event ev;
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
        if (ev.type == SDL_EVENT_KEY_DOWN) {
            switch (ev.key.key) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
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
        if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
                case SDLK_UP:
                    return M12_MENU_INPUT_UP;
                case SDLK_DOWN:
                    return M12_MENU_INPUT_DOWN;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    return M12_MENU_INPUT_ACCEPT;
                case SDLK_ESCAPE:
                    return M12_MENU_INPUT_BACK;
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
    const char* scriptCursor = o->script;
    int quitRequested = 0;

    int rc = M11_Render_Init(o->windowWidth, o->windowHeight, o->scaleMode);
    if (rc != M11_RENDER_OK) {
        return rc;
    }

    M12_StartupMenu_Init(&menuState);
    M12_StartupMenu_Draw(&menuState,
                         M11_Render_GetFramebuffer(),
                         M11_FB_WIDTH,
                         M11_FB_HEIGHT);

    /* Compute deadlines using millisecond ticks. SDL_GetTicks returns
       Uint64 in SDL3 and Uint32 in SDL2. Both are fine for our math. */
#if SDL_VERSION_ATLEAST(3, 0, 0)
    Uint64 start = SDL_GetTicks();
    Uint64 now = start;
    const Uint64 duration = (Uint64)(o->durationMs < 0 ? 0 : o->durationMs);
    const Uint64 interval = (Uint64)(o->presentEveryMs < 1
                                         ? 1
                                         : o->presentEveryMs);
#else
    Uint32 start = SDL_GetTicks();
    Uint32 now = start;
    const Uint32 duration = (Uint32)(o->durationMs < 0 ? 0 : o->durationMs);
    const Uint32 interval = (Uint32)(o->presentEveryMs < 1
                                         ? 1
                                         : o->presentEveryMs);
#endif

    /* Always present at least once so the window actually has content. */
    M11_Render_Present();

    while (o->durationMs < 0 || (now - start) < duration) {
        M12_MenuInput input = M12_MENU_INPUT_NONE;
        if (scriptCursor && *scriptCursor != '\0') {
            input = m11_next_script_input(&scriptCursor);
        }
        if (input == M12_MENU_INPUT_NONE) {
            input = m11_poll_menu_input(&quitRequested);
        }
        if (quitRequested) {
            break;
        }
        if (input != M12_MENU_INPUT_NONE) {
            M12_StartupMenu_HandleInput(&menuState, input);
            if (menuState.shouldExit) {
                break;
            }
            M12_StartupMenu_Draw(&menuState,
                                 M11_Render_GetFramebuffer(),
                                 M11_FB_WIDTH,
                                 M11_FB_HEIGHT);
        }
        M11_Render_Present();
        SDL_Delay((Uint32)interval);
        now = SDL_GetTicks();
    }

    M11_Render_Shutdown();
    return 0;
}
