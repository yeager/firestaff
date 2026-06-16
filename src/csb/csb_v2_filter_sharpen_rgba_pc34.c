/*
 * csb_v2_filter_sharpen_rgba_pc34.c — V2.0 unsharp-mask sharpen on RGBA
 * for CSB.
 *
 * 3x3 unsharp mask: out = clamp(orig + (orig - blur) * strength).
 * Alpha channel is preserved.
 *
 * Source: Firestaff V2.0. No ReDMCSB original equivalent.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   No ReDMCSB original — presentation enhancement.
 *   CSB viewport: DUNVIEW.C:6204-6218 (door panel), DUNVIEW.C:6816-6831 (field draw)
 */

#include "csb_v2_filters.h"
#include "render_sdl_m11.h"

#include <stdint.h>
#include <string.h>

#define CSB_V2_SHARPEN_MAX_FB_PIXELS (M11_FB_WIDTH * M11_FB_HEIGHT)

static uint8_t clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

int csb_v2_filter_sharpen_rgba(uint8_t* rgba, int w, int h, int strength_pct) {
    static uint8_t scratch[CSB_V2_SHARPEN_MAX_FB_PIXELS * 4];
    int x, y, c;
    if (!rgba || w <= 2 || h <= 2) {
        return -1;
    }
    if (strength_pct <= 0) {
        return 0;
    }
    if (strength_pct > 100) {
        strength_pct = 100;
    }
    if ((long)w * (long)h > (long)(CSB_V2_SHARPEN_MAX_FB_PIXELS)) {
        return -1;
    }

    memcpy(scratch, rgba, (size_t)(w * h * 4));

    for (y = 1; y < h - 1; ++y) {
        for (x = 1; x < w - 1; ++x) {
            for (c = 0; c < 3; ++c) {  /* RGB only; alpha is preserved */
                int blur = 0;
                int dy, dx;
                int orig;
                for (dy = -1; dy <= 1; ++dy) {
                    for (dx = -1; dx <= 1; ++dx) {
                        blur += scratch[(y + dy) * w * 4 + (x + dx) * 4 + c];
                    }
                }
                blur /= 9;  /* 3x3 box blur */
                orig = scratch[y * w * 4 + x * 4 + c];
                /* High-boost: out = orig + (orig - blur) * strength */
                int out = orig + ((orig - blur) * strength_pct) / 100;
                rgba[y * w * 4 + x * 4 + c] = clamp_u8(out);
            }
            /* Alpha preserved. */
        }
    }
    return 0;
}
