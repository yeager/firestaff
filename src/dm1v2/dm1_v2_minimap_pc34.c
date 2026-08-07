#include "dm1_v2_minimap_pc34.h"

/* PC34 does not own a transparent explored-tile overlay.  Dungeon drawing is
 * source-owned by F0128/F0097, so this V2 compatibility API has no map cache,
 * party marker, zoom state or framebuffer writes. */

void v2_minimap_init(M11_V2_Minimap* map) {
    (void)map;
}

void v2_minimap_reveal_tile(M11_V2_Minimap* map, int16_t x, int16_t y,
                            M11_V2_MinimapTileType type) {
    (void)map;
    (void)x;
    (void)y;
    (void)type;
}

void v2_minimap_set_party(M11_V2_Minimap* map, int16_t x, int16_t y,
                          int8_t facing) {
    (void)map;
    (void)x;
    (void)y;
    (void)facing;
}

void v2_minimap_render(const M11_V2_Minimap* map, uint8_t* framebuffer,
                       int framebuffer_width, int framebuffer_height) {
    (void)map;
    (void)framebuffer;
    (void)framebuffer_width;
    (void)framebuffer_height;
}

void v2_minimap_toggle_visible(M11_V2_Minimap* map) {
    (void)map;
}

void v2_minimap_zoom(M11_V2_Minimap* map, bool zoom_in) {
    (void)map;
    (void)zoom_in;
}

bool v2_minimap_is_explored(const M11_V2_Minimap* map, int16_t x, int16_t y) {
    (void)map;
    (void)x;
    (void)y;
    return false;
}
