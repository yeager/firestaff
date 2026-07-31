/* Legacy in-memory tile grid used only by historical data-free tests.
 *
 * It is deliberately not part of firestaff_m10: live CSB resolves squares
 * and Things from the loaded DUNGEON.DAT byte map. */
#include "csb_v1_dungeon_world_pc34_compat.h"

#include <string.h>

void csb_world_init(CSB_DungeonWorld* w) { if (w) memset(w, 0, sizeof(*w)); }

int csb_world_add_level(CSB_DungeonWorld* w, int width, int height) {
    CSB_Level* level;
    if (!w || w->levelCount >= CSB_MAX_LEVELS) return -1;
    level = &w->levels[w->levelCount];
    memset(level, 0, sizeof(*level));
    level->width = width;
    level->height = height;
    level->levelIndex = w->levelCount;
    w->levelCount++;
    return w->levelCount - 1;
}

CSB_Tile* csb_world_get_tile(CSB_DungeonWorld* w, int level, int x, int y) {
    CSB_Level* row;
    if (!w || level < 0 || level >= w->levelCount) return NULL;
    row = &w->levels[level];
    return x >= 0 && x < row->width && y >= 0 && y < row->height
        ? &row->tiles[y][x] : NULL;
}

const CSB_Tile* csb_world_get_tile_const(const CSB_DungeonWorld* w,
                                          int level, int x, int y) {
    return csb_world_get_tile((CSB_DungeonWorld*)w, level, x, y);
}

int csb_world_is_walkable(const CSB_DungeonWorld* w, int level, int x, int y) {
    const CSB_Tile* tile = csb_world_get_tile_const(w, level, x, y);
    if (!tile) return 0;
    switch (tile->type) {
    case CSB_TILE_FLOOR: case CSB_TILE_PIT: case CSB_TILE_STAIRS_UP:
    case CSB_TILE_STAIRS_DOWN: case CSB_TILE_DOOR: case CSB_TILE_TELEPORTER:
        return 1;
    default: return 0;
    }
}

int csb_world_is_wall(const CSB_DungeonWorld* w, int level, int x, int y) {
    const CSB_Tile* tile = csb_world_get_tile_const(w, level, x, y);
    return tile && tile->type == CSB_TILE_WALL;
}

void csb_world_set_tile_type(CSB_DungeonWorld* w, int level, int x, int y,
                             uint8_t type) {
    CSB_Tile* tile = csb_world_get_tile(w, level, x, y); if (tile) tile->type = type;
}

void csb_world_set_wall(CSB_DungeonWorld* w, int level, int x, int y,
                        int dir, uint8_t type) {
    CSB_Tile* tile = csb_world_get_tile(w, level, x, y); if (!tile) return;
    if (dir == 0) tile->wallN = type; else if (dir == 1) tile->wallE = type;
    else if (dir == 2) tile->wallS = type; else if (dir == 3) tile->wallW = type;
}

void csb_world_set_ornament(CSB_DungeonWorld* w, int level, int x, int y,
                            int dir, uint8_t ornament) {
    CSB_Tile* tile = csb_world_get_tile(w, level, x, y); if (!tile) return;
    if (dir == 0) tile->ornamentN = ornament; else if (dir == 1) tile->ornamentE = ornament;
    else if (dir == 2) tile->ornamentS = ornament; else if (dir == 3) tile->ornamentW = ornament;
}

int csb_world_get_level_count(const CSB_DungeonWorld* w) { return w ? w->levelCount : 0; }
void csb_world_set_current_level(CSB_DungeonWorld* w, int level) {
    if (w && level >= 0 && level < w->levelCount) w->currentLevel = level;
}
