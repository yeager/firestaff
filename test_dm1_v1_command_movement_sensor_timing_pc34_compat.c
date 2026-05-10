#include <stdio.h>
#include <string.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_movement_command_core_pc34_compat.h"
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
 *   processing on the current square; CLIKMENU.C:237-255 F0366 applies
 *   living-champion movement stamina before stairs/blocker resolution;
 *   CLIKMENU.C:256-347 maps command arrows through DUNGEON.C:1371-1421 relative-coordinate math, blocks walls/
 *   closed doors/fake-walls/groups at CLIKMENU.C:278-323 before any F0267
 *   movement/sensor core call, calls F0267_MOVE_GetMoveResult_CPSCE for
 *   successful steps, then writes G0310_i_DisabledMovementTicks from
 *   CHAMPION.C:1180-1215 movement ticks and clears
 *   G0311_i_ProjectileDisabledMovementTicks.
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
    party->champions[0].stamina.current = 100;
    party->champions[0].stamina.maximum = 100;
    party->champions[1].hp.current = 35;
    party->champions[1].load = 100;
    party->champions[1].maxLoad = 100;
    party->champions[1].stamina.current = 100;
    party->champions[1].stamina.maximum = 100;
}

static int command_to_move_action(int command)
{
    return command - DM1_V1_COMMAND_MOVE_FORWARD;
}

static int expect_relative_step_delta_table(void)
{
    static const int wantDx[4][4] = {
        { 0, 1, 0, -1 },
        { 1, 0, -1, 0 },
        { 0, -1, 0, 1 },
        { -1, 0, 1, 0 }
    };
    static const int wantDy[4][4] = {
        { -1, 0, 1, 0 },
        { 0, 1, 0, -1 },
        { 1, 0, -1, 0 },
        { 0, -1, 0, 1 }
    };
    int dir;
    int action;
    int dx;
    int dy;
    int ok = 1;

    for (dir = DIR_NORTH; dir <= DIR_WEST; ++dir) {
        for (action = MOVE_FORWARD; action <= MOVE_LEFT; ++action) {
            F0701_MOVEMENT_GetStepDelta_Compat(dir, action, &dx, &dy);
            ok &= expect_int("relative movement delta dx", dx, wantDx[dir][action]);
            ok &= expect_int("relative movement delta dy", dy, wantDy[dir][action]);
        }
    }
    return ok;
}

static int expect_blocked_step_without_side_effects(
    const char* label,
    struct DungeonDatState_Compat* dungeon,
    struct DungeonThings_Compat* things,
    struct PartyState_Compat* party,
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int expectedBlockCode,
    int expectGroupBlock)
{
    struct Dm1V1InputQueueProcessResultPc34Compat queueResult;
    struct MovementResult_Compat moveResult;
    struct Dm1V1MovementTimingResultPc34Compat timing;
    int ok = 1;
    int sensorSideEffectCalls = 0;
    int timingSideEffectCalls = 0;
    int sourceX = party->mapX;
    int sourceY = party->mapY;
    unsigned long previousLastMovementTime = 990ul;

    DM1_V1_InputCommandQueue_InitPc34Compat(queue);
    ok &= expect_int(label, DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(queue, party->direction, 0, 0, 0);
    ok &= expect_int("blocked-side-effects command dequeued before legality", queueResult.dequeued, 1);
    ok &= expect_int("blocked-side-effects command dispatched as move", queueResult.dispatchedMove, 1);

    if (expectGroupBlock) {
        ok &= expect_int("group target is tile-passable", F0702_MOVEMENT_TryMove_Compat(dungeon, party,
            command_to_move_action(queueResult.command), &moveResult), 1);
        ok &= expect_int("group target base movement ok", moveResult.resultCode, MOVE_OK);
        ok &= expect_int("group blocks before move core", F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
            dungeon, things, party, command_to_move_action(queueResult.command)), 1);
    } else {
        ok &= expect_int("wall/door/fakewall blocks before move core", F0702_MOVEMENT_TryMove_Compat(dungeon, party,
            command_to_move_action(queueResult.command), &moveResult), 0);
        ok &= expect_int("blocked result code", moveResult.resultCode, expectedBlockCode);
    }

    /* Source lock: CLIKMENU.C:317-323 returns on blocked movement before
     * MOVESENS.C:F0267 can run leave/enter sensors (MOVESENS.C:799-818) or
     * successful-step scent/timing (MOVESENS.C:764-783).  The probe models
     * that orchestration gate explicitly: the success-only side-effect calls
     * below must remain skipped for all blocked reasons. */
    if ((!expectGroupBlock && moveResult.resultCode == MOVE_OK) ||
        (expectGroupBlock && !F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
            dungeon, things, party, command_to_move_action(queueResult.command)))) {
        struct SensorEffectList_Compat effects;
        ++sensorSideEffectCalls;
        (void)F0718_SENSOR_ProcessPartyEnterLeave_Compat(
            dungeon, things, party->mapIndex, party->mapX, party->mapY, SENSOR_EVENT_WALK_OFF, &effects);
        party->mapX = moveResult.newMapX;
        party->mapY = moveResult.newMapY;
        ++timingSideEffectCalls;
        timing = DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(
            party, 0, sourceX, sourceY, 1000ul, previousLastMovementTime, NULL);
    } else {
        memset(&timing, 0, sizeof(timing));
        timing.lastPartyMovementTime = previousLastMovementTime;
    }

    ok &= expect_int("blocked side effects leave party x", party->mapX, sourceX);
    ok &= expect_int("blocked side effects leave party y", party->mapY, sourceY);
    ok &= expect_int("blocked movement skips enter/leave sensors", sensorSideEffectCalls, 0);
    ok &= expect_int("blocked movement skips timing update", timingSideEffectCalls, 0);
    ok &= expect_int("blocked movement preserves last movement time", (int)timing.lastPartyMovementTime, (int)previousLastMovementTime);
    ok &= expect_int("blocked movement records no scent", timing.scentRecorded, 0);
    return ok;
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
    struct DungeonGroup_Compat groups[1];
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
    printf("sourceEvidence=COMMAND.C:396-405,2045-2156; CLIKMENU.C:142-179,180-347 including 237-255 pre-resolution stamina; DUNGEON.C:1371-1447; CHAMPION.C:1180-1215,2025-2048; MOVESENS.C:738-783,799-818,1553-1794; GAMELOOP.C:90,215-219; DRAWVIEW.C:709-724\n");

    ok &= expect_relative_step_delta_table();

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

    party.mapX = moveResult.newMapX;
    party.mapY = moveResult.newMapY;
    ok &= expect_int("source leave sensors empty after party coord update", F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, party.mapIndex, sourceX, sourceY, SENSOR_EVENT_WALK_OFF, &leaveEffects), 1);
    ok &= expect_int("source leave effect count", leaveEffects.count, 0);

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
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_WALL, 0);
    ok &= expect_blocked_step_without_side_effects(
        "wall blocked-side-effects key queued", &dungeon, &things, &party, &queue, MOVE_BLOCKED_WALL, 0);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_DOOR, 2);
    ok &= expect_blocked_step_without_side_effects(
        "door blocked-side-effects key queued", &dungeon, &things, &party, &queue, MOVE_BLOCKED_DOOR, 0);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_FAKEWALL, 0);
    ok &= expect_blocked_step_without_side_effects(
        "fakewall blocked-side-effects key queued", &dungeon, &things, &party, &queue, MOVE_BLOCKED_WALL, 0);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    things.groups = groups;
    squareFirstThings[0] = thing_ref(THING_TYPE_GROUP, 0);
    things.groupCount = 1;
    ok &= expect_blocked_step_without_side_effects(
        "group blocked-side-effects key queued", &dungeon, &things, &party, &queue, MOVE_OK, 1);

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
    squares[1 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[1] = thing_ref(THING_TYPE_SENSOR, 1);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("turn current-square sensor key queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 6, 3, DIR_NORTH);
    ok &= expect_int("turn current-square sensor bypasses movement gates", queueResult.movementDisabledGate, 0);
    ok &= expect_int("turn current-square sensor dequeued", queueResult.dequeued, 1);
    ok &= expect_int("turn current-square sensor dispatched", queueResult.dispatchedTurn, 1);
    sourceX = party.mapX;
    sourceY = party.mapY;
    ok &= expect_int("turn current-square leave sensors processed", F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, party.mapIndex, party.mapX, party.mapY, SENSOR_EVENT_WALK_OFF, &leaveEffects), 1);
    ok &= expect_int("turn current-square leave has no v1 effects", leaveEffects.count, 0);
    party.direction = F0700_MOVEMENT_TurnDirection_Compat(party.direction, 1);
    ok &= expect_int("turn current-square enter sensors processed", F0718_SENSOR_ProcessPartyEnterLeave_Compat(
        &dungeon, &things, party.mapIndex, party.mapX, party.mapY, SENSOR_EVENT_WALK_ON, &enterEffects), 1);
    ok &= expect_int("turn current-square enter effect count", enterEffects.count, 1);
    ok &= expect_int("turn current-square enter text effect", enterEffects.effects[0].kind, SENSOR_EFFECT_SHOW_TEXT);
    ok &= expect_int("turn current-square enter text id", enterEffects.effects[0].textIndex, 77);
    ok &= expect_int("turn current-square leaves party x", party.mapX, sourceX);
    ok &= expect_int("turn current-square leaves party y", party.mapY, sourceY);
    ok &= expect_int("turn current-square updates direction only", party.direction, DIR_EAST);

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


    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    {
        struct Dm1V1MovementCommandCoreResultPc34Compat core;
        int redraws = 0;
        int stops = 0;
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("core forward1 queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
        ok &= expect_int("core forward1 processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 0, 0, 0, 1000ul, 990ul, footwear, &core), 1);
        redraws += core.viewportRedrawRequested;
        stops += core.stopWaitingForPlayerInput;
        ok &= expect_int("core forward1 step applied", core.stepApplied, 1);
        ok &= expect_int("core forward1 party x", party.mapX, 1);
        ok &= expect_int("core forward1 party y", party.mapY, 0);
        ok &= expect_int("core forward1 destination sensors", core.enterEffects.count, 2);
        ok &= expect_int("core forward1 teleport sensor first", core.enterEffects.effects[0].kind, SENSOR_EFFECT_TELEPORT);
        ok &= expect_int("core forward1 text sensor second", core.enterEffects.effects[1].kind, SENSOR_EFFECT_SHOW_TEXT);
        ok &= expect_int("core forward1 timing records scent", core.timing.scentRecorded, 1);
        ok &= expect_int("core forward1 stamina cost champion0", core.staminaCost[0], 2);
        ok &= expect_int("core forward1 stamina cost champion1", core.staminaCost[1], 4);
        ok &= expect_int("core forward1 decrements champion0 stamina", party.champions[0].stamina.current, 98);
        ok &= expect_int("core forward1 decrements champion1 stamina", party.champions[1].stamina.current, 96);
        ok &= expect_int("core forward1 requests viewport", core.viewportRedrawRequested, 1);
        ok &= expect_int("core forward1 releases input wait", core.stopWaitingForPlayerInput, 1);

        ok &= expect_int("core turn-right queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
        ok &= expect_int("core turn-right processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 9, 5, DIR_NORTH, 1001ul, core.timing.lastPartyMovementTime, footwear, &core), 1);
        redraws += core.viewportRedrawRequested;
        stops += core.stopWaitingForPlayerInput;
        ok &= expect_int("core turn bypasses movement gates", core.queue.movementDisabledGate, 0);
        ok &= expect_int("core turn applied", core.turnApplied, 1);
        ok &= expect_int("core turn keeps x", party.mapX, 1);
        ok &= expect_int("core turn keeps y", party.mapY, 0);
        ok &= expect_int("core turn updates direction", party.direction, DIR_EAST);
        ok &= expect_int("core turn current-square enter sensors", core.enterEffects.count, 2);
        ok &= expect_int("core turn leaves champion0 stamina", party.champions[0].stamina.current, 98);
        ok &= expect_int("core turn leaves champion1 stamina", party.champions[1].stamina.current, 96);
        ok &= expect_int("core turn requests viewport", core.viewportRedrawRequested, 1);

        ok &= expect_int("core right queued from east", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB33, 0, 0, 0 }), 1);
        ok &= expect_int("core relative strafe/back seam processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 0, 0, 0, 1002ul, 1000ul, footwear, &core), 1);
        redraws += core.viewportRedrawRequested;
        stops += core.stopWaitingForPlayerInput;
        ok &= expect_int("core east-facing right strafe x unchanged", party.mapX, 1);
        ok &= expect_int("core east-facing right strafe moves south", party.mapY, 1);
        ok &= expect_int("core east-facing right strafe direction unchanged", party.direction, DIR_EAST);
        ok &= expect_int("core east-facing right strafe stamina affected count", core.staminaAffectedCount, 2);
        ok &= expect_int("core east-facing right strafe decrements champion0 stamina again", party.champions[0].stamina.current, 96);
        ok &= expect_int("core east-facing right strafe decrements champion1 stamina again", party.champions[1].stamina.current, 92);
        ok &= expect_int("core multi-command viewport redraw count", redraws, 3);
        ok &= expect_int("core multi-command stop-wait count", stops, 3);
    }

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_WALL, 0);
    {
        struct Dm1V1MovementCommandCoreResultPc34Compat core;
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("core blocked wall queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
        ok &= expect_int("core blocked wall followup queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
        ok &= expect_int("core blocked wall processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 0, 0, 0, 1000ul, 990ul, footwear, &core), 1);
        ok &= expect_int("core blocked wall dequeued", core.queue.dequeued, 1);
        ok &= expect_int("core blocked wall marked", core.movementBlocked, 1);
        ok &= expect_int("core blocked wall code", core.movement.resultCode, MOVE_BLOCKED_WALL);
        ok &= expect_int("core blocked wall skips sensors", core.enterEffects.count + core.leaveEffects.count, 0);
        ok &= expect_int("core blocked wall skips viewport", core.viewportRedrawRequested, 0);
        ok &= expect_int("core blocked wall still applies pre-resolution stamina", party.champions[0].stamina.current, 98);
        ok &= expect_int("core blocked wall records stamina cost", core.staminaCost[0], 2);
        ok &= expect_int("core blocked wall discards input", core.inputDiscardRequested, 1);
        ok &= expect_int("core blocked wall clears queued followup", (int)queue.count, 0);
        ok &= expect_int("core blocked wall keeps x", party.mapX, 1);
        ok &= expect_int("core blocked wall keeps y", party.mapY, 1);
    }

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    memset(groups, 0, sizeof(groups));
    groups[0].next = THING_ENDOFLIST;
    things.groups = groups;
    squareFirstThings[0] = thing_ref(THING_TYPE_GROUP, 0);
    things.groupCount = 1;
    {
        struct Dm1V1MovementCommandCoreResultPc34Compat core;
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("core group block queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
        ok &= expect_int("core group block followup queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
        ok &= expect_int("core group block processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 0, 0, 0, 1000ul, 990ul, footwear, &core), 1);
        ok &= expect_int("core group block detected", core.blockedByGroup, 1);
        ok &= expect_int("core group block requests adjacent reaction", core.groupReactionPartyAdjacentRequested, 1);
        ok &= expect_int("core group block skips sensors", core.enterEffects.count + core.leaveEffects.count, 0);
        ok &= expect_int("core group block skips viewport", core.viewportRedrawRequested, 0);
        ok &= expect_int("core group block still applies pre-resolution stamina", party.champions[0].stamina.current, 98);
        ok &= expect_int("core group block records stamina cost", core.staminaCost[0], 2);
        ok &= expect_int("core group block clears queued followup", (int)queue.count, 0);
        ok &= expect_int("core group block keeps source", party.mapX + party.mapY, 2);
    }

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, sensors, &party);
    squareFirstThings[0] = thing_ref(THING_TYPE_GROUP, 0);
    {
        struct DungeonGroup_Compat group;
        struct Dm1V1MovementCommandCoreResultPc34Compat core;
        memset(&group, 0, sizeof(group));
        group.next = THING_ENDOFLIST;
        things.groups = &group;
        things.groupCount = 1;
        party.championCount = 0;
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("empty-party group collision bug queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
        ok &= expect_int("empty-party group collision bug processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 0, 0, 0, 1000ul, 990ul, footwear, &core), 1);
        ok &= expect_int("empty-party group collision bug not blocked", core.movementBlocked, 0);
        ok &= expect_int("empty-party group collision bug step applied", core.stepApplied, 1);
        ok &= expect_int("empty-party group collision bug reaches group square x", party.mapX, 1);
        ok &= expect_int("empty-party group collision bug reaches group square y", party.mapY, 0);
        ok &= expect_int("empty-party group collision bug no scent", core.timing.scentRecorded, 0);
        ok &= expect_int("empty-party group collision bug minimum cooldown", core.timing.disabledMovementTicks, 1);
    }

    printf("dm1V1CommandMovementSensorTimingIntegrationOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
