#include "csb_v1_f0149_dungeon_wall_ornament_alcove_pc34_compat.h"

#include <assert.h>

int main(void)
{
    static const int16_t alcoves[] = { 3, 17, 63 };
    (void)alcoves;

    assert(!csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
        -1, alcoves, 3u));
    assert(csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
        3, alcoves, 3u));
    assert(csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
        63, alcoves, 3u));
    assert(!csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
        18, alcoves, 3u));
    assert(!csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
        3, NULL, 0u));
    assert(!csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
        3, NULL, 1u));
    return 0;
}
