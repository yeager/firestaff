#include "dm2_v1_fire_blit_rows.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char* label) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        failures++;
    }
}

static unsigned get_ref_4(const unsigned char* bytes, unsigned pixel) {
    unsigned packed = bytes[pixel >> 1];
    return (pixel & 1U) ? (packed & 0x0fU) : (packed >> 4);
}

static void set_ref_4(unsigned char* bytes, unsigned pixel, unsigned value) {
    if (pixel & 1U) {
        bytes[pixel >> 1] = (unsigned char)((bytes[pixel >> 1] & 0xf0U) |
                                            (value & 0x0fU));
    } else {
        bytes[pixel >> 1] = (unsigned char)((bytes[pixel >> 1] & 0x0fU) |
                                            ((value & 0x0fU) << 4));
    }
}

static void ref_4to4(const unsigned char* src,
                     unsigned src_off,
                     unsigned char* dst,
                     unsigned dst_off,
                     unsigned width) {
    unsigned i;
    for (i = 0; i < width; ++i) {
        set_ref_4(dst, dst_off + i, get_ref_4(src, src_off + i));
    }
}

static void test_4to4_alignment_matrix(void) {
    static const unsigned char src[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab};
    unsigned src_off;
    unsigned dst_off;
    unsigned width;
    for (src_off = 0; src_off < 4; ++src_off) {
        for (dst_off = 0; dst_off < 4; ++dst_off) {
            for (width = 1; width < 10; ++width) {
                unsigned char got[8];
                unsigned char want[8];
                DM2_V1_FireBlitRowsReceipt receipt;
                memset(got, 0xcc, sizeof(got));
                memset(want, 0xcc, sizeof(want));
                ref_4to4(src, src_off, want, dst_off, width);
                expect_true(dm2_v1_fire_blit_to_memory_row_4to4bpp(
                                src, sizeof(src), (uint16_t)src_off,
                                got, sizeof(got), (uint16_t)dst_off,
                                (uint16_t)width, &receipt),
                            "4to4 accepts bounded row");
                expect_true(memcmp(got, want, sizeof(got)) == 0,
                            "4to4 matches skproject nibble order");
                expect_true(receipt.handled && receipt.source_locked &&
                                receipt.bounds_ok,
                            "4to4 receipt source locked");
                expect_true(receipt.copied_pixels == (int)width,
                            "4to4 copied count");
            }
        }
    }
}

static void test_4to8_palette_and_key(void) {
    static const unsigned char src[] = {0x12, 0x30, 0x45, 0x67};
    unsigned char palette[16];
    unsigned char got[10];
    DM2_V1_FireBlitRowsReceipt receipt;
    unsigned i;
    for (i = 0; i < 16; ++i) {
        palette[i] = (unsigned char)(0xa0U + i);
    }
    memset(got, 0xee, sizeof(got));
    expect_true(dm2_v1_fire_blit_to_memory_row_4to8bpp(
                    src, sizeof(src), 1, got, sizeof(got), 2, 6,
                    palette, 3, &receipt),
                "4to8 accepts bounded row");
    expect_true(got[0] == 0xee && got[1] == 0xee, "4to8 preserves prefix");
    expect_true(got[2] == 0xa2, "4to8 first odd source nibble");
    expect_true(got[3] == 0xee, "4to8 colorkey skip preserves dest");
    expect_true(got[4] == 0xa0 && got[5] == 0xa4 &&
                    got[6] == 0xa5 && got[7] == 0xa6,
                "4to8 palette expansion");
    expect_true(receipt.copied_pixels == 5 &&
                    receipt.skipped_colorkey_pixels == 1,
                "4to8 receipt counts copied and skipped");
}

static void test_bounds_fail_closed(void) {
    unsigned char src[1] = {0x12};
    unsigned char dst[1] = {0x34};
    unsigned char pal[16] = {0};
    DM2_V1_FireBlitRowsReceipt receipt;

    expect_true(!dm2_v1_fire_blit_to_memory_row_4to4bpp(
                    src, sizeof(src), 1, dst, sizeof(dst), 0, 3, &receipt),
                "4to4 rejects source overrun");
    expect_true(receipt.handled && !receipt.bounds_ok,
                "4to4 overrun receipt fail-closed");
    expect_true(dst[0] == 0x34, "4to4 overrun preserves destination");

    expect_true(!dm2_v1_fire_blit_to_memory_row_4to8bpp(
                    src, sizeof(src), 0, dst, sizeof(dst), 0, 2, pal, 15,
                    &receipt),
                "4to8 rejects dest overrun");
    expect_true(receipt.handled && !receipt.bounds_ok,
                "4to8 overrun receipt fail-closed");
}

static void test_stretch_4to4_nearest_neighbor(void) {
    unsigned char src[2] = {0x12, 0x34};
    unsigned char got[8];
    unsigned char want[8];
    DM2_V1_FireBlitRowsReceipt receipt;
    unsigned x;
    unsigned y;

    memset(got, 0xee, sizeof(got));
    memset(want, 0xee, sizeof(want));
    for (y = 0; y < 3; ++y) {
        unsigned sy = (y * 2U) / 3U;
        for (x = 0; x < 4; ++x) {
            unsigned sx = (x * 2U) / 4U;
            set_ref_4(want, y * 4U + x, get_ref_4(src, sy * 2U + sx));
        }
    }

    expect_true(dm2_v1_fire_stretch_blit_to_memory_4to4bpp(
                    src, sizeof(src), 0, 2, 2, 2,
                    got, sizeof(got), 0, 4, 4, 3, &receipt),
                "stretch 4to4 accepts bounded rectangle");
    expect_true(memcmp(got, want, sizeof(got)) == 0,
                "stretch 4to4 uses source-order nearest neighbor nibbles");
    expect_true(receipt.handled && receipt.source_locked &&
                    receipt.bounds_ok &&
                    receipt.copied_pixels == 12,
                "stretch 4to4 receipt records copied pixels");
    expect_true(strcmp(receipt.symbol,
                       "FIRE_STRETCH_BLIT_TO_MEMORY_4TO4BPP") == 0,
                "stretch 4to4 receipt names skproject symbol");
}

static void test_stretch_bounds_fail_closed(void) {
    unsigned char src[1] = {0x12};
    unsigned char dst[1] = {0x34};
    DM2_V1_FireBlitRowsReceipt receipt;

    expect_true(!dm2_v1_fire_stretch_blit_to_memory_4to4bpp(
                    src, sizeof(src), 0, 2, 2, 1,
                    dst, sizeof(dst), 0, 3, 3, 1, &receipt),
                "stretch 4to4 rejects destination overrun");
    expect_true(receipt.handled && !receipt.bounds_ok,
                "stretch 4to4 overrun receipt fail-closed");
    expect_true(dst[0] == 0x34, "stretch 4to4 overrun preserves destination");
}

static void test_ibmio_palette_and_row(void) {
    static const unsigned char raw_pal[17] = {
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0xff
    };
    static const unsigned char src[] = {0x98, 0x76, 0x54};
    unsigned char palette[16];
    unsigned char got[8];
    DM2_V1_FireBlitRowsReceipt receipt;

    memset(palette, 0, sizeof(palette));
    expect_true(dm2_v1_ibmio_load_4to8bpp_pal(
                    raw_pal, sizeof(raw_pal), palette, &receipt),
                "IBMIO_LOAD_4TO8BPP_PAL copies first 16 bytes");
    expect_true(memcmp(palette, raw_pal, 16) == 0,
                "IBMIO palette preserves byte order");
    expect_true(receipt.copied_pixels == 16 &&
                    strcmp(receipt.symbol, "IBMIO_LOAD_4TO8BPP_PAL") == 0,
                "IBMIO palette receipt names source symbol");

    memset(got, 0xee, sizeof(got));
    expect_true(dm2_v1_ibmio_blit_row_4to8bpp(
                    src, sizeof(src), 1, got, sizeof(got), 1, 4,
                    palette, &receipt),
                "IBMIO_BLIT_ROW_4TO8BPP accepts bounded row");
    expect_true(got[0] == 0xee && got[1] == 0x18 && got[2] == 0x17 &&
                    got[3] == 0x16 && got[4] == 0x15 && got[5] == 0xee,
                "IBMIO row expands odd source nibbles through palette");
    expect_true(receipt.copied_pixels == 4 &&
                    strcmp(receipt.symbol, "IBMIO_BLIT_ROW_4TO8BPP") == 0,
                "IBMIO row receipt names source symbol");
}

int main(void) {
    test_4to4_alignment_matrix();
    test_4to8_palette_and_key();
    test_stretch_4to4_nearest_neighbor();
    test_stretch_bounds_fail_closed();
    test_ibmio_palette_and_row();
    test_bounds_fail_closed();
    expect_true(strstr(dm2_v1_fire_blit_rows_source_evidence(),
                       "SKWIN/SkWinCore.cpp") != 0,
                "source evidence names skproject file");
    if (failures) {
        return 1;
    }
    puts("DM2 fire blit rows: ok");
    return 0;
}
