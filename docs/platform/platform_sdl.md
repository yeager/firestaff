# DM1 V1 SDL/Graphics Backend — Source Lock

## SDL Version Strategy

CMake SDL discovery prefers SDL3, falls back to SDL2:
- find_package(SDL3 QUIET CONFIG) first
- pkg_check_modules(SDL3) as fallback
- If SDL2 found instead, aliased as SDL3::SDL3 target
- Fatal error if neither SDL2 nor SDL3 found

The build always links against SDL3::SDL3.

## SDL Version Guard in Code

src/shared/audio_sdl_m11.c conditionally includes SDL headers:
#ifdef FIRESTAFF_NO_SDL_AUDIO
    #define M11_HAVE_SDL_AUDIO 0
#else
    #include <SDL3/SDL.h>
    #define M11_HAVE_SDL_AUDIO 1
#endif

src/engine/main_loop_m11.c handles SDL2/SDL3 API differences:
#if !SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL.h>
#define SDLK_A SDLK_a
// ... key symbol compatibility shim for SDL2
#endif

## SDL2/SDL3 Key Differences

| SDL3 | SDL2 fallback |
|------|---------------|
| SDL_GetTicks() returns Uint64 | SDL_GetTicks() returns Uint32 |
| SDL_AudioStream | no audio stream — direct callback |
| SDL_InitSubSystem(SDL_INIT_AUDIO) | SDL_Init(SDL_INIT_AUDIO) |
| SDL_DestroyAudioStream() | N/A |
| SDL_SetAudioStreamGain() | N/A |

## Graphics Backend Abstraction

### FS_SDLBridge (include/firestaff_sdl_bridge.h)

Owns the SDL window, renderer, and texture:
typedef struct {
    void *window;     /* SDL_Window* */
    void *renderer;   /* SDL_Renderer* */
    void *texture;    /* SDL_Texture* */
    int tex_width;
    int tex_height;
    int initialized;
} FS_SDLBridge;

Functions: fs_sdl_init, fs_sdl_present_rgba, fs_sdl_present_indexed, fs_sdl_shutdown.

### M11_Audio_Backend Enum

typedef enum {
    M11_AUDIO_BACKEND_NONE = 0,
    M11_AUDIO_BACKEND_SDL3 = 1
} M11_AudioBackend;

Backend stored as void* sdlStream (SDL_AudioStream*, NULL on SDL2).

### Render Module (render_sdl_m11.c / include/render_sdl_m11.h)

Owns the internal RGBA framebuffer. SDL3 window + renderer + texture.
Double-buffered presentation via SDL_RenderPresent. Scale mode support.
Comment explicitly states: NO platform ifdefs in this file.

## Palette-Indexed Rendering Path

Game renders into 320x200 8-bit indexed framebuffer. Palette (16-color VGA)
stored separately. M11_Render_PresentIndexed() converts indexed to RGBA + presents.
VGA palette sourced from vga_palette_pc34_compat.h (16-color PC34 palette).

## Audio Backend Selection

audio_sdl_m11.c:
1. Check M11_HAVE_SDL_AUDIO (compiled-in guard)
2. Call m11_sdl_audio_backend_enabled()
3. If enabled: SDL_InitSubSystem(SDL_INIT_AUDIO) (SDL3) or SDL_Init(SDL_INIT_AUDIO) (SDL2)
4. Create SDL_AudioStream via SDL_OpenAudioDevice / SDL_CreateAudioStream

Playback via m11_audio_play_sound() writes to audio stream.
SDL2 fallback uses direct SDL_MixAudio callback or queued buffer.
