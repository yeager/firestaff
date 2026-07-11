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

#define DM1_V1_SCREEN_W_PC34       320
#define DM1_V1_SCREEN_H_PC34       200
#define DM1_V1_PALETTE_SIZE_PC34    16

/* Palette entry — 6-bit RGB per channel (0-63, Atari ST/Amiga style) */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} DM1_V1_PaletteEntryPc34;

/* Screen state — double-buffered framebuffer */
typedef struct {
    uint8_t frontBuffer[DM1_V1_SCREEN_H_PC34][DM1_V1_SCREEN_W_PC34];
    uint8_t backBuffer[DM1_V1_SCREEN_H_PC34][DM1_V1_SCREEN_W_PC34];
    DM1_V1_PaletteEntryPc34 palette[DM1_V1_PALETTE_SIZE_PC34];
    int dirty;                  /* back buffer has unswapped changes */
    int32_t lastPresentMs;      /* timestamp of last present */
    int presentCount;           /* total presents */
} DM1_V1_ScreenStatePc34;

/*
 * Initialize screen state. Clears both buffers, loads default DM1 palette.
 */
void DM1_V1_Screen_InitPc34Compat(DM1_V1_ScreenStatePc34 *s);

/*
 * Get pointer to back buffer (for drawing into).
 */
uint8_t *DM1_V1_Screen_GetBackBufferPc34Compat(DM1_V1_ScreenStatePc34 *s);

/*
 * Get pointer to front buffer (for display).
 */
const uint8_t *DM1_V1_Screen_GetFrontBufferPc34Compat(const DM1_V1_ScreenStatePc34 *s);

/*
 * Swap buffers: copy back → front.
 */
void DM1_V1_Screen_SwapBuffersPc34Compat(DM1_V1_ScreenStatePc34 *s);

/*
 * Present: swap buffers, clear dirty flag, update timestamp.
 */
void DM1_V1_Screen_PresentPc34Compat(DM1_V1_ScreenStatePc34 *s, int32_t nowMs);

/*
 * Set a single palette entry (F0093).
 */
void DM1_V1_Screen_SetPalettePc34Compat(DM1_V1_ScreenStatePc34 *s, int idx,
                            uint8_t r, uint8_t g, uint8_t b);

/*
 * Set a block of palette entries (F0094).
 */
void DM1_V1_Screen_SetPaletteBlockPc34Compat(DM1_V1_ScreenStatePc34 *s, int startIdx,
                                  const DM1_V1_PaletteEntryPc34 *entries, int count);

/*
 * Get a palette entry.
 */
DM1_V1_PaletteEntryPc34 DM1_V1_Screen_GetPalettePc34Compat(const DM1_V1_ScreenStatePc34 *s, int idx);

/*
 * Clear back buffer to a color.
 */
void DM1_V1_Screen_ClearBackPc34Compat(DM1_V1_ScreenStatePc34 *s, uint8_t color);

/*
 * Mark back buffer as dirty (needs present).
 */
void DM1_V1_Screen_MarkDirtyPc34Compat(DM1_V1_ScreenStatePc34 *s);

/*
 * Check if back buffer is dirty.
 */
int DM1_V1_Screen_IsDirtyPc34Compat(const DM1_V1_ScreenStatePc34 *s);

/*
 * Copy a region within the back buffer (for scrolling, etc.).
 */
void DM1_V1_Screen_CopyRegionPc34Compat(DM1_V1_ScreenStatePc34 *s,
                            int sx, int sy, int dx, int dy, int w, int h);

/*
 * Source evidence string.
 */
const char *DM1_V1_Screen_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SCREEN_FRAMEBUFFER_PC34_COMPAT_H */
