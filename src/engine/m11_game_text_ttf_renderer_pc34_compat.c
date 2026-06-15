/*
 * m11_game_text_ttf_renderer_pc34_compat.c
 *
 * TTF-based text rendering for M11 game-text.  Renders UTF-8
 * strings using SDL3_ttf when a TTF font is available for the
 * current language; otherwise falls back to the bitmap-glyph
 * path (m11_find_glyph_utf8 + m11_draw_text in m11_game_view.c).
 *
 * Public API:
 *   int m11_ttf_renderer_init(void);
 *     Initializes the SDL3_ttf library.  Returns 1 on success.
 *
 *   void m11_ttf_renderer_shutdown(void);
 *     Shuts down SDL3_ttf and frees any loaded fonts.
 *
 *   int m11_ttf_render_string(
 *       unsigned char* framebuffer,
 *       int framebufferWidth,
 *       int framebufferHeight,
 *       int x, int y,
 *       const char* utf8_text,
 *       int fontSizePixels,
 *       unsigned char colorIndex);
 *     Renders utf8_text to the framebuffer using a TTF font.
 *     The font is selected by the language set via
 *     fs_l10n_set_language.  Returns 1 on success, 0 if
 *     no TTF is available for the language (caller should
 *     fall back to bitmap path).
 *
 * Strategy: this module is a thin adapter on top of SDL3_ttf
 * that converts the per-pixel framebuffer blit into a
 * 1-bit-per-pixel font rasterization.  We render the TTF
 * glyphs to a temporary 1-channel surface, then blit each
 * pixel as the requested color index.
 */
#include "m11_game_text_ttf_renderer_pc34_compat.h"
#include "firestaff_l10n.h"
#include "firestaff_font_cache_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#if defined(FIRESTAFF_HAVE_SDL3_TTF) && FIRESTAFF_HAVE_SDL3_TTF
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL.h>
#endif

#if defined(FIRESTAFF_HAVE_SDL3_TTF) && FIRESTAFF_HAVE_SDL3_TTF
static int g_sdl3_ttf_initialized = 0;
static void* g_active_ttf_font = NULL;
static int g_active_ttf_font_size = 0;
static FS_Language g_active_ttf_font_lang = FS_LANG_COUNT;
static char g_active_ttf_font_path[1024] = {0};

int m11_ttf_renderer_init(void) {
    if (g_sdl3_ttf_initialized) return 1;
    if (!TTF_Init()) {
        fprintf(stderr, "TTF_Init failed: %s\n", SDL_GetError());
        return 0;
    }
    g_sdl3_ttf_initialized = 1;
    firestaff_font_cache_init();
    return 1;
}

void m11_ttf_renderer_shutdown(void) {
    if (g_active_ttf_font) {
        TTF_CloseFont((TTF_Font*)g_active_ttf_font);
        g_active_ttf_font = NULL;
    }
    if (g_sdl3_ttf_initialized) {
        TTF_Quit();
        g_sdl3_ttf_initialized = 0;
    }
    firestaff_font_cache_shutdown();
}

/* Load (or reload) the active TTF font for the given language
 * and size.  Returns the TTF_Font* on success, NULL on failure. */
static void* load_ttf_for_language(FS_Language lang, int size_pixels) {
    const char* path;
    TTF_Font* font;
    if (g_active_ttf_font &&
        g_active_ttf_font_size == size_pixels &&
        g_active_ttf_font_lang == lang) {
        return g_active_ttf_font;
    }
    /* Free old. */
    if (g_active_ttf_font) {
        TTF_CloseFont((TTF_Font*)g_active_ttf_font);
        g_active_ttf_font = NULL;
    }
    path = firestaff_font_cache_get_path(lang);
    if (!path) return NULL;
    font = TTF_OpenFont(path, (float)size_pixels);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont(%s, %d) failed: %s\n",
                path, size_pixels, SDL_GetError());
        return NULL;
    }
    g_active_ttf_font = (void*)font;
    g_active_ttf_font_size = size_pixels;
    g_active_ttf_font_lang = lang;
    snprintf(g_active_ttf_font_path, sizeof(g_active_ttf_font_path),
             "%s", path);
    return g_active_ttf_font;
}

int m11_ttf_render_string(
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int x, int y,
    const char* utf8_text,
    int fontSizePixels,
    unsigned char colorIndex)
{
    TTF_Font* font;
    SDL_Surface* surface;
    int srcX, srcY;
    int px, py;
    unsigned char* src_pixels;
    int src_pitch;
    Uint32* src_pixel;
    Uint8 r, g, b, a;

    if (!utf8_text || !*utf8_text) return 0;
    font = (TTF_Font*)load_ttf_for_language(
        fs_l10n_get_language(), fontSizePixels);
    if (!font) return 0;

    surface = TTF_RenderUTF8_Blended(font, utf8_text,
        (SDL_Color){255, 255, 255, 255});
    if (!surface) return 0;

    src_pixels = (unsigned char*)surface->pixels;
    src_pitch = surface->pitch;
    int srcW = surface->w;
    int srcH = surface->h;

    /* Blit: only the alpha channel matters (we use a single
     * color index for all pixels).  Skip transparent pixels. */
    for (srcY = 0; srcY < srcH; ++srcY) {
        py = y + srcY;
        if (py < 0 || py >= framebufferHeight) continue;
        for (srcX = 0; srcX < srcW; ++srcX) {
            px = x + srcX;
            if (px < 0 || px >= framebufferWidth) continue;
            /* Read 32-bit RGBA pixel. */
            src_pixel = (Uint32*)(src_pixels + srcY * src_pitch + srcX * 4);
            SDL_GetRGBA(*src_pixel, SDL_GetPixelFormatDetails(surface->format),
                        NULL, &r, &g, &b, &a);
            if (a > 0) {
                framebuffer[py * framebufferWidth + px] = colorIndex;
            }
        }
    }
    SDL_DestroySurface(surface);
    return 1;
}

const char* m11_ttf_renderer_active_font_path(void) {
    return g_active_ttf_font_path[0] ? g_active_ttf_font_path : NULL;
}

int m11_ttf_renderer_is_active(void) {
    return g_active_ttf_font != NULL;
}

#else /* !FIRESTAFF_HAVE_SDL3_TTF */

int m11_ttf_renderer_init(void) { return 0; }
void m11_ttf_renderer_shutdown(void) { }
int m11_ttf_render_string(
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int x, int y,
    const char* utf8_text,
    int fontSizePixels,
    unsigned char colorIndex)
{
    (void)framebuffer; (void)framebufferWidth; (void)framebufferHeight;
    (void)x; (void)y; (void)utf8_text; (void)fontSizePixels; (void)colorIndex;
    return 0; /* SDL3_ttf not available; use bitmap fallback */
}
const char* m11_ttf_renderer_active_font_path(void) { return NULL; }
int m11_ttf_renderer_is_active(void) { return 0; }

#endif
