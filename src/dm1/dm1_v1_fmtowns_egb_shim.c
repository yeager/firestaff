#include "dm1_v1_fmtowns_egb_shim.h"

/* Software shim for the TownsOS EGB primitives that the FM Towns
 * DM1 menu draw chain calls. See header for the disassembly evidence
 * that motivates each primitive. This module intentionally handles
 * only the visible pixel effect; TownsOS work-area / VRAM-page
 * plumbing is out of scope. */

int dm1_v1_fmtowns_egb_clip_rect_pc34(int fb_width, int fb_height,
                                      int *x1, int *y1,
                                      int *x2, int *y2) {
    int lo_x, hi_x, lo_y, hi_y;
    if (!x1 || !y1 || !x2 || !y2) return 0;
    if (fb_width <= 0 || fb_height <= 0) return 0;
    lo_x = *x1 < *x2 ? *x1 : *x2;
    hi_x = *x1 > *x2 ? *x1 : *x2;
    lo_y = *y1 < *y2 ? *y1 : *y2;
    hi_y = *y1 > *y2 ? *y1 : *y2;
    if (hi_x < 0 || hi_y < 0) return 0;
    if (lo_x >= fb_width || lo_y >= fb_height) return 0;
    if (lo_x < 0)             lo_x = 0;
    if (lo_y < 0)             lo_y = 0;
    if (hi_x >= fb_width)     hi_x = fb_width - 1;
    if (hi_y >= fb_height)    hi_y = fb_height - 1;
    if (lo_x > hi_x || lo_y > hi_y) return 0;
    *x1 = lo_x; *y1 = lo_y; *x2 = hi_x; *y2 = hi_y;
    return 1;
}

size_t dm1_v1_fmtowns_egb_fill_rect_pc34(uint8_t *fb,
                                         int fb_width, int fb_height,
                                         int fb_stride,
                                         int x1, int y1,
                                         int x2, int y2,
                                         uint8_t colour) {
    int y, x;
    size_t pixels = 0;
    if (!fb || fb_stride < fb_width) return 0;
    if (!dm1_v1_fmtowns_egb_clip_rect_pc34(fb_width, fb_height,
                                           &x1, &y1, &x2, &y2)) return 0;
    for (y = y1; y <= y2; ++y) {
        uint8_t *row = fb + (size_t)y * (size_t)fb_stride;
        for (x = x1; x <= x2; ++x) {
            row[x] = colour;
            ++pixels;
        }
    }
    return pixels;
}

size_t dm1_v1_fmtowns_egb_put_block_pc34(uint8_t *fb,
                                         int fb_width, int fb_height,
                                         int fb_stride,
                                         int dst_x, int dst_y,
                                         const uint8_t *src,
                                         int src_width, int src_height,
                                         int src_stride,
                                         int colour_or_negative) {
    int sy, sx;
    int x1 = dst_x;
    int y1 = dst_y;
    int x2 = dst_x + src_width - 1;
    int y2 = dst_y + src_height - 1;
    int clipped_x1 = x1, clipped_y1 = y1, clipped_x2 = x2, clipped_y2 = y2;
    size_t pixels = 0;
    if (!fb || !src || fb_stride < fb_width) return 0;
    if (src_width <= 0 || src_height <= 0) return 0;
    if (src_stride < src_width) return 0;
    if (!dm1_v1_fmtowns_egb_clip_rect_pc34(fb_width, fb_height,
                                           &clipped_x1, &clipped_y1,
                                           &clipped_x2, &clipped_y2)) return 0;
    /* Iterate over the clipped destination rectangle, sampling the
     * source at the offset that keeps (dst_x, dst_y) aligned even
     * when the destination is off-screen top/left. */
    for (sy = clipped_y1; sy <= clipped_y2; ++sy) {
        int src_row = sy - dst_y;
        const uint8_t *sr = src + (size_t)src_row * (size_t)src_stride;
        uint8_t *dr = fb + (size_t)sy * (size_t)fb_stride;
        for (sx = clipped_x1; sx <= clipped_x2; ++sx) {
            int src_col = sx - dst_x;
            uint8_t sp = sr[src_col];
            if (colour_or_negative >= 0) {
                /* Masked copy: zero source pixels stay transparent;
                 * non-zero pixels become the caller-supplied colour.
                 * Mirrors PIX_BLOT's WRITEMODE 6 + COLOR mode 3. */
                if (sp != 0) {
                    dr[sx] = (uint8_t)colour_or_negative;
                    ++pixels;
                }
            } else {
                dr[sx] = sp;
                ++pixels;
            }
        }
    }
    return pixels;
}
