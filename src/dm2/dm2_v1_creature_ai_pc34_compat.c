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
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->creature_handle < 0) {
        return 0;
    }

    receipt->new_direction = request->new_direction & 3;

    /* Record access (GET_ADDRESS_OF_RECORD) and AI spec flag query
     * (QUERY_CREATURE_AI_SPEC_FLAGS) require live record pool.
     * Fail-closed. */
    receipt->fail_closed = 1;

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
