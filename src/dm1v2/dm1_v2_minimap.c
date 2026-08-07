#include "dm1_v2_minimap.h"

/* Legacy V2 minimap entry points deliberately do not construct a map or an
 * RGBA overlay.  The real dungeon surface stays on the PC34 draw route. */

void dm1_v2_minimap_init(DM1_V2_Minimap *minimap, int map_width,
                         int map_height) {
    (void)minimap;
    (void)map_width;
    (void)map_height;
}

void dm1_v2_minimap_update(DM1_V2_Minimap *minimap, int party_x,
                           int party_y, int party_direction) {
    (void)minimap;
    (void)party_x;
    (void)party_y;
    (void)party_direction;
}

void dm1_v2_minimap_reveal(DM1_V2_Minimap *minimap, int x, int y) {
    (void)minimap;
    (void)x;
    (void)y;
}

void dm1_v2_minimap_render(const DM1_V2_Minimap *minimap, uint32_t *rgba,
                           int screen_width, int screen_height) {
    (void)minimap;
    (void)rgba;
    (void)screen_width;
    (void)screen_height;
}
