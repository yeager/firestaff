/*
 * test_dm2_v1_input_pc34_compat.c — unit tests for DM2 input dispatcher
 * and bytecode event interpreter.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "dm2_v1_input_pc34_compat.h"

/* ── tracking helpers ────────────────────────────────────────────── */

typedef struct {
    int called;
    int16_t arg1;
    int16_t arg2;
    int16_t arg3;
} TrackRecord;

static TrackRecord g_track;

static void reset_track(void)
{
    memset(&g_track, 0, sizeof(g_track));
}

static void cb_perform_turn(void *ctx, int16_t dir)
{
    (void)ctx;
    g_track.called = 1;
    g_track.arg1 = dir;
}

static void cb_perform_move(void *ctx, int16_t dir)
{
    (void)ctx;
    g_track.called = 1;
    g_track.arg1 = dir;
}

static void cb_click_item_slot(void *ctx, int16_t slot)
{
    (void)ctx;
    g_track.called = 1;
    g_track.arg1 = slot;
}

static void cb_release_mouse(void *ctx)
{
    (void)ctx;
    g_track.called = 1;
}

static void cb_try_cast_spell(void *ctx)
{
    (void)ctx;
    g_track.called = 1;
}

static void cb_click_viewport(void *ctx, int16_t x, int16_t y)
{
    (void)ctx;
    g_track.called = 1;
    g_track.arg1 = x;
    g_track.arg2 = y;
}

/* exec_event tracking */
typedef struct {
    int set_delay_called;
    uint8_t set_delay_val;
    int terminate_count;
} ExecTrack;

static ExecTrack g_exec;

static void reset_exec_track(void)
{
    memset(&g_exec, 0, sizeof(g_exec));
}

static void cb_set_delay(void *ctx, uint8_t delay)
{
    (void)ctx;
    g_exec.set_delay_called = 1;
    g_exec.set_delay_val = delay;
}

/* ── tests ───────────────────────────────────────────────────────── */

static void test_handle_ui_event_turn(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));
    cb.perform_turn = cb_perform_turn;
    req.event_idx = 1;
    reset_track();

    dm2_v1_handle_ui_event(&cb, NULL, &req, &receipt);
    assert(g_track.called == 1);
    assert(g_track.arg1 == 1);
    assert(receipt.handled == 1);
    assert(receipt.dispatch_category == 1);
    assert(receipt.dispatched_idx == 1);

    printf("  PASS: handle_ui_event_turn\n");
}

static void test_handle_ui_event_move(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));
    cb.perform_move = cb_perform_move;
    req.event_idx = 4;
    reset_track();

    dm2_v1_handle_ui_event(&cb, NULL, &req, &receipt);
    assert(g_track.called == 1);
    assert(g_track.arg1 == 4);
    assert(receipt.handled == 1);
    assert(receipt.dispatch_category == 2);

    printf("  PASS: handle_ui_event_move\n");
}

static void test_handle_ui_event_item_slot(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));
    cb.click_item_slot = cb_click_item_slot;
    req.event_idx = 30;
    reset_track();

    dm2_v1_handle_ui_event(&cb, NULL, &req, &receipt);
    assert(g_track.called == 1);
    assert(g_track.arg1 == 10);
    assert(receipt.handled == 1);
    assert(receipt.dispatch_category == 3);

    printf("  PASS: handle_ui_event_item_slot\n");
}

static void test_handle_ui_event_release_mouse(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));
    cb.release_mouse = cb_release_mouse;
    req.event_idx = 227;
    reset_track();

    dm2_v1_handle_ui_event(&cb, NULL, &req, &receipt);
    assert(g_track.called == 1);
    assert(receipt.handled == 1);
    assert(receipt.dispatch_category == 27);

    printf("  PASS: handle_ui_event_release_mouse\n");
}

static void test_handle_ui_event_null_safety(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));

    /* NULL request */
    dm2_v1_handle_ui_event(&cb, NULL, NULL, &receipt);
    /* should not crash, receipt untouched */

    /* NULL receipt */
    dm2_v1_handle_ui_event(&cb, NULL, &req, NULL);
    /* should not crash */

    printf("  PASS: handle_ui_event_null_safety\n");
}

static void test_handle_ui_event_unknown(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));
    req.event_idx = 999;

    dm2_v1_handle_ui_event(&cb, NULL, &req, &receipt);
    assert(receipt.handled == 0);
    assert(receipt.dispatched_idx == 999);

    printf("  PASS: handle_ui_event_unknown\n");
}

static void test_exec_event_terminate(void)
{
    DM2_V1_ExecEventCallbacks cb;
    memset(&cb, 0, sizeof(cb));

    int8_t bytecode[] = {(int8_t)0x80};
    reset_exec_track();

    dm2_v1_exec_event(&cb, NULL, bytecode);
    assert(g_exec.set_delay_called == 0);

    printf("  PASS: exec_event_terminate\n");
}

static void test_exec_event_set_delay(void)
{
    DM2_V1_ExecEventCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.set_delay = cb_set_delay;

    /* bit6=1 (continue), opcode=0 (set_delay), arg=5, then terminate */
    int8_t bytecode[] = {0x40, 0x05, (int8_t)0x80};
    reset_exec_track();

    dm2_v1_exec_event(&cb, NULL, bytecode);
    assert(g_exec.set_delay_called == 1);
    assert(g_exec.set_delay_val == 5);

    printf("  PASS: exec_event_set_delay\n");
}

static void test_exec_event_null_safety(void)
{
    DM2_V1_ExecEventCallbacks cb;
    memset(&cb, 0, sizeof(cb));

    dm2_v1_exec_event(&cb, NULL, NULL);
    dm2_v1_exec_event(NULL, NULL, NULL);

    printf("  PASS: exec_event_null_safety\n");
}

static void test_table1d3efd_size(void)
{
    assert(sizeof(dm2_v1_table1d3efd) == 236);

    printf("  PASS: table1d3efd_size\n");
}

static void test_1031_03f2_basic(void)
{
    int16_t tree[] = {10, 20, 30, 40, 50};
    int16_t result;

    result = dm2_v1_1031_03f2(30, tree, 5);
    assert(result == 2);

    result = dm2_v1_1031_03f2(99, tree, 5);
    assert(result == -1);

    result = dm2_v1_1031_03f2(10, NULL, 5);
    assert(result == -1);

    result = dm2_v1_1031_03f2(10, tree, 0);
    assert(result == -1);

    printf("  PASS: 1031_03f2_basic\n");
}

static void test_handle_ui_event_spell(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));
    cb.try_cast_spell = cb_try_cast_spell;
    req.event_idx = 108;
    reset_track();

    dm2_v1_handle_ui_event(&cb, NULL, &req, &receipt);
    assert(g_track.called == 1);
    assert(receipt.handled == 1);
    assert(receipt.dispatch_category == 10);

    printf("  PASS: handle_ui_event_spell\n");
}

static void test_handle_ui_event_viewport(void)
{
    DM2_V1_HandleUiEventCallbacks cb;
    DM2_V1_HandleUiEventRequest req;
    DM2_V1_HandleUiEventReceipt receipt;

    memset(&cb, 0, sizeof(cb));
    memset(&req, 0, sizeof(req));
    cb.click_viewport = cb_click_viewport;
    req.event_idx = 80;
    req.x = 42;
    req.y = 77;
    reset_track();

    dm2_v1_handle_ui_event(&cb, NULL, &req, &receipt);
    assert(g_track.called == 1);
    assert(g_track.arg1 == 42);
    assert(g_track.arg2 == 77);
    assert(receipt.handled == 1);
    assert(receipt.dispatch_category == 19);

    printf("  PASS: handle_ui_event_viewport\n");
}

int main(void)
{
    printf("dm2_v1_input_pc34_compat tests:\n");

    test_handle_ui_event_turn();
    test_handle_ui_event_move();
    test_handle_ui_event_item_slot();
    test_handle_ui_event_release_mouse();
    test_handle_ui_event_null_safety();
    test_handle_ui_event_unknown();
    test_exec_event_terminate();
    test_exec_event_set_delay();
    test_exec_event_null_safety();
    test_table1d3efd_size();
    test_1031_03f2_basic();
    test_handle_ui_event_spell();
    test_handle_ui_event_viewport();

    printf("All 13 tests passed.\n");
    return 0;
}
