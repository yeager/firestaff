#include "dm2_v1_move_075f_1bc2.h"

#include "dm2_v1_world_model.h"

#include <string.h>

static uint32_t dm2_move_075f_1bc2_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static int dm2_move_075f_1bc2_is_open_door(int raw)
{
    return (raw & 0x07) == 0;
}

static void dm2_move_075f_1bc2_classify_target(
    DM2_V1_Move075f1bc2Receipt *out)
{
    if (!out->target_raw_valid) {
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_NO_TARGET_TILE;
        return;
    }

    if (out->target_square_type == DM2_SQUARE_WALL ||
        out->target_square_type == DM2_SQUARE_SECRET_DOOR ||
        out->target_square_type == DM2_SQUARE_FAKE_WALL) {
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_WALL;
    } else if (out->target_square_type == DM2_SQUARE_DOOR &&
               !dm2_move_075f_1bc2_is_open_door(out->target_raw)) {
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_CLOSED_DOOR;
    } else if (out->target_square_type == DM2_SQUARE_PIT) {
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_PIT;
    } else if (out->target_square_type == DM2_SQUARE_LAVA) {
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_LAVA;
    } else if (out->target_square_type == DM2_SQUARE_INACCESSIBLE) {
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_INACCESSIBLE;
    } else {
        out->accepted = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_NONE;
    }
}

int dm2_v1_DM2_move_075f_1bc2_target_receipt(
    const DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    int facing_dir,
    int move_dir,
    DM2_V1_Move075f1bc2Receipt *out)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int dir;
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!dungeon || !dungeon->raw_data) {
        out->valid = 1;
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_NO_DUNGEON;
        return 1;
    }

    dir = move_dir & 3;
    out->valid = 1;
    out->level = level;
    out->from_x = x;
    out->from_y = y;
    out->from_dir = facing_dir & 3;
    out->move_dir = dir;
    out->to_x = x + dx[dir];
    out->to_y = y + dy[dir];
    out->source_raw = dm2_v1_dungeon_get_tile_raw(dungeon, level, x, y);
    out->source_square_type =
        dm2_v1_dungeon_get_square_type(dungeon, level, x, y);

    if (out->source_raw < 0 || out->source_square_type < 0) {
        out->blocked = 1;
        out->block_reason = DM2_V1_MOVE_075F_1BC2_BLOCK_NO_SOURCE_TILE;
    } else {
        out->source_raw_valid = 1;
        out->target_raw =
            dm2_v1_dungeon_get_tile_raw(dungeon, level, out->to_x, out->to_y);
        out->target_square_type = dm2_v1_dungeon_get_square_type(
            dungeon, level, out->to_x, out->to_y);
        if (out->target_raw >= 0 && out->target_square_type >= 0) {
            out->target_raw_valid = 1;
            out->target_first_thing = dm2_v1_dungeon_get_first_thing(
                dungeon, level, out->to_x, out->to_y);
            out->target_is_outdoor =
                dm2_v1_dungeon_is_outdoor(dungeon, level) ? 1 : 0;
            out->target_is_door =
                out->target_square_type == DM2_SQUARE_DOOR ? 1 : 0;
            out->target_door_state = out->target_is_door
                                         ? (out->target_raw & 0x07)
                                         : -1;
            if (out->target_square_type == DM2_SQUARE_STAIRS_UP) {
                out->vertical_kind = DM2_V1_MOVE_075F_1BC2_VERTICAL_UP;
            } else if (out->target_square_type == DM2_SQUARE_STAIRS_DOWN) {
                out->vertical_kind = DM2_V1_MOVE_075F_1BC2_VERTICAL_DOWN;
            }
        } else {
            out->target_first_thing = -1;
            out->target_door_state = -1;
        }
        dm2_move_075f_1bc2_classify_target(out);
    }

    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->level);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->from_x);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->from_y);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->from_dir);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->move_dir);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->to_x);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->to_y);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->source_raw_valid);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->source_raw);
    hash = dm2_move_075f_1bc2_hash_step(
        hash, (uint32_t)out->source_square_type);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->target_raw_valid);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->target_raw);
    hash = dm2_move_075f_1bc2_hash_step(
        hash, (uint32_t)out->target_square_type);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->target_first_thing);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->target_is_outdoor);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->target_door_state);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->vertical_kind);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->block_reason);
    hash = dm2_move_075f_1bc2_hash_step(hash, (uint32_t)out->accepted);
    if (hash == 0u) return 0;
    out->movement_hash = hash;
    return 1;
}

const char *dm2_v1_DM2_move_075f_1bc2_source_evidence(void)
{
    return "skproject SKULLWIN/c_move.cpp:2861 DM2_move_075f_1bc2: "
           "bounded move target receipt over loaded DUNGEON.DAT square facts, "
           "normalizing N/E/S/W movement and recording raw tile, square type, "
           "door state, first thing link, passability, and vertical stair "
           "direction without fabricating chained move side effects.";
}
