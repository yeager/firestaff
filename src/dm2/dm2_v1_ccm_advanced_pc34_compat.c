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
