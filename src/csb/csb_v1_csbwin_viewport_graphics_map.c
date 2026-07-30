#include "csb_v1_csbwin_viewport_graphics_map.h"

int csb_v1_csbwin_floor_ceiling_graphic_index(uint16_t floor_set,
                                               int ceiling,
                                               uint16_t *out_graphic_index)
{
    if (!out_graphic_index || floor_set > 15u) return 0;
    *out_graphic_index = (uint16_t)(CSB_V1_CSBWIN_FLOORSET_FIRST_GRAPHIC +
        floor_set * CSB_V1_CSBWIN_FLOORSET_GRAPHIC_COUNT + (ceiling ? 1u : 0u));
    return 1;
}

int csb_v1_csbwin_packed_byte_width(uint16_t pixel_width,
                                    uint16_t *out_byte_width)
{
    if (!out_byte_width || pixel_width == 0u) return 0;
    *out_byte_width = (uint16_t)(((unsigned int)pixel_width + 1u) / 2u);
    return 1;
}

int csb_v1_csbwin_viewport_graphic_index(uint16_t wall_set,
                                          uint16_t slot,
                                          uint16_t *out_graphic_index)
{
    unsigned int index;

    if (!out_graphic_index || wall_set > 15u ||
        slot >= CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT) return 0;
    index = CSB_V1_CSBWIN_WALLSET_FIRST_GRAPHIC +
        wall_set * CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT + slot;
    if (index > UINT16_MAX) return 0;
    *out_graphic_index = (uint16_t)index;
    return 1;
}

int csb_v1_csbwin_viewport_wall_source(
    uint16_t wall_set, CSB_V1_CSBWinViewportWall wall,
    uint16_t *out_graphic_index, int *out_mirrored)
{
    static const uint8_t source_slot[CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT] = {
        5u, 4u, 4u, 4u, 5u, 3u, 3u, 3u, 2u, 2u, 2u, 1u, 0u
    };

    if (out_mirrored) *out_mirrored = 0;
    if (!out_graphic_index || !out_mirrored ||
        (unsigned int)wall >= CSB_V1_CSBWIN_VIEWPORT_WALL_COUNT) return 0;
    if (!csb_v1_csbwin_viewport_graphic_index(
            wall_set, (uint16_t)(CSB_V1_CSBWIN_DOOR_GRAPHIC_COUNT +
                                  source_slot[wall]), out_graphic_index)) {
        return 0;
    }
    *out_mirrored = wall == CSB_V1_CSBWIN_VIEWPORT_WALL_F3R2;
    return 1;
}
