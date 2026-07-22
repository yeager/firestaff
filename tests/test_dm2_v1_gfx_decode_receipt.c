/*
 * test_dm2_v1_gfx_decode_receipt.c
 *
 * Focused synthetic-data coverage for the SKULLWIN/c_gfx_decode.cpp source-named
 * decode receipts added in dm2_v1_gfx_decode_receipt.c.
 */

#include "dm2_v1_asset_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void test_read_nibble(void)
{
    static const uint8_t raw[] = { 0x12, 0x34 };
    size_t cursor = 0u;
    uint8_t n;

    CHECK(dm2_v1_decode_img3_read_nibble(raw, sizeof(raw), &cursor, &n) == 1
          && n == 0x01u, "read_img3_nibble returns high nibble first");
    CHECK(dm2_v1_decode_img3_read_nibble(raw, sizeof(raw), &cursor, &n) == 1
          && n == 0x02u, "read_img3_nibble returns low nibble second");
    CHECK(dm2_v1_decode_img3_read_nibble(raw, sizeof(raw), &cursor, &n) == 1
          && n == 0x03u, "read_img3_nibble crosses bytes");
    CHECK(dm2_v1_decode_img3_read_nibble(raw, sizeof(raw), &cursor, &n) == 1
          && n == 0x04u, "read_img3_nibble reaches final nibble");
    CHECK(dm2_v1_decode_img3_read_nibble(raw, sizeof(raw), &cursor, &n) == 0,
          "read_img3_nibble fails closed past end");
}

static void test_read_duration(void)
{
    /* nibble 0 -> duration 2 */
    static const uint8_t d0[] = { 0x00 };
    size_t c0 = 0u;
    int d = -1;
    CHECK(dm2_v1_decode_img3_read_duration(d0, sizeof(d0), &c0, &d) == 1
          && d == 2 && c0 == 1u, "duration 0 encodes run length 2");

    /* nibble 0xF + 0x00 -> duration 0x11 (17) */
    static const uint8_t d1[] = { 0xF0, 0x00 };
    size_t c1 = 0u;
    CHECK(dm2_v1_decode_img3_read_duration(d1, sizeof(d1), &c1, &d) == 1
          && d == 17 && c1 == 3u, "0xF0 extended duration encodes 17");

    /* nibble 0xF + 0xFF + 0x1234 -> raw value 0x1234 */
    static const uint8_t d2[] = { 0xFF, 0xF1, 0x23, 0x40 };
    size_t c2 = 0u;
    CHECK(dm2_v1_decode_img3_read_duration(d2, sizeof(d2), &c2, &d) == 1
          && d == 0x1234 && c2 == 7u, "full extended duration decodes");
}

static void test_func_44c8_1202(void)
{
    uint8_t dest[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    dm2_v1_decode_img3_func_44c8_1202(dest, 4, 0, 0x5);
    CHECK(dest[0] == 0x05, "func_44c8_1202 writes first pixel");
    dm2_v1_decode_img3_func_44c8_1202(dest, 4, 3, 0xE);
    CHECK(dest[3] == 0x0E, "func_44c8_1202 writes last pixel");
    dm2_v1_decode_img3_func_44c8_1202(dest, 4, 4, 0x1);
    CHECK(dest[0] == 0x05, "func_44c8_1202 ignores out-of-bounds offset");
}

static void test_spill_pixels(void)
{
    uint8_t dest[6] = { 1, 2, 3, 0, 0, 0 };
    CHECK(dm2_v1_decode_img3_spill_pixels(dest, 6, 3, 0, 3) == 1,
          "spill_pixels returns success");
    CHECK(dest[3] == 1 && dest[4] == 2 && dest[5] == 3,
          "spill_pixels copies previous-line pixels");
    CHECK(dm2_v1_decode_img3_spill_pixels(dest, 6, 3, 4, 3) == 0,
          "spill_pixels rejects out-of-bounds source");
}

static void test_transparent_pixels(void)
{
    uint8_t dest[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t underlay[4] = { 0x10, 0x20, 0x30, 0x40 };
    CHECK(dm2_v1_decode_img3_transparent_pixels(dest, underlay, 4, 1, 2) == 1,
          "transparent_pixels returns success");
    CHECK(dest[1] == 0x20 && dest[2] == 0x30,
          "transparent_pixels copies underlay slice");
    CHECK(dest[0] == 0xFF && dest[3] == 0xFF,
          "transparent_pixels leaves surrounding pixels");
}

static void test_overlay_decode(void)
{
    /* 2x1 IMG3 overlay: palette[0]=1, command idx=0|umask=8, duration=0 -> run 2.
     * Firestaff starts reading palette nibbles at nibble 8 (byte 4). */
    static const uint8_t raw[] = {
        0x02, 0x00, /* width = 2 */
        0x01, 0x00, /* height = 1 */
        0x12, 0x34, /* palette nibbles 1,2,3,4 (also header bpp/unknown) */
        0x58, 0x00, /* palette[4]=5, command=0x8, duration=0 */
        0x00, 0x00  /* header size padding */
    };
    uint8_t underlay[2] = { 0xAB, 0xCD };
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
    uint8_t *out;

    out = dm2_v1_decode_img3_overlay(raw, sizeof(raw), underlay, 2,
                                     2, 1, &fmt);
    CHECK(out != NULL, "decode_img3_overlay returns a buffer");
    CHECK(fmt == DM2_IMG_FMT_IMG3, "decode_img3_overlay sets IMG3 format");
    if (out) {
        CHECK(out[0] == 0x01 && out[1] == 0x01,
              "decode_img3_overlay fills run from palette");
        free(out);
    }

    CHECK(dm2_v1_decode_img3_overlay(NULL, 0, underlay, 2, 2, 1, NULL) == NULL,
          "decode_img3_overlay rejects null raw");
}

static void test_img9_mode2(void)
{
    /* Header: width=2, height=1, mode=2; flags all ones + two literals */
    static const uint8_t raw[] = {
        0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
        0xFF, 0xAB, 0xCD
    };
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
    uint8_t *out;

    out = dm2_v1_decode_img9_mode2(raw, sizeof(raw), 2, 1, &fmt);
    CHECK(out != NULL, "decode_img9 mode2 returns a buffer");
    CHECK(fmt == DM2_IMG_FMT_IMG9, "decode_img9 mode2 sets IMG9 format");
    if (out) {
        CHECK(out[0] == 0xAB && out[1] == 0xCD,
              "decode_img9 mode2 decodes literal bytes");
        free(out);
    }
}

static void test_img9_mode3(void)
{
    /* Header: width=2, height=1, mode=3; flags all ones + two literals */
    static const uint8_t raw[] = {
        0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00,
        0xFF, 0xAB, 0xCD
    };
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
    uint8_t *out;

    out = dm2_v1_decode_img9_mode3(raw, sizeof(raw), 2, 1, &fmt);
    CHECK(out != NULL, "decode_img9 mode3 returns a buffer");
    CHECK(fmt == DM2_IMG_FMT_IMG9, "decode_img9 mode3 sets IMG9 format");
    if (out) {
        CHECK(out[0] == 0xAB && out[1] == 0xCD,
              "decode_img9 mode3 decodes literal bytes");
        free(out);
    }
}

static void test_img9_mode1(void)
{
    /* Minimal mode-1 stream: literal 0x00, clear code 0x100, EOF.
     * 9-bit codes packed LSB-first:
     *   code0 = 0x000 -> bits 0..8 = 0
     *   code1 = 0x100 -> bit8=1, bits0..7=0
     * Stream bytes (3 bytes, 18 bits):
     *   byte0 bits0..7 = bits0..7 of code0 = 0
     *   byte1 bit0 = bit8 of code0 = 0; bits1..7 = bits0..6 of code1 = 0
     *   byte2 bits0..1 = bits7..8 of code1 = 01b -> byte2 = 0x01
     */
    static const uint8_t raw[] = {
        0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x02
    };
    DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
    uint8_t *out;

    out = dm2_v1_decode_img9_mode1(raw, sizeof(raw), 1, 1, &fmt);
    CHECK(out != NULL, "decode_img9 mode1 returns a buffer");
    CHECK(fmt == DM2_IMG_FMT_IMG9, "decode_img9 mode1 sets IMG9 format");
    if (out) {
        CHECK(out[0] == 0x00, "decode_img9 mode1 decodes literal 0x00");
        free(out);
    }

    CHECK(dm2_v1_decode_img9_mode1(raw, sizeof(raw), 1, 1, NULL) != NULL,
          "decode_img9 mode1 accepts null format pointer");
}

static void test_img9_dispatch(void)
{
    static const uint8_t raw2[] = {
        0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
        0xFF, 0xAB, 0xCD
    };
    uint8_t *out;

    out = dm2_v1_decode_img9(raw2, sizeof(raw2), 2, 1, NULL);
    CHECK(out != NULL && out[0] == 0xAB && out[1] == 0xCD,
          "decode_img9 dispatches to mode 2");
    if (out) free(out);

    CHECK(dm2_v1_decode_img9(raw2, sizeof(raw2), 2, 1, NULL) != NULL,
          "decode_img9 accepts null format pointer");
    CHECK(dm2_v1_decode_img9(NULL, 0, 2, 1, NULL) == NULL,
          "decode_img9 rejects null raw");
}

static void test_lifecycle_noop(void)
{
    /* These simply document the platform boundary. */
    dm2_v1_decode_img3_init();
    dm2_v1_decode_img3_alloc();
    CHECK(1, "decode_img3 init/alloc lifecycle receipts are callable");
}

int main(void)
{
    printf("=== DM2 V1 c_gfx_decode receipt test ===\n");
    test_read_nibble();
    test_read_duration();
    test_func_44c8_1202();
    test_spill_pixels();
    test_transparent_pixels();
    test_overlay_decode();
    test_img9_mode2();
    test_img9_mode3();
    test_img9_mode1();
    test_img9_dispatch();
    test_lifecycle_noop();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
