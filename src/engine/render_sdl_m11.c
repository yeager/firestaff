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
#include <stdio.h>

#include <SDL3/SDL.h>

#if !SDL_VERSION_ATLEAST(3, 0, 0)
/* Fall back to SDL2 header if someone pointed us at an SDL2 install via
   a pkg-config that emitted -I.../SDL2. */
#include <SDL.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "vga_palette_pc34_compat.h"
#include "dm1v2/dm1_v2_filters.h"

#if SDL_VERSION_ATLEAST(3, 0, 0)
#define M11_SDL_MAJOR 3
#else
#define M11_SDL_MAJOR 2
#endif

/* ---------------- Internal state ---------------- */

typedef struct {
    int initialised;
    int windowW;       /* logical window size (for mouse coordinate mapping) */
    int windowH;
    int renderW;       /* pixel-level render output size (for destRect computation) */
    int renderH;
    int scaleMode;
    int displayAspectMode;
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

    /* DM1 V2.0 filter chain (V2-only; V1 launch path leaves these zero). */
    int v2_crt_enabled;
    int v2_crt_strength;
    int v2_palette_enabled;
    int v2_palette_gamma100;
    int v2_palette_brightness;
    int v2_palette_contrast;
    int v2_dither_enabled;
    int v2_sharpen_enabled;
    int v2_sharpen_strength;
    int v2_palette_lut_built;
    unsigned char v2_palette_corrected[M11_PALETTE_LEVELS][16][3];

    /* V2.0 visual extras (Firestaff-only, no ReDMCSB analogue). */
    int v2_phosphor_enabled;
    int v2_phosphor_decay;          /* 0..100 percent of previous frame */

    unsigned char* previousFrameBuffer;  /* prev RGBA frame at logical size */
    int previousFrameW;
    int previousFrameH;
} M11_RenderState;

static M11_RenderState g_state = {0};

/* ---------------- Helpers ---------------- */

static void m11_free_present_buffer(void) {
    if (g_state.presentBuffer) {
        free(g_state.presentBuffer);
        g_state.presentBuffer = NULL;
    }
    if (g_state.previousFrameBuffer) {
        free(g_state.previousFrameBuffer);
        g_state.previousFrameBuffer = NULL;
    }
    g_state.previousFrameW = 0;
    g_state.previousFrameH = 0;
}

/* Ensure previousFrameBuffer matches the requested size.  On size
 * change, the buffer is reallocated and zeroed so the first blend pass
 * does not pull in stale pixels.  Returns 0 on success, -1 on alloc
 * failure (the caller should treat that as "skip the effect"). */
static int m11_ensure_prev_frame_buffer(int w, int h) {
    size_t bytes;
    unsigned char* nb;
    if (w <= 0 || h <= 0) {
        return -1;
    }
    if (g_state.previousFrameBuffer
            && g_state.previousFrameW == w
            && g_state.previousFrameH == h) {
        return 0;
    }
    bytes = (size_t)w * (size_t)h * 4U;
    nb = (unsigned char*)realloc(g_state.previousFrameBuffer, bytes);
    if (!nb) {
        return -1;
    }
    g_state.previousFrameBuffer = nb;
    memset(g_state.previousFrameBuffer, 0, bytes);
    g_state.previousFrameW = w;
    g_state.previousFrameH = h;
    return 0;
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
    return mode == M11_WINDOW_MODE_WINDOWED ||
           mode == M11_WINDOW_MODE_MAXIMIZED ||
           mode == M11_WINDOW_MODE_FULLSCREEN;
}

static int m11_validate_display_aspect(int mode) {
    return mode == M11_DISPLAY_ASPECT_4_3 || mode == M11_DISPLAY_ASPECT_16_9 || mode == M11_DISPLAY_ASPECT_CONTENT;
}

static int m11_window_mode_from_sdl_window(void) {
    Uint32 flags;
    if (!g_state.window) {
        return g_state.windowMode;
    }
    flags = SDL_GetWindowFlags(g_state.window);
#if SDL_VERSION_ATLEAST(3, 0, 0)
    if ((flags & SDL_WINDOW_FULLSCREEN) != 0U) {
        return M11_WINDOW_MODE_FULLSCREEN;
    }
#else
    if ((flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0U) {
        return M11_WINDOW_MODE_FULLSCREEN;
    }
#endif
    if ((flags & SDL_WINDOW_MAXIMIZED) != 0U) {
        return M11_WINDOW_MODE_MAXIMIZED;
    }
    return M11_WINDOW_MODE_WINDOWED;
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

int M11_Render_ComputePresentationRect(int windowW,
                                       int windowH,
                                       int contentW,
                                       int contentH,
                                       int scaleMode,
                                       int integerScaling,
                                       int displayAspectMode,
                                       int* outX,
                                       int* outY,
                                       int* outW,
                                       int* outH) {
    int x = 0;
    int y = 0;
    int w = windowW;
    int h = windowH;
    int ratioW, ratioH;
    if (displayAspectMode == M11_DISPLAY_ASPECT_CONTENT) {
        ratioW = contentW;
        ratioH = contentH;
    } else if (displayAspectMode == M11_DISPLAY_ASPECT_4_3) {
        ratioW = 4;
        ratioH = 3;
    } else {
        ratioW = 16;
        ratioH = 9;
    }

    if (contentW <= 0 || contentH <= 0) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    if (!m11_validate_scale(scaleMode) || !m11_validate_display_aspect(displayAspectMode)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }

    if (windowW <= 0 || windowH <= 0) {
        w = M11_FB_WIDTH;
        h = M11_FB_HEIGHT;
    } else {
        switch (scaleMode) {
            case M11_SCALE_1X:
            case M11_SCALE_2X:
            case M11_SCALE_3X:
            case M11_SCALE_4X: {
                int factor = scaleMode + 1;
                w = contentW * factor;
                h = contentH * factor;
                break;
            }
            case M11_SCALE_FIT: {
                int fitW = windowW;
                int fitH = (fitW * ratioH) / ratioW;
                if (fitH > windowH) {
                    fitH = windowH;
                    fitW = (fitH * ratioW) / ratioH;
                }
                if (integerScaling && (contentW * ratioH) == (contentH * ratioW)) {
                    int factorW = windowW / contentW;
                    int factorH = windowH / contentH;
                    int factor = factorW < factorH ? factorW : factorH;
                    if (factor >= 1) {
                        fitW = contentW * factor;
                        fitH = contentH * factor;
                    }
                }
                if (fitW < 1) fitW = 1;
                if (fitH < 1) fitH = 1;
                w = fitW;
                h = fitH;
                break;
            }
            case M11_SCALE_STRETCH:
            default:
                w = windowW;
                h = (w * ratioH) / ratioW;
                if (h > windowH) {
                    h = windowH;
                    w = (h * ratioW) / ratioH;
                }
                break;
        }
    }

    x = (windowW - w) / 2;
    y = (windowH - h) / 2;
    if (outX) *outX = x;
    if (outY) *outY = y;
    if (outW) *outW = w;
    if (outH) *outH = h;
    return M11_RENDER_OK;
}

static void m11_compute_present_rect(int* outX, int* outY, int* outW, int* outH) {
    int contentW = g_state.contentW > 0 ? g_state.contentW : M11_FB_WIDTH;
    int contentH = g_state.contentH > 0 ? g_state.contentH : M11_FB_HEIGHT;
    /* Use pixel-level render output size for destRect computation.
     * SDL3 RenderTexture operates in pixel coordinates, not logical
     * window points.  On macOS Retina (2× scale), using logical size
     * here causes the game content to render at half size. */
    int rw = g_state.renderW > 0 ? g_state.renderW : g_state.windowW;
    int rh = g_state.renderH > 0 ? g_state.renderH : g_state.windowH;
    (void)M11_Render_ComputePresentationRect(rw,
                                             rh,
                                             contentW,
                                             contentH,
                                             g_state.scaleMode,
                                             g_state.integerScaling,
                                             g_state.displayAspectMode,
                                             outX,
                                             outY,
                                             outW,
                                             outH);
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
    if (windowMode == M11_WINDOW_MODE_MAXIMIZED) {
        SDL_MaximizeWindow(g_state.window);
    } else if (windowMode == M11_WINDOW_MODE_WINDOWED) {
        SDL_RestoreWindow(g_state.window);
    }
#else
    if (SDL_SetWindowFullscreen(g_state.window,
                                windowMode == M11_WINDOW_MODE_FULLSCREEN
                                    ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                    : 0) != 0) {
        return M11_RENDER_ERR_WINDOW;
    }
    if (windowMode == M11_WINDOW_MODE_MAXIMIZED) {
        SDL_MaximizeWindow(g_state.window);
    } else if (windowMode == M11_WINDOW_MODE_WINDOWED) {
        SDL_RestoreWindow(g_state.window);
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
    const int useV2Palette = (g_state.v2_palette_enabled && g_state.v2_palette_lut_built);
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
        rgb = useV2Palette
            ? g_state.v2_palette_corrected[level][idx]
            : G9010_auc_VgaPaletteAll_Compat[level][idx];
        dst[i * 4 + 0] = rgb[0];
        dst[i * 4 + 1] = rgb[1];
        dst[i * 4 + 2] = rgb[2];
        dst[i * 4 + 3] = 0xFF;
    }
}

/* DM1 V2.0 post-process hook. Called from M11_Render_PresentIndexed
 * after framebuffer-to-RGBA expansion, before SDL_UpdateTexture.
 * V1 launch path leaves all v2_* flags zero so this short-circuits. */
static void m11_apply_v2_filters_indexed_pre(unsigned char* fb,
                                             int w,
                                             int h) {
    if (g_state.v2_dither_enabled) {
        (void)dm1_v2_filter_dither_cleanup_indexed(fb, w, h);
    }
}

/* Phosphor persistence: bright pixels from the previous frame bleed
 * through using max(current, previous * decay).  Operates on the RGBA
 * present buffer at logical resolution. */
static void m11_apply_phosphor_persistence(int w, int h) {
    unsigned char* cur;
    unsigned char* prev;
    int pixelCount;
    int i;
    int decayNum;
    if (!g_state.v2_phosphor_enabled || g_state.v2_phosphor_decay <= 0) {
        return;
    }
    cur = g_state.presentBuffer;
    if (!cur) {
        return;
    }
    if (m11_ensure_prev_frame_buffer(w, h) != 0) {
        return;
    }
    prev = g_state.previousFrameBuffer;
    pixelCount = w * h;
    decayNum = g_state.v2_phosphor_decay;
    if (decayNum > 100) decayNum = 100;
    for (i = 0; i < pixelCount; ++i) {
        int o = i * 4;
        int pr = (prev[o + 0] * decayNum) / 100;
        int pg = (prev[o + 1] * decayNum) / 100;
        int pb = (prev[o + 2] * decayNum) / 100;
        int cr = cur[o + 0];
        int cg = cur[o + 1];
        int cb = cur[o + 2];
        cur[o + 0] = (unsigned char)(pr > cr ? pr : cr);
        cur[o + 1] = (unsigned char)(pg > cg ? pg : cg);
        cur[o + 2] = (unsigned char)(pb > cb ? pb : cb);
        /* alpha unchanged */
    }
}

/* Snapshot the current present buffer into previousFrameBuffer so the
 * next frame can read it.  Called once per present, after all RGBA
 * filters but before the SDL upload. */
static void m11_snapshot_prev_frame(int w, int h) {
    if (!g_state.v2_phosphor_enabled) {
        return;
    }
    if (!g_state.presentBuffer) {
        return;
    }
    if (m11_ensure_prev_frame_buffer(w, h) != 0) {
        return;
    }
    memcpy(g_state.previousFrameBuffer, g_state.presentBuffer,
           (size_t)w * (size_t)h * 4U);
}

static void m11_apply_v2_filters_rgba_post(int w, int h) {
    unsigned char* rgba = g_state.presentBuffer;
    if (!rgba) {
        return;
    }
    if (g_state.v2_sharpen_enabled && g_state.v2_sharpen_strength > 0) {
        (void)dm1_v2_filter_sharpen_rgba(rgba, w, h, g_state.v2_sharpen_strength);
    }
    if (g_state.v2_crt_enabled && g_state.v2_crt_strength > 0) {
        (void)dm1_v2_filter_crt_scanlines_rgba(rgba, w, h, g_state.v2_crt_strength);
    }
    m11_apply_phosphor_persistence(w, h);
    m11_snapshot_prev_frame(w, h);
}

static void m11_framebuffer_to_rgba_special(const unsigned char* src,
                                            int logicalWidth,
                                            int logicalHeight,
                                            int specialPalette) {
    unsigned char* dst = g_state.presentBuffer;
    int pixelCount;
    if (!dst) {
        return;
    }
    pixelCount = logicalWidth * logicalHeight;
    for (int i = 0; i < pixelCount; ++i) {
        unsigned char idx = src[i] & M11_FB_INDEX_MASK;
        const unsigned char* rgb = F9011_VGA_GetSpecialColorRgb_Compat(idx, (unsigned int)specialPalette);
        if (!rgb) {
            rgb = G9010_auc_VgaPaletteAll_Compat[g_state.paletteLevel][idx];
        }
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
        fprintf(stderr, "SDL3 init failed: %s\n", SDL_GetError());
        return M11_RENDER_ERR_SDL_INIT;
    }
#else
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL2 init failed: %s\n", SDL_GetError());
        return M11_RENDER_ERR_SDL_INIT;
    }
#endif

#if SDL_VERSION_ATLEAST(3, 0, 0)
    g_state.window = SDL_CreateWindow(
        "Firestaff",
        windowWidth,
        windowHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_HIGH_PIXEL_DENSITY);
#else
    g_state.window = SDL_CreateWindow(
        "Firestaff",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
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
    /* Store LOGICAL window size for mouse coordinate mapping.
     * SDL3: mouse events use logical (window) coordinates.
     * SDL2: use SDL_GL_GetDrawableSize for pixel-accurate rendering.
     *
     * Separately store PIXEL render output size for destRect computation.
     * SDL3 SDL_RenderTexture operates in pixel coordinates, not logical
     * window points.  On macOS Retina (2× scale), the render output is
     * 2× the logical window size.  Using logical size for destRect causes
     * the game content to render at half size — filling only the center
     * quarter of the window.  This was the "small content" bug on Mac. */
    {
        int ww = windowWidth, wh = windowHeight;
#if SDL_VERSION_ATLEAST(3, 0, 0)
        SDL_GetWindowSize(g_state.window, &ww, &wh);
#else
        SDL_GL_GetDrawableSize(g_state.window, &ww, &wh);
        if (ww <= 0 || wh <= 0) { ww = windowWidth; wh = windowHeight; }
#endif
        g_state.windowW = (ww > 0) ? ww : windowWidth;
        g_state.windowH = (wh > 0) ? wh : windowHeight;
    }
#if SDL_VERSION_ATLEAST(3, 0, 0)
    {
        int rw = 0, rh = 0;
        SDL_GetRenderOutputSize(g_state.renderer, &rw, &rh);
        g_state.renderW = (rw > 0) ? rw : g_state.windowW;
        g_state.renderH = (rh > 0) ? rh : g_state.windowH;
    }
#else
    g_state.renderW = g_state.windowW;
    g_state.renderH = g_state.windowH;
#endif
    g_state.scaleMode = scaleMode;
    g_state.displayAspectMode = M11_DISPLAY_ASPECT_CONTENT; /* content-native aspect */
    g_state.paletteLevel = 0;
    g_state.windowMode = M11_WINDOW_MODE_MAXIMIZED;
    g_state.integerScaling = 0; /* non-integer scaling for full-window FIT */
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
    g_state.renderW = 0;
    g_state.renderH = 0;
    g_state.paletteLevel = 0;
    g_state.displayAspectMode = M11_DISPLAY_ASPECT_16_9;
    g_state.windowMode = M11_WINDOW_MODE_WINDOWED;
    g_state.integerScaling = 0; /* non-integer scaling for full-window FIT */
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
    if (g_state.v2_dither_enabled
            && logicalWidth == M11_FB_WIDTH
            && logicalHeight == M11_FB_HEIGHT) {
        static unsigned char v2DitherScratch[M11_FB_BYTES];
        memcpy(v2DitherScratch, framebuffer, (size_t)M11_FB_BYTES);
        m11_apply_v2_filters_indexed_pre(v2DitherScratch, logicalWidth, logicalHeight);
        m11_framebuffer_to_rgba(v2DitherScratch, logicalWidth, logicalHeight);
    } else {
        m11_framebuffer_to_rgba(framebuffer, logicalWidth, logicalHeight);
    }
    m11_apply_v2_filters_rgba_post(logicalWidth, logicalHeight);
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


int M11_Render_PresentIndexedWithSpecialPalette(const unsigned char* framebuffer,
                                                int logicalWidth,
                                                int logicalHeight,
                                                int specialPalette) {
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
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!framebuffer || logicalWidth <= 0 || logicalHeight <= 0) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    if (specialPalette < 0 || specialPalette >= VGA_PALETTE_PC34_SPECIAL_PALETTE_COUNT) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    if (m11_recreate_texture_if_needed(logicalWidth, logicalHeight) != M11_RENDER_OK) {
        return M11_RENDER_ERR_TEXTURE;
    }
    g_state.contentW = logicalWidth;
    g_state.contentH = logicalHeight;
    m11_framebuffer_to_rgba_special(framebuffer, logicalWidth, logicalHeight, specialPalette);
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
        if (!SDL_UpdateTexture(g_state.texture, &updateRect, g_state.presentBuffer, logicalWidth * 4)) {
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
    if (SDL_UpdateTexture(g_state.texture, &sourceRect, g_state.presentBuffer, logicalWidth * 4) != 0) {
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
        } else if (ev.type == SDL_EVENT_WINDOW_RESIZED) {
            /* SDL3: use LOGICAL window size, not pixel size.
             * The renderer and mouse events both use logical coords.
             * ev.window.data1/data2 are the new logical width/height. */
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
                   (ev.window.event == SDL_WINDOWEVENT_RESIZED ||
                    ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)) {
            /* On macOS HiDPI/Retina, SDL_WINDOWEVENT_RESIZED gives
             * logical coordinates (e.g. 960x540) but the renderer
             * needs pixel coordinates (1920x1080).  Use
             * SDL_GL_GetDrawableSize for correct Retina scaling. */
            int pw = ev.window.data1;
            int ph = ev.window.data2;
            SDL_GL_GetDrawableSize(g_state.window, &pw, &ph);
            M11_Render_HandleResize(pw, ph);
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
#if SDL_VERSION_ATLEAST(3, 0, 0)
    /* SDL3 HiDPI fix: query authoritative sizes from SDL when the SDL
     * window has actually changed.  Headless probes and direct callers can
     * exercise this resize path without an SDL window event, so keep the
     * caller-provided logical size when SDL still reports the old size:
     * - windowW/H = logical (for mouse coordinate mapping)
     * - renderW/H = pixels (for SDL_RenderTexture destRect) */
    {
        int ww = 0, wh = 0;
        int sdlWindowMatchedResize = 0;
        SDL_GetWindowSize(g_state.window, &ww, &wh);
        if (ww == newWidth && wh == newHeight) {
            g_state.windowW = ww;
            g_state.windowH = wh;
            sdlWindowMatchedResize = 1;
        } else {
            g_state.windowW = newWidth;
            g_state.windowH = newHeight;
        }
        {
            int rw = 0, rh = 0;
            SDL_GetRenderOutputSize(g_state.renderer, &rw, &rh);
            if (sdlWindowMatchedResize && rw > 0 && rh > 0) {
                g_state.renderW = rw;
                g_state.renderH = rh;
            } else {
                g_state.renderW = g_state.windowW;
                g_state.renderH = g_state.windowH;
            }
        }
    }
#else
    g_state.windowW = newWidth;
    g_state.windowH = newHeight;
    g_state.renderW = newWidth;
    g_state.renderH = newHeight;
#endif
    M11_Render_SyncWindowModeFromWindow();
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

int M11_Render_SetDisplayAspectMode(int aspectMode) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    if (!m11_validate_display_aspect(aspectMode)) {
        return M11_RENDER_ERR_INVALID_ARG;
    }
    g_state.displayAspectMode = aspectMode;
    return M11_RENDER_OK;
}

int M11_Render_GetDisplayAspectMode(void) {
    return g_state.displayAspectMode;
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

    /* Mouse events in SDL3 use logical (window) coordinates, not pixel
     * coordinates.  Compute the presentation rect in logical space so
     * the hit-test and coordinate mapping are correct. */
    {
        int contentW = g_state.contentW > 0 ? g_state.contentW : M11_FB_WIDTH;
        int contentH = g_state.contentH > 0 ? g_state.contentH : M11_FB_HEIGHT;
        (void)M11_Render_ComputePresentationRect(g_state.windowW,
                                                 g_state.windowH,
                                                 contentW,
                                                 contentH,
                                                 g_state.scaleMode,
                                                 g_state.integerScaling,
                                                 g_state.displayAspectMode,
                                                 &rectX,
                                                 &rectY,
                                                 &rectW,
                                                 &rectH);
    }
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

int M11_Render_SyncWindowModeFromWindow(void) {
    if (!g_state.initialised) {
        return M11_RENDER_ERR_NOT_INIT;
    }
    g_state.windowMode = m11_window_mode_from_sdl_window();
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


/* ---------------- DM1 V2.0 filter chain API ---------------- */

int M11_Render_SetV2Filters(int crtEnabled,
                            int crtStrength,
                            int paletteEnabled,
                            int paletteGamma100,
                            int paletteBrightness,
                            int paletteContrast,
                            int ditherEnabled,
                            int sharpenEnabled,
                            int sharpenStrength) {
    int needRebuild = 0;

    if (crtStrength < 0) crtStrength = 0;
    if (crtStrength > 100) crtStrength = 100;
    if (sharpenStrength < 0) sharpenStrength = 0;
    if (sharpenStrength > 100) sharpenStrength = 100;
    if (paletteGamma100 < 80) paletteGamma100 = 80;
    if (paletteGamma100 > 260) paletteGamma100 = 260;
    if (paletteBrightness < -50) paletteBrightness = -50;
    if (paletteBrightness > 50) paletteBrightness = 50;
    if (paletteContrast < -50) paletteContrast = -50;
    if (paletteContrast > 50) paletteContrast = 50;

    if (paletteEnabled
            && (!g_state.v2_palette_lut_built
                || paletteGamma100 != g_state.v2_palette_gamma100
                || paletteBrightness != g_state.v2_palette_brightness
                || paletteContrast != g_state.v2_palette_contrast)) {
        needRebuild = 1;
    }

    g_state.v2_crt_enabled = crtEnabled ? 1 : 0;
    g_state.v2_crt_strength = crtStrength;
    g_state.v2_palette_enabled = paletteEnabled ? 1 : 0;
    g_state.v2_palette_gamma100 = paletteGamma100;
    g_state.v2_palette_brightness = paletteBrightness;
    g_state.v2_palette_contrast = paletteContrast;
    g_state.v2_dither_enabled = ditherEnabled ? 1 : 0;
    g_state.v2_sharpen_enabled = sharpenEnabled ? 1 : 0;
    g_state.v2_sharpen_strength = sharpenStrength;

    if (needRebuild) {
        if (dm1_v2_filter_palette_build_lut(paletteGamma100,
                                            paletteBrightness,
                                            paletteContrast,
                                            g_state.v2_palette_corrected) == 0) {
            g_state.v2_palette_lut_built = 1;
        } else {
            g_state.v2_palette_lut_built = 0;
        }
    }
    return M11_RENDER_OK;
}

int M11_Render_GetV2Filters(int* outCrtEnabled,
                            int* outCrtStrength,
                            int* outPaletteEnabled,
                            int* outPaletteGamma100,
                            int* outPaletteBrightness,
                            int* outPaletteContrast,
                            int* outDitherEnabled,
                            int* outSharpenEnabled,
                            int* outSharpenStrength) {
    if (outCrtEnabled) *outCrtEnabled = g_state.v2_crt_enabled;
    if (outCrtStrength) *outCrtStrength = g_state.v2_crt_strength;
    if (outPaletteEnabled) *outPaletteEnabled = g_state.v2_palette_enabled;
    if (outPaletteGamma100) *outPaletteGamma100 = g_state.v2_palette_gamma100;
    if (outPaletteBrightness) *outPaletteBrightness = g_state.v2_palette_brightness;
    if (outPaletteContrast) *outPaletteContrast = g_state.v2_palette_contrast;
    if (outDitherEnabled) *outDitherEnabled = g_state.v2_dither_enabled;
    if (outSharpenEnabled) *outSharpenEnabled = g_state.v2_sharpen_enabled;
    if (outSharpenStrength) *outSharpenStrength = g_state.v2_sharpen_strength;
    return M11_RENDER_OK;
}

/* ---------------- DM1 V2.0 visual extras API ---------------- */

static void m11_maybe_release_prev_frame(void) {
    /* Free the prev-frame buffer only when no consumer (currently only
     * phosphor) needs it.  Keeps V1 launches at zero memory cost. */
    if (!g_state.v2_phosphor_enabled) {
        if (g_state.previousFrameBuffer) {
            free(g_state.previousFrameBuffer);
            g_state.previousFrameBuffer = NULL;
            g_state.previousFrameW = 0;
            g_state.previousFrameH = 0;
        }
    }
}

int M11_Render_SetPhosphor(int enabled, int decay) {
    if (decay < 0) decay = 0;
    if (decay > 100) decay = 100;
    g_state.v2_phosphor_enabled = enabled ? 1 : 0;
    g_state.v2_phosphor_decay = decay;
    m11_maybe_release_prev_frame();
    return M11_RENDER_OK;
}

int M11_Render_GetPhosphor(int* outEnabled, int* outDecay) {
    if (outEnabled) *outEnabled = g_state.v2_phosphor_enabled;
    if (outDecay) *outDecay = g_state.v2_phosphor_decay;
    return M11_RENDER_OK;
}
