/*
 * test_dm2_v1_gfx_pixel_pc34_compat.c -- Tests for DM2 pixel type operations.
 *
 * Covers: pixel conversion, comparison, pixel16 nibble ops,
 * build_pixels16, build_pixels_masked16, source evidence.
 */

#include "dm2_v1_gfx_pixel_pc34_compat.h"
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

static void test_ui8_to_pixel_roundtrip(void)
{
    DM2_V1_Pixel px = dm2_v1_ui8_to_pixel(0x42);
    uint8_t val = dm2_v1_pixel_to_ui8(px);
    assert(val == 0x42);
}

static void test_pixel_eq(void)
{
    DM2_V1_Pixel a = dm2_v1_ui8_to_pixel(5);
    DM2_V1_Pixel b = dm2_v1_ui8_to_pixel(5);
    DM2_V1_Pixel c = dm2_v1_ui8_to_pixel(7);

    assert(dm2_v1_pixel_eq(a, b) == 1);
    assert(dm2_v1_pixel_ne(a, c) == 1);
    assert(dm2_v1_pixel_eq(a, c) == 0);
}

static void test_pixel_eq_color(void)
{
    DM2_V1_Pixel px = dm2_v1_ui8_to_pixel(0x05);
    assert(dm2_v1_pixel_eq_color(px, DM2_V1_E_COL05) == 1);
    assert(dm2_v1_pixel_ne_color(px, DM2_V1_E_COL00) == 1);
}

static void test_pixel_mkidx(void)
{
    DM2_V1_Pixel px = dm2_v1_ui8_to_pixel(0x0A);
    assert(dm2_v1_pixel_mkidx(px) == 0x0A);
}

static void test_pixel_is(void)
{
    DM2_V1_Pixel px = dm2_v1_ui8_to_pixel(0x0F);
    assert(dm2_v1_pixel_is(px, DM2_V1_E_COL15) == 1);
    assert(dm2_v1_pixel_is(px, DM2_V1_E_COL00) == 0);
}

static void test_pixel16_ltor(void)
{
    DM2_V1_Pixel px = dm2_v1_ui8_to_pixel(0x05);
    DM2_V1_Pixel16 p16 = dm2_v1_pixel16_ltor(px);
    assert(p16.p == 0x50);
}

static void test_pixel16_rtol(void)
{
    DM2_V1_Pixel px = dm2_v1_ui8_to_pixel(0x0A);
    DM2_V1_Pixel16 p16 = dm2_v1_pixel16_rtol(px);
    assert(p16.p == 0x0A);
}

static void test_pixel16_getl(void)
{
    DM2_V1_Pixel16 p16;
    p16.p = 0xA5;
    DM2_V1_Pixel px = dm2_v1_pixel16_getl(p16);
    assert(dm2_v1_pixel_to_ui8(px) == 0x0A);
}

static void test_pixel16_getr(void)
{
    DM2_V1_Pixel16 p16;
    p16.p = 0xA5;
    DM2_V1_Pixel px = dm2_v1_pixel16_getr(p16);
    assert(dm2_v1_pixel_to_ui8(px) == 0x05);
}

static void test_pixel16_set(void)
{
    DM2_V1_Pixel left = dm2_v1_ui8_to_pixel(0x0A);
    DM2_V1_Pixel right = dm2_v1_ui8_to_pixel(0x05);
    DM2_V1_Pixel16 p16 = dm2_v1_pixel16_set(left, right);
    assert(p16.p == 0xA5);
}

static void test_build_pixels16(void)
{
    DM2_V1_Pixel16 p16 = dm2_v1_build_pixels16(0x0C, 0x03);
    assert(p16.p == 0xC3);
}

static void test_build_pixels_masked16(void)
{
    DM2_V1_Pixel16 src;
    src.p = 0xAB;
    DM2_V1_Pixel16 dst;
    dst.p = 0x12;
    DM2_V1_Pixel16 mask;

    /* mask = 0xFF: take both nibbles from src */
    mask.p = 0xFF;
    DM2_V1_Pixel16 r1 = dm2_v1_build_pixels_masked16(src, dst, mask);
    assert(r1.p == 0xAB);

    /* mask = 0x00: take both nibbles from dst */
    mask.p = 0x00;
    DM2_V1_Pixel16 r2 = dm2_v1_build_pixels_masked16(src, dst, mask);
    assert(r2.p == 0x12);

    /* mask = 0xF0: take left from src, right from dst */
    mask.p = 0xF0;
    DM2_V1_Pixel16 r3 = dm2_v1_build_pixels_masked16(src, dst, mask);
    assert(r3.p == 0xA2);

    /* mask = 0x0F: take left from dst, right from src */
    mask.p = 0x0F;
    DM2_V1_Pixel16 r4 = dm2_v1_build_pixels_masked16(src, dst, mask);
    assert(r4.p == 0x1B);
}

static void test_e_color_values(void)
{
    assert(DM2_V1_E_COL00 == 0x00);
    assert(DM2_V1_E_COL15 == 0x0F);
    assert(DM2_V1_E_COLX90 == 0x90);
}

static void test_source_evidence(void)
{
    const char *ev = dm2_v1_gfx_pixel_source_evidence();
    assert(ev != NULL);
    assert(strstr(ev, "c_gfx_pixel.cpp") != NULL);
    assert(strstr(ev, "5 functions") != NULL);
}

/* ========================================================================
 * Main
 * ======================================================================== */

int main(void)
{
    printf("dm2_v1_gfx_pixel_pc34_compat tests\n");

    TEST(ui8_to_pixel_roundtrip);
    TEST(pixel_eq);
    TEST(pixel_eq_color);
    TEST(pixel_mkidx);
    TEST(pixel_is);
    TEST(pixel16_ltor);
    TEST(pixel16_rtol);
    TEST(pixel16_getl);
    TEST(pixel16_getr);
    TEST(pixel16_set);
    TEST(build_pixels16);
    TEST(build_pixels_masked16);
    TEST(e_color_values);
    TEST(source_evidence);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
