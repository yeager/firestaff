#ifndef FIRESTAFF_RENDER_SDL_M11_H
#define FIRESTAFF_RENDER_SDL_M11_H

/*
 * render_sdl_m11 — M11 Phase A
 *
 * Owns the SDL3 (or SDL2 fallback) window, renderer, and the internal
 * 320x200 framebuffer. All other render modules draw into this
 * framebuffer; render_sdl_m11 scales and presents it.
 *
 * Framebuffer layout: 320 * 200 = 64000 bytes, one byte per pixel, each
 * byte a 4-bit VGA palette index (values 0..15). Palette lookup converts
 * to 32-bit RGBA for the presentation texture.
 *
 * This module is deliberately the only SDL-aware surface in M11. All other
 * M11 modules should operate on raw byte framebuffers and call
 * M11_Render_Present() to flip.
 *
 * Cross-platform: pure C99, no platform ifdefs in the .c file apart from
 * the SDL3/SDL2 version gate which is a library-version check (not a
 * platform check).
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Framebuffer dimensions (DM native). Fixed at compile time. */
#define M11_FB_WIDTH   320
#define M11_FB_HEIGHT  200
#define M11_FB_BYTES   (M11_FB_WIDTH * M11_FB_HEIGHT)

/* Palette brightness levels supported by the VGA palette.
   0 = brightest, 5 = darkest. */
#define M11_PALETTE_LEVELS 6

/* Framebuffer pixel encoding for V1-faithful palette-level rendering.
 *
 * Each framebuffer byte encodes both the 4-bit VGA palette index and
 * a per-pixel palette brightness level, matching the original game's
 * use of VGA DAC register switching for depth-based dimming.
 *
 *   Bits 0-3: palette color index (0-15)
 *   Bits 4-7: palette brightness level (0-5, clamped to M11_PALETTE_LEVELS-1)
 *
 * Level 0 = brightest (default), Level 5 = darkest.  Code that writes
 * only a bare 4-bit index gets level 0 automatically (upper bits zero),
 * so existing callers need no changes.
 */
#define M11_FB_INDEX_MASK   0x0F
#define M11_FB_LEVEL_SHIFT  4
#define M11_FB_LEVEL_MASK   0xF0
#define M11_FB_ENCODE(index, level) \
    (unsigned char)(((unsigned)(level) << M11_FB_LEVEL_SHIFT) | ((unsigned)(index) & M11_FB_INDEX_MASK))
#define M11_FB_DECODE_INDEX(px) ((unsigned char)((px) & M11_FB_INDEX_MASK))
#define M11_FB_DECODE_LEVEL(px) ((unsigned char)(((px) & M11_FB_LEVEL_MASK) >> M11_FB_LEVEL_SHIFT))

/* Scale modes. Only 1x and 2x have meaningful behaviour in Phase A; the
   rest are wired in Phase B. Kept here so the ABI does not shift later. */
#define M11_SCALE_1X        0
#define M11_SCALE_2X        1
#define M11_SCALE_3X        2
#define M11_SCALE_4X        3
#define M11_SCALE_FIT       4
#define M11_SCALE_STRETCH   5

/* Return codes from init. Positive == success. */
#define M11_RENDER_OK                 0
#define M11_RENDER_ERR_ALREADY_INIT  -1
#define M11_RENDER_ERR_SDL_INIT      -2
#define M11_RENDER_ERR_WINDOW        -3
#define M11_RENDER_ERR_RENDERER      -4
#define M11_RENDER_ERR_TEXTURE       -5
#define M11_RENDER_ERR_INVALID_ARG   -6
#define M11_RENDER_ERR_NOT_INIT      -7

#define M11_WINDOW_MODE_WINDOWED     0
#define M11_WINDOW_MODE_MAXIMIZED    1
#define M11_WINDOW_MODE_FULLSCREEN   2

#define M11_SCALE_FILTER_NEAREST     0
#define M11_SCALE_FILTER_LINEAR      1

#define M11_VSYNC_OFF                0
#define M11_VSYNC_ON                 1

#define M11_DISPLAY_ASPECT_4_3       0
#define M11_DISPLAY_ASPECT_16_9      1
#define M11_DISPLAY_ASPECT_CONTENT   2

/* Lifecycle */
int  M11_Render_Init(int windowWidth, int windowHeight, int scaleMode);
void M11_Render_Shutdown(void);
int  M11_Render_IsInitialized(void);

/* Framebuffer access.
   Returns NULL if the module is not initialised. */
unsigned char* M11_Render_GetFramebuffer(void);
size_t         M11_Render_GetFramebufferSize(void);

/* Select active brightness level (0..M11_PALETTE_LEVELS-1).
   Returns M11_RENDER_OK or an error code. */
int  M11_Render_SetPaletteLevel(int level);
int  M11_Render_GetPaletteLevel(void);

/* Clear the framebuffer to a single 4-bit palette index value.
   Returns the number of bytes written, or -1 if not initialised. */
long M11_Render_ClearFramebuffer(unsigned char colorIndex);

/* Present: scale framebuffer via the palette to the SDL texture,
   copy to the window, and flip. Returns M11_RENDER_OK on success or
   an error code. Safe to call with an all-zero framebuffer. */
int  M11_Render_Present(void);
int  M11_Render_PresentIndexed(const unsigned char* framebuffer,
                               int logicalWidth,
                               int logicalHeight);
int  M11_Render_PresentIndexedWithSpecialPalette(const unsigned char* framebuffer,
                                                 int logicalWidth,
                                                 int logicalHeight,
                                                 int specialPalette);

/* Present a caller-owned 32-bit RGBA framebuffer directly, skipping the
 * VGA palette lookup. Used by the modern high-resolution true-color
 * startup-menu renderer. Pixel format is R, G, B, A in memory order
 * (matches SDL_PIXELFORMAT_RGBA32). */
int  M11_Render_PresentRGBA(const unsigned char* rgba,
                            int logicalWidth,
                            int logicalHeight);

/* Event pump — drains the SDL event queue once and returns whether a
   quit request (window close / ESC) was observed. Safe to call when
   not initialised (returns 0). */
int  M11_Render_PumpEvents(void);

/* Window resize notification. Phase B will act on this; Phase A stores
   the new logical size so INV_A10 can verify the plumbing. */
int  M11_Render_HandleResize(int newWidth, int newHeight);
int  M11_Render_GetWindowWidth(void);
int  M11_Render_GetWindowHeight(void);
int  M11_Render_SetScaleMode(int scaleMode);
int  M11_Render_GetScaleMode(void);
int  M11_Render_CycleScaleMode(void);
int  M11_Render_SetDisplayAspectMode(int aspectMode);
int  M11_Render_GetDisplayAspectMode(void);
int  M11_Render_ComputePresentationRect(int windowW,
                                        int windowH,
                                        int contentW,
                                        int contentH,
                                        int scaleMode,
                                        int integerScaling,
                                        int displayAspectMode,
                                        int* outX,
                                        int* outY,
                                        int* outW,
                                        int* outH);
int  M11_Render_ToggleFullscreen(void);
int  M11_Render_GetPresentRect(int* outX, int* outY, int* outW, int* outH);
int  M11_Render_MapWindowToFramebuffer(int windowX,
                                       int windowY,
                                       int* outFbX,
                                       int* outFbY);
int  M11_Render_SetWindowMode(int windowModeIndex);
int  M11_Render_GetWindowMode(void);
int  M11_Render_SyncWindowModeFromWindow(void);
int  M11_Render_SetIntegerScaling(int enabled);
int  M11_Render_GetIntegerScaling(void);
int  M11_Render_SetScaleFilter(int filterIndex);
int  M11_Render_GetScaleFilter(void);
int  M11_Render_SetVSync(int vsyncIndex);
int  M11_Render_GetVSync(void);
void M11_Render_RaiseWindow(void);

/* Query which SDL major version the build is linked against (2 or 3). */
int  M11_Render_GetSdlMajorVersion(void);

/* DM1 V2.0 filter chain controls. All filters default off (V2.0
 * starts identical to V1). Palette interpolation uses the 4-bit
 * per-pixel brightness field to blend smoothly between canonical
 * palette levels. Strength values clamped [0,100]; gamma [80,260];
 * brightness/contrast [-50,50]. */
int  M11_Render_SetV2Filters(int crtEnabled,
                             int crtStrength,
                             int paletteEnabled,
                             int paletteGamma100,
                             int paletteBrightness,
                             int paletteContrast,
                             int paletteInterpEnabled,
                             int paletteInterpStrength,
                             int ditherEnabled,
                             int sharpenEnabled,
                             int sharpenStrength);
int  M11_Render_GetV2Filters(int* outCrtEnabled,
                             int* outCrtStrength,
                             int* outPaletteEnabled,
                             int* outPaletteGamma100,
                             int* outPaletteBrightness,
                             int* outPaletteContrast,
                             int* outPaletteInterpEnabled,
                             int* outPaletteInterpStrength,
                             int* outDitherEnabled,
                             int* outSharpenEnabled,
                             int* outSharpenStrength);

/* DM1 V2.0 visual extras.  All extras default off so V1 paths stay
 * bit-identical to the original. */

/* Phosphor persistence: CRT-style afterglow on bright pixels.  Decay
 * is the percent of previous-frame intensity that bleeds through
 * (0..100, default 60). */
int  M11_Render_SetPhosphor(int enabled, int decay);
int  M11_Render_GetPhosphor(int* outEnabled, int* outDecay);

/* Color preset: 0 = Original (identity), 1..6 = named LUTs.  See
 * color_presets_m11.h for the index map. */
int  M11_Render_SetColorPreset(int preset);
int  M11_Render_GetColorPreset(int* outPreset);

/* Pixel grid overlay: thin darkening lines at every scaled-pixel
 * boundary on the output texture.  Intensity is percent darken
 * (0..100, default 20).  Auto-disables when the upscale factor is
 * below 3x to keep the grid from swamping the image. */
int  M11_Render_SetPixelGrid(int enabled, int intensity);
int  M11_Render_GetPixelGrid(int* outEnabled, int* outIntensity);

/* Motion blur during active movement: previous-frame mix-in while the
 * game reports active movement.  Strength is percent of the previous
 * frame (0..100, default 30). */
int  M11_Render_SetMotionBlur(int enabled, int strength);
int  M11_Render_GetMotionBlur(int* outEnabled, int* outStrength);

/* Movement signal used by the motion-blur path.  Set non-zero on
 * frames where the party position or facing has changed; cleared
 * automatically after the next present. */
void M11_Render_SetMovementActive(int active);
int  M11_Render_GetMovementActive(void);

/* DM1 V2 Phase 5 smooth movement: set the camera interpolation offset
 * and interpolated facing direction for the current render frame.
 *
 * offsetX/offsetY: pixel-space sub-grid camera nudge (0 = no offset).
 *   In 320x200 logical space; applied as a presentation nudge only.
 *   Source-lock: DUNGEON.C:1371-1391 direction-step movement is
 *   discrete; DUNVIEW.C:8606-8612 draws from G0306/G0307/G0308 without
 *   offset — this function is purely a V2 presentation layer.
 *
 * facingDir: interpolated 8-way facing direction (0..7) used by
 *   m11_draw_viewport to shift creature animation frames during turns.
 *   Source-lock: GAMELOOP.C:90 redraws from G0308_i_PartyDirection;
 *   interpolation is V2-only, does not change creature AI timing.
 *
 * These fields are set by the camera controller tick (dm1_v2_camera_*)
 * in the V2 presentation lane, consumed by m11_draw_viewport in
 * m11_game_view.c to offset the view cone during interpolation.
 * V1 paths leave them zero. */
int  M11_Render_SetCameraOffset(int offsetX, int offsetY, int16_t facingDir);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_RENDER_SDL_M11_H */
