#include "dm2_v1_gui_vp_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_total  = 0;

#define RUN(fn) do { \
    tests_total++; \
    fn(); \
    tests_passed++; \
    printf("  PASS  %s\n", #fn); \
} while (0)

/* ── Null-safety tests ─────────────────────────────────────────────── */

static void test_display_viewport_null_cb(void) {
    DM2_V1_DisplayViewportReceipt r;
    memset(&r, 0xFF, sizeof(r));
    dm2_v1_display_viewport(0, 0, 0, NULL, NULL, &r);
    assert(!r.viewport_rendered);
}

static void test_display_viewport_null_receipt(void) {
    DM2_V1_GuiVpCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    dm2_v1_display_viewport(0, 0, 0, &cb, NULL, NULL);
}

static void test_summarize_stone_room_null_cb(void) {
    DM2_V1_StoneRoomSummary s;
    dm2_v1_summarize_stone_room(&s, 0, 0, 0, NULL, NULL);
    assert(s.record_link == 0xFF);
    assert(s.ornament_word6 == 0xFF);
}

static void test_summarize_stone_room_null_out(void) {
    dm2_v1_summarize_stone_room(NULL, 0, 0, 0, NULL, NULL);
}

static void test_guivp_00f1_null_cb(void) {
    DM2_V1_GuiVp00f1Receipt r;
    bool hit = dm2_v1_guivp_32cb_00f1(0, 0, 0, NULL, NULL, &r);
    assert(!hit);
    assert(!r.hit);
}

static void test_guivp_0c7d_null_cb(void) {
    dm2_v1_guivp_32cb_0c7d(NULL, 0, 0, NULL, NULL);
}

static void test_guivp_15b8_null_cb(void) {
    DM2_V1_GuiVp15b8Receipt r;
    int32_t res = dm2_v1_guivp_32cb_15b8(0, 0, 0, NULL, NULL, &r);
    assert(res == -1);
    assert(!r.ornament_drawn);
}

static void test_draw_item_null_cb(void) {
    dm2_v1_draw_item(0, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, NULL);
}

static void test_trim_blit_rect_null_cb(void) {
    dm2_v1_trim_blit_rect(0, 0, 0, 0, NULL, NULL);
}

static void test_draw_wall_null_cb(void) {
    dm2_v1_draw_wall(0, NULL, NULL);
}

static void test_draw_pit_roof_null_cb(void) {
    dm2_v1_draw_pit_roof(0, NULL, NULL);
}

static void test_draw_pit_tile_null_cb(void) {
    dm2_v1_draw_pit_tile(0, NULL, NULL);
}

static void test_draw_stairs_front_null_cb(void) {
    dm2_v1_draw_stairs_front(0, NULL, NULL);
}

static void test_draw_stairs_side_null_cb(void) {
    dm2_v1_draw_stairs_side(0, NULL, NULL);
}

static void test_draw_rain_null_cb(void) {
    dm2_v1_draw_rain(NULL, NULL);
}

static void test_draw_dungeon_tiles_null_cb(void) {
    int32_t r = dm2_v1_draw_dungeon_tiles(NULL, NULL);
    assert(r == 0);
}

static void test_chance_table_operation_null_cb(void) {
    dm2_v1_chance_table_operation(NULL, NULL);
}

/* ── guivp_098d_0cd7 computation ───────────────────────────────────── */

static void test_guivp_098d_0cd7_no_vbool(void) {
    int16_t r = dm2_v1_guivp_098d_0cd7(0, 0, false);
    assert(r == 3100);
}

static void test_guivp_098d_0cd7_with_offset(void) {
    int16_t r = dm2_v1_guivp_098d_0cd7(1, 5, false);
    assert(r == 3130);
}

static void test_guivp_098d_0cd7_vbool(void) {
    int16_t r = dm2_v1_guivp_098d_0cd7(1, 0, true);
    assert(r == dm2_guivp_table1d27a0[1]);
}

/* ── guivp_32cb_54ce direction test ────────────────────────────────── */

static void test_guivp_54ce_heading0(void) {
    int16_t x = 5, y = 10;
    int32_t r = dm2_v1_guivp_32cb_54ce(0, &x, &y, 3, 8);
    (void)r;
    /* Verifies no crash; exact output depends on math */
}

static void test_guivp_54ce_null_ptrs(void) {
    int32_t r = dm2_v1_guivp_32cb_54ce(0, NULL, NULL, 0, 0);
    assert(r == 0);
}

/* ── Data table verification ───────────────────────────────────────── */

static void test_table1d27a0_entries(void) {
    assert(dm2_guivp_table1d27a0[0] == -1);
    assert(dm2_guivp_table1d27a0[1] == 0x1149);
    assert(dm2_guivp_table1d27a0[15] == 0x12a7);
}

static void test_table1d7029_first_last(void) {
    assert(dm2_guivp_table1d7029[0] == 0x16);
    assert(dm2_guivp_table1d7029[19] == 0x03);
}

/* ── guivp_32cb_35c1 null ptrs ─────────────────────────────────────── */

static void test_guivp_35c1_null_ptrs(void) {
    int32_t r = dm2_v1_guivp_32cb_35c1(NULL, NULL, 0, 0, NULL, NULL);
    assert(r == 0);
}

/* ── Callback dispatch tests ───────────────────────────────────────── */

static bool mock_hit_test_called = false;
static bool mock_hit_test(void *ctx, int16_t x, int16_t y, uint8_t flags,
                           int16_t *out_da, int16_t *out_d8) {
    (void)ctx; (void)x; (void)y; (void)flags;
    mock_hit_test_called = true;
    *out_da = 42;
    *out_d8 = 99;
    return true;
}

static void test_guivp_00f1_callback_dispatch(void) {
    DM2_V1_GuiVpCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.viewport_hit_test = mock_hit_test;
    mock_hit_test_called = false;

    DM2_V1_GuiVp00f1Receipt r;
    bool hit = dm2_v1_guivp_32cb_00f1(10, 20, 0, &cb, NULL, &r);
    assert(mock_hit_test_called);
    assert(hit);
    assert(r.hit);
    assert(r.viewport_da == 42);
    assert(r.viewport_d8 == 99);
}

static bool mock_fill_bg_called = false;
static void mock_fill_bg(void *ctx, void *image, int16_t bmpid, int16_t colidx) {
    (void)ctx; (void)image; (void)bmpid; (void)colidx;
    mock_fill_bg_called = true;
}

static void test_guivp_0c7d_callback_dispatch(void) {
    DM2_V1_GuiVpCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.fill_image_background = mock_fill_bg;
    mock_fill_bg_called = false;

    int dummy;
    dm2_v1_guivp_32cb_0c7d(&dummy, 5, 10, &cb, NULL);
    assert(mock_fill_bg_called);
}

static int16_t mock_backbuffer_w(void *ctx) { (void)ctx; return 320; }
static int16_t mock_backbuffer_h(void *ctx) { (void)ctx; return 200; }
static int16_t mock_blit_x, mock_blit_y, mock_blit_w, mock_blit_h;
static void mock_set_blit_rect(void *ctx, int16_t x, int16_t y, int16_t w, int16_t h) {
    (void)ctx;
    mock_blit_x = x; mock_blit_y = y;
    mock_blit_w = w; mock_blit_h = h;
}

static void test_trim_blit_rect_callback(void) {
    DM2_V1_GuiVpCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_backbuffer_w = mock_backbuffer_w;
    cb.get_backbuffer_h = mock_backbuffer_h;
    cb.set_blit_rect = mock_set_blit_rect;

    dm2_v1_trim_blit_rect(10, 20, 100, 50, &cb, NULL);
    assert(mock_blit_x == 10);
    assert(mock_blit_y == 20);
    assert(mock_blit_w == 210);  /* 320 - (10+100) */
    assert(mock_blit_h == 130);  /* 200 - (20+50) */
}

int main(void) {
    printf("test_dm2_v1_gui_vp_pc34_compat\n");

    RUN(test_display_viewport_null_cb);
    RUN(test_display_viewport_null_receipt);
    RUN(test_summarize_stone_room_null_cb);
    RUN(test_summarize_stone_room_null_out);
    RUN(test_guivp_00f1_null_cb);
    RUN(test_guivp_0c7d_null_cb);
    RUN(test_guivp_15b8_null_cb);
    RUN(test_draw_item_null_cb);
    RUN(test_trim_blit_rect_null_cb);
    RUN(test_draw_wall_null_cb);
    RUN(test_draw_pit_roof_null_cb);
    RUN(test_draw_pit_tile_null_cb);
    RUN(test_draw_stairs_front_null_cb);
    RUN(test_draw_stairs_side_null_cb);
    RUN(test_draw_rain_null_cb);
    RUN(test_draw_dungeon_tiles_null_cb);
    RUN(test_chance_table_operation_null_cb);
    RUN(test_guivp_098d_0cd7_no_vbool);
    RUN(test_guivp_098d_0cd7_with_offset);
    RUN(test_guivp_098d_0cd7_vbool);
    RUN(test_guivp_54ce_heading0);
    RUN(test_guivp_54ce_null_ptrs);
    RUN(test_table1d27a0_entries);
    RUN(test_table1d7029_first_last);
    RUN(test_guivp_35c1_null_ptrs);
    RUN(test_guivp_00f1_callback_dispatch);
    RUN(test_guivp_0c7d_callback_dispatch);
    RUN(test_trim_blit_rect_callback);

    printf("\n%d / %d tests passed\n", tests_passed, tests_total);
    return (tests_passed == tests_total) ? 0 : 1;
}
