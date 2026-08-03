
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

typedef struct {
    int cell_size;
    uint32_t wall_color;
    uint32_t floor_color;
    uint32_t party_color;
    uint32_t bg_color;
} Nexus_AutomapRenderConfig;

void nexus_v1_automap_default_config(Nexus_AutomapRenderConfig *cfg);

int nexus_v1_automap_render(const Nexus_Automap *map,
                            const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                            int party_x, int party_y,
                            const Nexus_AutomapRenderConfig *cfg,
                            uint32_t *pixels, int width, int height);

#endif
