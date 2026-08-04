/*
 * test_dm2_v1_image_pc34_compat.c — Tests for DM2 image module.
 *
 * skproject: c_image.cpp
 */

#include "dm2_v1_image_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ── Mock state ────────────────────────────────────────────────────── */

static int mock_alloc256_called;
static int16_t mock_alloc256_bmpid;
static int mock_alloc16_called;
static int mock_dealloc256_called;
static int mock_dealloc16_called;

/* Mock bitmap: 6-byte header (width=64 LE, height=48 LE, res=8 LE) */
static uint8_t mock_bmp_data[] = {
    0x40, 0x00,  /* width = 64 */
    0x30, 0x00,  /* height = 48 */
    0x08, 0x00   /* res = 8 */
};

static uint8_t *mock_get_bmp(void *ctx __attribute__((unused)),
                              int16_t bmpid __attribute__((unused)))
{
    return mock_bmp_data;
}

static void mock_allocate_gfx256(void *ctx __attribute__((unused)),
                                  int16_t bmpid)
{
    mock_alloc256_called++;
    mock_alloc256_bmpid = bmpid;
}

static void mock_allocate_gfx16(void *ctx __attribute__((unused)),
                                 int8_t a __attribute__((unused)),
                                 int8_t b __attribute__((unused)),
                                 int8_t c __attribute__((unused)))
{
    mock_alloc16_called++;
}

static void mock_deallocate_gfx256(void *ctx __attribute__((unused)),
                                    int16_t bmpid __attribute__((unused)))
{
    mock_dealloc256_called++;
}

static void mock_deallocate_gfx16(void *ctx __attribute__((unused)),
                                   int8_t a __attribute__((unused)),
                                   int8_t b __attribute__((unused)),
                                   int8_t c __attribute__((unused)))
{
    mock_dealloc16_called++;
}

static DM2_V1_ImageCallbacks mock_cb = {
    .get_bmp           = mock_get_bmp,
    .allocate_gfx256   = mock_allocate_gfx256,
    .allocate_gfx16    = mock_allocate_gfx16,
    .deallocate_gfx256 = mock_deallocate_gfx256,
    .deallocate_gfx16  = mock_deallocate_gfx16,
    .read_binary       = NULL
};

/* ── Tests ─────────────────────────────────────────────────────────── */

static void test_imgdesc_init(void)
{
    DM2_V1_ImgDesc desc;
    memset(&desc, 0xff, sizeof(desc));
    DM2_V1_ImgDescInitReceipt r = dm2_v1_imgdesc_init(&desc);
    assert(r.initialized);
    assert(desc.bmp == NULL);
    assert(desc.mode == 0);
    assert(desc.w_06 == 0);
    assert(desc.b_08 == 0 && desc.b_09 == 0 && desc.b_0a == 0 && desc.b_0b == 0);
    assert(desc.bmpid == 0);
    assert(desc.x == 0 && desc.y == 0);
    assert(desc.width == 0 && desc.height == 0);
    assert(desc.w_16 == 0);
    printf("test_imgdesc_init OK\n");
}

static void test_image_init(void)
{
    DM2_V1_Image img;
    memset(&img, 0xff, sizeof(img));
    DM2_V1_ImageInitReceipt r = dm2_v1_image_init(&img);
    assert(r.initialized);
    assert(img.imgdesc.bmp == NULL);
    assert(img.imgdesc.mode == 0);
    assert(img.query1 == 0);
    assert(img.srcx == 0 && img.srcy == 0);
    assert(img.bmp == NULL);
    assert(img.alphamask == 0);
    assert(img.colors == 0);
    /* Check palette zeroed */
    for (int i = 0; i < 256; i++)
        assert(img.palette[i] == 0);
    printf("test_image_init OK\n");
}

static void test_set_image(void)
{
    DM2_V1_ImgDesc desc;
    dm2_v1_imgdesc_init(&desc);
    DM2_V1_SetImageReceipt r = dm2_v1_set_image(&mock_cb, NULL, 42, &desc);
    assert(r.set);
    assert(r.width == 64);
    assert(r.height == 48);
    assert(r.res == 8);
    assert(desc.bmp == mock_bmp_data);
    assert(desc.mode == 0x8);
    assert(desc.bmpid == 42);
    assert(desc.x == 0 && desc.y == 0);
    assert(desc.width == 64);
    assert(desc.height == 48);
    assert(dm2_v1_imgdesc_getres(&desc) == 8);
    printf("test_set_image OK\n");
}

static void test_allocate_img_copy_mode8(void)
{
    mock_alloc256_called = 0;
    mock_alloc16_called = 0;
    DM2_V1_ImgDesc desc;
    dm2_v1_imgdesc_init(&desc);
    desc.mode = 0x8;
    desc.bmpid = 7;
    DM2_V1_AllocateImgCopyReceipt r = dm2_v1_allocate_img_copy(&mock_cb, NULL, &desc);
    assert(r.allocated);
    assert(r.mode_used == 0x8);
    assert(mock_alloc256_called == 1);
    assert(mock_alloc256_bmpid == 7);
    assert(mock_alloc16_called == 0);
    printf("test_allocate_img_copy_mode8 OK\n");
}

static void test_allocate_img_copy_mode4(void)
{
    mock_alloc256_called = 0;
    mock_alloc16_called = 0;
    DM2_V1_ImgDesc desc;
    dm2_v1_imgdesc_init(&desc);
    desc.mode = 0x4;
    DM2_V1_AllocateImgCopyReceipt r = dm2_v1_allocate_img_copy(&mock_cb, NULL, &desc);
    assert(r.allocated);
    assert(r.mode_used == 0x4);
    assert(mock_alloc16_called == 1);
    assert(mock_alloc256_called == 0);
    printf("test_allocate_img_copy_mode4 OK\n");
}

static void test_deallocate_mode8(void)
{
    mock_dealloc256_called = 0;
    mock_dealloc16_called = 0;
    DM2_V1_ImgDesc desc;
    dm2_v1_imgdesc_init(&desc);
    desc.mode = 0x8;
    DM2_V1_AllocateImgCopyReceipt r = dm2_v1_image_deallocate(&mock_cb, NULL, &desc);
    assert(r.allocated);
    assert(r.mode_used == 0x8);
    assert(mock_dealloc256_called == 1);
    assert(mock_dealloc16_called == 0);
    printf("test_deallocate_mode8 OK\n");
}

static void test_getres_setres(void)
{
    DM2_V1_ImgDesc desc;
    dm2_v1_imgdesc_init(&desc);
    dm2_v1_imgdesc_setres(&desc, 0x20);
    assert(dm2_v1_imgdesc_getres(&desc) == 0x20);
    dm2_v1_imgdesc_setres(&desc, 0xff);
    assert(dm2_v1_imgdesc_getres(&desc) == 0xff);
    dm2_v1_imgdesc_setres(&desc, 0);
    assert(dm2_v1_imgdesc_getres(&desc) == 0);
    printf("test_getres_setres OK\n");
}

static void test_init_global_images(void)
{
    DM2_V1_Image img1, img2;
    memset(&img1, 0xff, sizeof(img1));
    memset(&img2, 0xff, sizeof(img2));
    dm2_v1_init_global_images(&img1, &img2);
    assert(img1.imgdesc.bmp == NULL);
    assert(img1.imgdesc.mode == 0);
    assert(img1.query1 == 0);
    assert(img1.colors == 0);
    assert(img2.imgdesc.bmp == NULL);
    assert(img2.imgdesc.mode == 0);
    assert(img2.query1 == 0);
    assert(img2.colors == 0);
    /* Palette zeroed */
    for (int i = 0; i < 256; i++) {
        assert(img1.palette[i] == 0);
        assert(img2.palette[i] == 0);
    }
    /* NULL safety */
    dm2_v1_init_global_images(NULL, &img2);
    dm2_v1_init_global_images(&img1, NULL);
    dm2_v1_init_global_images(NULL, NULL);
    printf("test_init_global_images OK\n");
}

static void test_compute_cache_key(void)
{
    /* key = (bmpid << 8) | (mode & 0xff) */
    DM2_V1_CacheKeyReceipt r;

    r = dm2_v1_compute_cache_key(0, 0);
    assert(r.key == 0);

    r = dm2_v1_compute_cache_key(1, 0);
    assert(r.key == 256);

    r = dm2_v1_compute_cache_key(0, 8);
    assert(r.key == 8);

    r = dm2_v1_compute_cache_key(42, 0x08);
    assert(r.key == (42 << 8 | 0x08));
    assert(r.key == 10760);

    r = dm2_v1_compute_cache_key(100, 0x04);
    assert(r.key == (100 << 8 | 0x04));
    assert(r.key == 25604);

    /* Mode is masked to low byte */
    r = dm2_v1_compute_cache_key(5, 0x1ff);
    assert(r.key == (5 << 8 | 0xff));

    printf("test_compute_cache_key OK\n");
}

static void test_pixel_compare(void)
{
    uint8_t a[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
    uint8_t b[] = { 0, 1, 2, 3, 99, 5, 6, 7, 8 };

    DM2_V1_PixelCompareReceipt r;
    r = dm2_v1_image_pixel_compare(a, b, 0, 0, 3);
    assert(r.match); /* both 0 */

    r = dm2_v1_image_pixel_compare(a, b, 1, 1, 3);
    assert(r.match == false); /* a[4]=4, b[4]=99 */

    r = dm2_v1_image_pixel_compare(a, b, 2, 2, 3);
    assert(r.match); /* both 8 */

    /* NULL safety */
    r = dm2_v1_image_pixel_compare(NULL, b, 0, 0, 3);
    assert(r.match == false);

    printf("test_pixel_compare OK\n");
}

int main(void)
{
    test_imgdesc_init();
    test_image_init();
    test_set_image();
    test_allocate_img_copy_mode8();
    test_allocate_img_copy_mode4();
    test_deallocate_mode8();
    test_getres_setres();
    test_init_global_images();
    test_compute_cache_key();
    test_pixel_compare();
    printf("All dm2_v1_image tests passed.\n");
    return 0;
}
