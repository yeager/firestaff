#ifndef FIRESTAFF_DM2_V1_IMAGE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_IMAGE_PC34_COMPAT_H

/*
 * dm2_v1_image_pc34_compat.h — DM2 image descriptor and image operations.
 *
 * Ports image types and functions from skproject c_image.cpp/h.
 * Uses callback architecture for bitmap access and allocation.
 *
 * Source: skproject/SKWINSPX/src/v4/c_image.{h,cpp}
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "dm2_v1_rect_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Types (c_image.h) ─────────────────────────────────────────────── */

/* c_imgdesc: 0x18 bytes */
typedef struct {
    uint8_t *bmp;       /* +0x00 bitmap pointer */
    int16_t  mode;      /* +0x04 mode bits */
    int16_t  w_06;      /* +0x06 */
    int8_t   b_08;      /* +0x08 */
    int8_t   b_09;      /* +0x09 */
    int8_t   b_0a;      /* +0x0a */
    int8_t   b_0b;      /* +0x0b */
    int16_t  bmpid;     /* +0x0c t_dbidx */
    int16_t  x;         /* +0x0e */
    int16_t  y;         /* +0x10 */
    int16_t  width;     /* +0x12 */
    int16_t  height;    /* +0x14 */
    int16_t  w_16;      /* +0x16 resolution in lower byte */
} DM2_V1_ImgDesc;

/* c_image: 0x13a bytes */
typedef struct {
    DM2_V1_ImgDesc imgdesc;
    int16_t  query1;    /* +0x18 */
    int16_t  w_1a;      /* +0x1a */
    int16_t  w_1c;      /* +0x1c */
    int16_t  w_1e;      /* +0x1e */
    int16_t  srcx;      /* +0x20 */
    int16_t  srcy;      /* +0x22 */
    DM2_V1_Rect rect;   /* +0x24 */
    uint8_t *bmp;       /* +0x2c */
    int16_t  alphamask; /* +0x30 */
    int16_t  blitmode;  /* +0x32 */
    int16_t  w_34;      /* +0x34 */
    int16_t  w_36;      /* +0x36 */
    int16_t  colors;    /* +0x38 */
    uint8_t  palette[256]; /* +0x3a */
} DM2_V1_Image;

/* ── Constants ─────────────────────────────────────────────────────── */

#define DM2_V1_BPP_4   4
#define DM2_V1_BPP_8   8
#define DM2_V1_PAL256  256

/* ── Callbacks ─────────────────────────────────────────────────────── */

typedef struct {
    uint8_t *(*get_bmp)(void *ctx, int16_t bmpid);
    void (*allocate_gfx256)(void *ctx, int16_t bmpid);
    void (*allocate_gfx16)(void *ctx, int8_t a, int8_t b, int8_t c);
    void (*deallocate_gfx256)(void *ctx, int16_t bmpid);
    void (*deallocate_gfx16)(void *ctx, int8_t a, int8_t b, int8_t c);
    bool (*read_binary)(void *ctx, const char *filename, void *dest, int32_t size);

    /* Blitter callbacks for draw functions */
    void (*blit)(void *ctx, uint8_t *src, uint8_t *dst,
                 int16_t sx, int16_t sy, int16_t dx, int16_t dy,
                 int16_t w, int16_t h, int16_t mode);
    void (*blit_transparent)(void *ctx, uint8_t *src, uint8_t *dst,
                             uint8_t *mask, int16_t sx, int16_t sy,
                             int16_t dx, int16_t dy, int16_t w, int16_t h);
    uint8_t *(*get_screen_buffer)(void *ctx);
    void (*set_palette)(void *ctx, const uint8_t *pal, int16_t count);

    /* Multi-layer */
    int16_t (*get_layer_count)(void *ctx, int16_t bmpid);
    int16_t (*get_layer_bmpid)(void *ctx, int16_t bmpid, int16_t layer);

    /* Cache */
    uint8_t *(*alloc_cache)(void *ctx, int32_t size);
    void (*free_cache)(void *ctx, uint8_t *ptr);
    uint8_t *(*find_cached)(void *ctx, int32_t key);
    void (*store_cached)(void *ctx, int32_t key, uint8_t *data);
} DM2_V1_ImageCallbacks;

/* ── Receipts ──────────────────────────────────────────────────────── */

typedef struct {
    bool initialized;
} DM2_V1_ImgDescInitReceipt;

typedef struct {
    bool initialized;
} DM2_V1_ImageInitReceipt;

typedef struct {
    bool     set;
    int16_t  width;
    int16_t  height;
    uint8_t  res;
} DM2_V1_SetImageReceipt;

typedef struct {
    bool    allocated;
    int16_t mode_used;
} DM2_V1_AllocateImgCopyReceipt;

typedef struct {
    int32_t key;
} DM2_V1_CacheKeyReceipt;

typedef struct {
    bool ok;
} DM2_V1_DrawReceipt;

typedef struct {
    bool    match;
} DM2_V1_PixelCompareReceipt;

/* ── Functions ─────────────────────────────────────────────────────── */

DM2_V1_ImgDescInitReceipt dm2_v1_imgdesc_init(DM2_V1_ImgDesc *desc);

DM2_V1_ImageInitReceipt dm2_v1_image_init(DM2_V1_Image *img);

DM2_V1_SetImageReceipt dm2_v1_set_image(DM2_V1_ImageCallbacks *cb,
                                         void *ctx, int16_t bmpid,
                                         DM2_V1_ImgDesc *desc);

DM2_V1_AllocateImgCopyReceipt dm2_v1_allocate_img_copy(
    DM2_V1_ImageCallbacks *cb, void *ctx, DM2_V1_ImgDesc *desc);

DM2_V1_AllocateImgCopyReceipt dm2_v1_image_deallocate(
    DM2_V1_ImageCallbacks *cb, void *ctx, DM2_V1_ImgDesc *desc);

uint8_t dm2_v1_imgdesc_getres(const DM2_V1_ImgDesc *desc);

void dm2_v1_imgdesc_setres(DM2_V1_ImgDesc *desc, uint8_t res);

/* init_global_images — initialize two global image structs */
void dm2_v1_init_global_images(DM2_V1_Image *img1, DM2_V1_Image *img2);

/* DM2_0b36_068f — compute image cache key from bmpid and mode */
DM2_V1_CacheKeyReceipt dm2_v1_compute_cache_key(int16_t bmpid, int16_t mode);

/* DM2_image_0b36_11c0 — draw image to button group area */
DM2_V1_DrawReceipt dm2_v1_draw_image_to_button(
    DM2_V1_Image *img,
    int16_t bx, int16_t by, int16_t bw, int16_t bh,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_QUERY_PICST_IT — prepare image for rendering */
DM2_V1_DrawReceipt dm2_v1_query_picst_it(
    DM2_V1_Image *img,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_DRAW_PICST — core image drawing function */
DM2_V1_DrawReceipt dm2_v1_draw_picst(
    DM2_V1_Image *img,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_QUERY_MULTILAYERS_PIC — query multi-layer picture */
DM2_V1_DrawReceipt dm2_v1_query_multilayers_pic(
    DM2_V1_Image *img, int16_t bmpid,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_DRAW_TEMP_PICST — draw temporary picture */
DM2_V1_DrawReceipt dm2_v1_draw_temp_picst(
    DM2_V1_Image *img,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_DRAW_TRANSPARENT_STATIC_PIC — draw transparent static picture */
DM2_V1_DrawReceipt dm2_v1_draw_transparent_static_pic(
    DM2_V1_ImgDesc *desc, int16_t dx, int16_t dy,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_DRAW_STATIC_PIC — draw static picture */
DM2_V1_DrawReceipt dm2_v1_draw_static_pic(
    DM2_V1_ImgDesc *desc, int16_t dx, int16_t dy,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_DRAW_DUNGEON_GRAPHIC — draw dungeon graphic with palette */
DM2_V1_DrawReceipt dm2_v1_draw_dungeon_graphic(
    DM2_V1_Image *img, int16_t dx, int16_t dy,
    const uint8_t *pal,
    const DM2_V1_ImageCallbacks *cb, void *ctx);

/* DM2_image_0b36_1446 — pixel comparison test */
DM2_V1_PixelCompareReceipt dm2_v1_image_pixel_compare(
    const uint8_t *a, const uint8_t *b,
    int16_t x, int16_t y, int16_t stride);

/* DEBUGBLIT — debug blit helper (no-op in release) */
void dm2_v1_debugblit(const uint8_t *src, int16_t x, int16_t y,
                      int16_t w, int16_t h);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_IMAGE_PC34_COMPAT_H */
