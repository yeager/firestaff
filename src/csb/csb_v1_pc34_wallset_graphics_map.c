#include "csb_v1_pc34_wallset_graphics_map.h"

#include <stddef.h>

int csb_v1_pc34_wallset_graphics_entry_index(
    int wall_set,
    CSB_V1_PC34WallSetSurface surface,
    uint32_t graphics_entry_count,
    uint32_t *out_entry_index)
{
    uint64_t entry_index;

    if (out_entry_index) *out_entry_index = 0u;
    if (!out_entry_index || wall_set < 0 ||
        surface < CSB_V1_PC34_WALLSET_DOOR_FRAME_FRONT_D0C ||
        surface > CSB_V1_PC34_WALLSET_WALL_D3C ||
        graphics_entry_count == 0u) {
        return 0;
    }

    entry_index = (uint64_t)CSB_V1_PC34_WALLSET_FIRST_GRAPHICS_ENTRY +
                  (uint64_t)(unsigned int)wall_set *
                      CSB_V1_PC34_WALLSET_GRAPHICS_ENTRY_COUNT +
                  (uint64_t)(unsigned int)surface;
    if (entry_index >= graphics_entry_count || entry_index > UINT32_MAX) {
        return 0;
    }

    *out_entry_index = (uint32_t)entry_index;
    return 1;
}
