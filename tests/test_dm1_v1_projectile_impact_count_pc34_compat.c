#include "dm1_v1_projectile_impact_count_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void add_live_projectile(struct ProjectileList_Compat *projectiles,
                                int slot,
                                int mapIndex,
                                int mapX,
                                int mapY,
                                int cell)
{
    struct ProjectileInstance_Compat *projectile = &projectiles->entries[slot];

    projectile->slotIndex = slot;
    projectile->mapIndex = mapIndex;
    projectile->mapX = mapX;
    projectile->mapY = mapY;
    projectile->cell = cell;
}

int main(void)
{
    struct ProjectileList_Compat projectiles;
    struct ProjectileList_Compat before;

    memset(&projectiles, 0, sizeof(projectiles));
    for (int slot = 0; slot < PROJECTILE_LIST_CAPACITY; ++slot) {
        projectiles.entries[slot].slotIndex = -1;
    }

    add_live_projectile(&projectiles, 1, 2, 10, 11, 3);
    add_live_projectile(&projectiles, 17, 2, 10, 11, 3);
    add_live_projectile(&projectiles, 23, 2, 10, 11, 2);
    add_live_projectile(&projectiles, 31, 2, 10, 12, 3);
    add_live_projectile(&projectiles, 44, 3, 10, 11, 3);
    projectiles.count = 5;
    before = projectiles;

    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 2, 10, 11, 3) == 2);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 2, 10, 11, 2) == 1);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 3, 10, 11, 3) == 1);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 2, 10, 11, 4) == 0);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(NULL, 2, 10, 11, 3) == 0);
    assert(memcmp(&projectiles, &before, sizeof(projectiles)) == 0);

    puts("ok: DM1 F0218 live projectile impact inventory");
    return 0;
}
