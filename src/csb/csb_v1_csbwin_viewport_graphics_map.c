#include "csb_v1_csbwin_viewport_graphics_map.h"

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
