#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_collision_door_pc34_compat.h"
#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

/*
 * Extended Firestaff-side collision matrix for the pass1055 closed-door route.
 *
 * This is not an original-DOS pixel or transcript claim. It reuses the
 * canonical DM1 PC 3.4 DUNGEON.DAT and the same movement pipeline seam as
 * pass1055, then swaps the target square in front of the party to prove the
 * source-owned element decoder distinguishes wall, door, fakewall, pit, and
 * teleporter outcomes.
 *
 * Source lock:
 *   - COMMAND.C F0380 dispatches queued C001/C002 turns and C003..C006 steps.
 *   - CLIKMENU.C F0366 rejects walls, closed doors, and closed real fakewalls
 *     before F0267 side effects.
 *   - MOVESENS.C F0267 accepts pits/teleporters as consequence squares; later
 *     post-move handling owns fall/teleport side effects.
 *   - DUNGEON.C F0172 defines square element bits and door/fakewall low bits.
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
    if (!home || home[0] == '\0') {
        home = "/home/trv2";
    }
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

static void write_square(struct DungeonDatState_Compat* dungeon,
                         int mapIndex,
                         int x,
                         int y,
                         unsigned char value)
{
    const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
    dungeon->tiles[mapIndex].squareData[x * (int)map->height + y] = value;
}

static int square_type(unsigned char square)
{
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int door_state(unsigned char square)
{
    return square & DUNGEON_SQUARE_MASK_ATTRIBS;
}

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

static int drive_to_closed_door(struct GameWorld_Compat* world,
                                struct Dm1V1MovementPipelinePc34Compat* pipeline)
{
    struct Dm1V1MovementPipelineResultPc34Compat result;
    size_t i;

    for (i = 0; i < sizeof(pass1055Route) / sizeof(pass1055Route[0]); ++i) {
        memset(&result, 0, sizeof(result));
        if (!run_command(pipeline, world, pass1055Route[i], &result)) {
            return 0;
        }
    }
    return 1;
}

struct DoorTarget {
    int mapIndex;
    int partyX;
    int partyY;
    int partyDir;
    int targetX;
    int targetY;
    unsigned char originalTile;
};

static int setup_at_pass1055_target(const char* dungeonPath,
                                    struct GameWorld_Compat* world,
                                    struct Dm1V1MovementPipelinePc34Compat* pipeline,
                                    struct DoorTarget* target)
{
    int dx = 0;
    int dy = 0;

    if (!load_world(dungeonPath, world)) {
        return 0;
    }
    DM1_V1_MovementPipeline_InitPc34Compat(pipeline);
    if (!drive_to_closed_door(world, pipeline)) {
        return 0;
    }

    F0701_MOVEMENT_GetStepDelta_Compat(world->party.direction, MOVE_FORWARD, &dx, &dy);
    memset(target, 0, sizeof(*target));
    target->mapIndex = world->party.mapIndex;
    target->partyX = world->party.mapX;
    target->partyY = world->party.mapY;
    target->partyDir = world->party.direction;
    target->targetX = world->party.mapX + dx;
    target->targetY = world->party.mapY + dy;
    target->originalTile = read_square(world->dungeon,
                                       target->mapIndex,
                                       target->targetX,
                                       target->targetY);
    return 1;
}

static int run_single_step_case(const char* dungeonPath,
                                const char* label,
                                unsigned char replacementTile,
                                int expectedCode,
                                int expectedBlocked)
{
    struct GameWorld_Compat world;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    struct DoorTarget target;
    int ok = 1;

    if (!setup_at_pass1055_target(dungeonPath, &world, &pipeline, &target)) {
        return 0;
    }

    write_square(world.dungeon,
                 target.mapIndex,
                 target.targetX,
                 target.targetY,
                 replacementTile);
    memset(&result, 0, sizeof(result));
    ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_MOVE_FORWARD, &result);
    printf("[%s] tile=0x%02x resultCode=%d blocked=%d party=(%d,%d,%d,%d)\n",
           label,
           replacementTile,
           result.core.movement.resultCode,
           result.core.movementBlocked,
           world.party.mapIndex,
           world.party.mapX,
           world.party.mapY,
           world.party.direction);

    ok &= expect_int(label, result.core.movement.resultCode, expectedCode);
    ok &= expect_int("movementBlocked", result.core.movementBlocked, expectedBlocked);
    if (expectedBlocked) {
        ok &= expect_int("blocked.map", world.party.mapIndex, target.mapIndex);
        ok &= expect_int("blocked.x", world.party.mapX, target.partyX);
        ok &= expect_int("blocked.y", world.party.mapY, target.partyY);
        ok &= expect_int("blocked.dir", world.party.direction, target.partyDir);
    }
    return ok;
}

static int run_repeated_closed_door_case(const char* dungeonPath)
{
    struct GameWorld_Compat world;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    struct DoorTarget target;
    int ok = 1;
    int blockedCount = 0;
    int movedCount = 0;
    int i;

    if (!setup_at_pass1055_target(dungeonPath, &world, &pipeline, &target)) {
        return 0;
    }

    for (i = 0; i < 10; ++i) {
        memset(&result, 0, sizeof(result));
        ok &= run_command(&pipeline, &world, DM1_V1_COMMAND_MOVE_FORWARD, &result);
        if (result.core.movementBlocked) {
            blockedCount++;
        }
        if (result.anyMovementOccurred) {
            movedCount++;
        }
    }

    printf("[repeat_closed_door] blocked=%d moved=%d party=(%d,%d,%d,%d)\n",
           blockedCount,
           movedCount,
           world.party.mapIndex,
           world.party.mapX,
           world.party.mapY,
           world.party.direction);
    ok &= expect_int("repeat.closed_door.blocked", blockedCount, 10);
    ok &= expect_int("repeat.closed_door.moved", movedCount, 0);
    ok &= expect_int("repeat.closed_door.map", world.party.mapIndex, target.mapIndex);
    ok &= expect_int("repeat.closed_door.x", world.party.mapX, target.partyX);
    ok &= expect_int("repeat.closed_door.y", world.party.mapY, target.partyY);
    ok &= expect_int("repeat.closed_door.dir", world.party.direction, target.partyDir);
    return ok;
}

int main(int argc, char** argv)
{
    const char* dungeonPath =
        argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_CANONICAL_DUNGEON_DAT");
    struct GameWorld_Compat world;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct DoorTarget target;
    int ok = 1;

    if (!dungeonPath || dungeonPath[0] == '\0') {
        dungeonPath = default_dm1_dungeon_dat();
    }

    printf("probe=firestaff_dm1_v1_extended_collision_pair_probe\n");
    printf("dungeon=%s\n", dungeonPath);
    printf("source=COMMAND.C:F0380 CLIKMENU.C:F0366 MOVESENS.C:F0267 DUNGEON.C:F0172\n");

    if (!setup_at_pass1055_target(dungeonPath, &world, &pipeline, &target)) {
        return 1;
    }
    printf("[pass1055_target] map=%d party=(%d,%d,%d) target=(%d,%d) tile=0x%02x type=%d state=%d\n",
           target.mapIndex,
           target.partyX,
           target.partyY,
           target.partyDir,
           target.targetX,
           target.targetY,
           target.originalTile,
           square_type(target.originalTile),
           door_state(target.originalTile));
    ok &= expect_int("target.type.door",
                     square_type(target.originalTile),
                     DUNGEON_ELEMENT_DOOR);
    ok &= expect_int("target.closed", door_state(target.originalTile) != DM1_DOOR_STATE_OPEN, 1);

    ok &= run_repeated_closed_door_case(dungeonPath);
    ok &= run_single_step_case(dungeonPath,
                               "closed_door",
                               target.originalTile,
                               MOVE_BLOCKED_DOOR,
                               1);
    ok &= run_single_step_case(dungeonPath,
                               "wall",
                               (unsigned char)((DUNGEON_ELEMENT_WALL << 5) | 0x00),
                               MOVE_BLOCKED_WALL,
                               1);
    ok &= run_single_step_case(dungeonPath,
                               "closed_fakewall",
                               (unsigned char)((DUNGEON_ELEMENT_FAKEWALL << 5) | 0x00),
                               MOVE_BLOCKED_WALL,
                               1);
    ok &= run_single_step_case(dungeonPath,
                               "open_door",
                               (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_OPEN),
                               MOVE_OK,
                               0);
    ok &= run_single_step_case(dungeonPath,
                               "open_fakewall",
                               (unsigned char)((DUNGEON_ELEMENT_FAKEWALL << 5) | 0x04),
                               MOVE_OK,
                               0);
    ok &= run_single_step_case(dungeonPath,
                               "pit_consequence_square",
                               (unsigned char)((DUNGEON_ELEMENT_PIT << 5) | 0x00),
                               MOVE_OK,
                               0);
    ok &= run_single_step_case(dungeonPath,
                               "teleporter_consequence_square",
                               (unsigned char)((DUNGEON_ELEMENT_TELEPORTER << 5) | 0x00),
                               MOVE_OK,
                               0);

    printf("result=%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
