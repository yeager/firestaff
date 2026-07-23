/* ReDMCSB DUNGEON.C F0167 + TIMELINE.C F0247 C007/C009 regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void schedule_wall_event(struct GameWorld_Compat* world)
{
    struct TimelineEvent_Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world->gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1;
    event.aux0 = DM1_EVENT_WALL;
    event.aux1 = DM1_EFFECT_SET;
    assert(F0721_TIMELINE_Schedule_Compat(&world->timeline, &event));
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[2];
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensor;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonJunk_Compat junk;
    struct DungeonProjectile_Compat projectiles[3];
    unsigned char projectileRaw[3 * 8];
    unsigned short squareFirstThings[3];
    unsigned short columns[2] = { 0, 1 };
    struct GameWorld_Compat world;
    struct TickResult_Compat result;
    unsigned short sensorThing;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(squares, 0, sizeof(squares));
    map.width = 2;
    map.height = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    dungeon.dungeonColumnCount = 2;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    tiles.squareData = squares;
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);

    sensorThing = (unsigned short)((1u << 14) | (THING_TYPE_SENSOR << 10));
    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    memset(weapons, 0, sizeof(weapons));
    memset(&junk, 0, sizeof(junk));
    memset(projectiles, 0, sizeof(projectiles));
    memset(projectileRaw, 0xff, sizeof(projectileRaw));
    for (int i = 0; i < 3; ++i) squareFirstThings[i] = THING_NONE;
    weapons[0].next = THING_NONE;
    weapons[1].next = THING_NONE;
    junk.next = THING_NONE;
    sensor.next = THING_ENDOFLIST;
    things.loaded = 1;
    things.sensors = &sensor;
    things.sensorCount = 1;
    things.weapons = weapons;
    things.weaponCount = 2;
    things.junks = &junk;
    things.junkCount = 1;
    things.projectiles = projectiles;
    things.projectileCount = 3;
    things.thingCounts[THING_TYPE_PROJECTILE] = 3;
    things.rawThingData[THING_TYPE_PROJECTILE] = projectileRaw;
    for (int i = 0; i < 3; ++i) {
        projectiles[i].next = THING_NONE;
        projectiles[i].slot = THING_NONE;
        projectiles[i].eventIndex = THING_NONE;
    }
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 3;
    squareFirstThings[0] = sensorThing;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 0x5678u));
    world.dungeon = &dungeon;
    world.things = &things;

    /* C009 calls F0167 twice for arrows, materializing two unused source
     * records before F0212 gives them adjacent projectile cells. */
    sensor.sensorType = DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_NEW_OBJ;
    sensor.sensorData = 51;
    sensor.onceOnly = 1;
    sensor.localMultiple = (unsigned short)((6u << 8) | 29u);
    schedule_wall_event(&world);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(sensor.sensorType == DM1_SENSOR_DISABLED);
    assert(weapons[0].next == THING_ENDOFLIST && weapons[0].type == 27);
    assert(weapons[1].next == THING_ENDOFLIST && weapons[1].type == 27);
    assert(world.projectiles.count == 2);
    assert(world.projectiles.entries[0].reserved1 == (THING_TYPE_WEAPON << 10));
    assert(world.projectiles.entries[1].reserved1 == ((THING_TYPE_WEAPON << 10) | 1));
    assert(world.projectiles.entries[0].cell == 3 && world.projectiles.entries[1].cell == 0);

    /* C007 follows the same F0167 path for its JUNK boulder record. */
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));
    sensor.sensorType = DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ;
    sensor.sensorData = 128;
    sensor.onceOnly = 0;
    schedule_wall_event(&world);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(junk.next == THING_ENDOFLIST && junk.type == 25);
    assert(world.projectiles.count == 3);
    assert(world.projectiles.entries[2].reserved1 == (THING_TYPE_JUNK << 10));
    assert(world.projectiles.entries[2].cell == 3 || world.projectiles.entries[2].cell == 0);
    return 0;
}
