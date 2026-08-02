#ifndef THERON_V1_TRACK02_DUNGEON_MAP_H
#define THERON_V1_TRACK02_DUNGEON_MAP_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_DUNGEON_COUNT  7
#define THERON_TRACK02_MAX_MAPS       8
#define THERON_TRACK02_MAX_MAP_DIM    32

typedef enum {
    THERON_TILE_WALL       = 0,
    THERON_TILE_OPEN       = 1,
    THERON_TILE_PIT        = 2,
    THERON_TILE_STAIRS     = 3,
    THERON_TILE_DOOR       = 4,
    THERON_TILE_TELEPORTER = 5,
    THERON_TILE_FAKEWALL   = 6,
    THERON_TILE_TYPE7      = 7,
} Theron_TileType;

typedef struct {
    uint8_t  x_dim;
    uint8_t  y_dim;
    uint8_t  x_offset;
    uint8_t  y_offset;
    uint8_t  map_id;
    uint8_t  unk1;
    uint8_t  unk2;
    uint8_t  creature_count;
    uint8_t  xp_modifier;
    uint8_t  door_type1;
    uint8_t  door_type2;
} Theron_MapHeader;

typedef struct {
    Theron_MapHeader header;
    uint8_t tiles[THERON_TRACK02_MAX_MAP_DIM][THERON_TRACK02_MAX_MAP_DIM];
} Theron_Map;

typedef struct {
    uint8_t      dungeon_index;
    uint8_t      map_count;
    uint16_t     object_counts[16];
    Theron_Map   maps[THERON_TRACK02_MAX_MAPS];
} Theron_DungeonData;

typedef struct {
    uint32_t dims_offset;
    uint32_t map_data_offset;
    uint32_t ground_refs_offset;
    uint32_t items_part1_offset;
    uint32_t items_part2_offset;
    uint32_t text_data_offset;
} Theron_QuestBlockOffsets;

int theron_v1_track02_dungeon_map_quest_block_offsets(
    unsigned int dungeon_index,
    Theron_QuestBlockOffsets *out);

int theron_v1_track02_dungeon_map_count(unsigned int dungeon_index);

int theron_v1_track02_dungeon_map_load(
    const uint8_t *ud_data,
    size_t ud_size,
    unsigned int dungeon_index,
    Theron_DungeonData *out);

uint16_t theron_v1_track02_dungeon_text_data_size(unsigned int dungeon_index);

Theron_TileType theron_tile_type(uint8_t tile_byte);
int theron_tile_has_things(uint8_t tile_byte);
uint8_t theron_tile_attributes(uint8_t tile_byte);

#endif
