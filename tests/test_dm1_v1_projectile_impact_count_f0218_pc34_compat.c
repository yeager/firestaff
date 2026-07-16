#include "dm1_v1_projectile_impact_count_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void seed_projectile(
    struct ProjectileList_Compat *list,
    int slot,
    int slotIndex,
    int active,
    int mapIndex,
    int mapX,
    int mapY,
    int cell)
{
    struct ProjectileInstance_Compat *projectile = &list->entries[slot];

    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = slotIndex;
    projectile->reserved3 = active;
    projectile->mapIndex = mapIndex;
    projectile->mapX = mapX;
    projectile->mapY = mapY;
    projectile->cell = cell;
}

static void test_source_named_boundary_counts_active_projectiles_at_cell(void)
{
    struct ProjectileList_Compat list;

    memset(&list, 0, sizeof(list));
    seed_projectile(&list, 0, 0, 1, 2, 10, 11, 3);
    seed_projectile(&list, 1, 1, 5, 2, 10, 11, 3);
    seed_projectile(&list, 2, 2, 1, 2, 10, 11, 2);
    seed_projectile(&list, 3, 3, 1, 2, 10, 12, 3);
    seed_projectile(&list, 4, 4, 1, 3, 10, 11, 3);

    assert(F0218_PROJECTILE_GetImpactCount(&list, 2, 10, 11, 3) == 2);
}

static void test_empty_and_inactive_runtime_slots_are_ignored(void)
{
    struct ProjectileList_Compat list;

    memset(&list, 0, sizeof(list));
    seed_projectile(&list, 0, -1, 1, 2, 10, 11, 3);
    seed_projectile(&list, 1, 8, 0, 2, 10, 11, 3);
    seed_projectile(&list, 2, 9, 1, 2, 10, 11, 3);

    assert(F0218_PROJECTILE_GetImpactCount(&list, 2, 10, 11, 3) == 1);
}

static void test_compat_boundary_delegates_to_source_named_boundary(void)
{
    struct ProjectileList_Compat list;

    memset(&list, 0, sizeof(list));
    seed_projectile(&list, 0, 12, 1, 1, 4, 5, 0);
    seed_projectile(&list, 1, 13, 0, 1, 4, 5, 0);

    assert(F0218_PROJECTILE_GetImpactCount_Compat(&list, 1, 4, 5, 0) == 1);
}

static void test_invalid_inputs_return_no_impacts(void)
{
    struct ProjectileList_Compat list;

    memset(&list, 0, sizeof(list));
    seed_projectile(&list, 0, 3, 1, 1, 4, 5, 0);

    assert(F0218_PROJECTILE_GetImpactCount(NULL, 1, 4, 5, 0) == 0);
    assert(F0218_PROJECTILE_GetImpactCount(&list, 1, 4, 5, -1) == 0);
    assert(F0218_PROJECTILE_GetImpactCount(&list, 1, 4, 5, 4) == 0);
}

int main(void)
{
    test_source_named_boundary_counts_active_projectiles_at_cell();
    test_empty_and_inactive_runtime_slots_are_ignored();
    test_compat_boundary_delegates_to_source_named_boundary();
    test_invalid_inputs_return_no_impacts();

    puts("ok: DM1 F0218 projectile impact count callable");
    return 0;
}
