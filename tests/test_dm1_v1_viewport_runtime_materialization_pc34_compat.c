#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int tests;
static int passes;

#define CHECK(condition, message) do { \
    ++tests; \
    if (condition) ++passes; else printf("FAIL: %s\n", message); \
} while (0)

static DM1_V1_ViewportRuntimeMaterializationInputPc34 base_input(int origin)
{
    DM1_V1_ViewportRuntimeMaterializationInputPc34 input;
    memset(&input, 0, sizeof(input));
    input.relativeForward = 1;
    input.relativeSide = 0;
    input.elementType = 1;
    input.floorItemCount = 1;
    input.projectileCount = 1;
    input.projectileCell = 2;
    input.runtimeOrigin = (DM1_V1_ViewportRuntimeOriginPc34)origin;
    return input;
}

static void seed_live_effects(DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
                              struct ProjectileList_Compat *projectiles,
                              struct ExplosionList_Compat *explosions)
{
    memset(projectiles, 0, sizeof(*projectiles));
    memset(explosions, 0, sizeof(*explosions));
    projectiles->count = 1;
    projectiles->entries[0].slotIndex = 7;
    projectiles->entries[0].reserved3 = 1;
    projectiles->entries[0].projectileSubtype = 10;
    projectiles->entries[0].reserved1 =
        (THING_TYPE_POTION << 10) | 3;
    projectiles->entries[0].mapIndex = 2;
    projectiles->entries[0].mapX = 11;
    projectiles->entries[0].mapY = 12;
    projectiles->entries[0].cell = 3;
    projectiles->entries[0].direction = 1;
    explosions->count = 1;
    explosions->entries[0].slotIndex = 4;
    explosions->entries[0].reserved0 = 1;
    explosions->entries[0].explosionType = C000_EXPLOSION_FIREBALL;
    explosions->entries[0].mapIndex = 2;
    explosions->entries[0].mapX = 11;
    explosions->entries[0].mapY = 12;
    explosions->entries[0].currentFrame = 2;
    explosions->entries[0].maxFrames = 4;
    explosions->entries[0].attack = 96;
    input->mapIndex = 2;
    input->mapX = 11;
    input->mapY = 12;
    input->partyDirection = 0;
    input->liveProjectiles = projectiles;
    input->liveExplosions = explosions;
}

int main(void)
{
    int origin;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 d1c;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 side;
    struct ProjectileList_Compat projectiles;
    struct ExplosionList_Compat explosions;

    for (origin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
         origin <= DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34;
         ++origin) {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(origin);
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "D1C runtime decision is built");
        CHECK(d1c.valid && d1c.consumedF0172SquareFacts &&
              d1c.consumedF0115ThingPass && d1c.noM11Fallback,
              "new-start and resume paths receive a DM1-owned receipt");
        CHECK(d1c.drawFloorItems && d1c.drawRuntimeProjectiles &&
              d1c.drawDeferredSpellEffects,
              "ordinary D1C F0115 layers remain independently materialized");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34);
        seed_live_effects(&input, &projectiles, &explosions);
        input.projectileCount = 0; /* Static C14/C15 chains are not input. */
        input.projectileCell = -1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "new throw materializes through the live D1 decision");
        CHECK(d1c.liveProjectileCount == 1 && d1c.liveProjectileSlot == 7 &&
              d1c.liveProjectileSubtype == 10 && d1c.liveProjectileCell == 3 &&
              d1c.liveProjectileAssociatedThing ==
                  ((THING_TYPE_POTION << 10) | 3) &&
              d1c.liveExplosionCount == 1 && d1c.liveExplosionSlot == 4 &&
              d1c.liveExplosionFrame == 2 && d1c.liveExplosionAttack == 96 &&
              d1c.projectileZone >= 0,
              "D1 receipt preserves Projectile.Slot for F0142 material lookup");

        projectiles.entries[0].mapX = 12;
        projectiles.entries[0].cell = 1;
        explosions.entries[0].currentFrame = 3;
        input.mapX = 12;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "tick-mutated effects rebuild the D1 decision");
        CHECK(d1c.liveProjectileCount == 1 && d1c.liveProjectileCell == 1 &&
              d1c.liveExplosionCount == 0,
              "tick mutation never reuses the previous square effect list");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34);
        seed_live_effects(&input, &projectiles, &explosions);
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "save-resume rebuilds from live effect records");
        CHECK(d1c.liveProjectileCount == 1 && d1c.liveExplosionCount == 1 &&
              d1c.drawRuntimeProjectiles && d1c.drawDeferredSpellEffects,
              "save-resume takes the same D1-D3 materialization route");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34);
        input.hasVisibleChampionMirrorPayload = 1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "D1C mirror decision is built after original-save load");
        CHECK(d1c.suppressMaterializedItemPayload &&
              d1c.suppressMirrorAsFloorItem && d1c.suppressMirrorAsProjectile &&
              d1c.suppressMirrorAsSpellEffect && !d1c.drawDeferredSpellEffects,
              "D1C mirror remains a wall overlay across save provenance");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34);
        input.relativeForward = 2;
        input.relativeSide = -1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &side),
              "remaining visible side zone receives a decision");
        CHECK(side.valid && side.drawFloorItems && side.drawRuntimeProjectiles &&
              side.drawDeferredSpellEffects && !side.suppressMaterializedItemPayload,
              "D2L uses its own F0115 materialization rather than D1C fallback");
    }

    printf("%d/%d passed\n", passes, tests);
    return passes == tests ? 0 : 1;
}
