/*
 * dm2_v1_ccm_advanced_pc34_compat.c — see header for source anchors.
 *
 * Each function below fails closed (out->valid = 0, no callback
 * invoked) when the callback struct or any callback it needs is NULL,
 * matching the rest of this codebase's fail-closed convention.
 */

#include "dm2_v1_ccm_advanced_pc34_compat.h"

#include <stddef.h>

/* Record-chain terminator (OBJECT_END_MARKER) used by the item-chain
 * walks in TAKES_ITEM / PUTS_DOWN_ITEM. */
#define DM2_V1_CCM_ADV_RECORD_END   0xFFFEu
#define DM2_V1_CCM_ADV_RECORD_NULL  0xFFFFu

/* Bounded walk guard: no proven source chain is longer than this, and
 * the source's own record pools are far smaller — bounding here keeps
 * a corrupt/cyclic chain from hanging the caller (fail-closed, not a
 * silent infinite loop). */
#define DM2_V1_CCM_ADV_MAX_CHAIN 256

const int16_t dm2_v1_ccm_dir_dx[4] = { 0, 1, 0, -1 };  /* table1d27fc */
const int16_t dm2_v1_ccm_dir_dy[4] = { -1, 0, 1, 0 };  /* table1d2804 */

/* ── DM2_CREATURE_CCM03 — c_creature.cpp:1609 ─────────────────────── */
int dm2_v1_ccm_advanced_ccm03(const DM2_V1_CCMAdvancedCallbacks *cb,
                              DM2_V1_CCMAdvancedReceipt *out)
{
    uint8_t phase;
    int32_t result = 0;

    if (!out) return 0;
    out->valid = 0;
    out->result = 0;
    if (!cb || !cb->get_phase || !cb->set_phase || !cb->ccm06 || !cb->walk_now)
        return 0;

    phase = cb->get_phase(cb->ctx);
    if (phase == 0) {
        /* m_19FFD: DM2_CREATURE_CCM06(); RG4L = 0 */
        cb->ccm06(cb->ctx);
        result = 0;
    } else if (phase == 1) {
        /* m_1A006: RG4L = DM2_CREATURE_WALK_NOW() */
        result = cb->walk_now(cb->ctx);
    }
    /* Any other phase keeps the source's uninitialised-but-effectively-0
     * fallthrough to m_1A00D. */

    /* m_1A00D: inc8(byte@0x1f) unconditionally. */
    cb->set_phase(cb->ctx, (uint8_t)(phase + 1));

    out->valid = 1;
    out->result = result;
    return 1;
}

/* ── DM2_CREATURE_JUMPS — c_creature.cpp:1636 ─────────────────────── */
int dm2_v1_ccm_advanced_jumps(const DM2_V1_CCMAdvancedCallbacks *cb,
                              DM2_V1_CCMAdvancedReceipt *out)
{
    uint8_t phase;

    if (!out) return 0;
    out->valid = 0;
    out->result = 0;
    if (!cb || !cb->get_phase || !cb->set_phase || !cb->walk_now ||
        !cb->get_facing || !cb->get_pos_word || !cb->set_pos_word ||
        !cb->get_xy || !cb->move_record_to || !cb->transition_cache_poke ||
        !cb->get_dest_level)
        return 0;

    phase = cb->get_phase(cb->ctx);

    if (phase < 1) {
        /* m_1A041: source only reaches here with phase == 0 (the != 0
         * branch is unreachable for an unsigned byte < 1); phase == 0
         * proceeds. */
        int32_t walk_result;
        uint16_t pos;
        uint8_t facing;
        uint16_t low5, mid5;

        cb->set_phase(cb->ctx, 1);
        walk_result = cb->walk_now(cb->ctx);

        /* Rotate the packed sub-tile position word: low 5 bits get the
         * direction dx, bits [9:5] get the direction dy, both mod 32;
         * bits 10+ are preserved verbatim. */
        pos = cb->get_pos_word(cb->ctx);
        facing = (uint8_t)(cb->get_facing(cb->ctx) & 3);
        low5 = (uint16_t)(((pos & 0x1Fu) + (uint16_t)dm2_v1_ccm_dir_dx[facing]) & 0x1Fu);
        mid5 = (uint16_t)((((pos >> 5) & 0x1Fu) + (uint16_t)dm2_v1_ccm_dir_dy[facing]) & 0x1Fu);
        cb->set_pos_word(cb->ctx, (uint16_t)((pos & 0xFC00u) | (mid5 << 5) | low5));

        out->valid = 1;
        out->result = walk_result;
        return 1;
    }

    if (phase <= 1) {
        /* m_1A0BF */
        int32_t walk_result = cb->walk_now(cb->ctx);
        cb->set_phase(cb->ctx, (uint8_t)(walk_result != 0 ? 2 : 3));
        out->valid = 1;
        out->result = walk_result;
        return 1;
    }

    if (phase != 2) {
        /* source: any phase other than 0/1/2 returns the uninitialised
         * (effectively 0) result without touching state. */
        out->valid = 1;
        out->result = 0;
        return 1;
    }

    /* m_1A0D6: land the party on the creature's current tile. */
    {
        int16_t x, y;
        int moved;

        cb->set_phase(cb->ctx, 3);
        cb->get_xy(cb->ctx, &x, &y);
        moved = cb->move_record_to(cb->ctx, DM2_V1_CCM_ADV_RECORD_NULL,
                                    -1, -1, x, y, 0);
        if (!moved && cb->mark_needs_redraw)
            cb->mark_needs_redraw(cb->ctx);
        cb->transition_cache_poke(cb->ctx, cb->get_dest_level(cb->ctx));
    }

    out->valid = 1;
    out->result = 0;
    return 1;
}

/* ── DM2_CREATURE_TAKES_ITEM — c_creature.cpp:2176 ────────────────── */
int dm2_v1_ccm_advanced_takes_item(const DM2_V1_CCMAdvancedCallbacks *cb,
                                   DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t x, y;
    uint16_t rec;
    uint8_t creature_type;
    uint8_t filter;
    int took_any = 0;
    int guard = 0;

    if (!out) return 0;
    out->valid = 0;
    out->result = 1;
    if (!cb || !cb->get_xy || !cb->get_wall_tile_anyitem_record ||
        !cb->get_next_record_link || !cb->get_record_db_class ||
        !cb->creature_can_handle_it || !cb->move_record_to ||
        !cb->append_record_to || !cb->get_creature_type ||
        !cb->get_item_class_filter)
        return 0;

    cb->get_xy(cb->ctx, &x, &y);
    creature_type = cb->get_creature_type(cb->ctx);
    filter = cb->get_item_class_filter(cb->ctx);

    rec = cb->get_wall_tile_anyitem_record(cb->ctx, x, y);
    while (rec != DM2_V1_CCM_ADV_RECORD_END && guard++ < DM2_V1_CCM_ADV_MAX_CHAIN) {
        uint8_t db_class = cb->get_record_db_class(cb->ctx, rec);
        int skip00385 = 0;

        if (db_class == 4) {
            /* m_1AEB5 top: DB-class 4 (creature) records are always
             * skipped. */
            skip00385 = 1;
        } else if (filter != 0xFFu && db_class != filter) {
            skip00385 = 1;
        }

        if (!skip00385 && cb->creature_can_handle_it(cb->ctx, rec, creature_type)) {
            /* m_1AEF9: move the item onto the creature and append it
             * to the possession chain. */
            cb->move_record_to(cb->ctx, rec, x, y, -1, -1, 0);
            cb->append_record_to(cb->ctx, rec, NULL);
            took_any = 1;

            /* byte@0x1e & 0x40 — "keep picking up" flag. */
            if ((creature_type & 0x40u) == 0)
                break;
        }

        /* m_1AF60 */
        rec = cb->get_next_record_link(cb->ctx, rec);
    }

    out->valid = 1;
    out->result = took_any ? 0 : 1;
    return 1;
}

/* ── DM2_CREATURE_PUTS_DOWN_ITEM — c_creature.cpp:2284 ────────────── */
int dm2_v1_ccm_advanced_puts_down_item(const DM2_V1_CCMAdvancedCallbacks *cb,
                                       DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t x, y;
    uint8_t creature_type;
    uint16_t dropped_count = 0;
    int guard = 0;

    if (!out) return 0;
    out->valid = 0;
    out->result = 0;
    if (!cb || !cb->get_xy || !cb->get_creature_type ||
        !cb->get_possession_head || !cb->can_handle_item_in ||
        !cb->cut_record_from || !cb->move_record_to ||
        !cb->query_cls1_from_record || !cb->query_cls2_from_record ||
        !cb->queue_noise_gen2)
        return 0;

    cb->get_xy(cb->ctx, &x, &y);
    creature_type = cb->get_creature_type(cb->ctx);

    /* m_1AD74 */
    for (;;) {
        uint16_t head, rec;

        if (guard++ >= DM2_V1_CCM_ADV_MAX_CHAIN)
            break;

        head = cb->get_possession_head(cb->ctx);
        rec = cb->can_handle_item_in(cb->ctx, creature_type, head, 0xFFu);
        if (rec == DM2_V1_CCM_ADV_RECORD_END)
            break;

        dropped_count++;
        cb->cut_record_from(cb->ctx, rec, NULL);
        cb->move_record_to(cb->ctx, rec, -1, -1, x, y, 0);

        if (dropped_count == 1) {
            uint8_t cls1 = cb->query_cls1_from_record(cb->ctx, rec);
            uint8_t cls2 = cb->query_cls2_from_record(cb->ctx, rec);

            /* m_1AE43 area: DM2_QUEUE_NOISE_GEN2(cls1, cls2, 0x85,
             * 0xfe, x, y, map, 0x3a, 0x80) — the "item thud" cue. */
            cb->queue_noise_gen2(cb->ctx, (int8_t)cls1, (int8_t)cls2, 0,
                                 0x3A, x, y, 0x85, 0xFE, 0x80);
        }

        /* byte@0x1e & 0x40 — "keep dropping" flag. */
        if ((creature_type & 0x40u) == 0)
            break;
    }

    out->valid = 1;
    out->result = 0;
    return 1;
}

/* ── DM2_CREATURE_TRANSFORM — c_creature.cpp:2637 ─────────────────── */
int dm2_v1_ccm_advanced_transform(const DM2_V1_CCMAdvancedCallbacks *cb,
                                  DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t x, y;
    uint8_t phase;
    uint8_t state;

    if (!out) return 0;
    out->valid = 0;
    out->result = 0;
    if (!cb || !cb->get_xy || !cb->get_phase || !cb->set_phase ||
        !cb->get_state || !cb->set_state || !cb->create_cloud ||
        !cb->queue_noise_gen1 || !cb->randdir || !cb->rand16 ||
        !cb->get_creature_type || !cb->is_creature_allowed_on_level ||
        !cb->get_current_map || !cb->get_dest_level ||
        !cb->delete_creature_record)
        return 0;

    cb->get_xy(cb->ctx, &x, &y);
    phase = cb->get_phase(cb->ctx);      /* vb_04: pre-increment value */
    state = cb->get_state(cb->ctx);      /* byte@0x1a */
    cb->set_phase(cb->ctx, (uint8_t)(phase + 1));

    if (state != 0x3Bu) {
        /* m_1B5D0: single damage cloud + transform noise. */
        cb->create_cloud(cb->ctx, (int16_t)0xFFA8, 0xFF, x, y, 0xFF);
        cb->queue_noise_gen1(cb->ctx, 0x6C, 0x81, 0, 0, 0xC8, x, y, 1);
    } else {
        int16_t cloud_count = (int16_t)(cb->randdir(cb->ctx) + 1);
        int16_t i;

        /* m_1B453 loop */
        for (i = 0; i < cloud_count; i++) {
            int16_t roll = cb->rand16(cb->ctx, 5);
            int16_t dir_or_flag = (roll == 4) ? (int16_t)-1 : roll;
            int16_t magnitude = (int16_t)(cb->rand16(cb->ctx, 156) + 100);

            cb->create_cloud(cb->ctx, (int16_t)0xFF83, magnitude, x, y, dir_or_flag);
        }

        /* m_1B4A2 */
        if ((int8_t)phase >= 1) {
            cb->create_cloud(cb->ctx, (int16_t)0xFFA8,
                             (int16_t)((int8_t)phase * 0x55), x, y, 0xFF);
        }

        if ((int8_t)phase >= 3) {
            /* m_1B505: finish the transform. */
            uint8_t new_type;

            cb->set_state(cb->ctx, 0x3C);
            cb->set_phase(cb->ctx, 0);
            new_type = cb->get_creature_type(cb->ctx);
            (void)new_type; /* AI spec / animation rebind is host-owned */

            if (!cb->is_creature_allowed_on_level(cb->ctx,
                                                  cb->get_current_map(cb->ctx),
                                                  cb->get_dest_level(cb->ctx))) {
                cb->delete_creature_record(cb->ctx, x, y);
                if (cb->mark_needs_redraw)
                    cb->mark_needs_redraw(cb->ctx);
            }
            cb->queue_noise_gen1(cb->ctx, 0x6C, 0x81, 0, 0, 0xC8, x, y, 1);
        } else {
            cb->queue_noise_gen1(cb->ctx, 0x6C, 0x8B, 0, 0, 0xC8, x, y, 1);
        }
    }

    out->valid = 1;
    out->result = 0;
    return 1;
}

/* ── DM2_CREATURE_EXPLODE_OR_SUMMON — c_creature.cpp:2762 ─────────── */
int dm2_v1_ccm_advanced_explode_or_summon(
    const DM2_V1_CCMAdvancedCallbacks *cb,
    uint8_t mode,
    DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t x, y;

    if (!out) return 0;
    out->valid = 0;
    out->result = 0;
    if (!cb || !cb->get_xy || !cb->get_creature_type || !cb->rand16 ||
        !cb->randdir || !cb->create_cloud || !cb->is_creature_allowed_on_level ||
        !cb->get_current_map || !cb->get_dest_level || !cb->delete_creature_record)
        return 0;

    cb->get_xy(cb->ctx, &x, &y);

    if (mode == 0) {
        /* Self-destruct: two additive rand16 rolls off the AI spec
         * power field build the cloud magnitude, clamped to [20,255]
         * by construction (rand16 arguments stay within source bounds). */
        uint16_t power = cb->get_ai_spec_power ? cb->get_ai_spec_power(cb->ctx) : 0;
        int16_t base = (int16_t)(power / 4 + 1);
        int16_t magnitude = (int16_t)(base + cb->rand16(cb->ctx, base));

        magnitude = (int16_t)(magnitude + cb->rand16(cb->ctx, magnitude ? magnitude : 1));
        if (magnitude < 20) magnitude = 20;
        if (magnitude > 255) magnitude = 255;

        cb->create_cloud(cb->ctx, (int16_t)0xFF80, magnitude, x, y, (int16_t)0xFF);

        if ((cb->get_creature_type(cb->ctx) != 0) && cb->kill_on_timer_position)
            cb->kill_on_timer_position(cb->ctx);

        out->valid = 1;
        out->result = 0;
        return 1;
    }

    if (mode == 1) {
        /* Summon: DM2_CREATE_MINION at a random-direction offset.
         * Returns 1 iff creation failed (-1), else 0. */
        int16_t new_rec = -1;

        if (cb->create_minion) {
            new_rec = cb->create_minion(cb->ctx, cb->get_creature_type(cb->ctx),
                                        0x07, cb->randdir(cb->ctx), x, y,
                                        cb->get_current_map ? cb->get_current_map(cb->ctx) : 0,
                                        0xFFFFu, cb->get_dest_level ? cb->get_dest_level(cb->ctx) : 0);
        }

        out->valid = 1;
        out->result = (new_rec == -1) ? 1 : 0;
        return 1;
    }

    if (mode == 2) {
        /* Rebind: switch to state 0x11 and reset phase; drop the
         * record if the (possibly unchanged) creature type is
         * disallowed on the destination level. */
        if (cb->set_state) cb->set_state(cb->ctx, 0x11);
        if (cb->set_phase) cb->set_phase(cb->ctx, 0);

        if (!cb->is_creature_allowed_on_level(cb->ctx,
                                              cb->get_current_map(cb->ctx),
                                              cb->get_dest_level(cb->ctx))) {
            cb->delete_creature_record(cb->ctx, x, y);
            if (cb->mark_needs_redraw)
                cb->mark_needs_redraw(cb->ctx);
        }

        out->valid = 1;
        out->result = 0;
        return 1;
    }

    /* mode > 2: source "no branch taken", returns 0. */
    out->valid = 1;
    out->result = 0;
    return 1;
}

/* ── DM2_1B7D5 (c_creature.cpp:2916-2928) ───────────────────────── */
int dm2_v1_ccm_advanced_1b7d5(const DM2_V1_CCMAdvancedCallbacks *cb,
                              DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t x, y, duration;
    if (!cb || !out) return 0;
    out->valid = 0; out->result = 0;
    if (!cb->get_xy || !cb->rand16 || !cb->create_cloud) return 0;

    cb->get_xy(cb->ctx, &x, &y);
    duration = cb->rand16(cb->ctx, 40) + 20;
    cb->create_cloud(cb->ctx, (int16_t)0xff8e, duration, x, y, (int16_t)0xff);

    out->valid = 1;
    out->result = 0;
    return 1;
}

/* ── DM2_CREATURE_CCM0B (c_creature.cpp:2145-2173) ──────────────── */
int dm2_v1_ccm_advanced_ccm0b(const DM2_V1_CCMAdvancedCallbacks *cb,
                              DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t cx, cy, tx, ty;
    uint16_t spx_0e;
    uint8_t facing, b_1a;
    int32_t r;

    if (!cb || !out) return 0;
    out->valid = 0; out->result = 0;
    if (!cb->get_xy || !cb->get_spx_creature_word_0e || !cb->get_target_x ||
        !cb->get_target_y || !cb->pathfind_0d10 || !cb->get_state ||
        !cb->get_mode_byte || !cb->invoke_message || !cb->get_gametick)
        return 0;

    /* Extract facing from SPX_Creature word@0x0e bits 8-9. */
    spx_0e = cb->get_spx_creature_word_0e(cb->ctx);
    facing = (uint8_t)((spx_0e << 6) >> 14);

    tx = cb->get_target_x(cb->ctx);
    ty = cb->get_target_y(cb->ctx);
    cb->get_xy(cb->ctx, &cx, &cy);

    /* DM2_19f0_0d10: byte@0x20 | 0x80 as flags. */
    r = cb->pathfind_0d10(cb->ctx,
                          (uint8_t)(cb->get_mode_byte(cb->ctx) | 0x80),
                          cx, cy, ty, tx, (int16_t)facing);
    if (r == 0) {
        out->valid = 1;
        out->result = 1;
        return 1;
    }

    b_1a = cb->get_state(cb->ctx);
    if (b_1a != 0x0B) {
        out->valid = 1;
        out->result = 1;
        return 1;
    }

    cb->invoke_message(cb->ctx, ty, tx, 0, 2, cb->get_gametick(cb->ctx));

    out->valid = 1;
    out->result = 0;
    return 1;
}

/* ── DM2_CREATURE_CCM0C (c_creature.cpp:2247-2281) ──────────────── */
int dm2_v1_ccm_advanced_ccm0c(const DM2_V1_CCMAdvancedCallbacks *cb,
                              DM2_V1_CCMAdvancedReceipt *out)
{
    uint8_t phase;
    int32_t result = 0;
    DM2_V1_CCMAdvancedReceipt sub;

    if (!cb || !out) return 0;
    out->valid = 0; out->result = 0;
    if (!cb->get_phase || !cb->set_phase || !cb->ccm06 ||
        !cb->get_spx_creature_word_0e || !cb->set_ai_facing)
        return 0;

    phase = cb->get_phase(cb->ctx);

    if (phase == 0) {
        /* Phase 0: extract facing from SPX_Creature, run CCM06, store
         * the facing result into byte@0x1d. */
        uint16_t spx_0e = cb->get_spx_creature_word_0e(cb->ctx);
        uint8_t facing = (uint8_t)((spx_0e << 6) >> 14);
        cb->ccm06(cb->ctx);
        cb->set_ai_facing(cb->ctx, facing);
    } else if (phase == 1) {
        /* Phase 1: CCM06 then TAKES_ITEM. */
        cb->ccm06(cb->ctx);
        sub.valid = 0; sub.result = 0;
        dm2_v1_ccm_advanced_takes_item(cb, &sub);
        result = sub.result;
    }
    /* else: source falls through with result=0. */

    /* Increment phase unconditionally. */
    cb->set_phase(cb->ctx, (uint8_t)(cb->get_phase(cb->ctx) + 1));

    out->valid = 1;
    out->result = (int32_t)(int16_t)result;
    return 1;
}

/* ── DM2_CREATURE_ACTIVATES_WALL (c_creature.cpp:2564-2633) ──────── */
int dm2_v1_ccm_advanced_activates_wall(const DM2_V1_CCMAdvancedCallbacks *cb,
                                       DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t cx, cy, tx, ty;
    uint8_t b_1e, b_20, facing_raw;
    uint16_t spx_0e, pos_w;
    int32_t r;
    int16_t item_rec;

    if (!cb || !out) return 0;
    out->valid = 0; out->result = 0;
    if (!cb->get_creature_type || !cb->get_spx_creature_word_0e ||
        !cb->get_target_x || !cb->get_target_y || !cb->get_xy ||
        !cb->get_mode_byte || !cb->pathfind_2813 ||
        !cb->wall_activate_event || !cb->get_pos_word ||
        !cb->get_ai_facing || !cb->get_current_map)
        return 0;

    b_1e = cb->get_creature_type(cb->ctx);
    b_20 = cb->get_mode_byte(cb->ctx);

    /* Extract facing from SPX_Creature word@0x0e. */
    spx_0e = cb->get_spx_creature_word_0e(cb->ctx);
    facing_raw = (uint8_t)((spx_0e << 6) >> 14);

    tx = cb->get_target_x(cb->ctx);
    ty = cb->get_target_y(cb->ctx);
    cb->get_xy(cb->ctx, &cx, &cy);

    /* DM2_19f0_2813 pathfind check. */
    r = cb->pathfind_2813(cb->ctx,
                          (uint8_t)(b_20 | 0x80),
                          cx, cy, ty, tx, (int16_t)facing_raw,
                          (int16_t)b_1e);
    if (r == 0) {
        out->valid = 1;
        out->result = 1;
        return 1;
    }

    /* If mode != 2 and creature_type != -1(0xff), find and cut an item. */
    if (b_20 != 2 && b_1e != 0xFF) {
        if (!cb->can_handle_item_in || !cb->cut_record_from ||
            !cb->get_spx_creature_word_02)
        {
            out->valid = 1;
            out->result = 1;
            return 1;
        }
        uint16_t poss_head = cb->get_spx_creature_word_02(cb->ctx);
        item_rec = (int16_t)cb->can_handle_item_in(cb->ctx, b_1e,
                                                    poss_head, 0xFF);
        if (item_rec == (int16_t)0xFFFE) {
            out->valid = 1;
            out->result = 1;
            return 1;
        }
        /* Cut the item from the possession chain. */
        uint16_t head = poss_head;
        cb->cut_record_from(cb->ctx, (uint16_t)item_rec, &head);
    } else {
        item_rec = (int16_t)0xFFFF;
    }

    /* Fire the wall activation event.
     * Direction = (byte@0x1d + 2) & 3.
     * x = pos_word & 0x1f, y = (pos_word << 6) >> 11. */
    {
        uint8_t ai_facing = cb->get_ai_facing(cb->ctx);
        uint8_t dir = (uint8_t)((ai_facing + 2) & 3);
        pos_w = cb->get_pos_word(cb->ctx);
        int16_t px = (int16_t)(pos_w & 0x1F);
        int16_t py = (int16_t)((pos_w << 6) >> 11);
        int16_t creature_rec = (int16_t)cb->get_possession_head(cb->ctx);

        cb->wall_activate_event(cb->ctx, px, py, (int16_t)dir,
                                creature_rec, item_rec);
    }

    out->valid = 1;
    out->result = 0;
    return 1;
}

/* ── DM2_CREATURE_USES_LADDER_HOLE (c_creature.cpp:1709-1861) ───── */
int dm2_v1_ccm_advanced_uses_ladder_hole(const DM2_V1_CCMAdvancedCallbacks *cb,
                                         DM2_V1_CCMAdvancedReceipt *out)
{
    int16_t cx, cy, tx, ty;
    uint8_t b_1a, b_1b;
    uint16_t spx_0e, pos_w;
    int32_t r, dir_rg6;
    void *ladder_record = NULL;
    int32_t pit_tele_flag;

    if (!cb || !out) return 0;
    out->valid = 0; out->result = 0;
    if (!cb->get_xy || !cb->get_state || !cb->get_facing ||
        !cb->get_target_x || !cb->get_target_y || !cb->creature_go_there ||
        !cb->get_mode_byte || !cb->get_creature_capability_bits ||
        !cb->get_spx_creature_word_0e || !cb->get_pos_word ||
        !cb->set_target_x || !cb->set_target_y ||
        !cb->find_ladder_around || !cb->move_record_to ||
        !cb->get_ai_spec_pit_tele_flag ||
        !cb->set_spx_creature_word_0e_facing ||
        !cb->get_dungeon_level)
        return 0;

    b_1b = cb->get_facing(cb->ctx);
    tx = cb->get_target_x(cb->ctx);
    ty = cb->get_target_y(cb->ctx);
    cb->get_xy(cb->ctx, &cx, &cy);

    /* DM2_CREATURE_GO_THERE with byte@0x20 | 0x80 flags. */
    r = cb->creature_go_there(cb->ctx,
                              (uint8_t)(cb->get_mode_byte(cb->ctx) | 0x80),
                              cx, cy, ty, tx, (int16_t)b_1b);
    if (r == 0) {
        out->valid = 1;
        out->result = 1;
        return 1;
    }

    /* Check creature capability bit 0x04. */
    b_1a = cb->get_state(cb->ctx);
    if ((cb->get_creature_capability_bits(cb->ctx, b_1a) & 0x04) == 0) {
        out->valid = 1;
        out->result = 1;
        return 1;
    }

    /* Extract facing from SPX_Creature. */
    spx_0e = cb->get_spx_creature_word_0e(cb->ctx);
    dir_rg6 = (int32_t)((spx_0e << 6) >> 14);

    /* Extract target from pos_word. */
    pos_w = cb->get_pos_word(cb->ctx);
    cb->set_target_y(cb->ctx, (int16_t)(pos_w & 0x1F));
    cb->set_target_x(cb->ctx, (int16_t)((pos_w << 6) >> 11));

    /* Determine ladder direction from b_1a. */
    if (b_1a == 0x39 || b_1a == 0x3A) {
        int16_t ladder_dir = (b_1a == 0x39) ? -1 : 1;
        int32_t find_result;

        cb->get_xy(cb->ctx, &cx, &cy);
        find_result = cb->find_ladder_around(cb->ctx, cx, cy,
                                             ladder_dir, &ladder_record);

        if (ladder_record == NULL) {
            /* No ladder on this level; try adjacent level. */
            if (cb->locate_other_level && cb->change_current_map_to) {
                int16_t lx = cx, ly = cy;
                int32_t cur_map = cb->get_dungeon_level(cb->ctx);
                void *other_rec = NULL;

                int16_t new_map = cb->locate_other_level(cb->ctx, cur_map,
                                                          (int32_t)ladder_dir,
                                                          &lx, &ly, &other_rec);
                cb->change_current_map_to(cb->ctx, (int32_t)new_map);
                find_result = cb->find_ladder_around(cb->ctx, lx, ly,
                                                     (int16_t)(-ladder_dir),
                                                     &ladder_record);
                dir_rg6 = find_result;
                cb->change_current_map_to(cb->ctx, cur_map);

                if ((int16_t)find_result == -1) {
                    dir_rg6 = cb->randdir(cb->ctx);
                }
            } else {
                dir_rg6 = cb->randdir(cb->ctx);
            }
        } else {
            /* Ladder found — check byte@0x04 bit 0x20. */
            if (cb->ladder_record_byte_04) {
                uint8_t b04 = cb->ladder_record_byte_04(cb->ctx, ladder_record);
                if ((b04 & 0x20) == 0) {
                    /* Extract direction from word@0x04 bits. */
                    if (cb->ladder_record_word_04) {
                        uint16_t w04 = cb->ladder_record_word_04(cb->ctx,
                                                                  ladder_record);
                        int32_t extracted = (int32_t)((w04 << 11) >> 14);
                        dir_rg6 = (extracted + find_result) & 3;
                    }
                } else {
                    /* Use word@0x04 direction directly. */
                    if (cb->ladder_record_word_04) {
                        uint16_t w04 = cb->ladder_record_word_04(cb->ctx,
                                                                  ladder_record);
                        dir_rg6 = (int32_t)((w04 << 11) >> 14);
                    }
                }
            }
        }
    }

    /* Check AI spec pit-tele flag (byte@0x09 bit 0x40). */
    pit_tele_flag = (int32_t)cb->get_ai_spec_pit_tele_flag(cb->ctx);
    if (pit_tele_flag != 0 && cb->operate_pit_tele_tile) {
        cb->get_xy(cb->ctx, &cx, &cy);
        cb->operate_pit_tele_tile(cb->ctx, cx, cy, 1);
    }

    /* MOVE_RECORD_TO: remove creature from current position. */
    cb->get_xy(cb->ctx, &cx, &cy);
    cb->move_record_to(cb->ctx, (int16_t)cb->get_possession_head(cb->ctx),
                       cx, cy, -1, 0, 0);

    /* Level transition cache poke. */
    pos_w = cb->get_pos_word(cb->ctx);
    if (cb->level_transition_cache_poke) {
        cb->level_transition_cache_poke(cb->ctx,
                                        (int32_t)(pos_w >> 10));
    }

    /* Check creature state for direction query. */
    if (b_1a == 0x35 || b_1a == 0x36) {
        if (cb->query_cell_direction) {
            ty = cb->get_target_y(cb->ctx);
            tx = cb->get_target_x(cb->ctx);
            dir_rg6 = cb->query_cell_direction(cb->ctx, ty, tx);
        }
    }

    /* Write facing into SPX_Creature. */
    cb->set_spx_creature_word_0e_facing(cb->ctx, (uint8_t)(dir_rg6 & 3));

    /* MOVE_RECORD_TO: place creature at destination. */
    tx = cb->get_target_x(cb->ctx);
    ty = cb->get_target_y(cb->ctx);
    r = cb->move_record_to(cb->ctx, (int16_t)cb->get_possession_head(cb->ctx),
                           -1, 0, ty, tx, 0);

    if (r != 0) {
        if (cb->mark_needs_redraw)
            cb->mark_needs_redraw(cb->ctx);
        out->valid = 1;
        out->result = 0;
        return 1;
    }

    /* Move succeeded — update creature's live position from dest coords. */
    if (cb->get_dest_coords && cb->set_xy) {
        int16_t dx, dy;
        uint8_t dl;
        cb->get_dest_coords(cb->ctx, &dx, &dy, &dl);
        cb->set_xy(cb->ctx, dx, dy);

        if (cb->level_transition_cache_poke)
            cb->level_transition_cache_poke(cb->ctx, (int32_t)dl);
    }

    /* Restore pit-tele tile if needed. */
    if (pit_tele_flag != 0 && cb->operate_pit_tele_tile) {
        cb->get_xy(cb->ctx, &cx, &cy);
        cb->operate_pit_tele_tile(cb->ctx, cx, cy, 0);
    }

    /* Decrement creature counter. */
    if (cb->get_creature_counter && cb->set_creature_counter) {
        uint8_t ctr = cb->get_creature_counter(cb->ctx);
        if (ctr != 0)
            cb->set_creature_counter(cb->ctx, (uint8_t)(ctr - 1));
    }

    out->valid = 1;
    out->result = 0;
    return 1;
}
