/*
 * dm2_v1_1031_pc34_compat.c -- DM2 segment 1031 UI logic.
 *
 * Ports the UI state machine, gate condition evaluator, recursive UI
 * tree walker, and hit-test system from skproject c_1031.cpp.
 *
 * Source: skproject/SKULLWIN/c_1031.cpp
 */

#include "dm2_v1_1031_pc34_compat.h"

#include <string.h>

/* ── DM2_1031_01d5 — rect query with offset ─────────────────────────── */

/* Source: c_1031.cpp DM2_1031_01d5 */
DM2_V1_1031_Rect *dm2_v1_1031_query_rect_with_offset(
    const DM2_V1_1031_Callbacks *cb,
    int16_t query, DM2_V1_1031_Rect *r)
{
    if (cb == NULL || r == NULL) return NULL;

    int16_t base_query = query & 0x3FFF;
    DM2_V1_1031_Rect *result = cb->query_expanded_rect(cb->ctx, base_query, r);
    if (result == NULL) return NULL;

    /* Check offset flags */
    if ((query & 0x8000) != 0) {
        /* Offset by rect 7 (viewport) */
        int16_t ox, oy;
        cb->query_topleft_of_rect(cb->ctx, 7, &ox, &oy);
        result->x += ox;
        result->y += oy;
    } else if ((query & 0x4000) != 0) {
        /* Offset by rect 18 (panel) */
        int16_t ox, oy;
        cb->query_topleft_of_rect(cb->ctx, 18, &ox, &oy);
        result->x += ox;
        result->y += oy;
    }

    return result;
}

/* ── gate_1031 — condition evaluator ────────────────────────────────── */

/* Source: c_1031.cpp gate_1031 — 12-case UI condition checker */
bool dm2_v1_1031_gate(
    const DM2_V1_1031_Callbacks *cb,
    const DM2_V1_1031_State *state,
    int32_t condition, const DM2_V1_1031_Bbw *entry)
{
    if (cb == NULL || state == NULL || entry == NULL) return false;

    int8_t v = entry->b_01;
    uint16_t uv = (uint16_t)(uint8_t)v;

    switch (condition) {
        case 0: return true;

        case 1: /* DM2_IS_GAME_ENDED */
            return uv == (uint16_t)state->dialog2;

        case 2: /* DM2_1031_0023 */
            return uv == (uint16_t)state->v1e0204;

        case 3: /* DM2_1031_003e */
        {
            if (uv == (uint16_t)state->v1e0976) return true;
            if (uv <= 4) return false;
            if (state->v1e0976 == 0) return true;
            return (uv - 4) != (uint16_t)state->v1e0976;
        }

        case 4: /* hero alive check */
            return cb->get_hero_hp(cb->ctx, (int)v) != 0;

        case 5: /* player at position */
            return cb->get_player_at_position(cb->ctx,
                (int16_t)((uv + state->v1e0258) & 3)) >= 0;

        case 6: /* v1e00b8 flag */
            if (v == 0)
                return state->v1e00b8 == 0;
            return state->v1e00b8 != 0;

        case 7: /* no active hero + position check */
            if (cb->get_curacthero(cb->ctx) != 0) return false;
            if (v > 3) return true;
            return cb->get_player_at_position(cb->ctx,
                (int16_t)((uv + state->v1e0258) & 3)) >= 0;

        case 8: /* active hero matches */
            if (cb->get_curacthero(cb->ctx) == 0) return false;
            return uv == (uint16_t)state->v1e0b7a;

        case 9: /* rune count bit check */
        {
            int16_t hero = cb->get_curacthero(cb->ctx);
            if (hero == 0) return false;
            int32_t rune_mask = 1 << cb->get_hero_nrunes(cb->ctx, hero - 1);
            return (rune_mask & (int32_t)uv) != 0;
        }

        case 10: /* active hero spell state */
            if (cb->get_curacthero(cb->ctx) == 0) return false;
            if ((state->v1e0b62 & 0x800) != 0)
                return uv == (uint16_t)state->v1e0b7a;
            return v == 5;

        case 11: /* v1d67bc match */
            return uv == (uint16_t)state->v1d67bc;

        default: return false;
    }
}

/* ── DM2_1031_027e — recursive visibility marking ───────────────────── */

/* Source: c_1031.cpp DM2_1031_027e */
void dm2_v1_1031_mark_visible(
    const DM2_V1_1031_Callbacks *cb,
    const DM2_V1_1031_State *state,
    const DM2_V1_1031_Bbw *root)
{
    if (cb == NULL || state == NULL || root == NULL) return;

    int16_t list_idx = root->w_02;
    const int8_t *list = &cb->table_cd0[list_idx];

    for (;;) {
        uint8_t entry_idx = (uint8_t)(*list) & 0x7F;
        const DM2_V1_1031_Bbw *child = &cb->table_bbw[entry_idx];
        int32_t cond = (int32_t)(root->b_00 & 0x7F);

        if (dm2_v1_1031_gate(cb, state, cond, child)) {
            if ((child->b_00 & 0x80) != 0) {
                /* Recurse */
                dm2_v1_1031_mark_visible(cb, state, child);
            } else {
                /* Mark element visible */
                int16_t idx = child->w_02;
                if (idx >= 0 && idx < DM2_V1_1031_TABLE_D23_SIZE)
                    cb->table_d23[idx].b_06 |= 0x40;
            }
        }

        list++;
        if ((*list & 0x80) != 0) break;
    }
}

/* ── DM2_1031_030a — recursive hit test ─────────────────────────────── */

/* Source: c_1031.cpp DM2_1031_030a */
DM2_V1_1031_HitTestReceipt dm2_v1_1031_hit_test(
    const DM2_V1_1031_Callbacks *cb,
    const DM2_V1_1031_State *state,
    const DM2_V1_1031_Bbw *root,
    int16_t x, int16_t y, int16_t mask)
{
    DM2_V1_1031_HitTestReceipt receipt;
    receipt.result = 0;

    if (cb == NULL || state == NULL || root == NULL) return receipt;

    int16_t list_idx = root->w_02;
    const int8_t *list = &cb->table_cd0[list_idx];

    for (;;) {
        uint8_t entry_idx = (uint8_t)(*list) & 0x7F;
        const DM2_V1_1031_Bbw *child = &cb->table_bbw[entry_idx];
        int32_t cond = (int32_t)(root->b_00 & 0x7F);

        if (dm2_v1_1031_gate(cb, state, cond, child)) {
            if ((child->b_00 & 0x80) != 0) {
                /* Recurse */
                DM2_V1_1031_HitTestReceipt sub =
                    dm2_v1_1031_hit_test(cb, state, child, x, y, mask);
                if (sub.result != 0) return sub;
            } else {
                /* Check rect */
                int16_t d23_idx = child->w_02;
                if (d23_idx >= 0 && d23_idx < DM2_V1_1031_TABLE_D23_SIZE) {
                    int16_t rect_query = cb->table_d23[d23_idx].w_00;
                    DM2_V1_1031_Rect rc;
                    DM2_V1_1031_Rect *r =
                        dm2_v1_1031_query_rect_with_offset(cb, rect_query, &rc);
                    if (r != NULL) {
                        if (x >= r->x && x < r->x + r->w &&
                            y >= r->y && y < r->y + r->h) {
                            /* Hit — check click rects */
                            int16_t www_idx = cb->table_d23[d23_idx].w_02;
                            if (www_idx >= 0) {
                                receipt.result =
                                    dm2_v1_1031_hit_test_clickrects(
                                        cb, (DM2_V1_1031_State *)state,
                                        &cb->table_338c[www_idx],
                                        x, y, mask);
                                if (receipt.result != 0)
                                    return receipt;
                            }
                        }
                    }
                }
            }
        }

        list++;
        if ((*list & 0x80) != 0) break;
    }

    return receipt;
}

/* ── DM2_10777 — reset UI state ─────────────────────────────────────── */

/* Source: c_1031.cpp DM2_10777 */
void dm2_v1_1031_reset_state(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state)
{
    if (state == NULL) return;

    state->v1e03a8 = 0;
    state->v1e048c = 0;
    state->v1e0478 = 0;

    if (cb != NULL) {
        cb->set_ev_table(cb->ctx, NULL);
        cb->champion_squad_recompute(cb->ctx);
        cb->release_mouse_captures(cb->ctx);
    }
}

/* ── DM2_1031_0541 — main UI state update ───────────────────────────── */

/* Source: c_1031.cpp DM2_1031_0541 */
int32_t dm2_v1_1031_update_state(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state,
    int16_t mode)
{
    if (cb == NULL || state == NULL) return 0;

    /* If mode changed, reset event table */
    if (mode != state->current_mode) {
        /* event_1031_098e equivalent — clear d23 visibility */
        for (int i = 0; i < DM2_V1_1031_TABLE_D23_SIZE; i++)
            cb->table_d23[i].b_06 &= ~0x40;
    }
    state->current_mode = mode;

    /* Mark visible elements via recursive tree walk */
    if (mode >= 0 && mode < DM2_V1_1031_TABLE_ED5_SIZE &&
        cb->table_ed5 != NULL && cb->table_bbw != NULL &&
        cb->table_cd0 != NULL)
        dm2_v1_1031_mark_visible(cb, state,
            &cb->table_ed5[mode]);

    /* Process visibility changes — source: m_1067E..m_1070A */
    for (int i = 0; i < DM2_V1_1031_TABLE_D23_SIZE; i++) {
        uint8_t flags = cb->table_d23[i].b_06;
        bool now_visible = (flags & 0x40) != 0;
        bool was_visible = (flags & 0x80) == 0;

        if (now_visible != was_visible) {
            int16_t click_idx = cb->table_d23[i].w_00 & 0x3F;
            if (was_visible) {
                /* Becoming hidden */
                if (click_idx > 0 && click_idx <= DM2_V1_1031_TABLE_D8_SIZE)
                    cb->table_d8[click_idx - 1].b_03 |= 0x10;
                cb->table_d23[i].b_06 |= 0x80;
            } else {
                /* Becoming visible */
                if (click_idx > 0 && click_idx <= DM2_V1_1031_TABLE_D8_SIZE)
                    cb->table_d8[click_idx - 1].b_03 |= 0x20;
                cb->table_d23[i].b_06 &= ~0x80;
            }
        }
        cb->table_d23[i].b_06 &= ~0x40;
    }

    /* Process click rect visibility changes — source: m_1071F..m_1076A */
    for (int i = 0; i < DM2_V1_1031_TABLE_D8_SIZE; i++) {
        bool needs_show = (cb->table_d8[i].b_03 & 0x10) != 0;
        bool needs_hide = (cb->table_d8[i].b_03 & 0x20) != 0;

        if (needs_show != needs_hide) {
            if (!needs_show)
                cb->refresh_clickrect_1(cb->ctx, i);
            else
                cb->refresh_clickrect_2(cb->ctx, i);
        }
        cb->table_d8[i].b_03 &= 0xCF;
    }

    return 0;
}

/* ── DM2_1031_0675 — switch UI mode ─────────────────────────────────── */

/* Source: c_1031.cpp DM2_1031_0675 */
int32_t dm2_v1_1031_switch_mode(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state,
    int16_t new_mode)
{
    if (state == NULL) return 0;

    state->saved_mode = state->current_mode;
    dm2_v1_1031_reset_state(cb, state);
    return dm2_v1_1031_update_state(cb, state, new_mode);
}

/* ── DM2_1031_06a5 — restore UI mode ───────────────────────────────── */

/* Source: c_1031.cpp DM2_1031_06a5 */
int32_t dm2_v1_1031_restore_mode(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state)
{
    if (state == NULL) return 0;
    return dm2_v1_1031_update_state(cb, state, state->saved_mode);
}

/* ── DM2_1031_0a88 — hit test click rects ───────────────────────────── */

/* Source: c_1031.cpp DM2_1031_0a88 */
int32_t dm2_v1_1031_hit_test_clickrects(
    const DM2_V1_1031_Callbacks *cb,
    DM2_V1_1031_State *state,
    const DM2_V1_1031_Www *entries,
    int16_t x, int16_t y, int16_t mask)
{
    if (cb == NULL || entries == NULL) return 0;

    int32_t result = 0;
    const DM2_V1_1031_Www *first = entries;
    const DM2_V1_1031_Www *e = entries;

    for (;;) {
        /* Check click mask match */
        if ((e->w_04 & 0x08) == 0) {
            int16_t click_mask = (int16_t)(e->w_04 & 0xFF);
            if ((click_mask & mask) != 0) {
                /* Check rect containment */
                DM2_V1_1031_Rect rc;
                DM2_V1_1031_Rect *r =
                    dm2_v1_1031_query_rect_with_offset(cb, e->w_02, &rc);
                if (r != NULL) {
                    if (x >= r->x && x < r->x + r->w &&
                        y >= r->y && y < r->y + r->h) {
                        /* Hit */
                        if (state != NULL) {
                            cb->set_event_unk05(cb->ctx, e->w_02);

                            int16_t unk05 = cb->get_event_unk05(cb->ctx);
                            if ((unk05 & 0x8000) != 0)
                                cb->set_event_unk09(cb->ctx, 7);
                            else if ((unk05 & 0x4000) != 0)
                                cb->set_event_unk09(cb->ctx, 18);
                            else
                                cb->set_event_unk09(cb->ctx, -1);

                            cb->set_event_unk05(cb->ctx,
                                (int16_t)(unk05 & 0x3FFF));
                            cb->set_event_unk08(cb->ctx, x);
                            cb->set_event_unk07(cb->ctx, y);

                            int16_t w0 = e->w_00;
                            w0 = (int16_t)(w0 & 0x07FF); /* mask to ID bits */
                            result = (int32_t)w0;
                            cb->set_event_unk0a(cb->ctx, w0);

                            state->v1e051e = 0;
                            int16_t first_w0 = (int16_t)(first->w_00 & 0x07FF);
                            cb->set_event_unk06(cb->ctx,
                                (int16_t)(result - first_w0 + 1));
                        }
                        return result;
                    }
                }
            }
        }

        /* Next entry */
        e++;
        int16_t next_w0 = e->w_00;
        if ((next_w0 & 0x8000) != 0) {
            /* End of list */
            return result;
        }
    }
}
