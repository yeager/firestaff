#ifndef FIRESTAFF_M11_V22_RENDER_OVERLAY_PC34_H
#define FIRESTAFF_M11_V22_RENDER_OVERLAY_PC34_H

/*
 * m11_v22_render_overlay_pc34.h
 *
 * DM1 V2.2 GPU render path: V22 modern-art overlay pass.
 *
 * This is the second half of the V2.2 GPU render path data flow:
 *
 *   1. m11_v22_shape_cache_update populates the per-frame V22 shape
 *      cache (D1..D3, L/C/R) with the resolved V22 shape data.
 *   2. m11_v22_render_overlay paints a V22 modern-art placeholder
 *      over each V22-active cell on the V1 framebuffer.
 *
 * The placeholder is a solid filled rectangle in a fixed
 * "modern art" palette index, with a 1-pixel border. This is
 * intentionally a stand-in: the real V22 modern art (PBR textures,
 * normal maps, etc.) lives in ~/.firestaff/assets/dm1/modern/ as a
 * separate iteration. This overlay proves the data flow works
 * end-to-end (V22 cache -> framebuffer pixels) and gives the user
 * a visible confirmation that V22 is active.
 *
 * Source-lock: m11_v22_shape_cache_pc34.h (the cache) +
 * ReDMCSB DUNVIEW.C:6697-6816 (composition draw order).
 *
 * Module: src/dm1v2/m11_v22_render_overlay_pc34.c
 * Test:   tests/test_m11_v22_render_overlay_pc34.c
 * Probe:  probes/firestaff_m11_v22_render_overlay_probe.c
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Paint the V22 modern-art overlay onto the V1 indexed framebuffer.
 * Iterates the 9 sampled cells (D1..D3, L/C/R); for each cell where
 * the V22 cache reports active=1, draws a colored rectangle at the
 * cell's screen position. When V22 is inactive, the cache reports
 * active=0 for all cells and the overlay is a no-op.
 *
 * framebuffer: V1 indexed framebuffer (1 byte per pixel).
 * fbW, fbH:    framebuffer dimensions.
 *
 * The overlay does not change the V1 framebuffer layout or any of the
 * V1 m11_draw_dm1_* draw passes. It is layered on top of the V1
 * pixels in the post-V1-draw pass.
 *
 * Returns the number of cells painted (0..9). */
int m11_v22_render_overlay(unsigned char* framebuffer,
                           int fbW,
                           int fbH);

/* The placeholder palette index written for V22 cells. The test
 * reads this to verify the overlay wrote the right color. */
#define M11_V22_OVERLAY_PLACEHOLDER_INDEX 0xFF  /* V1 palette index 255 = bright */

/* Source evidence for tests/probes. */
const char* m11_v22_render_overlay_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_V22_RENDER_OVERLAY_PC34_H */
