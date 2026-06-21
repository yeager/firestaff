#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

/*
 * Pass1055 Firestaff-side pair for the original PC 3.4 closed-door capture.
 *
 * The original DOSBox pass1055 route reaches a closed Hall-area door, then
 * records byte-identical viewport frames before and after a viewport click plus
 * a forward keypress. This probe replays the same movement key sequence through
 * the DM1 V1 movement pipeline and proves the Firestaff state reaches a closed
 * door that blocks the next forward command without changing party position.
 *
 * Source lock:
 * - COMMAND.C F0380 dispatches C001/C002 turns and C003..C006 movement.
 * - CLIKMENU.C F0365 handles turns; F0366 blocks wall/closed-door movement.
 * - MOVESENS.C F0267 mutates party position only for accepted movement.
 */

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static const char* default_dm1_dungeon_dat(void)
{
    static char path[1024];
    const char* home = getenv("HOME");
    if (!home || home[0] == '\0') home = "/home/trv2";
    snprintf(path, sizeof(path),
             "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT",
             home);
    return path;
}

static int load_world(const char* dungeonPath, struct GameWorld_Compat* world)
{
    memset(world, 0, sizeof(*world));
    if (!F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, 0x1055u, world)) {
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
        fprintf(stderr, "FAIL enqueue command=%d\n", command);
        return 0;
    }
    DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(pipeline);
    if (!DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(pipeline,
                                                          world->dungeon,
                                                          world->things,
                                                          &world->party,
                                                          NULL,
                                                          result)) {
        fprintf(stderr, "FAIL process command=%d\n", command);
        return 0;
    }
    world->gameTick++;
    return 1;
}

static unsigned char read_square(const struct DungeonDatState_Compat* dungeon,
                                 int mapIndex,
                                 int x,
                                 int y)
{
    const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
    return dungeon->tiles[mapIndex].squareData[x * (int)map->height + y];
}

static int square_type(unsigned char square)
{
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

int main(int argc, char** argv)
{
    const char* dungeonPath =
        argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_CANONICAL_DUNGEON_DAT");
    struct GameWorld_Compat world;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    int ok = 1;

    /*
     * Tokens from
     * verification-screens/pass1055-dm1-original-closed-door-collision/
     * original-viewpoint-route-keys.log.
     *
     * Mapping: kp5=forward, kp4=turn-left, kp6=turn-right.
     */
    static const int pass1055Route[] = {
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_LEFT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_LEFT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_LEFT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_LEFT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
        DM1_V1_COMMAND_MOVE_FORWARD,
    };

    if (!dungeonPath || dungeonPath[0] == '\0') dungeonPath = default_dm1_dungeon_dat();

    printf("probe=firestaff_dm1_v1_pass1055_closed_door_pair_probe\n");
    printf("dungeon=%s\n", dungeonPath);
    printf("source=COMMAND.C:F0380 CLIKMENU.C:F0365/F0366 MOVESENS.C:F0267\n");

    if (!load_world(dungeonPath, &world)) return 1;
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    ok &= expect_int("start.map", world.party.mapIndex, 0);
    ok &= expect_int("start.x", world.party.mapX, 1);
    ok &= expect_int("start.y", world.party.mapY, 3);
    ok &= expect_int("start.dir.south", world.party.direction, 2);

    for (unsigned i = 0; i < sizeof(pass1055Route) / sizeof(pass1055Route[0]); ++i) {
        memset(&result, 0, sizeof(result));
        ok &= run_command(&pipeline, &world, pass1055Route[i], &result);
        if (!ok) break;
    }

    {
        int beforeMap = world.party.mapIndex;
        int beforeX = world.party.mapX;
        int beforeY = world.party.mapY;
        int beforeDir = world.party.direction;
        int dx = 0;
        int dy = 0;
        int targetX = 0;
        int targetY = 0;
        unsigned char targetSquare = 0;

        F0701_MOVEMENT_GetStepDelta_Compat(beforeDir, MOVE_FORWARD, &dx, &dy);
        targetX = beforeX + dx;
        targetY = beforeY + dy;
        targetSquare = read_square(world.dungeon, beforeMap, targetX, targetY);

        printf("[before_closed_door] map=%d x=%d y=%d dir=%d target=(%d,%d) square=0x%02x type=%d passable=%d\n",
               beforeMap,
               beforeX,
               beforeY,
               beforeDir,
               targetX,
               targetY,
               targetSquare,
               square_type(targetSquare),
               F0706_MOVEMENT_IsSquarePassable_Compat(world.dungeon,
                                                       beforeMap,
                                                       targetX,
                                                       targetY));

        ok &= expect_int("target.type.door", square_type(targetSquare), DUNGEON_ELEMENT_DOOR);
        ok &= expect_int("target.not.passable",
                         F0706_MOVEMENT_IsSquarePassable_Compat(world.dungeon,
                                                                 beforeMap,
                                                                 targetX,
                                                                 targetY),
                         0);

        memset(&result, 0, sizeof(result));
        ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_MOVE_FORWARD, &result);

        printf("[after_forward_into_closed_door] resultCode=%d blocked=%d anyMove=%d pos=(%d,%d,%d,%d)\n",
               result.core.movement.resultCode,
               result.core.movementBlocked,
               result.anyMovementOccurred,
               world.party.mapIndex,
               world.party.mapX,
               world.party.mapY,
               world.party.direction);

        ok &= expect_int("blocked.result.door",
                         result.core.movement.resultCode,
                         MOVE_BLOCKED_DOOR);
        ok &= expect_int("blocked.flag", result.core.movementBlocked, 1);
        ok &= expect_int("blocked.no.movement", result.anyMovementOccurred, 0);
        ok &= expect_int("blocked.map.unchanged", world.party.mapIndex, beforeMap);
        ok &= expect_int("blocked.x.unchanged", world.party.mapX, beforeX);
        ok &= expect_int("blocked.y.unchanged", world.party.mapY, beforeY);
        ok &= expect_int("blocked.dir.unchanged", world.party.direction, beforeDir);
    }

    F0883_WORLD_Free_Compat(&world);
    printf("result=%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
