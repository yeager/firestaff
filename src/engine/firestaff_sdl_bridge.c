
#include "firestaff_sdl_bridge.h"
#include <string.h>
#include <stdio.h>

/* SDL bridge — stubbed for headless builds.
 * Full SDL3 implementation requires SDL3 linked.
 * This file compiles without SDL for CI/testing. */

#ifdef HAVE_SDL3
#include <SDL3/SDL.h>

int fs_sdl_init(FS_SDLBridge *b, const char *title, int w, int h, int fs, int vs) {
    if (!b) return -1;
    memset(b, 0, sizeof(*b));
    SDL_Init(SDL_INIT_VIDEO);
    b->window = SDL_CreateWindow(title, w, h, fs ? SDL_WINDOW_FULLSCREEN : 0);
    b->renderer = SDL_CreateRenderer(b->window, NULL);
    if (vs) SDL_SetRenderVSync(b->renderer, 1);
    b->tex_width = w; b->tex_height = h;
    b->texture = SDL_CreateTexture(b->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, w, h);
    b->initialized = 1;
    return 0;
}

void fs_sdl_present_rgba(FS_SDLBridge *b, const uint32_t *rgba, int w, int h) {
    if (!b || !b->initialized || !rgba) return;
    SDL_UpdateTexture(b->texture, NULL, rgba, w * 4);
    SDL_RenderClear(b->renderer);
    SDL_RenderTexture(b->renderer, b->texture, NULL, NULL);
    SDL_RenderPresent(b->renderer);
}

void fs_sdl_shutdown(FS_SDLBridge *b) {
    if (!b || !b->initialized) return;
    SDL_DestroyTexture(b->texture);
    SDL_DestroyRenderer(b->renderer);
    SDL_DestroyWindow(b->window);
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

