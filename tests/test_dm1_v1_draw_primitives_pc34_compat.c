#include "dm1_v1_draw_primitives_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint8_t s_buf[320 * 200];
static DM1_V1_BitmapPc34 s_bmp;

static void setup(void)
{
    memset(s_buf, 0, sizeof(s_buf));
    DM1_V1_Draw_InitBitmapPc34Compat(&s_bmp, s_buf, 320, 200, 0);
}

static void test_init_bitmap(void)
{
    setup();
    assert(s_bmp.width == 320);
    assert(s_bmp.height == 200);
    assert(s_bmp.stride == 320);
    assert(s_bmp.pixels == s_buf);
}

static void test_clear(void)
{
    setup();
    DM1_V1_Draw_ClearPc34Compat(&s_bmp, 9);
    assert(s_buf[0] == 9);
    assert(s_buf[320 * 200 - 1] == 9);
}

static void test_pixel_set_get(void)
{
    setup();
    DM1_V1_Draw_PixelPc34Compat(&s_bmp, 100, 50, 7);
    uint8_t v = DM1_V1_Draw_GetPixelPc34Compat(&s_bmp, 100, 50);
    (void)v;
    assert(v == 7);
}

static void test_pixel_oob(void)
{
    setup();
    DM1_V1_Draw_PixelPc34Compat(&s_bmp, -1, -1, 5);
    DM1_V1_Draw_PixelPc34Compat(&s_bmp, 999, 999, 5);
    uint8_t v = DM1_V1_Draw_GetPixelPc34Compat(&s_bmp, -1, -1);
    (void)v;
    assert(v == 0);
}

static void test_hline(void)
{
    setup();
    DM1_V1_Draw_HLinePc34Compat(&s_bmp, 10, 50, 20, 3);
    assert(s_buf[10 + 50 * 320] == 3);
    assert(s_buf[29 + 50 * 320] == 3);
    assert(s_buf[9 + 50 * 320] == 0);
    assert(s_buf[30 + 50 * 320] == 0);
}

static void test_vline(void)
{
    setup();
    DM1_V1_Draw_VLinePc34Compat(&s_bmp, 50, 10, 20, 4);
    assert(s_buf[50 + 10 * 320] == 4);
    assert(s_buf[50 + 29 * 320] == 4);
    assert(s_buf[50 + 9 * 320] == 0);
}

static void test_fill_rect(void)
{
    setup();
    DM1_V1_RectPc34 r = {5, 5, 10, 10};
    DM1_V1_Draw_FillRectPc34Compat(&s_bmp, &r, 6);
    assert(s_buf[5 + 5 * 320] == 6);
    assert(s_buf[14 + 14 * 320] == 6);
    assert(s_buf[4 + 5 * 320] == 0);
}

static void test_rect_outline(void)
{
    setup();
    DM1_V1_RectPc34 r = {10, 10, 20, 15};
    DM1_V1_Draw_RectPc34Compat(&s_bmp, &r, 8);
    assert(s_buf[10 + 10 * 320] == 8);
    assert(s_buf[29 + 10 * 320] == 8);
    assert(s_buf[10 + 24 * 320] == 8);
    assert(s_buf[15 + 15 * 320] == 0);
}

static void test_blit(void)
{
    setup();
    uint8_t src_buf[16];
    memset(src_buf, 11, sizeof(src_buf));
    DM1_V1_BitmapPc34 src;
    DM1_V1_Draw_InitBitmapPc34Compat(&src, src_buf, 4, 4, 0);
    DM1_V1_Draw_BlitPc34Compat(&s_bmp, 0, 0, &src, NULL);
    assert(s_buf[0] == 11);
    assert(s_buf[3] == 11);
    assert(s_buf[4] == 0);
}

static void test_blit_transparent(void)
{
    setup();
    uint8_t src_buf[4] = {0, 5, 0, 5};
    DM1_V1_BitmapPc34 src;
    DM1_V1_Draw_InitBitmapPc34Compat(&src, src_buf, 2, 2, 0);
    DM1_V1_Draw_ClearPc34Compat(&s_bmp, 99);
    DM1_V1_Draw_BlitTransparentPc34Compat(&s_bmp, 0, 0, &src, NULL, 0);
    assert(s_buf[0] == 99);
    assert(s_buf[1] == 5);
}

static void test_flip_h(void)
{
    setup();
    s_buf[0] = 1; s_buf[1] = 2; s_buf[2] = 3; s_buf[3] = 4;
    DM1_V1_RectPc34 r = {0, 0, 4, 1};
    DM1_V1_Draw_FlipHPc34Compat(&s_bmp, &r);
    assert(s_buf[0] == 4);
    assert(s_buf[3] == 1);
}

static void test_flip_v(void)
{
    setup();
    s_buf[0 + 0 * 320] = 1;
    s_buf[0 + 1 * 320] = 2;
    s_buf[0 + 2 * 320] = 3;
    DM1_V1_RectPc34 r = {0, 0, 1, 3};
    DM1_V1_Draw_FlipVPc34Compat(&s_bmp, &r);
    assert(s_buf[0 + 0 * 320] == 3);
    assert(s_buf[0 + 2 * 320] == 1);
}

static void test_darken_color(void)
{
    uint8_t c0 = DM1_V1_Draw_DarkenColorPc34Compat(15, 0);
    uint8_t c3 = DM1_V1_Draw_DarkenColorPc34Compat(15, 3);
    (void)c0; (void)c3;
    assert(c0 == 15);
}

static void test_source_evidence(void)
{
    const char *ev = DM1_V1_Draw_PrimitivesSourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_constants(void)
{
    assert(DM1_V1_DRAW_SCREEN_W_PC34 == 320);
    assert(DM1_V1_DRAW_SCREEN_H_PC34 == 200);
    assert(DM1_V1_DRAW_BPP_PC34 == 1);
}

int main(void)
{
    test_init_bitmap();
    test_clear();
    test_pixel_set_get();
    test_pixel_oob();
    test_hline();
    test_vline();
    test_fill_rect();
    test_rect_outline();
    test_blit();
    test_blit_transparent();
    test_flip_h();
    test_flip_v();
    test_darken_color();
    test_source_evidence();
    test_constants();

    puts("ok: DM1 draw primitives (Q-DM1-03) 15 tests passed");
    return 0;
}
