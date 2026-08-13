#include "dm2_v1_perform_move.h"

#include "dm2_v1_world_model.h"

#include <string.h>

static uint32_t dm2_perform_move_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

int dm2_v1_DM2_PERFORM_MOVE_plan(
    const DM2_V1_PerformMoveRequest *request,
    DM2_V1_PerformMoveReceipt *out)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int dir;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!request) return 0;

    dir = request->direction & 3;
    out->valid = 1;
    out->current_level = request->current_level;
    out->from_x = request->from_x;
    out->from_y = request->from_y;
    out->from_dir = request->from_dir & 3;
    out->to_x = request->from_x + dx[dir];
    out->to_y = request->from_y + dy[dir];
    out->to_dir = dir;
    out->outdoor = request->outdoor ? 1 : 0;
    out->target_raw_valid = request->target_raw_valid ? 1 : 0;
    out->target_raw = request->target_raw;
    out->target_square_type = request->target_square_type;
    out->target_is_door = request->target_is_door ? 1 : 0;
    out->target_door_state = request->target_door_state;
    out->target_pit_transition_admitted =
        request->target_pit_transition_admitted ? 1 : 0;

    if (!request->runtime_ready) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_NOT_READY;
    } else if (!request->can_move) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_COOLDOWN;
    } else if (!out->outdoor && !out->target_raw_valid) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_NO_TARGET_TILE;
    } else if (!out->outdoor &&
               (request->target_square_type == DM2_SQUARE_WALL ||
                request->target_square_type == DM2_SQUARE_SECRET_DOOR ||
                request->target_square_type == DM2_SQUARE_FAKE_WALL)) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_WALL;
    } else if (!out->outdoor && request->target_square_type == 5 &&
               !out->target_pit_transition_admitted) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_PIT;
    } else if (!out->outdoor && request->target_square_type == 11) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_LAVA;
    } else if (!out->outdoor && request->target_square_type == 13) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_INACCESSIBLE;
    } else if (!out->outdoor && request->target_is_door &&
               request->target_door_state != 0) {
        out->blocked = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_DOOR_CLOSED;
    } else {
        out->accepted = 1;
        out->block_reason = DM2_V1_PERFORM_MOVE_BLOCK_NONE;
    }

    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->current_level);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->from_x);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->from_y);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->from_dir);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->to_x);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->to_y);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->to_dir);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->outdoor);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->target_raw_valid);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->target_raw);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->target_square_type);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->target_is_door);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->target_door_state);
    hash = dm2_perform_move_hash_step(
        hash, (uint32_t)out->target_pit_transition_admitted);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->block_reason);
    hash = dm2_perform_move_hash_step(hash, (uint32_t)out->accepted);
    if (hash == 0u) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->movement_hash = hash;
    return 1;
}

const char *dm2_v1_DM2_PERFORM_MOVE_source_evidence(void)
{
    return "skproject SKULLWIN/c_move.cpp DM2_PERFORM_MOVE: direction is "
           "normalized to N/E/S/W, target square is selected from party "
           "position, dungeon movement is blocked by unavailable target, "
           "walls, pits, lava, inaccessible squares, and non-open doors, "
           "while outdoor movement bypasses dungeon tile blocking.";
}
