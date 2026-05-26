#ifndef FIRESTAFF_DM1_V2_FILTERS_H
#define FIRESTAFF_DM1_V2_FILTERS_H

/*
 * dm1_v2_filters — DM1 V2.0 filtered-presentation chain.
 *
 * V2.0 = V1 gameplay route + V1 framebuffer + CPU post-process filter
 * chain on the RGBA present buffer (and optionally on the indexed
 * framebuffer before palette expansion). All four filters are off by
 * default so V2.0 is bit-identical to V1 until a user opts in.
 *
 * Source: Firestaff V2.0 filtered presentation. No ReDMCSB original
 * equivalent — ReDMCSB targets raw VGA DAC output via real-hardware
 * CRTC port programming (VIDEODRV.C / PALETTE.C). V2.0 emulates the
 * perceived CRT look on modern flat-panel displays.
 *
 * See: DM1_V2_PLAN.md section 1.5/1.6.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Palette LUT shape: 6 brightness levels x 16 indices x RGB triplet.
 * Mirrors G9010_auc_VgaPaletteAll_Compat[6][16][3] layout in
 * include/vga_palette_pc34_compat.h so the LUT is a drop-in. */
#define DM1_V2_PALETTE_LEVELS 6

/* Build a corrected palette LUT from the canonical VGA palette.
 *
 *   gamma100   — desired display gamma * 100. 100 => identity (1.00),
 *                220 default (2.20 PC-CRT). Clamped to [80, 260].
 *   brightness — additive bias in percent of full white (-50..+50).
 *   contrast   — multiplicative gain in percent (-50..+50).
 *   out_lut    — destination [levels][16][3], filled in.
 *
 * Returns 0 on success, -1 on null pointer.
 *
 * Source: Firestaff V2.0. Uses G9010_auc_VgaPaletteAll_Compat as input.
 */
int dm1_v2_filter_palette_build_lut(int gamma100,
                                    int brightness,
                                    int contrast,
                                    unsigned char out_lut[DM1_V2_PALETTE_LEVELS][16][3]);

/* 3x3 mode-filter on the indexed framebuffer. Each byte is
 * (level << 4) | index. We mode-filter only the 4-bit color index;
 * the brightness level is preserved per-pixel.
 *
 * Operates in-place via a single temporary row buffer in static
 * storage (no heap allocation). Pixels on the outer 1-pixel border
 * are left untouched.
 *
 *   fb   — pointer to indexed framebuffer (w*h bytes).
 *   w,h  — width and height in pixels.
 *
 * Returns 0 on success, -1 on null pointer or non-positive dim.
 */
int dm1_v2_filter_dither_cleanup_indexed(unsigned char* fb, int w, int h);

/* 3x3 unsharp mask on RGBA8888 buffer.
 *
 *   rgba         — pointer to RGBA buffer (w*h*4 bytes).
 *   w,h          — dimensions in pixels.
 *   strength_pct — 0..100, 0 = no-op, 100 = full strength.
 *
 * Returns 0 on success, -1 on invalid args. Alpha channel preserved.
 */
int dm1_v2_filter_sharpen_rgba(unsigned char* rgba, int w, int h, int strength_pct);

/* CRT scanlines: even rows multiplied by (1 - strength/100).
 *
 *   rgba         — pointer to RGBA buffer (w*h*4 bytes).
 *   w,h          — dimensions in pixels.
 *   strength_pct — 0..100, 0 = no-op, 100 = even rows fully black.
 *
 * Returns 0 on success, -1 on invalid args. Alpha channel preserved.
 */
int dm1_v2_filter_crt_scanlines_rgba(unsigned char* rgba, int w, int h, int strength_pct);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_FILTERS_H */
