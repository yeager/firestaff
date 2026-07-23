#include "dm1_v1_projectile_terminal_materialization_f0217_pc34_compat.h"
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
    struct DungeonProjectile_Compat projectileRow;
    struct DungeonExplosion_Compat explosionRows[2];
    struct ProjectileInstance_Compat runtimeProjectile;
    struct ProjectileTickResult_Compat impact;
    struct TimelineEvent_Compat event;
    DM1_C15PoolReservationPc34 reservation;
    DM1_C15C25PublicationReceiptPc34 c15Publication;
    DM1_V1_F0115SourcePixelsPc34 c14Surface;
    DM1_V1_F0115SourcePixelsPc34 c15Surface;
    DM1_V1_F0115SourcePixelsPc34 objectSurface;
    DM1_V1_F0248LiveEffectMaterialInputPc34 liveInput;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 c14Material;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 c15Material;
    DM1_V1_F0115SourceMaterialInputPc34 objectInput;
    DM1_V1_F0115SourceMaterialHandoffPc34 objectMaterial;
    DM1_ProjectileMaterializationReceiptPc34 f0215;
    DM1_ProjectileTerminationSourceInputPc34 terminationInput;
    DM1_ProjectileTerminationSourceReceiptPc34 termination;
    DM1_ProjectileImpactWorldReceiptPc34 impactWorld;
    DM1_ProjectileTerminalMaterializationInputF0217Pc34 input;
    DM1_ProjectileTerminalMaterializationReceiptF0217Pc34 receipt;
    unsigned char square[1] = {
        (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST
    };
    unsigned short squareFirstThings[1] = {
        (unsigned short)(THING_TYPE_EXPLOSION << 10)
    };
    unsigned short columnBase[1] = { 0 };
    unsigned char c14Raw[8] = { 0xfe, 0xff, 0x00, 0x14, 36, 24, 0, 0 };
    unsigned char c15Raw[8] = {
        0xfe, 0xff, 0, 0,
        0xff, 0xff, 0, 0
    };
    unsigned char weaponRaw[4] = { 0xfe, 0xff, 0, 0 };
    static const unsigned char pixels[32 * 32] = { 1 };
    static const unsigned char palette[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    unsigned short projectileThing = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short weaponThing = (unsigned short)(THING_TYPE_WEAPON << 10);
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(explosionRows, 0, sizeof(explosionRows));
    things.loaded = 1;
    things.projectiles = &projectileRow;
    things.projectileCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = c14Raw;
    things.explosions = explosionRows;
    things.explosionCount = 2;
    things.thingCounts[THING_TYPE_EXPLOSION] = 2;
    things.rawThingData[THING_TYPE_EXPLOSION] = c15Raw;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.rawThingData[THING_TYPE_WEAPON] = weaponRaw;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    projectileRow.next = THING_ENDOFLIST;
    projectileRow.slot = weaponThing;
    projectileRow.kineticEnergy = 36;
    projectileRow.attack = 24;
    projectileRow.eventIndex = 0;
    explosionRows[0].next = THING_ENDOFLIST;
    explosionRows[1].next = THING_NONE;
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

    c14Surface.pixels = pixels;
    c14Surface.pixelCount = sizeof(pixels);
    c14Surface.graphicIndex = (unsigned int)dm1_v1_projectile_subtype_graphic_index(
        PROJECTILE_SUBTYPE_KINETIC_ARROW);
    c14Surface.width = 32;
    c14Surface.height = 32;
    c14Surface.sourceOwned = 1;
    c14Surface.verifiedPc34GraphicsDat = 1;
    memset(&liveInput, 0, sizeof(liveInput));
    liveInput.kind = DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34;
    liveInput.things = &things;
    liveInput.rawThing = projectileThing;
    liveInput.associatedThing = weaponThing;
    liveInput.expectedGraphicIndex = (int)c14Surface.graphicIndex;
    liveInput.surface = &c14Surface;
    liveInput.palette = palette;
    liveInput.paletteByteCount = sizeof(palette);
    liveInput.paletteOwnedByPc34GraphicsDat = 1;
    ok &= check(dm1_v1_f0248_live_effect_material_receipt_pc34(
                    &liveInput, &c14Material) && c14Material.valid,
                "C14 is bound to original PC34 pixels and palette");

    memset(&runtimeProjectile, 0, sizeof(runtimeProjectile));
    runtimeProjectile.slotIndex = 0;
    runtimeProjectile.mapIndex = 0;
    runtimeProjectile.mapX = 0;
    runtimeProjectile.mapY = 0;
    runtimeProjectile.cell = 0;
    runtimeProjectile.reserved1 = weaponThing;
    runtimeProjectile.reserved3 = 1;
    memset(&impact, 0, sizeof(impact));
    impact.despawn = 1;
    impact.emittedExplosion = 1;
    ok &= check(dm1_v1_projectile_materialization_receipt_f0215_pc34(
                    &runtimeProjectile, &impact, 0, 0, THING_ENDOFLIST, NULL, 0,
                    &f0215) && f0215.valid && f0215.shouldMaterialize,
                "F0215 retains the raw thrown object for terminal materialization");

    ok &= check(dm1_v1_c15_pool_reserve_pc34(&things, &reservation) &&
                    dm1_v1_c15_c25_publish_pc34(
                        &reservation, &dungeon, DM1_EXPLOSION_FIREBALL, 24, 0,
                        0, 0, 0, 0, 101u, 7, &c15Publication),
                "F0217 publishes a reserved raw C15 and C25 event");
    c15Surface = c14Surface;
    c15Surface.graphicIndex = (unsigned int)dm1_v1_explosion_pattern_graphic_index(
        DM1_EXPLOSION_FIREBALL, 24);
    liveInput.kind = DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34;
    liveInput.rawThing = c15Publication.slot;
    liveInput.associatedThing = THING_NONE;
    liveInput.explosionType = DM1_EXPLOSION_FIREBALL;
    liveInput.explosionAttack = 24;
    liveInput.explosionCentered = 0;
    liveInput.expectedGraphicIndex = (int)c15Surface.graphicIndex;
    liveInput.surface = &c15Surface;
    ok &= check(dm1_v1_f0248_live_effect_material_receipt_pc34(
                    &liveInput, &c15Material) && c15Material.valid,
                "published C15 retains original PC34 material receipt");

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.aux0 = 0;
    memset(&terminationInput, 0, sizeof(terminationInput));
    terminationInput.things = &things;
    terminationInput.projectileThing = projectileThing;
    terminationInput.timelineIndex = 0;
    terminationInput.timelineEvent = &event;
    terminationInput.runtimeProjectile = &runtimeProjectile;
    terminationInput.impactResult = &impact;
    terminationInput.termination = &f0215;
    terminationInput.projectileMaterial = &c14Material;
    terminationInput.explosionMaterial = &c15Material;
    ok &= check(dm1_v1_projectile_termination_source_receipt_f0213_f0215_pc34(
                    &terminationInput, &termination) && termination.valid,
                "F0213-F0215 source receipt authenticates C14/C15 handoff");

    memset(&objectInput, 0, sizeof(objectInput));
    objectInput.kind = DM1_V1_F0115_SOURCE_MATERIAL_FLOOR_OBJECT_PC34;
    objectInput.thingType = THING_TYPE_WEAPON;
    objectInput.subtype = 0;
    objectInput.relativeForward = 1;
    objectInput.relativeCell = 0;
    objectInput.sourceZoneRow = 1;
    objectInput.viewportW = 224;
    objectInput.viewportH = 136;
    objectSurface = c14Surface;
    objectSurface.graphicIndex = (unsigned int)dm1_item_sprite_index(
        THING_TYPE_WEAPON, 0);
    objectSurface.verifiedPc34GraphicsDat = 0;
    objectInput.surface = &objectSurface;
    ok &= check(dm1_v1_f0115_source_material_handoff_pc34(
                    &objectInput, &objectMaterial) && objectMaterial.valid,
                "dropped object receives its source-bound F0115 surface");

    memset(&impactWorld, 0, sizeof(impactWorld));
    impactWorld.valid = 1;
    impactWorld.c15Thing = c15Publication.slot;
    impactWorld.rawC14FNV1a = termination.rawC14FNV1a;
    impactWorld.rawC15FNV1a = c15Material.rawRecordFNV1a;
    memset(&input, 0, sizeof(input));
    input.dungeon = &dungeon;
    input.things = &things;
    input.termination = &termination;
    input.f0215 = &f0215;
    input.c14Material = &c14Material;
    input.associatedMaterial = &objectMaterial;
    input.c15Publication = &c15Publication;
    input.c15Material = &c15Material;
    input.impactWorld = &impactWorld;
    ok &= check(dm1_v1_projectile_terminal_materialization_f0217_pc34(
                    &input, &receipt) && receipt.valid && receipt.shouldDeleteC14 &&
                    receipt.shouldMaterializeAssociatedThing && receipt.shouldAdvanceC15 &&
                    receipt.rawAssociatedThingFNV1a != 0u,
                "F0217 joins authenticated C14, F0215 object and C15 reservation");
    weaponRaw[2] ^= 1u;
    ok &= check(dm1_v1_projectile_terminal_materialization_f0217_pc34(
                    &input, &receipt) && !receipt.valid,
                "raw associated Thing drift fails terminal materialization closed");

    if (!ok) return 1;
    puts("PASS: DM1 F0217 PC34 terminal C14/C15 materialization receipt");
    return 0;
}
