/* ReDMCSB TIMELINE.C F0242/F0244/F0250/F0251 through F0261. */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_f0259_quiver_refill_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void schedule(struct GameWorld_Compat* world, int type, int effect,
                     int x, int y, int cell)
{
    struct TimelineEvent_Compat event;
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_SQUARE_STATE;
    event.fireAtTick = 0;
    event.mapIndex = 0;
    event.mapX = x;
    event.mapY = y;
    event.cell = cell;
    event.aux0 = type;
    event.aux1 = effect;
    assert(F0721_TIMELINE_Schedule_Compat(&world->timeline, &event));
}

static int has_square_state_event(const struct GameWorld_Compat* world,
                                  int type, int effect, int x, int y)
{
    int i;
    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];
        if (event->kind == TIMELINE_EVENT_SQUARE_STATE &&
            event->aux0 == type && event->aux1 == effect &&
            event->mapX == x && event->mapY == y) {
            return 1;
        }
    }
    return 0;
}

static int has_generator_reenable_event(const struct GameWorld_Compat* world,
                                        int x, int y)
{
    int i;
    for (i = 0; i < world->timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &world->timeline.events[i];
        if (event->kind == TIMELINE_EVENT_GROUP_GENERATOR &&
            event->aux0 == GENERATOR_EVENT_AUX0_REENABLE &&
            event->mapX == x && event->mapY == y) {
            return 1;
        }
    }
    return 0;
}

static int has_text_message_emission(const struct TickResult_Compat* result,
                                     int textIndex, int mapIndex,
                                     int mapX, int mapY)
{
    int i;
    if (!result) return 0;
    for (i = 0; i < result->emissionCount; ++i) {
        const struct TickEmission_Compat* emission = &result->emissions[i];
        if (emission->kind == EMIT_TEXT_MESSAGE &&
            emission->payload[0] == textIndex &&
            emission->payload[1] == mapIndex &&
            emission->payload[2] == mapX &&
            emission->payload[3] == mapY) {
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[4];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonTeleporter_Compat teleporter;
    struct DungeonTextString_Compat text;
    struct DungeonSensor_Compat sensors[3];
    struct DungeonGroup_Compat group;
    unsigned short squareFirstThings[4];
    struct GameWorld_Compat world;
    struct TickResult_Compat result;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(squares, 0, sizeof(squares));
    map.width = 2;
    map.height = 2;
    map.creatureTypeCount = 1;
    map.allowedCreatureTypes[0] = 0;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    tiles.squareData = squares;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    memset(&things, 0, sizeof(things));
    memset(&weapon, 0, sizeof(weapon));
    memset(&teleporter, 0, sizeof(teleporter));
    memset(&text, 0, sizeof(text));
    memset(sensors, 0, sizeof(sensors));
    memset(&group, 0, sizeof(group));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    things.loaded = 1;
    things.weapons = &weapon;
    things.weaponCount = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.teleporters = &teleporter;
    things.teleporterCount = 1;
    things.textStrings = &text;
    things.textStringCount = 1;
    things.sensors = sensors;
    things.sensorCount = 3;
    things.groups = &group;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    weapon.next = THING_ENDOFLIST;
    squareFirstThings[0] = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    squareFirstThings[1] = THING_ENDOFLIST;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 1));
    world.dungeon = &dungeon;
    world.things = &things;

    /* C10 door event becomes C01 animation, which performs one opening
     * step at the same Map_Time. */
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 4);
    schedule(&world, DM1_EVENT_DOOR, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 2);
    assert((squares[0] & 7) == 3);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_DOOR_ANIMATE);

    /* C09 and C08 share F0250/F0251's bit-3 SET/CLEAR/toggle behavior. */
    squares[2] = (unsigned char)(DUNGEON_ELEMENT_PIT << 5);
    schedule(&world, DM1_EVENT_PIT, DOOR_EFFECT_SET, 1, 0, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((squares[2] & 0x08) != 0);

    squares[1] = (unsigned char)(DUNGEON_ELEMENT_TELEPORTER << 5);
    schedule(&world, DM1_EVENT_TELEPORTER, DOOR_EFFECT_TOGGLE, 0, 1, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((squares[1] & 0x08) != 0);

    /* F0249: opening a teleporter replays its existing ordinary thing
     * chain through F0267. The weapon starts on (0,0) and is linked onto
     * the source-verified target square (0,1). */
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_TELEPORTER << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    squares[1] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    teleporter.next = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    teleporter.targetMapIndex = 0;
    teleporter.targetMapX = 0;
    teleporter.targetMapY = 1;
    teleporter.scope = 0x02;
    weapon.next = THING_ENDOFLIST;
    squareFirstThings[0] = (unsigned short)((THING_TYPE_TELEPORTER << 10) | 0);
    squareFirstThings[1] = THING_ENDOFLIST;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 1;
    world.party.mapY = 1;
    schedule(&world, DM1_EVENT_TELEPORTER, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(squareFirstThings[0] == (unsigned short)((THING_TYPE_TELEPORTER << 10) | 0));
    assert(teleporter.next == THING_ENDOFLIST);
    assert(squareFirstThings[1] == (unsigned short)((THING_TYPE_WEAPON << 10) | 0));

    /* F0249's C04 branch is before the ordinary snapshot.  A group on an
     * opening creature-scope teleporter therefore reaches the same F0267
     * destination path, while the source square no longer retains C04. */
    teleporter.next = (unsigned short)(THING_TYPE_GROUP << 10);
    teleporter.targetMapX = 0;
    teleporter.targetMapY = 1;
    teleporter.scope = 0x01;
    teleporter.rotation = 1;
    teleporter.absoluteRotation = 0;
    group.next = THING_ENDOFLIST;
    group.creatureType = 0;
    group.cells = 0;
    group.count = 0;
    group.direction = 0;
    group.health[0] = 100;
    squareFirstThings[0] = (unsigned short)(THING_TYPE_TELEPORTER << 10);
    squareFirstThings[1] = THING_ENDOFLIST;
    world.party.mapX = 1;
    world.party.mapY = 1;
    world.creatureAICount = 0;
    schedule(&world, DM1_EVENT_TELEPORTER, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(squareFirstThings[0] == (unsigned short)(THING_TYPE_TELEPORTER << 10));
    assert(teleporter.next == THING_ENDOFLIST);
    assert(squareFirstThings[1] == (unsigned short)(THING_TYPE_GROUP << 10));
    assert(group.next == THING_ENDOFLIST);
    /* MOVESENS.C F0262 is still called by F0249's C04 insertion path:
     * the creature's orientation and occupied cell rotate with the real
     * creature-scope teleporter, rather than merely changing its square. */
    assert(group.direction == 1);
    assert(group.cells == 1);

    /* F0249 sends party through F0267 before ordinary source-chain things. */
    teleporter.scope = 0x02;
    teleporter.rotation = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = 1;
    schedule(&world, DM1_EVENT_TELEPORTER, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(world.party.mapIndex == 0 && world.party.mapX == 0 &&
           world.party.mapY == 1);

    /* C07 SET exposes the fakewall by setting its open bit. */
    squares[3] = (unsigned char)(DUNGEON_ELEMENT_FAKEWALL << 5);
    schedule(&world, DM1_EVENT_FAKEWALL, DOOR_EFFECT_SET, 1, 1, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((squares[3] & 0x04) != 0);

    /* F0248: only TextStrings on the event wall cell change. C006 then
     * remembers a local TOGGLE and F0271 rotates the matching sensor run
     * once after the whole source-list batch. */
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    text.visible = 0;
    text.next = (unsigned short)((1u << 14) | (THING_TYPE_SENSOR << 10) | 0);
    sensors[0].next = (unsigned short)((1u << 14) | (THING_TYPE_SENSOR << 10) | 1);
    sensors[0].sensorType = DM1_SENSOR_WALL_COUNTDOWN;
    sensors[0].sensorData = 1;
    sensors[0].effect = DM1_EFFECT_SET;
    sensors[0].localEffect = 1;
    sensors[0].localMultiple = DM1_EFFECT_TOGGLE;
    sensors[1].next = (unsigned short)((2u << 14) | (THING_TYPE_SENSOR << 10) | 2);
    sensors[1].sensorType = DM1_SENSOR_WALL_COUNTDOWN;
    sensors[2].next = THING_ENDOFLIST;
    sensors[2].sensorType = DM1_SENSOR_WALL_AND_OR_GATE;
    sensors[2].sensorData = 0x20;
    sensors[2].effect = DM1_EFFECT_SET;
    sensors[2].targetMapX = 1;
    sensors[2].targetMapY = 1;
    squareFirstThings[0] = (unsigned short)((1u << 14) |
                                             (THING_TYPE_TEXTSTRING << 10));

    schedule(&world, DM1_EVENT_WALL, DM1_EFFECT_SET, 0, 0, 1);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(text.visible == 1);
    assert(sensors[0].sensorData == 2);
    assert(text.next == (unsigned short)((1u << 14) |
                                          (THING_TYPE_SENSOR << 10) | 0));
    assert(has_square_state_event(&world, DM1_EVENT_FAKEWALL,
                                  DM1_EFFECT_SET, 1, 1));

    sensors[0].sensorData = 1;
    schedule(&world, DM1_EVENT_WALL, DM1_EFFECT_CLEAR, 0, 0, 1);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(text.visible == 0);
    assert(sensors[0].sensorData == 0);
    assert(text.next == (unsigned short)((1u << 14) |
                                          (THING_TYPE_SENSOR << 10) | 1));
    assert(sensors[1].next == (unsigned short)((1u << 14) |
                                                 (THING_TYPE_SENSOR << 10) | 0));
    assert(sensors[0].next == (unsigned short)((2u << 14) |
                                                 (THING_TYPE_SENSOR << 10) | 2));

    /* F0245: corridor TextStrings have no wall-cell filter, and each C006
     * is materialized immediately in list order through F0185. */
    squares[1] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    /* F0245 prints a newly-visible corridor text immediately when the
     * party is standing on that square. Keep this first route text-only:
     * a generator on the party square has independent F0185 placement
     * rules and would obscure the message assertion. */
    text.visible = 0;
    text.next = THING_ENDOFLIST;
    squareFirstThings[1] = (unsigned short)(THING_TYPE_TEXTSTRING << 10);
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 1;
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));
    schedule(&world, DM1_EVENT_CORRIDOR, DM1_EFFECT_SET, 0, 1, 3);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(text.visible == 1);
    assert(has_text_message_emission(&result, 0, 0, 0, 1));

    /* C006 remains materialized in source list order, but it does not
     * print the TextString when the party is elsewhere. */
    text.visible = 0;
    text.next = (unsigned short)(THING_TYPE_SENSOR << 10);
    sensors[0].next = THING_ENDOFLIST;
    sensors[0].sensorType = DM1_SENSOR_FLOOR_GROUP_GENERATOR;
    sensors[0].sensorData = 0;
    sensors[0].value = 1;
    sensors[0].onceOnly = 0;
    sensors[0].audible = 1;
    sensors[0].localMultiple = (unsigned short)((2u << 4) | 1u);
    group.next = THING_NONE;
    squareFirstThings[1] = (unsigned short)(THING_TYPE_TEXTSTRING << 10);
    world.party.mapX = 1;
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));

    schedule(&world, DM1_EVENT_CORRIDOR, DM1_EFFECT_SET, 0, 1, 3);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(text.visible == 1);
    assert(!has_text_message_emission(&result, 0, 0, 0, 1));
    assert(sensors[0].sensorType == RUNTIME_SENSOR_TYPE_DISABLED);
    assert(squareFirstThings[1] == (unsigned short)(THING_TYPE_GROUP << 10));
    assert(group.next == (unsigned short)(THING_TYPE_TEXTSTRING << 10));
    assert(group.creatureType == 0 && group.health[0] > 0);

    /* A repeated SET remains silent. */
    schedule(&world, DM1_EVENT_CORRIDOR, DM1_EFFECT_SET, 0, 1, 3);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(!has_text_message_emission(&result, 0, 0, 0, 1));
    assert(has_generator_reenable_event(&world, 0, 1));

    /* F0259: C11 with MENU.C's C01 ordinal uses C12, C07, C08, C09 source
     * order and refuses to overwrite a nonempty action hand. */
    world.party.champions[0].present = 1;
    world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] = THING_NONE;
    world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_1] =
        (unsigned short)(THING_TYPE_JUNK << 10);
    world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_3] =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_2] =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));
    {
        struct TimelineEvent_Compat refill;
        memset(&refill, 0, sizeof(refill));
        refill.kind = TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
        refill.fireAtTick = world.gameTick;
        refill.aux0 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
        refill.aux1 = 2; /* M000_INDEX_TO_ORDINAL(C01_SLOT_ACTION_HAND) */
        refill.aux2 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
        refill.aux4 = 0;
        assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &refill));
    }
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] ==
           (unsigned short)((THING_TYPE_WEAPON << 10) | 1));
    assert(world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_3] == THING_NONE);
    assert(world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_2] ==
           (unsigned short)((THING_TYPE_WEAPON << 10) | 2));
    return 0;
}
