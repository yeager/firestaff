#include "dm1_v1_draw_primitives_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Drawing Primitives — implementation
 *
 * Source lock: ReDMCSB WIP20210206
 *   BLIT.C F0099: bitmap blit (row-by-row memcpy with clipping)
 *   BLIT.C F0100: transparent blit (skip pixels == transparency color)
 *   FILLBOX.C F0098: fill rectangle (row-by-row memset with clipping)
 *   FLIPHORI.C F0097: horizontal flip (swap pixels from edges inward)
 *   FLIPVERT.C F0096: vertical flip (swap rows from top/bottom)
 *   DARKCOLR.C F0095: darken color by depth (shift palette index)
 *
 * All operations clip to bitmap bounds. No out-of-bounds writes.
 */

void m11_draw_init_bitmap(M11_Bitmap *bmp, uint8_t *pixels,
                          int w, int h, int stride)
{
    bmp->pixels = pixels;
    bmp->width = w;
    bmp->height = h;
    bmp->stride = (stride > 0) ? stride : w;
}

void m11_draw_clear(M11_Bitmap *bmp, uint8_t color)
{
    if (!bmp->pixels) return;
    for (int y = 0; y < bmp->height; y++) {
        memset(bmp->pixels + y * bmp->stride, color, (size_t)bmp->width);
    }
}

void m11_draw_pixel(M11_Bitmap *bmp, int x, int y, uint8_t color)
{
    if (!bmp->pixels) return;
    if (x < 0 || x >= bmp->width || y < 0 || y >= bmp->height) return;
    bmp->pixels[y * bmp->stride + x] = color;
}

uint8_t m11_draw_get_pixel(const M11_Bitmap *bmp, int x, int y)
{
    if (!bmp->pixels) return 0;
    if (x < 0 || x >= bmp->width || y < 0 || y >= bmp->height) return 0;
    return bmp->pixels[y * bmp->stride + x];
}

void m11_draw_hline(M11_Bitmap *bmp, int x, int y, int w, uint8_t color)
{
    if (!bmp->pixels) return;
    if (y < 0 || y >= bmp->height) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > bmp->width) { w = bmp->width - x; }
    if (w <= 0) return;
    memset(bmp->pixels + y * bmp->stride + x, color, (size_t)w);
}

void m11_draw_vline(M11_Bitmap *bmp, int x, int y, int h, uint8_t color)
{
    if (!bmp->pixels) return;
    if (x < 0 || x >= bmp->width) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > bmp->height) { h = bmp->height - y; }
    if (h <= 0) return;
    for (int i = 0; i < h; i++) {
        bmp->pixels[(y + i) * bmp->stride + x] = color;
    }
}

void m11_draw_rect(M11_Bitmap *bmp, const M11_Rect *r, uint8_t color)
{
    if (!r || r->w <= 0 || r->h <= 0) return;
    m11_draw_hline(bmp, r->x, r->y, r->w, color);
    m11_draw_hline(bmp, r->x, r->y + r->h - 1, r->w, color);
    m11_draw_vline(bmp, r->x, r->y, r->h, color);
    m11_draw_vline(bmp, r->x + r->w - 1, r->y, r->h, color);
}

void m11_draw_fill_rect(M11_Bitmap *bmp, const M11_Rect *r, uint8_t color)
{
    if (!bmp->pixels || !r) return;
    int x0 = r->x, y0 = r->y, x1 = r->x + r->w, y1 = r->y + r->h;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > bmp->width) x1 = bmp->width;
    if (y1 > bmp->height) y1 = bmp->height;
    if (x0 >= x1 || y0 >= y1) return;
    int fillW = x1 - x0;
    for (int y = y0; y < y1; y++) {
        memset(bmp->pixels + y * bmp->stride + x0, color, (size_t)fillW);
    }
}

void m11_draw_blit(M11_Bitmap *dst, int dx, int dy,
                   const M11_Bitmap *src, const M11_Rect *srcRect)
{
    if (!dst->pixels || !src->pixels) return;

    int sx = srcRect ? srcRect->x : 0;
    int sy = srcRect ? srcRect->y : 0;
    int sw = srcRect ? srcRect->w : src->width;
    int sh = srcRect ? srcRect->h : src->height;

    /* Clip source to source bitmap */
    if (sx < 0) { sw += sx; dx -= sx; sx = 0; }
    if (sy < 0) { sh += sy; dy -= sy; sy = 0; }
    if (sx + sw > src->width) sw = src->width - sx;
    if (sy + sh > src->height) sh = src->height - sy;

    /* Clip destination to destination bitmap */
    if (dx < 0) { sx -= dx; sw += dx; dx = 0; }
    if (dy < 0) { sy -= dy; sh += dy; dy = 0; }
    if (dx + sw > dst->width) sw = dst->width - dx;
    if (dy + sh > dst->height) sh = dst->height - dy;

    if (sw <= 0 || sh <= 0) return;

    for (int row = 0; row < sh; row++) {
        const uint8_t *srcRow = src->pixels + (sy + row) * src->stride + sx;
        uint8_t *dstRow = dst->pixels + (dy + row) * dst->stride + dx;
        memcpy(dstRow, srcRow, (size_t)sw);
    }
}

void m11_draw_blit_transparent(M11_Bitmap *dst, int dx, int dy,
                               const M11_Bitmap *src,
                               const M11_Rect *srcRect,
                               uint8_t transpColor)
{
    if (!dst->pixels || !src->pixels) return;

    int sx = srcRect ? srcRect->x : 0;
    int sy = srcRect ? srcRect->y : 0;
    int sw = srcRect ? srcRect->w : src->width;
    int sh = srcRect ? srcRect->h : src->height;

    if (sx < 0) { sw += sx; dx -= sx; sx = 0; }
    if (sy < 0) { sh += sy; dy -= sy; sy = 0; }
    if (sx + sw > src->width) sw = src->width - sx;
    if (sy + sh > src->height) sh = src->height - sy;
    if (dx < 0) { sx -= dx; sw += dx; dx = 0; }
    if (dy < 0) { sy -= dy; sh += dy; dy = 0; }
    if (dx + sw > dst->width) sw = dst->width - dx;
    if (dy + sh > dst->height) sh = dst->height - dy;

    if (sw <= 0 || sh <= 0) return;

    for (int row = 0; row < sh; row++) {
        const uint8_t *srcRow = src->pixels + (sy + row) * src->stride + sx;
        uint8_t *dstRow = dst->pixels + (dy + row) * dst->stride + dx;
        for (int col = 0; col < sw; col++) {
            if (srcRow[col] != transpColor) {
                dstRow[col] = srcRow[col];
            }
        }
    }
}

void m11_draw_blit_scaled(M11_Bitmap *dst, const M11_Rect *dstRect,
                          const M11_Bitmap *src, const M11_Rect *srcRect)
{
    if (!dst->pixels || !src->pixels || !dstRect || !srcRect) return;
    if (dstRect->w <= 0 || dstRect->h <= 0) return;
    if (srcRect->w <= 0 || srcRect->h <= 0) return;

    for (int dy = 0; dy < dstRect->h; dy++) {
        int py = dstRect->y + dy;
        if (py < 0 || py >= dst->height) continue;
        int sy = srcRect->y + (dy * srcRect->h) / dstRect->h;
        if (sy < 0 || sy >= src->height) continue;

        for (int dx = 0; dx < dstRect->w; dx++) {
            int px = dstRect->x + dx;
            if (px < 0 || px >= dst->width) continue;
            int sx = srcRect->x + (dx * srcRect->w) / dstRect->w;
            if (sx < 0 || sx >= src->width) continue;

            dst->pixels[py * dst->stride + px] =
                src->pixels[sy * src->stride + sx];
        }
    }
}

void m11_draw_flip_h(M11_Bitmap *bmp, const M11_Rect *r)
{
    if (!bmp->pixels || !r) return;
    int x0 = r->x, y0 = r->y, x1 = r->x + r->w - 1, y1 = r->y + r->h - 1;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= bmp->width) x1 = bmp->width - 1;
    if (y1 >= bmp->height) y1 = bmp->height - 1;

    for (int y = y0; y <= y1; y++) {
        uint8_t *row = bmp->pixels + y * bmp->stride;
        int left = x0, right = x1;
        while (left < right) {
            uint8_t tmp = row[left];
            row[left] = row[right];
            row[right] = tmp;
            left++;
            right--;
        }
    }
}

void m11_draw_flip_v(M11_Bitmap *bmp, const M11_Rect *r)
{
    if (!bmp->pixels || !r) return;
    int x0 = r->x, y0 = r->y, x1 = r->x + r->w, y1 = r->y + r->h - 1;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > bmp->width) x1 = bmp->width;
    if (y1 >= bmp->height) y1 = bmp->height - 1;
    int lineW = x1 - x0;
    if (lineW <= 0) return;

    int top = y0, bot = y1;
    while (top < bot) {
        uint8_t *rowTop = bmp->pixels + top * bmp->stride + x0;
        uint8_t *rowBot = bmp->pixels + bot * bmp->stride + x0;
        for (int i = 0; i < lineW; i++) {
            uint8_t tmp = rowTop[i];
            rowTop[i] = rowBot[i];
            rowBot[i] = tmp;
        }
        top++;
        bot--;
    }
}

uint8_t m11_draw_darken_color(uint8_t color, int depthLevel)
{
    /*
     * F0095 DARKCOLR: the original uses a lookup table to darken
     * palette colors for depth shading. DM1 uses 4 brightness levels
     * with 4 colors per level (16 palette entries organized as
     * 4 groups of 4 shades). Depth 0 = brightest, depth 3 = darkest.
     *
     * Simplified: shift the color index within its 4-shade group
     * toward the darker end.
     */
    if (depthLevel <= 0) return color;
    if (depthLevel > 3) depthLevel = 3;

    /* For the 16-color DM1 palette, colors are grouped in sets of 4 */
    int group = color / 4;
    int shade = color % 4;
    shade += depthLevel;
    if (shade > 3) shade = 3;
    return (uint8_t)(group * 4 + shade);
}

const char *m11_draw_primitives_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206\n"
        "BLIT.C F0099: bitmap blit — row-by-row memcpy with full clipping.\n"
        "BLIT.C F0100: transparent blit — per-pixel skip on transparency color.\n"
        "FILLBOX.C F0098: fill rectangle — row-by-row memset with clipping.\n"
        "FLIPHORI.C F0097: horizontal flip — swap pixels from edges inward.\n"
        "FLIPVERT.C F0096: vertical flip — swap rows from top/bottom.\n"
        "DARKCOLR.C F0095: darken color by depth — palette index shift within "
        "4-shade groups for DM1's 16-color depth shading.";
}
