#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
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
    projectiles->entries[0].projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    projectiles->entries[0].mapIndex = 2;
    projectiles->entries[0].mapX = 11;
    projectiles->entries[0].mapY = 12;
    projectiles->entries[0].cell = 3;
    projectiles->entries[0].direction = 1;
    projectiles->entries[0].reserved1 =
        (int)((THING_TYPE_WEAPON << 10) | 12);
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

static void admit_live_pc34_surfaces(
    DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    DM1_V1_ObjectWorldGraphicsSurfacePc34 surfaces[3])
{
    /* Test-only decoded bytes. Production callers must provide bytes decoded
     * from the selected PC34 GRAPHICS.DAT, never a generated substitute. */
    static unsigned char projectilePixels[32 * 32] = { 1 };
    static unsigned char explosionPixels[32 * 32] = { 2 };
    surfaces[0].graphicIndex = dm1_v1_projectile_subtype_graphic_index(
        PROJECTILE_SUBTYPE_FIREBALL);
    surfaces[0].pixels = projectilePixels; surfaces[0].pixelCount = sizeof(projectilePixels);
    surfaces[0].width = 32; surfaces[0].height = 32;
    surfaces[0].verifiedPc34GraphicsDat = 1;
    surfaces[1].graphicIndex = dm1_v1_explosion_pattern_graphic_index(
        C000_EXPLOSION_FIREBALL, 96);
    surfaces[1].pixels = explosionPixels; surfaces[1].pixelCount = sizeof(explosionPixels);
    surfaces[1].width = 32; surfaces[1].height = 32;
    surfaces[1].verifiedPc34GraphicsDat = 1;
    surfaces[2] = surfaces[0];
    surfaces[2].graphicIndex = 345;
    input->pc34GraphicsSurfaces = surfaces;
    input->pc34GraphicsSurfaceCount = 3;
}

static void admit_live_catalog(
    DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    DM1_V1_ObjectWorldGraphicsSurfacePc34 surfaces[3],
    DM1_V1_C14C15GraphicsCatalogPc34 *catalog,
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 receipts[2])
{
    memset(receipts, 0, sizeof(DM1_V1_F0248LiveEffectMaterialReceiptPc34) * 2);
    CHECK(dm1_v1_c14_c15_graphics_catalog_build_pc34(surfaces, 3, catalog) &&
          catalog->valid, "authenticated PC34 decoder catalog builds");
    receipts[0].valid = 1; receipts[0].saveReceiptBound = 1;
    receipts[0].rawThing = (unsigned short)((THING_TYPE_PROJECTILE << 10) | 7);
    receipts[0].associatedThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 12);
    receipts[0].graphicIndex = 345;
    receipts[0].rawRecordFNV1a = receipts[0].graphicsPixelsFNV1a =
        receipts[0].paletteFNV1a = 1u;
    receipts[1].valid = 1; receipts[1].saveReceiptBound = 1;
    receipts[1].rawThing = (unsigned short)((THING_TYPE_EXPLOSION << 10) | 4);
    receipts[1].associatedThing = THING_NONE;
    receipts[1].graphicIndex = surfaces[1].graphicIndex;
    receipts[1].rawRecordFNV1a = receipts[1].graphicsPixelsFNV1a =
        receipts[1].paletteFNV1a = 1u;
    input->c14C15GraphicsCatalog = catalog;
    input->c14C15MaterialReceipts = receipts;
    input->c14C15MaterialReceiptCount = 2;
}

int main(void)
{
    int origin;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 d1c;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 side;
    struct ProjectileList_Compat projectiles;
    struct ExplosionList_Compat explosions;
    DM1_V1_ObjectWorldGraphicsSurfacePc34 surfaces[3];

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
        /* A carried-object C14 needs its exact F0142/G0209 receipt, so the
         * generic native bank must reject it even when other PC34 surfaces
         * are available. */
        admit_live_pc34_surfaces(&input, surfaces);
        input.projectileCount = 0; /* Static C14/C15 chains are not input. */
        input.projectileCell = -1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "new throw materializes through the live D1 decision");
        CHECK(d1c.liveProjectileCount == 1 && d1c.liveProjectileSlot == 7 &&
              d1c.liveProjectileSubtype == PROJECTILE_SUBTYPE_FIREBALL &&
              d1c.liveProjectileCell == 3 &&
              d1c.liveProjectileAssociatedThing ==
                  (unsigned short)((THING_TYPE_WEAPON << 10) | 12) &&
              d1c.liveExplosionCount == 1 && d1c.liveExplosionSlot == 4 &&
              d1c.liveExplosionFrame == 2 && d1c.liveExplosionAttack == 96 &&
              d1c.admittedLiveProjectileCount == 0 &&
              d1c.admittedLiveExplosionCount == 1 && d1c.projectileZone < 0,
              "live C14/C15 records retain identity while missing C14 material fails closed");

        projectiles.entries[0].reserved1 = THING_NONE;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c) &&
              d1c.admittedLiveProjectileCount == 1 && d1c.projectileZone >= 0,
              "native C14 admits only verified PC34 GRAPHICS.DAT material");

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
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34);
        DM1_V1_C14C15GraphicsCatalogPc34 catalog;
        DM1_V1_F0248LiveEffectMaterialReceiptPc34 receipts[2];
        seed_live_effects(&input, &projectiles, &explosions);
        admit_live_pc34_surfaces(&input, surfaces);
        admit_live_catalog(&input, surfaces, &catalog, receipts);
        input.projectileCount = 0;
        input.projectileCell = -1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c) &&
              d1c.admittedLiveProjectileCount == 1 &&
              d1c.admittedLiveExplosionCount == 1,
              "F0142/G0209 C14 and C15 bind only through catalog-backed receipts");
        receipts[0].associatedThing = THING_NONE;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c) &&
              d1c.admittedLiveProjectileCount == 0 &&
              d1c.admittedLiveExplosionCount == 1,
              "C14 Slot drift fails closed without hiding a valid C15");
        receipts[0].associatedThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 12);
        ((unsigned char *)surfaces[2].pixels)[0] ^= 0x0f;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c) &&
              d1c.admittedLiveProjectileCount == 0,
              "catalog fingerprint rejects substituted decoded C14 pixels");
        ((unsigned char *)surfaces[2].pixels)[0] ^= 0x0f;
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34);
        seed_live_effects(&input, &projectiles, &explosions);
        admit_live_pc34_surfaces(&input, surfaces);
        projectiles.count = 2;
        projectiles.entries[1] = projectiles.entries[0];
        projectiles.entries[1].slotIndex = 8;
        projectiles.entries[1].projectileSubtype = 11;
        projectiles.entries[1].cell = 1;
        projectiles.entries[1].direction = 3;
        projectiles.entries[1].reserved1 =
            (int)((THING_TYPE_POTION << 10) | 6);
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "same-square live projectile list builds");
        CHECK(d1c.liveProjectileCount == 2 &&
              d1c.admittedLiveProjectileCount == 0 &&
              d1c.liveRenderableProjectileCount == 0,
              "associated-object C14s cannot borrow native projectile material");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34);
        seed_live_effects(&input, &projectiles, &explosions);
        admit_live_pc34_surfaces(&input, surfaces);
        input.projectileCount = 0;
        input.projectileCell = -1;
        input.relativeForward = 3; /* G2028 row 0: cells 0/1 are no-draw. */
        projectiles.count = 2;
        projectiles.entries[1] = projectiles.entries[0];
        projectiles.entries[0].cell = 0;
        projectiles.entries[1].slotIndex = 8;
        projectiles.entries[1].cell = 3;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "mixed C2900-cell projectile receipt builds");
        CHECK(d1c.liveRenderableProjectileCount == 0 &&
              !d1c.drawRuntimeProjectiles,
              "unadmitted C14s cannot open a C2900 render lane");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34);
        seed_live_effects(&input, &projectiles, &explosions);
        projectiles.entries[0].reserved1 = THING_NONE;
        admit_live_pc34_surfaces(&input, surfaces);
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "save-resume rebuilds from live effect records");
        CHECK(d1c.liveProjectileCount == 1 && d1c.liveExplosionCount == 1 &&
              d1c.admittedLiveProjectileCount == 1 &&
              d1c.admittedLiveExplosionCount == 1 &&
              d1c.drawRuntimeProjectiles && d1c.drawDeferredSpellEffects,
              "save-resume takes the same D1-D3 materialization route");
    }

    {
        DM1_V1_ViewportRuntimeMaterializationInputPc34 input = base_input(
            DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34);
        DM1_F0115AlcoveItemMaterialPlanPc34 alcove;
        input.hasVisibleChampionMirrorPayload = 1;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c),
              "D1C mirror decision is built after original-save load");
        CHECK(d1c.suppressMaterializedItemPayload &&
              d1c.suppressMirrorAsFloorItem && d1c.suppressMirrorAsProjectile &&
              d1c.suppressMirrorAsSpellEffect && !d1c.drawDeferredSpellEffects,
              "D1C mirror remains a wall overlay across save provenance");
        input.elementType = DUNGEON_ELEMENT_WALL;
        CHECK(dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &d1c) &&
              d1c.valid && !d1c.drawFloorItems &&
              d1c.suppressMaterializedItemPayload &&
              dm1_v1_f0115_alcove_item_material_plan_pc34(
                  &alcove, THING_TYPE_WEAPON, 0, 1, 0, 2) &&
              alcove.coordinate_binding_ready && alcove.source_zone >= 2548,
              "original-save C127 suppression survives the otherwise legal C2548 alcove plan");
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
