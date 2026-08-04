/*
 * test_dm2_v1_gfx_bmp_pc34_compat.c -- Tests for DM2 bitmap header module.
 *
 * Covers: get_header negative offset, init zeroing, calc_image_byte_length
 * for 4bpp and 8bpp, source evidence.
 */

#include "dm2_v1_gfx_bmp_pc34_compat.h"
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
 * Tests
 * ======================================================================== */

static void test_get_header(void)
{
    /* Allocate header + pixel data contiguously */
    uint8_t buf[sizeof(DM2_V1_BmpHeader) + 16];
    memset(buf, 0, sizeof(buf));

    DM2_V1_BmpHeader *hdr = (DM2_V1_BmpHeader *)buf;
    hdr->res = DM2_V1_BPP_8;
    hdr->width = 4;
    hdr->height = 4;

    DM2_V1_Bmp *bmp = (DM2_V1_Bmp *)(buf + sizeof(DM2_V1_BmpHeader));

    DM2_V1_BmpHeader *result = dm2_v1_gfx_bmp_get_header(bmp);
    assert(result == hdr);
    assert(result->res == DM2_V1_BPP_8);
    assert(result->width == 4);
    assert(result->height == 4);
}

static void test_init(void)
{
    DM2_V1_Screen256Bmp screen;
    /* Fill with non-zero to verify zeroing */
    memset(&screen, 0xAB, sizeof(screen));

    DM2_V1_GfxBmpInitReceipt receipt = dm2_v1_gfx_bmp_init(&screen);

    assert(receipt.initialized == true);
    assert(receipt.screen_pixel_count == DM2_V1_ORIG_SWIDTH * DM2_V1_ORIG_SHEIGHT);
    assert(screen.header.res == 0);
    assert(screen.header.width == 0);
    assert(screen.header.height == 0);
    assert(screen.pixel[0].p == 0);
    assert(screen.pixel[DM2_V1_ORIG_SWIDTH * DM2_V1_ORIG_SHEIGHT - 1].p == 0);
}

static void test_init_null(void)
{
    DM2_V1_GfxBmpInitReceipt receipt = dm2_v1_gfx_bmp_init(NULL);
    assert(receipt.initialized == false);
    assert(receipt.screen_pixel_count == 0);
}

static void test_calc_length_8bpp(void)
{
    DM2_V1_BmpHeader hdr;
    hdr.res = DM2_V1_BPP_8;
    hdr.unused = 0;
    hdr.width = 10;
    hdr.height = 20;

    DM2_V1_GfxBmpCalcLengthReceipt receipt =
        dm2_v1_gfx_bmp_calc_image_byte_length(&hdr);

    assert(receipt.byte_length == 200); /* 10 * 20 */
    assert(receipt.res == DM2_V1_BPP_8);
    assert(receipt.width == 10);
    assert(receipt.height == 20);
}

static void test_calc_length_4bpp_even(void)
{
    DM2_V1_BmpHeader hdr;
    hdr.res = DM2_V1_BPP_4;
    hdr.unused = 0;
    hdr.width = 10;  /* MK_EVEN(10) = 10, 10 >> 1 = 5 */
    hdr.height = 20;

    DM2_V1_GfxBmpCalcLengthReceipt receipt =
        dm2_v1_gfx_bmp_calc_image_byte_length(&hdr);

    assert(receipt.byte_length == 100); /* 5 * 20 */
}

static void test_calc_length_4bpp_odd(void)
{
    DM2_V1_BmpHeader hdr;
    hdr.res = DM2_V1_BPP_4;
    hdr.unused = 0;
    hdr.width = 7;  /* MK_EVEN(7) = 8, 8 >> 1 = 4 */
    hdr.height = 10;

    DM2_V1_GfxBmpCalcLengthReceipt receipt =
        dm2_v1_gfx_bmp_calc_image_byte_length(&hdr);

    assert(receipt.byte_length == 40); /* 4 * 10 */
}

static void test_calc_length_320x200(void)
{
    DM2_V1_BmpHeader hdr;
    hdr.res = DM2_V1_BPP_8;
    hdr.unused = 0;
    hdr.width = 320;
    hdr.height = 200;

    DM2_V1_GfxBmpCalcLengthReceipt receipt =
        dm2_v1_gfx_bmp_calc_image_byte_length(&hdr);

    assert(receipt.byte_length == 64000); /* 320 * 200 */
}

static void test_mk_even_macro(void)
{
    assert(DM2_V1_MK_EVEN(0) == 0);
    assert(DM2_V1_MK_EVEN(1) == 2);
    assert(DM2_V1_MK_EVEN(2) == 2);
    assert(DM2_V1_MK_EVEN(3) == 4);
    assert(DM2_V1_MK_EVEN(7) == 8);
    assert(DM2_V1_MK_EVEN(10) == 10);
}

static void test_source_evidence(void)
{
    const char *ev = dm2_v1_gfx_bmp_source_evidence();
    assert(ev != NULL);
    assert(strstr(ev, "c_gfx_bmp.cpp") != NULL);
    assert(strstr(ev, "3 functions") != NULL);
}

static void test_header_size(void)
{
    /* The header must be exactly 6 bytes (packed) */
    assert(sizeof(DM2_V1_BmpHeader) == 6);
}

/* ========================================================================
 * Main
 * ======================================================================== */

int main(void)
{
    printf("dm2_v1_gfx_bmp_pc34_compat tests\n");

    TEST(get_header);
    TEST(init);
    TEST(init_null);
    TEST(calc_length_8bpp);
    TEST(calc_length_4bpp_even);
    TEST(calc_length_4bpp_odd);
    TEST(calc_length_320x200);
    TEST(mk_even_macro);
    TEST(header_size);
    TEST(source_evidence);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
