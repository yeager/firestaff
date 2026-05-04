#ifndef FIRESTAFF_DM1_V1_DRAW_PRIMITIVES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DRAW_PRIMITIVES_PC34_COMPAT_H

/*
 * DM1 V1 Drawing Primitives — source-locked to ReDMCSB DRAW.C/BLIT.C
 *
 * Bitmap blit, rect fill, pixel operations for 320x200 indexed framebuffer.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   BLIT.C: F0099 (blit bitmap), F0100 (blit with transparency)
 *   FILLBOX.C: F0098 (fill rectangle)
 *   FLIPHORI.C: F0097 (flip horizontal)
 *   FLIPVERT.C: F0096 (flip vertical)
 *   DARKCOLR.C: F0095 (darken color for depth shading)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M11_SCREEN_W  320
#define M11_SCREEN_H  200
#define M11_BPP       1   /* 8-bit indexed color, 1 byte per pixel */

/* Bitmap descriptor — wraps a pixel buffer with dimensions */
typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int stride;   /* bytes per row, >= width */
} M11_Bitmap;

/* Rectangle */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} M11_Rect;

/*
 * Initialize a bitmap descriptor.
 * If stride==0, stride is set to width.
 */
void m11_draw_init_bitmap(M11_Bitmap *bmp, uint8_t *pixels,
                          int w, int h, int stride);

/*
 * Clear entire bitmap to a single color index.
 */
void m11_draw_clear(M11_Bitmap *bmp, uint8_t color);

/*
 * Set a single pixel (bounds-checked).
 */
void m11_draw_pixel(M11_Bitmap *bmp, int x, int y, uint8_t color);

/*
 * Get a single pixel (bounds-checked, returns 0 if out of bounds).
 */
uint8_t m11_draw_get_pixel(const M11_Bitmap *bmp, int x, int y);

/*
 * Draw a horizontal line (clipped to bitmap bounds).
 */
void m11_draw_hline(M11_Bitmap *bmp, int x, int y, int w, uint8_t color);

/*
 * Draw a vertical line (clipped to bitmap bounds).
 */
void m11_draw_vline(M11_Bitmap *bmp, int x, int y, int h, uint8_t color);

/*
 * Draw a rectangle outline (1px border).
 */
void m11_draw_rect(M11_Bitmap *bmp, const M11_Rect *r, uint8_t color);

/*
 * Fill a rectangle (clipped).
 * Source: FILLBOX.C F0098.
 */
void m11_draw_fill_rect(M11_Bitmap *bmp, const M11_Rect *r, uint8_t color);

/*
 * Blit source bitmap region to destination.
 * Source: BLIT.C F0099.
 * If srcRect is NULL, blits entire source.
 */
void m11_draw_blit(M11_Bitmap *dst, int dx, int dy,
                   const M11_Bitmap *src, const M11_Rect *srcRect);

/*
 * Blit with transparency — skip pixels matching transpColor.
 * Source: BLIT.C F0100.
 */
void m11_draw_blit_transparent(M11_Bitmap *dst, int dx, int dy,
                               const M11_Bitmap *src,
                               const M11_Rect *srcRect,
                               uint8_t transpColor);

/*
 * Nearest-neighbor scaled blit.
 */
void m11_draw_blit_scaled(M11_Bitmap *dst, const M11_Rect *dstRect,
                          const M11_Bitmap *src, const M11_Rect *srcRect);

/*
 * Flip a region horizontally in-place.
 * Source: FLIPHORI.C F0097.
 */
void m11_draw_flip_h(M11_Bitmap *bmp, const M11_Rect *r);

/*
 * Flip a region vertically in-place.
 * Source: FLIPVERT.C F0096.
 */
void m11_draw_flip_v(M11_Bitmap *bmp, const M11_Rect *r);

/*
 * Darken a color index by depth level (0=full bright, 3=darkest).
 * Source: DARKCOLR.C F0095.
 * Returns the darkened palette index.
 */
uint8_t m11_draw_darken_color(uint8_t color, int depthLevel);

/*
 * Source evidence string.
 */
const char *m11_draw_primitives_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DRAW_PRIMITIVES_PC34_COMPAT_H */
