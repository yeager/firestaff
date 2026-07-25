#include "dm1_v1_blit_fill_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static DM1_V1_BlitFramebufferPc34 make_fb(uint8_t *buf, int w, int h)
{
    DM1_V1_BlitFramebufferPc34 fb;
    fb.pixels = buf;
    fb.width = (uint16_t)w;
    fb.height = (uint16_t)h;
    fb.pitch = (uint16_t)w;
    return fb;
}

static void test_clear(void)
{
    uint8_t buf[320 * 200];
    memset(buf, 0, sizeof(buf));
    DM1_V1_BlitFramebufferPc34 fb = make_fb(buf, 320, 200);
    DM1_V1_BlitClearPc34Compat(&fb, 5);
    assert(buf[0] == 5);
    assert(buf[320 * 200 - 1] == 5);
}

static void test_fill_rect(void)
{
    uint8_t buf[320 * 200];
    memset(buf, 0, sizeof(buf));
    DM1_V1_BlitFramebufferPc34 fb = make_fb(buf, 320, 200);
    DM1_V1_BlitRectPc34 rect = {10, 10, 20, 20};
    DM1_V1_BlitFillRectPc34Compat(&fb, &rect, 7);
    assert(buf[10 + 10 * 320] == 7);
    assert(buf[29 + 29 * 320] == 7);
    assert(buf[0] == 0);
}

static void test_fill_rect_clipped(void)
{
    uint8_t buf[320 * 200];
    memset(buf, 0, sizeof(buf));
    DM1_V1_BlitFramebufferPc34 fb = make_fb(buf, 320, 200);
    DM1_V1_BlitRectPc34 rect = {300, 180, 100, 100};
    DM1_V1_BlitFillRectPc34Compat(&fb, &rect, 3);
}

static void test_hline(void)
{
    uint8_t buf[320 * 200];
    memset(buf, 0, sizeof(buf));
    DM1_V1_BlitFramebufferPc34 fb = make_fb(buf, 320, 200);
    DM1_V1_BlitHLinePc34Compat(&fb, 10, 30, 50, 9);
    assert(buf[10 + 50 * 320] == 9);
    assert(buf[30 + 50 * 320] == 9);
    assert(buf[9 + 50 * 320] == 0);
}

static void test_vline(void)
{
    uint8_t buf[320 * 200];
    memset(buf, 0, sizeof(buf));
    DM1_V1_BlitFramebufferPc34 fb = make_fb(buf, 320, 200);
    DM1_V1_BlitVLinePc34Compat(&fb, 50, 10, 30, 11);
    assert(buf[50 + 10 * 320] == 11);
    assert(buf[50 + 30 * 320] == 11);
    assert(buf[50 + 9 * 320] == 0);
}

static void test_blit_copy(void)
{
    uint8_t buf[320 * 200];
    memset(buf, 0, sizeof(buf));
    DM1_V1_BlitFramebufferPc34 fb = make_fb(buf, 320, 200);

    uint8_t src_data[4] = {1, 2, 3, 4};
    DM1_V1_BlitSourcePc34 src;
    src.data = src_data;
    src.width = 2;
    src.height = 2;
    src.byte_width = 2;
    src.bitplanes = 1;
    src.trans_color = 0;

    DM1_V1_BlitPc34Compat(&fb, &src, 0, 0, DM1_BF_BLIT_COPY);
}

static void test_copy_region(void)
{
    uint8_t src_buf[320 * 200];
    uint8_t dst_buf[320 * 200];
    memset(src_buf, 42, sizeof(src_buf));
    memset(dst_buf, 0, sizeof(dst_buf));
    DM1_V1_BlitFramebufferPc34 src_fb = make_fb(src_buf, 320, 200);
    DM1_V1_BlitFramebufferPc34 dst_fb = make_fb(dst_buf, 320, 200);
    DM1_V1_BlitRectPc34 region = {0, 0, 10, 10};
    DM1_V1_BlitCopyRegionPc34Compat(&dst_fb, &src_fb, &region);
    assert(dst_buf[0] == 42);
}

static void test_constants(void)
{
    assert(DM1_BF_SCREEN_W == 320);
    assert(DM1_BF_SCREEN_H == 200);
    assert(DM1_BF_BYTE_WIDTH == 160);
    assert(DM1_BF_BITPLANES == 4);
    assert(DM1_BF_PIXEL_LINE == 160);
    assert(DM1_BF_BLIT_COPY == 0x00);
    assert(DM1_BF_BLIT_TRANS == 0x01);
    assert(DM1_BF_BLIT_XOR == 0x02);
    assert(DM1_BF_BLIT_FLIP_H == 0x04);
    assert(DM1_BF_BLIT_FLIP_V == 0x08);
}

int main(void)
{
    test_clear();
    test_fill_rect();
    test_fill_rect_clipped();
    test_hline();
    test_vline();
    test_blit_copy();
    test_copy_region();
    test_constants();

    puts("ok: DM1 blit fill primitives (Q-DM1-03) 8 tests passed");
    return 0;
}
