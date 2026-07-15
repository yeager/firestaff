#include "csb_v1_f0149_dungeon_wall_ornament_alcove_pc34_compat.h"

int csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
    int16_t wall_ornament_index,
    const int16_t *current_map_alcove_ornament_indices,
    size_t alcove_ornament_count)
{
    size_t index;

    if (wall_ornament_index < 0 ||
        (!current_map_alcove_ornament_indices && alcove_ornament_count != 0u)) {
        return 0;
    }
    for (index = 0u; index < alcove_ornament_count; ++index) {
        if (current_map_alcove_ornament_indices[index] == wall_ornament_index) {
            return 1;
        }
    }
    return 0;
}
