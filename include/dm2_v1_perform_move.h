#ifndef FIRESTAFF_DM2_V1_PERFORM_MOVE_H
#define FIRESTAFF_DM2_V1_PERFORM_MOVE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM2_V1_PERFORM_MOVE_BLOCK_NONE = 0,
    DM2_V1_PERFORM_MOVE_BLOCK_NOT_READY,
    DM2_V1_PERFORM_MOVE_BLOCK_COOLDOWN,
    DM2_V1_PERFORM_MOVE_BLOCK_NO_TARGET_TILE,
    DM2_V1_PERFORM_MOVE_BLOCK_WALL,
    DM2_V1_PERFORM_MOVE_BLOCK_PIT,
    DM2_V1_PERFORM_MOVE_BLOCK_LAVA,
    DM2_V1_PERFORM_MOVE_BLOCK_INACCESSIBLE,
    DM2_V1_PERFORM_MOVE_BLOCK_DOOR_CLOSED
} DM2_V1_PerformMoveBlockReason;

typedef struct {
    int runtime_ready;
    int can_move;
    int outdoor;
    int current_level;
    int from_x;
    int from_y;
    int from_dir;
    int direction;
    int target_raw_valid;
    int target_raw;
    int target_square_type;
    int target_is_door;
    int target_door_state;
    /* Source-owned open pit transition has resolved a destination through
     * DM2_LOCATE_OTHER_LEVEL; it is not ordinary pit passability. */
    int target_pit_transition_admitted;
} DM2_V1_PerformMoveRequest;

typedef struct {
    int valid;
    int accepted;
    int blocked;
    DM2_V1_PerformMoveBlockReason block_reason;
    int current_level;
    int from_x;
    int from_y;
    int from_dir;
    int to_x;
    int to_y;
    int to_dir;
    int outdoor;
    int target_raw_valid;
    int target_raw;
    int target_square_type;
    int target_is_door;
    int target_door_state;
    int target_pit_transition_admitted;
    uint32_t movement_hash;
} DM2_V1_PerformMoveReceipt;

int dm2_v1_DM2_PERFORM_MOVE_plan(
    const DM2_V1_PerformMoveRequest *request,
    DM2_V1_PerformMoveReceipt *out);

const char *dm2_v1_DM2_PERFORM_MOVE_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PERFORM_MOVE_H */
