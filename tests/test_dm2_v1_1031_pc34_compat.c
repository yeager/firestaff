/*
 * test_dm2_v1_1031_pc34_compat.c -- unit tests for DM2 segment 1031 UI logic.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_1031_pc34_compat.h"

/* ── Mock callbacks ─────────────────────────────────────────────────── */

static DM2_V1_1031_Rect *mock_query_expanded_rect(void *ctx, int16_t query,
    DM2_V1_1031_Rect *r)
{
    (void)ctx; (void)query;
    r->x = 10; r->y = 20; r->w = 100; r->h = 50;
    return r;
}

static int16_t mock_topleft_x, mock_topleft_y;
static void mock_query_topleft(void *ctx, int16_t query,
    int16_t *x, int16_t *y)
{
    (void)ctx; (void)query;
    *x = mock_topleft_x;
    *y = mock_topleft_y;
}

static int16_t mock_curacthero = 0;
static int16_t mock_get_curacthero(void *ctx) { (void)ctx; return mock_curacthero; }
static int16_t mock_get_hero_hp(void *ctx, int idx) { (void)ctx; return idx == 0 ? 100 : 0; }
static int16_t mock_get_hero_nrunes(void *ctx, int idx) { (void)ctx; (void)idx; return 3; }
static int16_t mock_get_player_at_pos(void *ctx, int16_t pos) { (void)ctx; return pos < 2 ? pos : -1; }

static void mock_void(void *ctx) { (void)ctx; }
static void mock_void_ptr(void *ctx, void *p) { (void)ctx; (void)p; }
static void mock_refresh(void *ctx, int i) { (void)ctx; (void)i; }

/* ── Test gate conditions ───────────────────────────────────────────── */

static void test_gate_conditions(void)
{
    DM2_V1_1031_Callbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_curacthero = mock_get_curacthero;
    cb.get_hero_hp = mock_get_hero_hp;
    cb.get_hero_nrunes = mock_get_hero_nrunes;
    cb.get_player_at_position = mock_get_player_at_pos;
    cb.ctx = NULL;

    DM2_V1_1031_State state;
    memset(&state, 0, sizeof(state));
    state.dialog2 = 1;
    state.v1e0204 = 5;
    state.v1e0976 = 3;

    DM2_V1_1031_Bbw entry;

    /* Case 0: always true */
    entry.b_01 = 0;
    assert(dm2_v1_1031_gate(&cb, &state, 0, &entry) == true);

    /* Case 1: dialog2 match */
    entry.b_01 = 1;
    assert(dm2_v1_1031_gate(&cb, &state, 1, &entry) == true);
    entry.b_01 = 0;
    assert(dm2_v1_1031_gate(&cb, &state, 1, &entry) == false);

    /* Case 2: v1e0204 match */
    entry.b_01 = 5;
    assert(dm2_v1_1031_gate(&cb, &state, 2, &entry) == true);
    entry.b_01 = 4;
    assert(dm2_v1_1031_gate(&cb, &state, 2, &entry) == false);

    /* Case 4: hero alive */
    entry.b_01 = 0;
    assert(dm2_v1_1031_gate(&cb, &state, 4, &entry) == true);
    entry.b_01 = 1;
    assert(dm2_v1_1031_gate(&cb, &state, 4, &entry) == false);

    /* Case 6: v1e00b8 flag */
    state.v1e00b8 = 0;
    entry.b_01 = 0;
    assert(dm2_v1_1031_gate(&cb, &state, 6, &entry) == true);
    entry.b_01 = 1;
    assert(dm2_v1_1031_gate(&cb, &state, 6, &entry) == false);

    printf("  PASS: gate_conditions\n");
}

/* ── Test rect with offset ──────────────────────────────────────────── */

static void test_rect_with_offset(void)
{
    DM2_V1_1031_Callbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.query_expanded_rect = mock_query_expanded_rect;
    cb.query_topleft_of_rect = mock_query_topleft;
    cb.ctx = NULL;

    DM2_V1_1031_Rect r;

    /* No offset */
    mock_topleft_x = 0; mock_topleft_y = 0;
    DM2_V1_1031_Rect *result = dm2_v1_1031_query_rect_with_offset(&cb, 5, &r);
    assert(result != NULL);
    assert(result->x == 10);
    assert(result->y == 20);

    /* Viewport offset (0x8000 flag) */
    mock_topleft_x = 30; mock_topleft_y = 40;
    result = dm2_v1_1031_query_rect_with_offset(&cb,
        (int16_t)(5 | (int16_t)0x8000), &r);
    assert(result != NULL);
    assert(result->x == 40); /* 10 + 30 */
    assert(result->y == 60); /* 20 + 40 */

    /* Panel offset (0x4000 flag) */
    mock_topleft_x = 15; mock_topleft_y = 25;
    result = dm2_v1_1031_query_rect_with_offset(&cb, 5 | 0x4000, &r);
    assert(result != NULL);
    assert(result->x == 25); /* 10 + 15 */
    assert(result->y == 45); /* 20 + 25 */

    printf("  PASS: rect_with_offset\n");
}

/* ── Test reset state ───────────────────────────────────────────────── */

static void test_reset_state(void)
{
    DM2_V1_1031_Callbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.set_ev_table = mock_void_ptr;
    cb.champion_squad_recompute = mock_void;
    cb.release_mouse_captures = mock_void;
    cb.ctx = NULL;

    DM2_V1_1031_State state;
    state.v1e03a8 = 5;
    state.v1e048c = 10;
    state.v1e0478 = 15;

    dm2_v1_1031_reset_state(&cb, &state);
    assert(state.v1e03a8 == 0);
    assert(state.v1e048c == 0);
    assert(state.v1e0478 == 0);

    printf("  PASS: reset_state\n");
}

/* ── Test mode switching ────────────────────────────────────────────── */

static void test_mode_switching(void)
{
    DM2_V1_1031_Callbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.set_ev_table = mock_void_ptr;
    cb.champion_squad_recompute = mock_void;
    cb.release_mouse_captures = mock_void;
    cb.refresh_clickrect_1 = mock_refresh;
    cb.refresh_clickrect_2 = mock_refresh;
    cb.ctx = NULL;

    /* Need mutable tables */
    DM2_V1_1031_Wwwb d23[DM2_V1_1031_TABLE_D23_SIZE];
    DM2_V1_1031_ClickNode d8[DM2_V1_1031_TABLE_D8_SIZE];
    DM2_V1_1031_Bbw ed5[DM2_V1_1031_TABLE_ED5_SIZE];
    memset(d23, 0, sizeof(d23));
    memset(d8, 0, sizeof(d8));
    memset(ed5, 0, sizeof(ed5));
    cb.table_d23 = d23;
    cb.table_d8 = d8;
    cb.table_ed5 = ed5;

    DM2_V1_1031_State state;
    memset(&state, 0, sizeof(state));
    state.current_mode = 2;

    dm2_v1_1031_switch_mode(&cb, &state, 4);
    assert(state.saved_mode == 2);
    assert(state.current_mode == 4);

    dm2_v1_1031_restore_mode(&cb, &state);
    assert(state.current_mode == 2);

    printf("  PASS: mode_switching\n");
}

/* ── Test null safety ───────────────────────────────────────────────── */

static void test_null_safety(void)
{
    DM2_V1_1031_Rect r;
    assert(dm2_v1_1031_query_rect_with_offset(NULL, 0, &r) == NULL);
    assert(dm2_v1_1031_gate(NULL, NULL, 0, NULL) == false);

    DM2_V1_1031_HitTestReceipt hr = dm2_v1_1031_hit_test(NULL, NULL, NULL, 0, 0, 0);
    assert(hr.result == 0);

    dm2_v1_1031_reset_state(NULL, NULL); /* should not crash */

    assert(dm2_v1_1031_hit_test_clickrects(NULL, NULL, NULL, 0, 0, 0) == 0);

    printf("  PASS: null_safety\n");
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void)
{
    printf("test_dm2_v1_1031_pc34_compat:\n");
    test_null_safety();
    test_gate_conditions();
    test_rect_with_offset();
    test_reset_state();
    test_mode_switching();
    printf("All 1031 tests passed.\n");
    return 0;
}
