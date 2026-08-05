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
 *   2. The authenticated in-place renderer paints V22 art. This legacy
 *      overlay API is retained as a strict no-draw compatibility boundary.
 *
 * Missing or unverified V2.2 art must remain no-draw. No procedural
 * rectangle, palette ramp, tint, or host diagnostic is permitted here.
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

/* Legacy compatibility entry point. It always returns 0 and leaves the
 * framebuffer untouched; authenticated V22 pixels belong to the in-place
 * asset renderer.
 *
 * framebuffer: V1 indexed framebuffer (1 byte per pixel).
 * fbW, fbH:    framebuffer dimensions.
 *
 * The overlay does not change the V1 framebuffer layout or any of the
 * V1 m11_draw_dm1_* draw passes. It is layered on top of the V1
 * pixels in the post-V1-draw pass.
 *
 * Returns 0. */
int m11_v22_render_overlay(unsigned char* framebuffer,
                           int fbW,
                           int fbH);
int m11_v22_render_overlay_with_palette(unsigned char* framebuffer,
                                        int fbW,
                                        int fbH,
                                        int sourcePaletteIndex);

/* DM1 V2.2 4x3 cell rect coordinates (depth x lateral) used by both
 * the overlay pass and the in-place pass. Exposed so the in-place
 * module can paint bitmaps at the same coords the overlay uses,
 * guaranteeing pixel-for-pixel Z-order equivalence. */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} M11_V22_CellRect;

/* Return the shared V2.2 viewport cell rectangle for a source-view cell.
 * depth is {1,2,3}; lateral is {-1,0,1}. Returns NULL out of range.
 * Both the retired overlay boundary and in-place bitmap pass use this source
 * so their DM1 4x3 cell geometry cannot drift apart. */
const M11_V22_CellRect* m11_v22_cell_rect(int depth, int lateral);

/* Source evidence for tests/probes. */
const char* m11_v22_render_overlay_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_V22_RENDER_OVERLAY_PC34_H */
