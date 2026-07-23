#ifndef CSB_V1_F0217_GROUP_LOOKUP_PC34_COMPAT_H
#define CSB_V1_F0217_GROUP_LOOKUP_PC34_COMPAT_H

#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <stdint.h>

/* ReDMCSB GROUP1.C F0175: scan a real square Thing chain for C04. */
int csb_v1_f0217_find_group_thing_pc34_compat(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int map_x,
    int map_y,
    uint16_t *out_thing);

#endif
