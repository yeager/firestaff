/*
 * Pass 39 bounded probe — creature walkability unification.
 *
 * Scope: V1_BLOCKERS.md §3 (creature walkability not unified with party
 * F0706).  Pass 30 kept m11_square_walkable_for_creature on legacy logic
 * because a naive delegation to F0706 changed creature walkability for
 * STAIRS squares (STAIRS is legal for the party as a consequence
 * square, but DM1 PC 3.4 creatures never walk onto stairs per
 * ReDMCSB GROUP.C / F0264_MOVE_IsSquareAccessibleForCreature).
 *
 * Pass 39 introduces F0707_MOVEMENT_IsSquarePassableForContext_Compat
 * which shares element/door decoding with F0706 but rejects STAIRS
 * when passContext == MOVEMENT_PASS_CTX_CREATURE.
 *
 * This probe verifies:
 *   1. F0707(PARTY, s)    == F0706(s)                  for every s.
 *   2. F0707(CREATURE, s) == F0706(s) && elem != STAIRS for every s.
 *   3. Creature context rejects STAIRS explicitly (no regression guard).
 *   4. Creature context accepts corridor/pit/teleporter/fakewall/open
 *      door/destroyed door — i.e. same rules as party for every
 *      non-stairs element.
 *   5. Creature context rejects walls, closed doors, and opening/
 *      closing intermediate door states (1..4).
 *   6. Bounds and NULL safety match F0706.
 *   7. F0706 itself is unchanged (stairs still passable for party).
 *
 * Strictly a compat-layer probe.  Does not touch the game view, the
 * tick orchestrator, or any rendering path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_movement_pc34_compat.h"

#define MAP_W 4
#define MAP_H 4

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
    int c, r;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(squareData, 0, (size_t)MAP_W * MAP_H);

    dungeon->header.mapCount = 1;
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(struct DungeonMapDesc_Compat));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(struct DungeonMapTiles_Compat));
    dungeon->maps[0].width = MAP_W;
    dungeon->maps[0].height = MAP_H;
    dungeon->tiles[0].squareData = squareData;
    dungeon->tilesLoaded = 1;
    dungeon->loaded = 1;

    for (c = 0; c < MAP_W; ++c) {
        for (r = 0; r < MAP_H; ++r) {
            squareData[c * MAP_H + r] = sqb(DUNGEON_ELEMENT_WALL, 0);
        }
    }

    /*
     * Element map (col,row):
     *   (0,0) WALL          (1,0) CORRIDOR    (2,0) DOOR closed(4) (3,0) DOOR open(0)
     *   (0,1) STAIRS down   (1,1) STAIRS up   (2,1) FAKEWALL       (3,1) PIT
     *   (0,2) TELEPORTER    (1,2) DOOR dest(5)(2,2) DOOR anim(2)   (3,2) DOOR anim(1)
     *   (0,3) WALL          (1,3) CORRIDOR    (2,3) DOOR anim(3)   (3,3) WALL
     */
    squareData[0 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_WALL, 0);
    squareData[1 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    squareData[2 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_DOOR, 4);
    squareData[3 * MAP_H + 0] = sqb(DUNGEON_ELEMENT_DOOR, 0);

    squareData[0 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_STAIRS, 0);
    squareData[1 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_STAIRS, 1);
    squareData[2 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_FAKEWALL, 0);
    squareData[3 * MAP_H + 1] = sqb(DUNGEON_ELEMENT_PIT, 0);

    squareData[0 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_TELEPORTER, 0);
    squareData[1 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_DOOR, 5);
    squareData[2 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_DOOR, 2);
    squareData[3 * MAP_H + 2] = sqb(DUNGEON_ELEMENT_DOOR, 1);

    squareData[0 * MAP_H + 3] = sqb(DUNGEON_ELEMENT_WALL, 0);
    squareData[1 * MAP_H + 3] = sqb(DUNGEON_ELEMENT_CORRIDOR, 0);
    squareData[2 * MAP_H + 3] = sqb(DUNGEON_ELEMENT_DOOR, 3);
    squareData[3 * MAP_H + 3] = sqb(DUNGEON_ELEMENT_WALL, 0);
}

static void free_fixture(struct DungeonDatState_Compat* dungeon) {
    free(dungeon->maps);
    free(dungeon->tiles);
    memset(dungeon, 0, sizeof(*dungeon));
}

int main(void) {
    struct DungeonDatState_Compat dungeon;
    unsigned char squareData[MAP_W * MAP_H];
    int c, r;

    build_fixture(&dungeon, squareData);

    /* --- Invariant 1: party-context F0707 agrees with F0706 for every square. --- */
    {
        int all_match = 1;
        for (c = 0; c < MAP_W; ++c) {
            for (r = 0; r < MAP_H; ++r) {
                int f706 = F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, c, r);
                int f707 = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
                    &dungeon, 0, c, r, MOVEMENT_PASS_CTX_PARTY);
                if (f706 != f707) {
                    printf("    mismatch (party) at (%d,%d): f706=%d f707=%d\n",
                           c, r, f706, f707);
                    all_match = 0;
                }
            }
        }
        record("P39_F0707_PARTY_EQ_F0706",
               all_match,
               "F0707(PARTY) equals F0706 for every square in fixture");
    }

    /* --- Invariant 2: creature-context F0707 equals F0706 on every non-stairs
     *                  square, and is 0 on every stairs square. --- */
    {
        int all_match = 1;
        for (c = 0; c < MAP_W; ++c) {
            for (r = 0; r < MAP_H; ++r) {
                unsigned char b = squareData[c * MAP_H + r];
                int elem = (b & DUNGEON_SQUARE_MASK_TYPE) >> 5;
                int f706 = F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, c, r);
                int f707 = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
                    &dungeon, 0, c, r, MOVEMENT_PASS_CTX_CREATURE);
                int expected = (elem == DUNGEON_ELEMENT_STAIRS) ? 0 : f706;
                if (f707 != expected) {
                    printf("    mismatch (creature) at (%d,%d) elem=%d: f706=%d f707=%d exp=%d\n",
                           c, r, elem, f706, f707, expected);
                    all_match = 0;
                }
            }
        }
        record("P39_F0707_CREATURE_NON_STAIRS_EQ_F0706",
               all_match,
               "F0707(CREATURE) equals F0706 except stairs are blocked");
    }

    /* --- Invariant 3: creature context rejects STAIRS explicitly (regression guard). --- */
    {
        int stairs_down = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 0, 1, MOVEMENT_PASS_CTX_CREATURE);
        int stairs_up = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 1, 1, MOVEMENT_PASS_CTX_CREATURE);
        record("P39_F0707_CREATURE_REJECTS_STAIRS_DOWN",
               stairs_down == 0,
               "creature context blocks down-stairs square");
        record("P39_F0707_CREATURE_REJECTS_STAIRS_UP",
               stairs_up == 0,
               "creature context blocks up-stairs square");
    }

    /* --- Invariant 4: creature context accepts corridor/pit/teleporter/fakewall/
     *                  open door/destroyed door (element parity with party). --- */
    {
        int corridor = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 1, 0, MOVEMENT_PASS_CTX_CREATURE);
        int pit = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 3, 1, MOVEMENT_PASS_CTX_CREATURE);
        int teleporter = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 0, 2, MOVEMENT_PASS_CTX_CREATURE);
        int fakewall = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 2, 1, MOVEMENT_PASS_CTX_CREATURE);
        int door_open = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 3, 0, MOVEMENT_PASS_CTX_CREATURE);
        int door_destroyed = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 1, 2, MOVEMENT_PASS_CTX_CREATURE);
        record("P39_F0707_CREATURE_ACCEPTS_CORRIDOR",
               corridor == 1, "creature context accepts corridor");
        record("P39_F0707_CREATURE_ACCEPTS_PIT",
               pit == 1, "creature context accepts pit");
        record("P39_F0707_CREATURE_ACCEPTS_TELEPORTER",
               teleporter == 1, "creature context accepts teleporter");
        record("P39_F0707_CREATURE_ACCEPTS_FAKEWALL",
               fakewall == 1, "creature context accepts fake wall");
        record("P39_F0707_CREATURE_ACCEPTS_DOOR_OPEN",
               door_open == 1, "creature context accepts fully-open door");
        record("P39_F0707_CREATURE_ACCEPTS_DOOR_DESTROYED",
               door_destroyed == 1, "creature context accepts destroyed door");
    }

    /* --- Invariant 5: creature context rejects walls, closed doors, and every
     *                  animating door intermediate state (1..4). --- */
    {
        int wall = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 0, 0, MOVEMENT_PASS_CTX_CREATURE);
        int door4 = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 2, 0, MOVEMENT_PASS_CTX_CREATURE); /* state 4 */
        int door3 = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 2, 3, MOVEMENT_PASS_CTX_CREATURE); /* state 3 */
        int door2 = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 2, 2, MOVEMENT_PASS_CTX_CREATURE); /* state 2 */
        int door1 = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 3, 2, MOVEMENT_PASS_CTX_CREATURE); /* state 1 */
        record("P39_F0707_CREATURE_REJECTS_WALL",
               wall == 0, "creature context blocks wall");
        record("P39_F0707_CREATURE_REJECTS_DOOR_CLOSED",
               door4 == 0, "creature context blocks closed door (state 4)");
        record("P39_F0707_CREATURE_REJECTS_DOOR_STATE3",
               door3 == 0, "creature context blocks animating door (state 3)");
        record("P39_F0707_CREATURE_REJECTS_DOOR_STATE2",
               door2 == 0, "creature context blocks animating door (state 2)");
        record("P39_F0707_CREATURE_REJECTS_DOOR_STATE1",
               door1 == 0, "creature context blocks animating door (state 1)");
    }

    /* --- Invariant 6: bounds / NULL safety. --- */
    {
        int null_dun = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            NULL, 0, 0, 0, MOVEMENT_PASS_CTX_CREATURE);
        int oob_neg = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, -1, 0, MOVEMENT_PASS_CTX_CREATURE);
        int oob_y   = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 0, MAP_H, MOVEMENT_PASS_CTX_CREATURE);
        int oob_map = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 99, 0, 0, MOVEMENT_PASS_CTX_CREATURE);
        record("P39_F0707_NULL_SAFE",
               null_dun == 0, "F0707 with NULL dungeon returns 0");
        record("P39_F0707_OOB_NEG",
               oob_neg == 0, "F0707 with negative coordinate returns 0");
        record("P39_F0707_OOB_Y",
               oob_y == 0, "F0707 with y == height returns 0");
        record("P39_F0707_OOB_MAP",
               oob_map == 0, "F0707 with out-of-range mapIndex returns 0");
    }

    /* --- Invariant 7: F0706 is unchanged for STAIRS (no party regression). --- */
    {
        int stairs_down = F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 0, 1);
        int stairs_up = F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 1, 1);
        record("P39_F0706_PARTY_STAIRS_DOWN_UNCHANGED",
               stairs_down == 1,
               "F0706 still treats down-stairs as passable for the party");
        record("P39_F0706_PARTY_STAIRS_UP_UNCHANGED",
               stairs_up == 1,
               "F0706 still treats up-stairs as passable for the party");
    }

    /* --- Invariant 8: creature context with unknown/invalid passContext
     *                  falls back to party semantics (defensive default). --- */
    {
        /* MOVEMENT_PASS_CTX_PARTY == 0; any non-CREATURE value should behave
         * like party per the switch's default branch (stairs == 1).  Test
         * with an arbitrary non-creature value. */
        int stairs_unknown = F0707_MOVEMENT_IsSquarePassableForContext_Compat(
            &dungeon, 0, 0, 1, 42);
        record("P39_F0707_UNKNOWN_CTX_DEFAULTS_TO_PARTY",
               stairs_unknown == 1,
               "unknown passContext defaults to party semantics for stairs");
    }

    free_fixture(&dungeon);

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
