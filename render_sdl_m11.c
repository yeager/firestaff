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
    int integerScaling;
    int scaleFilter;
    int vsync;
    int quitRequested;
    int contentW;
    int contentH;
    int textureW;
    int textureH;

    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
    SDL_Texture*  retiredTextures[8];
    int retiredTextureCount;

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

static void m11_retire_texture(SDL_Texture* texture) {
    if (!texture) {
        return;
    }
    if (g_state.retiredTextureCount < (int)(sizeof(g_state.retiredTextures) /
                                            sizeof(g_state.retiredTextures[0]))) {
        g_state.retiredTextures[g_state.retiredTextureCount++] = texture;
        return;
    }
    SDL_DestroyTexture(texture);
}

static int m11_min_texture_w(void) { return 480; }
static int m11_min_texture_h(void) { return 270; }

static int m11_validate_scale(int mode) {
    return (mode >= M11_SCALE_1X && mode <= M11_SCALE_STRETCH) ? 1 : 0;
}

static int m11_validate_window_mode(int mode) {
    return mode == M11_WINDOW_MODE_WINDOWED || mode == M11_WINDOW_MODE_FULLSCREEN;
}

static int m11_validate_binary_setting(int mode) {
    return mode == 0 || mode == 1;
}

static void m11_apply_texture_scale_filter(SDL_Texture* texture) {
    if (!texture) {
        return;
    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
    SDL_SetTextureScaleMode(texture,
                            g_state.scaleFilter == M11_SCALE_FILTER_LINEAR
                                ? SDL_SCALEMODE_LINEAR
                                : SDL_SCALEMODE_NEAREST);
#else
    SDL_SetTextureScaleMode(texture,
                            g_state.scaleFilter == M11_SCALE_FILTER_LINEAR
                                ? SDL_ScaleModeLinear
                                : SDL_ScaleModeNearest);
#endif
}

static int m11_recreate_texture_if_needed(int logicalWidth,
                                          int logicalHeight) {
    unsigned char* resizedBuffer;
    SDL_Texture* texture;
    if (logicalWidth <= 0 || logicalHeight <= 0) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    if (g_state.texture &&
        g_state.textureW >= logicalWidth &&
        g_state.textureH >= logicalHeight) {
        return M11_RENDER_OK;
    }
    if (logicalWidth < m11_min_texture_w()) {
        logicalWidth = m11_min_texture_w();
    }
    if (logicalHeight < m11_min_texture_h()) {
        logicalHeight = m11_min_texture_h();
    }
    resizedBuffer = (unsigned char*)realloc(g_state.presentBuffer,
                                            (size_t)logicalWidth * (size_t)logicalHeight * 4U);
    if (!resizedBuffer) {
        return M11_RENDER_ERR_TEXTURE;
    }
    g_state.presentBuffer = resizedBuffer;
    if (g_state.texture) {
        m11_retire_texture(g_state.texture);
        g_state.texture = NULL;
    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
    texture = SDL_CreateTexture(
        g_state.renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        logicalWidth,
        logicalHeight);
#else
    texture = SDL_CreateTexture(
        g_state.renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        logicalWidth,
        logicalHeight);
#endif
    if (!texture) {
        return M11_RENDER_ERR_TEXTURE;
    }
    m11_apply_texture_scale_filter(texture);
    g_state.texture = texture;
    g_state.textureW = logicalWidth;
    g_state.textureH = logicalHeight;
    return M11_RENDER_OK;
}

static void m11_compute_present_rect(int* outX, int* outY, int* outW, int* outH) {
    int x = 0;
    int y = 0;
    int w = g_state.windowW;
    int h = g_state.windowH;
    int contentW = g_state.contentW > 0 ? g_state.contentW : M11_FB_WIDTH;
    int contentH = g_state.contentH > 0 ? g_state.contentH : M11_FB_HEIGHT;

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
                w = contentW * factor;
                h = contentH * factor;
                break;
            }
            case M11_SCALE_FIT: {
                int fitW = g_state.windowW;
                int fitH = (fitW * contentH) / contentW;
                if (fitH > g_state.windowH) {
                    fitH = g_state.windowH;
                    fitW = (fitH * contentW) / contentH;
                }
                if (g_state.integerScaling) {
                    int factorW = g_state.windowW / contentW;
                    int factorH = g_state.windowH / contentH;
                    int factor = factorW < factorH ? factorW : factorH;
                    if (factor < 1) {
                        factor = 1;
                    }
                    fitW = contentW * factor;
                    fitH = contentH * factor;
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

/* Build the 32-bit RGBA presentation pixels from the framebuffer.
 *
 * Each source byte encodes both a 4-bit VGA palette colour index
 * (bits 0-3) and a per-pixel palette brightness level (bits 4-7).
 * This matches the original game's approach of using VGA DAC
 * register switching for depth-based dimming: the same colour
 * index produces different RGB values at different brightness
 * levels.
 *
 * When no per-pixel level is encoded (upper bits zero), the global
 * paletteLevel acts as a fallback, preserving backward compatibility
 * with code that writes bare 4-bit indices.
 *
 * Pixel format: SDL_PIXELFORMAT_RGBA32 — R, G, B, A in memory order. */
static void m11_framebuffer_to_rgba(const unsigned char* src,
                                    int logicalWidth,
                                    int logicalHeight) {
    unsigned char* dst = g_state.presentBuffer;
    const int globalLevel = g_state.paletteLevel;
    int pixelCount;
    if (!dst) {
        return;
    }
    pixelCount = logicalWidth * logicalHeight;
    for (int i = 0; i < pixelCount; ++i) {
        unsigned char raw = src[i];
        unsigned char idx = raw & M11_FB_INDEX_MASK;
        int perPixelLevel = (raw & M11_FB_LEVEL_MASK) >> M11_FB_LEVEL_SHIFT;
        int level = perPixelLevel > 0 ? perPixelLevel : globalLevel;
        const unsigned char* rgb;
        if (level >= M11_PALETTE_LEVELS) {
            level = M11_PALETTE_LEVELS - 1;
        }
        rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
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
        m11_min_texture_w(),
        m11_min_texture_h());
#else
    g_state.texture = SDL_CreateTexture(
        g_state.renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        m11_min_texture_w(),
        m11_min_texture_h());
#endif
    if (!g_state.texture) {
        SDL_DestroyRenderer(g_state.renderer);
        SDL_DestroyWindow(g_state.window);
        g_state.renderer = NULL;
        g_state.window = NULL;
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return M11_RENDER_ERR_TEXTURE;
    }
    m11_apply_texture_scale_filter(g_state.texture);

    g_state.presentBuffer =
        (unsigned char*)calloc((size_t)m11_min_texture_w() * (size_t)m11_min_texture_h(), 4);
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
    g_state.integerScaling = 1;
    g_state.scaleFilter = M11_SCALE_FILTER_NEAREST;
    g_state.vsync = M11_VSYNC_ON;
    g_state.quitRequested = 0;
    g_state.contentW = M11_FB_WIDTH;
    g_state.contentH = M11_FB_HEIGHT;
    g_state.textureW = m11_min_texture_w();
    g_state.textureH = m11_min_texture_h();
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
    for (int i = 0; i < g_state.retiredTextureCount; ++i) {
        if (g_state.retiredTextures[i]) {
            SDL_DestroyTexture(g_state.retiredTextures[i]);
            g_state.retiredTextures[i] = NULL;
        }
    }
    g_state.retiredTextureCount = 0;
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
    g_state.integerScaling = 1;
    g_state.scaleFilter = M11_SCALE_FILTER_NEAREST;
    g_state.vsync = M11_VSYNC_ON;
    g_state.contentW = 0;
    g_state.contentH = 0;
    g_state.textureW = 0;
    g_state.textureH = 0;
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
    return M11_Render_PresentIndexed(g_state.framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
}

int M11_Render_PresentIndexed(const unsigned char* framebuffer,
                              int logicalWidth,
                              int logicalHeight) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!framebuffer || logicalWidth <= 0 || logicalHeight <= 0) {
        return M11_RENDER_ERR_INVALID_ARG;
    }

    int destX = 0;
    int destY = 0;
    int destW = 0;
    int destH = 0;
#if SDL_VERSION_ATLEAST(3, 0, 0)
    SDL_FRect sourceRect;
    SDL_FRect destRect;
#else
    SDL_Rect sourceRect;
    SDL_Rect destRect;
#endif

    if (m11_recreate_texture_if_needed(logicalWidth, logicalHeight) != M11_RENDER_OK) {
        return M11_RENDER_ERR_TEXTURE;
    }
    g_state.contentW = logicalWidth;
    g_state.contentH = logicalHeight;
    m11_framebuffer_to_rgba(framebuffer, logicalWidth, logicalHeight);
    m11_compute_present_rect(&destX, &destY, &destW, &destH);

#if SDL_VERSION_ATLEAST(3, 0, 0)
    sourceRect.x = 0.0f;
    sourceRect.y = 0.0f;
    sourceRect.w = (float)logicalWidth;
    sourceRect.h = (float)logicalHeight;
    {
        SDL_Rect updateRect;
        updateRect.x = 0;
        updateRect.y = 0;
        updateRect.w = logicalWidth;
        updateRect.h = logicalHeight;
        if (!SDL_UpdateTexture(
                g_state.texture,
                &updateRect,
                g_state.presentBuffer,
                logicalWidth * 4)) {
            return M11_RENDER_ERR_TEXTURE;
        }
    }
    if (!SDL_RenderClear(g_state.renderer)) {
        return M11_RENDER_ERR_RENDERER;
    }
    destRect.x = (float)destX;
    destRect.y = (float)destY;
    destRect.w = (float)destW;
    destRect.h = (float)destH;
    if (!SDL_RenderTexture(g_state.renderer, g_state.texture, &sourceRect, &destRect)) {
        return M11_RENDER_ERR_RENDERER;
    }
    if (!SDL_RenderPresent(g_state.renderer)) {
        return M11_RENDER_ERR_RENDERER;
    }
#else
    sourceRect.x = 0;
    sourceRect.y = 0;
    sourceRect.w = logicalWidth;
    sourceRect.h = logicalHeight;
    if (SDL_UpdateTexture(
            g_state.texture,
            &sourceRect,
            g_state.presentBuffer,
            logicalWidth * 4) != 0) {
        return M11_RENDER_ERR_TEXTURE;
    }
    if (SDL_RenderClear(g_state.renderer) != 0) {
        return M11_RENDER_ERR_RENDERER;
    }
    destRect.x = destX;
    destRect.y = destY;
    destRect.w = destW;
    destRect.h = destH;
    if (SDL_RenderCopy(g_state.renderer, g_state.texture, &sourceRect, &destRect) != 0) {
        return M11_RENDER_ERR_RENDERER;
    }
    SDL_RenderPresent(g_state.renderer);
#endif
    return M11_RENDER_OK;
}

int M11_Render_PresentRGBA(const unsigned char* rgba,
                           int logicalWidth,
                           int logicalHeight) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!rgba || logicalWidth <= 0 || logicalHeight <= 0) {
        return M11_RENDER_ERR_INVALID_ARG;
    }

    int destX = 0;
    int destY = 0;
    int destW = 0;
    int destH = 0;
#if SDL_VERSION_ATLEAST(3, 0, 0)
    SDL_FRect sourceRect;
    SDL_FRect destRect;
#else
    SDL_Rect sourceRect;
    SDL_Rect destRect;
#endif

    if (m11_recreate_texture_if_needed(logicalWidth, logicalHeight) != M11_RENDER_OK) {
        return M11_RENDER_ERR_TEXTURE;
    }
    g_state.contentW = logicalWidth;
    g_state.contentH = logicalHeight;
    if (g_state.presentBuffer) {
        memcpy(g_state.presentBuffer, rgba,
               (size_t)logicalWidth * (size_t)logicalHeight * 4U);
    }
    m11_compute_present_rect(&destX, &destY, &destW, &destH);

#if SDL_VERSION_ATLEAST(3, 0, 0)
    sourceRect.x = 0.0f;
    sourceRect.y = 0.0f;
    sourceRect.w = (float)logicalWidth;
    sourceRect.h = (float)logicalHeight;
    {
        SDL_Rect updateRect;
        updateRect.x = 0;
        updateRect.y = 0;
        updateRect.w = logicalWidth;
        updateRect.h = logicalHeight;
        if (!SDL_UpdateTexture(
                g_state.texture,
                &updateRect,
                g_state.presentBuffer,
                logicalWidth * 4)) {
            return M11_RENDER_ERR_TEXTURE;
        }
    }
    if (!SDL_RenderClear(g_state.renderer)) {
        return M11_RENDER_ERR_RENDERER;
    }
    destRect.x = (float)destX;
    destRect.y = (float)destY;
    destRect.w = (float)destW;
    destRect.h = (float)destH;
    if (!SDL_RenderTexture(g_state.renderer, g_state.texture, &sourceRect, &destRect)) {
        return M11_RENDER_ERR_RENDERER;
    }
    if (!SDL_RenderPresent(g_state.renderer)) {
        return M11_RENDER_ERR_RENDERER;
    }
#else
    sourceRect.x = 0;
    sourceRect.y = 0;
    sourceRect.w = logicalWidth;
    sourceRect.h = logicalHeight;
    if (SDL_UpdateTexture(
            g_state.texture,
            &sourceRect,
            g_state.presentBuffer,
            logicalWidth * 4) != 0) {
        return M11_RENDER_ERR_TEXTURE;
    }
    if (SDL_RenderClear(g_state.renderer) != 0) {
        return M11_RENDER_ERR_RENDERER;
    }
    destRect.x = destX;
    destRect.y = destY;
    destRect.w = destW;
    destRect.h = destH;
    if (SDL_RenderCopy(g_state.renderer, g_state.texture, &sourceRect, &destRect) != 0) {
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
    *outFbX = (localX * g_state.contentW) / rectW;
    *outFbY = (localY * g_state.contentH) / rectH;
    if (*outFbX < 0) *outFbX = 0;
    if (*outFbY < 0) *outFbY = 0;
    if (*outFbX >= g_state.contentW) *outFbX = g_state.contentW - 1;
    if (*outFbY >= g_state.contentH) *outFbY = g_state.contentH - 1;
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

int M11_Render_SetIntegerScaling(int enabled) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!m11_validate_binary_setting(enabled)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    g_state.integerScaling = enabled;
    return M11_RENDER_OK;
}

int M11_Render_GetIntegerScaling(void) {
    return g_state.integerScaling;
}

int M11_Render_SetScaleFilter(int filterIndex) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!m11_validate_binary_setting(filterIndex)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    g_state.scaleFilter = filterIndex;
    m11_apply_texture_scale_filter(g_state.texture);
    return M11_RENDER_OK;
}

int M11_Render_GetScaleFilter(void) {
    return g_state.scaleFilter;
}

int M11_Render_SetVSync(int vsyncIndex) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!m11_validate_binary_setting(vsyncIndex)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
    (void)SDL_SetRenderVSync(g_state.renderer, vsyncIndex ? 1 : 0);
#elif SDL_VERSION_ATLEAST(2, 0, 18)
    (void)SDL_RenderSetVSync(g_state.renderer, vsyncIndex ? 1 : 0);
#endif
    g_state.vsync = vsyncIndex;
    return M11_RENDER_OK;
}

int M11_Render_GetVSync(void) {
    return g_state.vsync;
}

int M11_Render_GetSdlMajorVersion(void) {
    return M11_SDL_MAJOR;
}
