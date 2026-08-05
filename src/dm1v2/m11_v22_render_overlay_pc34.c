/*
 * m11_v22_render_overlay_pc34.c
 *
 * DM1 V2.2 legacy overlay compatibility boundary.
 *
 * V2.2 pixels belong to the authenticated in-place source-art renderer.
 * This older API remains link-compatible for probes and downstream callers,
 * but missing or unverified art must never become generated framebuffer
 * pixels, palette ramps, or host diagnostics.
 */
#include "m11_v22_render_overlay_pc34.h"

int m11_v22_render_overlay_with_palette(unsigned char* framebuffer,
                                        int fbW,
                                        int fbH,
                                        int sourcePaletteIndex) {
    (void)framebuffer;
    (void)fbW;
    (void)fbH;
    (void)sourcePaletteIndex;
    return 0;
}

int m11_v22_render_overlay(unsigned char* framebuffer, int fbW, int fbH) {
    return m11_v22_render_overlay_with_palette(framebuffer, fbW, fbH, 0);
}

const char* m11_v22_render_overlay_source_evidence(void) {
    return
        "DM1 V22 legacy overlay boundary: strict no-draw when no authenticated "
        "V2.2 asset is available. The per-frame shape cache is consumed by the "
        "source-owned in-place renderer; this compatibility API never invents "
        "framebuffer pixels or palette entries. Shared cell rectangles remain "
        "available for the real renderer. Source: ReDMCSB DUNVIEW.C:6697-6816.\n";
}
