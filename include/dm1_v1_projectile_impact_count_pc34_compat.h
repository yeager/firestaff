#ifndef FIRESTAFF_DM1_V1_PROJECTILE_IMPACT_COUNT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_IMPACT_COUNT_PC34_COMPAT_H

/*
 * ReDMCSB PROJECTILE.C F0218_PROJECTILE_GetImpactCount.
 *
 * The original walks the projectile records linked at one square and cell.
 * Firestaff keeps those records in ProjectileList_Compat, so this callable
 * reports the live records at the requested impact location.  Applying an
 * impact remains the responsibility of the existing collision path, which
 * owns the target digest and lifecycle changes.
 */

#include "memory_projectile_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

int F0218_PROJECTILE_GetImpactCount_Compat(
    const struct ProjectileList_Compat *projectiles,
    int mapIndex,
    int mapX,
    int mapY,
    int cell);

/* Source owner for GROUP.C F0209's pending-impact check. It walks the
 * loaded C14 chain at one exact square/cell and requires each raw record to
 * match its decoded and live runtime owner. */
int dm1_v1_f0218_count_authenticated_c14_at_cell_pc34(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct ProjectileList_Compat* projectiles,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    int* outCount);

#endif
