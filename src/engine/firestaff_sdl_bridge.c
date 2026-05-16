
#include "firestaff_sdl_bridge.h"
#include <string.h>
#include <stdio.h>

/* SDL bridge — SDL2 and headless support.
 * Compiles with SDL2 when HAVE_SDL2 is defined, otherwise headless stubs. */

#ifdef HAVE_SDL2
#include <SDL2/SDL.h>

int fs_sdl_init(FS_SDLBridge *b, const char *title, int w, int h, int fs, int vs) {
    if (!b) return -1;
    memset(b, 0, sizeof(*b));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }
    b->window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h, fs ? SDL_WINDOW_FULLSCREEN : 0);
    if (!b->window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return -1;
    }
    b->renderer = SDL_CreateRenderer(b->window, -1,
        SDL_RENDERER_ACCELERATED | (vs ? SDL_RENDERER_PRESENTVSYNC : 0));
    if (!b->renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return -1;
    }
    b->tex_width = w; b->tex_height = h;
    b->texture = SDL_CreateTexture(b->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, w, h);
    b->initialized = 1;
    printf("SDL2 initialized: %dx%d\n", w, h);
    return 0;
}

void fs_sdl_present_rgba(FS_SDLBridge *b, const uint32_t *rgba, int w, int h) {
    if (!b || !b->initialized || !rgba) return;
    SDL_UpdateTexture(b->texture, NULL, rgba, w * 4);
    SDL_RenderClear(b->renderer);
    SDL_RenderCopy(b->renderer, b->texture, NULL, NULL);
    SDL_RenderPresent(b->renderer);
}

void fs_sdl_shutdown(FS_SDLBridge *b) {
    if (!b || !b->initialized) return;
    if (b->texture) SDL_DestroyTexture(b->texture);
    if (b->renderer) SDL_DestroyRenderer(b->renderer);
    if (b->window) SDL_DestroyWindow(b->window);
    SDL_Quit();
    b->initialized = 0;
}

#else
/* Headless stubs */
int fs_sdl_init(FS_SDLBridge *b, const char *t, int w, int h, int fs, int vs) {
    if (b) { memset(b, 0, sizeof(*b)); b->tex_width = w; b->tex_height = h; }
    (void)t; (void)fs; (void)vs;
    printf("SDL bridge: headless mode (%dx%d)\n", w, h);
    return 0;
}
void fs_sdl_present_rgba(FS_SDLBridge *b, const uint32_t *rgba, int w, int h) {
    (void)b; (void)rgba; (void)w; (void)h;
}
void fs_sdl_present_indexed(FS_SDLBridge *b, const uint8_t *idx,
    const uint32_t *pal, int w, int h) {
    (void)b; (void)idx; (void)pal; (void)w; (void)h;
}
void fs_sdl_shutdown(FS_SDLBridge *b) { if (b) b->initialized = 0; }
#endif

