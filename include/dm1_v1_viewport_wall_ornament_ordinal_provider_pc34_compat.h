#ifndef FIRESTAFF_DM1_V1_VIEWPORT_WALL_ORNAMENT_ORDINAL_PROVIDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_WALL_ORNAMENT_ORDINAL_PROVIDER_PC34_COMPAT_H

#include <stdint.h>
#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Wall ornament ordinal provider for the DM1 viewport 3D callback interface.
 *
 * Resolves ornament ordinals from dungeon thing data:
 *   1. Sensor-placed ornaments (ornamentOrdinal field on sensor things)
 *   2. Random ornaments (F0170/F0171 map metadata formula)
 *
 * Sensor ornaments override random ornaments per ReDMCSB DUNGEON.C F0172.
 *
 * Usage: populate a DM1_V1_WallOrnamentOrdinalProviderPc34 struct with
 * dungeon data pointers, then pass it as user_data to the viewport callback
 * with dm1_v1_viewport_wall_ornament_ordinal_resolve_pc34 as the function.
 */

typedef struct {
    const struct DungeonThings_Compat *things;
    int mapIndex;
    int mapWidth;
    int mapHeight;
    int randomWallOrnamentCount;
    uint16_t ornamentRandomSeed;
} DM1_V1_WallOrnamentOrdinalProviderPc34;

int dm1_v1_viewport_wall_ornament_ordinal_resolve_pc34(
    void *user_data,
    int map_x, int map_y);

#ifdef __cplusplus
}
#endif

#endif
