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

/* The record pool is the live runtime view while DungeonData holds the
 * ground-stack structure. SKProject's link word is shared between those
 * views, so every record-link write made by the raw dungeon helpers must be
 * reflected in the live pool before another runtime walk. */
static int move_sync_record_link(DM2_V1_RecordPoolSet *pool_set,
                                 DM2_V1_DungeonData *dungeon,
                                 uint16_t object_id)
{
    uint16_t record_id = (uint16_t)(object_id & 0x3fffu);
    uint8_t *dungeon_record = (uint8_t *)dm2_v1_dungeon_get_thing_record(
        dungeon, record_id, NULL, NULL, NULL);
    uint8_t *pool_record = dm2_v1_record_pool_address_mut(
        pool_set, (int16_t)object_id);
    if (!dungeon_record || !pool_record) return 0;
    memcpy(pool_record, dungeon_record, 2u);
    return 1;
}

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

    /* c_moverec.cpp:392-1100: CUT from source, APPEND to destination.
     * Party moves (0xFFFF) don't touch the record chain. */
    if (receipt->is_party_move) {
        receipt->valid = 1;
        return 1;
    }

    /* CUT from source tile (unless from-nowhere). */
    if (!receipt->is_from_nowhere) {
        DM2_V1_SkprojectCutRecordReceipt cut_rc;
        memset(&cut_rc, 0, sizeof(cut_rc));
        if (dm2_v1_skproject_cut_record_from(
                dungeon, (uint16_t)record_handle, NULL,
                current_map, from_x, from_y, &cut_rc)) {
            receipt->record_cut = 1;
            if (!move_sync_record_link(pool_set, dungeon,
                                       (uint16_t)record_handle) ||
                (cut_rc.tile_chain_unlink_route &&
                 !move_sync_record_link(pool_set, dungeon,
                                        cut_rc.preceding_object_id))) {
                receipt->fail_closed = 1;
            }
        } else {
            receipt->fail_closed = 1;
        }
    }

    /* APPEND to destination tile (unless to-nowhere). */
    if (!receipt->is_to_nowhere && !receipt->fail_closed) {
        DM2_V1_SkprojectAppendRecordReceipt app_rc;
        uint16_t destination_handle = (uint16_t)record_handle;

        /* SKULLWIN/c_move.cpp:2152 and c_moverec.cpp:392 preserve the
         * destination ObjectID's direction bits when inserting the record.
         * A negative direction means the caller did not request a change;
         * otherwise the low two direction bits replace the packed ObjectID
         * orientation before APPEND_RECORD_TO links it into the tile chain. */
        if (dest_direction >= 0) {
            destination_handle = (uint16_t)(
                ((uint16_t)record_handle & 0x3fffu) |
                (((uint16_t)dest_direction & 0x3u) << 14));
        }
        memset(&app_rc, 0, sizeof(app_rc));
        if (dm2_v1_skproject_append_record_to(
                dungeon, destination_handle, NULL,
                current_map, dest_x, dest_y, &app_rc)) {
            receipt->record_appended = 1;
            if (!move_sync_record_link(pool_set, dungeon,
                                       destination_handle) ||
                (app_rc.existing_tile_chain_route &&
                 !move_sync_record_link(pool_set, dungeon,
                                        app_rc.tail_object_id))) {
                receipt->fail_closed = 1;
            }
        } else {
            receipt->fail_closed = 1;
        }
    }

    receipt->valid = 1;
    return 1;
}
