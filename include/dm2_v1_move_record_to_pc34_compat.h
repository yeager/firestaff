#ifndef FIRESTAFF_DM2_V1_MOVE_RECORD_TO_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_MOVE_RECORD_TO_PC34_COMPAT_H

/*
 * dm2_v1_move_record_to_pc34_compat.h — DM2 record relocation primitive.
 *
 * Ports DM2_MOVE_RECORD_TO from skproject/SKULLWIN/c_moverec.cpp:392.
 * This function cuts a record from one tile's record chain and appends
 * it to another tile's chain, handling creature level restrictions,
 * cross-map moves, and arrival/departure actuator triggers.
 *
 * The function also handles the "from nowhere" case (record_handle==0xFFFF)
 * where a new party record is placed on a tile, and the "to nowhere" case
 * (dest_x < 0) where a record is removed from a tile chain.
 *
 * Source: skproject/SKULLWIN/c_moverec.cpp DM2_MOVE_RECORD_TO
 */

#include <stdint.h>

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_timeline.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    int fail_closed;

    int16_t record_handle;
    int16_t from_x;
    int16_t from_y;
    int16_t dest_x;
    int16_t dest_y;
    int16_t dest_direction;

    int is_party_move;
    int is_creature_move;
    int is_from_nowhere;
    int is_to_nowhere;
    int is_same_tile;
    int cross_map;

    int creature_level_blocked;
    int arrival_actuator_fired;
    int departure_actuator_fired;
    int record_cut;
    int record_appended;
} DM2_V1_MoveRecordToReceipt;

/* DM2_MOVE_RECORD_TO — relocate a record between tile chains.
 *
 * record_handle: the object record handle (0xFFFF = party/self).
 *   Bits 10-13 encode the DB type (4 = creature).
 * from_x, from_y: source tile (-1 = from nowhere / new placement).
 * dest_x: destination x (-1 = to nowhere / removal).
 * dest_y: destination y.
 * dest_direction: direction on destination tile.
 *
 * Returns 1 on success, 0 on fail-closed.
 *
 * Source: c_moverec.cpp:392-1100 */
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
    DM2_V1_MoveRecordToReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MOVE_RECORD_TO_PC34_COMPAT_H */
