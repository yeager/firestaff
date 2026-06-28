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
 * - CLIKMENU.C:291-318 preserves the empty-party bug, then checks
 *   F0175_GROUP_GetThing on otherwise-passable target squares and blocks
 *   non-empty parties before F0267 side effects/cooldowns.
 * - CLIKMENU.C:317-328 returns on blocked movement before calling F0267;
 *   MOVESENS.C:438-443 is where accepted party movement first mutates
 *   G0306/G0307, so wall/door/fakewall/group blockers must leave party
 *   coordinates unchanged and skip pit/teleporter/sensor side effects.
 * - MOVESENS.C:493-518 F0267 requires an open party-scoped teleporter,
 *   switches to TargetMapIndex/TargetMapX/TargetMapY, then applies absolute
 *   or relative party rotation.
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

static int expect_blocked_move_kept_party_state(
    const char* label,
    const struct PartyState_Compat* party,
    const struct MovementResult_Compat* result)
{
    int ok = 1;
    ok &= expect_int(label, result->resultCode != MOVE_OK && result->resultCode != MOVE_TURN_ONLY, 1);
    ok &= expect_int("blocked move keeps x", result->newMapX, party->mapX);
    ok &= expect_int("blocked move keeps y", result->newMapY, party->mapY);
    ok &= expect_int("blocked move keeps map", result->newMapIndex, party->mapIndex);
    ok &= expect_int("blocked move keeps direction", result->newDirection, party->direction);
    return ok;
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

static void fill_corridor_map(unsigned char* squares, int width, int height)
{
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

static int expect_door_state_dispatch_contract(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    const struct DungeonDatState_Compat* dungeon,
    const struct PartyState_Compat* party,
    unsigned char* squares,
    int height)
{
    static const int doorStates[] = { 0, 1, 2, 3, 4, 5 };
    int ok = 1;
    size_t i;

    /* ReDMCSB: CLIKMENU.C F0366 lines 282-284 allows door states
     * C0 open, C1 closed-one-fourth, and C5 destroyed; states C2..C4
     * block before MOVESENS.C F0267 mutates the party coordinates. */
    for (i = 0; i < sizeof(doorStates) / sizeof(doorStates[0]); ++i) {
        const int doorState = doorStates[i];
        const int passable = (doorState == 0 || doorState == 1 || doorState == 5);
        struct Dm1V1InputQueueProcessResultPc34Compat queueResult;
        struct MovementResult_Compat moveResult;

        set_square(squares, height, 2, 1,
            square_type(DUNGEON_ELEMENT_DOOR, doorState));
        DM1_V1_InputCommandQueue_InitPc34Compat(queue);
        ok &= expect_int("door-state key dispatch",
            process_key_and_try_move(queue, dungeon, party, 0xAB35, 0, 0, 0,
                &queueResult, &moveResult), passable);
        ok &= expect_int("door-state command dequeued", queueResult.dequeued, 1);
        ok &= expect_int("door-state command dispatched", queueResult.dispatchedMove, 1);
        if (passable) {
            ok &= expect_int("door-state passable result code",
                moveResult.resultCode, MOVE_OK);
            ok &= expect_int("door-state passable target x", moveResult.newMapX, 2);
            ok &= expect_int("door-state passable target y", moveResult.newMapY, 1);
            ok &= expect_int("door-state passable keeps direction",
                moveResult.newDirection, party->direction);
        } else {
            ok &= expect_int("door-state blocked result code",
                moveResult.resultCode, MOVE_BLOCKED_DOOR);
            ok &= expect_blocked_move_kept_party_state(
                "door-state blocked skips accepted-move side effects",
                party, &moveResult);
        }
    }

    set_square(squares, height, 2, 1,
        square_type(DUNGEON_ELEMENT_CORRIDOR, 0));
    return ok;
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
    struct DungeonThings_Compat things;
    unsigned short squareFirstThings[1];
    struct DungeonGroup_Compat groups[1];
    int dx;
    int dy;
    int ok = 1;

    printf("probe=dm1_v1_movement_core_pc34_compat\n");
    printf("sourceEvidence=COMMAND.C:2045-2156; CLIKMENU.C:180-347,224-233,278-288,291-318,317-328; DUNGEON.C:1371-1391; MOVESENS.C:272-310,438-443,493-518; PROJEXPL.C:459\n");

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(groups, 0, sizeof(groups));
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
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
    ok &= expect_blocked_move_kept_party_state("wall block skips accepted-move side effects", &party, &moveResult);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));

    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_DOOR, 2));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("closed door state blocks forward",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 0);
    ok &= expect_int("closed door block result code", moveResult.resultCode, MOVE_BLOCKED_DOOR);
    ok &= expect_blocked_move_kept_party_state("closed door block skips accepted-move side effects", &party, &moveResult);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_DOOR, 1));
    ok &= expect_int("one-fourth door passable", F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 2, 1), 1);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_DOOR, 5));
    ok &= expect_int("destroyed door passable", F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 2, 1), 1);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));

    ok &= expect_door_state_dispatch_contract(
        &queue, &dungeon, &party, squares, 5);

    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_FAKEWALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("closed real fakewall blocks forward",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 0);
    ok &= expect_int("closed real fakewall block result code", moveResult.resultCode, MOVE_BLOCKED_WALL);
    ok &= expect_blocked_move_kept_party_state("closed real fakewall block skips accepted-move side effects", &party, &moveResult);
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

    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST));
    squareFirstThings[0] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    groups[0].next = THING_ENDOFLIST;
    ok &= expect_int("group on passable target blocks non-empty party",
        F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(&dungeon, &things, &party, MOVE_FORWARD), 1);
    party.championCount = 0;
    ok &= expect_int("empty party preserves source group-collision bug",
        F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(&dungeon, &things, &party, MOVE_FORWARD), 0);
    party.championCount = 1;
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, DUNGEON_SQUARE_MASK_THING_LIST));
    ok &= expect_int("impassable target skips group collision gate",
        F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(&dungeon, &things, &party, MOVE_FORWARD), 0);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("structural wall with thing-list still blocks before move result",
        process_key_and_try_move(&queue, &dungeon, &party, 0xAB35, 0, 0, 0, &queueResult, &moveResult), 0);
    ok &= expect_blocked_move_kept_party_state("structural blocker skips move-result chain", &party, &moveResult);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));
    squareFirstThings[0] = THING_ENDOFLIST;

    /* MOVESENS.C:F0267:493-518 party teleporter chain:
     * an open teleporter must have MASK0x0002_SCOPE_PARTY, then the party
     * position is changed to the target tuple and relative rotation is added
     * to the incoming party direction.  A creature-only teleporter on the
     * same square is a precise negative control for the scope gate. */
    {
        struct DungeonMapDesc_Compat teleporterMaps[2];
        struct DungeonMapTiles_Compat teleporterTiles[2];
        unsigned char teleporterMap0[9];
        unsigned char teleporterMap1[16];
        unsigned short teleporterFirstThings[25];
        struct DungeonTeleporter_Compat teleporters[1];
        struct PartyState_Compat teleporterParty;
        struct PostMoveResolution_Compat postMove;

        memset(&dungeon, 0, sizeof(dungeon));
        memset(teleporterMaps, 0, sizeof(teleporterMaps));
        memset(teleporterTiles, 0, sizeof(teleporterTiles));
        memset(teleporterFirstThings, 0xFF, sizeof(teleporterFirstThings));
        memset(&things, 0, sizeof(things));
        memset(teleporters, 0, sizeof(teleporters));
        fill_corridor_map(teleporterMap0, 3, 3);
        fill_corridor_map(teleporterMap1, 4, 4);
        set_square(teleporterMap0, 3, 1, 1,
            square_type(DUNGEON_ELEMENT_TELEPORTER,
                DUNGEON_SQUARE_MASK_THING_LIST | 0x08));
        teleporterMaps[0].width = 3;
        teleporterMaps[0].height = 3;
        teleporterMaps[1].width = 4;
        teleporterMaps[1].height = 4;
        teleporterTiles[0].squareData = teleporterMap0;
        teleporterTiles[0].squareCount = 9;
        teleporterTiles[1].squareData = teleporterMap1;
        teleporterTiles[1].squareCount = 16;
        dungeon.header.mapCount = 2;
        dungeon.maps = teleporterMaps;
        dungeon.tiles = teleporterTiles;
        dungeon.loaded = 1;
        dungeon.tilesLoaded = 1;
        teleporterFirstThings[1 * 3 + 1] = (unsigned short)((THING_TYPE_TELEPORTER << 10) | 0);
        things.loaded = 1;
        things.squareFirstThings = teleporterFirstThings;
        things.squareFirstThingCount = 25;
        things.teleporters = teleporters;
        things.teleporterCount = 1;
        teleporters[0].next = THING_ENDOFLIST;
        teleporters[0].targetMapIndex = 1;
        teleporters[0].targetMapX = 3;
        teleporters[0].targetMapY = 2;
        teleporters[0].rotation = 1;
        teleporters[0].absoluteRotation = 0;
        teleporters[0].scope = 0x02;
        teleporters[0].audible = 1;
        memset(&teleporterParty, 0, sizeof(teleporterParty));
        teleporterParty.mapIndex = 0;
        teleporterParty.mapX = 1;
        teleporterParty.mapY = 1;
        teleporterParty.direction = DIR_EAST;
        teleporterParty.championCount = 1;

        ok &= expect_int("party-scoped teleporter resolves",
            F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
                &dungeon, &things, &teleporterParty, 0, &postMove), 1);
        ok &= expect_int("party-scoped teleporter transitioned", postMove.transitioned, 1);
        ok &= expect_int("party-scoped teleporter count", postMove.teleporterCount, 1);
        ok &= expect_int("party-scoped audible teleporter count",
            postMove.teleporterAudibleCount, 1);
        ok &= expect_int("party-scoped teleporter target map", postMove.finalMapIndex, 1);
        ok &= expect_int("party-scoped teleporter target x", postMove.finalMapX, 3);
        ok &= expect_int("party-scoped teleporter target y", postMove.finalMapY, 2);
        ok &= expect_int("relative teleporter rotation east plus one is south",
            postMove.finalDirection, DIR_SOUTH);

        teleporters[0].audible = 0;
        ok &= expect_int("silent party teleporter resolves",
            F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
                &dungeon, &things, &teleporterParty, 0, &postMove), 1);
        ok &= expect_int("silent party teleporter transitioned", postMove.transitioned, 1);
        ok &= expect_int("silent party teleporter has no audible count",
            postMove.teleporterAudibleCount, 0);
        ok &= expect_int("silent party teleporter target map", postMove.finalMapIndex, 1);
        ok &= expect_int("silent party teleporter target x", postMove.finalMapX, 3);
        ok &= expect_int("silent party teleporter target y", postMove.finalMapY, 2);
        ok &= expect_int("silent party teleporter keeps relative rotation",
            postMove.finalDirection, DIR_SOUTH);

        teleporters[0].audible = 1;
        teleporters[0].scope = 0x01;
        ok &= expect_int("creature-only teleporter scope still resolves call",
            F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat(
                &dungeon, &things, &teleporterParty, 0, &postMove), 1);
        ok &= expect_int("creature-only teleporter does not transition party",
            postMove.transitioned, 0);
        ok &= expect_int("creature-only teleporter has no audible count",
            postMove.teleporterAudibleCount, 0);
        ok &= expect_int("creature-only teleporter leaves map", postMove.finalMapIndex, 0);
        ok &= expect_int("creature-only teleporter leaves x", postMove.finalMapX, 1);
        ok &= expect_int("creature-only teleporter leaves y", postMove.finalMapY, 1);
        ok &= expect_int("creature-only teleporter leaves direction",
            postMove.finalDirection, DIR_EAST);

        setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
        memset(&things, 0, sizeof(things));
        things.loaded = 1;
        things.squareFirstThings = squareFirstThings;
        things.squareFirstThingCount = 1;
        things.groups = groups;
        things.groupCount = 1;
    }

    /* MOVESENS.C:272-310 / F0266 intermediary projectile-impact cell maps.
     * Exact source comment case: adjacent east move with a champion in cell 2
     * must populate intermediary cell 3 so a destination-square projectile in
     * cell 3 can hit instead of being passed through. */
    {
        unsigned char sourceCells[4] = { 0, 0, 3, 0 };
        unsigned char destinationCells[4] = { 0, 0, 0, 0 };
        unsigned char intermediaryCells[4] = { 9, 9, 9, 9 };
        unsigned char northSourceCells[4] = { 1, 2, 0, 0 };
        int checkDestination;

        checkDestination = F0709_MOVEMENT_BuildIntermediaryProjectileImpactCells_Compat(
            5, 5, 6, 5, sourceCells, destinationCells, intermediaryCells);
        ok &= expect_int("intermediary east move requests destination projectile impact check", checkDestination, 1);
        ok &= expect_int("MOVESENS documented cell2 crosses through intermediary cell3", intermediaryCells[3], 3);
        ok &= expect_int("other intermediary cells stay empty", intermediaryCells[0] + intermediaryCells[1] + intermediaryCells[2], 0);
        ok &= expect_int("destination primary east cell remains empty", destinationCells[1], 0);
        ok &= expect_int("destination secondary south cell keeps source occupant", destinationCells[2], 3);

        checkDestination = F0709_MOVEMENT_BuildIntermediaryProjectileImpactCells_Compat(
            5, 5, 5, 4, northSourceCells, destinationCells, intermediaryCells);
        ok &= expect_int("intermediary north move requests destination projectile impact check", checkDestination, 1);
        ok &= expect_int("north primary source cell0 maps to intermediary cell3", intermediaryCells[3], 1);
        ok &= expect_int("north secondary source cell1 maps to intermediary cell2", intermediaryCells[2], 2);
        ok &= expect_int("north destination front cells retain occupied source cells", destinationCells[0] + destinationCells[1], 3);

        checkDestination = F0709_MOVEMENT_BuildIntermediaryProjectileImpactCells_Compat(
            5, 5, 7, 5, sourceCells, destinationCells, intermediaryCells);
        ok &= expect_int("non-adjacent movement skips intermediary projectile check", checkDestination, 0);
        ok &= expect_int("non-adjacent intermediary cells clear", intermediaryCells[0] + intermediaryCells[1] + intermediaryCells[2] + intermediaryCells[3], 0);
    }

    printf("dm1V1MovementCoreInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
