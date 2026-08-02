
#include "nexus_v1_automap.h"
#include <string.h>

void nexus_v1_automap_init(Nexus_Automap *map) {
    if (!map) return;
    memset(map, 0, sizeof(*map));
}

void nexus_v1_automap_reveal(Nexus_Automap *map, int level, int x, int y) {
    if (!map || level < 0 || level >= NEXUS_AUTOMAP_MAX_LEVELS) return;
    if (x < 0 || x >= NEXUS_MAX_MAP_SIZE || y < 0 || y >= NEXUS_MAX_MAP_SIZE) return;
    map->explored[level][y][x] = 1;
}

void nexus_v1_automap_reveal_radius(Nexus_Automap *map, int level,
                                     int x, int y, int radius) {
    int dx, dy;
    if (!map) return;
    for (dy = -radius; dy <= radius; dy++)
        for (dx = -radius; dx <= radius; dx++)
            nexus_v1_automap_reveal(map, level, x + dx, y + dy);
}

int nexus_v1_automap_is_explored(const Nexus_Automap *map, int level, int x, int y) {
    if (!map || level < 0 || level >= NEXUS_AUTOMAP_MAX_LEVELS) return 0;
    if (x < 0 || x >= NEXUS_MAX_MAP_SIZE || y < 0 || y >= NEXUS_MAX_MAP_SIZE) return 0;
    return map->explored[level][y][x] ? 1 : 0;
}

void nexus_v1_automap_toggle(Nexus_Automap *map) {
    if (!map) return;
    map->map_open = !map->map_open;
}

void nexus_v1_automap_set_level(Nexus_Automap *map, int level) {
    if (!map || level < 0 || level >= NEXUS_AUTOMAP_MAX_LEVELS) return;
    map->current_level = level;
}

int nexus_v1_automap_explored_count(const Nexus_Automap *map, int level) {
    int x, y, count = 0;
    if (!map || level < 0 || level >= NEXUS_AUTOMAP_MAX_LEVELS) return 0;
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; y++)
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++)
            if (map->explored[level][y][x]) count++;
    return count;
}
