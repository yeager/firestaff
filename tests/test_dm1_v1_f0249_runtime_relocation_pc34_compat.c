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
