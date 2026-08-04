/*
 * test_dm2_v1_gfx_main_pc34_compat.c — unit tests for DM2 main
 * graphics operations (c_gfx_main.cpp port).
 *
 * Uses mock callbacks that track call counts to verify correct
 * delegation and parameter passing.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_gfx_main_pc34_compat.h"

/* ── Mock state ────────────────────────────────────────────────────── */

typedef struct MockCtx {
    /* Call counts */
    int fill_count;
    int blit_count;
    int blit_within_screen_count;
    int query_expanded_rect_count;
    int query_blit_rect_count;
    int hide_mouse_count;
    int show_mouse_count;
    int select_palette_set_count;
    int draw_string_count;
    int set_bmpheader_width_count;
    int set_bmpheader_height_count;
    int set_bmpheader_res_count;
    int mblitter_blit_count;
    int mblitter_blit_hline_stretched_count;

    /* Last arguments captured */
    DM2_V1_GfxRect last_fill_rect;
    uint8_t        last_fill_pixel;
    uint8_t       *last_fill_dst;
    int16_t        last_palette_set;
    int16_t        last_query_id;
    int16_t        last_set_width;
    int16_t        last_set_height;
    int16_t        last_set_res;

    /* v1e141e scrollbox counter */
    int16_t        v1e141e;
    int16_t        v1e1420;

    /* Screen buffer */
    uint8_t        screen[DM2_GFX_ORIG_SWIDTH * DM2_GFX_ORIG_SHEIGHT];

    /* Fake bitmap */
    uint8_t        fakebmp[256];
    DM2_V1_BmpHeader fakebmp_hdr;
} MockCtx;

static MockCtx g_mock;

/* ── Mock callbacks ────────────────────────────────────────────────── */

static void mock_fill(void *ctx, uint8_t *dst, const DM2_V1_GfxRect *rect,
                      uint8_t pixel)
{
    MockCtx *m = (MockCtx *)ctx;
    m->fill_count++;
    m->last_fill_rect = *rect;
    m->last_fill_pixel = pixel;
    m->last_fill_dst = dst;
}

static void mock_blit(void *ctx, uint8_t *dst, const uint8_t *src,
                      const DM2_V1_GfxRect *rect)
{
    MockCtx *m = (MockCtx *)ctx;
    m->blit_count++;
    (void)dst; (void)src; (void)rect;
}

static void mock_blit_within_screen(void *ctx, const DM2_V1_GfxRect *src_rect,
                                     int16_t dst_x, int16_t dst_y)
{
    MockCtx *m = (MockCtx *)ctx;
    m->blit_within_screen_count++;
    (void)src_rect; (void)dst_x; (void)dst_y;
}

static void mock_query_expanded_rect(void *ctx, int16_t query_id,
                                      DM2_V1_GfxRect *out)
{
    MockCtx *m = (MockCtx *)ctx;
    m->query_expanded_rect_count++;
    m->last_query_id = query_id;
    out->x = 10;
    out->y = 20;
    out->w = 100;
    out->h = 50;
}

static void mock_query_blit_rect(void *ctx, int16_t query_id,
                                  DM2_V1_GfxRect *out)
{
    MockCtx *m = (MockCtx *)ctx;
    m->query_blit_rect_count++;
    out->x = 0; out->y = 0; out->w = 32; out->h = 16;
    (void)query_id;
}

static void mock_hide_mouse(void *ctx) {
    ((MockCtx *)ctx)->hide_mouse_count++;
}
static void mock_show_mouse(void *ctx) {
    ((MockCtx *)ctx)->show_mouse_count++;
}

static void mock_select_palette_set(void *ctx, int16_t set_index) {
    MockCtx *m = (MockCtx *)ctx;
    m->select_palette_set_count++;
    m->last_palette_set = set_index;
}

static uint8_t *mock_get_dm2screen(void *ctx) {
    return ((MockCtx *)ctx)->screen;
}

static uint8_t *mock_get_bmp(void *ctx, int16_t id) {
    (void)id;
    return ((MockCtx *)ctx)->fakebmp;
}

static void mock_set_bmpheader_width(void *ctx, uint8_t *bmp, int16_t w) {
    MockCtx *m = (MockCtx *)ctx;
    m->set_bmpheader_width_count++;
    m->last_set_width = w;
    m->fakebmp_hdr.width = w;
    (void)bmp;
}

static void mock_set_bmpheader_height(void *ctx, uint8_t *bmp, int16_t h) {
    MockCtx *m = (MockCtx *)ctx;
    m->set_bmpheader_height_count++;
    m->last_set_height = h;
    m->fakebmp_hdr.height = h;
    (void)bmp;
}

static void mock_set_bmpheader_res(void *ctx, uint8_t *bmp, int16_t r) {
    MockCtx *m = (MockCtx *)ctx;
    m->set_bmpheader_res_count++;
    m->last_set_res = r;
    m->fakebmp_hdr.res = r;
    (void)bmp;
}

static int16_t mock_get_bmpheader_width(void *ctx, uint8_t *bmp) {
    (void)bmp;
    return ((MockCtx *)ctx)->fakebmp_hdr.width;
}

static int16_t mock_get_bmpheader_height(void *ctx, uint8_t *bmp) {
    (void)bmp;
    return ((MockCtx *)ctx)->fakebmp_hdr.height;
}

static void mock_draw_string(void *ctx, uint8_t *dst, int16_t x, int16_t y,
                              uint8_t color, const char *text) {
    ((MockCtx *)ctx)->draw_string_count++;
    (void)dst; (void)x; (void)y; (void)color; (void)text;
}

static int16_t mock_gfxstrw1(void *ctx) {
    (void)ctx;
    return 8;
}

static int16_t mock_get_v1e141e(void *ctx) {
    return ((MockCtx *)ctx)->v1e141e;
}
static void mock_set_v1e141e(void *ctx, int16_t val) {
    ((MockCtx *)ctx)->v1e141e = val;
}
static int16_t mock_get_v1e1420(void *ctx) {
    return ((MockCtx *)ctx)->v1e1420;
}
static void mock_set_v1e1420(void *ctx, int16_t val) {
    ((MockCtx *)ctx)->v1e1420 = val;
}

static uint8_t *mock_DRV_screen256(void *ctx) {
    return ((MockCtx *)ctx)->screen;
}

static void mock_mblitter_blit(void *ctx, uint8_t *dst, const uint8_t *src,
                                const DM2_V1_GfxRect *rect) {
    ((MockCtx *)ctx)->mblitter_blit_count++;
    (void)dst; (void)src; (void)rect;
}

static void mock_mblitter_blit_hline_stretched(void *ctx, uint8_t *dst,
                                                const uint8_t *src,
                                                int16_t src_x, int16_t dst_x,
                                                int16_t width, int16_t y,
                                                int32_t stretch_frac) {
    ((MockCtx *)ctx)->mblitter_blit_hline_stretched_count++;
    (void)dst; (void)src; (void)src_x; (void)dst_x;
    (void)width; (void)y; (void)stretch_frac;
}

/* ── Helper: build standard callback set ───────────────────────────── */

static DM2_V1_GfxMainCallbacks make_callbacks(void)
{
    DM2_V1_GfxMainCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx = &g_mock;
    cb.fill = mock_fill;
    cb.blit = mock_blit;
    cb.blit_within_screen = mock_blit_within_screen;
    cb.query_expanded_rect = mock_query_expanded_rect;
    cb.query_blit_rect = mock_query_blit_rect;
    cb.hide_mouse = mock_hide_mouse;
    cb.show_mouse = mock_show_mouse;
    cb.select_palette_set = mock_select_palette_set;
    cb.get_dm2screen = mock_get_dm2screen;
    cb.get_bmp = mock_get_bmp;
    cb.set_bmpheader_width = mock_set_bmpheader_width;
    cb.set_bmpheader_height = mock_set_bmpheader_height;
    cb.set_bmpheader_res = mock_set_bmpheader_res;
    cb.get_bmpheader_width = mock_get_bmpheader_width;
    cb.get_bmpheader_height = mock_get_bmpheader_height;
    cb.draw_string = mock_draw_string;
    cb.gfxstrw1 = mock_gfxstrw1;
    cb.get_v1e141e = mock_get_v1e141e;
    cb.set_v1e141e = mock_set_v1e141e;
    cb.get_v1e1420 = mock_get_v1e1420;
    cb.set_v1e1420 = mock_set_v1e1420;
    cb.DRV_screen256 = mock_DRV_screen256;
    cb.mblitter_blit = mock_mblitter_blit;
    cb.mblitter_blit_hline_stretched = mock_mblitter_blit_hline_stretched;
    return cb;
}

static void reset_mock(void) {
    memset(&g_mock, 0, sizeof(g_mock));
}

/* ── Tests ─────────────────────────────────────────────────────────── */

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(fn) do { \
    tests_run++; \
    reset_mock(); \
    fn(); \
    tests_passed++; \
    printf("  PASS: %s\n", #fn); \
} while (0)

static void test_init_defaults(void)
{
    DM2_V1_GfxMainState st;
    dm2_v1_gfx_main_init(&st);

    assert(st.backbuffer_w == 0xe0);
    assert(st.backbuffer_h == 0x88);
    assert(st.disable_video == 0);
    assert(st.bitmapptr == NULL);
    assert(st.pictbuff == NULL);
    assert(st.backbuffrect_x == 0);
    assert(st.backbuffrect_y == 0);
}

static void test_init_backbuff(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();

    dm2_v1_gfx_main_init(&st);
    dm2_v1_gfx_main_init_backbuff(&st, &cb);

    assert(g_mock.set_bmpheader_width_count == 1);
    assert(g_mock.last_set_width == 0xe0);
    assert(g_mock.set_bmpheader_height_count == 1);
    assert(g_mock.last_set_height == 0x88);
    assert(g_mock.set_bmpheader_res_count == 1);
    assert(g_mock.last_set_res == 4);
    assert(st.bitmapptr == g_mock.fakebmp);
    assert(st.backbuffrect_w == 0xe0);
    assert(st.backbuffrect_h == 0x88);
}

static void test_fill_backbuff_rect(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();
    DM2_V1_GfxRect rect = {5, 10, 20, 30};

    dm2_v1_gfx_main_init(&st);
    st.bitmapptr = g_mock.fakebmp;

    dm2_v1_gfx_main_fill_backbuff_rect(&st, &cb, &rect, 0xAB);

    assert(g_mock.fill_count == 1);
    assert(g_mock.last_fill_dst == g_mock.fakebmp);
    assert(g_mock.last_fill_rect.x == 5);
    assert(g_mock.last_fill_rect.y == 10);
    assert(g_mock.last_fill_rect.w == 20);
    assert(g_mock.last_fill_rect.h == 30);
    assert(g_mock.last_fill_pixel == 0xAB);
}

static void test_fill_screen_rect(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();

    dm2_v1_gfx_main_init(&st);
    dm2_v1_gfx_main_fill_screen_rect(&st, &cb, 42, 0xFF);

    /* Should query rect first, then fill */
    assert(g_mock.query_expanded_rect_count == 1);
    assert(g_mock.last_query_id == 42);
    assert(g_mock.fill_count == 1);
    assert(g_mock.last_fill_dst == g_mock.screen);
    assert(g_mock.last_fill_pixel == 0xFF);
    /* Mock returns rect (10,20,100,50) */
    assert(g_mock.last_fill_rect.x == 10);
    assert(g_mock.last_fill_rect.y == 20);
}

static void test_fill_halftone_checkerboard(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();
    DM2_V1_GfxRect rect = {0, 0, 4, 4};

    dm2_v1_gfx_main_init(&st);

    /* Fill screen with 0xFF first */
    memset(g_mock.screen, 0xFF, sizeof(g_mock.screen));

    dm2_v1_gfx_main_fill_halftone_rectv(&st, &cb, &rect);

    /* Checkerboard: (x+y)&1==0 => 0, else 0xFF */
    assert(g_mock.screen[0 * DM2_GFX_ORIG_SWIDTH + 0] == 0x00); /* (0,0) */
    assert(g_mock.screen[0 * DM2_GFX_ORIG_SWIDTH + 1] == 0xFF); /* (1,0) */
    assert(g_mock.screen[0 * DM2_GFX_ORIG_SWIDTH + 2] == 0x00); /* (2,0) */
    assert(g_mock.screen[1 * DM2_GFX_ORIG_SWIDTH + 0] == 0xFF); /* (0,1) */
    assert(g_mock.screen[1 * DM2_GFX_ORIG_SWIDTH + 1] == 0x00); /* (1,1) */
    assert(g_mock.screen[1 * DM2_GFX_ORIG_SWIDTH + 2] == 0xFF); /* (2,1) */
    assert(g_mock.screen[2 * DM2_GFX_ORIG_SWIDTH + 0] == 0x00); /* (0,2) */
    assert(g_mock.screen[2 * DM2_GFX_ORIG_SWIDTH + 1] == 0xFF); /* (1,2) */
}

static void test_fade_screen_mode0(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();

    dm2_v1_gfx_main_init(&st);
    dm2_v1_gfx_main_fade_screen(&st, &cb, 0);

    assert(g_mock.select_palette_set_count == 1);
    assert(g_mock.last_palette_set == 1);
    assert(g_mock.fill_count == 0); /* mode 0 does not fill */
}

static void test_fade_screen_mode1(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();

    dm2_v1_gfx_main_init(&st);
    dm2_v1_gfx_main_fade_screen(&st, &cb, 1);

    assert(g_mock.select_palette_set_count == 1);
    assert(g_mock.last_palette_set == 0);
    assert(g_mock.fill_count == 1);
    assert(g_mock.last_fill_pixel == 0);
    assert(g_mock.last_fill_rect.w == DM2_GFX_ORIG_SWIDTH);
    assert(g_mock.last_fill_rect.h == DM2_GFX_ORIG_SHEIGHT);
}

static void test_advance_scrollbox(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();

    dm2_v1_gfx_main_init(&st);
    g_mock.v1e141e = 3;

    dm2_v1_gfx_main_advance_scrollbox(&st, &cb);

    assert(g_mock.v1e141e == 4);
    assert(g_mock.blit_within_screen_count == 1);
    assert(g_mock.fill_count == 1); /* bottom line cleared */
}

static void test_blit_toscreen_disable_video(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();
    DM2_V1_GfxRect rect = {0, 0, 32, 32};
    uint8_t src[1024];

    dm2_v1_gfx_main_init(&st);
    assert(st.disable_video == 0);

    dm2_v1_gfx_main_blit_toscreen(&st, &cb, src, &rect);

    /* disable_video incremented before blit, decremented after */
    assert(st.disable_video == 0);
    assert(g_mock.blit_count == 1);
}

static void test_specialblit_normal(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();
    DM2_V1_GfxRect rect = {0, 0, 32, 32};
    uint8_t dst[1024], src[1024];

    dm2_v1_gfx_main_init(&st);
    dm2_v1_gfx_main_specialblit(&st, &cb, dst, src, &rect, 0);

    assert(g_mock.mblitter_blit_count == 1);
    assert(g_mock.mblitter_blit_hline_stretched_count == 0);
}

static void test_specialblit_stretched(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();
    DM2_V1_GfxRect rect = {0, 0, 32, 4};
    uint8_t dst[1024], src[1024];

    dm2_v1_gfx_main_init(&st);
    dm2_v1_gfx_main_specialblit(&st, &cb, dst, src, &rect, 1);

    assert(g_mock.mblitter_blit_count == 0);
    assert(g_mock.mblitter_blit_hline_stretched_count == 4); /* one per line */
}

static void test_drawings_completed(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();

    dm2_v1_gfx_main_init(&st);
    dm2_v1_gfx_main_init_backbuff(&st, &cb);
    reset_mock(); /* clear counts from init_backbuff */
    cb = make_callbacks();

    dm2_v1_gfx_main_drawings_completed(&st, &cb);

    assert(g_mock.blit_count == 1);
    assert(g_mock.show_mouse_count == 1);
    assert(st.disable_video == 0);
}

static void test_fill_entire_pict(void)
{
    DM2_V1_GfxMainState st;
    DM2_V1_GfxMainCallbacks cb = make_callbacks();

    dm2_v1_gfx_main_init(&st);
    g_mock.fakebmp_hdr.width = 64;
    g_mock.fakebmp_hdr.height = 32;

    dm2_v1_gfx_main_fill_entire_pict(&st, &cb, g_mock.fakebmp, 0x42);

    assert(g_mock.fill_count == 1);
    assert(g_mock.last_fill_rect.x == 0);
    assert(g_mock.last_fill_rect.y == 0);
    assert(g_mock.last_fill_rect.w == 64);
    assert(g_mock.last_fill_rect.h == 32);
    assert(g_mock.last_fill_pixel == 0x42);
}

/* ── Main ──────────────────────────────────────────────────────────── */

int main(void)
{
    printf("test_dm2_v1_gfx_main_pc34_compat:\n");

    RUN_TEST(test_init_defaults);
    RUN_TEST(test_init_backbuff);
    RUN_TEST(test_fill_backbuff_rect);
    RUN_TEST(test_fill_screen_rect);
    RUN_TEST(test_fill_halftone_checkerboard);
    RUN_TEST(test_fade_screen_mode0);
    RUN_TEST(test_fade_screen_mode1);
    RUN_TEST(test_advance_scrollbox);
    RUN_TEST(test_blit_toscreen_disable_video);
    RUN_TEST(test_specialblit_normal);
    RUN_TEST(test_specialblit_stretched);
    RUN_TEST(test_drawings_completed);
    RUN_TEST(test_fill_entire_pict);

    printf("\n  %d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
