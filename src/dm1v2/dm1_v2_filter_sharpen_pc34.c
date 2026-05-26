/*
 * dm1_v2_filter_sharpen — V2.0 3x3 unsharp mask on RGBA.
 *
 * Computes a 3x3 box-blurred copy of the input and combines
 *   out = clamp(orig + (orig - blur) * strength)
 * which is the classic unsharp-mask formula. Alpha is preserved.
 *
 * Memory: one static RGBA scratch buffer (sized for 320x200). If the
 * caller passes a larger surface we skip and return -1.
 *
 * Source: Firestaff V2.0. No ReDMCSB original equivalent.
 */

#include "dm1v2/dm1_v2_filters.h"
#include "render_sdl_m11.h"

#include <stdlib.h>
#include <string.h>

#define DM1_V2_SHARP_MAX_PIXELS (M11_FB_WIDTH * M11_FB_HEIGHT)

static int clamp_byte(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

int dm1_v2_filter_sharpen_rgba(unsigned char* rgba, int w, int h, int strength_pct) {
    static unsigned char scratch[DM1_V2_SHARP_MAX_PIXELS * 4];
    int x, y, c;
    int s;

    if (!rgba || w <= 2 || h <= 2) {
        return -1;
    }
    if ((long)w * (long)h > (long)DM1_V2_SHARP_MAX_PIXELS) {
        return -1;
    }
    if (strength_pct <= 0) {
        return 0;
    }
    if (strength_pct > 100) strength_pct = 100;
    s = strength_pct; /* 0..100 */

    memcpy(scratch, rgba, (size_t)(w * h * 4));

    for (y = 1; y < h - 1; ++y) {
        for (x = 1; x < w - 1; ++x) {
            int center_off = (y * w + x) * 4;
            for (c = 0; c < 3; ++c) {
                int sum = 0;
                int dy, dx;
                int orig = scratch[center_off + c];
                int blur, hi;
                for (dy = -1; dy <= 1; ++dy) {
                    for (dx = -1; dx <= 1; ++dx) {
                        sum += scratch[((y + dy) * w + (x + dx)) * 4 + c];
                    }
                }
                blur = sum / 9;
                /* High-frequency component scaled by strength. */
                hi = orig - blur;
                rgba[center_off + c] = (unsigned char)clamp_byte(orig + (hi * s) / 100);
            }
            /* alpha untouched */
        }
    }
    return 0;
}
