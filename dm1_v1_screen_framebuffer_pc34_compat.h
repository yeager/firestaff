#ifndef FIRESTAFF_DM1_V1_SCREEN_FRAMEBUFFER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SCREEN_FRAMEBUFFER_PC34_COMPAT_H

/*
 * DM1 V1 Screen/Framebuffer — source-locked to ReDMCSB SCREEN.C
 *
 * Framebuffer management, palette, double buffering, present.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   Multiple files contribute to screen management:
 *   PALETTE.C: F0093 (set palette entry), F0094 (set palette block)
 *   SCRLMGMT.C: scroll/screen management
 *   DM.C/INIT.C: framebuffer initialization
 *   VBLANK.C: vertical blank timing for present
 *
 * DM1 uses a 320x200 8-bit indexed framebuffer with a 16-color palette.
 * The Atari ST original uses bitplane format; we use chunky 8-bit.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M11_SCREEN_W       320
#define M11_SCREEN_H       200
#define M11_PALETTE_SIZE    16

/* Palette entry — 6-bit RGB per channel (0-63, Atari ST/Amiga style) */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} M11_PaletteEntry;

/* Screen state — double-buffered framebuffer */
typedef struct {
    uint8_t frontBuffer[M11_SCREEN_H][M11_SCREEN_W];
    uint8_t backBuffer[M11_SCREEN_H][M11_SCREEN_W];
    M11_PaletteEntry palette[M11_PALETTE_SIZE];
    int dirty;                  /* back buffer has unswapped changes */
    int32_t lastPresentMs;      /* timestamp of last present */
    int presentCount;           /* total presents */
} M11_ScreenState;

/*
 * Initialize screen state. Clears both buffers, loads default DM1 palette.
 */
void m11_screen_init(M11_ScreenState *s);

/*
 * Get pointer to back buffer (for drawing into).
 */
uint8_t *m11_screen_get_back_buffer(M11_ScreenState *s);

/*
 * Get pointer to front buffer (for display).
 */
const uint8_t *m11_screen_get_front_buffer(const M11_ScreenState *s);

/*
 * Swap buffers: copy back → front.
 */
void m11_screen_swap_buffers(M11_ScreenState *s);

/*
 * Present: swap buffers, clear dirty flag, update timestamp.
 */
void m11_screen_present(M11_ScreenState *s, int32_t nowMs);

/*
 * Set a single palette entry (F0093).
 */
void m11_screen_set_palette(M11_ScreenState *s, int idx,
                            uint8_t r, uint8_t g, uint8_t b);

/*
 * Set a block of palette entries (F0094).
 */
void m11_screen_set_palette_block(M11_ScreenState *s, int startIdx,
                                  const M11_PaletteEntry *entries, int count);

/*
 * Get a palette entry.
 */
M11_PaletteEntry m11_screen_get_palette(const M11_ScreenState *s, int idx);

/*
 * Clear back buffer to a color.
 */
void m11_screen_clear_back(M11_ScreenState *s, uint8_t color);

/*
 * Mark back buffer as dirty (needs present).
 */
void m11_screen_mark_dirty(M11_ScreenState *s);

/*
 * Check if back buffer is dirty.
 */
int m11_screen_is_dirty(const M11_ScreenState *s);

/*
 * Copy a region within the back buffer (for scrolling, etc.).
 */
void m11_screen_copy_region(M11_ScreenState *s,
                            int sx, int sy, int dx, int dy, int w, int h);

/*
 * Source evidence string.
 */
const char *m11_screen_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SCREEN_FRAMEBUFFER_PC34_COMPAT_H */
