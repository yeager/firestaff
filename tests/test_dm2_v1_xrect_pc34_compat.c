/*
 * test_dm2_v1_xrect_pc34_compat.c -- unit tests for the DM2
 * extended rectangle operations (c_xrect.cpp port).
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_xrect_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, name) do { \
    if (cond) { printf("  PASS: %s\n", name); g_pass++; } \
    else      { printf("  FAIL: %s\n", name); g_fail++; } \
} while (0)

/* ── Mock callbacks ─────────────────────────────────────────────────── */

static int16_t mock_between_value(void *ctx, int16_t minv, int16_t maxv,
                                  int16_t val)
{
    (void)ctx;
    if (val < minv) return minv;
    if (val > maxv) return maxv;
    return val;
}

static int16_t mock_min_i16(void *ctx, int16_t a, int16_t b)
{
    (void)ctx;
    return a < b ? a : b;
}

static int16_t mock_get_bmp_width(void *ctx, void *bmp)
{
    (void)ctx; (void)bmp;
    return 32;
}

static int16_t mock_get_bmp_height(void *ctx, void *bmp)
{
    (void)ctx; (void)bmp;
    return 24;
}

static bool mock_get_false(void *ctx) { (void)ctx; return false; }
static int16_t mock_get_zero(void *ctx) { (void)ctx; return 0; }

static DM2_V1_XrectCallbacks make_callbacks(void)
{
    DM2_V1_XrectCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx = NULL;
    cb.between_value = mock_between_value;
    cb.min_i16 = mock_min_i16;
    cb.get_bmp_width = mock_get_bmp_width;
    cb.get_bmp_height = mock_get_bmp_height;
    cb.get_v1e01d0 = mock_get_false;
    cb.get_v1e01d8 = mock_get_false;
    cb.get_v1e025c = mock_get_zero;
    return cb;
}

/* ── Test: init ─────────────────────────────────────────────────────── */

static void test_init(void)
{
    printf("test_init:\n");
    DM2_V1_XrectState st;
    dm2_v1_xrect_init(&st);
    CHECK(st.queryrectsindex == 0, "queryrectsindex is 0");
    CHECK(st.rnodep_rectanglelist == NULL, "rectanglelist is NULL");
    CHECK(st.queryrects[0].x == 0, "queryrects[0].x is 0");
}

/* ── Test: calc_size_of_compressed_rect ─────────────────────────────── */

static void test_calc_size(void)
{
    printf("test_calc_size:\n");

    /* mask=0x00: no flags, full 8 bytes per rect */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x00) == 8,
          "mask 0x00 -> 8");

    /* mask=0x04: byte x/y -> 6 */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x04) == 6,
          "mask 0x04 -> 6");

    /* mask=0x02: shared x -> 6 */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x02) == 6,
          "mask 0x02 -> 6");

    /* mask=0x01: shared y -> 6 */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x01) == 6,
          "mask 0x01 -> 6");

    /* mask=0x03: shared x and y -> 4 */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x03) == 4,
          "mask 0x03 -> 4");

    /* mask=0x08: signed byte w/h -> 6 (8-2) */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x08) == 6,
          "mask 0x08 -> 6");

    /* mask=0x10: unsigned byte w/h -> 6 */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x10) == 6,
          "mask 0x10 -> 6");

    /* mask=0x18: both byte flags on w/h -> 6 */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x18) == 6,
          "mask 0x18 -> 6");

    /* mask=0x0b: shared x+y + signed byte w/h -> 2 */
    CHECK(dm2_v1_calc_size_of_compressed_rect(0x0b) == 2,
          "mask 0x0b -> 2");
}

/* ── Test: query_rect with NULL list ────────────────────────────────── */

static void test_query_rect_null(void)
{
    printf("test_query_rect_null:\n");
    DM2_V1_XrectState st;
    dm2_v1_xrect_init(&st);

    CHECK(dm2_v1_query_rect(&st, 0) == NULL, "query 0 returns NULL");
    CHECK(dm2_v1_query_rect(&st, 1) == NULL, "query with no list returns NULL");
}

/* ── Test: query_rect with a simple node ────────────────────────────── */

static void test_query_rect_simple(void)
{
    printf("test_query_rect_simple:\n");

    /* Build a node with mask=0x00 (no compression), min=1, max=1,
     * containing one rect: x=10, y=20, w=30, h=40 */
    uint8_t buf[sizeof(DM2_V1_RNode) + 8];
    memset(buf, 0, sizeof(buf));
    DM2_V1_RNode *node = (DM2_V1_RNode *)buf;
    node->next = NULL;
    node->min = 1;
    node->max = 1;
    node->mask = 0x00;
    node->b_x = 0;

    /* Data at offset 10: x(i16), y(i16), w(i16), h(i16) */
    int16_t *data = (int16_t *)(buf + sizeof(DM2_V1_RNode));
    data[0] = 10;  /* x */
    data[1] = 20;  /* y */
    data[2] = 30;  /* w */
    data[3] = 40;  /* h */

    DM2_V1_XrectState st;
    dm2_v1_xrect_init(&st);
    st.rnodep_rectanglelist = node;

    DM2_V1_Rect *r = dm2_v1_query_rect(&st, 1);
    CHECK(r != NULL, "query 1 returns non-NULL");
    if (r) {
        CHECK(r->x == 10, "x == 10");
        CHECK(r->y == 20, "y == 20");
        CHECK(r->w == 30, "w == 30");
        CHECK(r->h == 40, "h == 40");
    }
}

/* ── Test: query_rect with byte-compressed w/h (mask 0x10) ──────────── */

static void test_query_rect_byte_wh(void)
{
    printf("test_query_rect_byte_wh:\n");

    /* mask=0x10: unsigned byte w/h, word x/y
     * compressed size = 6 (x:2, y:2, w:1, h:1) */
    uint8_t buf[sizeof(DM2_V1_RNode) + 6];
    memset(buf, 0, sizeof(buf));
    DM2_V1_RNode *node = (DM2_V1_RNode *)buf;
    node->next = NULL;
    node->min = 1;
    node->max = 1;
    node->mask = 0x10;
    node->b_x = 0;

    int16_t *wdata = (int16_t *)(buf + sizeof(DM2_V1_RNode));
    wdata[0] = 5;   /* x */
    wdata[1] = 15;  /* y */
    uint8_t *bdata = (uint8_t *)(buf + sizeof(DM2_V1_RNode) + 4);
    bdata[0] = 100; /* w (unsigned byte) */
    bdata[1] = 200; /* h (unsigned byte) */

    DM2_V1_XrectState st;
    dm2_v1_xrect_init(&st);
    st.rnodep_rectanglelist = node;

    DM2_V1_Rect *r = dm2_v1_query_rect(&st, 1);
    CHECK(r != NULL, "query returns non-NULL");
    if (r) {
        CHECK(r->x == 5, "x == 5");
        CHECK(r->y == 15, "y == 15");
        CHECK(r->w == 100, "w == 100");
        CHECK(r->h == 200, "h == 200");
    }
}

/* ── Test: crdecode modes via query_blit_rect ───────────────────────── */

static void test_crdecode_modes(void)
{
    printf("test_crdecode_modes:\n");

    /* Set up a simple rinfo node: mode1=1, mode2=0, datax=50, datay=60
     * Then another with mode1=9, mode2=0, datax=W, datay=H
     * We test mode 1 = identity: blitrect.x = datax, y = datay */

    /* Node for query 1: mode1(x)=1, mode2(y)=0, datax(w)=50, datay(h)=60 */
    uint8_t buf1[sizeof(DM2_V1_RNode) + 8];
    memset(buf1, 0, sizeof(buf1));
    DM2_V1_RNode *node1 = (DM2_V1_RNode *)buf1;
    node1->next = NULL;
    node1->min = 1;
    node1->max = 1;
    node1->mask = 0x00;
    node1->b_x = 0;
    int16_t *d1 = (int16_t *)(buf1 + sizeof(DM2_V1_RNode));
    d1[0] = 1;   /* mode1 = identity */
    d1[1] = 0;   /* mode2 = 0 (no chain) */
    d1[2] = 50;  /* datax */
    d1[3] = 60;  /* datay */

    /* Node for query 2: mode1(x)=9, mode2(y)=0, datax(w)=32, datay(h)=24 */
    uint8_t buf2[sizeof(DM2_V1_RNode) + 8];
    memset(buf2, 0, sizeof(buf2));
    DM2_V1_RNode *node2 = (DM2_V1_RNode *)buf2;
    node2->next = NULL;
    node2->min = 2;
    node2->max = 2;
    node2->mask = 0x00;
    node2->b_x = 0;
    int16_t *d2 = (int16_t *)(buf2 + sizeof(DM2_V1_RNode));
    d2[0] = 9;   /* mode1 = size marker */
    d2[1] = 0;   /* mode2 */
    d2[2] = 32;  /* w */
    d2[3] = 24;  /* h */

    node1->next = node2;

    DM2_V1_XrectState st;
    dm2_v1_xrect_init(&st);
    st.rnodep_rectanglelist = node1;

    DM2_V1_XrectCallbacks cb = make_callbacks();

    /* Test QUERY_BLIT_RECT with mode 1 (identity) */
    DM2_V1_Rect blitrect;
    int16_t xout = 32, yout = 24;
    DM2_V1_Rect *result = dm2_v1_query_blit_rect(
        &st, &cb, NULL, &blitrect, 1, &xout, &yout, -1);

    CHECK(result != NULL, "blit_rect mode1 returns non-NULL");
    if (result) {
        CHECK(result->x == 50, "blit_rect mode1 x == 50");
        CHECK(result->y == 60, "blit_rect mode1 y == 60");
    }
}

/* ── Test: pt_in_expanded_rect ──────────────────────────────────────── */

static void test_pt_in_expanded_rect(void)
{
    printf("test_pt_in_expanded_rect:\n");

    /* We need two linked nodes for scale_rect / expanded_rect:
     * query Q -> mode1=X, mode2=Q2, ...
     * query Q2 -> mode1=9, mode2=0, datax=W, datay=H
     *
     * expanded_rect calls scale_rect(Q, SCALE, SCALE, &r)
     * scale_rect: qrect1 = query_rect(Q), needs qrect1->y != 0
     *             qrect2 = query_rect(qrect1->y), needs qrect2->x == 9
     *             then calls query_blit_rect(NULL, r, Q, &w, &h, -1)
     */

    /* Node for query 1: x=1(mode1), y=2(mode2/link), w=10(datax), h=20(datay) */
    uint8_t buf1[sizeof(DM2_V1_RNode) + 8];
    memset(buf1, 0, sizeof(buf1));
    DM2_V1_RNode *n1 = (DM2_V1_RNode *)buf1;
    n1->min = 1; n1->max = 1; n1->mask = 0x00; n1->b_x = 0;
    int16_t *d1 = (int16_t *)(buf1 + sizeof(DM2_V1_RNode));
    d1[0] = 1;  /* mode1 = identity */
    d1[1] = 2;  /* mode2 = link to query 2 */
    d1[2] = 10; /* datax */
    d1[3] = 20; /* datay */

    /* Node for query 2: x=9(marker), y=0, w=30, h=40 */
    uint8_t buf2[sizeof(DM2_V1_RNode) + 8];
    memset(buf2, 0, sizeof(buf2));
    DM2_V1_RNode *n2 = (DM2_V1_RNode *)buf2;
    n2->next = NULL;
    n2->min = 2; n2->max = 2; n2->mask = 0x00; n2->b_x = 0;
    int16_t *d2 = (int16_t *)(buf2 + sizeof(DM2_V1_RNode));
    d2[0] = 9;  /* mode1 = 9 (size marker) */
    d2[1] = 0;  /* mode2 */
    d2[2] = 30; /* w */
    d2[3] = 40; /* h */

    n1->next = n2;

    DM2_V1_XrectState st;
    dm2_v1_xrect_init(&st);
    st.rnodep_rectanglelist = n1;

    DM2_V1_XrectCallbacks cb = make_callbacks();

    /* First verify expanded_rect works */
    DM2_V1_Rect r;
    DM2_V1_Rect *result = dm2_v1_query_expanded_rect(&st, &cb, 1, &r);
    CHECK(result != NULL, "expanded_rect returns non-NULL");

    if (result) {
        /* With mode1=1 (identity), blitrect should be at (10,20) with size 30x40 */
        /* Point inside */
        bool inside = dm2_v1_pt_in_expanded_rect(&st, &cb, 1, 15, 30);
        CHECK(inside, "point (15,30) inside expanded rect");

        /* Point outside */
        bool outside = !dm2_v1_pt_in_expanded_rect(&st, &cb, 1, -5, -5);
        CHECK(outside, "point (-5,-5) outside expanded rect");
    }
}

/* ── Test: rect_098d_04c7 ───────────────────────────────────────────── */

static void test_rect_098d_04c7(void)
{
    printf("test_rect_098d_04c7:\n");

    /* Two rects: query 1 has w=10, h=20; query 2 has w=30, h=60 */
    uint8_t buf1[10 + 8], buf2[10 + 8];
    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));

    DM2_V1_RNode *n1 = (DM2_V1_RNode *)buf1;
    n1->min = 1; n1->max = 1; n1->mask = 0x00; n1->b_x = 0;
    int16_t *d1 = (int16_t *)(buf1 + sizeof(DM2_V1_RNode));
    d1[0] = 0; d1[1] = 0; d1[2] = 10; d1[3] = 20;

    DM2_V1_RNode *n2 = (DM2_V1_RNode *)buf2;
    n2->next = NULL;
    n2->min = 2; n2->max = 2; n2->mask = 0x00; n2->b_x = 0;
    int16_t *d2 = (int16_t *)(buf2 + sizeof(DM2_V1_RNode));
    d2[0] = 0; d2[1] = 0; d2[2] = 30; d2[3] = 60;

    n1->next = n2;

    DM2_V1_XrectState st;
    dm2_v1_xrect_init(&st);
    st.rnodep_rectanglelist = n1;

    DM2_V1_XrectCallbacks cb = make_callbacks();

    /* wb=50 -> wc = (30-10)*50/100 = 10, we = (60-20)*50/100 = 20 */
    DM2_V1_Rect098d04c7Receipt rcpt = dm2_v1_rect_098d_04c7(
        &st, &cb, 1, 2, 50);
    CHECK(rcpt.wc == 10, "wc == 10 at 50%");
    CHECK(rcpt.we == 20, "we == 20 at 50%");

    /* wb=0 -> wc=0, we=0 */
    rcpt = dm2_v1_rect_098d_04c7(&st, &cb, 1, 2, 0);
    CHECK(rcpt.wc == 0, "wc == 0 at 0%");
    CHECK(rcpt.we == 0, "we == 0 at 0%");

    /* wb clamped to 100 */
    rcpt = dm2_v1_rect_098d_04c7(&st, &cb, 1, 2, 200);
    CHECK(rcpt.wc == 20, "wc == 20 at 100% (clamped)");
    CHECK(rcpt.we == 40, "we == 40 at 100% (clamped)");
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void)
{
    test_init();
    test_calc_size();
    test_query_rect_null();
    test_query_rect_simple();
    test_query_rect_byte_wh();
    test_crdecode_modes();
    test_pt_in_expanded_rect();
    test_rect_098d_04c7();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
