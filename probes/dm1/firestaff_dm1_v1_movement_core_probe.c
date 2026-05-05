#include <stdio.h>
#include <string.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_movement_command_core_pc34_compat.h"
#include "dm1_v1_movement_timing_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"

/*
 * Focused DM1 V1 movement command->execution probe.
 *
 * Primary source lock: ReDMCSB WIP20210206,
 * Toolchains/Common/Source.
 * - COMMAND.C:1379-1449 F0358_COMMAND_GetCommandFromMouseInput_CPSC maps
 *   mouse zone/button rows to commands; COMMAND.C:1452-1661 F0359 queues
 *   mouse commands/pending clicks; COMMAND.C:1692-1707 F0360 replays a
 *   pending click after unlock; COMMAND.C:1709-1813 F0361 queues keyboard
 *   commands; COMMAND.C:2045-2156 F0380 movement-gates/dequeues/dispatches.
 * - CLIKMENU.C:142-179 F0365 handles turns; CLIKMENU.C:180-347 F0366 handles
 *   steps, relative vectors, blockers, F0267 movement, and G0310 cooldown.
 * - CHAMPION.C:93-130 F0284 applies quarter-turn direction/cell deltas;
 *   CHAMPION.C:1180-1215 F0310 computes per-champion movement ticks.
 * - DUNGEON.C:1371-1421 F0150 converts forward/right counts to map deltas.
 * - MOVESENS.C:752-783 updates scent/G0362_l_LastPartyMovementTime on real
 *   party square changes; MOVESENS.C:799-820 triggers leave/enter sensors.
 * - GAMELOOP.C:150-155 decrements movement/projectile cooldowns once per loop;
 *   GAMELOOP.C:215-219 processes one queued command until input wait stops.
 */

#define MAP_W 4
#define MAP_H 4
#define BOOTS_OF_SPEED_ICON 194

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
    return (unsigned char)(((elementType & 7) << 5) | (attrs & DUNGEON_SQUARE_MASK_ATTRIBS));
}

static void reset_fixture(
    struct DungeonDatState_Compat* dungeon,
    struct DungeonMapDesc_Compat* map,
    struct DungeonMapTiles_Compat* tiles,
    struct DungeonThings_Compat* things,
    unsigned char squares[MAP_W * MAP_H],
    unsigned short squareFirstThings[MAP_W * MAP_H],
    struct PartyState_Compat* party)
{
    int i;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(things, 0, sizeof(*things));
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

    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = MAP_W * MAP_H;
    things->loaded = 1;

    party->mapIndex = 0;
    party->mapX = 1;
    party->mapY = 1;
    party->direction = DIR_NORTH;
    party->championCount = 2;
    party->champions[0].hp.current = 25;
    party->champions[0].load = 40;
    party->champions[0].maxLoad = 100;
    party->champions[0].direction = DIR_NORTH;
    party->champions[1].hp.current = 35;
    party->champions[1].load = 100;    /* BUG0_72: load==maxLoad uses overloaded path. */
    party->champions[1].maxLoad = 100;
    party->champions[1].direction = DIR_NORTH;
}

static int enqueue_key(struct Dm1V1InputCommandQueuePc34Compat* queue, int keyCode)
{
    return DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
        queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, keyCode, 0, 0, 0 });
}

static void log_command_structure(void)
{
    printf("\n[command-structure]\n");
    printf("queuedCommandFields=command,x,y source=DEFS.H:197-205 COMMAND.C:1651-1660\n");
    printf("queueFields=commands[5],count,locked,pendingClick*,pendingReplayCount,droppedFullCount source=COMMAND.C:1452-1661,1692-1707,2045-2156\n");
    printf("partyFields=mapIndex,mapX,mapY,direction,champions[].direction source=CHAMPION.C:93-130 TOWNSGLB.H:551\n");
    printf("speedStepTimingFields=G0310_i_DisabledMovementTicks,G0311_i_ProjectileDisabledMovementTicks,G0312_i_LastProjectileDisabledMovementDirection,G0362_l_LastPartyMovementTime source=COMMAND.C:2095-2100 CLIKMENU.C:330-346 GAMELOOP.C:150-155 MOVESENS.C:772-774\n");
    printf("turnRateFields=oldDirection,newDirection,delta,quarterTurnSteps,animationFrames,intermediateFrames source=CLIKMENU.C:171-173 CHAMPION.C:117-130 GAMELOOP.C:90,215-219\n");
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    unsigned char squares[MAP_W * MAP_H];
    unsigned short squareFirstThings[MAP_W * MAP_H];
    struct PartyState_Compat party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputQueueProcessResultPc34Compat q;
    struct Dm1V1MovementCommandCoreResultPc34Compat core;
    int footwear[CHAMPION_MAX_PARTY] = { 0, 0, 0, 0 };
    int disabled;
    int projectileDisabled;
    int frame;
    int ok = 1;

    printf("probe=firestaff_dm1_v1_movement_core_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence.commandQueue=%s\n", DM1_V1_InputCommandQueue_SourceEvidencePc34Compat());
    printf("sourceEvidence.commandCore=%s\n", DM1_V1_MovementCommandCore_SourceEvidencePc34Compat());
    printf("sourceEvidence.timing=%s\n", DM1_V1_MovementTiming_SourceEvidencePc34Compat());
    printf("sourceEvidence.turning=%s\n", m11_v1_turning_presentation_source_evidence_pc34_compat());
    printf("compileStructureRefs=FTL.H:5 FTL.H:21 PRIM.H:46\n");
    log_command_structure();

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, &party);
    printf("\n[input-pipeline:key->queue->step]\n");
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("forward key enqueues", enqueue_key(&queue, 0xAB35), 1);
    ok &= expect_int("queue count after forward key", (int)queue.count, 1);
    ok &= expect_int("core forward processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 1000ul, 990ul, footwear, &core), 1);
    printf("input=key:0xAB35 command=%d dequeued=%d dispatchedMove=%d source=(COMMAND.C:1709-1813 -> COMMAND.C:2045-2156 -> CLIKMENU.C:180-347)\n",
        core.queue.command, core.queue.dequeued, core.queue.dispatchedMove);
    printf("stepDistanceCells dx=%d dy=%d old=(%d,%d) new=(%d,%d) direction=%d stepApplied=%d\n",
        core.movement.newMapX - core.sourceMapX,
        core.movement.newMapY - core.sourceMapY,
        core.sourceMapX, core.sourceMapY,
        party.mapX, party.mapY,
        party.direction,
        core.stepApplied);
    printf("timing disabledMovementTicks=%d projectileDisabledMovementTicks=%d scentRecorded=%d scentDelayTicks=%d lastPartyMovementTime=%lu\n",
        core.timing.disabledMovementTicks,
        core.timing.projectileDisabledMovementTicks,
        core.timing.scentRecorded,
        core.timing.scentDelayTicks,
        core.timing.lastPartyMovementTime);
    ok &= expect_int("forward reaches x", party.mapX, 1);
    ok &= expect_int("forward reaches y", party.mapY, 0);
    ok &= expect_int("one-cell step", core.movement.newMapY - core.sourceMapY, -1);
    ok &= expect_int("slowest champion sets 4 tick step delay", core.timing.disabledMovementTicks, 4);
    ok &= expect_int("successful step requests redraw", core.viewportRedrawRequested, 1);

    printf("\n[frame-sync:cooldown-decrement]\n");
    disabled = core.timing.disabledMovementTicks;
    projectileDisabled = 3;
    for (frame = 1; frame <= 4; ++frame) {
        DM1_V1_MovementTiming_DecrementCooldownsPc34Compat(&disabled, &projectileDisabled);
        printf("frame=%d disabledMovementTicks=%d projectileDisabledMovementTicks=%d source=GAMELOOP.C:150-155\n",
            frame, disabled, projectileDisabled);
    }
    ok &= expect_int("disabled cooldown reaches zero after four loops", disabled, 0);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, &party);
    printf("\n[input-pipeline:movement-gate-keeps-command]\n");
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("gated forward key enqueues", enqueue_key(&queue, 0xAB35), 1);
    q = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 3, 0, 0);
    printf("disabledGateResult command=%d movementDisabledGate=%d dequeued=%d remainingQueue=%u source=COMMAND.C:2095-2100\n",
        q.command, q.movementDisabledGate, q.dequeued, queue.count);
    ok &= expect_int("movement gate reported", q.movementDisabledGate, 1);
    ok &= expect_int("movement gate leaves command queued", (int)queue.count, 1);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, &party);
    printf("\n[input-pipeline:projectile-direction-gate]\n");
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile forward key enqueues", enqueue_key(&queue, 0xAB35), 1);
    q = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 7, DIR_NORTH);
    printf("projectileSameDirection command=%d movementDisabledGate=%d dequeued=%d remainingQueue=%u normalizedDirection=%d source=COMMAND.C:2095-2096\n",
        q.command, q.movementDisabledGate, q.dequeued, queue.count,
        (party.direction + q.command - DM1_V1_COMMAND_MOVE_FORWARD) & 3);
    ok &= expect_int("projectile same direction gates movement", q.movementDisabledGate, 1);
    ok &= expect_int("projectile gate leaves command queued", (int)queue.count, 1);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, &party);
    printf("\n[input-pipeline:mouse-pending-click-replay]\n");
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    ok &= expect_int("locked mouse event becomes pending", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
        &queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 270, 130, DM1_V1_BUTTON_LEFT }), 0);
    printf("pendingClickPresent=%d pendingX=%d pendingY=%d pendingCommand=%d source=COMMAND.C:1452-1505\n",
        queue.pendingClickPresent, queue.pendingClickX, queue.pendingClickY, queue.pendingClickCommand);
    q = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 0, 0);
    printf("afterUnlockReplay pendingReplayCount=%d queueCount=%u source=COMMAND.C:1692-1707,2087-2100,2126-2127\n",
        q.pendingReplayCount, queue.count);
    q = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, party.direction, 0, 0, 0);
    printf("mouseCommand command=%d dequeued=%d dispatchedMove=%d source=COMMAND.C:1379-1449,396-405\n",
        q.command, q.dequeued, q.dispatchedMove);
    ok &= expect_int("mouse replay maps to move forward", q.command, DM1_V1_COMMAND_MOVE_FORWARD);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, &party);
    printf("\n[turning:instant-quarter-turn]\n");
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("turn right key enqueues", enqueue_key(&queue, 0xAB36), 1);
    ok &= expect_int("core turn processed despite movement gate", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 9, 4, DIR_NORTH, 2000ul, 1990ul, footwear, &core), 1);
    printf("turn command=%d oldDirection=%d newDirection=%d delta=%d quarterTurnSteps=%d animationFrames=%d intermediateFrames=%d stopWait=%d redrawNextLoop=%d wallBlockCheck=%d\n",
        core.queue.command,
        core.turning.oldDirection,
        core.turning.newDirection,
        core.turning.delta,
        core.turning.quarterTurnSteps,
        core.turning.animationFrames,
        core.turning.intermediateFrames,
        core.turning.stopWaitingForPlayerInput,
        core.turning.redrawOnNextGameLoop,
        core.turning.wallBlockCheck);
    ok &= expect_int("turn bypasses movement gate", core.queue.movementDisabledGate, 0);
    ok &= expect_int("turn changes direction to east", party.direction, DIR_EAST);
    ok &= expect_int("turn is one quarter step", core.turning.quarterTurnSteps, 1);
    ok &= expect_int("turn has no intermediate frames", core.turning.intermediateFrames, 0);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, &party);
    printf("\n[stepping:blocked-wall-no-side-effects]\n");
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_WALL, 0);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("wall forward key enqueues", enqueue_key(&queue, 0xAB35), 1);
    ok &= expect_int("core wall processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 3000ul, 2990ul, footwear, &core), 1);
    printf("wallBlock command=%d dequeued=%d movementBlocked=%d resultCode=%d inputDiscardRequested=%d viewportRedraw=%d stopWait=%d old=(%d,%d) newParty=(%d,%d) cooldownApplied=%d\n",
        core.queue.command,
        core.queue.dequeued,
        core.movementBlocked,
        core.movement.resultCode,
        core.inputDiscardRequested,
        core.viewportRedrawRequested,
        core.stopWaitingForPlayerInput,
        core.sourceMapX, core.sourceMapY,
        party.mapX, party.mapY,
        core.timing.disabledMovementTicks);
    ok &= expect_int("wall blocks movement", core.movementBlocked, 1);
    ok &= expect_int("wall result code", core.movement.resultCode, MOVE_BLOCKED_WALL);
    ok &= expect_int("blocked path keeps x", party.mapX, 1);
    ok &= expect_int("blocked path keeps y", party.mapY, 1);
    ok &= expect_int("blocked path does not set movement cooldown", core.timing.disabledMovementTicks, 0);
    ok &= expect_int("blocked path requests input discard", core.inputDiscardRequested, 1);

    reset_fixture(&dungeon, &map, &tiles, &things, squares, squareFirstThings, &party);
    printf("\n[stepping:closed-door-block]\n");
    squares[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_DOOR, 2);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("door forward key enqueues", enqueue_key(&queue, 0xAB35), 1);
    ok &= expect_int("core door processed", DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
        &queue, &dungeon, &things, &party, 0, 0, 0, 3010ul, 3000ul, footwear, &core), 1);
    printf("doorBlock doorState=2 movementBlocked=%d resultCode=%d old=(%d,%d) newParty=(%d,%d) source=CLIKMENU.C:282-284\n",
        core.movementBlocked, core.movement.resultCode, core.sourceMapX, core.sourceMapY, party.mapX, party.mapY);
    ok &= expect_int("closed door blocks movement", core.movementBlocked, 1);
    ok &= expect_int("closed door result code", core.movement.resultCode, MOVE_BLOCKED_DOOR);

    printf("\n[summary]\n");
    printf("movementCoreProbeOk=%u\n", ok ? 1u : 0u);
    printf("primaryLocks=COMMAND.C:F0358/F0359/F0360/F0361/F0380;CLIKMENU.C:F0365/F0366;CHAMPION.C:F0284/F0310;DUNGEON.C:F0150;MOVESENS.C:F0267/F0276;GAMELOOP.C:cooldown+process-loop;DEFS.H/TOWNSGLB.H structs+globals\n");
    return ok ? 0 : 1;
}
