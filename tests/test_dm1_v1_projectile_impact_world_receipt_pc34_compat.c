#include "dm1_v1_projectile_impact_world_receipt_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"

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
    struct DungeonExplosion_Compat explosions[2];
    struct ProjectileInstance_Compat projectile;
    struct ProjectileInstance_Compat projectileAfter;
    struct ProjectileTickResult_Compat impact;
    struct ExplosionInstance_Compat explosionBefore;
    struct ExplosionInstance_Compat explosionAfter;
    struct ExplosionTickResult_Compat explosionTick;
    struct CellContentDigest_Compat digest;
    struct TimelineEvent_Compat c25Event;
    DM1_C15PoolReservationPc34 reservation;
    DM1_C15C25PublicationReceiptPc34 c25;
    DM1_V1_F0115SourcePixelsPc34 surface;
    DM1_V1_F0248LiveEffectMaterialInputPc34 materialInput;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 c15Material;
    DM1_ProjectileAdvanceSourceReceiptPc34 advance;
    DM1_ProjectileTerminationSourceReceiptPc34 termination;
    DM1_ProjectileImpactWorldInputPc34 input;
    DM1_ProjectileImpactWorldReceiptPc34 receipt;
    unsigned char square[1] = {
        (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST
    };
    unsigned short squareFirstThings[1] = {
        (unsigned short)(THING_TYPE_EXPLOSION << 10)
    };
    unsigned short columnBase[1] = { 0 };
    unsigned char rawC15[8] = {
        0xfe, 0xff, 0x00, 48,
        0xff, 0xff, 0x00, 0x00
    };
    static const unsigned char pixels[32 * 32] = { 1 };
    static const unsigned char palette[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(explosions, 0, sizeof(explosions));
    explosions[0].next = THING_ENDOFLIST;
    explosions[0].type = DM1_EXPLOSION_FIREBALL;
    explosions[0].attack = 48;
    explosions[1].next = THING_NONE;
    things.loaded = 1;
    things.explosions = explosions;
    things.explosionCount = 2;
    things.thingCounts[THING_TYPE_EXPLOSION] = 2;
    things.rawThingData[THING_TYPE_EXPLOSION] = rawC15;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
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

    ok &= check(dm1_v1_c15_pool_reserve_pc34(&things, &reservation),
                "C15 pool reserves original unused row");
    ok &= check(dm1_v1_c15_c25_publish_pc34(
                    &reservation, &dungeon, DM1_EXPLOSION_FIREBALL, 48, 0, 0,
                    0, 0, 0, 101u, 7, &c25),
                "F0217 publishes C15 and C25 from original pool");
    surface.pixels = pixels;
    surface.pixelCount = sizeof(pixels);
    surface.graphicIndex = (unsigned int)dm1_v1_explosion_pattern_graphic_index(
        DM1_EXPLOSION_FIREBALL, 48);
    surface.width = 32;
    surface.height = 32;
    surface.sourceOwned = 1;
    surface.verifiedPc34GraphicsDat = 1;
    memset(&materialInput, 0, sizeof(materialInput));
    materialInput.kind = DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34;
    materialInput.things = &things;
    materialInput.rawThing = (unsigned short)(THING_TYPE_EXPLOSION << 10 | 1);
    materialInput.explosionType = DM1_EXPLOSION_FIREBALL;
    materialInput.explosionAttack = 48;
    materialInput.explosionCentered = 0;
    materialInput.expectedGraphicIndex = (int)surface.graphicIndex;
    materialInput.surface = &surface;
    materialInput.palette = palette;
    materialInput.paletteByteCount = sizeof(palette);
    materialInput.paletteOwnedByPc34GraphicsDat = 1;
    ok &= check(dm1_v1_f0248_live_effect_material_receipt_pc34(
                    &materialInput, &c15Material) && c15Material.valid,
                "F0217 C15 visual material has original palette");

    memset(&projectile, 0, sizeof(projectile));
    projectile.slotIndex = 0;
    projectile.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    projectile.projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    projectile.kineticEnergy = 48;
    projectile.attack = 48;
    projectile.stepEnergy = 2;
    projectile.cell = 0;
    projectile.direction = 0;
    projectile.reserved3 = 1;
    memset(&digest, 0, sizeof(digest));
    digest.sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    digest.destSquareType = PROJECTILE_ELEMENT_WALL;
    ok &= check(F0811_PROJECTILE_Advance_Compat(&projectile, &digest, 100u,
                                                 NULL, &projectileAfter, &impact),
                "F0217 wall impact dispatches real effect");
    ok &= check(impact.despawn && impact.emittedExplosion &&
                    impact.outExplosion.explosionType == DM1_EXPLOSION_FIREBALL &&
                    impact.outExplosion.attack == 48,
                "F0216/F0217 impact output is fireball C15 shaped");

    memset(&explosionBefore, 0, sizeof(explosionBefore));
    explosionBefore.slotIndex = 1;
    explosionBefore.explosionType = DM1_EXPLOSION_FIREBALL;
    explosionBefore.attack = 48;
    explosionBefore.mapIndex = 0;
    explosionBefore.mapX = 0;
    explosionBefore.mapY = 0;
    explosionBefore.cell = 0;
    memset(&c25Event, 0, sizeof(c25Event));
    c25Event.kind = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    c25Event.fireAtTick = 101u;
    c25Event.aux0 = 1;
    c25Event.aux1 = DM1_EXPLOSION_FIREBALL;
    c25Event.aux2 = 48;
    c25Event.aux4 = 7;
    ok &= check(F0220_EXPLOSION_ProcessEvent25_Compat(
                    &explosionBefore, &digest, c25Event.fireAtTick, NULL,
                    &explosionAfter, &explosionTick),
                "F0220 consumes published C25 explosion");

    memset(&advance, 0, sizeof(advance));
    advance.valid = 1;
    advance.rawC14FNV1a = 0x12345678u;
    advance.sourceEventFNV1a = 0x87654321u;
    memset(&termination, 0, sizeof(termination));
    termination.valid = 1;
    termination.shouldTerminate = 1;
    termination.rawC14FNV1a = advance.rawC14FNV1a;
    memset(&input, 0, sizeof(input));
    input.dungeon = &dungeon;
    input.things = &things;
    input.advance = &advance;
    input.termination = &termination;
    input.impact = &impact;
    input.explosionMaterial = &c15Material;
    input.c25 = &c25;
    input.c25Event = &c25Event;
    input.explosionBefore = &explosionBefore;
    input.explosionAfter = &explosionAfter;
    input.explosionResult = &explosionTick;
    ok &= check(dm1_v1_projectile_impact_world_receipt_f0216_f0220_pc34(
                    &input, &receipt) && receipt.valid && receipt.shouldDispatchImpact &&
                    receipt.shouldRemoveExplosion && receipt.rawC15FNV1a != 0u,
                "F0216-F0220 receipt binds C14/C15/C48/C49/C25 effect chain");
    rawC15[7] ^= 1u;
    ok &= check(dm1_v1_projectile_impact_world_receipt_f0216_f0220_pc34(
                    &input, &receipt) && !receipt.valid,
                "C15/C25 raw drift fails closed");

    if (!ok) return 1;
    puts("PASS: DM1 F0216-F0220 PC34 impact and world receipts");
    return 0;
}
