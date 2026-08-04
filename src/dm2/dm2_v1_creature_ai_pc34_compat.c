/*
 * dm2_v1_creature_ai_pc34_compat.c — DM2 creature AI subsystem.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp, c_creature.cpp
 *
 * ROTATE_CREATURE (c_creature.cpp:58-101):
 *   - Reads current direction from record word at offset 0xe, bits 14-15
 *   - If rotate_relative: new_dir = (current + delta) & 3
 *   - Writes new direction into bits 14-15 via shift/mask
 *   - If creature AI spec flag bit 0 set, iterates possession chain
 *     at record+2, rotating each possession's direction bits
 *
 * THINK_CREATURE (c_ai.cpp:5649-5850+):
 *   - GET_CREATURE_AT(x, y) to find creature handle
 *   - PREPARE_LOCAL_CREATURE_VAR sets up s350.creatures, s350.v1e054e,
 *     s350.v1e0552 for the AI tick
 *   - Timing: gametick >> 2 modulo creature speed determines action rate
 *   - Self-damage: if accumulated damage (RG62W) > 0, WOUND_CREATURE
 *   - XACT dispatch: reads next action from AI script, dispatches to
 *     PROCEED_XACT_56 through PROCEED_XACT_88
 *
 * All functions are fail-closed until bound to live creature data
 * (record pool, AI spec tables, timer queue).
 */

#include "dm2_v1_creature_ai_pc34_compat.h"

#include <string.h>

int dm2_v1_rotate_creature(
    const DM2_V1_RotateCreatureRequest *request,
    DM2_V1_RotateCreatureReceipt *receipt)
{
    uint8_t *rec;
    uint16_t w0e;
    int16_t old_dir;
    int16_t new_dir;
    int16_t delta;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->creature_handle < 0 || !request->pool_set) {
        receipt->fail_closed = 1;
        return 0;
    }

    rec = dm2_v1_record_pool_address_mut(request->pool_set,
                                          request->creature_handle);
    if (!rec) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_creature.cpp:70-71 — word_at(rec, 0xe) << 6 >> 14 = bits 14-15 >> 8
     * Actually: (word_at(rec,0xe) << 6) >> 14 extracts bits 8-9.
     * But the write path (lines 80-82) does: and8(rec+0xf, 0xfc); or16(rec+0xe, dir<<8)
     * So direction is bits 8-9 of word at 0xe, i.e. byte 0xf bits 0-1. */
    w0e = (uint16_t)rec[0xe] | ((uint16_t)rec[0xf] << 8);
    old_dir = (int16_t)((w0e >> 8) & 3);
    receipt->old_direction = old_dir;

    if (request->rotate_relative) {
        new_dir = (old_dir + request->new_direction) & 3;
    } else {
        new_dir = request->new_direction & 3;
    }
    receipt->new_direction = new_dir;

    /* c_creature.cpp:78 — delta = (new_dir - old_dir) & 3 */
    delta = (new_dir - old_dir) & 3;

    /* c_creature.cpp:80-82 — write direction into byte 0xf bits 0-1,
     * and into word 0xe bits 8-9 */
    rec[0xf] = (uint8_t)((rec[0xf] & 0xFC) | (new_dir & 3));

    /* c_creature.cpp:83-84 — if AI spec flag bit 0 clear, done */
    if (request->query_ai_spec_flags) {
        uint16_t flags = request->query_ai_spec_flags(
            request->ai_spec_ctx, (uint16_t)request->creature_handle);
        if ((flags & 1) == 0)
            return 1;
    } else {
        return 1;
    }

    /* c_creature.cpp:86-99 — rotate each possession in the chain at rec+2.
     * Walk the word-linked list; for each, rotate bits 14-15 by delta. */
    {
        uint8_t *ptr = rec + 2;
        int budget = 256;
        for (;;) {
            uint16_t link_w;
            uint16_t cur_dir;
            uint16_t rotated;
            uint8_t *pos_rec;

            link_w = (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);
            if (link_w == 0xFFFEu)
                break;
            if (--budget <= 0)
                break;

            cur_dir = (link_w >> 14) & 3;
            rotated = (uint16_t)((cur_dir + delta) & 3);
            link_w = (link_w & 0x3FFFu) | (rotated << 14);
            ptr[0] = (uint8_t)(link_w & 0xFF);
            ptr[1] = (uint8_t)(link_w >> 8);
            receipt->possessions_rotated++;

            pos_rec = dm2_v1_record_pool_address_mut(
                request->pool_set, (int16_t)(link_w & 0x3FFFu));
            if (!pos_rec) break;
            ptr = pos_rec;
        }
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* XACT dispatch (c_ai.cpp:2152-2338)                                 */
/* ------------------------------------------------------------------ */

int dm2_v1_xact_dispatch_action(int8_t action_byte,
                                 DM2_V1_XactDispatchReceipt *receipt)
{
    int opt;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->handler_id = -1;

    /* c_ai.cpp:2155 — opt = eaxb - 63; valid range 0..35 maps to XACT 56..91
     * Wait: the code says eaxb-63 but case 0 calls XACT_56. That means
     * the action byte encoding is: action_byte = xact_number + 7.
     * So opt = action_byte - 63, and xact_number = opt + 56. */
    opt = (int)action_byte - 63;
    if (opt < 0 || opt > 35) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->handler_id = opt + 56;

    switch (opt) {
    case 0:  /* XACT 56 — stop */
    case 1:  /* XACT 57 — stop/clear */
    case 3:  /* XACT 59/76 — move forward */
    case 6:  /* XACT 62 — attack */
    case 7:  /* XACT 63 — cast spell */
    case 8:  /* XACT 64 — shoot missile */
    case 9:  /* XACT 65 — flee */
    case 10: /* XACT 66 — move random */
    case 11: /* XACT 67 — move to target */
    case 12: /* XACT 68 — guard position */
    case 14: /* XACT 70 — face party */
    case 15: /* XACT 71 — patrol */
    case 17: /* XACT 73 — spawn */
    case 18: /* XACT 74 — open door */
    case 19: /* XACT 75 — use item */
    case 21: /* XACT 77 — follow */
    case 22: /* XACT 78 — sleep */
    case 24: /* XACT 80 */
    case 25: /* XACT 81 */
    case 26: /* XACT 82 */
    case 27: /* XACT 83 */
    case 28: /* XACT 84 */
    case 29: /* XACT 85 */
    case 33: /* XACT 89 */
    case 34: /* XACT 90 — random */
    case 35: /* XACT 91 — item check */
        receipt->sets_ret = 1;
        break;

    case 2:  /* XACT 58 — inline: b_1a = 19 */
        receipt->inline_action = 1;
        receipt->inline_b_1a = 19;
        break;

    case 4:  /* XACT 60 — inline: b_1a = 0 */
        receipt->inline_action = 1;
        receipt->inline_b_1a = 0;
        break;

    case 5:  /* XACT 61 — nop */
        break;

    case 13: /* XACT 69 — sound (void) */
    case 23: /* XACT 79 — wake (void) */
        break;

    case 16: /* XACT 72 — regenerate (via 72/87/88 handler) */
    case 31: /* XACT 87 — sleep (via 72/87/88 handler) */
    case 32: /* XACT 88 — wake (via 72/87/88 handler) */
        break;

    case 20: /* XACT 76 — move backward: sets v1e0572=-1, v1e0574=0, then calls 59/76 */
        receipt->sets_ret = 1;
        receipt->inline_action = 1;
        receipt->inline_v1e0572 = -1;
        receipt->inline_v1e0574 = 0;
        receipt->handler_id = 76; /* actual handler is 59/76 after state setup */
        break;

    case 30: /* XACT 86 — set state */
        receipt->sets_ret = 1;
        break;

    default:
        receipt->fail_closed = 1;
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_72_87_88 (c_ai.cpp:949-955)                          */
/* ------------------------------------------------------------------ */

int dm2_v1_proceed_xact_72_87_88(const DM2_V1_Xact72_87_88Request *req,
                                  DM2_V1_Xact72_87_88Receipt *receipt)
{
    int8_t n;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:951 — n = CUTX8(s350.v1e0572) */
    n = (int8_t)(req->v1e0572 & 0xFF);

    /* c_ai.cpp:952-953 — if n == -1, use v1e07d8.w_04 instead */
    if (n == -1)
        n = (int8_t)(req->v1e07d8_w04 & 0xFF);

    receipt->b_1a = n;
    return 1;
}

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_86 (c_ai.cpp:2120-2126)                              */
/* ------------------------------------------------------------------ */

int dm2_v1_proceed_xact_86(const DM2_V1_Xact86Request *req,
                            DM2_V1_Xact86Receipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:2122 — b_20 = CUTX8(v1e07d8.w_04) */
    receipt->b_20 = (int8_t)(req->v1e07d8_w04 & 0xFF);

    /* c_ai.cpp:2123 — b_1e = CUTX8(v1e07d8.w_06) */
    receipt->b_1e = (int8_t)(req->v1e07d8_w06 & 0xFF);

    /* c_ai.cpp:2124 — b_1a = CUTX8(v1e0572) + 61 */
    receipt->b_1a = (int8_t)((req->v1e0572 & 0xFF) + 61);

    return 1;
}

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_90 (c_ai.cpp:2136-2138)                              */
/* ------------------------------------------------------------------ */

int dm2_v1_proceed_xact_90(const DM2_V1_Xact90Request *req,
                            DM2_V1_Xact90Receipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:2138 — return (v1e0572 > RAND16(100) ? 1 : 0) - 3
     * So if threshold > rand: 1-3 = -2 (pass); else 0-3 = -3 (fail) */
    receipt->result = (req->v1e0572 > req->rand_value) ? -2 : -3;

    return 1;
}

/* ------------------------------------------------------------------ */
/* PROCEED_XACT_91 (c_ai.cpp:2142-2150)                              */
/* ------------------------------------------------------------------ */

int dm2_v1_proceed_xact_91(const DM2_V1_Xact91Request *req,
                            DM2_V1_Xact91Receipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->can_handle_item) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:2144-2147 — if both checks return -2, return -3; else -2 */
    if (req->can_handle_item(req->ctx, req->v1e0572,
                              req->possession_w00, 0xFF) == -2) {
        if (req->can_handle_item(req->ctx, req->v1e0574,
                                  req->possession_w00, 0xFF) == -2) {
            receipt->result = -3;
            return 1;
        }
    }

    receipt->result = -2;
    return 1;
}

/* ------------------------------------------------------------------ */

int dm2_v1_think_creature(
    const DM2_V1_ThinkCreatureRequest *request,
    DM2_V1_ThinkCreatureReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->tile_x < 0 || request->tile_y < 0) {
        return 0;
    }

    /* GET_CREATURE_AT, PREPARE_LOCAL_CREATURE_VAR, and the entire
     * XACT state machine require live dungeon data, creature records,
     * AI spec tables, and the timer queue. Fail-closed. */
    receipt->fail_closed = 1;

    return 1;
}
