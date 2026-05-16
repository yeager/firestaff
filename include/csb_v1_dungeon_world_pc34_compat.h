#ifndef FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_MAX_LEVELS 16
#define CSB_MAX_WIDTH 64
#define CSB_MAX_HEIGHT 64

enum {
    CSB_TILE_EMPTY = 0,
    CSB_TILE_WALL,
    CSB_TILE_FLOOR,
    CSB_TILE_PIT,
    CSB_TILE_STAIRS_UP,
    CSB_TILE_STAIRS_DOWN,
    CSB_TILE_DOOR,
    CSB_TILE_TELEPORTER,
    CSB_TILE_FAKE_WALL,
    CSB_TILE_COUNT
};

typedef struct {
    uint8_t type;
    uint8_t flags;
    uint8_t wallN;
    uint8_t wallE;
    uint8_t wallS;
    uint8_t wallW;
    uint8_t ornamentN;
    uint8_t ornamentE;
    uint8_t ornamentS;
    uint8_t ornamentW;
    int16_t thingList;
    int16_t creatureList;
} CSB_Tile;

typedef struct {
    CSB_Tile tiles[CSB_MAX_HEIGHT][CSB_MAX_WIDTH];
    int width;
    int height;
    int levelIndex;
    int lightLevel;
} CSB_Level;

typedef struct {
    CSB_Level levels[CSB_MAX_LEVELS];
    int levelCount;
    int currentLevel;
} CSB_DungeonWorld;

void csb_world_init(CSB_DungeonWorld* w);
int csb_world_add_level(CSB_DungeonWorld* w, int width, int height);
CSB_Tile* csb_world_get_tile(CSB_DungeonWorld* w, int level, int x, int y);
const CSB_Tile* csb_world_get_tile_const(const CSB_DungeonWorld* w, int level, int x, int y);
int csb_world_is_walkable(const CSB_DungeonWorld* w, int level, int x, int y);
int csb_world_is_wall(const CSB_DungeonWorld* w, int level, int x, int y);
void csb_world_set_tile_type(CSB_DungeonWorld* w, int level, int x, int y, uint8_t type);
void csb_world_set_wall(CSB_DungeonWorld* w, int level, int x, int y, int dir, uint8_t wallType);
void csb_world_set_ornament(CSB_DungeonWorld* w, int level, int x, int y, int dir, uint8_t ornament);
int csb_world_get_level_count(const CSB_DungeonWorld* w);
void csb_world_set_current_level(CSB_DungeonWorld* w, int level);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_DUNGEON_WORLD_PC34_COMPAT_H */