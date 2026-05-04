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
    M11_DL_DOOR = 0,
    M11_DL_TELEPORTER = 1,
    M11_DL_TEXT = 2,
    M11_DL_SENSOR = 3,
    M11_DL_GROUP = 4,
    M11_DL_WEAPON = 5,
    M11_DL_ARMOUR = 6,
    M11_DL_SCROLL = 7,
    M11_DL_POTION = 8,
    M11_DL_CONTAINER = 9,
    M11_DL_JUNK = 10,
    M11_DL_PROJECTILE = 14
} M11_DL_ThingType;

typedef struct {
    uint16_t offset;
    uint8_t width;
    uint8_t height;
    uint8_t wall_ornament_count;
    uint8_t floor_ornament_count;
    uint8_t door_ornament_count;
} M11_DL_LevelDescriptor;

typedef struct {
    uint16_t level_count;
    uint16_t map_data_offset;
    uint16_t thing_data_offset;
    M11_DL_LevelDescriptor levels[DM1_MAX_LEVELS];
} M11_DL_DungeonHeader;

typedef struct {
    uint8_t type;
    uint8_t attributes;
    uint16_t first_thing;
} M11_DL_Tile;

typedef struct {
    M11_DL_DungeonHeader header;
    M11_DL_Tile tiles[DM1_MAX_LEVELS][DM1_MAX_MAP_W][DM1_MAX_MAP_H];
    int8_t step_east[4];
    int8_t step_north[4];
    uint8_t thing_byte_count[DM1_THING_TYPE_COUNT];
    bool loaded;
} M11_DL_DungeonState;

void m11_dl_init(M11_DL_DungeonState *state);
bool m11_dl_load_from_file(M11_DL_DungeonState *state, const char *path);
const M11_DL_Tile *m11_dl_get_tile(const M11_DL_DungeonState *state, uint8_t level, uint8_t x, uint8_t y);
void m11_dl_step_forward(int *x, int *y, uint8_t dir);
void m11_dl_cleanup(M11_DL_DungeonState *state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DUNGEON_LOADER_PC34_COMPAT_H */
