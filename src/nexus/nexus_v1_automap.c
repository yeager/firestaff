
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
    cfg->cell_size = 4;
    cfg->wall_color = 0xFF808080u;
    cfg->floor_color = 0xFF303030u;
    cfg->party_color = 0xFF00FF00u;
    cfg->bg_color = 0xFF000000u;
}

int nexus_v1_automap_render(const Nexus_Automap *map,
                            const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                            int party_x, int party_y,
                            const Nexus_AutomapRenderConfig *cfg,
                            uint32_t *pixels, int width, int height) {
    int x, y, px, py, cs, drawn = 0;
    int level;
    if (!map || !pixels || !cfg || width <= 0 || height <= 0)
        return 0;

    cs = cfg->cell_size;
    if (cs < 1) cs = 1;
    level = map->current_level;

    for (py = 0; py < height; py++)
        for (px = 0; px < width; px++)
            pixels[py * width + px] = cfg->bg_color;

    for (y = 0; y < NEXUS_MAX_MAP_SIZE; y++) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++) {
            uint32_t color;
            int cx, cy;
            if (!map->explored[level][y][x])
                continue;
            if (squares && squares[y][x] == 0)
                color = cfg->wall_color;
            else
                color = cfg->floor_color;

            for (cy = 0; cy < cs; cy++) {
                for (cx = 0; cx < cs; cx++) {
                    int sx = x * cs + cx;
                    int sy = y * cs + cy;
                    if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                        pixels[sy * width + sx] = color;
                        drawn++;
                    }
                }
            }
        }
    }

    if (party_x >= 0 && party_x < NEXUS_MAX_MAP_SIZE &&
        party_y >= 0 && party_y < NEXUS_MAX_MAP_SIZE) {
        int cx, cy;
        for (cy = 0; cy < cs; cy++) {
            for (cx = 0; cx < cs; cx++) {
                int sx = party_x * cs + cx;
                int sy = party_y * cs + cy;
                if (sx >= 0 && sx < width && sy >= 0 && sy < height)
                    pixels[sy * width + sx] = cfg->party_color;
            }
        }
    }

    return drawn;
}
