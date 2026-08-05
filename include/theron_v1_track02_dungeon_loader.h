#ifndef THERON_V1_TRACK02_DUNGEON_LOADER_H
#define THERON_V1_TRACK02_DUNGEON_LOADER_H

#include <stddef.h>
#include <stdint.h>

typedef struct Theron_V1_World Theron_V1_World;

#define THERON_TRACK02_SOURCE_OBJECT_MAX 2048u

/* Source-faithful object occurrence retained for the next consumer pass.
 * DMBUILDER6/src/dms.h documents the first two bytes as the linked-list
 * next-reference for every placable object.  The remaining bytes stay raw
 * here: this handoff does not invent a host inventory/object type. */
typedef struct {
    uint16_t source_ref;
    uint16_t next_ref;
    uint8_t category;
    uint8_t source_index;
    uint8_t position;
    uint8_t raw_size;
    uint8_t raw[16];
    uint16_t map;
    uint16_t x;
    uint16_t y;
} Theron_Track02SourceObjectOccurrence;

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
    int source_records_decoded;
    int unbound_item_refs;
    int raw_only_item_refs;
    unsigned int source_object_count;
    Theron_Track02SourceObjectOccurrence source_objects[
        THERON_TRACK02_SOURCE_OBJECT_MAX];
} Theron_DungeonLoadResult;

int theron_v1_track02_load_full_dungeon(
    Theron_V1_World *world,
    int dungeon_id,
    const uint8_t *ud_data,
    size_t ud_size,
    Theron_DungeonLoadResult *result);

#endif
