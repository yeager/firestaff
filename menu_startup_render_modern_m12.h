/*
 * menu_startup_render_modern_m12.h
 *
 * Modern, high-resolution, true-color (24-bit RGB) renderer for the
 * M12 startup menu. Operates on the existing M12_StartupMenuState
 * without modifying it, and is strictly limited to the startup-menu
 * presentation (front door of the app). In-game V1 presentation is
 * untouched by this module: once the user enters a game, the
 * palette-indexed pipeline continues to render the authentic VGA
 * experience.
 *
 * Output:
 *   - 1920x1080 RGBA32 framebuffer, byte order R, G, B, A.
 *   - Caller provides the buffer (must be at least W*H*4 bytes).
 *   - Pixel format matches SDL_PIXELFORMAT_RGBA32 so the renderer
 *     can hand it directly to SDL for true-color presentation.
 *
 * Scope boundary (locked):
 *   - Only the startup menu shell (MAIN / SETTINGS / GAME OPTIONS /
 *     MESSAGE views) uses this renderer.
 *   - V1 in-game rendering does NOT use this renderer.
 *   - V2 enhanced 2D rendering inside a game is a separate future
 *     milestone and is not implemented here.
 */

#ifndef FIRESTAFF_MENU_STARTUP_RENDER_MODERN_M12_H
#define FIRESTAFF_MENU_STARTUP_RENDER_MODERN_M12_H

#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_MODERN_MENU_NATIVE_WIDTH = 1920,
    M12_MODERN_MENU_NATIVE_HEIGHT = 1080
};

int M12_ModernMenu_NativeWidth(void);
int M12_ModernMenu_NativeHeight(void);

/* Render the startup menu into an RGBA32 true-color buffer. Always
 * succeeds when inputs are valid. Safe with any non-null buffer whose
 * dimensions are >= 16x16. */
void M12_ModernMenu_Render(const M12_StartupMenuState* state,
                           unsigned char* rgba,
                           int width,
                           int height);

/* Count how many distinct 24-bit RGB values appear in the first
 * `sampleBytes` bytes of the given RGBA32 buffer. Used by the modern
 * menu probe to prove true-color output (>> 16 distinct colours,
 * unlike the 16-entry VGA palette). */
int M12_ModernMenu_CountDistinctColors(const unsigned char* rgba,
                                       int width,
                                       int height,
                                       int capAfter);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_STARTUP_RENDER_MODERN_M12_H */
