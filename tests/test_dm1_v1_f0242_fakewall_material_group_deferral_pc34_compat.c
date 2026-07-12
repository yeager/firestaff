/* ReDMCSB TIMELINE.C F0242 material-group fakewall retry regression. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void schedule_fakewall_clear(struct GameWorld_Compat* world)
{
    struct TimelineEvent_Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = world->gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.aux0 = DM1_EVENT_FAKEWALL;
    event.aux1 = DM1_EFFECT_CLEAR;
    assert(F0721_TIMELINE_Schedule_Compat(&world->timeline, &event));
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char square;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat group;
    unsigned short firstThing;
    struct GameWorld_Compat world;
    struct TickResult_Compat result;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    square = (unsigned char)((DUNGEON_ELEMENT_FAKEWALL << 5) | 0x04u |
                             DUNGEON_SQUARE_MASK_THING_LIST);
    map.width = 1;
    map.height = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    tiles.squareData = &square;

    memset(&things, 0, sizeof(things));
    memset(&group, 0, sizeof(group));
    firstThing = (unsigned short)(THING_TYPE_GROUP << 10);
    group.next = THING_ENDOFLIST;
    group.creatureType = 0; /* Material PC34 creature-table entry. */
    things.loaded = 1;
    things.groups = &group;
    things.groupCount = 1;
    things.squareFirstThings = &firstThing;
    things.squareFirstThingCount = 1;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 0x89ABu));
    world.dungeon = &dungeon;
    world.things = &things;
    world.party.mapIndex = 0;
    world.party.mapX = 1; /* Party is not on the fakewall square. */
    world.party.mapY = 0;

    schedule_fakewall_clear(&world);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((square & 0x04u) != 0);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_SQUARE_STATE);
    assert(world.timeline.events[0].aux0 == DM1_EVENT_FAKEWALL);
    assert(world.timeline.events[0].fireAtTick == 1u);

    /* Once the material group leaves, the delayed C07 applies without
     * inventing an active-group relocation path. */
    firstThing = THING_ENDOFLIST;
    world.gameTick = 1;
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((square & 0x04u) == 0);
    assert(world.timeline.count == 0);
    return 0;
}
