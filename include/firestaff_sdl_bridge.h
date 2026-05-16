
#ifndef FIRESTAFF_SDL_BRIDGE_H
#define FIRESTAFF_SDL_BRIDGE_H

#include <stdint.h>

/* Firestaff SDL Bridge — framebuffer → window presentation.
 * Handles window creation, texture upload, and VSync. */

typedef struct {
    void *window;     /* SDL_Window* */
    void *renderer;   /* SDL_Renderer* */
    void *texture;    /* SDL_Texture* */
    int tex_width;
    int tex_height;
    int initialized;
} FS_SDLBridge;

int fs_sdl_init(FS_SDLBridge *bridge, const char *title, int w, int h, int fullscreen, int vsync);
void fs_sdl_present_rgba(FS_SDLBridge *bridge, const uint32_t *rgba, int w, int h);
void fs_sdl_present_indexed(FS_SDLBridge *bridge, const uint8_t *indexed,
    const uint32_t *palette, int w, int h);
void fs_sdl_shutdown(FS_SDLBridge *bridge);

#endif

