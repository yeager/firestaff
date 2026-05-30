/*
 * csb_v2_filters.h — CSB V2.0 Post-Processing Filter Declarations
 *
 * V2.0 filtered presentation applies post-process filters over the
 * RGBA output of the V2.1 EPX pipeline (or directly over V1 surfaces
 * when EPX is disabled). These are presentation-only transforms; they
 * do not alter V1 game-logic state.
 *
 * Filter ordering (V2.0 FILTERED mode):
 *   1. dither_cleanup — 3x3 mode-filter on indexed FB (before palette expand)
 *   2. palette_interpolate — smooth per-pixel brightness in indexed FB
 *   3. crt_scanlines    — even-row dimming for CRT effect (RGBA surface)
 *   4. palette_correct  — gamma/brightness/contrast (LUT build, used at expand time)
 *   5. sharpen          — 3x3 unsharp mask on RGBA surface
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   No ReDMCSB original equivalent — all filters are Firestaff V2.0
 *   presentation enhancements targeting modern flat-panel displays.
 *   Palette constants: DATA.C:359-360 (k_source_palette_light_amount_floor)
 *   Palette selection: PANEL.C:418-428 (G0304_i_DungeonViewPaletteIndex)
 *   CSB-specific palette: ENTRANCE.C:409-441 (C28_ENTRANCE_CSB)
 *
 * Requires: csb_v2_asset_pipeline_pc34.h (CSB_V2_PALETTE_LEVELS)
 */

#ifndef FIRESTAFF_CSB_V2_FILTERS_H
#define FIRESTAFF_CSB_V2_FILTERS_H

#include <stddef.h>
#include <stdint.h>

/* CSB_V2_PALETTE_LEVELS defined by csb_v2_asset_pipeline_pc34.h */
#include "csb_v2_asset_pipeline_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── CRT Scanlines ────────────────────────────────────────────────── */

/* Apply CRT scanline dimming to an RGBA surface in-place.
 * Every even-numbered row (y =0, 2, 4, ...) is multiplied by
 *   gain = (100 - strength_pct) / 100
 * Odd rows are left untouched. Alpha channel is preserved.
 *
 * rgba:       RGBA buffer (4 bytes/pixel, RGBA8888)
 * w, h:       surface dimensions
 * strength_pct: 0..100 (0 = no effect, 100 = maximum dimming)
 *
 * Returns 0 on success, -1 on invalid input. */
int csb_v2_filter_crt_scanlines_rgba(uint8_t* rgba, int w, int h,
    int strength_pct);

/* ── Palette Correction LUT Builder ───────────────────────────────── */

/* Build a corrected [levels][16][3] RGB LUT from G9010_auc_VgaPaletteAll_Compat
 * with gamma / brightness / contrast adjustments.
 *
 * gamma100:    80..260 (100 = no gamma change, 220 ≈ 2.2 CRT approximation)
 * brightness:  -50..+50 (raw byte offset added to normalized output)
 * contrast:    -50..+50 (multiplicative gain around midpoint 128)
 * out_lut:     output buffer [CSB_V2_PALETTE_LEVELS][16][3]
 *
 * Returns 0 on success, -1 if out_lut is NULL. */
int csb_v2_filter_palette_build_lut(int gamma100, int brightness, int contrast,
    uint8_t out_lut[CSB_V2_PALETTE_LEVELS][16][3]);

/* ── Dither Cleanup (indexed FB) ─────────────────────────────────── */

/* 3x3 mode-filter on indexed framebuffer bytes.
 * Each byte encodes (level << 4) | palette_index. The4-bit palette
 * index is replaced by the statistical mode of the 3x3 neighborhood
 * only when the mode is strictly more common than the center pixel.
 * The4-bit brightness level is preserved unchanged.
 *
 * fb:  indexed framebuffer (M11_FB_WIDTH × M11_FB_HEIGHT)
 * w, h: surface dimensions
 *
 * Returns 0 on success, -1 on invalid input or oversized surface. */
int csb_v2_filter_dither_cleanup_indexed(uint8_t* fb, int w, int h);

/* ── Palette Interpolation (indexed FB) ──────────────────────────── */

/* Smooth per-pixel brightness interpolation on indexed framebuffer.
 * CSB encodes brightness in 4 bits (0-15) per pixel, with 6 discrete
 * canonical levels (0=brightest, 5=darkest). Intermediate values are
 * blended between adjacent canonical levels for smooth gradients.
 *
 * fb:           indexed framebuffer
 * w, h:         surface dimensions
 * strength_pct:  0..100 (0 = no interpolation, 100 = full strength)
 *
 * Returns 0 on success, -1 on invalid input. */
int csb_v2_filter_palette_interpolate_indexed(uint8_t* fb, int w, int h,
    int strength_pct);

/* ── Sharpen (RGBA surface) ────────────────────────────────────────── */

/* 3x3 unsharp mask on an RGBA surface in-place.
 * Computes a 3x3 box-blurred copy and combines:
 *   out = clamp(orig + (orig - blur) * strength)
 * Alpha channel is preserved.
 *
 * rgba:          RGBA buffer (4 bytes/pixel, RGBA8888)
 * w, h:          surface dimensions
 * strength_pct:  0..100 (0 = no effect, 100 = maximum sharpen)
 *
 * Returns 0 on success, -1 if surface is too large for scratch buffer. */
int csb_v2_filter_sharpen_rgba(uint8_t* rgba, int w, int h, int strength_pct);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_FILTERS_H */
