#include <stdio.h>
#include <string.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "memory_movement_pc34_compat.h"

/*
 * Broad DM1 V1 movement core invariant probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC locks the queue,
 *   leaves movement commands queued while G0310/G0311 movement gates are active,
 *   replays pending clicks after unlock, then dispatches turns to F0365 and
 *   movement commands C003..C006 to F0366.
 * - CLIKMENU.C:180-347 F0366_COMMAND_ProcessTypes3To6_MoveParty maps movement
 *   arrow index to relative step vectors, checks wall / door-state / fake-wall
 *   blockers, allows stairs as a consequence square, detects group blocking,
 *   discards input on blocked movement, and sets movement/projectile cooldowns
 *   only after successful movement.
 * - CLIKMENU.C:224-233 defines forward/right/back/left relative step counts.
 * - CLIKMENU.C:278-288 blocks wall, closed door states, and closed real
 *   fake-walls; pits/teleporters fall through as passable square types.
 * - DUNGEON.C:1371-1391 F0150 applies direction-relative forward/right deltas.
 */

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static unsigned char square_type(int elementType, int attrs)
{
    return (unsigned char)((elementType << 5) | (attrs & DUNGEON_SQUARE_MASK_ATTRIBS));
}

static void set_square(unsigned char* squares, int height, int x, int y, unsigned char value)
{
    squares[x * height + y] = value;
}

static void setup_dungeon(struct DungeonDatState_Compat* dungeon,
    struct DungeonMapDesc_Compat* map,
    struct DungeonMapTiles_Compat* tiles,
    unsigned char* squares,
    int width,
    int height)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(squares, 0, (size_t)(width * height));
    map->width = (unsigned char)width;
    map->height = (unsigned char)height;
    tiles->squareData = squares;
    tiles->squareCount = width * height;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            set_square(squares, height, x, y, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));
        }
    }
}

static int command_to_move_action(int command)
{
    return command - DM1_V1_COMMAND_MOVE_FORWARD;
}

static int process_key_and_try_move(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party,
    int keyCode,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection,
    struct Dm1V1InputQueueProcessResultPc34Compat* outQueueResult,
    struct MovementResult_Compat* outMoveResult)
{
    if (!DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, keyCode, 0, 0, 0 })) {
        return 0;
    }
    *outQueueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
        queue,
        party->direction,
        disabledMovementTicks,
        projectileDisabledMovementTicks,
        lastProjectileDisabledMovementDirection);
    if (!outQueueResult->dequeued || !outQueueResult->dispatchedMove) {
        memset(outMoveResult, 0, sizeof(*outMoveResult));
        return 0;
    }
    return F0702_MOVEMENT_TryMove_Compat(
        dungeon,
        party,
        command_to_move_action(outQueueResult->command),
        outMoveResult);
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[25];
    struct PartyState_Compat party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputQueueProcessResultPc34Compat queueResult;
    struct MovementResult_Compat moveResult;
    int dx;
    int dy;
    int ok = 1;

    printf("probe=dm1_v1_movement_core_pc34_compat\n");
    printf("sourceEvidence=COMMAND.C:2045-2156; CLIKMENU.C:180-347,224-233,278-288; DUNGEON.C:1371-1391\n");

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0;
    party.mapX = 2;
    party.mapY = 2;
    party.direction = DIR_NORTH;
    party.championCount = 1;

    F0701_MOVEMENT_GetStepDelta_Compat(DIR_NORTH, MOVE_FORWARD, &dx, &dy);
    ok &= expect_int("north forward dx", dx, 0);
    ok &= expect_int("north forward dy", dy, -1);
    F0701_MOVEMENT_GetStepDelta_Compat(DIR_NORTH, MOVE_RIGHT, &dx, &dy);
    ok &= expect_int("north right dx", dx, 1);
    ok &= expect_int("north right dy", dy, 0);
    F0701_MOVEMENT_GetStepDelta_Compat(DIR_EAST, MOVE_BACKWARD, &dx, &dy);
    ok &= expect_int("east backward dx", dx, -1);
    ok &= expect_int("east backward dy", dy, 0);
    F0701_MOVEMENT_GetStepDelta_Compat(DIR_SOUTH, MOVE_LEFT, &dx, &dy);
    ok &= expect_int("south left dx", dx, 1);
    ok &= expect_int("south left dy", dy, 0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("forward key dispatches queued movement",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 1);
    ok &= expect_int("forward command dequeued", queueResult.dequeued, 1);
    ok &= expect_int("forward command dispatched move", queueResult.dispatchedMove, 1);
    ok &= expect_int("forward reaches target x", moveResult.newMapX, 2);
    ok &= expect_int("forward reaches target y", moveResult.newMapY, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("movement gate keeps movement queued",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 3, 0, 0, &queueResult, &moveResult), 0);
    ok &= expect_int("movement gate reported", queueResult.movementDisabledGate, 1);
    ok &= expect_int("movement gate does not dequeue", queueResult.dequeued, 0);
    ok &= expect_int("movement gate leaves queued command", (int)queue.count, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile gate blocks matching absolute forward",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 4, DIR_NORTH, &queueResult, &moveResult), 0);
    ok &= expect_int("projectile matching gate reported", queueResult.movementDisabledGate, 1);
    ok &= expect_int("projectile matching gate leaves queued command", (int)queue.count, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile gate allows nonmatching right",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB33, 0, 4, DIR_NORTH, &queueResult, &moveResult), 1);
    ok &= expect_int("right movement target x", moveResult.newMapX, 3);
    ok &= expect_int("right movement target y", moveResult.newMapY, 2);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("turn dequeues while movement gate active", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 9, 4, DIR_NORTH);
    ok &= expect_int("turn not movement-gated", queueResult.movementDisabledGate, 0);
    ok &= expect_int("turn dispatches", queueResult.dispatchedTurn, 1);
    ok &= expect_int("turn helper new direction", F0700_MOVEMENT_TurnDirection_Compat(DIR_NORTH, 1), DIR_EAST);

    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("wall target blocks forward",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 0);
    ok &= expect_int("wall block result code", moveResult.resultCode, MOVE_BLOCKED_WALL);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));

    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_DOOR, 2));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("closed door state blocks forward",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 0);
    ok &= expect_int("closed door block result code", moveResult.resultCode, MOVE_BLOCKED_DOOR);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_DOOR, 1));
    ok &= expect_int("one-fourth door passable", F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 2, 1), 1);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_DOOR, 5));
    ok &= expect_int("destroyed door passable", F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 2, 1), 1);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));

    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_FAKEWALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("closed real fakewall blocks forward",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 0);
    ok &= expect_int("closed real fakewall block result code", moveResult.resultCode, MOVE_BLOCKED_WALL);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_FAKEWALL, 0x04));
    ok &= expect_int("open fakewall passable", F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 2, 1), 1);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_FAKEWALL, 0x01));
    ok &= expect_int("imaginary fakewall passable", F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 2, 1), 1);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));

    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_PIT, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pit square passable by movement dispatch",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 1);
    ok &= expect_int("pit target result ok", moveResult.resultCode, MOVE_OK);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_TELEPORTER, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("teleporter square passable by movement dispatch",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 1);
    ok &= expect_int("teleporter target result ok", moveResult.resultCode, MOVE_OK);

    printf("dm1V1MovementCoreInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
