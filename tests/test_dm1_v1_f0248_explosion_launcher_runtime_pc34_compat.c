/* ReDMCSB TIMELINE.C F0247/F0248 C008/C010 live M10 regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void schedule_wall_event(struct GameWorld_Compat* world, int cell)
{
    struct TimelineEvent_Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world->gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = cell;
    event.aux0 = DM1_EVENT_WALL;
    event.aux1 = DM1_EFFECT_SET;
    assert(F0721_TIMELINE_Schedule_Compat(&world->timeline, &event));
}

static int pending_projectile_moves(const struct GameWorld_Compat* world)
{
    int i;
    int count = 0;

    for (i = 0; i < world->timeline.count; ++i) {
        if (world->timeline.events[i].kind == TIMELINE_EVENT_PROJECTILE_MOVE) {
            ++count;
        }
    }
    return count;
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[2];
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensor;
    unsigned short squareFirstThings[2];
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
    tiles.squareData = squares;
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);

    memset(&things, 0, sizeof(things));
    memset(&sensor, 0, sizeof(sensor));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    sensorThing = (unsigned short)((1u << 14) | (THING_TYPE_SENSOR << 10));
    sensor.next = THING_ENDOFLIST;
    things.loaded = 1;
    things.sensors = &sensor;
    things.sensorCount = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    squareFirstThings[0] = sensorThing;
    squareFirstThings[1] = THING_ENDOFLIST;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 0x2468u));
    world.dungeon = &dungeon;
    world.things = &things;

    /* C010: two real explosion Things become two magical projectiles in
     * the square ahead, retaining C010's once-only mutation and C25 moves. */
    sensor.sensorType = DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION;
    sensor.sensorData = C002_EXPLOSION_LIGHTNING_BOLT;
    sensor.onceOnly = 1;
    sensor.localMultiple = (unsigned short)((9u << 8) | 37u);
    schedule_wall_event(&world, 1);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(sensor.sensorType == DM1_SENSOR_DISABLED);
    assert(world.projectiles.count == 2);
    assert(world.projectiles.entries[0].projectileCategory == PROJECTILE_CATEGORY_MAGICAL);
    assert(world.projectiles.entries[0].projectileSubtype == PROJECTILE_SUBTYPE_LIGHTNING_BOLT);
    assert(world.projectiles.entries[0].mapX == 1 && world.projectiles.entries[0].mapY == 0);
    assert(world.projectiles.entries[0].cell == 3 && world.projectiles.entries[1].cell == 0);
    assert(world.projectiles.entries[0].kineticEnergy == 37);
    assert(world.projectiles.entries[0].stepEnergy == 9);
    assert(pending_projectile_moves(&world) == 2);

    /* C008 consumes exactly one RNG bit and creates one projectile rather
     * than relying on a synthetic visual or dormant launcher receipt. */
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));
    sensor.sensorType = DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_EXPLOSION;
    sensor.sensorData = C000_EXPLOSION_FIREBALL;
    sensor.onceOnly = 0;
    sensor.localMultiple = (unsigned short)((4u << 8) | 55u);
    schedule_wall_event(&world, 1);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(world.projectiles.count == 3);
    assert(world.projectiles.entries[2].projectileSubtype == PROJECTILE_SUBTYPE_FIREBALL);
    assert(world.projectiles.entries[2].mapX == 1 && world.projectiles.entries[2].mapY == 0);
    assert(world.projectiles.entries[2].cell == 3 || world.projectiles.entries[2].cell == 0);
    assert(pending_projectile_moves(&world) == 1);

    /* A C008 on another wall cell does not reach F0247 and must leave the
     * master RNG untouched. */
    {
        uint32_t seedBefore = world.masterRng.seed;
        assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));
        schedule_wall_event(&world, 2);
        memset(&result, 0, sizeof(result));
        (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
        assert(world.projectiles.count == 3);
        assert(world.masterRng.seed == seedBefore);
    }
    return 0;
}
