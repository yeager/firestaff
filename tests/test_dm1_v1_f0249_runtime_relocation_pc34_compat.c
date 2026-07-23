/* ReDMCSB TIMELINE.C F0249 through the live DM1 F0267 move consumer. */
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); return 1; } \
} while (0)

static void init_world(struct GameWorld_Compat* world,
                       struct DungeonDatState_Compat* dungeon,
                       struct DungeonThings_Compat* things,
                       struct DungeonMapDesc_Compat* map,
                       struct DungeonMapTiles_Compat* tiles,
                       unsigned char squareData[2],
                       unsigned short firstThings[2],
                       struct DungeonProjectile_Compat* projectile,
                       struct DungeonExplosion_Compat* explosion,
                       unsigned char rawProjectile[8],
                       unsigned char rawExplosion[4]) {
    memset(world, 0, sizeof(*world));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(things, 0, sizeof(*things));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(squareData, DUNGEON_ELEMENT_CORRIDOR << 5, 2);
    memset(firstThings, 0xff, 2 * sizeof(*firstThings));
    memset(projectile, 0, sizeof(*projectile));
    memset(explosion, 0, sizeof(*explosion));
    memset(rawProjectile, 0, 8);
    memset(rawExplosion, 0, 4);

    map->width = 2;
    map->height = 1;
    tiles->squareData = squareData;
    tiles->squareCount = 2;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;

    things->loaded = 1;
    things->squareFirstThings = firstThings;
    things->squareFirstThingCount = 2;
    things->projectiles = projectile;
    things->projectileCount = 1;
    things->explosions = explosion;
    things->explosionCount = 1;
    things->thingCounts[THING_TYPE_PROJECTILE] = 1;
    things->thingCounts[THING_TYPE_EXPLOSION] = 1;
    things->rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    things->rawThingData[THING_TYPE_EXPLOSION] = rawExplosion;
    world->dungeon = dungeon;
    world->things = things;
}

static int test_f0249_c14_relocation_precedes_champion_impact(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat tick;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonProjectile_Compat sourceProjectile;
    struct F0267ThingMoveRequestPc34Compat request;
    struct F0267ThingMoveResultPc34Compat move;
    unsigned char squareData[3];
    unsigned short firstThings[3];
    unsigned char rawProjectile[8];
    unsigned short projectileThing =
        (unsigned short)((THING_TYPE_PROJECTILE << 10) | (1u << 14));
    int hpAfterImpact;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&tick, 0, sizeof(tick));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(&request, 0, sizeof(request));
    memset(&move, 0, sizeof(move));
    memset(squareData, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squareData));
    memset(firstThings, 0xff, sizeof(firstThings));
    memset(rawProjectile, 0xff, sizeof(rawProjectile));

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    map.width = 3;
    map.height = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 3;
    squareData[0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[1] |= DUNGEON_SQUARE_MASK_THING_LIST;
    firstThings[0] = projectileThing;
    things.loaded = 1;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 3;
    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    sourceProjectile.next = THING_ENDOFLIST;
    sourceProjectile.slot = THING_NONE;
    sourceProjectile.kineticEnergy = 40;
    sourceProjectile.attack = 40;
    sourceProjectile.eventIndex = 0;
    rawProjectile[4] = 40;
    rawProjectile[5] = 40;
    rawProjectile[6] = 0;
    rawProjectile[7] = 0;

    world.dungeon = &dungeon;
    world.things = &things;
    world.partyMapIndex = 0;
    world.party.mapIndex = 0;
    world.party.mapX = 2;
    world.party.mapY = 0;
    world.party.championCount = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].cell = 0;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;
    world.gameTick = 100;
    world.timeline.nowTick = 100;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;
    world.projectiles.entries[0].cell = 1;
    world.projectiles.entries[0].direction = 1;
    world.projectiles.entries[0].kineticEnergy = 40;
    world.projectiles.entries[0].attack = 40;
    world.projectiles.entries[0].stepEnergy = 4;
    world.timeline.count = 1;
    world.timeline.events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[0].fireAtTick = 100;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 0;
    world.timeline.events[0].mapY = 0;
    world.timeline.events[0].cell = 1;
    world.timeline.events[0].aux0 = 0;

    request.thing = projectileThing;
    request.sourceMapIndex = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &move),
          "F0249 moves authenticated C14 before its C48 impact");
    CHECK(move.moved && move.timelineRelocationCount == 1 &&
          world.projectiles.entries[0].mapX == 1 &&
          world.timeline.events[0].mapX == 1,
          "F0249 keeps C14, live M10 projectile, and C48 location aligned");
    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &tick) == ORCH_OK,
          "C48 dispatches after F0249 C14 relocation");
    CHECK(world.party.champions[0].hp.current < 100 &&
          world.projectiles.entries[0].reserved3 == 0 &&
          sourceProjectile.eventIndex == 0xFFFFu &&
          world.timeline.count == 0,
          "F0217 damages the destination champion once and retires C14/C48");
    hpAfterImpact = world.party.champions[0].hp.current;
    CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &tick) == ORCH_OK &&
          world.party.champions[0].hp.current == hpAfterImpact,
          "retired C14/C48 cannot apply champion damage twice");
    return 0;
}

static int test_f0249_c14_c04_teleporter_rotates_m10_and_c48(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct DungeonProjectile_Compat projectile;
    struct DungeonTeleporter_Compat teleporter;
    struct F0267ThingMoveRequestPc34Compat request;
    struct F0267ThingMoveResultPc34Compat move;
    unsigned char squareData[2][3];
    unsigned short firstThings[6];
    unsigned char rawProjectile[8];
    unsigned short projectileThing = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short teleporterThing = (unsigned short)(THING_TYPE_TELEPORTER << 10);

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&projectile, 0, sizeof(projectile));
    memset(&teleporter, 0, sizeof(teleporter));
    memset(&request, 0, sizeof(request));
    memset(&move, 0, sizeof(move));
    memset(squareData, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squareData));
    memset(firstThings, 0xff, sizeof(firstThings));
    memset(rawProjectile, 0, sizeof(rawProjectile));

    maps[0].width = maps[1].width = 3;
    maps[0].height = maps[1].height = 1;
    tiles[0].squareData = squareData[0];
    tiles[1].squareData = squareData[1];
    tiles[0].squareCount = tiles[1].squareCount = 3;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;

    /* A loaded object-scope C04 teleporter is the real F0267 route: C14
     * reaches its target square, F0263 rotates direction+packed cell, then
     * F0249 relocates only its physical C48 owner. */
    squareData[0][0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[0][1] = (DUNGEON_ELEMENT_TELEPORTER << 5) | 0x18;
    squareData[1][2] |= DUNGEON_SQUARE_MASK_THING_LIST;
    firstThings[0] = projectileThing;
    firstThings[1] = teleporterThing;
    projectile.next = THING_ENDOFLIST;
    projectile.slot = THING_NONE;
    teleporter.next = THING_ENDOFLIST;
    teleporter.scope = 2;
    teleporter.targetMapIndex = 1;
    teleporter.targetMapX = 2;
    teleporter.targetMapY = 0;
    teleporter.rotation = 1;
    teleporter.absoluteRotation = 0;
    rawProjectile[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawProjectile[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    things.loaded = 1;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 6;
    things.projectiles = &projectile;
    things.projectileCount = 1;
    things.teleporters = &teleporter;
    things.teleporterCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    world.dungeon = &dungeon;
    world.things = &things;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;
    world.projectiles.entries[0].cell = 0;
    world.projectiles.entries[0].direction = 1;
    world.timeline.count = 1;
    world.timeline.events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[0].aux0 = 0;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 0;
    world.timeline.events[0].mapY = 0;
    world.timeline.events[0].cell = 0;

    request.thing = projectileThing;
    request.sourceMapIndex = 0;
    request.sourceMapX = 0;
    request.sourceMapY = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    request.destinationMapY = 0;
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &move),
          "F0267 routes C14 through an open object-scope C04 teleporter");
    CHECK(move.teleporterChainCount == 1 && move.finalMapIndex == 1 &&
          move.finalMapX == 2 && move.finalMapY == 0 &&
          THING_GET_CELL(move.finalThing) == 1,
          "F0263 relative C04 rotation updates C14's packed destination cell");
    CHECK(world.projectiles.entries[0].mapIndex == 1 &&
          world.projectiles.entries[0].mapX == 2 &&
          world.projectiles.entries[0].mapY == 0 &&
          world.projectiles.entries[0].cell == 1 &&
          world.projectiles.entries[0].direction == 2,
          "F0263 keeps the active M10 C14 projection direction and cell aligned");
    CHECK(world.timeline.count == 1 &&
          world.timeline.events[0].kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
          world.timeline.events[0].aux0 == 0 &&
          world.timeline.events[0].mapIndex == 1 &&
          world.timeline.events[0].mapX == 2 &&
          world.timeline.events[0].mapY == 0 &&
          world.timeline.events[0].cell == 1,
          "F0249 relocates exactly the C14-owned C48 after the C04 rotation");
    return 0;
}

static int test_f0249_c14_c04_chain_accumulates_rotation_once(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat maps[3];
    struct DungeonMapTiles_Compat tiles[3];
    struct DungeonProjectile_Compat projectile;
    struct DungeonTeleporter_Compat teleporters[2];
    struct F0267ThingMoveRequestPc34Compat request;
    struct F0267ThingMoveResultPc34Compat move;
    unsigned char squareData[3][3];
    unsigned short firstThings[4];
    unsigned char rawProjectile[8];
    unsigned short projectileThing = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short teleporterThing = (unsigned short)(THING_TYPE_TELEPORTER << 10);

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&projectile, 0, sizeof(projectile));
    memset(teleporters, 0, sizeof(teleporters));
    memset(&request, 0, sizeof(request));
    memset(&move, 0, sizeof(move));
    memset(squareData, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squareData));
    memset(firstThings, 0xff, sizeof(firstThings));
    memset(rawProjectile, 0, sizeof(rawProjectile));

    for (int mapIndex = 0; mapIndex < 3; ++mapIndex) {
        maps[mapIndex].width = 3;
        maps[mapIndex].height = 1;
        tiles[mapIndex].squareData = squareData[mapIndex];
        tiles[mapIndex].squareCount = 3;
    }
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 3;
    dungeon.maps = maps;
    dungeon.tiles = tiles;

    /* Two real loaded object-scope C04 hops: each relative F0263 rotation
     * advances both C14 direction and its packed Generic cell before F0249
     * resolves the single physical C48 owner at the terminal square. */
    squareData[0][0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[0][1] = (DUNGEON_ELEMENT_TELEPORTER << 5) | 0x18;
    squareData[1][1] = (DUNGEON_ELEMENT_TELEPORTER << 5) | 0x18;
    squareData[2][2] |= DUNGEON_SQUARE_MASK_THING_LIST;
    firstThings[0] = projectileThing;
    firstThings[1] = teleporterThing;
    /* squareFirstThings is compact: source and first C04 consume entries
     * 0/1, then map 1's C04 is entry 2 and the terminal square is entry 3. */
    firstThings[2] = (unsigned short)(teleporterThing | 1u);
    projectile.next = THING_ENDOFLIST;
    projectile.slot = THING_NONE;
    teleporters[0].next = THING_ENDOFLIST;
    teleporters[0].scope = 2;
    teleporters[0].targetMapIndex = 1;
    teleporters[0].targetMapX = 1;
    teleporters[0].targetMapY = 0;
    teleporters[0].rotation = 1;
    teleporters[1].next = THING_ENDOFLIST;
    teleporters[1].scope = 2;
    teleporters[1].targetMapIndex = 2;
    teleporters[1].targetMapX = 2;
    teleporters[1].targetMapY = 0;
    teleporters[1].rotation = 1;
    rawProjectile[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawProjectile[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    things.loaded = 1;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 4;
    things.projectiles = &projectile;
    things.projectileCount = 1;
    things.teleporters = teleporters;
    things.teleporterCount = 2;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    world.dungeon = &dungeon;
    world.things = &things;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;
    world.projectiles.entries[0].cell = 0;
    world.projectiles.entries[0].direction = 1;
    world.timeline.count = 1;
    world.timeline.events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[0].aux0 = 0;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 0;
    world.timeline.events[0].mapY = 0;
    world.timeline.events[0].cell = 0;

    request.thing = projectileThing;
    request.sourceMapIndex = 0;
    request.sourceMapX = 0;
    request.sourceMapY = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    request.destinationMapY = 0;
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &move),
          "F0267 routes C14 through both loaded C04 teleporters");
    CHECK(move.teleporterChainCount == 2 && move.finalMapIndex == 2 &&
          move.finalMapX == 2 && move.finalMapY == 0 &&
          THING_GET_CELL(move.finalThing) == 2,
          "two F0263 hops accumulate the raw C14 packed cell");
    CHECK(world.projectiles.entries[0].mapIndex == 2 &&
          world.projectiles.entries[0].mapX == 2 &&
          world.projectiles.entries[0].mapY == 0 &&
          world.projectiles.entries[0].cell == 2 &&
          world.projectiles.entries[0].direction == 3,
          "two F0263 hops accumulate active M10 C14 direction and cell");
    CHECK(world.timeline.count == 1 &&
          world.timeline.events[0].kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
          world.timeline.events[0].aux0 == 0 &&
          world.timeline.events[0].mapIndex == 2 &&
          world.timeline.events[0].mapX == 2 &&
          world.timeline.events[0].mapY == 0 &&
          world.timeline.events[0].cell == 2,
          "F0249 retains one physical C14-owned C48 across a C04 chain");
    return 0;
}

static int test_f0249_c14_open_c02_then_c04_continues_one_owner(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat tick;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct DungeonProjectile_Compat sourceProjectile;
    struct DungeonExplosion_Compat explosion;
    struct DungeonDoor_Compat door;
    struct DungeonTeleporter_Compat teleporter;
    struct DungeonWeapon_Compat weapon;
    unsigned char squareData[2][3];
    unsigned short firstThings[4];
    unsigned char rawDoor[4];
    unsigned char rawTeleporter[6];
    unsigned char rawProjectile[8];
    unsigned char rawExplosion[4];
    unsigned short projectileThing =
        (unsigned short)((THING_TYPE_PROJECTILE << 10) | (1u << 14));
    unsigned short weaponThing = (unsigned short)(THING_TYPE_WEAPON << 10);
    unsigned short teleporterThing = (unsigned short)(THING_TYPE_TELEPORTER << 10);
    int i;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&tick, 0, sizeof(tick));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(&explosion, 0, sizeof(explosion));
    memset(&door, 0, sizeof(door));
    memset(&teleporter, 0, sizeof(teleporter));
    memset(&weapon, 0, sizeof(weapon));
    memset(squareData, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squareData));
    memset(firstThings, 0xff, sizeof(firstThings));
    memset(rawDoor, 0, sizeof(rawDoor));
    memset(rawTeleporter, 0, sizeof(rawTeleporter));
    memset(rawProjectile, 0, sizeof(rawProjectile));
    memset(rawExplosion, 0, sizeof(rawExplosion));

    maps[0].width = maps[1].width = 3;
    maps[0].height = maps[1].height = 1;
    tiles[0].squareData = squareData[0];
    tiles[1].squareData = squareData[1];
    tiles[0].squareCount = tiles[1].squareCount = 3;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;

    /* C14 first crosses an open loaded C02 door, then reaches the loaded
     * object-scope C04.  This is F0219 motion followed by MOVESENS.C F0263:
     * the associated material and momentum stay on the real C14 owner while
     * its sole C48 is requeued at the teleporter target. */
    squareData[0][0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[0][1] = (DUNGEON_ELEMENT_DOOR << 5) |
        DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[0][2] = (DUNGEON_ELEMENT_TELEPORTER << 5) | 0x18;
    squareData[1][2] |= DUNGEON_SQUARE_MASK_THING_LIST;
    firstThings[0] = projectileThing;
    firstThings[1] = 0; /* C02 Door, index 0 */
    firstThings[2] = teleporterThing;
    sourceProjectile.next = THING_ENDOFLIST;
    sourceProjectile.slot = weaponThing;
    sourceProjectile.kineticEnergy = 40;
    sourceProjectile.attack = 30;
    sourceProjectile.eventIndex = 0;
    door.next = THING_ENDOFLIST;
    teleporter.next = THING_ENDOFLIST;
    teleporter.scope = 2;
    teleporter.targetMapIndex = 1;
    teleporter.targetMapX = 2;
    teleporter.targetMapY = 0;
    teleporter.rotation = 1;
    weapon.next = THING_ENDOFLIST;
    weapon.type = 1;
    rawDoor[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawDoor[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawTeleporter[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawTeleporter[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawProjectile[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawProjectile[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawProjectile[2] = (unsigned char)(weaponThing & 0xffu);
    rawProjectile[3] = (unsigned char)(weaponThing >> 8);
    rawProjectile[4] = 40;
    rawProjectile[5] = 30;
    rawProjectile[6] = 0;
    rawProjectile[7] = 0;
    explosion.next = THING_NONE;
    things.loaded = 1;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 4;
    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.explosions = &explosion;
    things.explosionCount = 1;
    things.doors = &door;
    things.doorCount = 1;
    things.teleporters = &teleporter;
    things.teleporterCount = 1;
    things.weapons = &weapon;
    things.weaponCount = 1;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    things.rawThingData[THING_TYPE_DOOR] = rawDoor;
    things.thingCounts[THING_TYPE_TELEPORTER] = 1;
    things.rawThingData[THING_TYPE_TELEPORTER] = rawTeleporter;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = rawExplosion;
    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 100;
    world.timeline.nowTick = 100;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved1 = weaponThing;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;
    world.projectiles.entries[0].cell = 1;
    world.projectiles.entries[0].direction = 1;
    world.projectiles.entries[0].kineticEnergy = 40;
    world.projectiles.entries[0].attack = 30;
    world.projectiles.entries[0].stepEnergy = 4;
    world.timeline.count = 1;
    world.timeline.events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[0].fireAtTick = 100;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 0;
    world.timeline.events[0].mapY = 0;
    world.timeline.events[0].cell = 1;
    world.timeline.events[0].aux0 = 0;

    for (i = 0; i < 3; ++i) {
        CHECK(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &tick) == ORCH_OK,
              "C48 continues C14 through the open C02/C04 route");
    }
    CHECK(world.projectiles.entries[0].reserved3 != 0 &&
          world.projectiles.entries[0].reserved1 == weaponThing &&
          sourceProjectile.slot == weaponThing &&
          world.projectiles.entries[0].kineticEnergy == 28 &&
          world.projectiles.entries[0].attack == 18 &&
          rawProjectile[4] == 28 && rawProjectile[5] == 18,
          "C14 preserves material identity and F0219 momentum across C02/C04");
    CHECK(world.projectiles.entries[0].mapIndex == 1 &&
          world.projectiles.entries[0].mapX == 2 &&
          world.projectiles.entries[0].mapY == 0 &&
          world.projectiles.entries[0].cell == 1 &&
          world.projectiles.entries[0].direction == 2,
          "F0263 publishes the C04 target and packed rotation to active C14");
    CHECK(world.timeline.count == 1 && sourceProjectile.eventIndex == 0 &&
          rawProjectile[6] == 0 && rawProjectile[7] == 0 &&
          world.timeline.events[0].kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
          world.timeline.events[0].aux0 == 0 &&
          world.timeline.events[0].mapIndex == 1 &&
          world.timeline.events[0].mapX == 2 &&
          world.timeline.events[0].mapY == 0 &&
          world.timeline.events[0].cell == 1,
          "F0249/F0219 retains one physical C14-owned C48 at the C04 target");
    return 0;
}

int main(void) {
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squareData[2];
    unsigned short firstThings[2];
    struct DungeonProjectile_Compat projectile;
    struct DungeonExplosion_Compat explosion;
    unsigned char rawProjectile[8];
    unsigned char rawExplosion[4];
    struct F0267ThingMoveRequestPc34Compat request;
    struct F0267ThingMoveResultPc34Compat result;
    unsigned short projectileThing =
        (unsigned short)((THING_TYPE_PROJECTILE << 10) | (2u << 14));
    unsigned short explosionThing =
        (unsigned short)((THING_TYPE_EXPLOSION << 10) | (1u << 14));

    if (test_f0249_c14_relocation_precedes_champion_impact()) return 1;
    if (test_f0249_c14_c04_teleporter_rotates_m10_and_c48()) return 1;
    if (test_f0249_c14_c04_chain_accumulates_rotation_once()) return 1;
    if (test_f0249_c14_open_c02_then_c04_continues_one_owner()) return 1;

    init_world(&world, &dungeon, &things, &map, &tiles, squareData,
               firstThings, &projectile, &explosion, rawProjectile,
               rawExplosion);
    squareData[0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[1] |= DUNGEON_SQUARE_MASK_THING_LIST;
    firstThings[0] = projectileThing;
    firstThings[1] = THING_ENDOFLIST;
    projectile.next = THING_ENDOFLIST;
    rawProjectile[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawProjectile[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    world.timeline.count = 2;
    world.timeline.events[0].kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    world.timeline.events[0].aux0 = 0;
    world.timeline.events[0].mapX = 0;
    world.timeline.events[0].mapY = 0;
    world.timeline.events[0].cell = 0;
    world.timeline.events[1] = world.timeline.events[0];
    world.timeline.events[1].aux0 = 1;

    memset(&request, 0, sizeof(request));
    request.thing = projectileThing;
    request.sourceMapIndex = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &result),
          "F0267 moves the raw C14 owner");
    CHECK(result.moved && result.destinationLinked &&
          result.timelineRelocationCount == 1 &&
          F0511_DUNGEON_GetSquareFirstThing_Compat(
              &dungeon, &things, 0, 0, 0) == THING_ENDOFLIST &&
          projectile.next == THING_ENDOFLIST &&
          rawProjectile[0] == (unsigned char)(THING_ENDOFLIST & 0xffu) &&
          rawProjectile[1] == (unsigned char)(THING_ENDOFLIST >> 8),
          "C14 move keeps decoded and raw source-list ownership aligned");
    CHECK(world.timeline.events[0].mapIndex == 0 &&
          world.timeline.events[0].mapX == 1 &&
          world.timeline.events[0].mapY == 0 &&
          world.timeline.events[0].cell == 2 &&
          world.timeline.events[1].mapX == 0,
          "F0249 relocates only the C14-owned C48/C49 event");

    init_world(&world, &dungeon, &things, &map, &tiles, squareData,
               firstThings, &projectile, &explosion, rawProjectile,
               rawExplosion);
    squareData[0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[1] |= DUNGEON_SQUARE_MASK_THING_LIST;
    firstThings[0] = explosionThing;
    firstThings[1] = projectileThing;
    projectile.next = THING_ENDOFLIST;
    rawProjectile[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawProjectile[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    explosion.next = THING_ENDOFLIST;
    rawExplosion[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawExplosion[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    world.timeline.count = 2;
    world.timeline.events[0].kind = TIMELINE_EVENT_EXPLOSION_ADVANCE;
    world.timeline.events[0].aux0 = 0;
    world.timeline.events[1].kind = TIMELINE_EVENT_REMOVE_FLUXCAGE;
    world.timeline.events[1].aux0 = 0;
    world.timeline.events[1].mapX = 0;
    world.timeline.events[1].mapY = 0;

    memset(&request, 0, sizeof(request));
    request.thing = explosionThing;
    request.sourceMapIndex = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &result),
          "F0267 moves the raw C15 owner");
    CHECK(result.moved && result.destinationLinked &&
          result.timelineRelocationCount == 1 &&
          F0511_DUNGEON_GetSquareFirstThing_Compat(
              &dungeon, &things, 0, 0, 0) == THING_ENDOFLIST &&
          explosion.next == THING_ENDOFLIST &&
          rawExplosion[0] == (unsigned char)(THING_ENDOFLIST & 0xffu) &&
          rawExplosion[1] == (unsigned char)(THING_ENDOFLIST >> 8) &&
          projectile.next == explosionThing &&
          rawProjectile[0] == (unsigned char)(explosionThing & 0xffu) &&
          rawProjectile[1] == (unsigned char)(explosionThing >> 8),
          "C15 append keeps decoded and raw square-tail ownership aligned");
    CHECK(world.timeline.events[0].mapX == 1 &&
          world.timeline.events[0].mapY == 0 &&
          world.timeline.events[0].cell == 1 &&
          world.timeline.events[1].mapX == 0 &&
          world.timeline.events[1].mapY == 0,
          "F0249 relocates C25 but preserves the original C24 fluxcage owner");

    puts("ok: F0249 runtime relocation follows raw C14/C15 movement without C24 fallback");
    return 0;
}
