#ifndef FIRESTAFF_DM2_V1_MOVE_075F_1BC2_H
#define FIRESTAFF_DM2_V1_MOVE_075F_1BC2_H

#include <stdint.h>

#include "dm2_v1_dungeon_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM2_V1_MOVE_075F_1BC2_BLOCK_NONE = 0,
    DM2_V1_MOVE_075F_1BC2_BLOCK_NO_DUNGEON,
    DM2_V1_MOVE_075F_1BC2_BLOCK_NO_SOURCE_TILE,
    DM2_V1_MOVE_075F_1BC2_BLOCK_NO_TARGET_TILE,
    DM2_V1_MOVE_075F_1BC2_BLOCK_WALL,
    DM2_V1_MOVE_075F_1BC2_BLOCK_CLOSED_DOOR,
    DM2_V1_MOVE_075F_1BC2_BLOCK_PIT,
    DM2_V1_MOVE_075F_1BC2_BLOCK_LAVA,
    DM2_V1_MOVE_075F_1BC2_BLOCK_INACCESSIBLE,
    /* c_move.cpp:2861 needs party-position and RNG owners.  It is not a
     * dungeon collision routine, so never substitute a target-tile result. */
    DM2_V1_MOVE_075F_1BC2_BLOCK_SOURCE_STATE_UNBOUND
} DM2_V1_Move075f1bc2BlockReason;

typedef enum {
    DM2_V1_MOVE_075F_1BC2_VERTICAL_NONE = 0,
    DM2_V1_MOVE_075F_1BC2_VERTICAL_UP = -1,
    DM2_V1_MOVE_075F_1BC2_VERTICAL_DOWN = 1
} DM2_V1_Move075f1bc2VerticalKind;

typedef struct {
    int valid;
    int source_state_unbound;
    int accepted;
    int blocked;
    DM2_V1_Move075f1bc2BlockReason block_reason;
    int level;
    int from_x;
    int from_y;
    int from_dir;
    int move_dir;
    int to_x;
    int to_y;
    int source_raw_valid;
    int source_raw;
    int source_square_type;
    int target_raw_valid;
    int target_raw;
    int target_square_type;
    int target_first_thing;
    int target_is_outdoor;
    int target_is_door;
    int target_door_state;
    DM2_V1_Move075f1bc2VerticalKind vertical_kind;
    uint32_t movement_hash;
} DM2_V1_Move075f1bc2Receipt;

int dm2_v1_DM2_move_075f_1bc2_target_receipt(
    const DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    int facing_dir,
    int move_dir,
    DM2_V1_Move075f1bc2Receipt *out);

const char *dm2_v1_DM2_move_075f_1bc2_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MOVE_075F_1BC2_H */
