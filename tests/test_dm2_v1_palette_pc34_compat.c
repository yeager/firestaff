#include "dm2_v1_palette_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_palette_size(void)
{
    assert(DM2_V1_PAL_SIZE == 768);
    assert(DM2_V1_PAL_ENTRIES == 256);
    printf("test_palette_size OK\n");
}

static void test_palette_entry0_black(void)
{
    assert(dm2_v1_default_palette[0] == 0);
    assert(dm2_v1_default_palette[1] == 0);
    assert(dm2_v1_default_palette[2] == 0);
    printf("test_palette_entry0_black OK\n");
}

static void test_palette_ramp0_entry1(void)
{
    /* Ramp 0 entry 1: R=3, G=2, B=0 */
    assert(dm2_v1_default_palette[3] == 0x03);
    assert(dm2_v1_default_palette[4] == 0x02);
    assert(dm2_v1_default_palette[5] == 0x00);
    printf("test_palette_ramp0_entry1 OK\n");
}

static void test_palette_ramp0_last(void)
{
    /* Ramp 0 entry 15: R=0x3b, G=0x2d, B=0x12 */
    assert(dm2_v1_default_palette[15 * 3 + 0] == 0x3b);
    assert(dm2_v1_default_palette[15 * 3 + 1] == 0x2d);
    assert(dm2_v1_default_palette[15 * 3 + 2] == 0x12);
    printf("test_palette_ramp0_last OK\n");
}

static void test_palette_ramp15_last_white(void)
{
    /* Ramp 15 entry 15 (entry 255): R=G=B=0x3f */
    assert(dm2_v1_default_palette[255 * 3 + 0] == 0x3f);
    assert(dm2_v1_default_palette[255 * 3 + 1] == 0x3f);
    assert(dm2_v1_default_palette[255 * 3 + 2] == 0x3f);
    printf("test_palette_ramp15_last_white OK\n");
}

static void test_palette_ramp2_pure_blue(void)
{
    /* Ramp 2 (entries 32-47): pure blue, R=G=0 */
    for (int i = 1; i < 16; i++) {
        int idx = (32 + i) * 3;
        assert(dm2_v1_default_palette[idx + 0] == 0);
        assert(dm2_v1_default_palette[idx + 1] == 0);
        assert(dm2_v1_default_palette[idx + 2] != 0);
    }
    printf("test_palette_ramp2_pure_blue OK\n");
}

static void test_palette_ramp11_pure_red(void)
{
    /* Ramp 11 (entries 176-191): pure red, G=B=0 */
    for (int i = 1; i < 16; i++) {
        int idx = (176 + i) * 3;
        assert(dm2_v1_default_palette[idx + 0] != 0);
        assert(dm2_v1_default_palette[idx + 1] == 0);
        assert(dm2_v1_default_palette[idx + 2] == 0);
    }
    printf("test_palette_ramp11_pure_red OK\n");
}

static void test_convert_driver_palette(void)
{
    uint8_t src[4 * 4] = {
        0xFF, 0xFC, 0x80, 0x40, /* A=0xFF, R=0xFC, G=0x80, B=0x40 */
        0x00, 0x00, 0x00, 0x00,
        0x00, 0xFF, 0xFF, 0xFF,
        0x00, 0x04, 0x08, 0x0C
    };
    int8_t dst[3 * 4];
    memset(dst, 0x7F, sizeof(dst));

    /* Only convert 4 entries — pad src to 256 is not needed for logic test,
     * but the function always processes 256 entries. Use a full buffer. */
    uint8_t full_src[1024];
    int8_t full_dst[768];
    memset(full_src, 0, sizeof(full_src));
    memcpy(full_src, src, sizeof(src));

    dm2_v1_convert_driver_palette(full_src, full_dst);

    /* Entry 0: R=0xFC>>2=0x3F, G=0x80>>2=0x20, B=0x40>>2=0x10 */
    assert(full_dst[0] == 0x3F);
    assert(full_dst[1] == 0x20);
    assert(full_dst[2] == 0x10);
    /* Entry 1: all zero */
    assert(full_dst[3] == 0x00);
    assert(full_dst[4] == 0x00);
    assert(full_dst[5] == 0x00);
    /* Entry 2: R=0xFF>>2=0x3F, G=0xFF>>2=0x3F, B=0xFF>>2=0x3F */
    assert(full_dst[6] == 0x3F);
    assert(full_dst[7] == 0x3F);
    assert(full_dst[8] == 0x3F);
    printf("test_convert_driver_palette OK\n");
}

static void test_expand_6to8(void)
{
    int8_t pal6[3] = {0, 0x3F, 0x20};
    uint8_t pal8[3];

    dm2_v1_expand_palette_6to8(pal6, pal8);

    assert(pal8[0] == 0);
    assert(pal8[1] == 255);
    /* 0x20=32: (32*255+31)/63 = 8191/63 = 130 */
    assert(pal8[2] == 130);
    printf("test_expand_6to8 OK\n");
}

static void test_expand_roundtrip(void)
{
    /* Expand the default palette and verify entry 255 = white (255,255,255) */
    uint8_t pal8[DM2_V1_PAL_SIZE];
    dm2_v1_expand_palette_6to8(dm2_v1_default_palette, pal8);

    assert(pal8[255 * 3 + 0] == 255);
    assert(pal8[255 * 3 + 1] == 255);
    assert(pal8[255 * 3 + 2] == 255);
    assert(pal8[0] == 0);
    assert(pal8[1] == 0);
    assert(pal8[2] == 0);
    printf("test_expand_roundtrip OK\n");
}

static void test_all_ramps_start_black(void)
{
    for (int ramp = 0; ramp < 16; ramp++) {
        int idx = ramp * 16 * 3;
        assert(dm2_v1_default_palette[idx + 0] == 0);
        assert(dm2_v1_default_palette[idx + 1] == 0);
        assert(dm2_v1_default_palette[idx + 2] == 0);
    }
    printf("test_all_ramps_start_black OK\n");
}

int main(void)
{
    test_palette_size();
    test_palette_entry0_black();
    test_palette_ramp0_entry1();
    test_palette_ramp0_last();
    test_palette_ramp15_last_white();
    test_palette_ramp2_pure_blue();
    test_palette_ramp11_pure_red();
    test_convert_driver_palette();
    test_expand_6to8();
    test_expand_roundtrip();
    test_all_ramps_start_black();
    printf("All dm2_v1_palette tests passed.\n");
    return 0;
}
