#include "dm2_v1_find_ladder_around.h"

#include "dm2_v1_world_model.h"

#include <string.h>

static uint32_t dm2_find_ladder_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static int dm2_find_ladder_abs(int value)
{
    return value < 0 ? -value : value;
}

int dm2_v1_FIND_LADDER_AROUND(
    const DM2_V1_DungeonData *dungeon,
    int level,
    int x,
    int y,
    DM2_V1_FindLadderAroundReceipt *out)
{
    static const int offsets[9][2] = {
        { 0,  0},
        { 0, -1},
        { 1,  0},
        { 0,  1},
        {-1,  0},
        { 1, -1},
        { 1,  1},
        {-1,  1},
        {-1, -1}
    };
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->level = level;
    out->origin_x = x;
    out->origin_y = y;
    out->ladder_x = -1;
    out->ladder_y = -1;
    out->search_slot = -1;
    out->manhattan_distance = -1;
    out->raw_tile = -1;
    out->square_type = -1;

    if (!dungeon || !dungeon->raw_data || level < 0 ||
        level >= dungeon->level_count || x < 0 || y < 0 ||
        x >= dungeon->level_widths[level] ||
        y >= dungeon->level_heights[level]) {
        return 0;
    }

    for (int slot = 0; slot < 9; ++slot) {
        int sx = x + offsets[slot][0];
        int sy = y + offsets[slot][1];
        int raw;
        int type;

        hash = dm2_find_ladder_hash_step(hash, (uint32_t)slot);
        if (sx < 0 || sy < 0 ||
            sx >= dungeon->level_widths[level] ||
            sy >= dungeon->level_heights[level]) {
            hash = dm2_find_ladder_hash_step(hash, 0xffffu);
            continue;
        }

        raw = dm2_v1_dungeon_get_tile_raw(dungeon, level, sx, sy);
        type = dm2_v1_dungeon_get_square_type(dungeon, level, sx, sy);
        if (raw < 0 || type < 0) {
            hash = dm2_find_ladder_hash_step(hash, 0xfffeu);
            continue;
        }
        hash = dm2_find_ladder_hash_step(hash, (uint32_t)(raw & 0xffff));
        hash = dm2_find_ladder_hash_step(hash, (uint32_t)type);

        if (type == DM2_SQUARE_STAIRS_UP ||
            type == DM2_SQUARE_STAIRS_DOWN) {
            out->found = 1;
            out->ladder_x = sx;
            out->ladder_y = sy;
            out->search_slot = slot;
            out->manhattan_distance =
                dm2_find_ladder_abs(offsets[slot][0]) +
                dm2_find_ladder_abs(offsets[slot][1]);
            out->raw_tile = raw;
            out->square_type = type;
            out->kind = type == DM2_SQUARE_STAIRS_UP
                ? DM2_V1_LADDER_AROUND_UP
                : DM2_V1_LADDER_AROUND_DOWN;
            out->vertical_delta = type == DM2_SQUARE_STAIRS_UP ? -1 : 1;
            break;
        }
    }

    hash = dm2_find_ladder_hash_step(hash, (uint32_t)level);
    hash = dm2_find_ladder_hash_step(hash, (uint32_t)x);
    hash = dm2_find_ladder_hash_step(hash, (uint32_t)y);
    hash = dm2_find_ladder_hash_step(hash, (uint32_t)out->found);
    hash = dm2_find_ladder_hash_step(hash, (uint32_t)out->search_slot);
    if (hash == 0u) return 0;
    out->valid = 1;
    out->search_hash = hash;
    return 1;
}

const char *dm2_v1_FIND_LADDER_AROUND_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp FIND_LADDER_AROUND: bounded "
           "dungeon-neighbour query around the party square. Firestaff "
           "admits only already loaded DM2 dungeon square facts and returns "
           "an explicit not-found receipt instead of fabricating ladder or "
           "hole semantics from missing map data.";
}
