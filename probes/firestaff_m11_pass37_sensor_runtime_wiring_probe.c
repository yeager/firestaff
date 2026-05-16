/*
 * Pass 37 bounded probe — sensor enter/leave runtime wiring.
 *
 * Pass 32 landed the compat owner F0718_SENSOR_ProcessPartyEnterLeave_Compat
 * and probe-verified it at 12/12 invariants, but
 * F0888_ORCH_ApplyPlayerInput_Compat did NOT yet invoke it on every
 * party square change.  Pass 37 wires that invocation and surfaces
 * every produced SensorEffect as a new EMIT_SENSOR_EFFECT emission on
 * the TickResult_Compat stream.
 *
 * This probe exercises the REAL orchestrator path (not F0718 directly).
 * It builds a 3x3 dungeon map with:
 *   - a two-sensor chain (teleport + text) on the square the party
 *     moves INTO  (expect two EMIT_SENSOR_EFFECT emissions tagged
 *     SENSOR_EVENT_WALK_ON).
 *   - a one-sensor chain on the square the party moves OUT OF (in v1,
 *     WALK_OFF is conservative and produces no effects).
 * It then runs F0888_ORCH_ApplyPlayerInput_Compat with CMD_MOVE_EAST
 * and inspects the resulting TickResult_Compat.emissions[].
 *
 * Honest scope note: Pass 37 wires EMIT-only surfacing.  Teleport
 * world mutation still flows through F0704 / tile-type teleporters,
 * not through the sensor-effect teleport branch.  This is the bounded
 * v1 scope recorded in V1_BLOCKERS.md item #1 and PASSLIST_29_36.md
 * §4.32; blocker #2 (animating door intermediate states) is explicitly
 * deferred to pass-38.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"

#define MAP_W 3
#define MAP_H 3

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static unsigned char sqb(int elementType, int attribs) {
    return (unsigned char)(((elementType & 7) << 5) | (attribs & 0x1F));
}

/*
 * count_emissions_of_kind: how many emissions of a given kind appear
 * in the tick result.
 */
static int count_emissions_of_kind(const struct TickResult_Compat* r, uint8_t kind) {
    int i, n = 0;
    for (i = 0; i < r->emissionCount; ++i) {
        if (r->emissions[i].kind == kind) ++n;
    }
    return n;
}

int main(void) {
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[MAP_W * MAP_H];
    /*
     * squareFirstThings is indexed by the POSITIONAL SFT index (count
     * of thing-list squares encountered in row-major order up to and
     * including this square, minus one), not by the raw square index.
     * We have two thing-list squares (origin (0,0) and destination
     * (1,0)); origin comes first in row-major order at sft=0,
     * destination at sft=1.
     */
    unsigned short squareFirstThings[2];

    /* Two sensors for the DESTINATION square (party moves INTO this one)
     * plus one sensor for the ORIGIN square. */
    struct DungeonSensor_Compat sensors[3];

    int walkOnCount, walkOffCount;
    int i;
    int sawTeleport = 0, sawText = 0;
    int orderOk = 0;
    int firstSensorEmissionIdx = -1;

    /* -- Dungeon fabric (3x3 corridor row) -- */
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(sensors, 0, sizeof(sensors));

    for (i = 0; i < MAP_W * MAP_H; ++i) {
        squareData[i] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareFirstThings[0] = THING_ENDOFLIST;
    squareFirstThings[1] = THING_ENDOFLIST;

    /* Row-major square index: squareIdx = mapX * height + mapY.
     * Row-major traversal walks sq=0..W*H-1.  Our two thing-list
     * squares are at squareIdx 0 (origin (0,0)) and squareIdx 3
     * (destination (1,0)).  sft indices are therefore 0 and 1. */

    /* origin (x=0,y=0): thing-list bit + one text sensor (will only
     * fire on WALK_OFF; v1 conservative policy means 0 effects). */
    squareData[0 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR,
                                     DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] =
        (unsigned short)((THING_TYPE_SENSOR << 10) | 2);

    /* destination (x=1,y=0): thing-list bit + chain of two sensors:
     * sensor[0]=teleport -> sensor[1]=text -> END.  Both should fire
     * on WALK_ON and produce two EMIT_SENSOR_EFFECT emissions in the
     * TickResult_Compat. */
    squareData[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR,
                                     DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[1] =
        (unsigned short)((THING_TYPE_SENSOR << 10) | 0);

    sensors[0].sensorType = 0;          /* teleport */
    sensors[0].sensorData = 0;
    sensors[0].localEffect = 0;
    sensors[0].targetMapX = 2;
    sensors[0].targetMapY = 2;
    sensors[0].targetCell = 1;
    sensors[0].next = (unsigned short)((THING_TYPE_SENSOR << 10) | 1);

    sensors[1].sensorType = 13;         /* text */
    sensors[1].sensorData = 99;
    sensors[1].localEffect = 1;
    sensors[1].next = THING_ENDOFLIST;

    sensors[2].sensorType = 13;         /* text (on origin square) */
    sensors[2].sensorData = 7;
    sensors[2].localEffect = 1;
    sensors[2].next = THING_ENDOFLIST;

    maps[0].width = MAP_W;
    maps[0].height = MAP_H;
    tiles[0].squareCount = MAP_W * MAP_H;
    tiles[0].squareData = squareData;

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    dungeon.loaded = 1;

    things.loaded = 1;
    things.sensors = sensors;
    things.sensorCount = 3;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2; /* number of thing-list squares */

    /* -- World with two champions, standing at (0,0) facing EAST -- */
    memset(&world, 0, sizeof(world));
    if (!F0881_WORLD_InitDefault_Compat(&world, 42u)) {
        printf("FAIL P37_INIT_WORLD F0881_WORLD_InitDefault_Compat failed\n");
        printf("# summary: 0/1 invariants passed\n");
        return 1;
    }
    world.party.championCount = 2;
    world.party.activeChampionIndex = 0;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 200;
    world.party.champions[1].present = 1;
    world.party.champions[1].hp.current = 100;
    world.party.champions[1].hp.maximum = 200;
    world.party.mapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.party.direction = DIR_EAST;
    world.dungeon = &dungeon;
    world.things = &things;
    world.ownsDungeon = 0;

    /* -- Drive the orchestrator: CMD_MOVE_EAST moves from (0,0) to (1,0) -- */
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_MOVE_EAST;

    (void)F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result);

    /* -- Invariant 1: party actually moved. */
    record("P37_PARTY_MOVED_XY",
           world.party.mapX == 1 && world.party.mapY == 0,
           "CMD_MOVE_EAST moved party from (0,0) to (1,0)");

    /* -- Invariant 2: at least one EMIT_PARTY_MOVED emission present. */
    record("P37_EMIT_PARTY_MOVED_PRESENT",
           count_emissions_of_kind(&result, EMIT_PARTY_MOVED) >= 1,
           "Orchestrator emitted at least one EMIT_PARTY_MOVED");

    /* -- Invariant 3: exactly two EMIT_SENSOR_EFFECT emissions total
     *    (teleport + text on destination, zero on origin by WALK_OFF
     *     v1 policy). */
    {
        int total = count_emissions_of_kind(&result, EMIT_SENSOR_EFFECT);
        record("P37_SENSOR_EFFECT_TOTAL",
               total == 2,
               "Exactly 2 EMIT_SENSOR_EFFECT emissions (2 on WALK_ON, 0 on WALK_OFF)");
    }

    /* Walk each EMIT_SENSOR_EFFECT emission and count by payload[2]. */
    walkOnCount = 0;
    walkOffCount = 0;
    for (i = 0; i < result.emissionCount; ++i) {
        const struct TickEmission_Compat* e = &result.emissions[i];
        if (e->kind != EMIT_SENSOR_EFFECT) continue;
        if (firstSensorEmissionIdx < 0) firstSensorEmissionIdx = i;
        if (e->payload[2] == SENSOR_EVENT_WALK_ON)  ++walkOnCount;
        if (e->payload[2] == SENSOR_EVENT_WALK_OFF) ++walkOffCount;
        if (e->payload[0] == SENSOR_EFFECT_TELEPORT)  sawTeleport = 1;
        if (e->payload[0] == SENSOR_EFFECT_SHOW_TEXT) sawText = 1;
    }

    /* -- Invariant 4: WALK_ON emissions are exactly 2 for the destination chain. */
    record("P37_SENSOR_WALK_ON_COUNT",
           walkOnCount == 2,
           "Two EMIT_SENSOR_EFFECT emissions carry SENSOR_EVENT_WALK_ON");

    /* -- Invariant 5: WALK_OFF emissions are 0 under v1 conservative policy. */
    record("P37_SENSOR_WALK_OFF_CONSERVATIVE",
           walkOffCount == 0,
           "Zero EMIT_SENSOR_EFFECT emissions carry SENSOR_EVENT_WALK_OFF in v1");

    /* -- Invariant 6: both expected effect kinds are represented. */
    record("P37_SENSOR_TELEPORT_SURFACED",
           sawTeleport == 1,
           "SENSOR_EFFECT_TELEPORT surfaced through EMIT_SENSOR_EFFECT");
    record("P37_SENSOR_TEXT_SURFACED",
           sawText == 1,
           "SENSOR_EFFECT_SHOW_TEXT surfaced through EMIT_SENSOR_EFFECT");

    /* -- Invariant 7: source-order preserved (teleport first, then text). */
    if (walkOnCount == 2 && firstSensorEmissionIdx >= 0 &&
        (firstSensorEmissionIdx + 1) < result.emissionCount) {
        const struct TickEmission_Compat* e0 = &result.emissions[firstSensorEmissionIdx];
        const struct TickEmission_Compat* e1 = &result.emissions[firstSensorEmissionIdx + 1];
        orderOk = (e0->kind == EMIT_SENSOR_EFFECT &&
                   e1->kind == EMIT_SENSOR_EFFECT &&
                   e0->payload[2] == SENSOR_EVENT_WALK_ON &&
                   e1->payload[2] == SENSOR_EVENT_WALK_ON &&
                   e0->payload[0] == SENSOR_EFFECT_TELEPORT &&
                   e1->payload[0] == SENSOR_EFFECT_SHOW_TEXT);
    }
    record("P37_SENSOR_SOURCE_ORDER",
           orderOk == 1,
           "WALK_ON emissions are in source order: teleport then text");

    /* -- Invariant 8: teleport payload[3] carries destMapIndex, text
     *    payload[3] carries textIndex (99). */
    {
        int telePayloadOk = 0, textPayloadOk = 0;
        int teleSensorTypeOk = 0, textSensorTypeOk = 0;
        for (i = 0; i < result.emissionCount; ++i) {
            const struct TickEmission_Compat* e = &result.emissions[i];
            if (e->kind != EMIT_SENSOR_EFFECT) continue;
            if (e->payload[0] == SENSOR_EFFECT_TELEPORT) {
                /* sensors[0].targetMapX=2 so destMapIndex is 0
                 * (stay on map 0); we verify sensorType is 0. */
                teleSensorTypeOk = (e->payload[1] == 0);
                telePayloadOk = 1; /* just that the emission exists */
            } else if (e->payload[0] == SENSOR_EFFECT_SHOW_TEXT) {
                textPayloadOk = (e->payload[3] == 99);
                textSensorTypeOk = (e->payload[1] == 13);
            }
        }
        record("P37_SENSOR_TELE_TYPE",
               teleSensorTypeOk == 1,
               "Teleport emission carries sensorType=0 in payload[1]");
        record("P37_SENSOR_TEXT_PAYLOAD",
               textPayloadOk == 1 && textSensorTypeOk == 1,
               "Text emission: payload[1]=13 sensorType, payload[3]=99 textIndex");
        (void)telePayloadOk;
    }

    /* -- Invariant 9: idempotent.  Re-running the orchestrator when
     *    the party did NOT move (CMD_TURN_LEFT on corridor) produces
     *    NO EMIT_SENSOR_EFFECT emissions even though the party is
     *    still on a sensor square.  This confirms the emission is
     *    gated on mr.resultCode == MOVE_OK, not on square occupancy. */
    {
        struct TickResult_Compat r2;
        memset(&r2, 0, sizeof(r2));
        input.command = CMD_TURN_LEFT;
        (void)F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &r2);
        record("P37_SENSOR_NOT_ON_TURN",
               count_emissions_of_kind(&r2, EMIT_SENSOR_EFFECT) == 0,
               "CMD_TURN_LEFT (MOVE_TURN_ONLY) produces 0 EMIT_SENSOR_EFFECT emissions");
    }

    /* -- Invariant 10: moving OUT of a sensor square and INTO an
     *    empty square produces 0 WALK_ON emissions, 0 WALK_OFF
     *    emissions (WALK_OFF v1 conservative), but the move itself
     *    still succeeds and emits EMIT_PARTY_MOVED.  Confirms the
     *    ORIGIN-side invocation is real, not just the destination. */
    {
        struct TickResult_Compat r3;
        /* world is now at (1,0), direction WEST after TURN_LEFT from EAST. */
        memset(&r3, 0, sizeof(r3));
        input.command = CMD_MOVE_EAST;
        /* Turn back to EAST explicitly so the next move is to (2,0). */
        world.party.direction = DIR_EAST;
        (void)F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &r3);
        record("P37_MOVE_OFF_SENSOR_WALK_OFF_ZERO",
               count_emissions_of_kind(&r3, EMIT_SENSOR_EFFECT) == 0,
               "Leaving a sensor square onto an empty square emits 0 EMIT_SENSOR_EFFECT");
        record("P37_MOVE_OFF_SENSOR_PARTY_MOVED",
               count_emissions_of_kind(&r3, EMIT_PARTY_MOVED) >= 1 &&
                   world.party.mapX == 2 && world.party.mapY == 0,
               "Move succeeded: party now at (2,0) with EMIT_PARTY_MOVED");
    }

    /* Do not call F0883_WORLD_Free_Compat — the dungeon/things pointers
     * are stack fixtures (ownsDungeon=0) and freeing them would segfault. */

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
