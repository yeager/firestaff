/*
 * test_dm2_v1_0aaf_pc34_compat.c -- unit tests for DM2 segment 0AAF
 * dialogue/menu logic.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_0aaf_pc34_compat.h"

/* ── Mock state ─────────────────────────────────────────────────────── */

static int mock_draw_count = 0;
static int mock_fill_count = 0;
static int mock_menu_visible = 0;
static int16_t mock_menu_event = 0xff;
static int16_t mock_menu_pending_event = 0xff;

/* ── Mock callbacks ─────────────────────────────────────────────────── */

static const char *mock_query_gdat_text(void *ctx, uint8_t cat,
    uint8_t type, uint8_t sub, char *buf)
{
    (void)ctx; (void)cat; (void)type;
    if (sub < 3) {
        snprintf(buf, 40, "Option %d", sub);
        return buf;
    }
    buf[0] = '\0';
    return buf;
}

static int16_t mock_query_gdat_entry_data_index(void *ctx, uint8_t cat,
    uint8_t type, uint8_t sub1, uint8_t sub2)
{
    (void)ctx; (void)cat; (void)type; (void)sub1;
    return (int16_t)(sub2 + 1);
}

static bool mock_query_gdat_entry_if_loadable(void *ctx, uint8_t cat,
    uint8_t type, uint8_t sub1, uint8_t sub2)
{
    (void)ctx; (void)cat; (void)type; (void)sub1; (void)sub2;
    return false;
}

static void mock_fill_backbuff_rect(void *ctx, const DM2_V1_0aafRect *r,
    uint8_t pixel)
{
    (void)ctx; (void)r; (void)pixel;
    mock_fill_count++;
}

static DM2_V1_0aafRect *mock_query_expanded_rect(void *ctx, int16_t query,
    DM2_V1_0aafRect *r)
{
    (void)ctx; (void)query;
    r->x = 10; r->y = 10; r->w = 100; r->h = 50;
    return r;
}

static void mock_draw_vp_rc_str(void *ctx, int16_t rect_id, int16_t color,
    const char *text)
{
    (void)ctx; (void)rect_id; (void)color; (void)text;
    mock_draw_count++;
}

static uint8_t mock_palette_to_ui8(void *ctx, int idx)
{
    (void)ctx;
    return (uint8_t)(idx & 0xFF);
}

static bool mock_false(void *ctx) { (void)ctx; return false; }
static int16_t mock_zero16(void *ctx) { (void)ctx; return 0; }
static void mock_void_i(void *ctx, int a) { (void)ctx; (void)a; }
static void mock_set_i16(void *ctx, int16_t v) { (void)ctx; (void)v; }
static int16_t mock_get_bbw(void *ctx) { (void)ctx; return 320; }
static int16_t mock_get_bbh(void *ctx) { (void)ctx; return 200; }
static int32_t mock_mode_i32(void *ctx, int16_t mode)
{ (void)ctx; (void)mode; return 0; }
static void mock_void(void *ctx) { (void)ctx; }
static const char *mock_v1d1044(void *ctx) { (void)ctx; return "Title"; }
static int16_t mock_table_val(void *ctx, int idx) { (void)ctx; return (int16_t)(0x100 + idx); }
static int32_t mock_bigpool(void *ctx) { (void)ctx; return 0; }
static void mock_draw_to_screen(void *ctx) { (void)ctx; }
static void mock_fade(void *ctx, int m) { (void)ctx; (void)m; }
static bool mock_menu_is_visible(void *ctx) { (void)ctx; return mock_menu_visible != 0; }
static void mock_menu_show(void *ctx) { (void)ctx; mock_menu_visible = 1; }
static void mock_menu_hide(void *ctx) { (void)ctx; mock_menu_visible = 0; }
static void mock_menu_set_event(void *ctx, int16_t event) { (void)ctx; mock_menu_event = event; }
static int16_t mock_menu_get_event(void *ctx) { (void)ctx; return mock_menu_event; }
static void mock_menu_event_loop(void *ctx)
{
    (void)ctx;
    if (mock_menu_event == 0xff) mock_menu_event = mock_menu_pending_event;
}
static void mock_menu_wait(void *ctx) { (void)ctx; }

/* ── Test dialogue part drawing (palette path) ──────────────────────── */

static void test_draw_part_palette(void)
{
    DM2_V1_0aafCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_v1e0a88         = mock_false;
    cb.palette_to_ui8      = mock_palette_to_ui8;
    cb.query_expanded_rect = mock_query_expanded_rect;
    cb.fill_backbuff_rect  = mock_fill_backbuff_rect;
    cb.sleep_several_time  = mock_void_i;
    cb.ctx = NULL;

    mock_fill_count = 0;
    dm2_v1_0aaf_draw_part(&cb, 10, 0);
    assert(mock_fill_count == 1);

    mock_fill_count = 0;
    dm2_v1_0aaf_draw_part(&cb, 10, 1);
    assert(mock_fill_count == 1);

    printf("  PASS: draw_part_palette\n");
}

/* c_0aaf.cpp writes choice bytes at tarr_00+0x28 and reads choice event n
 * from 0x26 + 2*n. The original UI ordinal is one-based. */
static void test_menu_selection_uses_source_stack_offset(void)
{
    DM2_V1_0aafCallbacks cb;
    DM2_V1_0aafMenuReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    cb.query_gdat_text = mock_query_gdat_text;
    cb.query_gdat_entry_data_index = mock_query_gdat_entry_data_index;
    cb.set_v1e0204 = mock_set_i16;
    cb.mode_1031_0675 = mock_mode_i32;
    cb.is_mouse_visible = mock_menu_is_visible;
    cb.show_mouse = mock_menu_show;
    cb.hide_mouse = mock_menu_hide;
    cb.set_event_unk06 = mock_menu_set_event;
    cb.get_event_unk06 = mock_menu_get_event;
    cb.event_loop = mock_menu_event_loop;
    cb.wait_screen_refresh = mock_menu_wait;
    cb.has_key = mock_false;
    cb.set_backbuff2 = mock_set_i16;
    cb.mode_1031_06a5 = mock_void;

    mock_menu_visible = 0;
    mock_menu_pending_event = 1;
    receipt = dm2_v1_0aaf_menu_select(&cb, 0);
    assert(receipt.selection == 1);

    /* An event outside the source's one-based item ordinal cannot borrow
     * stack bytes as a fake menu result. */
    mock_menu_visible = 0;
    mock_menu_pending_event = 4;
    receipt = dm2_v1_0aaf_menu_select(&cb, 0);
    assert(receipt.selection == -1);

    printf("  PASS: menu_selection_uses_source_stack_offset\n");
}

/* ── Test dialogue construction ─────────────────────────────────────── */

static void test_construct_dialogue(void)
{
    DM2_V1_0aafCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.query_gdat_text                = mock_query_gdat_text;
    cb.query_gdat_entry_data_index    = mock_query_gdat_entry_data_index;
    cb.query_gdat_entry_if_loadable   = mock_query_gdat_entry_if_loadable;
    cb.get_v1e0a88                    = mock_false;
    cb.get_gfxalloc_done              = mock_false;
    cb.palette_to_ui8                 = mock_palette_to_ui8;
    cb.query_expanded_rect            = mock_query_expanded_rect;
    cb.fill_backbuff_rect             = mock_fill_backbuff_rect;
    cb.draw_vp_rc_str                 = mock_draw_vp_rc_str;
    cb.get_backbuffer_w               = mock_get_bbw;
    cb.get_backbuffer_h               = mock_get_bbh;
    cb.get_dialog2                    = mock_zero16;
    cb.set_backbuff2                  = mock_set_i16;
    cb.get_v1d1044                    = mock_v1d1044;
    cb.get_table1d27c4                = mock_table_val;
    cb.get_table1d27d4                = mock_table_val;
    cb.sleep_several_time             = mock_void_i;
    cb.draw_gameload_dialogue_to_screen = mock_draw_to_screen;
    cb.fade_screen                    = mock_fade;
    cb.bigpool_available              = mock_bigpool;
    cb.ctx = NULL;

    mock_draw_count = 0;
    DM2_V1_0aafDialogueReceipt r =
        dm2_v1_0aaf_construct_dialogue(&cb, 0x07, 0);

    /* Should have drawn title + text entries */
    assert(mock_draw_count > 0);
    assert(r.dialogue_type == 0x07 || r.dialogue_type == 0x59);

    printf("  PASS: construct_dialogue\n");
}

/* ── Test null safety ───────────────────────────────────────────────── */

static void test_null_safety(void)
{
    DM2_V1_0aafMenuReceipt mr = dm2_v1_0aaf_menu_select(NULL, 0);
    assert(mr.selection == -1);

    dm2_v1_0aaf_draw_part(NULL, 0, 0); /* should not crash */

    DM2_V1_0aafDialogueReceipt dr =
        dm2_v1_0aaf_construct_dialogue(NULL, 0, 0);
    assert(dr.result == 0);

    printf("  PASS: null_safety\n");
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void)
{
    printf("test_dm2_v1_0aaf_pc34_compat:\n");
    test_null_safety();
    test_draw_part_palette();
    test_menu_selection_uses_source_stack_offset();
    test_construct_dialogue();
    printf("All 0AAF tests passed.\n");
    return 0;
}
