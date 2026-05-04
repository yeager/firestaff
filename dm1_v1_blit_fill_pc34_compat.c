/* DM1 V1 Blit/Fill Drawing Primitives — source-locked from ReDMCSB
 * BLIT.C F0132: BlitToBitmap, multi-bitplane blit with mask
 * FILLBOX.C F0133: FillScreenBox, solid color rectangle fill
 * IMAGE.C G2158: pixel line buffer for scanline operations */

#include "dm1_v1_blit_fill_pc34_compat.h"
#include <string.h>

static inline bool clip_rect(const M11_BF_Framebuffer* fb, M11_BF_Rect* r) {
    if (r->x < 0) { r->w += r->x; r->x = 0; }
    if (r->y < 0) { r->h += r->y; r->y = 0; }
    if (r->x + r->w > (int16_t)fb->width)  r->w = (int16_t)fb->width - r->x;
    if (r->y + r->h > (int16_t)fb->height) r->h = (int16_t)fb->height - r->y;
    return (r->w > 0 && r->h > 0);
}

/* FILLBOX.C F0133 pattern: fill rectangle with solid color */
void m11_bf_fill_rect(M11_BF_Framebuffer* fb, const M11_BF_Rect* rect,
                       uint8_t color) {
    if (!fb || !fb->pixels || !rect) return;

    M11_BF_Rect r = *rect;
    if (!clip_rect(fb, &r)) return;

    for (int16_t y = r.y; y < r.y + r.h; y++) {
        memset(fb->pixels + y * fb->pitch + r.x, color, (size_t)r.w);
    }
}

void m11_bf_clear(M11_BF_Framebuffer* fb, uint8_t color) {
    if (!fb || !fb->pixels) return;
    memset(fb->pixels, color, (size_t)fb->pitch * fb->height);
}

/* BLIT.C F0132 pattern: copy source bitmap to framebuffer with flags */
void m11_bf_blit(M11_BF_Framebuffer* fb, const M11_BF_BlitSource* src,
                  int16_t dst_x, int16_t dst_y, uint8_t flags) {
    if (!fb || !fb->pixels || !src || !src->data) return;

    for (int16_t sy = 0; sy < (int16_t)src->height; sy++) {
        int16_t fy = (flags & DM1_BF_BLIT_FLIP_V) ?
                     (int16_t)(src->height - 1 - sy) : sy;
        int16_t dy = dst_y + sy;
        if (dy < 0 || dy >= (int16_t)fb->height) continue;

        for (int16_t sx = 0; sx < (int16_t)src->width; sx++) {
            int16_t fx = (flags & DM1_BF_BLIT_FLIP_H) ?
                         (int16_t)(src->width - 1 - sx) : sx;
            int16_t dx = dst_x + sx;
            if (dx < 0 || dx >= (int16_t)fb->width) continue;

            /* Read source pixel (chunky 8-bit indexed) */
            uint8_t pixel = src->data[fy * src->byte_width + fx];

            /* Transparency check */
            if ((flags & DM1_BF_BLIT_TRANS) && pixel == 0) continue;

            uint8_t* dst = &fb->pixels[dy * fb->pitch + dx];
            if (flags & DM1_BF_BLIT_XOR) {
                *dst ^= pixel;
            } else {
                *dst = pixel;
            }
        }
    }
}

/* Scaled blit — nearest-neighbor scaling for depth-based rendering */
void m11_bf_blit_scaled(M11_BF_Framebuffer* fb, const M11_BF_BlitSource* src,
                         int16_t dst_x, int16_t dst_y,
                         int16_t dst_w, int16_t dst_h, uint8_t flags) {
    if (!fb || !fb->pixels || !src || !src->data) return;
    if (dst_w <= 0 || dst_h <= 0) return;

    for (int16_t dy = 0; dy < dst_h; dy++) {
        int16_t screen_y = dst_y + dy;
        if (screen_y < 0 || screen_y >= (int16_t)fb->height) continue;

        int16_t sy = (int16_t)((uint32_t)dy * src->height / (uint32_t)dst_h);
        if (flags & DM1_BF_BLIT_FLIP_V)
            sy = (int16_t)(src->height - 1 - sy);

        for (int16_t dx = 0; dx < dst_w; dx++) {
            int16_t screen_x = dst_x + dx;
            if (screen_x < 0 || screen_x >= (int16_t)fb->width) continue;

            int16_t sx = (int16_t)((uint32_t)dx * src->width / (uint32_t)dst_w);
            if (flags & DM1_BF_BLIT_FLIP_H)
                sx = (int16_t)(src->width - 1 - sx);

            uint8_t pixel = src->data[sy * src->byte_width + sx];
            if ((flags & DM1_BF_BLIT_TRANS) && pixel == 0) continue;

            fb->pixels[screen_y * fb->pitch + screen_x] = pixel;
        }
    }
}

void m11_bf_hline(M11_BF_Framebuffer* fb, int16_t x1, int16_t x2,
                   int16_t y, uint8_t color) {
    if (!fb || !fb->pixels) return;
    if (y < 0 || y >= (int16_t)fb->height) return;
    if (x1 > x2) { int16_t t = x1; x1 = x2; x2 = t; }
    if (x1 < 0) x1 = 0;
    if (x2 >= (int16_t)fb->width) x2 = (int16_t)(fb->width - 1);
    if (x1 > x2) return;
    memset(fb->pixels + y * fb->pitch + x1, color, (size_t)(x2 - x1 + 1));
}

void m11_bf_vline(M11_BF_Framebuffer* fb, int16_t x, int16_t y1,
                   int16_t y2, uint8_t color) {
    if (!fb || !fb->pixels) return;
    if (x < 0 || x >= (int16_t)fb->width) return;
    if (y1 > y2) { int16_t t = y1; y1 = y2; y2 = t; }
    if (y1 < 0) y1 = 0;
    if (y2 >= (int16_t)fb->height) y2 = (int16_t)(fb->height - 1);
    for (int16_t y = y1; y <= y2; y++) {
        fb->pixels[y * fb->pitch + x] = color;
    }
}

void m11_bf_copy_region(M11_BF_Framebuffer* dst, const M11_BF_Framebuffer* src,
                         const M11_BF_Rect* region) {
    if (!dst || !dst->pixels || !src || !src->pixels || !region) return;

    M11_BF_Rect r = *region;
    /* Clip to both source and destination */
    if (r.x < 0) { r.w += r.x; r.x = 0; }
    if (r.y < 0) { r.h += r.y; r.y = 0; }
    if (r.x + r.w > (int16_t)src->width)  r.w = (int16_t)src->width - r.x;
    if (r.y + r.h > (int16_t)src->height) r.h = (int16_t)src->height - r.y;
    if (r.x + r.w > (int16_t)dst->width)  r.w = (int16_t)dst->width - r.x;
    if (r.y + r.h > (int16_t)dst->height) r.h = (int16_t)dst->height - r.y;
    if (r.w <= 0 || r.h <= 0) return;

    for (int16_t y = r.y; y < r.y + r.h; y++) {
        memcpy(dst->pixels + y * dst->pitch + r.x,
               src->pixels + y * src->pitch + r.x,
               (size_t)r.w);
    }
}
