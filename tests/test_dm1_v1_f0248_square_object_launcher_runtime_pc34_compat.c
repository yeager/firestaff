/* ReDMCSB TIMELINE.C F0247 C014/C015 live-object launcher regression. */
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
    unsigned short squareFirstThings[2];
    struct GameWorld_Compat world;
    struct TickResult_Compat result;
    unsigned short sensorThing;
    unsigned short weapon0;
    unsigned short weapon1;

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
    tiles.squareData = squares;
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);

    sensorThing = (unsigned short)((1u << 14) | (THING_TYPE_SENSOR << 10));
    weapon0 = (unsigned short)((1u << 14) | (THING_TYPE_WEAPON << 10) | 0);
    weapon1 = (unsigned short)((2u << 14) | (THING_TYPE_WEAPON << 10) | 1);
    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    memset(weapons, 0, sizeof(weapons));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    sensor.next = weapon0;
    weapons[0].next = weapon1;
    weapons[1].next = THING_ENDOFLIST;
    things.loaded = 1;
    things.sensors = &sensor;
    things.sensorCount = 1;
    things.weapons = weapons;
    things.weaponCount = 2;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    squareFirstThings[0] = sensorThing;
    squareFirstThings[1] = THING_ENDOFLIST;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 0x4567u));
    world.dungeon = &dungeon;
    world.things = &things;

    /* C015 removes two real square Things before it creates two kinetic
     * projectiles. The sensor chain remains, with no duplicate floor art. */
    sensor.sensorType = DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_SQUARE_OBJ;
    sensor.onceOnly = 1;
    sensor.localMultiple = (unsigned short)((7u << 8) | 31u);
    schedule_wall_event(&world);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(sensor.sensorType == DM1_SENSOR_DISABLED);
    assert(sensor.next == THING_ENDOFLIST);
    assert(world.projectiles.count == 2);
    assert(world.projectiles.entries[0].projectileCategory == PROJECTILE_CATEGORY_KINETIC);
    assert(world.projectiles.entries[0].reserved1 == weapon0);
    assert(world.projectiles.entries[1].reserved1 == weapon1);
    assert(world.projectiles.entries[0].cell == 3 && world.projectiles.entries[1].cell == 0);
    assert(world.timeline.count == 2);

    /* C014 uses one newly relinked real object and retains the source's
     * random single-cell placement path. */
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));
    sensor.sensorType = DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ;
    sensor.onceOnly = 0;
    sensor.next = weapon0;
    weapons[0].next = THING_ENDOFLIST;
    schedule_wall_event(&world);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(sensor.next == THING_ENDOFLIST);
    assert(world.projectiles.count == 3);
    assert(world.projectiles.entries[2].reserved1 == weapon0);
    assert(world.projectiles.entries[2].cell == 3 || world.projectiles.entries[2].cell == 0);
    return 0;
}
