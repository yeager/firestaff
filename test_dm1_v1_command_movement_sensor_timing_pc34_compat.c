#include <stdio.h>
#include <string.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_movement_timing_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"

/*
 * End-to-end DM1 V1 movement-core integration probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC locks the command
 *   queue, movement-gates C003..C006 without dequeueing, then dispatches turns
 *   to F0365 and moves to F0366.
 * - CLIKMENU.C:142-174 F0365 turns the party and runs leave/enter sensor
 *   processing on the current square; CLIKMENU.C:256-347 F0366 maps command
 *   arrows to relative steps, blocks walls/closed doors/fake-walls, calls
 *   F0267_MOVE_GetMoveResult_CPSCE for successful steps, then writes
 *   G0310_i_DisabledMovementTicks and clears G0311_i_ProjectileDisabledMovementTicks.
 * - MOVESENS.C:760-783 updates party scent and G0362_l_LastPartyMovementTime
 *   after a real square change; MOVESENS.C:799-818 calls
 *   F0276_SENSOR_ProcessThingAdditionOrRemoval on source leave and destination
 *   enter for party movement.
 * - MOVESENS.C:1553-1794 F0276 walks the square thing list and triggers each
 *   matching sensor effect in source order.
 */

#define MAP_W 4
#define MAP_H 4

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static unsigned char sqb(int elementType, int attrs)
{
    return (unsigned char)(((elementType & 7) << 5) | (attrs & 0x1f));
}

static unsigned short thing_ref(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static void reset_fixture(
    struct DungeonDatState_Compat* dungeon,
    struct DungeonMapDesc_Compat* map,
    struct DungeonMapTiles_Compat* tiles,
    struct DungeonThings_Compat* things,
    unsigned char squares[MAP_W * MAP_H],
    unsigned short squareFirstThings[MAP_W * MAP_H],
    struct DungeonSensor_Compat sensors[2],
    struct PartyState_Compat* party)
{
    int i;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(things, 0, sizeof(*things));
    memset(sensors, 0, 2 * sizeof(sensors[0]));
    memset(party, 0, sizeof(*party));

    for (i = 0; i < MAP_W * MAP_H; ++i) {
        squares[i] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    map->width = MAP_W;
    map->height = MAP_H;
    tiles->squareData = squares;
    tiles->squareCount = MAP_W * MAP_H;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    /* Destination (1,0) is the first thing-list square in scan order. */
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    sensors[0].sensorType = 0; /* teleport */
    sensors[0].localEffect = 0;
    sensors[0].targetMapX = 3;
    sensors[0].targetMapY = 2;
    sensors[0].targetCell = 1;
    sensors[0].next = thing_ref(THING_TYPE_SENSOR, 1);
    sensors[1].sensorType = 13; /* text/message */
    sensors[1].sensorData = 77;
    sensors[1].localEffect = 1;
    sensors[1].next = THING_ENDOFLIST;

    squareFirstThings[0] = thing_ref(THING_TYPE_SENSOR, 0);
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = MAP_W * MAP_H;
    things->sensors = sensors;
    things->sensorCount = 2;
    things->loaded = 1;

    party->mapIndex = 0;
    party->mapX = 1;
    party->mapY = 1;
    party->direction = DIR_NORTH;
    party->championCount = 2;
    party->champions[0].hp.current = 25;
    party->champions[0].load = 40;
    party->champions[0].maxLoad = 100;
    party->champions[1].hp.current = 35;
    party->champions[1].load = 100;
    party->champions[1].maxLoad = 100;
}

static int command_to_move_action(int command)
{
    return command - DM1_V1_COMMAND_MOVE_FORWARD;
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    unsigned char squares[MAP_W * MAP_H];
    unsigned short squareFirstThings[MAP_W * MAP_H];
    struct DungeonSensor_Compat sensors[2];
    struct PartyState_Compat party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputQueueProcessResultPc34Compat queueResult;
    struct MovementResult_Compat moveResult;
    struct SensorEffectList_Compat leaveEffects;
    struct SensorEffectList_Compat enterEffects;
    struct Dm1V1MovementTimingResultPc34Compat timing;
    int footwear[CHAMPION_MAX_PARTY] = { 0, 0, 0, 0 };
    int sourceX;
    int sourceY;
    int ok = 1;

    printf("probe=dm1_v1_command_movement_sensor_timing_pc34_compat\n");
    printf("sourceEvidence=COMMAND.C:396-405,2045-2156; CLIKMENU.C:142-174,256-347; MOVESENS.C:738-783,799-818,1553-1794\n");

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("forward key queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 0, 0);
    ok &= expect_int("forward dequeued", queueResult.dequeued, 1);
    ok &= expect_int("forward dispatched as move", queueResult.dispatchedMove, 1);

    sourceX = party.mapX;
    sourceY = party.mapY;
    ok &= expect_int("movement accepted", F0702_MOVEMENT_TryMove_Compat(&dungeon, &party,
        command_to_move_action(queueResult.command), &moveResult), 1);
    ok &= expect_int("movement target x", moveResult.newMapX, 1);
    ok &= expect_int("movement target y", moveResult.newMapY, 0);

    ok &= expect_int("source leave sensors empty", F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, party.mapIndex, sourceX, sourceY, SENSOR_EVENT_WALK_OFF, &leaveEffects), 1);
    ok &= expect_int("source leave effect count", leaveEffects.count, 0);

    party.mapX = moveResult.newMapX;
    party.mapY = moveResult.newMapY;
    ok &= expect_int("destination enter sensors processed", F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, party.mapIndex, party.mapX, party.mapY, SENSOR_EVENT_WALK_ON, &enterEffects), 1);
    ok &= expect_int("destination enter effect count", enterEffects.count, 2);
    ok &= expect_int("destination first effect teleport", enterEffects.effects[0].kind, SENSOR_EFFECT_TELEPORT);
    ok &= expect_int("destination teleport x", enterEffects.effects[0].destMapX, 3);
    ok &= expect_int("destination teleport y", enterEffects.effects[0].destMapY, 2);
    ok &= expect_int("destination second effect text", enterEffects.effects[1].kind, SENSOR_EFFECT_SHOW_TEXT);
    ok &= expect_int("destination text id", enterEffects.effects[1].textIndex, 77);

    timing = DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(&party, 0, sourceX, sourceY, 1000ul, 990ul, footwear);
    ok &= expect_int("successful step cadence from slowest living champion", timing.disabledMovementTicks, 4);
    ok &= expect_int("successful step clears projectile cadence", timing.projectileDisabledMovementTicks, 0);
    ok &= expect_int("successful step records scent", timing.scentRecorded, 1);
    ok &= expect_int("successful step scent delta", timing.scentDelayTicks, 10);
    ok &= expect_int("successful step updates last movement time", (int)timing.lastPartyMovementTime, 1000);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("blocked forward key queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_DOOR, 2);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 0, 0);
    ok &= expect_int("blocked command still dequeues before move legality", queueResult.dequeued, 1);
    ok &= expect_int("closed door blocks movement", F0702_MOVEMENT_TryMove_Compat(&dungeon, &party,
        command_to_move_action(queueResult.command), &moveResult), 0);
    ok &= expect_int("closed door block code", moveResult.resultCode, MOVE_BLOCKED_DOOR);
    ok &= expect_int("blocked movement leaves party x", party.mapX, 1);
    ok &= expect_int("blocked movement leaves party y", party.mapY, 1);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("gated forward key queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 3, 0, 0);
    ok &= expect_int("disabled movement gate reported", queueResult.movementDisabledGate, 1);
    ok &= expect_int("disabled movement not dequeued", queueResult.dequeued, 0);
    ok &= expect_int("disabled movement not dispatched", queueResult.dispatchedMove, 0);
    ok &= expect_int("disabled movement leaves command queued", (int)queue.count, 1);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile forward key queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 7, DIR_NORTH);
    ok &= expect_int("projectile same-direction movement gate reported", queueResult.movementDisabledGate, 1);
    ok &= expect_int("projectile same-direction movement not dequeued", queueResult.dequeued, 0);
    ok &= expect_int("projectile same-direction movement leaves command queued", (int)queue.count, 1);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile sidestep key queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB33, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 7, DIR_NORTH);
    ok &= expect_int("projectile different-direction movement bypasses gate", queueResult.movementDisabledGate, 0);
    ok &= expect_int("projectile different-direction movement dequeued", queueResult.dequeued, 1);
    ok &= expect_int("projectile different-direction movement dispatched", queueResult.dispatchedMove, 1);
    ok &= expect_int("projectile different-direction command id", queueResult.command, DM1_V1_COMMAND_MOVE_RIGHT);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("turn key queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 9, 0, 0);
    ok &= expect_int("turn bypasses movement gate", queueResult.movementDisabledGate, 0);
    ok &= expect_int("turn dequeued", queueResult.dequeued, 1);
    ok &= expect_int("turn dispatched", queueResult.dispatchedTurn, 1);
    ok &= expect_int("turn new direction", F0700_MOVEMENT_TurnDirection_Compat(party.direction, 1), DIR_EAST);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    ok &= expect_int("locked mouse forward stored as pending", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 270, 130, DM1_V1_BUTTON_LEFT }), 0);
    ok &= expect_int("pending click present while locked", queue.pendingClickPresent, 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 0, 0);
    ok &= expect_int("empty locked queue replays pending click", queueResult.pendingReplayCount, 1);
    ok &= expect_int("pending click becomes queued command", (int)queue.count, 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 0, 0);
    ok &= expect_int("mouse forward command dequeued", queueResult.dequeued, 1);
    ok &= expect_int("mouse forward dispatched as move", queueResult.dispatchedMove, 1);
    ok &= expect_int("mouse forward command id", queueResult.command, DM1_V1_COMMAND_MOVE_FORWARD);
    sourceX = party.mapX;
    sourceY = party.mapY;
    ok &= expect_int("mouse movement accepted", F0702_MOVEMENT_TryMove_Compat(&dungeon, &party,
        command_to_move_action(queueResult.command), &moveResult), 1);
    party.mapX = moveResult.newMapX;
    party.mapY = moveResult.newMapY;
    ok &= expect_int("mouse movement destination sensors processed", F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, party.mapIndex, party.mapX, party.mapY, SENSOR_EVENT_WALK_ON, &enterEffects), 1);
    ok &= expect_int("mouse movement destination effect count", enterEffects.count, 2);

    printf("dm1V1CommandMovementSensorTimingIntegrationOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
