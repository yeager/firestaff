
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

void nexus_v1_automap_default_config(Nexus_AutomapRenderConfig *cfg) {
    if (!cfg) return;
    /* The retail SMAP*.BIN pixels and Saturn VDP2 placement are not the same
     * thing as a host-colored grid. Keep this compatibility config inert until
     * the original explored-state and VDP2 consumer are captured. */
    memset(cfg, 0, sizeof(*cfg));
}

int nexus_v1_automap_render(const Nexus_Automap *map,
                            const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                            int party_x, int party_y,
                            const Nexus_AutomapRenderConfig *cfg,
                            uint32_t *pixels, int width, int height) {
    (void)map;
    (void)squares;
    (void)party_x;
    (void)party_y;
    (void)cfg;
    (void)pixels;
    (void)width;
    (void)height;

    /* No host grid, guessed palette, party marker, or explored-radius pixels:
     * SMAP/VDP2 placement and the Saturn explored-state write path remain
     * unproven. */
    return 0;
}
