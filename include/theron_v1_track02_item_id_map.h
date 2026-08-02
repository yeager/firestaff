#ifndef THERON_V1_TRACK02_ITEM_ID_MAP_H
#define THERON_V1_TRACK02_ITEM_ID_MAP_H

#include <stdint.h>

typedef struct {
    uint8_t tq_id;
    uint8_t dm1_id;
    const char *name;
} Theron_ItemIdMapping;

unsigned int theron_v1_track02_item_id_map_count(void);
const Theron_ItemIdMapping *theron_v1_track02_item_id_map(unsigned int index);
uint8_t theron_v1_track02_translate_item_id(uint8_t tq_id);

typedef struct {
    uint8_t wall_ornament_ordinal;
    uint8_t dm1_ornament_id;
    const char *name;
} Theron_WallOrnament;

typedef struct {
    uint8_t floor_ornament_ordinal;
    uint8_t dm1_ornament_id;
    const char *name;
} Theron_FloorOrnament;

unsigned int theron_v1_track02_akutuba_wall_ornament_count(unsigned int level);
const Theron_WallOrnament *theron_v1_track02_akutuba_wall_ornaments(
    unsigned int level, unsigned int *count);

#endif
