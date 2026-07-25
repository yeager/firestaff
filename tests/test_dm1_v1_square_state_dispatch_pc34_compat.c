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
    (void)world;
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

static __attribute__((unused)) int has_square_state_event(const struct GameWorld_Compat* world,
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

static __attribute__((unused)) int has_generator_reenable_event(const struct GameWorld_Compat* world,
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

static __attribute__((unused)) int has_text_message_emission(const struct TickResult_Compat* result,
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

static __attribute__((unused)) int has_action_enabled_emission(const struct TickResult_Compat* result,
                                       int championIndex, int slotOrdinal)
{
    int i;
    if (!result) return 0;
    for (i = 0; i < result->emissionCount; ++i) {
        const struct TickEmission_Compat* emission = &result->emissions[i];
        if (emission->kind == EMIT_ACTION_ENABLED &&
            emission->payload[0] == championIndex &&
            emission->payload[1] == slotOrdinal) {
            return 1;
        }
    }
    return 0;
}

static void write_u16(unsigned char* bytes, unsigned short value)
{
    bytes[0] = (unsigned char)(value & 0xffu);
    bytes[1] = (unsigned char)(value >> 8);
}

static void sync_text_raw(unsigned char raw[4],
                          const struct DungeonTextString_Compat* text)
{
    unsigned short bits = (unsigned short)((text->visible ? 1u : 0u) |
        ((text->textDataWordOffset & 0x1fffu) << 3));
    write_u16(raw, text->next);
    write_u16(raw + 2, bits);
}

static void sync_sensor_raw(unsigned char raw[8],
                            const struct DungeonSensor_Compat* sensor)
{
    unsigned short typeData = (unsigned short)((sensor->sensorType & 0x7fu) |
        ((sensor->sensorData & 0x01ffu) << 7));
    unsigned short common = (unsigned short)(((sensor->onceOnly & 1u) << 2) |
        ((sensor->effect & 3u) << 3) | ((sensor->revertEffect & 1u) << 5) |
        ((sensor->audible & 1u) << 6) | ((sensor->value & 0x0fu) << 7) |
        ((sensor->localEffect & 1u) << 11) |
        ((sensor->ornamentOrdinal & 0x0fu) << 12));
    write_u16(raw, sensor->next);
    write_u16(raw + 2, typeData);
    write_u16(raw + 4, common);
    write_u16(raw + 6, sensor->localMultiple & 0x0fffu);
}

static void sync_teleporter_raw(unsigned char raw[6],
                                 const struct DungeonTeleporter_Compat* tp)
{
    unsigned short fields = (unsigned short)((tp->targetMapX & 0x1fu) |
        ((tp->targetMapY & 0x1fu) << 5) |
        ((tp->rotation & 0x03u) << 10) |
        ((tp->absoluteRotation & 0x01u) << 12) |
        ((tp->scope & 0x03u) << 13) |
        ((tp->audible & 0x01u) << 15));
    write_u16(raw, tp->next);
    write_u16(raw + 2, fields);
    write_u16(raw + 4, (unsigned short)(tp->targetMapIndex << 8));
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[4];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[3];
    struct DungeonDoor_Compat doors[1];
    struct DungeonTeleporter_Compat teleporter;
    struct DungeonTextString_Compat text;
    struct DungeonSensor_Compat sensors[3];
    struct DungeonGroup_Compat group;
    unsigned char rawGroupData[16];
    unsigned char rawWeaponData[12];
    unsigned char rawDoorData[4];
    unsigned char rawTeleporterData[6];
    unsigned char rawTextData[4];
    unsigned char rawSensorData[24];
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
    dungeon.loaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    tiles.squareData = squares;

    memset(&world, 0, sizeof(world));
    world.dungeon = &dungeon;
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(doors, 0, sizeof(doors));
    memset(&teleporter, 0, sizeof(teleporter));
    memset(&text, 0, sizeof(text));
    memset(sensors, 0, sizeof(sensors));
    memset(&group, 0, sizeof(group));
    memset(rawGroupData, 0, sizeof(rawGroupData));
    memset(rawWeaponData, 0, sizeof(rawWeaponData));
    memset(rawDoorData, 0, sizeof(rawDoorData));
    memset(rawTeleporterData, 0, sizeof(rawTeleporterData));
    memset(rawTextData, 0, sizeof(rawTextData));
    memset(rawSensorData, 0, sizeof(rawSensorData));
    rawGroupData[0] = 0xffu;
    rawGroupData[1] = 0xffu;
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 3;
    things.thingCounts[THING_TYPE_WEAPON] = 3;
    things.teleporters = &teleporter;
    things.teleporterCount = 1;
    things.thingCounts[THING_TYPE_TELEPORTER] = 1;
    things.rawThingData[THING_TYPE_TELEPORTER] = rawTeleporterData;
    things.textStrings = &text;
    things.textStringCount = 1;
    things.thingCounts[THING_TYPE_TEXTSTRING] = 1;
    things.rawThingData[THING_TYPE_TEXTSTRING] = rawTextData;
    things.sensors = sensors;
    things.sensorCount = 3;
    things.thingCounts[THING_TYPE_SENSOR] = 3;
    things.rawThingData[THING_TYPE_SENSOR] = rawSensorData;
    things.groups = &group;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = rawGroupData;
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 3;
    things.thingCounts[THING_TYPE_WEAPON] = 3;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeaponData;
    things.doors = doors;
    things.doorCount = 1;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    things.rawThingData[THING_TYPE_DOOR] = rawDoorData;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    weapons[0].next = THING_ENDOFLIST;
    squareFirstThings[0] = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    squareFirstThings[1] = THING_ENDOFLIST;
    world.things = &things;
    assert(F0881_WORLD_InitDefault_Compat(&world, 1));

    /* C10 door event becomes C01 animation, which performs one opening
     * step at the same Map_Time. */
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST | 4);
    doors[0].next = THING_ENDOFLIST;
    rawDoorData[0] = 0xfe;
    rawDoorData[1] = 0xff;
    squareFirstThings[0] = (unsigned short)(THING_TYPE_DOOR << 10);
    schedule(&world, DM1_EVENT_DOOR, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 2);
    assert((squares[0] & 7) == 3);
    assert(world.timeline.count == 1);
    assert(world.timeline.events[0].kind == TIMELINE_EVENT_DOOR_ANIMATE);
    /* C10 is not allowed to animate a decoded-only door. */
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST | 4);
    things.rawThingData[THING_TYPE_DOOR] = NULL;
    schedule(&world, DM1_EVENT_DOOR, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
    assert((squares[0] & 7) == 4);
    things.rawThingData[THING_TYPE_DOOR] = rawDoorData;

    /* C09 and C08 share F0250/F0251's bit-3 SET/CLEAR/toggle behavior. */
    squares[2] = (unsigned char)(DUNGEON_ELEMENT_PIT << 5);
    schedule(&world, DM1_EVENT_PIT, DOOR_EFFECT_SET, 1, 0, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((squares[2] & 0x08) != 0);

    squares[1] = (unsigned char)(DUNGEON_ELEMENT_TELEPORTER << 5);
    teleporter.next = THING_ENDOFLIST;
    sync_teleporter_raw(rawTeleporterData, &teleporter);
    squareFirstThings[1] = (unsigned short)(THING_TYPE_TELEPORTER << 10);
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
    sync_teleporter_raw(rawTeleporterData, &teleporter);
    weapons[0].next = THING_ENDOFLIST;
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

    /* C08 must not run from a decoded-only/stale C01 cache, and C09 must
     * never flip the open bit on a teleporter square. */
    squares[0] = (unsigned char)(DUNGEON_ELEMENT_TELEPORTER << 5);
    teleporter.next = THING_ENDOFLIST;
    teleporter.targetMapIndex = 0;
    teleporter.targetMapX = 0;
    teleporter.targetMapY = 1;
    teleporter.scope = 0x02;
    sync_teleporter_raw(rawTeleporterData, &teleporter);
    rawTeleporterData[2] ^= 0x01u;
    squareFirstThings[0] = (unsigned short)(THING_TYPE_TELEPORTER << 10);
    schedule(&world, DM1_EVENT_TELEPORTER, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((squares[0] & 0x08) == 0);
    sync_teleporter_raw(rawTeleporterData, &teleporter);
    schedule(&world, DM1_EVENT_PIT, DOOR_EFFECT_SET, 0, 0, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert((squares[0] & 0x08) == 0);

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
    sync_text_raw(rawTextData, &text);
    squareFirstThings[1] = (unsigned short)(THING_TYPE_TEXTSTRING << 10);
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 1;
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));
    /* F0245 must not publish decoded-only text when its C02 owner drifts. */
    rawTextData[2] ^= 1u;
    schedule(&world, DM1_EVENT_CORRIDOR, DM1_EFFECT_SET, 0, 1, 0);
    memset(&result, 0, sizeof(result));
    assert(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) == 1);
    assert(text.visible == 0 && !has_text_message_emission(&result, 0, 0, 0, 1));
    sync_text_raw(rawTextData, &text);
    schedule(&world, DM1_EVENT_CORRIDOR, DM1_EFFECT_SET, 0, 1, 0);
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
    sync_text_raw(rawTextData, &text);
    sync_sensor_raw(rawSensorData, &sensors[0]);
    group.next = THING_NONE;
    squareFirstThings[1] = (unsigned short)(THING_TYPE_TEXTSTRING << 10);
    world.party.mapX = 1;
    assert(F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick));

    schedule(&world, DM1_EVENT_CORRIDOR, DM1_EFFECT_SET, 0, 1, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(text.visible == 1);
    assert(!has_text_message_emission(&result, 0, 0, 0, 1));
    assert(sensors[0].sensorType == RUNTIME_SENSOR_TYPE_DISABLED);
    assert((rawSensorData[2] & 0x7fu) == RUNTIME_SENSOR_TYPE_DISABLED);
    assert(squareFirstThings[1] == (unsigned short)(THING_TYPE_GROUP << 10));
    assert(group.next == (unsigned short)(THING_TYPE_TEXTSTRING << 10));
    assert(group.creatureType == 0 && group.health[0] > 0);

    /* A repeated SET remains silent. */
    schedule(&world, DM1_EVENT_CORRIDOR, DM1_EFFECT_SET, 0, 1, 0);
    memset(&result, 0, sizeof(result));
    (void)F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result);
    assert(!has_text_message_emission(&result, 0, 0, 0, 1));
    assert(has_generator_reenable_event(&world, 0, 1));

    /* F0259: C11 with MENU.C's C01 ordinal uses C12, C07, C08, C09 source
     * order and refuses to overwrite a nonempty action hand. */
    {
        struct DM1F0259QuiverRefillPlanPc34 plan;
        (void)plan;

        world.party.champions[0].present = 1;
        world.party.championCount = 1;
        world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] =
            THING_NONE;
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
        assert(has_action_enabled_emission(&result, 0, 2));
        assert(world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] ==
               (unsigned short)((THING_TYPE_WEAPON << 10) | 1));
        assert(world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_3] ==
               THING_NONE);
        world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] = THING_NONE;
        world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_3] =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
        assert(DM1_V1_F0259_PlanQuiverRefillPc34Compat(
            &world.party.champions[0], 0, CHAMPION_SLOT_HAND_RIGHT, &plan));
        assert(plan.valid && plan.moved);
        assert(plan.sourceSlot == CHAMPION_SLOT_QUIVER_3);
        assert(plan.thing == (unsigned short)((THING_TYPE_WEAPON << 10) | 1));
        world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
        assert(DM1_V1_F0259_PlanQuiverRefillPc34Compat(
            &world.party.champions[0], 0, CHAMPION_SLOT_HAND_RIGHT, &plan));
        assert(plan.valid && !plan.moved && plan.sourceSlot == -1);

        world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] =
            THING_NONE;
        world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_1] =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
        things.rawThingData[THING_TYPE_WEAPON] = NULL;
        assert(!DM1_V1_F0259_ApplyQuiverRefillFromDungeonPc34Compat(
            &world.party.champions[0], 0, CHAMPION_SLOT_HAND_RIGHT,
            &things, &plan));
        assert(world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] ==
               THING_NONE);
        assert(world.party.champions[0].inventory[CHAMPION_SLOT_QUIVER_1] ==
               (unsigned short)((THING_TYPE_WEAPON << 10) | 0));
        things.rawThingData[THING_TYPE_WEAPON] = rawWeaponData;
    }
    return 0;
}
