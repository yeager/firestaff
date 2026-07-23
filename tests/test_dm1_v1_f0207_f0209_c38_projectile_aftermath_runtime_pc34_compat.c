#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

static int expect(int condition, const char* label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

/* A dispatched C49 no longer exists in the compact queue when F0217 reaches
 * its impact tail. Its original C14 owner must still retire atomically with
 * the M10 projectile slot on a source-shaped wall impact. */
static int test_m10_c38_c14_c49_impact_retirement(int doorImpact)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat sourceProjectile;
    struct TimelineEvent_Compat event;
    unsigned char rawProjectile[8];
    unsigned char squareData[3];
    unsigned short squareFirstThings[3];
    unsigned short projectileThing;
    int i;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(&event, 0, sizeof(event));
    memset(rawProjectile, 0xff, sizeof(rawProjectile));
    squareData[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[1] = doorImpact
        ? (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 4u)
        : (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    squareData[2] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    squareFirstThings[0] = THING_ENDOFLIST;
    squareFirstThings[1] = THING_ENDOFLIST;
    squareFirstThings[2] = THING_ENDOFLIST;
    if (!F0881_WORLD_InitDefault_Compat(&world, 0xC38021u)) return 0;

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    map.width = 3;
    map.height = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 3;
    projectileThing = (unsigned short)((THING_TYPE_PROJECTILE << 10) |
                                       (1u << 14));
    sourceProjectile.next = THING_ENDOFLIST;
    sourceProjectile.slot = THING_NONE;
    sourceProjectile.kineticEnergy = 20;
    sourceProjectile.attack = 30;
    sourceProjectile.eventIndex = 9;
    rawProjectile[0] = 0xfe;
    rawProjectile[1] = 0xff;
    rawProjectile[2] = 0xff;
    rawProjectile[3] = 0xff;
    rawProjectile[4] = 20;
    rawProjectile[5] = 30;
    rawProjectile[6] = 9;
    rawProjectile[7] = 0;
    squareFirstThings[0] = projectileThing;
    things.loaded = 1;
    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 3;

    world.dungeon = &dungeon;
    world.things = &things;
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
    world.projectiles.entries[0].kineticEnergy = 20;
    world.projectiles.entries[0].attack = 30;
    world.projectiles.entries[0].stepEnergy = 4;
    world.projectiles.entries[0].projectileSubtype =
        doorImpact ? PROJECTILE_SUBTYPE_KINETIC_ARROW : 0;
    world.projectiles.entries[0].reserved1 = THING_NONE;
    /* Place the C48 at the exact C14 EventIndex. F0887 compacts the first
     * nine due records, then dispatches this source-owned C49 at index zero. */
    for (i = 0; i < 9; ++i) {
        event.kind = TIMELINE_EVENT_MOVE_TIMER;
        event.fireAtTick = world.gameTick;
        if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event)) return 0;
    }
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1;
    event.aux0 = 0;
    if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) ||
        !F0890g_ORCH_ValidateF0218ImpactOwner_Compat(&world, 0) ||
        F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) != ORCH_OK) {
        return 0;
    }
    return expect(world.projectiles.entries[0].reserved3 == 0,
                  "C38/F0217 retires live M10 projectile on impact") &&
           expect(sourceProjectile.next == THING_NONE &&
                  sourceProjectile.eventIndex == 0xFFFFu,
                  "C38/F0217 retires decoded C14 next and C49 owner") &&
           expect(rawProjectile[0] == 0xff && rawProjectile[1] == 0xff &&
                  rawProjectile[6] == 0xff && rawProjectile[7] == 0xff,
                  "C38/F0217 writes retired C14/C49 bytes") &&
           expect(doorImpact
                      ? world.timeline.count == 1 &&
                        world.timeline.events[0].kind ==
                            TIMELINE_EVENT_DOOR_DESTRUCTION &&
                        world.timeline.events[0].fireAtTick == 101u
                      : world.timeline.count == 0,
                  "C38/F0217 retains only the source door follow-up");
}

int main(void)
{
    if (!test_m10_c38_c14_c49_impact_retirement(0) ||
        !test_m10_c38_c14_c49_impact_retirement(1)) return 1;
    return 0;
}
