/*
 * csb_v2_filter_dither_cleanup_pc34.c — V2.0 3x3 mode-filter on indexed FB
 * for CSB.
 *
 * Each framebuffer byte is (level << 4) | index. We compute the
 * statistical mode of the 4-bit color index in a 3x3 neighborhood
 * and replace the center pixel only when the mode is strictly more
 * common than the center value. The 4-bit brightness level is
 * preserved per pixel.
 *
 * Source: Firestaff V2.0. No ReDMCSB original equivalent — original
 * VGA output has no dither cleanup; this targets the perceived
 * dot-crawl on modern flat panels.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   No ReDMCSB original — presentation enhancement.
 *   CSB viewport: DUNVIEW.C:4547-4602 F0115 object/creature pass
 */

#include "csb_v2_filters.h"
#include "render_sdl_m11.h"

#include <stdint.h>
#include <string.h>

#define CSB_V2_MAX_FB_BYTES (M11_FB_WIDTH * M11_FB_HEIGHT)

int csb_v2_filter_dither_cleanup_indexed(uint8_t* fb, int w, int h)
{
    static uint8_t scratch[CSB_V2_MAX_FB_BYTES];
    int x, y;

    if (!fb || w <= 2 || h <= 2) {
        return -1;
    }
    if ((long)w * (long)h > (long)sizeof(scratch)) {
        /* Buffer larger than canonical 320x200 fb — skip cleanly. */
        return -1;
    }

    memcpy(scratch, fb, (size_t)(w * h));

    for (y = 1; y < h - 1; ++y) {
        for (x = 1; x < w - 1; ++x) {
            int dy, dx;
            int counts[16];
            int mode_idx, mode_count;
            int center_byte = scratch[y * w + x];
            int center_idx   = center_byte & M11_FB_INDEX_MASK;
            int center_level = center_byte & M11_FB_LEVEL_MASK;
            int center_count = 0;

            for (dy = 0; dy < 16; ++dy) counts[dy] = 0;

            for (dy = -1; dy <= 1; ++dy) {
                for (dx = -1; dx <= 1; ++dx) {
                    int b = scratch[(y + dy) * w + (x + dx)];
                    counts[b & M11_FB_INDEX_MASK]++;
                }
            }

            mode_idx   = center_idx;
            mode_count = counts[center_idx];
            center_count = counts[center_idx];
            for (dy = 0; dy < 16; ++dy) {
                if (counts[dy] > mode_count) {
                    mode_count = counts[dy];
                    mode_idx   = dy;
                }
            }

            /* Only rewrite when the neighborhood mode is strictly more
             * common than the center index (avoids touching solid
             * uniform regions and flicker on ties). */
            if (mode_count > center_count && mode_idx != center_idx) {
                fb[y * w + x] = (uint8_t)(center_level
                    | (mode_idx & M11_FB_INDEX_MASK));
            }
        }
    }
    return 0;
}
