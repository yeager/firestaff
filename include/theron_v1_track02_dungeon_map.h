#ifndef THERON_V1_TRACK02_DUNGEON_MAP_H
#define THERON_V1_TRACK02_DUNGEON_MAP_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_track02.h"

#ifndef THERON_TRACK02_DUNGEON_COUNT
#define THERON_TRACK02_DUNGEON_COUNT  7u
#endif
#define THERON_TRACK02_MAX_MAPS       8
#define THERON_TRACK02_MAX_MAP_DIM    32
#define THERON_TRACK02_MAX_COLUMNS    256
#define THERON_TRACK02_THING_TYPE_COUNT 12

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

typedef struct Theron_DungeonData {
    uint8_t      dungeon_index;
    uint8_t      map_count;
    uint16_t     object_counts[16];
    uint16_t     creature_gfx_bank[THERON_TRACK02_MAX_MAPS];
    uint16_t     cumulative_column_items[THERON_TRACK02_MAX_MAPS];
    uint16_t     column_thing_counts[THERON_TRACK02_MAX_COLUMNS];
    uint16_t     column_thing_count_total;
    uint8_t      thing_descriptor_sizes[THERON_TRACK02_THING_TYPE_COUNT];
    size_t       thing_list_offset;
    size_t       thing_list_size;
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

int theron_v1_track02_dungeon_map_quest_block_offsets_for_variant(
    Theron_Track02Variant variant,
    unsigned int dungeon_index,
    Theron_QuestBlockOffsets *out);

int theron_v1_track02_dungeon_map_count(unsigned int dungeon_index);

int theron_v1_track02_dungeon_map_load(
    const uint8_t *ud_data,
    size_t ud_size,
    unsigned int dungeon_index,
    Theron_DungeonData *out);

int theron_v1_track02_dungeon_map_load_for_variant(
    const uint8_t *ud_data,
    size_t ud_size,
    Theron_Track02Variant variant,
    unsigned int dungeon_index,
    Theron_DungeonData *out);

uint16_t theron_v1_track02_dungeon_text_data_size(unsigned int dungeon_index);

static inline Theron_TileType theron_tile_type(uint8_t tile_byte) {
    return (Theron_TileType)(tile_byte >> 5);
}
static inline int theron_tile_has_things(uint8_t tile_byte) {
    return (tile_byte >> 4) & 1;
}
static inline uint8_t theron_tile_attributes(uint8_t tile_byte) {
    return tile_byte & 0x0F;
}

#endif
