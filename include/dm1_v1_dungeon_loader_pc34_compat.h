#ifndef FIRESTAFF_DM1_V1_DUNGEON_LOADER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_LOADER_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_MAX_LEVELS 16
#define DM1_MAX_MAP_W 32
#define DM1_MAX_MAP_H 32
#define DM1_THING_TYPE_COUNT 16
#define DM1_OBJECT_NAME_COUNT 199
#define DM1_CREATURE_TYPE_COUNT 27
#define DM1_WEAPON_COUNT 46
#define DM1_ARMOUR_COUNT 58
#define DM1_DOOR_TYPE_COUNT 4

typedef enum {
    DM1_V1_DUNGEON_THING_DOOR_PC34 = 0,
    DM1_V1_DUNGEON_THING_TELEPORTER_PC34 = 1,
    DM1_V1_DUNGEON_THING_TEXT_PC34 = 2,
    DM1_V1_DUNGEON_THING_SENSOR_PC34 = 3,
    DM1_V1_DUNGEON_THING_GROUP_PC34 = 4,
    DM1_V1_DUNGEON_THING_WEAPON_PC34 = 5,
    DM1_V1_DUNGEON_THING_ARMOUR_PC34 = 6,
    DM1_V1_DUNGEON_THING_SCROLL_PC34 = 7,
    DM1_V1_DUNGEON_THING_POTION_PC34 = 8,
    DM1_V1_DUNGEON_THING_CONTAINER_PC34 = 9,
    DM1_V1_DUNGEON_THING_JUNK_PC34 = 10,
    DM1_V1_DUNGEON_THING_PROJECTILE_PC34 = 14
} DM1_V1_DungeonThingTypePc34;

typedef struct {
    uint16_t offset;
    uint8_t width;
    uint8_t height;
    uint8_t wall_ornament_count;
    uint8_t floor_ornament_count;
    uint8_t door_ornament_count;
} DM1_V1_DungeonLevelDescriptorPc34;

typedef struct {
    uint16_t level_count;
    uint16_t map_data_offset;
    uint16_t thing_data_offset;
    DM1_V1_DungeonLevelDescriptorPc34 levels[DM1_MAX_LEVELS];
} DM1_V1_DungeonHeaderPc34;

typedef struct {
    uint8_t type;
    uint8_t attributes;
    uint16_t first_thing;
} DM1_V1_DungeonTilePc34;

typedef struct {
    DM1_V1_DungeonHeaderPc34 header;
    DM1_V1_DungeonTilePc34 tiles[DM1_MAX_LEVELS][DM1_MAX_MAP_W][DM1_MAX_MAP_H];
    int8_t step_east[4];
    int8_t step_north[4];
    uint8_t thing_byte_count[DM1_THING_TYPE_COUNT];
    bool loaded;
} DM1_V1_DungeonStatePc34;

void DM1_V1_DungeonLoader_InitPc34Compat(DM1_V1_DungeonStatePc34 *state);
bool DM1_V1_DungeonLoader_LoadFromFilePc34Compat(DM1_V1_DungeonStatePc34 *state, const char *path);
const DM1_V1_DungeonTilePc34 *DM1_V1_DungeonLoader_GetTilePc34Compat(const DM1_V1_DungeonStatePc34 *state, uint8_t level, uint8_t x, uint8_t y);
void DM1_V1_DungeonLoader_StepForwardPc34Compat(int *x, int *y, uint8_t dir);
void DM1_V1_DungeonLoader_CleanupPc34Compat(DM1_V1_DungeonStatePc34 *state);

typedef DM1_V1_DungeonThingTypePc34 M11_DL_ThingType;
typedef DM1_V1_DungeonLevelDescriptorPc34 M11_DL_LevelDescriptor;
typedef DM1_V1_DungeonHeaderPc34 M11_DL_DungeonHeader;
typedef DM1_V1_DungeonTilePc34 M11_DL_Tile;
typedef DM1_V1_DungeonStatePc34 M11_DL_DungeonState;

#define M11_DL_DOOR DM1_V1_DUNGEON_THING_DOOR_PC34
#define M11_DL_TELEPORTER DM1_V1_DUNGEON_THING_TELEPORTER_PC34
#define M11_DL_TEXT DM1_V1_DUNGEON_THING_TEXT_PC34
#define M11_DL_SENSOR DM1_V1_DUNGEON_THING_SENSOR_PC34
#define M11_DL_GROUP DM1_V1_DUNGEON_THING_GROUP_PC34
#define M11_DL_WEAPON DM1_V1_DUNGEON_THING_WEAPON_PC34
#define M11_DL_ARMOUR DM1_V1_DUNGEON_THING_ARMOUR_PC34
#define M11_DL_SCROLL DM1_V1_DUNGEON_THING_SCROLL_PC34
#define M11_DL_POTION DM1_V1_DUNGEON_THING_POTION_PC34
#define M11_DL_CONTAINER DM1_V1_DUNGEON_THING_CONTAINER_PC34
#define M11_DL_JUNK DM1_V1_DUNGEON_THING_JUNK_PC34
#define M11_DL_PROJECTILE DM1_V1_DUNGEON_THING_PROJECTILE_PC34

#define m11_dl_init DM1_V1_DungeonLoader_InitPc34Compat
#define m11_dl_load_from_file DM1_V1_DungeonLoader_LoadFromFilePc34Compat
#define m11_dl_get_tile DM1_V1_DungeonLoader_GetTilePc34Compat
#define m11_dl_step_forward DM1_V1_DungeonLoader_StepForwardPc34Compat
#define m11_dl_cleanup DM1_V1_DungeonLoader_CleanupPc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DUNGEON_LOADER_PC34_COMPAT_H */
