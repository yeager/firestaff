#ifndef CSB_V1_VIEWPORT_WALL_ORNAMENT_ORDINAL_RESOLVER_PC34_COMPAT_H
#define CSB_V1_VIEWPORT_WALL_ORNAMENT_ORDINAL_RESOLVER_PC34_COMPAT_H

/*
 * CSB wall ornament ordinal resolver.
 *
 * Resolves wall ornament ordinals directly from CSB_V1_DungeonData raw
 * bytes without requiring a DungeonThings_Compat adapter. Matches the
 * DM1_ViewportWallOrnamentOrdinalCallback signature so it can be wired
 * into the CSB viewport config.
 *
 * Phase 1: walk the thing list at (map_x, map_y), check sensor records
 *          for ornamentOrdinal > 0 (bits 15:12 of bytes 4-5).
 * Phase 2: compute random ornament from map metadata via F0170/F0171.
 *
 * Source: ReDMCSB DUNGEON.C F0172 sensor scan + F0170/F0171 random.
 */

#include <stdint.h>

struct CSB_V1_DungeonData;

typedef struct {
    const struct CSB_V1_DungeonData *dungeon;
    int level;
    int randomWallOrnamentCount;
    uint16_t ornamentRandomSeed;
} CSB_V1_WallOrnamentOrdinalResolverPc34;

int csb_v1_viewport_wall_ornament_ordinal_resolve_pc34(
    void *user_data,
    int map_x, int map_y);

#endif
