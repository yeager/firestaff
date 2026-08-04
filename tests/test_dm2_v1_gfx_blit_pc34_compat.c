/*
 * test_dm2_v1_gfx_blit_pc34_compat.c — Tests for DM2 graphics blitting engine.
 *
 * Tests cover: init, fill operations, 8-to-8 plain/masked/mirror blit,
 * 4-to-4 aligned plain blit, 8-to-8 palette translation, calc_stretched_size,
 * and the unified blit_picture dispatcher.
 */

#include "dm2_v1_gfx_blit_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  [TEST] %s ... ", #name); \
    test_##name(); \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)

/* ========================================================================
 * Stub callbacks for 4-to-8 tests
 * ======================================================================== */

static uint8_t test_pal16to256[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

static const uint8_t *stub_get_pal16to256(void *ctx)
{
    (void)ctx;
    return test_pal16to256;
}

static void stub_update_blit_palette(void *ctx, const uint8_t *palette)
{
    (void)ctx;
    (void)palette;
}

static const uint8_t *stub_get_default_palette(void *ctx)
{
    (void)ctx;
    return NULL;
}

static DM2_V1_BlitCallbacks make_test_callbacks(void)
{
    DM2_V1_BlitCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx = NULL;
    cb.get_pal16to256 = stub_get_pal16to256;
    cb.update_blit_palette = stub_update_blit_palette;
    cb.get_default_palette = stub_get_default_palette;
    return cb;
}

/* ========================================================================
 * Tests
 * ======================================================================== */

static void test_init(void)
{
    DM2_V1_BlitterState st;
    uint8_t xblitb[DM2_V1_BLIT_XBLITB_SIZE];
    memset(xblitb, 0xAB, sizeof(xblitb));

    dm2_v1_blit_init(&st, xblitb, make_test_callbacks());

    assert(st.initialized == true);
    assert(st.bmpdata_src16 == NULL);
    assert(st.bmpdata_dest16 == NULL);
    assert(st.xblitb[0] == 0xAB);
    assert(st.xblitb[DM2_V1_BLIT_XBLITB_SIZE - 1] == 0xAB);
}

static void test_init_null_xblitb(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());
    assert(st.initialized == true);
    assert(st.xblitb[0] == 0x00);
}

static void test_fill_8(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t buf[64];
    memset(buf, 0, sizeof(buf));

    DM2_V1_BlitRect rect = { 1, 1, 3, 2 };
    dm2_v1_blit_fill_8(&st, buf, 0x42, 8, &rect);

    /* Row 0 should be untouched */
    assert(buf[0] == 0x00);
    /* Row 1, col 1-3 should be 0x42 */
    assert(buf[8 + 0] == 0x00);
    assert(buf[8 + 1] == 0x42);
    assert(buf[8 + 2] == 0x42);
    assert(buf[8 + 3] == 0x42);
    assert(buf[8 + 4] == 0x00);
    /* Row 2, col 1-3 */
    assert(buf[16 + 1] == 0x42);
    assert(buf[16 + 3] == 0x42);
    /* Row 3 untouched */
    assert(buf[24 + 1] == 0x00);
}

static void test_fill_4(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    /* 8 pixel-wide bitmap = 4 bytes per row in 4bpp */
    uint8_t buf[16];
    memset(buf, 0, sizeof(buf));

    /* Fill a 4x2 rect starting at pixel (2, 0) with color 0x05 */
    DM2_V1_BlitRect rect = { 2, 0, 4, 2 };
    /* stride_pixels=7, MK_EVEN(7+1)=8, so 4 bytes per row */
    dm2_v1_blit_fill_4(&st, buf, 0x05, 7, &rect);

    /* After fill, pixels 2-5 in rows 0-1 should have value 5 in their nibbles */
    /* Row 0: byte 1 = pixels 2,3 -> 0x55, byte 2 = pixels 4,5 -> 0x55 */
    assert(buf[1] == 0x55);
    assert(buf[2] == 0x55);
    /* Row 1: same pattern */
    assert(buf[4 + 1] == 0x55);
    assert(buf[4 + 2] == 0x55);
}

static void test_blit_88_plain(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t src[16] = { 0x10, 0x11, 0x12, 0x13,
                        0x14, 0x15, 0x16, 0x17,
                        0x18, 0x19, 0x1A, 0x1B,
                        0x1C, 0x1D, 0x1E, 0x1F };
    uint8_t dest[16];
    memset(dest, 0, sizeof(dest));

    DM2_V1_BlitRect rect = { 0, 0, 4, 2 };
    dm2_v1_blit_blitline_88(&st, src, dest, &rect, 0, 0, 4, 4, -1, DM2_V1_BLITMODE_NORMAL);

    assert(dest[0] == 0x10);
    assert(dest[1] == 0x11);
    assert(dest[2] == 0x12);
    assert(dest[3] == 0x13);
    assert(dest[4] == 0x14);
    assert(dest[7] == 0x17);
}

static void test_blit_88_masked(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t src[4] = { 0x00, 0xAA, 0x00, 0xBB };
    uint8_t dest[4] = { 0x11, 0x22, 0x33, 0x44 };

    DM2_V1_BlitRect rect = { 0, 0, 4, 1 };
    /* alphamask=0 means color 0 is transparent */
    dm2_v1_blit_blitline_88(&st, src, dest, &rect, 0, 0, 4, 4, 0, DM2_V1_BLITMODE_NORMAL);

    /* 0x00 pixels should be skipped (transparent), others overwritten */
    assert(dest[0] == 0x11); /* kept: src was 0 (transparent) */
    assert(dest[1] == 0xAA); /* overwritten */
    assert(dest[2] == 0x33); /* kept: src was 0 (transparent) */
    assert(dest[3] == 0xBB); /* overwritten */
}

static void test_blit_88_mirror(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t src[4] = { 0x01, 0x02, 0x03, 0x04 };
    uint8_t dest[4];
    memset(dest, 0, sizeof(dest));

    DM2_V1_BlitRect rect = { 0, 0, 4, 1 };
    dm2_v1_blit_blitline_88(&st, src, dest, &rect, 0, 0, 4, 4, -1, DM2_V1_BLITMODE_HMIRROR);

    /* Horizontal mirror: source read right-to-left */
    assert(dest[0] == 0x04);
    assert(dest[1] == 0x03);
    assert(dest[2] == 0x02);
    assert(dest[3] == 0x01);
}

static void test_blit_88_vmirror(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t src[8] = { 0xAA, 0xAA, 0xAA, 0xAA,
                        0xBB, 0xBB, 0xBB, 0xBB };
    uint8_t dest[8];
    memset(dest, 0, sizeof(dest));

    DM2_V1_BlitRect rect = { 0, 0, 4, 2 };
    dm2_v1_blit_blitline_88(&st, src, dest, &rect, 0, 0, 4, 4, -1, DM2_V1_BLITMODE_VMIRROR);

    /* Vertical mirror: row 1 of src -> row 0 of dest, row 0 -> row 1 */
    assert(dest[0] == 0xBB);
    assert(dest[4] == 0xAA);
}

static void test_blit_88xlat_plain(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    /* Palette: identity + 0x10 */
    uint8_t palette[256];
    for (int i = 0; i < 256; i++)
        palette[i] = (uint8_t)(i + 0x10);

    uint8_t src[4] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t dest[4];
    memset(dest, 0, sizeof(dest));

    DM2_V1_BlitRect rect = { 0, 0, 4, 1 };
    dm2_v1_blit_blitline_88xlat(&st, src, dest, &rect, 0, 0, 4, 4, -1, DM2_V1_BLITMODE_NORMAL, palette);

    assert(dest[0] == 0x10);
    assert(dest[1] == 0x11);
    assert(dest[2] == 0x12);
    assert(dest[3] == 0x13);
}

static void test_blit_44_aligned_plain(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    /* 4 pixels wide = 2 bytes per row in 4bpp */
    uint8_t src[4] = { 0xAB, 0xCD, 0xEF, 0x12 };
    uint8_t dest[4];
    memset(dest, 0, sizeof(dest));

    DM2_V1_BlitRect rect = { 0, 0, 4, 2 };
    dm2_v1_blit_blitline_44(&st, src, dest, &rect, 0, 0, 4, 4, -1, DM2_V1_BLITMODE_NORMAL);

    assert(dest[0] == 0xAB);
    assert(dest[1] == 0xCD);
    assert(dest[2] == 0xEF);
    assert(dest[3] == 0x12);
}

static void test_blit_picture_null_rect(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    /* Should not crash with NULL rect */
    dm2_v1_blit_picture(&st, NULL, NULL, NULL, 0, 0, 4, 4, -1, 0, DM2_V1_BPP_8, DM2_V1_BPP_8, NULL);
}

static void test_blit_picture_88(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t src[4] = { 0x10, 0x20, 0x30, 0x40 };
    uint8_t dest[4];
    memset(dest, 0, sizeof(dest));

    DM2_V1_BlitRect rect = { 0, 0, 4, 1 };
    dm2_v1_blit_picture(&st, src, dest, &rect, 0, 0, 4, 4, -1, DM2_V1_BLITMODE_NORMAL, DM2_V1_BPP_8, DM2_V1_BPP_8, NULL);

    assert(dest[0] == 0x10);
    assert(dest[3] == 0x40);
}

static void test_calc_stretched_size(void)
{
    /* Verify the formula: (eaxw * edxw + edxw / 2) >> 6 */
    int32_t result = dm2_v1_blit_calc_stretched_size(64, 64);
    /* 64*64 = 4096, 64/2 = 32, 4096+32 = 4128, 4128>>6 = 64 */
    assert(result == 64);

    result = dm2_v1_blit_calc_stretched_size(10, 128);
    /* 10*128 = 1280, 128/2 = 64, 1280+64 = 1344, 1344>>6 = 21 */
    assert(result == 21);
}

static void test_fill_zero_rect(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t buf[16];
    memset(buf, 0xCC, sizeof(buf));

    DM2_V1_BlitRect rect = { 0, 0, 0, 0 };
    dm2_v1_blit_fill_8(&st, buf, 0x42, 4, &rect);

    /* Nothing should change */
    assert(buf[0] == 0xCC);
}

static void test_blit_88_zero_size(void)
{
    DM2_V1_BlitterState st;
    dm2_v1_blit_init(&st, NULL, make_test_callbacks());

    uint8_t src[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t dest[4] = { 0x00, 0x00, 0x00, 0x00 };

    DM2_V1_BlitRect rect = { 0, 0, 0, 0 };
    dm2_v1_blit_blitline_88(&st, src, dest, &rect, 0, 0, 4, 4, -1, 0);

    assert(dest[0] == 0x00);
}

static void test_source_evidence(void)
{
    const char *ev = dm2_v1_gfx_blit_source_evidence();
    assert(ev != NULL);
    assert(strstr(ev, "c_gfx_blit.cpp") != NULL);
    assert(strstr(ev, "37 functions") != NULL);
}

/* ========================================================================
 * Main
 * ======================================================================== */

int main(void)
{
    printf("dm2_v1_gfx_blit_pc34_compat tests\n");

    TEST(init);
    TEST(init_null_xblitb);
    TEST(fill_8);
    TEST(fill_4);
    TEST(fill_zero_rect);
    TEST(blit_88_plain);
    TEST(blit_88_masked);
    TEST(blit_88_mirror);
    TEST(blit_88_vmirror);
    TEST(blit_88_zero_size);
    TEST(blit_88xlat_plain);
    TEST(blit_44_aligned_plain);
    TEST(blit_picture_null_rect);
    TEST(blit_picture_88);
    TEST(calc_stretched_size);
    TEST(source_evidence);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
