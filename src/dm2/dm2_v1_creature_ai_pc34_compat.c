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
