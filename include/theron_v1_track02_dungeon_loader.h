#ifndef THERON_V1_TRACK02_DUNGEON_LOADER_H
#define THERON_V1_TRACK02_DUNGEON_LOADER_H

#include <stddef.h>
#include <stdint.h>

typedef struct Theron_V1_World Theron_V1_World;

typedef struct {
    int levels_loaded;
    int ground_refs_linked;
    int actuator_value_fixes;
    int total_things_placed;
    int doors_placed;
    int teleporters_placed;
    int actuators_placed;
    int creatures_placed;
    int champions_placed;
    int items_placed;
} Theron_DungeonLoadResult;

int theron_v1_track02_load_full_dungeon(
    Theron_V1_World *world,
    int dungeon_id,
    const uint8_t *ud_data,
    size_t ud_size,
    Theron_DungeonLoadResult *result);

#endif
