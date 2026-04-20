/*
 * render_sdl_m11.c — M11 Phase A implementation.
 *
 * SDL3 is preferred; SDL2 is a drop-in fallback behind a version check.
 * No platform ifdefs (no __APPLE__, no _WIN32) in this file; the
 * SDL_VERSION_ATLEAST gate selects API shapes, not platforms.
 *
 * The module keeps a single internal state block. All public functions
 * are safe to call in any order — double-init returns an error, calls
 * before init return errors without crashing, shutdown is idempotent.
 */

#include "render_sdl_m11.h"

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
/* Fall back to SDL2 header if someone pointed us at an SDL2 install via
   a pkg-config that emitted -I.../SDL2. */
#include <SDL.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "vga_palette_pc34_compat.h"

#if SDL_VERSION_ATLEAST(3, 0, 0)
#define M11_SDL_MAJOR 3
#else
#define M11_SDL_MAJOR 2
#endif

/* ---------------- Internal state ---------------- */

typedef struct {
    int initialised;
    int windowW;
    int windowH;
    int scaleMode;
    int paletteLevel;
    int windowMode;
    int quitRequested;

    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  texture;

    unsigned char  framebuffer[M11_FB_BYTES];
    unsigned char* presentBuffer; /* 320*200*4 bytes RGBA */
} M11_RenderState;

static M11_RenderState g_state = {0};

/* ---------------- Helpers ---------------- */

static void m11_free_present_buffer(void) {
    if (g_state.presentBuffer) {
        free(g_state.presentBuffer);
        g_state.presentBuffer = NULL;
    }
}

static int m11_validate_scale(int mode) {
    return (mode >= M11_SCALE_1X && mode <= M11_SCALE_STRETCH) ? 1 : 0;
}

static int m11_validate_window_mode(int mode) {
    return mode == M11_WINDOW_MODE_WINDOWED || mode == M11_WINDOW_MODE_FULLSCREEN;
}

static void m11_compute_present_rect(int* outX, int* outY, int* outW, int* outH) {
    int x = 0;
    int y = 0;
    int w = g_state.windowW;
    int h = g_state.windowH;

    if (g_state.windowW <= 0 || g_state.windowH <= 0) {
        w = M11_FB_WIDTH;
        h = M11_FB_HEIGHT;
    } else {
        switch (g_state.scaleMode) {
            case M11_SCALE_1X:
            case M11_SCALE_2X:
            case M11_SCALE_3X:
            case M11_SCALE_4X: {
                int factor = g_state.scaleMode + 1;
                w = M11_FB_WIDTH * factor;
                h = M11_FB_HEIGHT * factor;
                break;
            }
            case M11_SCALE_FIT: {
                int fitW = g_state.windowW;
                int fitH = (fitW * M11_FB_HEIGHT) / M11_FB_WIDTH;
                if (fitH > g_state.windowH) {
                    fitH = g_state.windowH;
                    fitW = (fitH * M11_FB_WIDTH) / M11_FB_HEIGHT;
                }
                if (fitW < 1) fitW = 1;
                if (fitH < 1) fitH = 1;
                w = fitW;
                h = fitH;
                break;
            }
            case M11_SCALE_STRETCH:
            default:
                w = g_state.windowW;
                h = g_state.windowH;
                break;
        }
    }

    x = (g_state.windowW - w) / 2;
    y = (g_state.windowH - h) / 2;
    if (outX) *outX = x;
    if (outY) *outY = y;
    if (outW) *outW = w;
    if (outH) *outH = h;
}

static int m11_apply_window_mode(int windowMode) {
    if (!g_state.window) {
        return M11_RENDER_ERR_WINDOW;
    }
    if (!m11_validate_window_mode(windowMode)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
    if (!SDL_SetWindowFullscreen(g_state.window,
                                 windowMode == M11_WINDOW_MODE_FULLSCREEN)) {
        return M11_RENDER_ERR_WINDOW;
    }
#else
    if (SDL_SetWindowFullscreen(g_state.window,
                                windowMode == M11_WINDOW_MODE_FULLSCREEN
                                    ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                    : 0) != 0) {
        return M11_RENDER_ERR_WINDOW;
    }
#endif
    g_state.windowMode = windowMode;
    return M11_RENDER_OK;
}

/* Build the 32-bit RGBA presentation pixels from the 4-bit framebuffer
   using the currently active VGA palette level.
   Pixel format: SDL_PIXELFORMAT_RGBA32 — R, G, B, A in memory order. */
static void m11_framebuffer_to_rgba(void) {
    const unsigned char* src = g_state.framebuffer;
    unsigned char* dst = g_state.presentBuffer;
    const int level = g_state.paletteLevel;
    if (!dst) {
        return;
    }
    for (int i = 0; i < M11_FB_BYTES; ++i) {
        unsigned char idx = src[i] & 0x0F; /* 4-bit palette */
        const unsigned char* rgb =
            G9010_auc_VgaPaletteAll_Compat[level][idx];
        dst[i * 4 + 0] = rgb[0];
        dst[i * 4 + 1] = rgb[1];
        dst[i * 4 + 2] = rgb[2];
        dst[i * 4 + 3] = 0xFF;
    }
}

/* ---------------- Public API ---------------- */

int M11_Render_Init(int windowWidth, int windowHeight, int scaleMode) {
    if (g_state.initialised) {
        return M11_RENDER_ERR_ALREADY_INIT;
    }
    if (windowWidth <= 0 || windowHeight <= 0) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    if (!m11_validate_scale(scaleMode)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }

#if SDL_VERSION_ATLEAST(3, 0, 0)
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        return M11_RENDER_ERR_SDL_INIT;
    }
#else
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        return M11_RENDER_ERR_SDL_INIT;
    }
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
    g_state.window = SDL_CreateWindow(
        "Firestaff",
        windowWidth,
        windowHeight,
        SDL_WINDOW_RESIZABLE);
#else
    g_state.window = SDL_CreateWindow(
        "Firestaff",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_RESIZABLE);
#endif

    if (!g_state.window) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return M11_RENDER_ERR_WINDOW;
    }

#if SDL_VERSION_ATLEAST(3, 0, 0)
    /* SDL3 picks the best renderer when name is NULL. The software
       renderer is always available as a fallback. */
    g_state.renderer = SDL_CreateRenderer(g_state.window, NULL);
#else
    g_state.renderer = SDL_CreateRenderer(g_state.window, -1, 0);
#endif
    if (!g_state.renderer) {
        SDL_DestroyWindow(g_state.window);
        g_state.window = NULL;
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return M11_RENDER_ERR_RENDERER;
    }

#if SDL_VERSION_ATLEAST(3, 0, 0)
    g_state.texture = SDL_CreateTexture(
        g_state.renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        M11_FB_WIDTH,
        M11_FB_HEIGHT);
#else
    g_state.texture = SDL_CreateTexture(
        g_state.renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        M11_FB_WIDTH,
        M11_FB_HEIGHT);
#endif
    if (!g_state.texture) {
        SDL_DestroyRenderer(g_state.renderer);
        SDL_DestroyWindow(g_state.window);
        g_state.renderer = NULL;
        g_state.window = NULL;
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return M11_RENDER_ERR_TEXTURE;
    }

    g_state.presentBuffer =
        (unsigned char*)calloc(M11_FB_BYTES, 4);
    if (!g_state.presentBuffer) {
        SDL_DestroyTexture(g_state.texture);
        SDL_DestroyRenderer(g_state.renderer);
        SDL_DestroyWindow(g_state.window);
        g_state.texture = NULL;
        g_state.renderer = NULL;
        g_state.window = NULL;
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return M11_RENDER_ERR_TEXTURE;
    }

    memset(g_state.framebuffer, 0, M11_FB_BYTES);
    g_state.windowW = windowWidth;
    g_state.windowH = windowHeight;
    g_state.scaleMode = scaleMode;
    g_state.paletteLevel = 0;
    g_state.windowMode = M11_WINDOW_MODE_WINDOWED;
    g_state.quitRequested = 0;
    g_state.initialised = 1;
    return M11_RENDER_OK;
}

void M11_Render_Shutdown(void) {
    if (!g_state.initialised) {
        return;
    }
    m11_free_present_buffer();
    if (g_state.texture) {
        SDL_DestroyTexture(g_state.texture);
        g_state.texture = NULL;
    }
    if (g_state.renderer) {
        SDL_DestroyRenderer(g_state.renderer);
        g_state.renderer = NULL;
    }
    if (g_state.window) {
        SDL_DestroyWindow(g_state.window);
        g_state.window = NULL;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    g_state.initialised = 0;
    g_state.quitRequested = 0;
    g_state.windowW = 0;
    g_state.windowH = 0;
    g_state.paletteLevel = 0;
    g_state.windowMode = M11_WINDOW_MODE_WINDOWED;
}

int M11_Render_IsInitialized(void) {
    return g_state.initialised ? 1 : 0;
}

unsigned char* M11_Render_GetFramebuffer(void) {
    if (!g_state.initialised) {
        return NULL;
    }
    return g_state.framebuffer;
}

size_t M11_Render_GetFramebufferSize(void) {
    return (size_t)M11_FB_BYTES;
}

int M11_Render_SetPaletteLevel(int level) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (level < 0 || level >= M11_PALETTE_LEVELS) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    g_state.paletteLevel = level;
    return M11_RENDER_OK;
}

int M11_Render_GetPaletteLevel(void) {
    return g_state.paletteLevel;
}

long M11_Render_ClearFramebuffer(unsigned char colorIndex) {
    if (!g_state.initialised) {
        return -1;
    }
    unsigned char v = colorIndex & 0x0F;
    memset(g_state.framebuffer, v, M11_FB_BYTES);
    return (long)M11_FB_BYTES;
}

int M11_Render_Present(void) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }

    int destX = 0;
    int destY = 0;
    int destW = 0;
    int destH = 0;
#if SDL_VERSION_ATLEAST(3, 0, 0)
    SDL_FRect destRect;
#else
    SDL_Rect destRect;
#endif

    m11_framebuffer_to_rgba();
    m11_compute_present_rect(&destX, &destY, &destW, &destH);

#if SDL_VERSION_ATLEAST(3, 0, 0)
    if (!SDL_UpdateTexture(
            g_state.texture,
            NULL,
            g_state.presentBuffer,
            M11_FB_WIDTH * 4)) {
        return M11_RENDER_ERR_TEXTURE;
    }
    if (!SDL_RenderClear(g_state.renderer)) {
        return M11_RENDER_ERR_RENDERER;
    }
    destRect.x = (float)destX;
    destRect.y = (float)destY;
    destRect.w = (float)destW;
    destRect.h = (float)destH;
    if (!SDL_RenderTexture(g_state.renderer, g_state.texture, NULL, &destRect)) {
        return M11_RENDER_ERR_RENDERER;
    }
    if (!SDL_RenderPresent(g_state.renderer)) {
        return M11_RENDER_ERR_RENDERER;
    }
#else
    if (SDL_UpdateTexture(
            g_state.texture,
            NULL,
            g_state.presentBuffer,
            M11_FB_WIDTH * 4) != 0) {
        return M11_RENDER_ERR_TEXTURE;
    }
    if (SDL_RenderClear(g_state.renderer) != 0) {
        return M11_RENDER_ERR_RENDERER;
    }
    destRect.x = destX;
    destRect.y = destY;
    destRect.w = destW;
    destRect.h = destH;
    if (SDL_RenderCopy(g_state.renderer, g_state.texture, NULL, &destRect) != 0) {
        return M11_RENDER_ERR_RENDERER;
    }
    SDL_RenderPresent(g_state.renderer);
#endif
    return M11_RENDER_OK;
}

int M11_Render_PumpEvents(void) {
    if (!g_state.initialised) {
        return 0;
    }
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
#if SDL_VERSION_ATLEAST(3, 0, 0)
        if (ev.type == SDL_EVENT_QUIT) {
            g_state.quitRequested = 1;
        } else if (ev.type == SDL_EVENT_KEY_DOWN) {
            if (ev.key.key == SDLK_ESCAPE) {
                g_state.quitRequested = 1;
            }
        } else if (ev.type == SDL_EVENT_WINDOW_RESIZED ||
                   ev.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
            M11_Render_HandleResize(ev.window.data1, ev.window.data2);
        }
#else
        if (ev.type == SDL_QUIT) {
            g_state.quitRequested = 1;
        } else if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_ESCAPE) {
                g_state.quitRequested = 1;
            }
        } else if (ev.type == SDL_WINDOWEVENT &&
                   ev.window.event == SDL_WINDOWEVENT_RESIZED) {
            M11_Render_HandleResize(ev.window.data1, ev.window.data2);
        }
#endif
    }
    return g_state.quitRequested;
}

int M11_Render_HandleResize(int newWidth, int newHeight) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (newWidth <= 0 || newHeight <= 0) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    g_state.windowW = newWidth;
    g_state.windowH = newHeight;
    return M11_RENDER_OK;
}

int M11_Render_GetWindowWidth(void) {
    return g_state.windowW;
}

int M11_Render_GetWindowHeight(void) {
    return g_state.windowH;
}

int M11_Render_SetScaleMode(int scaleMode) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!m11_validate_scale(scaleMode)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    g_state.scaleMode = scaleMode;
    return M11_RENDER_OK;
}

int M11_Render_GetScaleMode(void) {
    return g_state.scaleMode;
}

int M11_Render_CycleScaleMode(void) {
    int next;
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    next = g_state.scaleMode + 1;
    if (next > M11_SCALE_STRETCH) {
        next = M11_SCALE_1X;
    }
    g_state.scaleMode = next;
    return g_state.scaleMode;
}

int M11_Render_ToggleFullscreen(void) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    return m11_apply_window_mode(g_state.windowMode == M11_WINDOW_MODE_FULLSCREEN
                                     ? M11_WINDOW_MODE_WINDOWED
                                     : M11_WINDOW_MODE_FULLSCREEN);
}

int M11_Render_GetPresentRect(int* outX, int* outY, int* outW, int* outH) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    m11_compute_present_rect(outX, outY, outW, outH);
    return M11_RENDER_OK;
}

int M11_Render_MapWindowToFramebuffer(int windowX,
                                      int windowY,
                                      int* outFbX,
                                      int* outFbY) {
    int rectX;
    int rectY;
    int rectW;
    int rectH;
    int localX;
    int localY;

    if (!g_state.initialised || !outFbX || !outFbY) {
        return 0;
    }

    m11_compute_present_rect(&rectX, &rectY, &rectW, &rectH);
    if (rectW <= 0 || rectH <= 0) {
        return 0;
    }
    if (windowX < rectX || windowY < rectY || windowX >= rectX + rectW || windowY >= rectY + rectH) {
        return 0;
    }

    localX = windowX - rectX;
    localY = windowY - rectY;
    *outFbX = (localX * M11_FB_WIDTH) / rectW;
    *outFbY = (localY * M11_FB_HEIGHT) / rectH;
    if (*outFbX < 0) *outFbX = 0;
    if (*outFbY < 0) *outFbY = 0;
    if (*outFbX >= M11_FB_WIDTH) *outFbX = M11_FB_WIDTH - 1;
    if (*outFbY >= M11_FB_HEIGHT) *outFbY = M11_FB_HEIGHT - 1;
    return 1;
}

int M11_Render_SetWindowMode(int windowModeIndex) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    return m11_apply_window_mode(windowModeIndex);
}

int M11_Render_GetWindowMode(void) {
    return g_state.windowMode;
}

int M11_Render_GetSdlMajorVersion(void) {
    return M11_SDL_MAJOR;
}
