/* DM2 STORE_EXTRA_DUNGEON_DATA tile-header SUPPRESS size table.
 * Source: sksvgame.cpp:1992-2023. */
#include "dm2_v1_save_dungeon_data_pc34_compat.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Tile type → SUPPRESS byte count (sksvgame.cpp switch at line 1995) */
    assert(dm2_v1_save_tile_suppress_size(0) == 0); /* wall */
    assert(dm2_v1_save_tile_suppress_size(1) == 0); /* open */
    assert(dm2_v1_save_tile_suppress_size(2) == 8); /* pit */
    assert(dm2_v1_save_tile_suppress_size(3) == 0); /* stairs */
    assert(dm2_v1_save_tile_suppress_size(4) == 7); /* door */
    assert(dm2_v1_save_tile_suppress_size(5) == 8); /* teleporter */
    assert(dm2_v1_save_tile_suppress_size(6) == 4); /* fake wall */
    assert(dm2_v1_save_tile_suppress_size(7) == 0); /* open+trick */

    /* Forward teleporter reference (target map > current map → skip) */
    assert(dm2_v1_save_teleporter_is_forward_ref(3, 5) == 1); /* forward */
    assert(dm2_v1_save_teleporter_is_forward_ref(5, 3) == 0); /* backward */
    assert(dm2_v1_save_teleporter_is_forward_ref(3, 3) == 0); /* same */

    printf("PASS: dm2_v1_save_dungeon_data\n");
    return 0;
}
