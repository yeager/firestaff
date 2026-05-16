/* DM1 V1 Blit/Fill Drawing Primitives — source-locked from ReDMCSB
 * BLIT.C: F0132 Blit operations, bitmap copy with transparency
 * FILLBOX.C: F0133 FillScreenBox, rectangle fill with color
 * EXPAND.C: F0129 decompress + blit combined operation
 * IMAGE.C: G2158 pixel line buffer[160], scanline operations */
#ifndef FIRESTAFF_DM1_V1_BLIT_FILL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BLIT_FILL_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_BF_SCREEN_W     320
#define DM1_BF_SCREEN_H     200
#define DM1_BF_BYTE_WIDTH   160   /* 320 pixels / 2 (4bpp packed) */
#define DM1_BF_BITPLANES      4
#define DM1_BF_PIXEL_LINE   160   /* G2158 buffer size */

/* Blit mode flags */
#define DM1_BF_BLIT_COPY      0x00  /* Direct copy */
#define DM1_BF_BLIT_TRANS     0x01  /* Transparent (skip color 0) */
#define DM1_BF_BLIT_XOR       0x02  /* XOR blend */
#define DM1_BF_BLIT_FLIP_H    0x04  /* Horizontal flip */
#define DM1_BF_BLIT_FLIP_V    0x08  /* Vertical flip */

/* Source bitmap descriptor */
typedef struct {
    const uint8_t* data;
    uint16_t width, height;
    uint16_t byte_width;       /* Bytes per row per bitplane */
    uint8_t  bitplanes;
} M11_BF_BlitSource;

/* Screen region for fill/blit target */
typedef struct {
    int16_t x, y;
    int16_t w, h;
} M11_BF_Rect;

/* Framebuffer reference */
typedef struct {
    uint8_t* pixels;           /* 320x200 indexed framebuffer */
    uint16_t width, height;
    uint16_t pitch;            /* Bytes per row */
} M11_BF_Framebuffer;

void m11_bf_fill_rect(M11_BF_Framebuffer* fb, const M11_BF_Rect* rect,
                       uint8_t color);
void m11_bf_clear(M11_BF_Framebuffer* fb, uint8_t color);
void m11_bf_blit(M11_BF_Framebuffer* fb, const M11_BF_BlitSource* src,
                  int16_t dst_x, int16_t dst_y, uint8_t flags);
void m11_bf_blit_scaled(M11_BF_Framebuffer* fb, const M11_BF_BlitSource* src,
                         int16_t dst_x, int16_t dst_y,
                         int16_t dst_w, int16_t dst_h, uint8_t flags);
void m11_bf_hline(M11_BF_Framebuffer* fb, int16_t x1, int16_t x2,
                   int16_t y, uint8_t color);
void m11_bf_vline(M11_BF_Framebuffer* fb, int16_t x, int16_t y1,
                   int16_t y2, uint8_t color);
void m11_bf_copy_region(M11_BF_Framebuffer* dst, const M11_BF_Framebuffer* src,
                         const M11_BF_Rect* region);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_BLIT_FILL_PC34_COMPAT_H */
