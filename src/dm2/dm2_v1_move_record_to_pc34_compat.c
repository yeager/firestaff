/*
 * dm2_v1_move_record_to_pc34_compat.c — DM2 record relocation primitive.
 *
 * Source: skproject/SKULLWIN/c_moverec.cpp:392-1100
 *
 * DM2_MOVE_RECORD_TO cuts a record from one tile's linked-list chain
 * and appends it to another tile's chain.  The full function handles:
 *
 *   - Party move (handle == 0xFFFF): updates party position globals
 *   - Creature move: validates level restriction, drops possessions
 *     if creature not allowed on destination level
 *   - Cross-map: CHANGE_CURRENT_MAP_TO destination, re-resolve tile
 *   - Same-tile: early return (no cut/append needed)
 *   - From-nowhere (from_x == -1): append only, no cut
 *   - To-nowhere (dest_x < 0): cut only, no append
 *   - Arrival/departure actuator triggers via ACTUATE_FLOOR_MECHA
 *   - Sound effect on creature arrival (type 0x5d timer)
 *
 * The record chain manipulation itself uses:
 *   DM2_2fcf_0234: departure-side tile link update
 *   DM2_move_2fcf_0434: arrival-side tile link insertion
 *   DM2_CUT_RECORD_FROM: unlinks record from source chain
 *   DM2_APPEND_RECORD_TO: links record into destination chain
 *
 * Fail-closed: the record chain manipulation and actuator dispatch
 * are not yet bound.  This module classifies the move and returns
 * a receipt; the runtime fires actuators separately.
 */

#include "dm2_v1_move_record_to_pc34_compat.h"

#include <string.h>

#define MOVE_DB_CREATURE 4

int dm2_v1_move_record_to(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_SourceTimerQueue *queue,
    int16_t record_handle,
    int16_t from_x,
    int16_t from_y,
    int16_t dest_x,
    int16_t dest_y,
    int16_t dest_direction,
    int current_map,
    uint32_t game_tick,
    DM2_V1_MoveRecordToReceipt *receipt)
{
    (void)queue;
    (void)current_map;
    (void)game_tick;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!pool_set || !dungeon) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;
    receipt->record_handle = record_handle;
    receipt->from_x = from_x;
    receipt->from_y = from_y;
    receipt->dest_x = dest_x;
    receipt->dest_y = dest_y;
    receipt->dest_direction = dest_direction;

    receipt->is_party_move = (record_handle == (int16_t)0xFFFF);
    receipt->is_from_nowhere = (from_x < 0);
    receipt->is_to_nowhere = (dest_x < 0);

    if (record_handle != (int16_t)0xFFFF) {
        int db_type = (int)((record_handle >> 10) & 0xF);
        receipt->is_creature_move = (db_type == MOVE_DB_CREATURE);
    }

    if (!receipt->is_from_nowhere && !receipt->is_to_nowhere &&
        from_x == dest_x && from_y == dest_y) {
        receipt->is_same_tile = 1;
        return 1;
    }

    /* Departure/arrival actuator triggers (ACTUATE_FLOOR_MECHA) are
     * fired by the runtime's existing movement path
     * (dm2_v1_runtime_invoke_square_actuators), not here.
     *
     * The record chain cut/append (CUT_RECORD_FROM + APPEND_RECORD_TO)
     * requires live pool data.  Fail-closed until the record chain
     * manipulation is bound to the tile linked-list heads. */
    receipt->fail_closed = 1;

    return 1;
}
