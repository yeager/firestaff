#include "csb_v1_dungeon_world_pc34_compat.h"
#include <string.h>

void csb_world_init(CSB_DungeonWorld* w) {
    if (!w) return;
    memset(w, 0, sizeof(CSB_DungeonWorld));
}

int csb_world_add_level(CSB_DungeonWorld* w, int width, int height) {
    if (!w) return -1;
    if (w->levelCount >= CSB_MAX_LEVELS) return -1;
    
    int idx = w->levelCount;
    CSB_Level* lvl = &w->levels[idx];
    
    memset(lvl, 0, sizeof(CSB_Level));
    lvl->width = width;
    lvl->height = height;
    lvl->levelIndex = idx;
    lvl->lightLevel = 0;
    
    w->levelCount++;
    return idx;
}

CSB_Tile* csb_world_get_tile(CSB_DungeonWorld* w, int level, int x, int y) {
    if (!w) return NULL;
    if (level < 0 || level >= w->levelCount) return NULL;
    
    CSB_Level* lvl = &w->levels[level];
    if (x < 0 || x >= lvl->width || y < 0 || y >= lvl->height) return NULL;
    
    return &lvl->tiles[y][x];
}

const CSB_Tile* csb_world_get_tile_const(const CSB_DungeonWorld* w, int level, int x, int y) {
    if (!w) return NULL;
    if (level < 0 || level >= w->levelCount) return NULL;
    
    const CSB_Level* lvl = &w->levels[level];
    if (x < 0 || x >= lvl->width || y < 0 || y >= lvl->height) return NULL;
    
    return &lvl->tiles[y][x];
}

int csb_world_is_walkable(const CSB_DungeonWorld* w, int level, int x, int y) {
    const CSB_Tile* tile = csb_world_get_tile_const(w, level, x, y);
    if (!tile) return 0;
    
    switch (tile->type) {
        case CSB_TILE_FLOOR:
        case CSB_TILE_PIT:
        case CSB_TILE_STAIRS_UP:
        case CSB_TILE_STAIRS_DOWN:
        case CSB_TILE_DOOR:
        case CSB_TILE_TELEPORTER:
            return 1;
        default:
            return 0;
    }
}

int csb_world_is_wall(const CSB_DungeonWorld* w, int level, int x, int y) {
    const CSB_Tile* tile = csb_world_get_tile_const(w, level, x, y);
    if (!tile) return 0;
    
    return (tile->type == CSB_TILE_WALL);
}

void csb_world_set_tile_type(CSB_DungeonWorld* w, int level, int x, int y, uint8_t type) {
    CSB_Tile* tile = csb_world_get_tile(w, level, x, y);
    if (tile) {
        tile->type = type;
    }
}

void csb_world_set_wall(CSB_DungeonWorld* w, int level, int x, int y, int dir, uint8_t wallType) {
    CSB_Tile* tile = csb_world_get_tile(w, level, x, y);
    if (!tile) return;
    
    switch (dir) {
        case 0: tile->wallN = wallType; break;
        case 1: tile->wallE = wallType; break;
        case 2: tile->wallS = wallType; break;
        case 3: tile->wallW = wallType; break;
        default: break;
    }
}

void csb_world_set_ornament(CSB_DungeonWorld* w, int level, int x, int y, int dir, uint8_t ornament) {
    CSB_Tile* tile = csb_world_get_tile(w, level, x, y);
    if (!tile) return;
    
    switch (dir) {
        case 0: tile->ornamentN = ornament; break;
        case 1: tile->ornamentE = ornament; break;
        case 2: tile->ornamentS = ornament; break;
        case 3: tile->ornamentW = ornament; break;
        default: break;
    }
}

int csb_world_get_level_count(const CSB_DungeonWorld* w) {
    if (!w) return 0;
    return w->levelCount;
}

void csb_world_set_current_level(CSB_DungeonWorld* w, int level) {
    if (!w) return;
    if (level < 0 || level >= w->levelCount) return;
    w->currentLevel = level;
}