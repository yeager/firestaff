#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

/*
 * Focused DM1 V1 playable-route gate (Opus 4.6 heavy pass).
 *
 * Locks the minimum playable behaviour through the canonical
 * DUNGEON.DAT layout fix: from canonical start (map=0,x=1,y=3,dir=2)
 * the party can walk a real corridor route, blocked steps stay blocked,
 * turns change orientation only, and the live world hash + party state
 * advance with each legal command. Any one of these failing means DM1
 * V1 is not playable end-to-end.
 *
 * Source audit lock (ReDMCSB_WIP20210206/Toolchains/Common/Source):
 * - DUNGEON.C:30-44   G0233/G0234 direction-to-step deltas (N/E/W/S).
 * - DUNGEON.C:1370-1421 F0150 relative-movement coordinate update.
 * - DUNGEON.C:1423-1479 F0151 currentMapData[x][y] square access.
 * - CLIKMENU.C:180-347 F0366 step legality (wall blocks, door states,
 *   fakewall) and F0150 forward/right deltas.
 * - CLIKMENU.C:142-179 F0365 turn orientation only (no position change).
 * - MOVESENS.C:316-480 F0267 move result (CM3_MOVE_OK / CM5_MOVE_BLOCKED).
 * - MOVESENS.C:752-820 LastPartyMovementTime / sensor enter/leave on real
 *   square change.
 * - LOADSAVE.C:1997/2032 MEDIA529 DUNGEON.DAT layout (TextData *before*
 *   thing data) — locked into memory_dungeon_dat_pc34_compat.c.
 *
 * Canonical map 0 (18x19) at start, decoded with the corrected layout:
 *   y=3 row "0 5 0 0 1 0 1 1 1 0 0 0 0 1 1 1 0 0"
 *     - (1,3) = 5 teleporter  (party start)
 *     - (0,3) = 0 wall        (west blocked)
 *     - (2,3) = 0 wall        (east blocked)
 *   y=4 row "0 1 0 0 1 0 1 0 1 1 1 1 1 1 1 1 0 0"
 *     - (1,4) = 1 corridor    (south of start, walkable)
 *     - (0,4) = 0 wall        (west of (1,4) blocked)
 *   y=5 row "1 1 0 0 1 0 0 0 0 1 1 1 0 1 1 1 0 0"
 *     - (1,5) = 1 corridor    (further south, walkable)
 *     - (0,5) = 1 corridor    (west of (1,5), walkable)
 *
 * Route exercised: forward, forward, turn-right, forward.
 *   (1,3,SOUTH) -fwd-> (1,4,SOUTH) -fwd-> (1,5,SOUTH)
 *                      -turnR-> (1,5,WEST) -fwd-> (0,5,WEST).
 * In addition we exercise a true blocked step (turn-right from start
 * to face WEST and try forward into (0,3) wall).
 */

static const char* default_dm1_dungeon_dat(void)
{
    static char path[1024];
    const char* home = getenv("HOME");
    if (!home || home[0] == '\0') home = "/home/trv2";
    snprintf(path, sizeof(path), "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT", home);
    return path;
}

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_neq(const char* label, unsigned long got, unsigned long forbidden)
{
    if (got == forbidden) {
        fprintf(stderr, "FAIL %s want!=%lu got=%lu\n", label, forbidden, got);
        return 0;
    }
    return 1;
}

static int load_world(const char* dungeonPath, struct GameWorld_Compat* world)
{
    memset(world, 0, sizeof(*world));
    if (!F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, 0xF1A5u, world)) {
        fprintf(stderr, "FAIL load canonical dungeon path=%s\n", dungeonPath);
        return 0;
    }
    return 1;
}

static int run_command(struct Dm1V1MovementPipelinePc34Compat* pipeline,
                       struct GameWorld_Compat* world,
                       int command,
                       struct Dm1V1MovementPipelineResultPc34Compat* result)
{
    if (!DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(pipeline, command, 0, 0)) {
        fprintf(stderr, "FAIL enqueue cmd=%d\n", command);
        return 0;
    }
    DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(pipeline);
    if (!DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(pipeline, world->dungeon,
                                                          world->things, &world->party,
                                                          NULL, result)) {
        fprintf(stderr, "FAIL process cmd=%d\n", command);
        return 0;
    }
    world->gameTick++;
    return 1;
}

int main(int argc, char** argv)
{
    const char* dungeonPath = argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_CANONICAL_DUNGEON_DAT");
    struct GameWorld_Compat world;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    uint32_t hashStart = 0, hashAfterFwd1 = 0, hashAfterFwd2 = 0;
    uint32_t hashAfterTurn = 0, hashAfterFwd3 = 0, hashAfterBlocked = 0;
    int ok = 1;

    if (!dungeonPath || dungeonPath[0] == '\0') dungeonPath = default_dm1_dungeon_dat();

    printf("probe=firestaff_dm1_v1_playable_route_probe\n");
    printf("dungeon=%s\n", dungeonPath);
    printf("source=ReDMCSB_WIP20210206/Toolchains/Common/Source DUNGEON.C,F0150,F0151 "
           "CLIKMENU.C,F0365,F0366 MOVESENS.C,F0267 LOADSAVE.C,MEDIA529\n");

    /* ----- Phase 1: canonical start state ----- */
    if (!load_world(dungeonPath, &world)) return 1;
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    F0891_ORCH_WorldHash_Compat(&world, &hashStart);
    printf("[start] map=%d x=%d y=%d dir=%d hash=%u\n",
           world.party.mapIndex, world.party.mapX, world.party.mapY,
           world.party.direction, hashStart);
    ok &= expect_int("start map", world.party.mapIndex, 0);
    ok &= expect_int("start x", world.party.mapX, 1);
    ok &= expect_int("start y", world.party.mapY, 3);
    ok &= expect_int("start direction (south)", world.party.direction, 2);

    /* ----- Phase 2: legal forward south (1,3) -> (1,4) ----- */
    memset(&result, 0, sizeof(result));
    ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_MOVE_FORWARD, &result);
    F0891_ORCH_WorldHash_Compat(&world, &hashAfterFwd1);
    printf("[fwd1] resultCode=%d pos=(%d,%d,%d,%d) hash=%u blocked=%d anyMove=%d\n",
           result.core.movement.resultCode, world.party.mapIndex, world.party.mapX,
           world.party.mapY, world.party.direction, hashAfterFwd1,
           result.core.movementBlocked, result.anyMovementOccurred);
    ok &= expect_int("fwd1 not blocked", result.core.movementBlocked, 0);
    ok &= expect_int("fwd1 result ok", result.core.movement.resultCode, MOVE_OK);
    ok &= expect_int("fwd1 x stays 1", world.party.mapX, 1);
    ok &= expect_int("fwd1 y advanced to 4", world.party.mapY, 4);
    ok &= expect_int("fwd1 dir unchanged", world.party.direction, 2);
    ok &= expect_int("fwd1 anyMovement", result.anyMovementOccurred, 1);
    ok &= expect_neq("fwd1 hash differs from start", (unsigned long)hashAfterFwd1, (unsigned long)hashStart);

    /* ----- Phase 3: legal forward south (1,4) -> (1,5) ----- */
    memset(&result, 0, sizeof(result));
    ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_MOVE_FORWARD, &result);
    F0891_ORCH_WorldHash_Compat(&world, &hashAfterFwd2);
    printf("[fwd2] resultCode=%d pos=(%d,%d,%d,%d) hash=%u\n",
           result.core.movement.resultCode, world.party.mapIndex, world.party.mapX,
           world.party.mapY, world.party.direction, hashAfterFwd2);
    ok &= expect_int("fwd2 not blocked", result.core.movementBlocked, 0);
    ok &= expect_int("fwd2 x stays 1", world.party.mapX, 1);
    ok &= expect_int("fwd2 y advanced to 5", world.party.mapY, 5);
    ok &= expect_neq("fwd2 hash differs from fwd1",
                     (unsigned long)hashAfterFwd2, (unsigned long)hashAfterFwd1);

    /* ----- Phase 4: turn right at (1,5) -> face WEST (dir=3) ----- */
    memset(&result, 0, sizeof(result));
    ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_TURN_RIGHT, &result);
    F0891_ORCH_WorldHash_Compat(&world, &hashAfterTurn);
    printf("[turnR] turn=%d pos=(%d,%d,%d,%d) hash=%u\n",
           result.core.turnApplied, world.party.mapIndex, world.party.mapX,
           world.party.mapY, world.party.direction, hashAfterTurn);
    ok &= expect_int("turn applied", result.core.turnApplied, 1);
    ok &= expect_int("turn dir west", world.party.direction, 3);
    ok &= expect_int("turn x unchanged", world.party.mapX, 1);
    ok &= expect_int("turn y unchanged", world.party.mapY, 5);
    ok &= expect_neq("turn hash differs", (unsigned long)hashAfterTurn,
                     (unsigned long)hashAfterFwd2);

    /* ----- Phase 5: legal forward west (1,5) -> (0,5) ----- */
    memset(&result, 0, sizeof(result));
    ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_MOVE_FORWARD, &result);
    F0891_ORCH_WorldHash_Compat(&world, &hashAfterFwd3);
    printf("[fwd3] resultCode=%d pos=(%d,%d,%d,%d) hash=%u\n",
           result.core.movement.resultCode, world.party.mapIndex, world.party.mapX,
           world.party.mapY, world.party.direction, hashAfterFwd3);
    ok &= expect_int("fwd3 not blocked", result.core.movementBlocked, 0);
    ok &= expect_int("fwd3 result ok", result.core.movement.resultCode, MOVE_OK);
    ok &= expect_int("fwd3 x is 0", world.party.mapX, 0);
    ok &= expect_int("fwd3 y stays 5", world.party.mapY, 5);
    ok &= expect_int("fwd3 dir stays west", world.party.direction, 3);
    ok &= expect_neq("fwd3 hash differs", (unsigned long)hashAfterFwd3,
                     (unsigned long)hashAfterTurn);

    F0883_WORLD_Free_Compat(&world);

    /* ----- Phase 6: blocked step (fresh world, west wall at start) ----- */
    if (!load_world(dungeonPath, &world)) return 1;
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    {
        uint32_t hashBeforeBlocked = 0;
        F0891_ORCH_WorldHash_Compat(&world, &hashBeforeBlocked);

        /* Turn right -> face WEST */
        memset(&result, 0, sizeof(result));
        ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_TURN_RIGHT, &result);
        ok &= expect_int("blocked-prep turn west", world.party.direction, 3);

        /* Try forward west -> (0,3) wall, must remain at (1,3) */
        memset(&result, 0, sizeof(result));
        ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_MOVE_FORWARD, &result);
        F0891_ORCH_WorldHash_Compat(&world, &hashAfterBlocked);
        printf("[blocked] resultCode=%d pos=(%d,%d,%d,%d) blocked=%d anyMove=%d hash=%u\n",
               result.core.movement.resultCode, world.party.mapIndex, world.party.mapX,
               world.party.mapY, world.party.direction, result.core.movementBlocked,
               result.anyMovementOccurred, hashAfterBlocked);
        ok &= expect_int("blocked reports blocked", result.core.movementBlocked, 1);
        ok &= expect_int("blocked result wall", result.core.movement.resultCode, MOVE_BLOCKED_WALL);
        ok &= expect_int("blocked x unchanged", world.party.mapX, 1);
        ok &= expect_int("blocked y unchanged", world.party.mapY, 3);
        ok &= expect_int("blocked dir stays west", world.party.direction, 3);
        ok &= expect_int("blocked no movement", result.anyMovementOccurred, 0);
        (void)hashBeforeBlocked;
    }
    F0883_WORLD_Free_Compat(&world);

    printf("result=%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
