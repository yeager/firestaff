#include "dm1_v2_minimap_pc34.h"

static const uint8_t M11_V2_MINIMAP_COLORS[] = {0x00, 0x55, 0xAA, 0xFF, 0x33, 0x77};

void v2_minimap_init(M11_V2_Minimap* map) {
    memset(map, 0, sizeof(M11_V2_Minimap));
    map->zoom = 1.0f;
    map->opacity = 0.8f;
    map->visible = true;
}

void v2_minimap_reveal_tile(M11_V2_Minimap* map, int16_t x, int16_t y, M11_V2_MinimapTileType type) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) return;
    map->grid[x][y].type = type;
    map->grid[x][y].explored = true;
    map->grid[x][y].visible = true;
}

void v2_minimap_set_party(M11_V2_Minimap* map, int16_t x, int16_t y, int8_t facing) {
    map->party_x = x;
    map->party_y = y;
    map->party_facing = facing;
}

static void v2_minimap_plot_pixel(uint8_t* fb, int w, int x, int y, uint8_t color) {
    if (x < 0 || x >= w || y < 0 || y >= w) return;
    fb[y * w + x] = color;
}

void v2_minimap_render(const M11_V2_Minimap* map, uint8_t* framebuffer, int fb_w, int fb_h) {
    if (!map->visible || !framebuffer) return;

    int center_x = fb_w / 2;
    int center_y = fb_h / 2;
    int tile_size = (int)(4.0f * map->zoom);
    if (tile_size < 1) tile_size = 1;

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (!map->grid[x][y].explored) continue;

            int sx = center_x + (int)((x - map->party_x) * tile_size);
            int sy = center_y + (int)((y - map->party_y) * tile_size);

            if (sx < 0 || sx >= fb_w || sy < 0 || sy >= fb_h) continue;

            uint8_t color = M11_V2_MINIMAP_COLORS[map->grid[x][y].type];
            if (map->opacity < 0.5f) color = (uint8_t)(color * map->opacity);

            for (int dy = 0; dy < tile_size; dy++) {
                for (int dx = 0; dx < tile_size; dx++) {
                    v2_minimap_plot_pixel(framebuffer, fb_w, sx + dx, sy + dy, color);
                }
            }
        }
    }

    int px = center_x;
    int py = center_y;
    uint8_t arrow_color = 0xFF;
    int ax = 0, ay = 0;
    switch (map->party_facing) {
        case 0: ax = 0; ay = -2; break;
        case 1: ax = 2; ay = 0; break;
        case 2: ax = 0; ay = 2; break;
        case 3: ax = -2; ay = 0; break;
        default: ax = 0; ay = -2; break;
    }
    v2_minimap_plot_pixel(framebuffer, fb_w, px, py, arrow_color);
    v2_minimap_plot_pixel(framebuffer, fb_w, px + ax, py + ay, arrow_color);
}

void v2_minimap_toggle_visible(M11_V2_Minimap* map) {
    map->visible = !map->visible;
}

void v2_minimap_zoom(M11_V2_Minimap* map, bool in_out) {
    if (in_out) {
        map->zoom += 0.25f;
        if (map->zoom > 2.0f) map->zoom = 2.0f;
    } else {
        map->zoom -= 0.25f;
        if (map->zoom < 0.5f) map->zoom = 0.5f;
    }
}

bool v2_minimap_is_explored(const M11_V2_Minimap* map, int16_t x, int16_t y) {
    if (x < 0 || x >= 32 || y < 0 || y >= 32) return false;
    return map->grid[x][y].explored;
}
