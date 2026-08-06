#include "dm1_v1_fmtowns_egb_shim.h"
#include "dm1_v1_fmtowns_menu_regions.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define FB_W 320
#define FB_H 200

static uint8_t fb[FB_W * FB_H];

static void reset_fb(void) { memset(fb, 0, sizeof(fb)); }

static size_t count_colour(uint8_t colour) {
    size_t n = 0;
    size_t i;
    for (i = 0; i < sizeof(fb); ++i) if (fb[i] == colour) ++n;
    return n;
}

static void test_clip_rect_basic(void) {
    int x1 = 10, y1 = 10, x2 = 20, y2 = 20;
    assert(dm1_v1_fmtowns_egb_clip_rect_pc34(FB_W, FB_H,
                                             &x1, &y1, &x2, &y2) == 1);
    assert(x1 == 10 && y1 == 10 && x2 == 20 && y2 == 20);
}

static void test_clip_rect_off_screen(void) {
    int x1 = -20, y1 = -20, x2 = -1, y2 = -1;
    assert(dm1_v1_fmtowns_egb_clip_rect_pc34(FB_W, FB_H,
                                             &x1, &y1, &x2, &y2) == 0);
    x1 = FB_W; y1 = FB_H; x2 = FB_W + 5; y2 = FB_H + 5;
    assert(dm1_v1_fmtowns_egb_clip_rect_pc34(FB_W, FB_H,
                                             &x1, &y1, &x2, &y2) == 0);
}

static void test_clip_rect_partial(void) {
    /* Rect that runs off the right edge should be clipped. */
    int x1 = 300, y1 = 10, x2 = 400, y2 = 30;
    assert(dm1_v1_fmtowns_egb_clip_rect_pc34(FB_W, FB_H,
                                             &x1, &y1, &x2, &y2) == 1);
    assert(x1 == 300 && x2 == FB_W - 1);
    assert(y1 == 10 && y2 == 30);
}

static void test_clip_rect_swapped(void) {
    /* Callers may pass reversed corners; clip must normalise. */
    int x1 = 20, y1 = 30, x2 = 10, y2 = 15;
    assert(dm1_v1_fmtowns_egb_clip_rect_pc34(FB_W, FB_H,
                                             &x1, &y1, &x2, &y2) == 1);
    assert(x1 == 10 && y1 == 15 && x2 == 20 && y2 == 30);
}

static void test_fill_rect_pixel_count(void) {
    reset_fb();
    /* Inclusive 11x21 rectangle => 11*21 = 231 pixels. */
    size_t n = dm1_v1_fmtowns_egb_fill_rect_pc34(fb, FB_W, FB_H, FB_W,
                                                 10, 20, 20, 40, 0x0B);
    assert(n == 11u * 21u);
    assert(count_colour(0x0B) == n);
}

static void test_fill_rect_menu_panel_region(void) {
    /* Draw region 10 (SPC_BLOT menu panel, 87x45) at the anchor
     * derived from region 11 (319, 77). Verify the byte-verified
     * dimensions produce the expected pixel count and that the
     * panel does not overflow the framebuffer. */
    DM1_V1_FmtownsRegionRecord panel, clear;
    int x1, y1, x2, y2;
    size_t painted;
    assert(dm1_v1_fmtowns_region_menu_panel_pc34(&panel) == 1);
    assert(dm1_v1_fmtowns_region_menu_clear_area_pc34(&clear) == 1);
    /* Panel width x height = 87 x 45 (region 10) */
    /* Anchor (319, 77) is the FM Towns coordinate; when applied to
     * a 320-wide FB the rectangle clips to 1 pixel wide (x=319..319)
     * on the right edge — this is exactly the FM Towns behaviour
     * when the menu is on the right edge of a 320-wide surface.
     * Verify the clip result rather than a raw multiplication. */
    reset_fb();
    x1 = clear.a - panel.a;   /* left = anchor.x - width  */
    y1 = clear.b;             /* top  = anchor.y          */
    x2 = clear.a - 1;         /* right = anchor.x - 1     */
    y2 = clear.b + panel.b - 1;
    painted = dm1_v1_fmtowns_egb_fill_rect_pc34(fb, FB_W, FB_H, FB_W,
                                                x1, y1, x2, y2, 0x0B);
    /* The panel fits entirely on-screen for FB_W=320, FB_H=200:
     * x1 = 319 - 87 = 232, x2 = 318, y1 = 77, y2 = 121.
     * Rectangle is 87 wide x 45 tall = 3915 pixels. */
    assert(x1 == 232 && x2 == 318 && y1 == 77 && y2 == 121);
    assert(painted == 87u * 45u);
    assert(count_colour(0x0B) == painted);
}

static void test_put_block_plain_copy(void) {
    static const uint8_t src[4 * 3] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
    };
    reset_fb();
    size_t n = dm1_v1_fmtowns_egb_put_block_pc34(fb, FB_W, FB_H, FB_W,
                                                 5, 10, src, 4, 3, 4, -1);
    assert(n == 12u);
    /* Verify byte-exact copy. */
    assert(fb[10 * FB_W + 5] == 1);
    assert(fb[10 * FB_W + 8] == 4);
    assert(fb[12 * FB_W + 5] == 9);
    assert(fb[12 * FB_W + 8] == 12);
    /* Nothing written outside the block. */
    assert(fb[9 * FB_W + 5] == 0);
    assert(fb[13 * FB_W + 5] == 0);
    assert(fb[10 * FB_W + 4] == 0);
    assert(fb[10 * FB_W + 9] == 0);
}

static void test_put_block_masked(void) {
    /* Masked copy: zero pixels stay transparent, non-zero become
     * the caller's colour (WRITEMODE 6 emulation). */
    static const uint8_t src[3 * 3] = {
        1, 0, 1,
        0, 1, 0,
        1, 0, 1,
    };
    reset_fb();
    /* Pre-paint the FB so masked transparency is observable. */
    (void)dm1_v1_fmtowns_egb_fill_rect_pc34(fb, FB_W, FB_H, FB_W,
                                            0, 0, 4, 4, 0x33);
    size_t n = dm1_v1_fmtowns_egb_put_block_pc34(fb, FB_W, FB_H, FB_W,
                                                 1, 1, src, 3, 3, 3, 0x77);
    assert(n == 5u); /* 5 non-zero source pixels */
    /* Corners should be 0x77 (recoloured). */
    assert(fb[1 * FB_W + 1] == 0x77);
    assert(fb[1 * FB_W + 3] == 0x77);
    assert(fb[3 * FB_W + 1] == 0x77);
    assert(fb[3 * FB_W + 3] == 0x77);
    /* Centre also 0x77. */
    assert(fb[2 * FB_W + 2] == 0x77);
    /* Zero source pixels leave the pre-painted 0x33 alone. */
    assert(fb[1 * FB_W + 2] == 0x33);
    assert(fb[2 * FB_W + 1] == 0x33);
}

static void test_put_block_clips_off_screen(void) {
    static const uint8_t src[2 * 2] = { 1, 2, 3, 4 };
    reset_fb();
    /* Destination x = FB_W-1 => only one column visible. */
    size_t n = dm1_v1_fmtowns_egb_put_block_pc34(fb, FB_W, FB_H, FB_W,
                                                 FB_W - 1, 5, src, 2, 2, 2, -1);
    assert(n == 2u);
    assert(fb[5 * FB_W + (FB_W - 1)] == 1);
    assert(fb[6 * FB_W + (FB_W - 1)] == 3);
}

static void test_null_guards(void) {
    static const uint8_t src[1] = {1};
    assert(dm1_v1_fmtowns_egb_fill_rect_pc34(NULL, FB_W, FB_H, FB_W,
                                             0, 0, 1, 1, 0) == 0);
    assert(dm1_v1_fmtowns_egb_put_block_pc34(NULL, FB_W, FB_H, FB_W,
                                             0, 0, src, 1, 1, 1, -1) == 0);
    assert(dm1_v1_fmtowns_egb_put_block_pc34(fb, FB_W, FB_H, FB_W,
                                             0, 0, NULL, 1, 1, 1, -1) == 0);
}

int main(void) {
    test_clip_rect_basic();
    test_clip_rect_off_screen();
    test_clip_rect_partial();
    test_clip_rect_swapped();
    test_fill_rect_pixel_count();
    test_fill_rect_menu_panel_region();
    test_put_block_plain_copy();
    test_put_block_masked();
    test_put_block_clips_off_screen();
    test_null_guards();
    printf("All dm1_v1_fmtowns_egb_shim tests passed.\n");
    return 0;
}
