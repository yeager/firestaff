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

int F0218_PROJECTILE_GetImpactCount_Compat(
    const struct ProjectileList_Compat *projectiles,
    int mapIndex,
    int mapX,
    int mapY,
    int cell);

#endif
