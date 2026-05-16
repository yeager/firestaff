/*
 * Pass 32 bounded probe — sensor addition/removal processing for party
 * movement.
 *
 * Verifies the new compat owners introduced in Pass 32:
 *   - F0717_SENSOR_EnumerateOnSquare_Compat walks the thing linked list
 *     and returns ALL sensors on a square (not just the first).
 *   - F0718_SENSOR_ProcessPartyEnterLeave_Compat runs F0710 over every
 *     enumerated sensor for a given event and concatenates effects
 *     into a single output list, preserving source order.
 *
 * The probe synthesises a tiny in-memory dungeon with a square that
 * carries two chained sensors (teleport + text) and verifies that
 * both are enumerated, executed in source order for WALK_ON, and that
 * WALK_OFF produces no effects in v1 (matching the conservative policy).
 *
 * Honest scope note:  this pass lands the compat-side sensor owner
 * surface as described in PASSLIST_29_36.md \u00a74.32.2.  The tick
 * orchestrator does not yet emit enter/leave events through this owner
 * for every party move; that full runtime integration is an explicit
 * remaining gap tracked in the pass report.  This probe exercises the
 * compat API directly as required by the pass verification gates.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_sensor_execution_pc34_compat.h"

#define MAP_W 2
#define MAP_H 2

static int g_pass = 0;
static int g_fail = 0;

static unsigned char sqb(int elementType, int attribs) {
    return (unsigned char)(((elementType & 7) << 5) | (attribs & 0x1F));
}

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

/*
 * Fixture:
 *   1x1-style minimal map (2x2) where square (0,0) has thing-list bit
 *   set and points to a chain: sensor[0] (teleport) -> sensor[1] (text)
 *   -> END.  Other squares have no thing-list bit.
 */
int main(void) {
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    unsigned char squareData[MAP_W * MAP_H];
    unsigned short squareFirstThings[MAP_W * MAP_H];
    struct DungeonSensor_Compat sensors[2];
    struct SensorOnSquare_Compat enumerated[SENSOR_ENUM_CAPACITY];
    struct SensorEffectList_Compat list;
    int count;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(sensors, 0, sizeof(sensors));

    dungeon.header.mapCount = 1;
    dungeon.maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(struct DungeonMapDesc_Compat));
    dungeon.tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(struct DungeonMapTiles_Compat));
    dungeon.maps[0].width = MAP_W;
    dungeon.maps[0].height = MAP_H;
    dungeon.tiles[0].squareData = squareData;
    dungeon.tilesLoaded = 1;
    dungeon.loaded = 1;

    /* Square (0,0) has thing-list bit set. */
    squareData[0 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR,
                                     DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[0 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    squareData[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR,
                                     DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[1 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);

    /* Thing list: sensor 0 (teleport type=0) -> sensor 1 (text type=13) -> END.
     * THING encoding: (type << 14) | (index & 0x3FFF). */
    sensors[0].sensorType = 0;   /* teleport */
    sensors[0].sensorData = 0;
    sensors[0].localEffect = 0;
    sensors[0].targetMapX = 3;
    sensors[0].targetMapY = 4;
    sensors[0].targetCell = 1;
    sensors[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 1);
    sensors[1].sensorType = 13;  /* text */
    sensors[1].sensorData = 42;
    sensors[1].localEffect = 1;
    sensors[1].next = THING_ENDOFLIST;

    things.loaded = 1;
    things.sensors = sensors;
    things.sensorCount = 2;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = MAP_W * MAP_H;
    squareFirstThings[0 * MAP_H + 0] = (unsigned short)((THING_TYPE_SENSOR << 10) | 0);
    /* Second thing-list square has an empty chain (ENDOFLIST). */
    squareFirstThings[1 * MAP_H + 0] = THING_ENDOFLIST;

    /* ---- F0717 enumeration ---- */
    count = F0717_SENSOR_EnumerateOnSquare_Compat(&dungeon, &things, 0, 0, 0, enumerated);
    record("P32_F0717_COUNT",
           count == 2,
           "F0717 reports 2 sensors on the chained square");
    record("P32_F0717_ORDER",
           count == 2 && enumerated[0].sensorType == 0 &&
               enumerated[1].sensorType == 13,
           "F0717 preserves source order: teleport first, text second");
    record("P32_F0717_TOTALS",
           count == 2 && enumerated[0].totalSensorsOnSquare == 2 &&
               enumerated[1].totalSensorsOnSquare == 2,
           "F0717 populates totalSensorsOnSquare on every entry");
    record("P32_F0717_EMPTY_SQUARE",
           F0717_SENSOR_EnumerateOnSquare_Compat(&dungeon, &things, 0, 1, 0, enumerated) == 0,
           "F0717 returns 0 on a thing-list square with empty chain");
    record("P32_F0717_NO_THING_LIST",
           F0717_SENSOR_EnumerateOnSquare_Compat(&dungeon, &things, 0, 0, 1, enumerated) == 0,
           "F0717 returns 0 on a square without the thing-list bit");

    /* ---- F0718 party enter/leave ---- */

    /* WALK_ON: should produce 2 effects (teleport + text) in that order. */
    memset(&list, 0, sizeof(list));
    F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, 0, 0, 0, SENSOR_EVENT_WALK_ON, &list);
    record("P32_F0718_WALK_ON_COUNT",
           list.count == 2,
           "F0718 WALK_ON on two-sensor square produces 2 effects");
    record("P32_F0718_WALK_ON_ORDER",
           list.count == 2 &&
               list.effects[0].kind == SENSOR_EFFECT_TELEPORT &&
               list.effects[0].destMapX == 3 &&
               list.effects[0].destMapY == 4 &&
               list.effects[1].kind == SENSOR_EFFECT_SHOW_TEXT &&
               list.effects[1].textIndex == 42,
           "F0718 WALK_ON effects are (teleport dest=3,4) then (text id=42)");

    /* WALK_OFF: v1 is conservative and produces no effects. */
    memset(&list, 0, sizeof(list));
    F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, 0, 0, 0, SENSOR_EVENT_WALK_OFF, &list);
    record("P32_F0718_WALK_OFF_EMPTY",
           list.count == 0,
           "F0718 WALK_OFF produces 0 effects in v1 (conservative)");

    /* Other events accepted but produce no effects. */
    memset(&list, 0, sizeof(list));
    F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, 0, 0, 0, SENSOR_EVENT_CHAMPION_ACTION, &list);
    record("P32_F0718_CHAMPION_ACTION",
           list.count == 0,
           "F0718 CHAMPION_ACTION on party square produces 0 effects");

    /* Empty chain square: no effects. */
    memset(&list, 0, sizeof(list));
    F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, 0, 1, 0, SENSOR_EVENT_WALK_ON, &list);
    record("P32_F0718_EMPTY_SQUARE",
           list.count == 0,
           "F0718 on empty-chain square produces 0 effects");

    /* Invalid event rejected. */
    memset(&list, 0, sizeof(list));
    record("P32_F0718_BAD_EVENT",
           F0718_SENSOR_ProcessPartyEnterLeave_Compat(
               &dungeon, &things, 0, 0, 0, -1, &list) == 0,
           "F0718 with out-of-range event returns 0");

    /* NULL safety. */
    record("P32_F0718_NULL_OUT",
           F0718_SENSOR_ProcessPartyEnterLeave_Compat(
               &dungeon, &things, 0, 0, 0, SENSOR_EVENT_WALK_ON, NULL) == 0,
           "F0718 with NULL outList returns 0");

    free(dungeon.maps);
    free(dungeon.tiles);

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
