
#ifndef NEXUS_V1_AUTOMAP_H
#define NEXUS_V1_AUTOMAP_H

#include <stdint.h>
#include "nexus_v1_dungeon.h"

#define NEXUS_AUTOMAP_MAX_LEVELS 16

typedef struct {
    uint8_t explored[NEXUS_AUTOMAP_MAX_LEVELS][NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    int current_level;
    int map_open;
} Nexus_Automap;

void nexus_v1_automap_init(Nexus_Automap *map);

void nexus_v1_automap_reveal(Nexus_Automap *map, int level, int x, int y);

void nexus_v1_automap_reveal_radius(Nexus_Automap *map, int level,
                                     int x, int y, int radius);

int nexus_v1_automap_is_explored(const Nexus_Automap *map, int level, int x, int y);

void nexus_v1_automap_toggle(Nexus_Automap *map);

void nexus_v1_automap_set_level(Nexus_Automap *map, int level);

int nexus_v1_automap_explored_count(const Nexus_Automap *map, int level);

#endif
