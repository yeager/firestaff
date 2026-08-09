#ifndef THERON_V1_TRACK02_SPAWN_BINDING_H
#define THERON_V1_TRACK02_SPAWN_BINDING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* These are the source-record shapes at UD $274000 in the authenticated US
 * Track 02 BIN.  Keeping them in a dependency-light header lets the live
 * world retain the verified records without making world.h depend on the
 * complete Track 02 loader (which itself includes world.h). */
#define THERON_TRACK02_SPAWN_POINTER_COUNT 8u
#define THERON_TRACK02_SPAWN_ZONE_COUNT 5u

typedef struct {
    uint16_t sprite_desc_offset;
    uint16_t constant_278a;
    uint16_t spawn_data_offset;
    uint16_t constant_016b;
} Theron_CreaturePointerEntry;

typedef struct {
    uint16_t map_width;
    uint16_t map_height;
    uint8_t category;
    uint8_t count;
    uint8_t param1;
    uint8_t param2;
} Theron_SpawnZoneDesc;

typedef struct {
    int authenticated;
    /* 1 = JP BIN, 2 = US BIN.  Keep this boundary primitive so the record
     * header cannot pull the large startup/runtime include graph into
     * world.h. */
    int variant;
    Theron_CreaturePointerEntry pointers[THERON_TRACK02_SPAWN_POINTER_COUNT];
    Theron_SpawnZoneDesc zones[THERON_TRACK02_SPAWN_ZONE_COUNT];
} Theron_Track02SpawnSource;

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_SPAWN_BINDING_H */
