/*
 * dm2_v1_image_pc34_compat.c — DM2 image descriptor and image operations.
 *
 * skproject: c_image.cpp
 */

#include "dm2_v1_image_pc34_compat.h"

/* ── c_imgdesc::init() ─────────────────────────────────────────────── */

DM2_V1_ImgDescInitReceipt dm2_v1_imgdesc_init(DM2_V1_ImgDesc *desc)
{
    DM2_V1_ImgDescInitReceipt r = { false };
    if (!desc)
        return r;
    memset(desc, 0, sizeof(*desc));
    r.initialized = true;
    return r;
}

/* ── c_image::init() ───────────────────────────────────────────────── */

DM2_V1_ImageInitReceipt dm2_v1_image_init(DM2_V1_Image *img)
{
    DM2_V1_ImageInitReceipt r = { false };
    if (!img)
        return r;
    memset(img, 0, sizeof(*img));
    r.initialized = true;
    return r;
}

/* ── DM2_SET_IMAGE ─────────────────────────────────────────────────── */

DM2_V1_SetImageReceipt dm2_v1_set_image(DM2_V1_ImageCallbacks *cb,
                                         void *ctx, int16_t bmpid,
                                         DM2_V1_ImgDesc *desc)
{
    DM2_V1_SetImageReceipt r = { false, 0, 0, 0 };
    if (!cb || !cb->get_bmp || !desc)
        return r;

    uint8_t *bmp = cb->get_bmp(ctx, bmpid);
    if (!bmp)
        return r;

    int16_t width  = (int16_t)((uint16_t)bmp[0] | ((uint16_t)bmp[1] << 8));
    int16_t height = (int16_t)((uint16_t)bmp[2] | ((uint16_t)bmp[3] << 8));
    int16_t res_w  = (int16_t)((uint16_t)bmp[4] | ((uint16_t)bmp[5] << 8));
    uint8_t res    = (uint8_t)(res_w & 0xff);

    desc->bmp    = bmp;
    desc->mode   = 0x8;
    desc->bmpid  = bmpid;
    desc->x      = 0;
    desc->y      = 0;
    desc->width  = width;
    desc->height = height;
    desc->w_16   = (int16_t)((uint16_t)res & 0xff);

    r.set    = true;
    r.width  = width;
    r.height = height;
    r.res    = res;
    return r;
}

/* ── DM2_ALLOCATE_IMG_COPY ─────────────────────────────────────────── */

DM2_V1_AllocateImgCopyReceipt dm2_v1_allocate_img_copy(
    DM2_V1_ImageCallbacks *cb, void *ctx, DM2_V1_ImgDesc *desc)
{
    DM2_V1_AllocateImgCopyReceipt r = { false, 0 };
    if (!cb || !desc)
        return r;

    int16_t mode = desc->mode;
    if (mode & 0x8) {
        if (cb->allocate_gfx256)
            cb->allocate_gfx256(ctx, desc->bmpid);
        r.allocated = true;
        r.mode_used = 0x8;
    } else if (mode & 0x4) {
        if (cb->allocate_gfx16)
            cb->allocate_gfx16(ctx, desc->b_08, desc->b_09, desc->b_0a);
        r.allocated = true;
        r.mode_used = 0x4;
    }
    return r;
}

/* ── DM2_image_0b36_01cd (deallocate) ──────────────────────────────── */

DM2_V1_AllocateImgCopyReceipt dm2_v1_image_deallocate(
    DM2_V1_ImageCallbacks *cb, void *ctx, DM2_V1_ImgDesc *desc)
{
    DM2_V1_AllocateImgCopyReceipt r = { false, 0 };
    if (!cb || !desc)
        return r;

    int16_t mode = desc->mode;
    if (mode & 0x8) {
        if (cb->deallocate_gfx256)
            cb->deallocate_gfx256(ctx, desc->bmpid);
        r.allocated = true;
        r.mode_used = 0x8;
    } else if (mode & 0x4) {
        if (cb->deallocate_gfx16)
            cb->deallocate_gfx16(ctx, desc->b_08, desc->b_09, desc->b_0a);
        r.allocated = true;
        r.mode_used = 0x4;
    }
    return r;
}

/* ── Resolution accessors ──────────────────────────────────────────── */

uint8_t dm2_v1_imgdesc_getres(const DM2_V1_ImgDesc *desc)
{
    if (!desc)
        return 0;
    return (uint8_t)(desc->w_16 & 0xff);
}

void dm2_v1_imgdesc_setres(DM2_V1_ImgDesc *desc, uint8_t res)
{
    if (!desc)
        return;
    desc->w_16 = (int16_t)((uint16_t)res & 0xff);
}

/* ── init_global_images (c_image.cpp:18) ──────────────────────────── */

void dm2_v1_init_global_images(DM2_V1_Image *img1, DM2_V1_Image *img2)
{
    if (img1)
        dm2_v1_image_init(img1);
    if (img2)
        dm2_v1_image_init(img2);
}

/* ── DM2_0b36_068f — compute cache key ─────────────────────────────── */

DM2_V1_CacheKeyReceipt dm2_v1_compute_cache_key(int16_t bmpid, int16_t mode)
{
    DM2_V1_CacheKeyReceipt r;
    r.key = ((int32_t)bmpid << 8) | ((int32_t)mode & 0xff);
    return r;
}

/* ── DM2_image_0b36_11c0 — draw image to button group ─────────────── */

DM2_V1_DrawReceipt dm2_v1_draw_image_to_button(
    DM2_V1_Image *img,
    int16_t bx, int16_t by, int16_t bw, int16_t bh,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!img || !cb)
        return r;
    img->rect.x = bx;
    img->rect.y = by;
    img->rect.w = bw;
    img->rect.h = bh;
    return dm2_v1_draw_picst(img, cb, ctx);
}

/* ── DM2_QUERY_PICST_IT — prepare image for rendering ──────────────── */

DM2_V1_DrawReceipt dm2_v1_query_picst_it(
    DM2_V1_Image *img,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!img || !cb)
        return r;

    DM2_V1_ImgDesc *desc = &img->imgdesc;
    if (!desc->bmp && cb->get_bmp) {
        desc->bmp = cb->get_bmp(ctx, desc->bmpid);
        if (!desc->bmp)
            return r;
    }

    img->srcx = desc->x;
    img->srcy = desc->y;
    img->bmp  = desc->bmp;

    if (cb->find_cached) {
        DM2_V1_CacheKeyReceipt ck = dm2_v1_compute_cache_key(
            desc->bmpid, desc->mode);
        uint8_t *cached = cb->find_cached(ctx, ck.key);
        if (cached) {
            img->bmp = cached;
            img->query1 = 1;
        }
    }

    r.ok = true;
    return r;
}

/* ── DM2_DRAW_PICST — core image drawing ───────────────────────────── */

DM2_V1_DrawReceipt dm2_v1_draw_picst(
    DM2_V1_Image *img,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!img || !cb)
        return r;

    if (!img->bmp) {
        DM2_V1_DrawReceipt qr = dm2_v1_query_picst_it(img, cb, ctx);
        if (!qr.ok)
            return r;
    }

    uint8_t *dst = NULL;
    if (cb->get_screen_buffer)
        dst = cb->get_screen_buffer(ctx);
    if (!dst)
        return r;

    if (img->blitmode != 0 && cb->blit_transparent) {
        cb->blit_transparent(ctx, img->bmp, dst, NULL,
                             img->srcx, img->srcy,
                             img->rect.x, img->rect.y,
                             img->rect.w, img->rect.h);
    } else if (cb->blit) {
        cb->blit(ctx, img->bmp, dst,
                 img->srcx, img->srcy,
                 img->rect.x, img->rect.y,
                 img->rect.w, img->rect.h,
                 img->blitmode);
    }

    r.ok = true;
    return r;
}

/* ── DM2_QUERY_MULTILAYERS_PIC ─────────────────────────────────────── */

DM2_V1_DrawReceipt dm2_v1_query_multilayers_pic(
    DM2_V1_Image *img, int16_t bmpid,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!img || !cb)
        return r;

    int16_t layers = 0;
    if (cb->get_layer_count)
        layers = cb->get_layer_count(ctx, bmpid);

    if (layers <= 0) {
        DM2_V1_SetImageReceipt sr = dm2_v1_set_image(
            (DM2_V1_ImageCallbacks *)cb, ctx, bmpid, &img->imgdesc);
        r.ok = sr.set;
        return r;
    }

    for (int16_t i = 0; i < layers; i++) {
        int16_t layer_id = 0;
        if (cb->get_layer_bmpid)
            layer_id = cb->get_layer_bmpid(ctx, bmpid, i);
        DM2_V1_SetImageReceipt sr = dm2_v1_set_image(
            (DM2_V1_ImageCallbacks *)cb, ctx, layer_id, &img->imgdesc);
        if (sr.set)
            dm2_v1_draw_picst(img, cb, ctx);
    }

    r.ok = true;
    return r;
}

/* ── DM2_DRAW_TEMP_PICST ──────────────────────────────────────────── */

DM2_V1_DrawReceipt dm2_v1_draw_temp_picst(
    DM2_V1_Image *img,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!img || !cb)
        return r;

    DM2_V1_AllocateImgCopyReceipt ar = dm2_v1_allocate_img_copy(
        (DM2_V1_ImageCallbacks *)cb, ctx, &img->imgdesc);
    if (!ar.allocated)
        return r;

    r = dm2_v1_draw_picst(img, cb, ctx);

    dm2_v1_image_deallocate(
        (DM2_V1_ImageCallbacks *)cb, ctx, &img->imgdesc);

    return r;
}

/* ── DM2_DRAW_TRANSPARENT_STATIC_PIC ───────────────────────────────── */

DM2_V1_DrawReceipt dm2_v1_draw_transparent_static_pic(
    DM2_V1_ImgDesc *desc, int16_t dx, int16_t dy,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!desc || !cb)
        return r;

    DM2_V1_Image img;
    dm2_v1_image_init(&img);
    img.imgdesc = *desc;
    img.rect.x = dx;
    img.rect.y = dy;
    img.rect.w = desc->width;
    img.rect.h = desc->height;
    img.blitmode = 1;
    img.bmp = desc->bmp;

    return dm2_v1_draw_picst(&img, cb, ctx);
}

/* ── DM2_DRAW_STATIC_PIC ──────────────────────────────────────────── */

DM2_V1_DrawReceipt dm2_v1_draw_static_pic(
    DM2_V1_ImgDesc *desc, int16_t dx, int16_t dy,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!desc || !cb)
        return r;

    DM2_V1_Image img;
    dm2_v1_image_init(&img);
    img.imgdesc = *desc;
    img.rect.x = dx;
    img.rect.y = dy;
    img.rect.w = desc->width;
    img.rect.h = desc->height;
    img.blitmode = 0;
    img.bmp = desc->bmp;

    return dm2_v1_draw_picst(&img, cb, ctx);
}

/* ── DM2_DRAW_DUNGEON_GRAPHIC ─────────────────────────────────────── */

DM2_V1_DrawReceipt dm2_v1_draw_dungeon_graphic(
    DM2_V1_Image *img, int16_t dx, int16_t dy,
    const uint8_t *pal,
    const DM2_V1_ImageCallbacks *cb, void *ctx)
{
    DM2_V1_DrawReceipt r = { false };
    if (!img || !cb)
        return r;

    if (pal && cb->set_palette) {
        memcpy(img->palette, pal, sizeof(img->palette));
        cb->set_palette(ctx, pal, DM2_V1_PAL256);
    }

    img->rect.x = dx;
    img->rect.y = dy;

    return dm2_v1_draw_picst(img, cb, ctx);
}

/* ── DM2_image_0b36_1446 — pixel comparison ────────────────────────── */

DM2_V1_PixelCompareReceipt dm2_v1_image_pixel_compare(
    const uint8_t *a, const uint8_t *b,
    int16_t x, int16_t y, int16_t stride)
{
    DM2_V1_PixelCompareReceipt r = { false };
    if (!a || !b)
        return r;
    int32_t offset = (int32_t)y * stride + x;
    r.match = (a[offset] == b[offset]);
    return r;
}

/* ── DEBUGBLIT — no-op in release ──────────────────────────────────── */

void dm2_v1_debugblit(const uint8_t *src, int16_t x, int16_t y,
                      int16_t w, int16_t h)
{
    (void)src; (void)x; (void)y; (void)w; (void)h;
}
