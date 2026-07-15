#ifndef FIRESTAFF_CSB_V1_F0149_DUNGEON_WALL_ORNAMENT_ALCOVE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0149_DUNGEON_WALL_ORNAMENT_ALCOVE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB DUNGEON.C F0149_DUNGEON_IsWallOrnamentAnAlcove (1330-1347):
 * negative ornament ordinals are never alcoves; otherwise compare against the
 * caller-owned current-map G0267 alcove-ornament table. */
int csb_v1_f0149_dungeon_wall_ornament_is_alcove_pc34_compat(
    int16_t wall_ornament_index,
    const int16_t *current_map_alcove_ornament_indices,
    size_t alcove_ornament_count);

#endif
