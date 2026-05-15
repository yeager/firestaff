#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

/*
 * Focused regression gate for DM1 V1 canonical DUNGEON.DAT startup movement.
 *
 * Source audit lock:
 * - ReDMCSB DUNGEON.C:35-44 G0233/G0234 direction deltas.
 * - ReDMCSB DUNGEON.C:1371-1421 F0150 relative movement coordinate update.
 * - ReDMCSB DUNGEON.C:1423-1479 F0151 map-square access via currentMapData[x][y].
 * - ReDMCSB CLIKMENU.C:180-347 F0366 step command blocker/cooldown path.
 * - ReDMCSB MOVESENS.C:316-850 F0267 move result / post-move handling.
 * - ReDMCSB GROUP.C creature-square checks audited for separate group blockers.
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

static int square_type(const struct GameWorld_Compat* world, int x, int y)
{
    const struct DungeonMapDesc_Compat* map;
    unsigned char square;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded) return -1;
    map = &world->dungeon->maps[0];
    if (x < 0 || y < 0 || x >= map->width || y >= map->height) return -1;
    square = world->dungeon->tiles[0].squareData[x * map->height + y];
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
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

int main(int argc, char** argv)
{
    const char* dungeonPath = argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_CANONICAL_DUNGEON_DAT");
    struct GameWorld_Compat world;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    int ok = 1;

    if (!dungeonPath || dungeonPath[0] == '\0') dungeonPath = default_dm1_dungeon_dat();

    printf("probe=firestaff_dm1_v1_canonical_movement_probe\n");
    printf("dungeon=%s\n", dungeonPath);
    printf("source=ReDMCSB_WIP20210206/Toolchains/Common/Source DUNGEON.C,F0150,F0151 CLIKMENU.C,F0366 MOVESENS.C,F0267 GROUP.C\n");

    if (!load_world(dungeonPath, &world)) return 1;
    printf("[map-load] start map=%d x=%d y=%d dir=%d startType=%d frontType=%d westType=%d map0=%dx%d\n",
           world.party.mapIndex, world.party.mapX, world.party.mapY, world.party.direction,
           square_type(&world, world.party.mapX, world.party.mapY),
           square_type(&world, world.party.mapX, world.party.mapY + 1),
           square_type(&world, world.party.mapX - 1, world.party.mapY),
           world.dungeon->maps[0].width, world.dungeon->maps[0].height);
    ok &= expect_int("initial map", world.party.mapIndex, 0);
    ok &= expect_int("initial x", world.party.mapX, 1);
    ok &= expect_int("initial y", world.party.mapY, 3);
    ok &= expect_int("initial direction", world.party.direction, 2);
    ok &= expect_int("front square passable", F0706_MOVEMENT_IsSquarePassable_Compat(world.dungeon, 0, 1, 4), 1);
    ok &= expect_int("west square blocked", F0706_MOVEMENT_IsSquarePassable_Compat(world.dungeon, 0, 0, 3), 0);
    F0883_WORLD_Free_Compat(&world);

    if (!load_world(dungeonPath, &world)) return 1;
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    ok &= expect_int("enqueue forward", DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(&pipeline, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0), 1);
    ok &= expect_int("process forward", DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(&pipeline, world.dungeon, world.things, &world.party, NULL, &result), 1);
    printf("[legal-step] handled=%d step=%d blocked=%d resultCode=%d new=(%d,%d,%d,%d) anyMove=%d cooldown=%d\n",
           result.core.commandHandled, result.core.stepApplied, result.core.movementBlocked,
           result.core.movement.resultCode, world.party.mapIndex, world.party.mapX,
           world.party.mapY, world.party.direction, result.anyMovementOccurred,
           pipeline.disabledMovementTicks);
    ok &= expect_int("forward not blocked", result.core.movementBlocked, 0);
    ok &= expect_int("forward result ok", result.core.movement.resultCode, MOVE_OK);
    ok &= expect_int("forward x", world.party.mapX, 1);
    ok &= expect_int("forward y", world.party.mapY, 4);
    ok &= expect_int("forward direction stays south", world.party.direction, 2);
    ok &= expect_int("forward movement occurred", result.anyMovementOccurred, 1);
    F0883_WORLD_Free_Compat(&world);

    if (!load_world(dungeonPath, &world)) return 1;
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    ok &= expect_int("enqueue turn left", DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(&pipeline, DM1_V1_COMMAND_TURN_LEFT, 0, 0), 1);
    ok &= expect_int("process turn left", DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(&pipeline, world.dungeon, world.things, &world.party, NULL, &result), 1);
    printf("[legal-turn] handled=%d turn=%d blocked=%d resultCode=%d pos=(%d,%d,%d,%d) anyTurn=%d\n",
           result.core.commandHandled, result.core.turnApplied, result.core.movementBlocked,
           result.core.movement.resultCode, world.party.mapIndex, world.party.mapX,
           world.party.mapY, world.party.direction, result.anyTurnOccurred);
    ok &= expect_int("turn left applied", result.core.turnApplied, 1);
    ok &= expect_int("turn left direction", world.party.direction, 1);
    ok &= expect_int("turn left x unchanged", world.party.mapX, 1);
    ok &= expect_int("turn left y unchanged", world.party.mapY, 3);
    F0883_WORLD_Free_Compat(&world);

    if (!load_world(dungeonPath, &world)) return 1;
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    ok &= expect_int("enqueue blocked right", DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(&pipeline, DM1_V1_COMMAND_MOVE_RIGHT, 0, 0), 1);
    ok &= expect_int("process blocked right", DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(&pipeline, world.dungeon, world.things, &world.party, NULL, &result), 1);
    printf("[blocked-step] handled=%d step=%d blocked=%d resultCode=%d pos=(%d,%d,%d,%d) anyMove=%d queue=%u\n",
           result.core.commandHandled, result.core.stepApplied, result.core.movementBlocked,
           result.core.movement.resultCode, world.party.mapIndex, world.party.mapX,
           world.party.mapY, world.party.direction, result.anyMovementOccurred, pipeline.commandQueue.count);
    ok &= expect_int("blocked step reports blocked", result.core.movementBlocked, 1);
    ok &= expect_int("blocked step result wall", result.core.movement.resultCode, MOVE_BLOCKED_WALL);
    ok &= expect_int("blocked x unchanged", world.party.mapX, 1);
    ok &= expect_int("blocked y unchanged", world.party.mapY, 3);
    ok &= expect_int("blocked direction unchanged", world.party.direction, 2);
    ok &= expect_int("blocked no movement occurred", result.anyMovementOccurred, 0);
    F0883_WORLD_Free_Compat(&world);

    printf("result=%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
