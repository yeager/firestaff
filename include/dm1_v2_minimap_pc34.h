#ifndef FIRESTAFF_DM1_V2_MINIMAP_PC34_H
#define FIRESTAFF_DM1_V2_MINIMAP_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_V2_TILE_WALL = 0,
    M11_V2_TILE_FLOOR,
    M11_V2_TILE_DOOR,
    M11_V2_TILE_STAIRS,
    M11_V2_TILE_PIT
} M11_V2_MinimapTileType;

typedef struct {
    M11_V2_MinimapTileType type;
    bool explored;
    bool visible;
} M11_V2_MinimapTile;

typedef struct {
    M11_V2_MinimapTile grid[32][32];
    int16_t party_x;
    int16_t party_y;
    int8_t party_facing;
    float zoom;
    float opacity;
    bool visible;
} M11_V2_Minimap;

void v2_minimap_init(M11_V2_Minimap* map);
void v2_minimap_reveal_tile(M11_V2_Minimap* map, int16_t x, int16_t y, M11_V2_MinimapTileType type);
void v2_minimap_set_party(M11_V2_Minimap* map, int16_t x, int16_t y, int8_t facing);
void v2_minimap_render(const M11_V2_Minimap* map, uint8_t* framebuffer, int fb_w, int fb_h);
void v2_minimap_toggle_visible(M11_V2_Minimap* map);
void v2_minimap_zoom(M11_V2_Minimap* map, bool in_out);
bool v2_minimap_is_explored(const M11_V2_Minimap* map, int16_t x, int16_t y);

#ifdef __cplusplus
}
#endif

#endif
