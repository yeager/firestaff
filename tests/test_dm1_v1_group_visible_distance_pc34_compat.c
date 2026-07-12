/* ReDMCSB GROUP.C F0197/F0199/F0200 and PROJEXPL.C F0227 runtime gate. */
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        failures++; \
    } \
} while (0)

static void init_world(struct GameWorld_Compat* world,
                       struct DungeonDatState_Compat* dungeon,
                       struct DungeonMapDesc_Compat* map,
                       struct DungeonMapTiles_Compat* tiles,
                       unsigned char* squares)
{
    memset(world, 0, sizeof(*world));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(squares, DUNGEON_ELEMENT_CORRIDOR << 5, 36);
    map->width = 6;
    map->height = 6;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->tilesLoaded = 1;
    tiles->squareData = squares;
    world->dungeon = dungeon;
}

static void init_context(struct DM1GroupBehaviorContext_Compat* context,
                         int sourceX, int sourceY, int destinationX,
                         int destinationY, int attributes, int sightRange)
{
    memset(context, 0, sizeof(*context));
    context->currentMapIndex = 0;
    context->partyMapIndex = 0;
    context->currentGroupMapX = sourceX;
    context->currentGroupMapY = sourceY;
    context->partyMapX = destinationX;
    context->partyMapY = destinationY;
    context->creatureInfo.attributes = attributes;
    context->creatureInfo.ranges = (unsigned short)(sightRange & 0x0f);
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonGroup_Compat group;
    struct DM1GroupBehaviorContext_Compat context;
    unsigned char squares[36];

    init_world(&world, &dungeon, &map, &tiles, squares);
    memset(&group, 0, sizeof(group));
    group.direction = 1; /* East: the F0227 cone includes south-east. */

    init_context(&context, 1, 1, 3, 3, 0, 8);
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 4,
          "F0199 returns Manhattan distance across a clear diagonal");

    /* F0199's equal-axis path blocks a diagonal only when both adjacent
     * corners are blocked, not when just one orthogonal square is a wall. */
    squares[2 * map.height + 3] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 4,
          "F0199 permits the diagonal when one adjacent corner is clear");
    squares[3 * map.height + 2] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 0,
          "F0199 blocks the diagonal when both adjacent corners are walls");

    memset(squares, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squares));
    group.direction = 0; /* North cannot see a target in the south-east cone. */
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 0,
          "F0227 rejects a target outside the creature facing cone");

    context.creatureInfo.attributes = DM1_ATTR_SIDE_ATTACK;
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 4,
          "F0200 side-attack creatures bypass the facing-cone gate");

    context.creatureInfo.attributes = 0;
    group.direction = 1;
    world.magic.event71CountInvisibility = 1;
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 0,
          "F0200 invisibility blocks creatures without SEE_INVISIBLE");
    context.creatureInfo.attributes = DM1_ATTR_SEE_INVISIBLE;
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 4,
          "F0200 SEE_INVISIBLE bypasses the party invisibility gate");

    world.magic.event71CountInvisibility = 0;
    init_context(&context, 1, 1, 4, 4, DM1_ATTR_SIDE_ATTACK, 2);
    F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u);
    CHECK(F0890c_ORCH_GetGroupVisibleDistance_Compat(
              &world, &context, &group) == 0,
          "F0200 darkest palette reduces ordinary creature sight before F0199");

    if (failures != 0) return 1;
    printf("PASS: DM1 F0199/F0200 diagonal visibility runtime gate\n");
    return 0;
}
