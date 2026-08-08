
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
    cfg->wall_color = 0xFF808080U;
    cfg->floor_color = 0xFF404040U;
    cfg->party_color = 0xFF00FF00U;
    cfg->bg_color = 0xFF000000U;
}

int nexus_v1_automap_render(const Nexus_Automap *map,
                            const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                            int party_x, int party_y,
                            const Nexus_AutomapRenderConfig *cfg,
                            uint32_t *pixels, int width, int height) {
    int x, y, px, py, cs, drawn = 0;
    Nexus_AutomapRenderConfig default_cfg;

    if (!map || !squares || !pixels || width <= 0 || height <= 0) return 0;
    if (!cfg) {
        nexus_v1_automap_default_config(&default_cfg);
        cfg = &default_cfg;
    }

    cs = cfg->cell_size;
    if (cs <= 0) cs = 4;

    memset(pixels, 0, (size_t)(width * height) * sizeof(uint32_t));

    for (y = 0; y < NEXUS_MAX_MAP_SIZE; y++) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; x++) {
            if (!map->explored[map->current_level][y][x]) continue;
            uint32_t color = (squares[y][x] == 0) ? cfg->wall_color : cfg->floor_color;
            for (py = y * cs; py < (y + 1) * cs && py < height; py++) {
                for (px = x * cs; px < (x + 1) * cs && px < width; px++) {
                    pixels[py * width + px] = color;
                    drawn++;
                }
            }
        }
    }

    if (party_x >= 0 && party_x < NEXUS_MAX_MAP_SIZE &&
        party_y >= 0 && party_y < NEXUS_MAX_MAP_SIZE) {
        int cx = party_x * cs + cs / 2;
        int cy = party_y * cs + cs / 2;
        for (py = cy - 1; py <= cy + 1 && py < height; py++) {
            if (py < 0) continue;
            for (px = cx - 1; px <= cx + 1 && px < width; px++) {
                if (px < 0) continue;
                pixels[py * width + px] = cfg->party_color;
            }
        }
    }

    return drawn;
}
