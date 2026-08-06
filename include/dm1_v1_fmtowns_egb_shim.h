#ifndef DM1_V1_FMTOWNS_EGB_SHIM_H
#define DM1_V1_FMTOWNS_EGB_SHIM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Bounded software shim for the TownsOS EGB primitives that the
 * FM Towns DM1 menu draw chain calls. Backed by the disassembly
 * evidence:
 *
 *   parity-evidence/dm1_fmtowns_menu_p3_disassembly.md
 *     - FILL_RECT (EDM.EXP 0x1fccc): EGB_WRITEPAGE, EGB_WRITEMODE 0,
 *       EGB_COLOR mode 2, EGB_PAINTMODE 0x20, EGB_VIEWPORT,
 *       EGB_RECTANGLE with 4-word (x1,y1,x2,y2) EGBPARA.
 *     - PIX_BLOT (EDM.EXP 0x1fe7c): EGB_PUTBLOCK / EGB_PUTBLOCKCOLOR
 *       with colour < 0 => plain copy (WRITEMODE 0),
 *       colour >= 0 => masked copy (WRITEMODE 6, EGB_COLOR mode 3).
 *
 * This shim writes into an 8bpp indexed framebuffer of caller-owned
 * dimensions. It intentionally omits FM Towns hardware plumbing
 * (VRAM page selection, resolution registers, TBIOS work areas):
 * only the visible pixel effect of each EGB call is reproduced.
 * The exact rectangle coordinates and colour indices must come
 * from source-locked data (see dm1_v1_fmtowns_menu_regions,
 * dm1_v1_fmtowns_dynamenu). Do not pass invented values.
 *
 * The shim is deliberately independent of SDL, M11 or any GRAPHICS.DAT
 * state so it can be unit-tested against a heap framebuffer.
 */

/* Clip a rectangle against a framebuffer.  Returns 1 with the
 * inclusive bounds clipped to the framebuffer, 0 if the rectangle
 * is fully off-screen or degenerate. Every EGB_RECTANGLE the shim
 * paints runs through this. */
int dm1_v1_fmtowns_egb_clip_rect_pc34(int fb_width, int fb_height,
                                      int *x1, int *y1,
                                      int *x2, int *y2);

/* Software EGB_RECTANGLE / EGB_PAINTMODE 0x20 equivalent.
 * Fills the inclusive rectangle (x1..x2, y1..y2) with colour byte
 * `colour` into an 8bpp indexed framebuffer of size fb_width x
 * fb_height whose row stride is fb_stride bytes. Returns the number
 * of pixels actually written after clipping (0 if fb is NULL, if
 * the rect clips out, or if fb_stride is invalid). */
size_t dm1_v1_fmtowns_egb_fill_rect_pc34(uint8_t *fb,
                                         int fb_width, int fb_height,
                                         int fb_stride,
                                         int x1, int y1,
                                         int x2, int y2,
                                         uint8_t colour);

/* Software EGB_PUTBLOCK / EGB_PUTBLOCKCOLOR equivalent.
 * Copies a src_width x src_height 8bpp indexed source raster
 * (row stride src_stride) into the framebuffer at (dst_x, dst_y).
 * If `colour_or_negative` >= 0, the copy is masked: source pixels
 * equal to 0 are treated as transparent and every non-zero pixel
 * becomes the byte `colour_or_negative` (mirrors EGB_WRITEMODE 6 +
 * EGB_COLOR mode 3 as observed in PIX_BLOT). If < 0, a plain
 * source-byte copy is performed (mirrors WRITEMODE 0). Returns the
 * number of destination pixels actually written after clipping. */
size_t dm1_v1_fmtowns_egb_put_block_pc34(uint8_t *fb,
                                         int fb_width, int fb_height,
                                         int fb_stride,
                                         int dst_x, int dst_y,
                                         const uint8_t *src,
                                         int src_width, int src_height,
                                         int src_stride,
                                         int colour_or_negative);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_EGB_SHIM_H */
