#include "dm1_v1_projectile_launcher_admission_f0212_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonProjectile_Compat projectiles[2];
    DM1_ProjectileCreateRequestPc34 request;
    struct ProjectileCreateInput_Compat createInput;
    DM1_ProjectileCreateSourceReceiptPc34 createSource;
    DM1_C14PoolReservationPc34 reservation;
    DM1_V1_F0115SourcePixelsPc34 sourceSurface;
    DM1_V1_F0115SourceMaterialInputPc34 objectInput;
    DM1_V1_F0115SourceMaterialHandoffPc34 objectMaterial;
    DM1_ProjectileMaterialResolutionPc34 objectResolution;
    DM1_ProjectileLauncherPlanInputF0212Pc34 planInput;
    DM1_ProjectileLauncherPlanF0212Pc34 plan;
    DM1_V1_F0248LiveEffectMaterialInputPc34 c14Input;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 c14Material;
    DM1_V1_ObjectWorldGraphicsSurfacePc34 catalogSurface;
    DM1_V1_C14C15GraphicsCatalogPc34 catalog;
    DM1_ProjectileLauncherRuntimeInputF0212Pc34 runtimeInput;
    DM1_ProjectileLauncherRuntimeReceiptF0212Pc34 runtimeReceipt;
    unsigned char square[1] = {
        (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST)
    };
    unsigned short squareFirstThings[1] = {
        (unsigned short)(THING_TYPE_PROJECTILE << 10)
    };
    unsigned short columnBase[1] = { 0 };
    unsigned char c14Raw[16] = {
        0xfe, 0xff, 0x00, 0x14, 1, 1, 0, 0,
        0xff, 0xff, 0, 0, 0, 0, 0, 0
    };
    unsigned char weaponRaw[4] = { 0xfe, 0xff, 0, 0 };
    static const unsigned char pixels[32 * 32] = { 1 };
    static const unsigned char palette[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    unsigned short weaponThing = (unsigned short)(THING_TYPE_WEAPON << 10);
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(projectiles, 0, sizeof(projectiles));
    things.loaded = 1;
    things.projectiles = projectiles;
    things.projectileCount = 2;
    things.thingCounts[THING_TYPE_PROJECTILE] = 2;
    things.rawThingData[THING_TYPE_PROJECTILE] = c14Raw;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.rawThingData[THING_TYPE_WEAPON] = weaponRaw;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    projectiles[0].next = THING_ENDOFLIST;
    projectiles[0].slot = weaponThing;
    projectiles[0].kineticEnergy = 1;
    projectiles[0].attack = 1;
    projectiles[1].next = THING_NONE;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.columnsCumulativeSquareFirstThingCount = columnBase;
    dungeon.dungeonColumnCount = 1;
    map.width = 1;
    map.height = 1;
    tiles.squareData = square;
    tiles.squareCount = 1;

    memset(&request, 0, sizeof(request));
    request.championIndex = 0;
    request.championCell = 0;
    request.partyMapIndex = 0;
    request.partyMapX = 0;
    request.partyMapY = 0;
    request.partyDirection = 0;
    request.gameTick = 100;
    request.category = PROJECTILE_CATEGORY_KINETIC;
    request.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    request.kineticEnergy = 36;
    request.impactAttack = 24;
    request.stepEnergy = 3;
    request.launchCell = 0;
    request.launchDirection = 0;
    request.carriedThing = weaponThing;
    ok &= check(dm1_v1_build_projectile_create_input_source_bound_pc34(
                    &request, &things, &createInput, &createSource) &&
                    createSource.valid,
                "F0212 receives an authenticated raw launcher object");

    sourceSurface.pixels = pixels;
    sourceSurface.pixelCount = sizeof(pixels);
    sourceSurface.width = 32;
    sourceSurface.height = 32;
    sourceSurface.sourceOwned = 1;
    sourceSurface.verifiedPc34GraphicsDat = 1;
    memset(&objectInput, 0, sizeof(objectInput));
    objectInput.kind = DM1_V1_F0115_SOURCE_MATERIAL_THROWN_OBJECT_PC34;
    objectInput.thingType = THING_TYPE_WEAPON;
    objectInput.subtype = 0;
    objectInput.projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    objectInput.relativeForward = 1;
    objectInput.relativeCell = 2;
    objectInput.sourceZoneRow = 1;
    objectInput.viewportW = 224;
    objectInput.viewportH = 136;
    ok &= check(dm1_v1_projectile_material_resolve_pc34(
                    PROJECTILE_SUBTYPE_KINETIC_ARROW, THING_TYPE_WEAPON, 0, 0,
                    &objectResolution) && objectResolution.valid &&
                    objectResolution.uses_object_aspect,
                "F0142 selects the original launcher object aspect");
    sourceSurface.graphicIndex = (unsigned int)objectResolution.graphic_index;
    objectInput.surface = &sourceSurface;
    ok &= check(dm1_v1_f0115_source_material_handoff_pc34(
                    &objectInput, &objectMaterial) && objectMaterial.valid,
                "F0142/G0209 produces source-owned launcher aspect material");

    ok &= check(dm1_v1_c14_pool_reserve_pc34(&things, &reservation),
                "F0212 reserves an original C14 row");
    memset(&planInput, 0, sizeof(planInput));
    planInput.things = &things;
    planInput.createInput = &createInput;
    planInput.createSource = &createSource;
    planInput.c14Reservation = &reservation;
    planInput.associatedThingType = THING_TYPE_WEAPON;
    planInput.associatedThingSubtype = 0;
    planInput.weaponProjectileAspectOrdinal = 0;
    planInput.associatedMaterial = &objectMaterial;
    ok &= check(dm1_v1_projectile_launcher_plan_f0212_pc34(
                    &planInput, &plan) && plan.valid &&
                    plan.reservedC14Thing == reservation.thing,
                "F0212 binds raw launcher, aspect plan and C14 reservation");

    ok &= check(dm1_v1_c14_pool_initialize_and_link_pc34(
                    &reservation, &dungeon, weaponThing, createInput.kineticEnergy,
                    createInput.attack, 7, createInput.cell, 0, 0, 0),
                "F0212 publishes the reserved raw C14 row");
    memset(&c14Input, 0, sizeof(c14Input));
    c14Input.kind = DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34;
    c14Input.things = &things;
    c14Input.rawThing = reservation.thing;
    c14Input.associatedThing = weaponThing;
    c14Input.expectedGraphicIndex = plan.expectedGraphicIndex;
    c14Input.surface = &sourceSurface;
    c14Input.palette = palette;
    c14Input.paletteByteCount = sizeof(palette);
    c14Input.paletteOwnedByPc34GraphicsDat = 1;
    ok &= check(dm1_v1_f0248_live_effect_material_receipt_pc34(
                    &c14Input, &c14Material) && c14Material.valid,
                "published C14 keeps its original object-aspect receipt");
    memset(&catalogSurface, 0, sizeof(catalogSurface));
    catalogSurface.graphicIndex = plan.expectedGraphicIndex;
    catalogSurface.pixels = pixels;
    catalogSurface.pixelCount = sizeof(pixels);
    catalogSurface.width = 32;
    catalogSurface.height = 32;
    catalogSurface.verifiedPc34GraphicsDat = 1;
    ok &= check(dm1_v1_c14_c15_graphics_catalog_build_pc34(
                    &catalogSurface, 1, &catalog) && catalog.valid,
                "existing decoder catalog owns the launcher graphic");
    memset(&runtimeInput, 0, sizeof(runtimeInput));
    runtimeInput.plan = &plan;
    runtimeInput.c14Reservation = &reservation;
    runtimeInput.graphicsCatalog = &catalog;
    runtimeInput.c14Material = &c14Material;
    ok &= check(dm1_v1_projectile_launcher_runtime_admit_f0212_pc34(
                    &runtimeInput, &runtimeReceipt) && runtimeReceipt.valid &&
                    runtimeReceipt.shouldPublishRuntimeProjectile,
                "runtime consumes F0212 receipt without a synthetic marker");
    weaponRaw[2] ^= 1u;
    ok &= check(dm1_v1_projectile_launcher_plan_f0212_pc34(
                    &planInput, &plan) && !plan.valid,
                "raw launcher object drift fails closed before runtime publication");

    if (!ok) return 1;
    puts("PASS: DM1 F0212 PC34 launcher reservation and material admission");
    return 0;
}
