#include <stdio.h>
#include <string.h>

#include "dm1_v1_movement_command_core_pc34_compat.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_contains(const char* label, const char* haystack, const char* needle)
{
    if (!haystack || !needle || !strstr(haystack, needle)) {
        fprintf(stderr, "FAIL %s missing substring: %s\n", label, needle ? needle : "(null)");
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
    int x;
    int y;
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
    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            set_square(squares, height, x, y, square_type(DUNGEON_ELEMENT_CORRIDOR, 0));
        }
    }
}

static void setup_party(struct PartyState_Compat* party)
{
    memset(party, 0, sizeof(*party));
    party->mapIndex = 0;
    party->mapX = 2;
    party->mapY = 2;
    party->direction = DIR_NORTH;
    party->championCount = 2;
    party->champions[0].present = 1;
    party->champions[0].hp.current = 10;
    party->champions[0].maxLoad = 100;
    party->champions[0].stamina.current = 100;
    party->champions[0].stamina.maximum = 100;
    party->champions[0].direction = DIR_NORTH;
    party->champions[1].present = 1;
    party->champions[1].hp.current = 10;
    party->champions[1].maxLoad = 100;
    party->champions[1].stamina.current = 100;
    party->champions[1].stamina.maximum = 100;
    party->champions[1].direction = DIR_EAST;
}

static unsigned short make_thing_ref(int type, int index, int cell)
{
    return (unsigned short)(((cell & 3) << 14) | ((type & 15) << 10) | (index & 0x03FF));
}

static void setup_single_text_sensor(
    struct DungeonThings_Compat* things,
    unsigned short* firstThings,
    struct DungeonSensor_Compat* sensors,
    int textIndex)
{
    memset(things, 0, sizeof(*things));
    memset(sensors, 0, sizeof(*sensors));
    firstThings[0] = make_thing_ref(THING_TYPE_SENSOR, 0, 0);
    sensors[0].next = THING_ENDOFLIST;
    sensors[0].sensorType = 13;
    sensors[0].sensorData = (unsigned short)textIndex;
    things->loaded = 1;
    things->squareFirstThings = firstThings;
    things->squareFirstThingCount = 1;
    things->sensors = sensors;
    things->sensorCount = 1;
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[25];
    struct DungeonThings_Compat things;
    struct PartyState_Compat party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1MovementCommandCoreResultPc34Compat result;
    int footwear[CHAMPION_MAX_PARTY] = { 0, 0, 0, 0 };
    int ok = 1;

    const char* sourceEvidence = DM1_V1_MovementCommandCore_SourceEvidencePc34Compat();

    printf("probe=dm1_v1_movement_command_core_pc34_compat\n");
    printf("sourceEvidence=%s\n", sourceEvidence);
    ok &= expect_contains("source evidence command queue dispatch", sourceEvidence, "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2075-2099");
    ok &= expect_contains("source evidence move party stamina", sourceEvidence, "CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty:237-255");
    ok &= expect_contains("source evidence stamina clamp", sourceEvidence, "CHAMPION.C:F0325_CHAMPION_DecrementStamina:2025-2048");
    ok &= expect_contains("source evidence projectile direction gate", sourceEvidence, "blocks projectile cooldown only when G0312 matches normalized absolute movement direction");
    ok &= expect_contains("source evidence move party blockers", sourceEvidence, "224-233 arrow deltas");
    ok &= expect_contains("source evidence relative movement", sourceEvidence, "DUNGEON.C:F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:1389-1391");
    ok &= expect_contains("source evidence party rotation", sourceEvidence, "CHAMPION.C:F0284_CHAMPION_SetPartyDirection:117-130");
    ok &= expect_contains("source evidence move result", sourceEvidence, "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:316-328");
    ok &= expect_contains("source evidence move result globals", sourceEvidence, "738-741 move-result globals");
    ok &= expect_contains("source evidence movement scent", sourceEvidence, "752-783 party-square/scent/last-movement update");

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue turn right", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    ok &= expect_int("process turn", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 200, 100, footwear, &result), 1);
    ok &= expect_int("turn handled", result.commandHandled, 1);
    ok &= expect_int("turn applied", result.turnApplied, 1);
    ok &= expect_int("turn is no step", result.stepApplied, 0);
    ok &= expect_int("party direction rotates north to east", party.direction, DIR_EAST);
    ok &= expect_int("champion0 direction rotates with party", party.champions[0].direction, DIR_EAST);
    ok &= expect_int("champion1 direction rotates with party", party.champions[1].direction, DIR_SOUTH);
    ok &= expect_int("turn requests viewport redraw", result.viewportRedrawRequested, 1);
    ok &= expect_int("turn releases input wait", result.stopWaitingForPlayerInput, 1);
    ok &= expect_int("turn does not set movement cooldown", result.timing.disabledMovementTicks, 0);
    ok &= expect_int("turn does not decrement stamina", party.champions[0].stamina.current, 100);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue forward step", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("process forward step", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 260, 200, footwear, &result), 1);
    ok &= expect_int("step handled", result.commandHandled, 1);
    ok &= expect_int("step applied", result.stepApplied, 1);
    ok &= expect_int("forward x unchanged", party.mapX, 2);
    ok &= expect_int("forward y decremented", party.mapY, 1);
    ok &= expect_int("step requests viewport redraw", result.viewportRedrawRequested, 1);
    ok &= expect_int("step releases input wait", result.stopWaitingForPlayerInput, 1);
    ok &= expect_int("step records scent/last movement time", result.timing.scentRecorded, 1);
    ok &= expect_int("step updates last movement time", (int)result.timing.lastPartyMovementTime, 260);
    ok &= expect_int("step clears projectile cooldown", result.timing.projectileDisabledMovementTicks, 0);
    ok &= expect_int("step decrements living champion0 stamina before resolution", party.champions[0].stamina.current, 99);
    ok &= expect_int("step decrements living champion1 stamina before resolution", party.champions[1].stamina.current, 99);
    ok &= expect_int("step records stamina affected count", result.staminaAffectedCount, 2);
    ok &= expect_int("step records stamina cost", result.staminaCost[0], 1);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue blocked forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("enqueue queued trailing command", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    ok &= expect_int("process blocked step", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 320, 260, footwear, &result), 1);
    ok &= expect_int("blocked movement reported", result.movementBlocked, 1);
    ok &= expect_int("blocked input discard requested", result.inputDiscardRequested, 1);
    ok &= expect_int("blocked command does not release wait", result.stopWaitingForPlayerInput, 0);
    ok &= expect_int("blocked command does not redraw viewport", result.viewportRedrawRequested, 0);
    ok &= expect_int("blocked movement leaves x", party.mapX, 2);
    ok &= expect_int("blocked movement leaves y", party.mapY, 2);
    ok &= expect_int("blocked movement still decrements stamina", party.champions[0].stamina.current, 99);
    ok &= expect_int("blocked movement records stamina cost", result.staminaCost[0], 1);
    ok &= expect_int("blocked movement flushes queued input", (int)queue.count, 0);
    ok &= expect_int("blocked wall requests self damage seam", result.blockedByWallOrDoorDamageRequested, 1);
    ok &= expect_int("blocked wall damage attack is one", result.blockedByWallOrDoorDamageAttack, 1);
    ok &= expect_int("blocked wall damage attack type self", result.blockedByWallOrDoorDamageAttackTypeSelf, 2);
    ok &= expect_int("blocked wall damage wounds torso legs", result.blockedByWallOrDoorDamageAllowedWounds, 0x0018);
    ok &= expect_int("blocked forward north first target cell", result.blockedByWallOrDoorDamageFirstCell, DIR_SOUTH);
    ok &= expect_int("blocked forward north second target cell", result.blockedByWallOrDoorDamageSecondCell, DIR_WEST);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    party.championCount = CHAMPION_MAX_PARTY + 3;
    party.champions[0].load = 100;
    party.champions[0].maxLoad = 100;
    party.champions[0].stamina.current = 1;
    party.champions[0].stamina.maximum = 100;
    party.champions[1].hp.current = 0;
    party.champions[1].load = 100;
    party.champions[1].maxLoad = 100;
    party.champions[1].stamina.current = 50;
    party.champions[1].stamina.maximum = 100;
    party.champions[2].present = 1;
    party.champions[2].hp.current = 10;
    party.champions[2].load = 0;
    party.champions[2].maxLoad = 0;
    party.champions[2].stamina.current = 120;
    party.champions[2].stamina.maximum = 100;
    party.champions[3].present = 1;
    party.champions[3].hp.current = 10;
    party.champions[3].load = 200;
    party.champions[3].maxLoad = 100;
    party.champions[3].stamina.current = 100;
    party.champions[3].stamina.maximum = 100;
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue stamina bounds step", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("process stamina bounds step", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 330, 300, footwear, &result), 1);
    ok &= expect_int("stamina loop clamps to champion array window", result.staminaAffectedCount, CHAMPION_MAX_PARTY - 1);
    ok &= expect_int("stamina underflow clamps to zero", party.champions[0].stamina.current, 0);
    ok &= expect_int("stamina underflow records damage", result.staminaDamage[0], 1);
    ok &= expect_int("stamina underflow applies damage", party.champions[0].hp.current, 9);
    ok &= expect_int("dead champion skipped by stamina window", party.champions[1].stamina.current, 50);
    ok &= expect_int("dead champion records no stamina cost", result.staminaCost[1], 0);
    ok &= expect_int("max load zero still costs one", result.staminaCost[2], 1);
    ok &= expect_int("stamina clamps down to maximum after decrement", party.champions[2].stamina.current, 100);
    ok &= expect_int("overloaded champion cost uses load/maxLoad ratio", result.staminaCost[3], 7);
    ok &= expect_int("stamina bounds step still redraws viewport", result.viewportRedrawRequested, 1);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 core left arrow enqueues turn", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004B, 0, 0, 0 }), 1);
    ok &= expect_int("pc34 core left arrow turn bypasses movement gate", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 9, 4, DIR_NORTH, 340, 320, footwear, &result), 1);
    ok &= expect_int("pc34 core left arrow dequeued", result.queue.dequeued, 1);
    ok &= expect_int("pc34 core left arrow dispatches turn", result.queue.dispatchedTurn, 1);
    ok &= expect_int("pc34 core left arrow handled", result.commandHandled, 1);
    ok &= expect_int("pc34 core left arrow rotates west", party.direction, DIR_WEST);
    ok &= expect_int("pc34 core left arrow leaves position x", party.mapX, 2);
    ok &= expect_int("pc34 core left arrow leaves position y", party.mapY, 2);
    ok &= expect_int("pc34 core left arrow leaves no cooldown", result.timing.disabledMovementTicks, 0);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 core up arrow enqueues forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    ok &= expect_int("pc34 core up arrow processes forward", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 360, 340, footwear, &result), 1);
    ok &= expect_int("pc34 core up arrow dequeued", result.queue.dequeued, 1);
    ok &= expect_int("pc34 core up arrow dispatches move", result.queue.dispatchedMove, 1);
    ok &= expect_int("pc34 core up arrow applies step", result.stepApplied, 1);
    ok &= expect_int("pc34 core up arrow y decremented", party.mapY, 1);
    ok &= expect_int("pc34 core up arrow sets cooldown", result.timing.disabledMovementTicks, 2);
    ok &= expect_int("pc34 core up arrow clears projectile cooldown", result.timing.projectileDisabledMovementTicks, 0);
    ok &= expect_int("pc34 core up arrow decrements stamina", party.champions[0].stamina.current, 99);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 core disabled gate queues forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    ok &= expect_int("pc34 core disabled gate processes without dequeue", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 3, 0, 0, 370, 350, footwear, &result), 1);
    ok &= expect_int("pc34 core disabled gate observed", result.queue.movementDisabledGate, 1);
    ok &= expect_int("pc34 core disabled gate keeps command queued", (int)queue.count, 1);
    ok &= expect_int("pc34 core disabled gate does not dispatch move", result.queue.dispatchedMove, 0);
    ok &= expect_int("pc34 core disabled gate does not step", result.stepApplied, 0);
    ok &= expect_int("pc34 core disabled gate keeps x", party.mapX, 2);
    ok &= expect_int("pc34 core disabled gate keeps y", party.mapY, 2);

    ok &= expect_int("pc34 core projectile gate also holds same-direction move", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 2, DIR_NORTH, 372, 350, footwear, &result), 1);
    ok &= expect_int("pc34 core projectile gate observed", result.queue.movementDisabledGate, 1);
    ok &= expect_int("pc34 core projectile gate keeps command queued", (int)queue.count, 1);

    /* Pass542 lane A: ReDMCSB F0380 unlocks and replays one pending click
     * after the movement-disabled gate returns, before any movement command is
     * dequeued.  The held C003 step must stay at the front; the replayed
     * secondary movement click appends behind it and is not allowed to turn the
     * party until the step cooldown gate clears.
     */
    queue.locked = 1;
    ok &= expect_int("pass542 pending click captured during movement gate",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 300, 130, DM1_V1_BUTTON_LEFT }), 0);
    ok &= expect_int("pass542 pending click command is turn right", queue.pendingClickCommand, DM1_V1_COMMAND_TURN_RIGHT);
    queue.locked = 0;
    ok &= expect_int("pass542 cooldown gate with pending click", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 2, DIR_NORTH, 373, 350, footwear, &result), 1);
    ok &= expect_int("pass542 cooldown gate did not dequeue held step", result.queue.dequeued, 0);
    ok &= expect_int("pass542 cooldown gate replayed pending once", result.queue.pendingReplayCount, 1);
    ok &= expect_int("pass542 cooldown gate keeps held step plus replay", (int)queue.count, 2);
    ok &= expect_int("pass542 held step remains queue front", queue.commands[0].command, DM1_V1_COMMAND_MOVE_FORWARD);
    ok &= expect_int("pass542 replayed click appends as turn right", queue.commands[1].command, DM1_V1_COMMAND_TURN_RIGHT);
    ok &= expect_int("pass542 cooldown gate leaves direction unchanged", party.direction, DIR_NORTH);
    ok &= expect_int("pass542 cooldown gate leaves y unchanged", party.mapY, 2);

    ok &= expect_int("pc34 core cooldown expiry releases held move", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, DIR_NORTH, 374, 350, footwear, &result), 1);
    ok &= expect_int("pc34 core cooldown expiry dequeues", result.queue.dequeued, 1);
    ok &= expect_int("pc34 core cooldown expiry applies step", result.stepApplied, 1);
    ok &= expect_int("pc34 core cooldown expiry decrements y", party.mapY, 1);
    ok &= expect_int("pass542 replayed turn waits behind released step", (int)queue.count, 1);
    ok &= expect_int("pass542 replayed turn still queued", queue.commands[0].command, DM1_V1_COMMAND_TURN_RIGHT);


    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 core projectile nonmatching direction queues forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    ok &= expect_int("pc34 core projectile nonmatching direction processes move", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 2, DIR_EAST, 373, 350, footwear, &result), 1);
    ok &= expect_int("pc34 core projectile nonmatching direction not gated", result.queue.movementDisabledGate, 0);
    ok &= expect_int("pc34 core projectile nonmatching direction dequeues", result.queue.dequeued, 1);
    ok &= expect_int("pc34 core projectile nonmatching direction dispatches move", result.queue.dispatchedMove, 1);
    ok &= expect_int("pc34 core projectile nonmatching direction applies step", result.stepApplied, 1);
    ok &= expect_int("pc34 core projectile nonmatching direction clears projectile cooldown", result.timing.projectileDisabledMovementTicks, 0);
    ok &= expect_int("pc34 core projectile nonmatching direction consumes queue", (int)queue.count, 0);
    ok &= expect_int("pc34 core projectile nonmatching direction leaves x", party.mapX, 2);
    ok &= expect_int("pc34 core projectile nonmatching direction decrements y", party.mapY, 1);
    ok &= expect_int("pc34 core projectile nonmatching direction sets cooldown", result.timing.disabledMovementTicks, 2);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 core blocked up arrow queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    ok &= expect_int("pc34 core blocked followup queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
    ok &= expect_int("pc34 core blocked up arrow processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 380, 360, footwear, &result), 1);
    ok &= expect_int("pc34 core blocked up arrow dequeued", result.queue.dequeued, 1);
    ok &= expect_int("pc34 core blocked up arrow reports wall", result.movement.resultCode, MOVE_BLOCKED_WALL);
    ok &= expect_int("pc34 core blocked up arrow discards followup", (int)queue.count, 0);
    ok &= expect_int("pc34 core blocked up arrow skips turn", result.turnApplied, 0);
    ok &= expect_int("pc34 core blocked up arrow keeps y", party.mapY, 2);
    ok &= expect_int("pc34 core blocked up arrow damage request", result.blockedByWallOrDoorDamageRequested, 1);
    ok &= expect_int("pc34 core blocked up arrow first cell", result.blockedByWallOrDoorDamageFirstCell, DIR_SOUTH);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    party.championCount = 0;
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 core empty party blocked wall queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    ok &= expect_int("pc34 core empty party blocked wall processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 390, 380, footwear, &result), 1);
    ok &= expect_int("pc34 core empty party has no damage request", result.blockedByWallOrDoorDamageRequested, 0);

    /* Pass549: passable door states are accepted movement, not blocked
     * collision.  Source lock: CLIKMENU.C:282-284 blocks a door only when
     * M036_DOOR_STATE is neither C0_OPEN, C1_CLOSED_ONE_FOURTH, nor
     * C5_DESTROYED.  Accepted door movement then reaches the normal
     * CLIKMENU.C:325-346 / MOVESENS.C:F0267 success path.
     */
    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    setup_party(&party);
    {
        unsigned short firstThings[1];
        struct DungeonSensor_Compat sensors[1];
        setup_single_text_sensor(&things, firstThings, sensors, 53);
        set_square(squares, 5, 2, 1,
            square_type(DUNGEON_ELEMENT_DOOR, 1 | DUNGEON_SQUARE_MASK_THING_LIST));
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("pass549 one-fourth door front command queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
        ok &= expect_int("pass549 one-fourth door trailing turn queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
        ok &= expect_int("pass549 one-fourth door processed",
            DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
                &queue, &dungeon, &things, &party, 0, 0, 0, 410, 400, footwear, &result), 1);
        ok &= expect_int("pass549 one-fourth door is accepted movement", result.stepApplied, 1);
        ok &= expect_int("pass549 one-fourth door not blocked", result.movementBlocked, 0);
        ok &= expect_int("pass549 one-fourth door result ok", result.movement.resultCode, MOVE_OK);
        ok &= expect_int("pass549 one-fourth door moves into target x", party.mapX, 2);
        ok &= expect_int("pass549 one-fourth door moves into target y", party.mapY, 1);
        ok &= expect_int("pass549 one-fourth door keeps trailing queued input", (int)queue.count, 1);
        ok &= expect_int("pass549 one-fourth door releases input wait", result.stopWaitingForPlayerInput, 1);
        ok &= expect_int("pass549 one-fourth door requests viewport", result.viewportRedrawRequested, 1);
        ok &= expect_int("pass549 one-fourth door has no damage request", result.blockedByWallOrDoorDamageRequested, 0);
        ok &= expect_int("pass549 one-fourth door destination sensor", result.enterEffects.count, 1);
        ok &= expect_int("pass549 one-fourth door sensor text", result.enterEffects.effects[0].textIndex, 53);
        ok &= expect_int("pass549 one-fourth door sets cooldown", result.timing.disabledMovementTicks, 2);
        ok &= expect_int("pass549 one-fourth door records scent", result.timing.scentRecorded, 1);
        ok &= expect_int("pass549 one-fourth door updates last movement", (int)result.timing.lastPartyMovementTime, 410);
        ok &= expect_int("pass549 one-fourth door clears projectile cooldown", result.timing.projectileDisabledMovementTicks, 0);
    }

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    setup_party(&party);
    {
        unsigned short firstThings[1];
        struct DungeonSensor_Compat sensors[1];
        setup_single_text_sensor(&things, firstThings, sensors, 54);
        set_square(squares, 5, 2, 1,
            square_type(DUNGEON_ELEMENT_DOOR, 5 | DUNGEON_SQUARE_MASK_THING_LIST));
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("pass549 destroyed door front command queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
        ok &= expect_int("pass549 destroyed door processed",
            DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
                &queue, &dungeon, &things, &party, 0, 0, 0, 420, 410, footwear, &result), 1);
        ok &= expect_int("pass549 destroyed door is accepted movement", result.stepApplied, 1);
        ok &= expect_int("pass549 destroyed door not blocked", result.movementBlocked, 0);
        ok &= expect_int("pass549 destroyed door result ok", result.movement.resultCode, MOVE_OK);
        ok &= expect_int("pass549 destroyed door moves into target y", party.mapY, 1);
        ok &= expect_int("pass549 destroyed door releases input wait", result.stopWaitingForPlayerInput, 1);
        ok &= expect_int("pass549 destroyed door requests viewport", result.viewportRedrawRequested, 1);
        ok &= expect_int("pass549 destroyed door has no damage request", result.blockedByWallOrDoorDamageRequested, 0);
        ok &= expect_int("pass549 destroyed door destination sensor", result.enterEffects.count, 1);
        ok &= expect_int("pass549 destroyed door sensor text", result.enterEffects.effects[0].textIndex, 54);
        ok &= expect_int("pass549 destroyed door sets cooldown", result.timing.disabledMovementTicks, 2);
        ok &= expect_int("pass549 destroyed door records scent", result.timing.scentRecorded, 1);
        ok &= expect_int("pass549 destroyed door updates last movement", (int)result.timing.lastPartyMovementTime, 420);
    }


    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    memset(&things, 0, sizeof(things));
    setup_party(&party);
    set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, 0));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pass544 blocked collision front move queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    ok &= expect_int("pass544 blocked collision trailing turn queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
    ok &= expect_int("pass544 blocked collision reserved release queued",
        DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(&queue, DM1_V1_COMMAND_RELEASE_CHAMPION_ICON, 11, 22), 1);
    queue.locked = 1;
    ok &= expect_int("pass544 blocked collision pending stop captured",
        DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
            &queue, DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL, 12, 23, DM1_V1_BUTTON_LEFT), 0);
    ok &= expect_int("pass544 blocked collision pending present before dispatch", queue.pendingClickPresent, 1);
    ok &= expect_int("pass544 blocked collision processed after cooldown clear", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 400, 390, footwear, &result), 1);
    ok &= expect_int("pass544 blocked collision move dequeued before F0366", result.queue.dequeued, 1);
    ok &= expect_int("pass544 blocked collision dispatched to move handler", result.queue.dispatchedMove, 1);
    ok &= expect_int("pass544 blocked collision reports wall", result.movement.resultCode, MOVE_BLOCKED_WALL);
    ok &= expect_int("pass544 blocked collision reports movement blocked", result.movementBlocked, 1);
    ok &= expect_int("pass544 blocked collision flush requested", result.inputDiscardRequested, 1);
    ok &= expect_int("pass544 blocked collision one blocked vblank requested", result.blockedMovementVblankWaitRequested, 1);
    ok &= expect_int("pass544 blocked collision no successful step cooldown", result.timing.disabledMovementTicks, 0);
    ok &= expect_int("pass544 blocked collision keeps input wait armed", result.stopWaitingForPlayerInput, 0);
    ok &= expect_int("pass544 blocked collision pending replayed once", (int)queue.pendingReplayCount, 1);
    ok &= expect_int("pass544 blocked collision pending cleared", queue.pendingClickPresent, 0);
    ok &= expect_int("pass544 blocked collision keeps only reserved commands", (int)queue.count, 2);
    ok &= expect_int("pass544 blocked collision drops nonreserved trailing turn", queue.commands[0].command, DM1_V1_COMMAND_RELEASE_CHAMPION_ICON);
    ok &= expect_int("pass544 blocked collision preserves pending stop command", queue.commands[1].command, DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL);
    ok &= expect_int("pass544 blocked collision release x preserved", queue.commands[0].x, 11);
    ok &= expect_int("pass544 blocked collision pending stop x preserved", queue.commands[1].x, 12);

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    setup_party(&party);
    {
        unsigned short firstThings[1];
        struct DungeonSensor_Compat sensors[1];
        set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST));
        setup_single_text_sensor(&things, firstThings, sensors, 37);
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("pass545 successful movement front command queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
        ok &= expect_int("pass545 successful movement trailing turn retained source queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
        ok &= expect_int("pass545 successful movement processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 0, 0, 0, 420, 400, footwear, &result), 1);
        ok &= expect_int("pass545 successful movement dequeued only front command", result.queue.dequeued, 1);
        ok &= expect_int("pass545 successful movement applied step", result.stepApplied, 1);
        ok &= expect_int("pass545 successful movement retains trailing command", (int)queue.count, 1);
        ok &= expect_int("pass545 successful movement trailing command is turn right", queue.commands[0].command, DM1_V1_COMMAND_TURN_RIGHT);
        ok &= expect_int("pass545 successful movement no discard requested", result.inputDiscardRequested, 0);
        ok &= expect_int("pass545 successful movement leaves source sensor effects empty", result.leaveEffects.count, 0);
        ok &= expect_int("pass545 successful movement destination sensor fires once", result.enterEffects.count, 1);
        ok &= expect_int("pass545 successful movement destination sensor kind text", result.enterEffects.effects[0].kind, SENSOR_EFFECT_SHOW_TEXT);
        ok &= expect_int("pass545 successful movement destination sensor text index", result.enterEffects.effects[0].textIndex, 37);
        ok &= expect_int("pass545 successful movement sets cooldown after sensors", result.timing.disabledMovementTicks, 2);
    }

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    setup_party(&party);
    {
        unsigned short firstThings[1];
        struct DungeonSensor_Compat sensors[1];
        set_square(squares, 5, 2, 1, square_type(DUNGEON_ELEMENT_WALL, DUNGEON_SQUARE_MASK_THING_LIST));
        setup_single_text_sensor(&things, firstThings, sensors, 41);
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("pass545 blocked movement front command queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
        ok &= expect_int("pass545 blocked movement trailing turn queued", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
            (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
        ok &= expect_int("pass546 blocked movement reserved release queued",
            DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(&queue, DM1_V1_COMMAND_RELEASE_CHAMPION_ICON, 31, 42), 1);
        queue.locked = 1;
        ok &= expect_int("pass546 blocked movement pending stop captured",
            DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
                &queue, DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL, 32, 43, DM1_V1_BUTTON_LEFT), 0);
        ok &= expect_int("pass546 blocked movement pending present before dispatch", queue.pendingClickPresent, 1);
        ok &= expect_int("pass545 blocked movement processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
            &queue, &dungeon, &things, &party, 0, 0, 0, 430, 420, footwear, &result), 1);
        ok &= expect_int("pass545 blocked movement dequeued front command", result.queue.dequeued, 1);
        ok &= expect_int("pass545 blocked movement reports blocked", result.movementBlocked, 1);
        ok &= expect_int("pass545 blocked movement discards trailing command but preserves reserved", (int)queue.count, 2);
        ok &= expect_int("pass546 blocked movement pending replayed once", (int)queue.pendingReplayCount, 1);
        ok &= expect_int("pass546 blocked movement pending cleared", queue.pendingClickPresent, 0);
        ok &= expect_int("pass546 blocked movement preserves release command", queue.commands[0].command, DM1_V1_COMMAND_RELEASE_CHAMPION_ICON);
        ok &= expect_int("pass546 blocked movement preserves pending stop command", queue.commands[1].command, DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL);
        ok &= expect_int("pass546 blocked movement release x preserved", queue.commands[0].x, 31);
        ok &= expect_int("pass546 blocked movement pending stop x preserved", queue.commands[1].x, 32);
        ok &= expect_int("pass545 blocked movement requests discard", result.inputDiscardRequested, 1);
        ok &= expect_int("pass545 blocked movement leaves sensor effects empty", result.leaveEffects.count, 0);
        ok &= expect_int("pass545 blocked movement destination sensor not fired", result.enterEffects.count, 0);
        ok &= expect_int("pass545 blocked movement no cooldown after blocked", result.timing.disabledMovementTicks, 0);
    }


    /* Pass547: blocked door/fakewall/group command lifecycle.
     * Source lock: CLIKMENU.C:282-288 marks closed doors and closed real
     * fakewalls as movement blockers; CLIKMENU.C:311-313 marks groups as
     * blockers and requests the party-adjacent reaction.  All three blocked
     * paths converge through CLIKMENU.C:317-323, discarding input and doing the
     * PC-34 VBlank wait while leaving G0321_B_StopWaitingForPlayerInput false
     * and never reaching the successful-step cooldown/sensor path at
     * CLIKMENU.C:325-346 / MOVESENS.C:799-818.
     */
    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    setup_party(&party);
    {
        unsigned short firstThings[1];
        struct DungeonSensor_Compat sensors[1];
        setup_single_text_sensor(&things, firstThings, sensors, 51);
        set_square(squares, 5, 2, 1,
            square_type(DUNGEON_ELEMENT_DOOR, 2 | DUNGEON_SQUARE_MASK_THING_LIST));
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("pass547 closed door front command queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
        ok &= expect_int("pass547 closed door trailing turn queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
        ok &= expect_int("pass547 closed door processed",
            DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
                &queue, &dungeon, &things, &party, 0, 0, 0, 440, 430, footwear, &result), 1);
        ok &= expect_int("pass547 closed door reports door block", result.movement.resultCode, MOVE_BLOCKED_DOOR);
        ok &= expect_int("pass547 closed door movement blocked", result.movementBlocked, 1);
        ok &= expect_int("pass547 closed door damage request", result.blockedByWallOrDoorDamageRequested, 1);
        ok &= expect_int("pass590 closed door damage attack is one", result.blockedByWallOrDoorDamageAttack, 1);
        ok &= expect_int("pass590 closed door damage attack type self", result.blockedByWallOrDoorDamageAttackTypeSelf, 2);
        ok &= expect_int("pass590 closed door damage wounds torso legs", result.blockedByWallOrDoorDamageAllowedWounds, 0x0018);
        ok &= expect_int("pass590 closed door damage first cell", result.blockedByWallOrDoorDamageFirstCell, DIR_SOUTH);
        ok &= expect_int("pass590 closed door damage second cell", result.blockedByWallOrDoorDamageSecondCell, DIR_WEST);
        ok &= expect_int("pass547 closed door discards trailing input", (int)queue.count, 0);
        ok &= expect_int("pass547 closed door leaves input wait armed", result.stopWaitingForPlayerInput, 0);
        ok &= expect_int("pass547 closed door no cooldown", result.timing.disabledMovementTicks, 0);
        ok &= expect_int("pass547 closed door no sensor effects", result.leaveEffects.count + result.enterEffects.count, 0);
        ok &= expect_int("pass547 closed door no group reaction", result.groupReactionPartyAdjacentRequested, 0);
    }

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    setup_party(&party);
    {
        unsigned short firstThings[1];
        struct DungeonSensor_Compat sensors[1];
        setup_single_text_sensor(&things, firstThings, sensors, 52);
        set_square(squares, 5, 2, 1,
            square_type(DUNGEON_ELEMENT_FAKEWALL, DUNGEON_SQUARE_MASK_THING_LIST));
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("pass547 closed fakewall front command queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
        ok &= expect_int("pass547 closed fakewall trailing turn queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
        ok &= expect_int("pass547 closed fakewall processed",
            DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
                &queue, &dungeon, &things, &party, 0, 0, 0, 450, 440, footwear, &result), 1);
        ok &= expect_int("pass547 closed fakewall reports wall block", result.movement.resultCode, MOVE_BLOCKED_WALL);
        ok &= expect_int("pass547 closed fakewall damage request", result.blockedByWallOrDoorDamageRequested, 1);
        ok &= expect_int("pass590 closed fakewall damage attack is one", result.blockedByWallOrDoorDamageAttack, 1);
        ok &= expect_int("pass590 closed fakewall damage attack type self", result.blockedByWallOrDoorDamageAttackTypeSelf, 2);
        ok &= expect_int("pass590 closed fakewall damage wounds torso legs", result.blockedByWallOrDoorDamageAllowedWounds, 0x0018);
        ok &= expect_int("pass590 closed fakewall damage first cell", result.blockedByWallOrDoorDamageFirstCell, DIR_SOUTH);
        ok &= expect_int("pass590 closed fakewall damage second cell", result.blockedByWallOrDoorDamageSecondCell, DIR_WEST);
        ok &= expect_int("pass547 closed fakewall discards trailing input", (int)queue.count, 0);
        ok &= expect_int("pass547 closed fakewall leaves input wait armed", result.stopWaitingForPlayerInput, 0);
        ok &= expect_int("pass547 closed fakewall no cooldown", result.timing.disabledMovementTicks, 0);
        ok &= expect_int("pass547 closed fakewall no sensor effects", result.leaveEffects.count + result.enterEffects.count, 0);
        ok &= expect_int("pass547 closed fakewall no group reaction", result.groupReactionPartyAdjacentRequested, 0);
    }

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    setup_party(&party);
    {
        unsigned short firstThings[1];
        struct DungeonGroup_Compat groups[1];
        memset(&things, 0, sizeof(things));
        memset(groups, 0, sizeof(groups));
        set_square(squares, 5, 2, 1,
            square_type(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST));
        firstThings[0] = make_thing_ref(THING_TYPE_GROUP, 0, 0);
        groups[0].next = THING_ENDOFLIST;
        things.loaded = 1;
        things.squareFirstThings = firstThings;
        things.squareFirstThingCount = 1;
        things.groups = groups;
        things.groupCount = 1;
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        ok &= expect_int("pass547 group front command queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
        ok &= expect_int("pass547 group trailing turn queued",
            DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
        ok &= expect_int("pass547 group processed",
            DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
                &queue, &dungeon, &things, &party, 0, 0, 0, 460, 450, footwear, &result), 1);
        ok &= expect_int("pass547 group movement blocked", result.movementBlocked, 1);
        ok &= expect_int("pass547 group marked", result.blockedByGroup, 1);
        ok &= expect_int("pass547 group reaction requested", result.groupReactionPartyAdjacentRequested, 1);
        ok &= expect_int("pass547 group no wall damage request", result.blockedByWallOrDoorDamageRequested, 0);
        ok &= expect_int("pass547 group discards trailing input", (int)queue.count, 0);
        ok &= expect_int("pass547 group leaves input wait armed", result.stopWaitingForPlayerInput, 0);
        ok &= expect_int("pass547 group no cooldown", result.timing.disabledMovementTicks, 0);
        ok &= expect_int("pass547 group no sensor effects", result.leaveEffects.count + result.enterEffects.count, 0);
        ok &= expect_int("pass547 group keeps source x", party.mapX, 2);
        ok &= expect_int("pass547 group keeps source y", party.mapY, 2);
    }


    printf("dm1V1MovementCommandCoreInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
