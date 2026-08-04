#ifndef DM2_V1_GFX_BLIT_PC34_COMPAT_H
#define DM2_V1_GFX_BLIT_PC34_COMPAT_H

/*
 * dm2_v1_gfx_blit_pc34_compat.h — DM2 graphics blitting engine.
 *
 * Source: skproject c_gfx_blit.cpp (37 functions, ~1260 lines).
 *
 * Implements the c_blitter class as a stateful C struct with pixel-level
 * blitting operations:
 *   - 4bpp-to-4bpp  (blitline_44 family)
 *   - 4bpp-to-8bpp  (blitline_48 family)
 *   - 8bpp-to-8bpp  (blitline_88 family)
 *   - 8bpp-to-8bpp with palette translation (blitline_88xlat family)
 *   - Fill operations (4bpp and 8bpp rectangles)
 *   - Stretch blit (4bpp and 8bpp)
 *   - Special effects blit (teleporters, rain)
 *   - Dither fill (stretch_4to8, checkerboard pattern)
 *
 * Pixel formats:
 *   pixel16  = uint8_t containing two 4bpp nibbles (high=left, low=right)
 *   pixel256 = uint8_t containing one 8bpp index
 *
 * Blitmode flags (skproject e_blitmode):
 *   0 = normal
 *   1 = horizontal mirror
 *   2 = vertical mirror
 *   3 = both mirrors
 *
 * Alpha masking: alphamask >= 0 means masked blit (transparent color index).
 *   The low byte is the color index to treat as transparent.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define DM2_V1_BLIT_XBLITB_SIZE  0x1000

#define DM2_V1_BLITMODE_NORMAL   0
#define DM2_V1_BLITMODE_HMIRROR  1
#define DM2_V1_BLITMODE_VMIRROR  2
#define DM2_V1_BLITMODE_HVMIRROR 3

#define DM2_V1_BPP_4  4
#define DM2_V1_BPP_8  8

/* Original screen width used by DM2 */
#define DM2_V1_ORIG_SWIDTH 320

/* ========================================================================
 * Rect (matches skproject c_rect)
 * ======================================================================== */

typedef struct DM2_V1_BlitRect {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} DM2_V1_BlitRect;

/* ========================================================================
 * Callbacks — external dependencies injected at init
 * ======================================================================== */

typedef struct DM2_V1_BlitCallbacks {
    void *ctx;

    /*
     * update_blit_palette: called before any 4-to-8 or 8xlat blit to
     * ensure pal16to256 is current.  The palette argument is the one
     * passed to the blit call (may be NULL).
     */
    void (*update_blit_palette)(void *ctx, const uint8_t *palette);

    /*
     * pal16to256: the 16-entry palette mapping table.
     * Returns pointer to 16-byte array mapping 4bpp index -> 8bpp index.
     * Called after update_blit_palette.
     */
    const uint8_t *(*get_pal16to256)(void *ctx);

    /*
     * get_default_palette: returns pointer to the default palette
     * (used by stretch_4to8).
     */
    const uint8_t *(*get_default_palette)(void *ctx);

} DM2_V1_BlitCallbacks;

/* ========================================================================
 * Blitter state — replaces the C++ c_blitter singleton
 * ======================================================================== */

typedef struct DM2_V1_BlitterState {
    /* xblitb[0x1000]: 4bpp-to-4bpp mask lookup table, loaded from xblitb.dat */
    uint8_t xblitb[DM2_V1_BLIT_XBLITB_SIZE];

    /* Current source/dest bitmap pointers (set per blit call) */
    /* 4bpp (pixel16) pointers — each byte holds two 4bpp pixels */
    uint8_t *bmpdata_src16;
    uint8_t *bmpdata_dest16;

    /* 8bpp (pixel256) pointers */
    uint8_t *bmpdata_src256;
    uint8_t *bmpdata_dest256;

    /* Generic pixel pointers (used by special effects) */
    uint8_t *bmpdata_src;
    uint8_t *bmpdata_dest;

    /* Callbacks */
    DM2_V1_BlitCallbacks cb;

    /* Initialized flag */
    bool initialized;

} DM2_V1_BlitterState;

/* ========================================================================
 * Receipt
 * ======================================================================== */

typedef struct DM2_V1_GfxBlitReceipt {
    bool handled;
    int  lines_blitted;
    int  pixels_filled;
    const char *operation;
} DM2_V1_GfxBlitReceipt;

/* ========================================================================
 * Init
 * ======================================================================== */

/*
 * Initialize the blitter with the xblitb lookup table data and callbacks.
 * xblitb_data must point to DM2_V1_BLIT_XBLITB_SIZE bytes.
 * If xblitb_data is NULL, the table is zeroed.
 */
void dm2_v1_blit_init(DM2_V1_BlitterState *st,
                      const uint8_t *xblitb_data,
                      DM2_V1_BlitCallbacks cb);

/* ========================================================================
 * 4bpp-to-4bpp line blitters (blitline_44 family)
 * ======================================================================== */

void dm2_v1_blit_blitline_44_plain(DM2_V1_BlitterState *st,
                                   uint16_t srcofs, uint16_t destofs,
                                   uint16_t pixels);

void dm2_v1_blit_blitline_44_masked(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels, uint8_t alpha);

void dm2_v1_blit_blitline_44_mirror(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels);

void dm2_v1_blit_blitline_44_mirror_masked(DM2_V1_BlitterState *st,
                                           uint16_t srcofs, uint16_t destofs,
                                           uint16_t pixels, uint8_t alpha);

/*
 * Top-level 4-to-4 blit dispatcher.
 * Handles all blitmode/alphamask combinations.
 */
void dm2_v1_blit_blitline_44(DM2_V1_BlitterState *st,
                             uint8_t *srcgfx, uint8_t *destgfx,
                             DM2_V1_BlitRect *blitrect,
                             int16_t srcx, int16_t srcy,
                             uint16_t srcw, uint16_t destw,
                             int16_t alphamask, int blitmode);

/* ========================================================================
 * 4bpp-to-8bpp line blitters (blitline_48 family)
 * ======================================================================== */

void dm2_v1_blit_blitline_48_plain(DM2_V1_BlitterState *st,
                                   uint16_t srcofs, uint16_t destofs,
                                   uint16_t pixels);

void dm2_v1_blit_blitline_48_masked(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels, uint8_t alpha);

void dm2_v1_blit_blitline_48_mirror(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels);

void dm2_v1_blit_blitline_48_mirror_masked(DM2_V1_BlitterState *st,
                                           uint16_t srcofs, uint16_t destofs,
                                           uint16_t pixels, uint8_t alpha);

void dm2_v1_blit_blitline_48(DM2_V1_BlitterState *st,
                             uint8_t *srcgfx, uint8_t *destgfx,
                             DM2_V1_BlitRect *blitrect,
                             int16_t srcofs, int16_t srcy,
                             uint16_t srcw, uint16_t destw,
                             int16_t alphamask, int blitmode,
                             const uint8_t *palette);

/* ========================================================================
 * 8bpp-to-8bpp line blitters (blitline_88 family)
 * ======================================================================== */

void dm2_v1_blit_blitline_88_plain(DM2_V1_BlitterState *st,
                                   uint16_t srcofs, uint16_t destofs,
                                   uint16_t pixels);

void dm2_v1_blit_blitline_88_masked(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels, uint8_t alpha);

void dm2_v1_blit_blitline_88_mirror(DM2_V1_BlitterState *st,
                                    uint16_t srcofs, uint16_t destofs,
                                    uint16_t pixels);

void dm2_v1_blit_blitline_88_mirror_masked(DM2_V1_BlitterState *st,
                                           uint16_t srcofs, uint16_t destofs,
                                           uint16_t pixels, uint8_t alpha);

void dm2_v1_blit_blitline_88(DM2_V1_BlitterState *st,
                             uint8_t *srcgfx, uint8_t *destgfx,
                             DM2_V1_BlitRect *blitrect,
                             int16_t srcx, int16_t srcy,
                             uint16_t srcw, uint16_t destw,
                             int16_t alphamask, int blitmode);

/* ========================================================================
 * 8bpp-to-8bpp with palette translation (blitline_88xlat family)
 * ======================================================================== */

void dm2_v1_blit_blitline_88xlat_plain(DM2_V1_BlitterState *st,
                                       uint16_t srcofs, uint16_t destofs,
                                       uint16_t pixels,
                                       const uint8_t *palette);

void dm2_v1_blit_blitline_88xlat_masked(DM2_V1_BlitterState *st,
                                        uint16_t srcofs, uint16_t destofs,
                                        uint16_t pixels, uint8_t alpha,
                                        const uint8_t *palette);

void dm2_v1_blit_blitline_88xlat_mirror(DM2_V1_BlitterState *st,
                                        uint16_t srcofs, uint16_t destofs,
                                        uint16_t pixels,
                                        const uint8_t *palette);

void dm2_v1_blit_blitline_88xlat_mirror_masked(DM2_V1_BlitterState *st,
                                               uint16_t srcofs,
                                               uint16_t destofs,
                                               uint16_t pixels, uint8_t alpha,
                                               const uint8_t *palette);

void dm2_v1_blit_blitline_88xlat(DM2_V1_BlitterState *st,
                                 uint8_t *srcgfx, uint8_t *destgfx,
                                 DM2_V1_BlitRect *blitrect,
                                 int16_t srcx, int16_t srcy,
                                 uint16_t srcw, uint16_t destw,
                                 int16_t alphamask, int blitmode,
                                 const uint8_t *palette);

/* ========================================================================
 * Unified blit dispatcher (was DM2_BLIT_PICTURE)
 * ======================================================================== */

void dm2_v1_blit_picture(DM2_V1_BlitterState *st,
                         uint8_t *srcgfx, uint8_t *destgfx,
                         DM2_V1_BlitRect *blitrect,
                         int16_t srcx, int16_t srcy,
                         uint16_t srcw, uint16_t destw,
                         int16_t alphamask, int blitmode,
                         int src_bpp, int dest_bpp,
                         const uint8_t *palette);

/* ========================================================================
 * Fill operations
 * ======================================================================== */

void dm2_v1_blit_fill_line_4(DM2_V1_BlitterState *st,
                             uint16_t ofs, uint16_t pixels, uint8_t pix16);

void dm2_v1_blit_fill_line_8(DM2_V1_BlitterState *st,
                             uint16_t ofs, uint16_t pixels, uint8_t pixel256);

void dm2_v1_blit_fill_4(DM2_V1_BlitterState *st,
                        uint8_t *gfxdata, uint8_t pix,
                        uint16_t stride_pixels, DM2_V1_BlitRect *blitrect);

void dm2_v1_blit_fill_8(DM2_V1_BlitterState *st,
                        uint8_t *gfxdata, uint8_t pixel256,
                        uint16_t stride_pixels, DM2_V1_BlitRect *blitrect);

void dm2_v1_blit_fill(DM2_V1_BlitterState *st,
                      uint8_t *gfxdata, uint8_t pix,
                      uint16_t stride_pixels, DM2_V1_BlitRect *blitrect,
                      int bpp);

/* ========================================================================
 * Stretch blit
 * ======================================================================== */

int32_t dm2_v1_blit_calc_stretched_size(int16_t eaxw, int16_t edxw);

void dm2_v1_blit_stretch16(DM2_V1_BlitterState *st,
                           uint8_t *srcgfx, uint8_t *destgfx,
                           int16_t width, int16_t height,
                           int16_t xpixels, int16_t totalpixels,
                           uint8_t *stretchptr);

void dm2_v1_blit_stretch256(DM2_V1_BlitterState *st,
                            uint8_t *srcgfx, uint8_t *destgfx,
                            int16_t width, int16_t height,
                            int16_t argw0, int16_t argw1);

/* ========================================================================
 * Within-screen blit (was sub_25AF)
 * ======================================================================== */

void dm2_v1_blit_within_screen(DM2_V1_BlitterState *st,
                               uint8_t *screen,
                               DM2_V1_BlitRect *rectp,
                               int16_t yofs);

/* ========================================================================
 * Special effects blit (teleporters, rain)
 * ======================================================================== */

void dm2_v1_blit_specialeffects(DM2_V1_BlitterState *st,
                                uint8_t *srcgfx, uint8_t *destgfx,
                                uint8_t *gfx,
                                DM2_V1_BlitRect *ecxrp,
                                int16_t argw0, int16_t argw1,
                                int16_t argw2, int16_t argw3,
                                const uint8_t *palette);

void dm2_v1_blit_sub_specialeffects(DM2_V1_BlitterState *st,
                                    uint8_t *srcgfx, uint8_t *destgfx,
                                    uint8_t *gfx,
                                    DM2_V1_BlitRect *ecxrp,
                                    int16_t xend, int16_t srcofs,
                                    int16_t argw2, int16_t argw3,
                                    int16_t argw4, int16_t pixperline,
                                    int16_t argw6,
                                    const uint8_t *palette);

/* ========================================================================
 * Dither fill (stretch_4to8, checkerboard pattern)
 * ======================================================================== */

void dm2_v1_blit_stretch_4to8(DM2_V1_BlitterState *st,
                              uint8_t *destgfx,
                              DM2_V1_BlitRect *rect,
                              uint8_t mask, int16_t width);

/* ========================================================================
 * Parity evidence
 * ======================================================================== */

const char *dm2_v1_gfx_blit_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_GFX_BLIT_PC34_COMPAT_H */
