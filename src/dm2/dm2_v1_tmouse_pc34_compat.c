/*
 * dm2_v1_tmouse_pc34_compat.c — DM2 mouse/touch input subsystem.
 *
 * Source: skproject c_tmouse.cpp (598 lines).
 * Ported to pure C with callback-based architecture.
 */

#include "dm2_v1_tmouse_pc34_compat.h"
#include <string.h>

/* ========================================================================
 * Helper — clamp to min/max (DM2_MAX / DM2_MIN from skproject)
 * ======================================================================== */

static inline int16_t tmouse_max(int16_t a, int16_t b) { return a > b ? a : b; }
static inline int16_t tmouse_min(int16_t a, int16_t b) { return a < b ? a : b; }

/* ========================================================================
 * Command queue
 * ======================================================================== */

void dm2_v1_tmouse_command_queue_init(DM2_TmouseCommandQueue *q)
{
    q->idx_in  = 0;
    q->idx_out = 0;
    memset(q->queue, 0, sizeof(q->queue));
}

DM2_TmouseCommandQueuePushReceipt
dm2_v1_tmouse_command_queue_push(DM2_TmouseCommandQueue *q, int16_t command)
{
    DM2_TmouseCommandQueuePushReceipt r;
    int16_t i = q->idx_in + 1;
    if (i == DM2_TMOUSE_COMMAND_QUEUE_LENGTH)
        i = 0;
    if (i == q->idx_out) {
        r.ok = false;
        return r;
    }
    q->queue[q->idx_in].command = command;
    q->idx_in = i;
    r.ok = true;
    return r;
}

DM2_TmouseCommandQueuePopReceipt
dm2_v1_tmouse_command_queue_pop(DM2_TmouseCommandQueue *q)
{
    DM2_TmouseCommandQueuePopReceipt r;
    if (q->idx_in == q->idx_out) {
        r.ok = false;
        r.command.command = 0;
        return r;
    }
    r.command = q->queue[q->idx_out];
    int16_t i = q->idx_out + 1;
    if (i == DM2_TMOUSE_COMMAND_QUEUE_LENGTH)
        i = 0;
    q->idx_out = i;
    r.ok = true;
    return r;
}

/* ========================================================================
 * Mouse queue
 * ======================================================================== */

void dm2_v1_tmouse_mouse_queue_init(DM2_TmouseMouseQueue *q)
{
    q->counter = 0;
    q->idx_in  = 0;
    q->idx_out = 0;
    memset(q->queue, 0, sizeof(q->queue));
}

DM2_TmouseMouseQueuePushReceipt
dm2_v1_tmouse_mouse_queue_push(DM2_TmouseMouseQueue *q, DM2_TmouseEventEntry entry)
{
    DM2_TmouseMouseQueuePushReceipt r;
    if (q->counter >= DM2_TMOUSE_MOUSE_QUEUE_LENGTH) {
        r.ok = false;
        return r;
    }
    q->counter++;
    q->idx_in++;
    if (q->idx_in == DM2_TMOUSE_MOUSE_QUEUE_LENGTH)
        q->idx_in = 0;
    q->queue[q->idx_in] = entry;
    r.ok = true;
    return r;
}

DM2_TmouseMouseQueuePopReceipt
dm2_v1_tmouse_mouse_queue_pop(DM2_TmouseMouseQueue *q)
{
    DM2_TmouseMouseQueuePopReceipt r;
    if (q->counter == 0) {
        r.ok = false;
        memset(&r.entry, 0, sizeof(r.entry));
        return r;
    }
    q->counter--;
    q->idx_out++;
    if (q->idx_out == DM2_TMOUSE_MOUSE_QUEUE_LENGTH)
        q->idx_out = 0;
    r.entry = q->queue[q->idx_out];
    r.ok = true;
    return r;
}

/* ========================================================================
 * XMouseRect
 * ======================================================================== */

void dm2_v1_tmouse_xmouserect_init(DM2_TmouseXMouseRect *xr)
{
    memset(&xr->rc, 0, sizeof(xr->rc));
    xr->b = 0;
}

/* ========================================================================
 * Tmouse init — c_Tmouse::init()
 * ======================================================================== */

void dm2_v1_tmouse_init(DM2_TmouseState *state)
{
    state->mouse_invisible = 0;
    state->block_mouse_input_counter = 0;

    /* Mouserect defaults */
    state->mouse_rect.x = 0;
    state->mouse_rect.y = 0;
    state->mouse_rect.w = 12;
    state->mouse_rect.h = 16;

    state->use_rect2 = false;

    /* Entry */
    memset(&state->entry, 0, sizeof(state->entry));
    state->last_x = 0;
    state->last_y = 0;

    /* Cursor */
    state->cursor_idx = DM2_TMOUSE_CURSOR0;

    /* Capture */
    state->mouse_captured_counter = 0;

    /* Mouse queue */
    dm2_v1_tmouse_mouse_queue_init(&state->mouse_queue);

    /* Click-rect state */
    state->mouse_setrect = false;
    state->mouse_rx0 = 0;
    state->mouse_rx1 = 0;
    state->mouse_ry0 = 0;
    state->mouse_ry1 = 0;

    state->rectlist1 = NULL;
    state->rectlist2 = NULL;

    memset(&state->dummy_rect, 0, sizeof(state->dummy_rect));
    dm2_v1_tmouse_xmouserect_init(&state->rect1);
    dm2_v1_tmouse_xmouserect_init(&state->rect2);

    state->xmouserect_ptr = &state->rect1;
}

/* ========================================================================
 * Visibility / blocking
 * ======================================================================== */

bool dm2_v1_tmouse_is_visible(const DM2_TmouseState *state)
{
    return state->mouse_invisible == 0;
}

void dm2_v1_tmouse_hide(DM2_TmouseState *state)
{
    state->mouse_invisible = 1;
}

void dm2_v1_tmouse_block_mouse_input(DM2_TmouseState *state)
{
    state->block_mouse_input_counter++;
}

void dm2_v1_tmouse_unblock_mouse_input(DM2_TmouseState *state)
{
    if (state->block_mouse_input_counter > 0)
        state->block_mouse_input_counter--;
}

/* ========================================================================
 * send_command — block, push, unblock
 * ======================================================================== */

void dm2_v1_tmouse_send_command(DM2_TmouseState *state,
                                DM2_TmouseCommandQueue *cq,
                                int16_t command)
{
    dm2_v1_tmouse_block_mouse_input(state);
    /* Original spins until push succeeds; we just push once (queue is large enough) */
    dm2_v1_tmouse_command_queue_push(cq, command);
    dm2_v1_tmouse_unblock_mouse_input(state);
}

/* ========================================================================
 * T1_queue_0x20 — click-rect hit-test and cursor determination
 * Source: c_Tmouse::T1_queue_0x20 (c_tmouse.cpp:240-323)
 * ======================================================================== */

DM2_TmouseCursorIdx
dm2_v1_tmouse_queue_0x20(DM2_TmouseState *state,
                          int16_t x, int16_t y,
                          const DM2_TmouseCallbacks *cb)
{
    /* Static in original — we keep it in function scope via static */
    static int16_t mouse_unk0b = 0;

    if (   state->mouse_setrect
        || x < state->mouse_rx0
        || x > state->mouse_rx1
        || y < state->mouse_ry0
        || y > state->mouse_ry1)
    {
        state->mouse_setrect = false;
        mouse_unk0b = 0;
        DM2_TmouseClickRectNode *node = state->rectlist2;
        int16_t x0 = 0;
        int16_t y0 = 0;
        int16_t x1 = DM2_TMOUSE_ORIG_SWIDTH;
        int16_t y1 = DM2_TMOUSE_ORIG_SHEIGHT;

        while (node != NULL) {
            DM2_TmouseRect *r = &node->node->r;
            bool xflag = false;
            if (x >= r->x) {
                if (x <= (r->x + r->w - 1)) {
                    xflag = true;
                    x0 = tmouse_max(x0, r->x);
                    x1 = tmouse_min(x1, r->x + r->w - 1);
                } else {
                    x0 = tmouse_max(x0, r->x + r->w - 1);
                }
            } else {
                x1 = tmouse_min(x1, r->x);
            }

            if (y < r->y) {
                y1 = tmouse_min(y1, r->y);
                node = node->node->next;
                continue;
            }

            if (y <= (r->y + r->h - 1)) {
                y0 = tmouse_max(y0, r->y);
                y1 = tmouse_min(y1, r->y + r->h - 1);
                if (!xflag) {
                    node = node->node->next;
                    continue;
                }
                mouse_unk0b = (int16_t)(uint8_t)node->b_02;
                if (node != state->rectlist1) {
                    int8_t bbuttons = node->buttons;
                    if (bbuttons != 0 && cb->queue_event)
                        cb->queue_event(cb->ctx, x, y, (int16_t)(uint8_t)bbuttons);
                }
                break;
            } else {
                y0 = tmouse_max(y0, r->y + r->h - 1);
                node = node->node->next;
            }
        }

        if (state->rectlist1 != NULL && state->rectlist1 != node) {
            if (state->rectlist1->b_05 != 0 && cb->queue_event)
                cb->queue_event(cb->ctx, x, y,
                                (int16_t)(uint8_t)state->rectlist1->b_05);
        }

        if (node != NULL && cb->set_clickrect_datas)
            cb->set_clickrect_datas(cb->ctx, node, x0, y0, x1, y1);
    }

    if (mouse_unk0b == 2)
        return DM2_TMOUSE_CURSOR3;
    if (mouse_unk0b != 1 || (cb->get_event_heroidx &&
                              cb->get_event_heroidx(cb->ctx) == -1))
        return DM2_TMOUSE_CURSOR0;
    if (cb->get_event_unk0f && !cb->get_event_unk0f(cb->ctx))
        return DM2_TMOUSE_CURSOR1;
    return DM2_TMOUSE_CURSOR2;
}

/* ========================================================================
 * T1_queue_event — event routing
 * Source: c_Tmouse::T1_queue_event (c_tmouse.cpp:326-344)
 * ======================================================================== */

DM2_TmouseQueueEventReceipt
dm2_v1_tmouse_queue_event(DM2_TmouseState *state,
                           int16_t x, int16_t y, int16_t b,
                           const DM2_TmouseCallbacks *cb)
{
    DM2_TmouseQueueEventReceipt r;
    state->entry.x = x;
    state->entry.y = y;
    state->entry.b = b;

    if (b >= 0x20) {
        r.cursor_idx = dm2_v1_tmouse_queue_0x20(state, x, y, cb);
    } else {
        if (cb->queue_event)
            cb->queue_event(cb->ctx, x, y, b);
        r.cursor_idx = DM2_TMOUSE_NOCURSOR;
    }
    return r;
}

/* ========================================================================
 * Command interpreter — processes command queue + mouse queue
 * Source: c_Tmouse::command_interpreter (c_tmouse.cpp:59-139)
 * ======================================================================== */

void dm2_v1_tmouse_command_interpreter(DM2_TmouseState *state,
                                       DM2_TmouseCommandQueue *cq,
                                       const DM2_TmouseCallbacks *cb)
{
    /* Process command queue */
    for (;;) {
        DM2_TmouseCommandQueuePopReceipt pop = dm2_v1_tmouse_command_queue_pop(cq);
        if (!pop.ok)
            break;

        switch (pop.command.command) {
        case 1:
            /* T1_drawmouse — update last position */
            state->last_x = state->entry.x;
            state->last_y = state->entry.y;
            break;
        case 2:
            /* Release capture: warp mouse, decrement counter */
            if (cb->set_mouse_pos)
                cb->set_mouse_pos(cb->ctx,
                                  state->entry.x + 5,
                                  state->entry.y + 15);
            state->mouse_captured_counter--;
            break;
        case 3:
            /* Refresh mouse: set rect, clear xmouserect, fall through to case 4 */
            state->mouse_setrect = true;
            state->xmouserect_ptr->rc = state->dummy_rect;
            state->xmouserect_ptr->b = 0x20;
            /* FALLTHROUGH */
        case 4:
            /* Copy rect1 -> rect2, enable rect2 */
            state->use_rect2 = false;
            state->rect2 = state->rect1;
            state->use_rect2 = true;
            break;
        case 5:
            /* Set capture */
            state->mouse_captured_counter++;
            break;
        case 6:
            /* Choose cursor 3 */
            state->cursor_idx = DM2_TMOUSE_CURSOR3;
            break;
        case 7:
            /* DM2_DRAWINGS_COMPLETED — no-op in skproject */
            break;
        }
    }

    /* Process mouse queue (was T1_execmousefifo / sub_3586) */
    static int16_t old_mb = 0;

    for (;;) {
        DM2_TmouseMouseQueuePopReceipt mpop = dm2_v1_tmouse_mouse_queue_pop(&state->mouse_queue);
        if (!mpop.ok)
            break;

        DM2_TmouseEventEntry me = mpop.entry;

        state->entry.x = me.x;
        state->entry.y = me.y;
        if ((me.x != state->last_x || me.y != state->last_y) &&
            dm2_v1_tmouse_is_visible(state))
        {
            state->last_x = me.x;
            state->last_y = me.y;
        }
        state->entry.b = me.b;

        int16_t mouse_b_change = me.b ^ old_mb;
        if (mouse_b_change & 0x1) {
            dm2_v1_tmouse_queue_event(state, me.x, me.y,
                                      (me.b & 0x1) ? 2 : 4, cb);
        }
        if (mouse_b_change & 0x2) {
            dm2_v1_tmouse_queue_event(state, me.x, me.y,
                                      (me.b & 0x2) ? 1 : 8, cb);
        }
        old_mb = me.b;
    }
}

/* ========================================================================
 * T1_driver_mouseint — driver interrupt handler
 * Source: c_Tmouse::T1_driver_mouseint (c_tmouse.cpp:419-432)
 * ======================================================================== */

DM2_TmouseDriverMouseIntReceipt
dm2_v1_tmouse_driver_mouseint(DM2_TmouseState *state,
                               DM2_TmouseEventEntry entry,
                               const DM2_TmouseCallbacks *cb)
{
    DM2_TmouseDriverMouseIntReceipt r;

    if (state->block_mouse_input_counter ||
        (cb->get_fetch_busy && cb->get_fetch_busy(cb->ctx)))
    {
        r.accepted = false;
        return r;
    }

    if (state->mouse_captured_counter != 0) {
        entry.x = state->entry.x;
        entry.y = state->entry.y;
    }

    dm2_v1_tmouse_mouse_queue_push(&state->mouse_queue, entry);
    r.accepted = true;
    return r;
}

/* ========================================================================
 * Public API wrappers
 * ======================================================================== */

/* DM2_HIDE_MOUSE (c_tmouse.cpp:538-541) */
void dm2_v1_tmouse_hide_mouse(DM2_TmouseState *state)
{
    state->mouse_invisible++;
}

/* DM2_SHOW_MOUSE (c_tmouse.cpp:544-549) */
void dm2_v1_tmouse_show_mouse(DM2_TmouseState *state,
                               DM2_TmouseCommandQueue *cq)
{
    if (state->mouse_invisible > 0) {
        if (state->mouse_invisible-- == 1)
            dm2_v1_tmouse_send_command(state, cq, 1);
    }
}

/* DM2_MOUSE_SET_CAPTURE (c_tmouse.cpp:552-555) */
void dm2_v1_tmouse_set_capture(DM2_TmouseState *state,
                                DM2_TmouseCommandQueue *cq)
{
    dm2_v1_tmouse_send_command(state, cq, 5);
}

/* DM2_MOUSE_RELEASE_CAPTURE (c_tmouse.cpp:558-562) */
void dm2_v1_tmouse_release_capture(DM2_TmouseState *state,
                                    DM2_TmouseCommandQueue *cq)
{
    dm2_v1_tmouse_send_command(state, cq, 2);
}

/* DM2_REFRESHMOUSE (c_tmouse.cpp:565-570) */
void dm2_v1_tmouse_refresh_mouse(DM2_TmouseState *state,
                                  DM2_TmouseCommandQueue *cq)
{
    dm2_v1_tmouse_hide_mouse(state);
    dm2_v1_tmouse_send_command(state, cq, 3);
    dm2_v1_tmouse_show_mouse(state, cq);
}

/* DM2_CHOOSE_CURSOR3 (c_tmouse.cpp:573-576) */
void dm2_v1_tmouse_choose_cursor3(DM2_TmouseState *state,
                                   DM2_TmouseCommandQueue *cq)
{
    dm2_v1_tmouse_send_command(state, cq, 6);
}

/* DM2_RELEASE_MOUSE_CAPTURES (c_tmouse.cpp:579-588) */
void dm2_v1_tmouse_release_mouse_captures(DM2_TmouseState *state,
                                           DM2_TmouseCommandQueue *cq,
                                           const DM2_TmouseCallbacks *cb)
{
    bool v1 = cb->get_vcapture1 ? cb->get_vcapture1(cb->ctx) : false;
    bool v2 = cb->get_vcapture2 ? cb->get_vcapture2(cb->ctx) : false;
    bool v3 = cb->get_vcapture3 ? cb->get_vcapture3(cb->ctx) : false;

    if (v1 || v2 || v3) {
        if (cb->clear_vcaptures)
            cb->clear_vcaptures(cb->ctx);
        dm2_v1_tmouse_release_capture(state, cq);
        dm2_v1_tmouse_hide(state);
        dm2_v1_tmouse_show_mouse(state, cq);
    }
}

/* DM2_GET_MOUSE_ENTRY_DATA (c_tmouse.cpp:591-598) */
DM2_TmouseGetEntryDataReceipt
dm2_v1_tmouse_get_mouse_entry_data(DM2_TmouseState *state)
{
    DM2_TmouseGetEntryDataReceipt r;
    dm2_v1_tmouse_block_mouse_input(state);
    r.mx = state->entry.x;
    r.my = state->entry.y;
    r.mb = state->entry.b;
    dm2_v1_tmouse_unblock_mouse_input(state);
    return r;
}
