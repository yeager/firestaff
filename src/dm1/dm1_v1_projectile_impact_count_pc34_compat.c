#include "dm1_v1_projectile_impact_count_pc34_compat.h"

int F0218_PROJECTILE_GetImpactCount(
    const struct ProjectileList_Compat *projectiles,
    int mapIndex,
    int mapX,
    int mapY,
    int cell)
{
    int count = 0;
    int slot;

    if (!projectiles || cell < 0 || cell > 3) {
        return 0;
    }

    /* F0218 scans the square's projectile chain.  The compatibility list
     * has no square-local chain, so scan its bounded live records instead. */
    for (slot = 0; slot < PROJECTILE_LIST_CAPACITY; ++slot) {
        const struct ProjectileInstance_Compat *projectile =
            &projectiles->entries[slot];

        if (projectile->slotIndex < 0 || projectile->reserved3 == 0 ||
            projectile->mapIndex != mapIndex || projectile->mapX != mapX ||
            projectile->mapY != mapY || projectile->cell != cell) {
            continue;
        }
        ++count;
    }

    return count;
}

int F0218_PROJECTILE_GetImpactCount_Compat(
    const struct ProjectileList_Compat *projectiles,
    int mapIndex,
    int mapX,
    int mapY,
    int cell)
{
    return F0218_PROJECTILE_GetImpactCount(
        projectiles, mapIndex, mapX, mapY, cell);
}
