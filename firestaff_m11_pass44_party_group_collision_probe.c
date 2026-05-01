/*
 * Pass 44 bounded probe — party/group collision source lock.
 *
 * Source evidence:
 *   - ReDMCSB CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty
 *     checks wall/door/fake-wall legality first, then when
 *     G0305_ui_PartyChampionCount != 0 blocks passable destination squares
 *     that contain F0175_GROUP_GetThing (lines 278-315 in WIP20210206).
 *   - CLIKMENU.C lines 291-292 preserve the empty-party custom-dungeon bug:
 *     no champions means the group-presence check is skipped.
 *   - COMMAND.C lines 2095-2100 / 2104-2110 block only cardinal movement
 *     while movement is disabled; turn commands are outside this group gate.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_movement_pc34_compat.h"

#define MAP_W 3
#define MAP_H 3

static int g_pass = 0;
static int g_fail = 0;

static unsigned char sqb(int elementType, int attrs) {
    return (unsigned char)(((elementType & 7) << 5) | (attrs & 0x1F));
}

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static void build_fixture(struct DungeonDatState_Compat* dungeon,
                          struct DungeonThings_Compat* things,
                          unsigned char* squares,
                          unsigned short* firstThings) {
    int i;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(things, 0, sizeof(*things));
    memset(squares, 0, MAP_W * MAP_H);
    for (i = 0; i < MAP_W * MAP_H; ++i) squares[i] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);

    /* Column-major.  The east target (2,1) is passable and has a group list. */
    squares[2 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    /* The west target (0,1) is a wall with a group list; source checks wall first. */
    squares[0 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_WALL, DUNGEON_SQUARE_MASK_THING_LIST);

    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(*dungeon->maps));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(*dungeon->tiles));
    dungeon->maps[0].width = MAP_W;
    dungeon->maps[0].height = MAP_H;
    dungeon->tiles[0].squareData = squares;
    dungeon->tilesLoaded = 1;
    dungeon->loaded = 1;

    firstThings[0] = (unsigned short)((THING_TYPE_GROUP << 10) | 0); /* wall target */
    firstThings[1] = (unsigned short)((THING_TYPE_GROUP << 10) | 0); /* corridor target */
    things->squareFirstThings = firstThings;
    things->squareFirstThingCount = 2;
    things->groupCount = 1;
}

static void free_fixture(struct DungeonDatState_Compat* dungeon) {
    free(dungeon->maps);
    free(dungeon->tiles);
}

int main(void) {
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    unsigned char squares[MAP_W * MAP_H];
    unsigned short firstThings[2];
    struct PartyState_Compat party;
    int blocked;

    build_fixture(&dungeon, &things, squares, firstThings);

    memset(&party, 0, sizeof(party));
    party.mapIndex = 0;
    party.mapX = 1;
    party.mapY = 1;
    party.direction = 1; /* east */
    party.championCount = 1;
    party.champions[0].present = 1;
    party.champions[0].hp.current = 10;

    blocked = F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
        &dungeon, &things, &party, MOVE_FORWARD);
    record("P44_F0708_GROUP_BLOCKS_PASSABLE_TARGET",
           blocked == 1,
           "party with champions is blocked by a group on a passable destination square");

    party.championCount = 0;
    blocked = F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
        &dungeon, &things, &party, MOVE_FORWARD);
    record("P44_F0708_EMPTY_PARTY_SKIPS_GROUP",
           blocked == 0,
           "empty party preserves ReDMCSB BUG0_85 and skips group collision");

    party.championCount = 1;
    blocked = F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
        &dungeon, &things, &party, MOVE_TURN_RIGHT);
    record("P44_F0708_TURNS_IGNORE_GROUPS",
           blocked == 0,
           "turn-only movement is not group-collision gated");

    party.direction = 3; /* west into wall that also has a group list */
    blocked = F0708_MOVEMENT_IsPartyStepBlockedByGroup_Compat(
        &dungeon, &things, &party, MOVE_FORWARD);
    record("P44_F0708_WALL_LEGALITY_WINS",
           blocked == 0,
           "impassable wall target is handled by legality before group collision");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    free_fixture(&dungeon);
    return g_fail ? 1 : 0;
}
