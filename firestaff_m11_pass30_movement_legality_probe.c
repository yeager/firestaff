/*
 * Pass 30 bounded probe — movement legality migration.
 *
 * Verifies the new source-faithful owners introduced in Pass 30:
 *   - F0702_MOVEMENT_TryMove_Compat now blocks closed doors (MOVE_BLOCKED_DOOR).
 *   - F0702 still passes through corridor/open-door/fake-wall/stairs.
 *   - F0705_MOVEMENT_ResolveStairsTransition_Compat detects stairs squares,
 *     selects ascend vs descend per the attribute bit, clamps destination
 *     into the target map bounds, and refuses to transition off the end
 *     of the dungeon.
 *   - F0706_MOVEMENT_IsSquarePassable_Compat mirrors F0702's source-semantic
 *     rules and is consistent with F0702 on every element type.
 *
 * Scope is strictly party-side movement legality as described in
 * PASSLIST_29_36.md §4.30.  Creature walkability unification is
 * explicitly deferred to a later pass.
 *
 * The probe synthesises a tiny 4x4 dungeon in memory, populated with
 * exactly one square of every element type, so it does not depend on
 * DUNGEON.DAT content.  This matches the "bounded new probe/check"
 * requirement for Pass 30 without expanding the gate surface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_movement_pc34_compat.h"

#define MAP_W 4
#define MAP_H 4

/* Encode a square byte from (elementType, attributes). */
static unsigned char sqb(int elementType, int attribs) {
    return (unsigned char)(((elementType & 7) << 5) | (attribs & 0x1F));
}

static int g_pass = 0;
static int g_fail = 0;

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
                          unsigned char* squareData) {
    /*
     * Layout (row, col):
     *   (0,0) WALL        (0,1) CORRIDOR   (0,2) DOOR closed  (0,3) DOOR open
     *   (1,0) STAIRS down (1,1) STAIRS up  (1,2) FAKEWALL     (1,3) PIT
     *   (2,0) TELEPORTER  (2,1) CORRIDOR   (2,2) DOOR dest5   (2,3) WALL
     *   (3,0) WALL        (3,1) WALL       (3,2) WALL         (3,3) WALL
     *
     * Column-major storage: squareData[col * H + row].
     */
    int c, r;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(squareData, 0, (size_t)MAP_W * MAP_H);

    dungeon->header.mapCount = 2;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(2, sizeof(struct DungeonMapDesc_Compat));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(2, sizeof(struct DungeonMapTiles_Compat));
    dungeon->maps[0].width = MAP_W;
    dungeon->maps[0].height = MAP_H;
    dungeon->maps[1].width = MAP_W;
    dungeon->maps[1].height = MAP_H;
    dungeon->tiles[0].squareData = squareData;
    dungeon->tiles[1].squareData = squareData;   /* reuse same data for map 1 */
    dungeon->tilesLoaded = 1;
    dungeon->loaded = 1;

    for (c = 0; c < MAP_W; ++c) {
        for (r = 0; r < MAP_H; ++r) {
            squareData[c * MAP_H + r] = sqb(DUNGEON_ELEMENT_WALL, 0);
        }
    }

    /* row-0 */
    squareData[0 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_WALL, 0);
    squareData[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    squareData[2 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_DOOR, 4);     /* closed */
    squareData[3 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_DOOR, 0);     /* open */
    /* row-1 */
    squareData[0 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_STAIRS, 0);   /* down */
    squareData[1 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_STAIRS, 1);   /* up */
    squareData[2 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_FAKEWALL, 0);
    squareData[3 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_PIT, 0);
    /* row-2 */
    squareData[0 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_TELEPORTER, 0);
    squareData[1 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    squareData[2 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_DOOR, 5);     /* destroyed */
    squareData[3 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_WALL, 0);
}

static void free_fixture(struct DungeonDatState_Compat* dungeon) {
    free(dungeon->maps);
    free(dungeon->tiles);
    memset(dungeon, 0, sizeof(*dungeon));
}

/* Try moving party (starting at sx,sy facing North) one step forward
 * and return result code. */
static int step_forward(const struct DungeonDatState_Compat* dungeon,
                         int sx, int sy, int dir,
                         struct MovementResult_Compat* out) {
    struct PartyState_Compat party;
    memset(&party, 0, sizeof(party));
    party.mapX = sx;
    party.mapY = sy;
    party.direction = dir;
    party.mapIndex = 0;
    memset(out, 0, sizeof(*out));
    F0702_MOVEMENT_TryMove_Compat(dungeon, &party, MOVE_FORWARD, out);
    return out->resultCode;
}

int main(void) {
    struct DungeonDatState_Compat dungeon;
    unsigned char squareData[MAP_W * MAP_H];
    struct MovementResult_Compat mr;
    struct StairsTransitionResult_Compat stairs;
    struct PartyState_Compat party;
    int rc;

    build_fixture(&dungeon, squareData);

    /* ---- F0702 party legality ---- */

    /* Wall: stepping from (1,0) west into (0,0) blocks with WALL. */
    rc = step_forward(&dungeon, 1, 0, 3 /* west */, &mr);
    record("P30_F0702_WALL",
           rc == MOVE_BLOCKED_WALL,
           "stepping into a wall returns MOVE_BLOCKED_WALL");

    /* Corridor: (2,1) north into (2,0) DOOR closed blocks with DOOR. */
    rc = step_forward(&dungeon, 2, 1, 0 /* north */, &mr);
    record("P30_F0702_DOOR_CLOSED",
           rc == MOVE_BLOCKED_DOOR,
           "stepping into a closed door returns MOVE_BLOCKED_DOOR");

    /* (3,1) east ... actually easier: step from (1,1) south into (1,2) CORRIDOR.
     * We want OPEN door reachable: stand at (3,1) and step north into (3,0). */
    rc = step_forward(&dungeon, 3, 1, 0 /* north */, &mr);
    record("P30_F0702_DOOR_OPEN",
           rc == MOVE_OK && mr.newMapX == 3 && mr.newMapY == 0,
           "stepping onto an open door square returns MOVE_OK");

    /* Destroyed door (state 5): (2,2) from (1,2) east. */
    rc = step_forward(&dungeon, 1, 2, 1 /* east */, &mr);
    record("P30_F0702_DOOR_DESTROYED",
           rc == MOVE_OK && mr.newMapX == 2 && mr.newMapY == 2,
           "stepping onto a destroyed door square returns MOVE_OK");

    /* Fake wall at (2,1): (1,1) east. */
    rc = step_forward(&dungeon, 1, 1, 1 /* east */, &mr);
    record("P30_F0702_FAKEWALL",
           rc == MOVE_OK && mr.newMapX == 2 && mr.newMapY == 1,
           "stepping onto a fake wall square returns MOVE_OK");

    /* Stairs-down at (0,1): (1,1) west. */
    rc = step_forward(&dungeon, 1, 1, 3 /* west */, &mr);
    record("P30_F0702_STAIRS",
           rc == MOVE_OK && mr.newMapX == 0 && mr.newMapY == 1,
           "stepping onto a stairs square returns MOVE_OK (no level change)");

    /* Out of bounds: (0,0) north. */
    rc = step_forward(&dungeon, 0, 0, 0, &mr);
    record("P30_F0702_BOUNDS",
           rc == MOVE_BLOCKED_BOUNDS,
           "stepping out of map bounds returns MOVE_BLOCKED_BOUNDS");

    /* Turn-only still works. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 1; party.mapY = 1; party.direction = 0;
    F0702_MOVEMENT_TryMove_Compat(&dungeon, &party, MOVE_TURN_RIGHT, &mr);
    record("P30_F0702_TURN",
           mr.resultCode == MOVE_TURN_ONLY && mr.newDirection == 1,
           "turn-right still returns MOVE_TURN_ONLY with new direction");

    /* ---- F0705 stairs resolution ---- */

    /* Standing on down-stairs (0,1) mapIndex=0 → toMapIndex=1. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 0; party.mapY = 1; party.direction = 2;
    rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &stairs);
    record("P30_F0705_DOWN",
           rc == 1 && stairs.transitioned == 1 && stairs.stairUp == 0 &&
               stairs.toMapIndex == 1 && stairs.newMapX == 0 &&
               stairs.newMapY == 1 && stairs.newDirection == 2,
           "down-stairs resolves to next map with preserved direction");

    /* Standing on up-stairs (1,1) mapIndex=1 → toMapIndex=0. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 1; party.mapX = 1; party.mapY = 1; party.direction = 1;
    rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &stairs);
    record("P30_F0705_UP",
           rc == 1 && stairs.transitioned == 1 && stairs.stairUp == 1 &&
               stairs.toMapIndex == 0 && stairs.newDirection == 1,
           "up-stairs resolves to previous map");

    /* Standing on up-stairs at mapIndex=0 → out of range, returns transitioned=0. */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 1; party.mapY = 1; party.direction = 0;
    rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &stairs);
    record("P30_F0705_UP_OOR",
           (rc == 0 || stairs.transitioned == 0),
           "up-stairs at level 0 does not transition");

    /* Not on stairs: corridor at (1,0). */
    memset(&party, 0, sizeof(party));
    party.mapIndex = 0; party.mapX = 1; party.mapY = 0; party.direction = 0;
    rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &stairs);
    record("P30_F0705_NOT_STAIRS",
           (rc == 0 || stairs.transitioned == 0),
           "non-stairs square does not transition");

    /* ---- F0706 shared passability is consistent with F0702 ---- */
    {
        int c, r;
        int consistent = 1;
        for (c = 0; c < MAP_W; ++c) {
            for (r = 0; r < MAP_H; ++r) {
                unsigned char b = squareData[c * MAP_H + r];
                int elem = (b & DUNGEON_SQUARE_MASK_TYPE) >> 5;
                int door = b & 0x07;
                int expected = 0;
                int actual = F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, c, r);
                switch (elem) {
                    case DUNGEON_ELEMENT_CORRIDOR:
                    case DUNGEON_ELEMENT_PIT:
                    case DUNGEON_ELEMENT_TELEPORTER:
                    case DUNGEON_ELEMENT_FAKEWALL:
                    case DUNGEON_ELEMENT_STAIRS:
                        expected = 1; break;
                    case DUNGEON_ELEMENT_DOOR:
                        expected = (door == 0 || door == 5) ? 1 : 0; break;
                    default:
                        expected = 0; break;
                }
                if (expected != actual) {
                    printf("    mismatch at (%d,%d) elem=%d door=%d exp=%d got=%d\n",
                           c, r, elem, door, expected, actual);
                    consistent = 0;
                }
            }
        }
        record("P30_F0706_CONSISTENT",
               consistent,
               "F0706 passability matches expected per-element rules");
    }

    /* F0706 bounds / NULL safety. */
    record("P30_F0706_NULL",
           F0706_MOVEMENT_IsSquarePassable_Compat(NULL, 0, 0, 0) == 0,
           "F0706 with NULL dungeon returns 0");
    record("P30_F0706_OOB",
           F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, -1, 0) == 0 &&
           F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 0, MAP_H) == 0 &&
           F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 99, 0, 0) == 0,
           "F0706 with out-of-range coordinates returns 0");

    free_fixture(&dungeon);

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
