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

#define DM1_V1_DRAW_SCREEN_W_PC34 320
#define DM1_V1_DRAW_SCREEN_H_PC34 200
#define DM1_V1_DRAW_BPP_PC34      1   /* 8-bit indexed color, 1 byte per pixel */

#define M11_SCREEN_W  DM1_V1_DRAW_SCREEN_W_PC34
#define M11_SCREEN_H  DM1_V1_DRAW_SCREEN_H_PC34
#define M11_BPP       DM1_V1_DRAW_BPP_PC34

/* Bitmap descriptor — wraps a pixel buffer with dimensions */
typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int stride;   /* bytes per row, >= width */
} DM1_V1_BitmapPc34;

/* Rectangle */
typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_RectPc34;

/*
 * Initialize a bitmap descriptor.
 * If stride==0, stride is set to width.
 */
void DM1_V1_Draw_InitBitmapPc34Compat(DM1_V1_BitmapPc34 *bmp, uint8_t *pixels,
                          int w, int h, int stride);

/*
 * Clear entire bitmap to a single color index.
 */
void DM1_V1_Draw_ClearPc34Compat(DM1_V1_BitmapPc34 *bmp, uint8_t color);

/*
 * Set a single pixel (bounds-checked).
 */
void DM1_V1_Draw_PixelPc34Compat(DM1_V1_BitmapPc34 *bmp, int x, int y, uint8_t color);

/*
 * Get a single pixel (bounds-checked, returns 0 if out of bounds).
 */
uint8_t DM1_V1_Draw_GetPixelPc34Compat(const DM1_V1_BitmapPc34 *bmp, int x, int y);

/*
 * Draw a horizontal line (clipped to bitmap bounds).
 */
void DM1_V1_Draw_HLinePc34Compat(DM1_V1_BitmapPc34 *bmp, int x, int y, int w, uint8_t color);

/*
 * Draw a vertical line (clipped to bitmap bounds).
 */
void DM1_V1_Draw_VLinePc34Compat(DM1_V1_BitmapPc34 *bmp, int x, int y, int h, uint8_t color);

/*
 * Draw a rectangle outline (1px border).
 */
void DM1_V1_Draw_RectPc34Compat(DM1_V1_BitmapPc34 *bmp, const DM1_V1_RectPc34 *r, uint8_t color);

/*
 * Fill a rectangle (clipped).
 * Source: FILLBOX.C F0098.
 */
void DM1_V1_Draw_FillRectPc34Compat(DM1_V1_BitmapPc34 *bmp, const DM1_V1_RectPc34 *r, uint8_t color);

/*
 * Blit source bitmap region to destination.
 * Source: BLIT.C F0099.
 * If srcRect is NULL, blits entire source.
 */
void DM1_V1_Draw_BlitPc34Compat(DM1_V1_BitmapPc34 *dst, int dx, int dy,
                   const DM1_V1_BitmapPc34 *src, const DM1_V1_RectPc34 *srcRect);

/*
 * Blit with transparency — skip pixels matching transpColor.
 * Source: BLIT.C F0100.
 */
void DM1_V1_Draw_BlitTransparentPc34Compat(DM1_V1_BitmapPc34 *dst, int dx, int dy,
                               const DM1_V1_BitmapPc34 *src,
                               const DM1_V1_RectPc34 *srcRect,
                               uint8_t transpColor);

/*
 * Nearest-neighbor scaled blit.
 */
void DM1_V1_Draw_BlitScaledPc34Compat(DM1_V1_BitmapPc34 *dst, const DM1_V1_RectPc34 *dstRect,
                          const DM1_V1_BitmapPc34 *src, const DM1_V1_RectPc34 *srcRect);

/*
 * Flip a region horizontally in-place.
 * Source: FLIPHORI.C F0097.
 */
void DM1_V1_Draw_FlipHPc34Compat(DM1_V1_BitmapPc34 *bmp, const DM1_V1_RectPc34 *r);

/*
 * Flip a region vertically in-place.
 * Source: FLIPVERT.C F0096.
 */
void DM1_V1_Draw_FlipVPc34Compat(DM1_V1_BitmapPc34 *bmp, const DM1_V1_RectPc34 *r);

/*
 * Darken a color index by depth level (0=full bright, 3=darkest).
 * Source: DARKCOLR.C F0095.
 * Returns the darkened palette index.
 */
uint8_t DM1_V1_Draw_DarkenColorPc34Compat(uint8_t color, int depthLevel);

/*
 * Source evidence string.
 */
const char *DM1_V1_Draw_PrimitivesSourceEvidencePc34Compat(void);

typedef DM1_V1_BitmapPc34 M11_Bitmap;
typedef DM1_V1_RectPc34 M11_Rect;

#define m11_draw_init_bitmap DM1_V1_Draw_InitBitmapPc34Compat
#define m11_draw_clear DM1_V1_Draw_ClearPc34Compat
#define m11_draw_pixel DM1_V1_Draw_PixelPc34Compat
#define m11_draw_get_pixel DM1_V1_Draw_GetPixelPc34Compat
#define m11_draw_hline DM1_V1_Draw_HLinePc34Compat
#define m11_draw_vline DM1_V1_Draw_VLinePc34Compat
#define m11_draw_rect DM1_V1_Draw_RectPc34Compat
#define m11_draw_fill_rect DM1_V1_Draw_FillRectPc34Compat
#define m11_draw_blit DM1_V1_Draw_BlitPc34Compat
#define m11_draw_blit_transparent DM1_V1_Draw_BlitTransparentPc34Compat
#define m11_draw_blit_scaled DM1_V1_Draw_BlitScaledPc34Compat
#define m11_draw_flip_h DM1_V1_Draw_FlipHPc34Compat
#define m11_draw_flip_v DM1_V1_Draw_FlipVPc34Compat
#define m11_draw_darken_color DM1_V1_Draw_DarkenColorPc34Compat
#define m11_draw_primitives_source_evidence DM1_V1_Draw_PrimitivesSourceEvidencePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DRAW_PRIMITIVES_PC34_COMPAT_H */
