/*
 * test_dm2_v1_tmouse_pc34_compat.c — Tests for DM2 mouse/touch input subsystem.
 *
 * Validates: command queue, mouse queue, Tmouse state machine, event routing,
 * driver interrupt handler, public API wrappers, click-rect hit testing.
 */

#include "dm2_v1_tmouse_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ========================================================================
 * Test helpers — mock callback context
 * ======================================================================== */

typedef struct {
    /* Event queue log */
    int16_t queued_events[64][3]; /* [n][0]=x, [1]=y, [2]=b */
    int     queued_count;

    /* State queries */
    int16_t event_heroidx;
    bool    event_unk0f;
    bool    fetch_busy;

    /* Capture state */
    bool    vcapture1;
    bool    vcapture2;
    bool    vcapture3;
    bool    captures_cleared;

    /* Mouse warp */
    int16_t warped_x;
    int16_t warped_y;
    bool    mouse_warped;

    /* Click-rect data */
    bool    clickrect_set;
    int16_t cr_x0, cr_y0, cr_x1, cr_y1;
} TestCtx;

static void test_queue_event(void *ctx, int16_t x, int16_t y, int16_t b)
{
    TestCtx *t = (TestCtx *)ctx;
    if (t->queued_count < 64) {
        t->queued_events[t->queued_count][0] = x;
        t->queued_events[t->queued_count][1] = y;
        t->queued_events[t->queued_count][2] = b;
        t->queued_count++;
    }
}

static int16_t test_get_event_heroidx(void *ctx)
{
    return ((TestCtx *)ctx)->event_heroidx;
}

static bool test_get_event_unk0f(void *ctx)
{
    return ((TestCtx *)ctx)->event_unk0f;
}

static bool test_get_fetch_busy(void *ctx)
{
    return ((TestCtx *)ctx)->fetch_busy;
}

static void test_set_mouse_pos(void *ctx, int16_t x, int16_t y)
{
    TestCtx *t = (TestCtx *)ctx;
    t->warped_x = x;
    t->warped_y = y;
    t->mouse_warped = true;
}

static bool test_get_vcapture1(void *ctx) { return ((TestCtx *)ctx)->vcapture1; }
static bool test_get_vcapture2(void *ctx) { return ((TestCtx *)ctx)->vcapture2; }
static bool test_get_vcapture3(void *ctx) { return ((TestCtx *)ctx)->vcapture3; }

static void test_clear_vcaptures(void *ctx)
{
    TestCtx *t = (TestCtx *)ctx;
    t->vcapture1 = t->vcapture2 = t->vcapture3 = false;
    t->captures_cleared = true;
}

static void test_set_clickrect_datas(void *ctx, DM2_TmouseClickRectNode *node,
                                      int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    (void)node;
    TestCtx *t = (TestCtx *)ctx;
    t->clickrect_set = true;
    t->cr_x0 = x0; t->cr_y0 = y0;
    t->cr_x1 = x1; t->cr_y1 = y1;
}

static void init_test_ctx(TestCtx *t)
{
    memset(t, 0, sizeof(*t));
    t->event_heroidx = -1; /* E_NOHERO */
}

static DM2_TmouseCallbacks make_callbacks(TestCtx *t)
{
    DM2_TmouseCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx = t;
    cb.queue_event = test_queue_event;
    cb.get_event_heroidx = test_get_event_heroidx;
    cb.get_event_unk0f = test_get_event_unk0f;
    cb.get_fetch_busy = test_get_fetch_busy;
    cb.set_mouse_pos = test_set_mouse_pos;
    cb.get_vcapture1 = test_get_vcapture1;
    cb.get_vcapture2 = test_get_vcapture2;
    cb.get_vcapture3 = test_get_vcapture3;
    cb.clear_vcaptures = test_clear_vcaptures;
    cb.set_clickrect_datas = test_set_clickrect_datas;
    return cb;
}

/* ========================================================================
 * Tests
 * ======================================================================== */

static int passed = 0;
static int failed = 0;

#define TEST(name) static void name(void)
#define RUN(name) do { \
    printf("  %-60s ", #name); \
    name(); \
    printf("PASS\n"); \
    passed++; \
} while(0)

/* ---- Command queue ---- */

TEST(test_command_queue_init)
{
    DM2_TmouseCommandQueue q;
    dm2_v1_tmouse_command_queue_init(&q);
    assert(q.idx_in == 0);
    assert(q.idx_out == 0);
}

TEST(test_command_queue_push_pop)
{
    DM2_TmouseCommandQueue q;
    dm2_v1_tmouse_command_queue_init(&q);

    DM2_TmouseCommandQueuePushReceipt pr = dm2_v1_tmouse_command_queue_push(&q, 42);
    assert(pr.ok);

    DM2_TmouseCommandQueuePopReceipt popr = dm2_v1_tmouse_command_queue_pop(&q);
    assert(popr.ok);
    assert(popr.command.command == 42);
}

TEST(test_command_queue_empty_pop)
{
    DM2_TmouseCommandQueue q;
    dm2_v1_tmouse_command_queue_init(&q);

    DM2_TmouseCommandQueuePopReceipt r = dm2_v1_tmouse_command_queue_pop(&q);
    assert(!r.ok);
}

TEST(test_command_queue_full)
{
    DM2_TmouseCommandQueue q;
    dm2_v1_tmouse_command_queue_init(&q);

    /* Queue holds COMMAND_QUEUE_LENGTH - 1 items (ring buffer) */
    for (int i = 0; i < DM2_TMOUSE_COMMAND_QUEUE_LENGTH - 1; i++) {
        DM2_TmouseCommandQueuePushReceipt r = dm2_v1_tmouse_command_queue_push(&q, (int16_t)i);
        assert(r.ok);
    }
    /* Next push should fail */
    DM2_TmouseCommandQueuePushReceipt r = dm2_v1_tmouse_command_queue_push(&q, 99);
    assert(!r.ok);
}

TEST(test_command_queue_fifo_order)
{
    DM2_TmouseCommandQueue q;
    dm2_v1_tmouse_command_queue_init(&q);

    dm2_v1_tmouse_command_queue_push(&q, 10);
    dm2_v1_tmouse_command_queue_push(&q, 20);
    dm2_v1_tmouse_command_queue_push(&q, 30);

    DM2_TmouseCommandQueuePopReceipt r;
    r = dm2_v1_tmouse_command_queue_pop(&q); assert(r.ok && r.command.command == 10);
    r = dm2_v1_tmouse_command_queue_pop(&q); assert(r.ok && r.command.command == 20);
    r = dm2_v1_tmouse_command_queue_pop(&q); assert(r.ok && r.command.command == 30);
    r = dm2_v1_tmouse_command_queue_pop(&q); assert(!r.ok);
}

/* ---- Mouse queue ---- */

TEST(test_mouse_queue_init)
{
    DM2_TmouseMouseQueue q;
    dm2_v1_tmouse_mouse_queue_init(&q);
    assert(q.counter == 0);
    assert(q.idx_in == 0);
    assert(q.idx_out == 0);
}

TEST(test_mouse_queue_push_pop)
{
    DM2_TmouseMouseQueue q;
    dm2_v1_tmouse_mouse_queue_init(&q);

    DM2_TmouseEventEntry e = {100, 50, 1};
    DM2_TmouseMouseQueuePushReceipt pr = dm2_v1_tmouse_mouse_queue_push(&q, e);
    assert(pr.ok);

    DM2_TmouseMouseQueuePopReceipt popr = dm2_v1_tmouse_mouse_queue_pop(&q);
    assert(popr.ok);
    assert(popr.entry.x == 100);
    assert(popr.entry.y == 50);
    assert(popr.entry.b == 1);
}

TEST(test_mouse_queue_empty_pop)
{
    DM2_TmouseMouseQueue q;
    dm2_v1_tmouse_mouse_queue_init(&q);

    DM2_TmouseMouseQueuePopReceipt r = dm2_v1_tmouse_mouse_queue_pop(&q);
    assert(!r.ok);
}

TEST(test_mouse_queue_full)
{
    DM2_TmouseMouseQueue q;
    dm2_v1_tmouse_mouse_queue_init(&q);

    for (int i = 0; i < DM2_TMOUSE_MOUSE_QUEUE_LENGTH; i++) {
        DM2_TmouseEventEntry e = {(int16_t)i, 0, 0};
        DM2_TmouseMouseQueuePushReceipt r = dm2_v1_tmouse_mouse_queue_push(&q, e);
        assert(r.ok);
    }
    DM2_TmouseEventEntry e2 = {99, 0, 0};
    DM2_TmouseMouseQueuePushReceipt r = dm2_v1_tmouse_mouse_queue_push(&q, e2);
    assert(!r.ok);
}

/* ---- XMouseRect ---- */

TEST(test_xmouserect_init)
{
    DM2_TmouseXMouseRect xr;
    dm2_v1_tmouse_xmouserect_init(&xr);
    assert(xr.rc.x == 0 && xr.rc.y == 0 && xr.rc.w == 0 && xr.rc.h == 0);
    assert(xr.b == 0);
}

/* ---- Tmouse init ---- */

TEST(test_tmouse_init)
{
    DM2_TmouseState s;
    dm2_v1_tmouse_init(&s);

    assert(s.mouse_invisible == 0);
    assert(s.block_mouse_input_counter == 0);
    assert(s.cursor_idx == DM2_TMOUSE_CURSOR0);
    assert(s.mouse_captured_counter == 0);
    assert(s.last_x == 0 && s.last_y == 0);
    assert(s.use_rect2 == false);
    assert(s.mouse_setrect == false);
    assert(s.rectlist1 == NULL);
    assert(s.rectlist2 == NULL);
    assert(s.xmouserect_ptr == &s.rect1);
    assert(dm2_v1_tmouse_is_visible(&s));
}

/* ---- Visibility ---- */

TEST(test_visibility)
{
    DM2_TmouseState s;
    dm2_v1_tmouse_init(&s);

    assert(dm2_v1_tmouse_is_visible(&s));
    dm2_v1_tmouse_hide(&s);
    assert(!dm2_v1_tmouse_is_visible(&s));
}

/* ---- Block/unblock ---- */

TEST(test_block_unblock)
{
    DM2_TmouseState s;
    dm2_v1_tmouse_init(&s);

    dm2_v1_tmouse_block_mouse_input(&s);
    assert(s.block_mouse_input_counter == 1);
    dm2_v1_tmouse_block_mouse_input(&s);
    assert(s.block_mouse_input_counter == 2);
    dm2_v1_tmouse_unblock_mouse_input(&s);
    assert(s.block_mouse_input_counter == 1);
    dm2_v1_tmouse_unblock_mouse_input(&s);
    assert(s.block_mouse_input_counter == 0);
    /* Unblock at 0 should not go negative */
    dm2_v1_tmouse_unblock_mouse_input(&s);
    assert(s.block_mouse_input_counter == 0);
}

/* ---- send_command ---- */

TEST(test_send_command)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);

    dm2_v1_tmouse_send_command(&s, &cq, 5);

    /* Block counter should be back to 0 (block then unblock) */
    assert(s.block_mouse_input_counter == 0);

    /* Command should be in queue */
    DM2_TmouseCommandQueuePopReceipt r = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r.ok);
    assert(r.command.command == 5);
}

/* ---- Command interpreter ---- */

TEST(test_command_interpreter_cmd1)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    s.entry.x = 77;
    s.entry.y = 88;
    dm2_v1_tmouse_command_queue_push(&cq, 1);
    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);

    assert(s.last_x == 77);
    assert(s.last_y == 88);
}

TEST(test_command_interpreter_cmd2)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    s.entry.x = 10;
    s.entry.y = 20;
    s.mouse_captured_counter = 1;
    dm2_v1_tmouse_command_queue_push(&cq, 2);
    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);

    assert(t.mouse_warped);
    assert(t.warped_x == 15);  /* 10 + 5 */
    assert(t.warped_y == 35);  /* 20 + 15 */
    assert(s.mouse_captured_counter == 0);
}

TEST(test_command_interpreter_cmd3_fallthrough_cmd4)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    dm2_v1_tmouse_command_queue_push(&cq, 3);
    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);

    assert(s.mouse_setrect == true);
    assert(s.xmouserect_ptr->b == 0x20);
    assert(s.use_rect2 == true);
}

TEST(test_command_interpreter_cmd5)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    dm2_v1_tmouse_command_queue_push(&cq, 5);
    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);

    assert(s.mouse_captured_counter == 1);
}

TEST(test_command_interpreter_cmd6)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    dm2_v1_tmouse_command_queue_push(&cq, 6);
    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);

    assert(s.cursor_idx == DM2_TMOUSE_CURSOR3);
}

/* ---- Command interpreter: mouse queue processing ---- */

TEST(test_command_interpreter_mouse_queue_left_press)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    /* Push a left-button press event */
    DM2_TmouseEventEntry e = {50, 60, 1}; /* left button down */
    dm2_v1_tmouse_mouse_queue_push(&s.mouse_queue, e);

    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);

    assert(s.entry.x == 50);
    assert(s.entry.y == 60);
    /* Left press: b=1, event code = 2 (left down) */
    assert(t.queued_count >= 1);
    assert(t.queued_events[0][2] == 2);
}

TEST(test_command_interpreter_mouse_queue_right_press)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    /* First release left button (old_mb carries over from prior test due to static) */
    DM2_TmouseEventEntry e0 = {50, 60, 0};
    dm2_v1_tmouse_mouse_queue_push(&s.mouse_queue, e0);
    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);
    t.queued_count = 0; /* clear events from release */

    /* Now push a right-button press event */
    DM2_TmouseEventEntry e = {50, 60, 2}; /* right button down */
    dm2_v1_tmouse_mouse_queue_push(&s.mouse_queue, e);
    dm2_v1_tmouse_command_interpreter(&s, &cq, &cb);

    /* Right press: b=2, event code = 1 (right down) */
    assert(t.queued_count >= 1);
    assert(t.queued_events[0][2] == 1);
}

/* ---- T1_queue_event ---- */

TEST(test_queue_event_below_0x20)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    DM2_TmouseQueueEventReceipt r = dm2_v1_tmouse_queue_event(&s, 10, 20, 4, &cb);
    assert(r.cursor_idx == DM2_TMOUSE_NOCURSOR);
    assert(t.queued_count == 1);
    assert(t.queued_events[0][0] == 10);
    assert(t.queued_events[0][1] == 20);
    assert(t.queued_events[0][2] == 4);
}

TEST(test_queue_event_0x20_no_rectlist)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    /* With mouse_setrect=true and no rectlists, should return CURSOR0 */
    s.mouse_setrect = true;
    DM2_TmouseQueueEventReceipt r = dm2_v1_tmouse_queue_event(&s, 10, 20, 0x20, &cb);
    assert(r.cursor_idx == DM2_TMOUSE_CURSOR0);
}

/* ---- T1_driver_mouseint ---- */

TEST(test_driver_mouseint_accepted)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    DM2_TmouseEventEntry e = {100, 200, 3};
    DM2_TmouseDriverMouseIntReceipt r = dm2_v1_tmouse_driver_mouseint(&s, e, &cb);
    assert(r.accepted);
    assert(s.mouse_queue.counter == 1);
}

TEST(test_driver_mouseint_blocked)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    dm2_v1_tmouse_block_mouse_input(&s);
    DM2_TmouseEventEntry e = {100, 200, 3};
    DM2_TmouseDriverMouseIntReceipt r = dm2_v1_tmouse_driver_mouseint(&s, e, &cb);
    assert(!r.accepted);
    assert(s.mouse_queue.counter == 0);
}

TEST(test_driver_mouseint_fetch_busy)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    t.fetch_busy = true;
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    DM2_TmouseEventEntry e = {100, 200, 3};
    DM2_TmouseDriverMouseIntReceipt r = dm2_v1_tmouse_driver_mouseint(&s, e, &cb);
    assert(!r.accepted);
}

TEST(test_driver_mouseint_captured)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    /* Set capture mode: mouse position locked to entry */
    s.mouse_captured_counter = 1;
    s.entry.x = 50;
    s.entry.y = 60;

    DM2_TmouseEventEntry e = {100, 200, 3};
    DM2_TmouseDriverMouseIntReceipt r = dm2_v1_tmouse_driver_mouseint(&s, e, &cb);
    assert(r.accepted);

    /* The queued event should have the locked position */
    DM2_TmouseMouseQueuePopReceipt pop = dm2_v1_tmouse_mouse_queue_pop(&s.mouse_queue);
    assert(pop.ok);
    assert(pop.entry.x == 50);
    assert(pop.entry.y == 60);
}

/* ---- Public API ---- */

TEST(test_hide_mouse)
{
    DM2_TmouseState s;
    dm2_v1_tmouse_init(&s);

    dm2_v1_tmouse_hide_mouse(&s);
    assert(s.mouse_invisible == 1);
    dm2_v1_tmouse_hide_mouse(&s);
    assert(s.mouse_invisible == 2);
}

TEST(test_show_mouse)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);

    /* Hide twice, then show twice — command 1 only on transition 1->0 */
    dm2_v1_tmouse_hide_mouse(&s);
    dm2_v1_tmouse_hide_mouse(&s);
    assert(s.mouse_invisible == 2);

    dm2_v1_tmouse_show_mouse(&s, &cq);
    assert(s.mouse_invisible == 1);
    /* No command yet — still invisible */
    DM2_TmouseCommandQueuePopReceipt r1 = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(!r1.ok);

    dm2_v1_tmouse_show_mouse(&s, &cq);
    assert(s.mouse_invisible == 0);
    /* Now command 1 should be queued (transition 1->0) */
    DM2_TmouseCommandQueuePopReceipt r2 = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r2.ok);
    assert(r2.command.command == 1);
}

TEST(test_set_capture)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);

    dm2_v1_tmouse_set_capture(&s, &cq);
    DM2_TmouseCommandQueuePopReceipt r = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r.ok && r.command.command == 5);
}

TEST(test_release_capture)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);

    dm2_v1_tmouse_release_capture(&s, &cq);
    DM2_TmouseCommandQueuePopReceipt r = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r.ok && r.command.command == 2);
}

TEST(test_refresh_mouse)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);

    dm2_v1_tmouse_refresh_mouse(&s, &cq);
    /* Should have queued command 3 (and possibly 1 from show_mouse) */
    /* hide increments invisible to 1 */
    /* send_command pushes 3 */
    /* show_mouse decrements invisible to 0 and pushes command 1 */
    DM2_TmouseCommandQueuePopReceipt r1 = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r1.ok && r1.command.command == 3);
    DM2_TmouseCommandQueuePopReceipt r2 = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r2.ok && r2.command.command == 1);
}

TEST(test_choose_cursor3)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);

    dm2_v1_tmouse_choose_cursor3(&s, &cq);
    DM2_TmouseCommandQueuePopReceipt r = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r.ok && r.command.command == 6);
}

TEST(test_release_mouse_captures_active)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    t.vcapture1 = true;
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    dm2_v1_tmouse_release_mouse_captures(&s, &cq, &cb);
    assert(t.captures_cleared);
    /* Should have release_capture (cmd 2) and show_mouse (cmd 1) */
    DM2_TmouseCommandQueuePopReceipt r1 = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r1.ok && r1.command.command == 2);
    DM2_TmouseCommandQueuePopReceipt r2 = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(r2.ok && r2.command.command == 1);
}

TEST(test_release_mouse_captures_none_active)
{
    DM2_TmouseState s;
    DM2_TmouseCommandQueue cq;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    dm2_v1_tmouse_command_queue_init(&cq);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    dm2_v1_tmouse_release_mouse_captures(&s, &cq, &cb);
    assert(!t.captures_cleared);
    /* No commands */
    DM2_TmouseCommandQueuePopReceipt r = dm2_v1_tmouse_command_queue_pop(&cq);
    assert(!r.ok);
}

TEST(test_get_mouse_entry_data)
{
    DM2_TmouseState s;
    dm2_v1_tmouse_init(&s);
    s.entry.x = 123;
    s.entry.y = 456;
    s.entry.b = 3;

    DM2_TmouseGetEntryDataReceipt r = dm2_v1_tmouse_get_mouse_entry_data(&s);
    assert(r.mx == 123);
    assert(r.my == 456);
    assert(r.mb == 3);
    /* Block counter back to 0 */
    assert(s.block_mouse_input_counter == 0);
}

/* ---- Click-rect hit testing (T1_queue_0x20) ---- */

TEST(test_queue_0x20_no_rectlist)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    s.mouse_setrect = true;
    DM2_TmouseCursorIdx ci = dm2_v1_tmouse_queue_0x20(&s, 100, 100, &cb);
    assert(ci == DM2_TMOUSE_CURSOR0);
}

TEST(test_queue_0x20_with_hit)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    /* Set up a single click-rect node */
    DM2_TmouseClickRectData data;
    data.next = NULL;
    data.r.x = 50; data.r.y = 50; data.r.w = 100; data.r.h = 100;

    DM2_TmouseClickRectNode node;
    node.w_00 = 0;
    node.b_02 = 1;      /* cursor type = 1 -> CURSOR0/1/2 depending on hero */
    node.b_03 = 0;
    node.buttons = 0;
    node.b_05 = 0;
    node.node = &data;

    s.rectlist1 = NULL;
    s.rectlist2 = &node;
    s.mouse_setrect = true;

    /* Hit inside the rect */
    t.event_heroidx = -1; /* E_NOHERO */
    DM2_TmouseCursorIdx ci = dm2_v1_tmouse_queue_0x20(&s, 75, 75, &cb);

    /* mouse_unk0b=1, heroidx=-1 -> CURSOR0 */
    assert(ci == DM2_TMOUSE_CURSOR0);
    assert(t.clickrect_set);
}

TEST(test_queue_0x20_cursor3_on_type2)
{
    DM2_TmouseState s;
    TestCtx t;
    dm2_v1_tmouse_init(&s);
    init_test_ctx(&t);
    DM2_TmouseCallbacks cb = make_callbacks(&t);

    DM2_TmouseClickRectData data;
    data.next = NULL;
    data.r.x = 0; data.r.y = 0; data.r.w = 320; data.r.h = 200;

    DM2_TmouseClickRectNode node;
    node.w_00 = 0;
    node.b_02 = 2;      /* cursor type = 2 -> CURSOR3 */
    node.b_03 = 0;
    node.buttons = 0;
    node.b_05 = 0;
    node.node = &data;

    s.rectlist1 = NULL;
    s.rectlist2 = &node;
    s.mouse_setrect = true;

    DM2_TmouseCursorIdx ci = dm2_v1_tmouse_queue_0x20(&s, 160, 100, &cb);
    assert(ci == DM2_TMOUSE_CURSOR3);
}

/* ========================================================================
 * Main
 * ======================================================================== */

int main(void)
{
    printf("test_dm2_v1_tmouse_pc34_compat:\n");

    /* Command queue */
    RUN(test_command_queue_init);
    RUN(test_command_queue_push_pop);
    RUN(test_command_queue_empty_pop);
    RUN(test_command_queue_full);
    RUN(test_command_queue_fifo_order);

    /* Mouse queue */
    RUN(test_mouse_queue_init);
    RUN(test_mouse_queue_push_pop);
    RUN(test_mouse_queue_empty_pop);
    RUN(test_mouse_queue_full);

    /* XMouseRect */
    RUN(test_xmouserect_init);

    /* Tmouse init */
    RUN(test_tmouse_init);

    /* Visibility */
    RUN(test_visibility);

    /* Block/unblock */
    RUN(test_block_unblock);

    /* send_command */
    RUN(test_send_command);

    /* Command interpreter */
    RUN(test_command_interpreter_cmd1);
    RUN(test_command_interpreter_cmd2);
    RUN(test_command_interpreter_cmd3_fallthrough_cmd4);
    RUN(test_command_interpreter_cmd5);
    RUN(test_command_interpreter_cmd6);
    RUN(test_command_interpreter_mouse_queue_left_press);
    RUN(test_command_interpreter_mouse_queue_right_press);

    /* T1_queue_event */
    RUN(test_queue_event_below_0x20);
    RUN(test_queue_event_0x20_no_rectlist);

    /* T1_driver_mouseint */
    RUN(test_driver_mouseint_accepted);
    RUN(test_driver_mouseint_blocked);
    RUN(test_driver_mouseint_fetch_busy);
    RUN(test_driver_mouseint_captured);

    /* Public API */
    RUN(test_hide_mouse);
    RUN(test_show_mouse);
    RUN(test_set_capture);
    RUN(test_release_capture);
    RUN(test_refresh_mouse);
    RUN(test_choose_cursor3);
    RUN(test_release_mouse_captures_active);
    RUN(test_release_mouse_captures_none_active);
    RUN(test_get_mouse_entry_data);

    /* Click-rect hit testing */
    RUN(test_queue_0x20_no_rectlist);
    RUN(test_queue_0x20_with_hit);
    RUN(test_queue_0x20_cursor3_on_type2);

    printf("\n  %d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
