/*
 * test_dm1_v1_dun06b_f0155_stairs_exit_direction_gate_pc34_compat.c
 *
 * DUN-06b follow-up gate — DM1 V1 stairs up/down level transition regression.
 *
 * The DUN-06 test (`test_dm1_v1_dun06_f0154_f0705_stairs_transition_pc34_compat.c`)
 * pins F0705_MOVEMENT_ResolveStairsTransition_Compat robustness: NULL/bounds
 * checks, zero-tiles, empty dungeon.  It does NOT exercise the positive
 * resolution path or the F0155_DUNGEON_GetStairsExitDirection lookup that
 * happens after F0154_DUNGEON_GetLocationAfterLevelChange.
 *
 * This gate pins the *positive* path end-to-end with a synthetic 2-level
 * dungeon whose stairs squares have an actual element byte, plus pins
 * the four M11_StairLevelState state-machine invariants (I1..I4) that
 * govern the level-transition timing in the M11 launcher seam.
 *
 * Why this matters now:
 *   - The recent history shows the F0155 lookup had the EAST/NORTH
 *     neighbor checks swapped (see the inline comment in
 *     `memory_movement_pc34_compat.c:132`).  Without a focused gate,
 *     that swap can silently regress again.
 *   - The M11_StairLevelState state machine has only informal invariants
 *     (I1..I4 in `dm1_v1_stairs_level_pc34_compat.c`) and no test that
 *     pins the no-concurrent-transitions / write-order / clamp-to-zero /
 *     idempotent-no-op behavior across M11 launcher ticks.
 *   - Both the F0705 positive path and the M11 state machine are part of
 *     the same stairs-triggered level change; if either regresses the
 *     player can fall through the floor (literally — wrong exit
 *     direction can walk into a wall on the destination level).
 *
 * ReDMCSB references:
 *   CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142   — stairs trigger
 *   DUNGEON.C:F0154_GetLocationAfterLevelChange:1508-1558 — level delta + map lookup
 *   DUNGEON.C:F0155_GetStairsExitDirection:1560-1582      — NS/EW + neighbor check
 *   DEFS.H:M034_SQUARE_TYPE = square >> 5                — type bits (DUNGEON_ELEMENT_*)
 *   DEFS.H:MASK0x0004_STAIRS_UP = square & 0x04          — direction bit (UP/DOWN)
 *   MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:441-451    — party X/Y/Direction mutate
 *   GAMELOOP.C:58-64                                     — deferred new-party-map processing
 *
 * Scope discipline:
 *   - No retest of F0705 NULL/zero robustness (covered by DUN-06).
 *   - No retest of stairs + light carry-over (covered by
 *     test_dm1_v1_stairs_transition_light_state_pc34_compat).
 *   - No retest of stairs + inventory preservation (covered by
 *     test_dm1_v1_stairs_inventory_state_pc34_compat).
 *   - This test pins the F0155 exit-direction math + the M11 state
 *     machine invariants on top of the F0705 positive resolution.
 */

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "dm1_v1_stairs_level_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ── Helpers ─────────────────────────────────────────────────────────── */

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char* id, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static unsigned char stairs_square(unsigned char dirUp, int hasThingList)
{
    /*
     * DEFS.H: M034_SQUARE_TYPE(square) = square >> 5 → 3 (DUNGEON_ELEMENT_STAIRS)
     * so the upper 3 bits = 0b011 << 5 = 0x60.
     *
     * MASK0x0004_STAIRS_UP sits in the attribute byte; set it for UP,
     * clear it for DOWN.  Optional MASK0x0010_THING_LIST_PRESENT can
     * tag a thing-list slot on the stairs square.
     */
    unsigned char byte = (unsigned char)((DUNGEON_ELEMENT_STAIRS << 5) | 0x00);
    if (dirUp) byte |= 0x04;
    if (hasThingList) byte |= DUNGEON_SQUARE_MASK_THING_LIST;
    return byte;
}

static unsigned char wall_square(void)
{
    return (unsigned char)((DUNGEON_ELEMENT_WALL << 5) | 0x00);
}

static unsigned char corridor_square(void)
{
    return (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | 0x00);
}

/* ── Test 1: F0705 positive path, level-1 to level-0 with NS-oriented
 *       stairs on the destination, EAST neighbor is a wall → blocked,
 *       so the encoded direction must be DIR_WEST (3).
 *       Bit layout: NS=1, blocked=1 → (1<<1)|1 = 3 = DIR_WEST.
 *       This is the exact scenario that triggered the recent swap-bug
 *       regression in F0155 (the inline comment cites it).
 *
 *       Level math: party starts on map 0 (level=1), takes an UP
 *       stairs (levelDelta=-1), so F0154 looks for a level=0 map
 *       covering the source global coordinate.  Map 1 (level=0)
 *       covers the same global coordinate (offsetMapY=0, width=3,
 *       height=3 → global (0..2, 0..2) range, party at (1, 1) is
 *       inside). */

static void test_f0155_ns_blocked_east_returns_dir_west(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct PartyState_Compat party;
    struct StairsTransitionResult_Compat r;
    unsigned char map0Squares[9];
    unsigned char map1Squares[9];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&party, 0, sizeof(party));
    memset(map0Squares, 0, sizeof(map0Squares));
    memset(map1Squares, 0, sizeof(map1Squares));

    /* Two maps, both 3x3.  Map 0 level=1, map 1 level=0; both have
     * offsetMapX/Y = 0 so they cover the same global coordinate
     * (0..2, 0..2).  The party at global (1, 1) sits in both maps. */
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    maps[0].level = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].offsetMapX = 0;
    maps[0].offsetMapY = 0;
    tiles[0].squareData = map0Squares;
    tiles[0].squareCount = 9;

    maps[1].level = 0;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].offsetMapX = 0;
    maps[1].offsetMapY = 0;
    tiles[1].squareData = map1Squares;
    tiles[1].squareCount = 9;

    /* Map 0: stairs-up at (1,1) (column-major index 1*3+1 = 4).
     * Bit 0x04 set → UP stairs; bit 0x08 cleared → NS-oriented. */
    for (int i = 0; i < 9; ++i) map0Squares[i] = corridor_square();
    map0Squares[1 * 3 + 1] = stairs_square(/*dirUp=*/1, /*thing=*/0);

    /* Map 1: stairs square at (1, 1) (matching global coord 1, 3+1=4).
     * Place a WALL on the EAST neighbor (2, 1) → blocked=1.
     * Bit 3 (0x08) cleared → NS-oriented → northSouth=1.
     * NS check: (x+1, y) → (2, 1) is wall → blocked. */
    for (int i = 0; i < 9; ++i) map1Squares[i] = corridor_square();
    map1Squares[1 * 3 + 1] = stairs_square(/*dirUp=*/1, /*thing=*/0); /* NS-oriented */
    map1Squares[2 * 3 + 1] = wall_square();

    party.mapIndex = 0;
    party.mapX = 1;
    party.mapY = 1;
    party.direction = DIR_NORTH;
    party.championCount = 0;

    int rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r);

    expect_int("f0155.ns_blocked.rc", rc, 1,
               "DUNGEON.C:F0154_DUNGEON_GetLocationAfterLevelChange:1508-1558");
    expect_int("f0155.ns_blocked.transitioned", r.transitioned, 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("f0155.ns_blocked.stairUp", r.stairUp, 1,
               "MASK0x0004_STAIRS_UP = square & 0x04 (DEFS.H)");
    expect_int("f0155.ns_blocked.fromMapIndex", r.fromMapIndex, 0,
               "DUNGEON.C:F0154:1508-1558 (source map)");
    expect_int("f0155.ns_blocked.toMapIndex", r.toMapIndex, 1,
               "DUNGEON.C:F0154:1508-1558 (level-1 = map 1, level=0)");
    expect_int("f0155.ns_blocked.newMapX", r.newMapX, 1,
               "DUNGEON.C:F0154:1508-1558 (globalX - target.offsetMapX = 1)");
    expect_int("f0155.ns_blocked.newMapY", r.newMapY, 1,
               "DUNGEON.C:F0154:1508-1558 (globalY - target.offsetMapY = 1-0)");
    /* NS=1, blocked=1 → (1<<1) | 1 = 3 = DIR_WEST. */
    expect_int("f0155.ns_blocked.newDirection", r.newDirection, DIR_WEST,
               "DUNGEON.C:F0155_DUNGEON_GetStairsExitDirection:1560-1582; "
               "NS-oriented + EAST neighbor wall = DIR_WEST (encode: blocked<<1 | ns)");
}

/* ── Test 2: F0155 NS-oriented, EAST neighbor is corridor → unblocked,
 *       so the encoded direction must be DIR_EAST (1).
 *       Bit layout: NS=1, blocked=0 → (0<<1)|1 = 1 = DIR_EAST. */

static void test_f0155_ns_open_east_returns_dir_east(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct PartyState_Compat party;
    struct StairsTransitionResult_Compat r;
    unsigned char map0Squares[9];
    unsigned char map1Squares[9];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&party, 0, sizeof(party));
    memset(map0Squares, 0, sizeof(map0Squares));
    memset(map1Squares, 0, sizeof(map1Squares));

    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    /* Map 0 level=1, map 1 level=0 (UP stairs go level=1 → 0). */
    maps[0].level = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].offsetMapX = 0;
    maps[0].offsetMapY = 0;
    tiles[0].squareData = map0Squares;
    tiles[0].squareCount = 9;

    maps[1].level = 0;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].offsetMapX = 0;
    maps[1].offsetMapY = 0;
    tiles[1].squareData = map1Squares;
    tiles[1].squareCount = 9;

    for (int i = 0; i < 9; ++i) map0Squares[i] = corridor_square();
    map0Squares[1 * 3 + 1] = stairs_square(/*dirUp=*/1, 0);

    /* Stairs square is NS-oriented (bit 3 cleared); east neighbor is corridor. */
    for (int i = 0; i < 9; ++i) map1Squares[i] = corridor_square();
    map1Squares[1 * 3 + 1] = stairs_square(/*dirUp=*/1, 0);
    /* (2,1) stays corridor → blocked=0 */

    party.mapIndex = 0;
    party.mapX = 1;
    party.mapY = 1;
    party.direction = DIR_SOUTH;

    int rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r);

    expect_int("f0155.ns_open.rc", rc, 1,
               "DUNGEON.C:F0154:1508-1558");
    expect_int("f0155.ns_open.stairUp", r.stairUp, 1,
               "MASK0x0004_STAIRS_UP = 1 (UP); level-1 = map 1");
    expect_int("f0155.ns_open.toMapIndex", r.toMapIndex, 1,
               "DUNGEON.C:F0154:1508-1558 (level-1 = map 1, level=0)");
    expect_int("f0155.ns_open.newDirection", r.newDirection, DIR_EAST,
               "DUNGEON.C:F0155:1560-1582; NS-oriented + EAST neighbor corridor = DIR_EAST "
               "(encode: 0<<1 | 1 = 1)");
}

/* ── Test 3: F0155 EW-oriented (bit 3 SET), NORTH neighbor is wall →
 *       blocked, encoded direction must be DIR_SOUTH (2).
 *       Bit layout: NS=0 (EW), blocked=1 → (1<<1)|0 = 2 = DIR_SOUTH.
 *       This is the "wrong direction after a swap regression" case:
 *       before the recent fix, EW+blocked encoded as 3 = DIR_WEST.
 *
 *       Level math: party on map 0 (level=1) takes UP stairs, target
 *       level = 0 → map 1 covers the destination. */

static void test_f0155_ew_blocked_north_returns_dir_south(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct PartyState_Compat party;
    struct StairsTransitionResult_Compat r;
    unsigned char map0Squares[9];
    unsigned char map1Squares[9];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&party, 0, sizeof(party));
    memset(map0Squares, 0, sizeof(map0Squares));
    memset(map1Squares, 0, sizeof(map1Squares));

    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    maps[0].level = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].offsetMapX = 0;
    maps[0].offsetMapY = 0;
    tiles[0].squareData = map0Squares;
    tiles[0].squareCount = 9;

    maps[1].level = 0;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].offsetMapX = 0;
    maps[1].offsetMapY = 0;
    tiles[1].squareData = map1Squares;
    tiles[1].squareCount = 9;

    for (int i = 0; i < 9; ++i) map0Squares[i] = corridor_square();
    /* UP stairs at (1, 1) on map 0 — bit 0x04 set, bit 0x08 set → EW-oriented. */
    map0Squares[1 * 3 + 1] = stairs_square(/*dirUp=*/1, 0);
    map0Squares[1 * 3 + 1] |= 0x08; /* override to EW orientation */

    /* Stairs square on map 1 is EW-oriented (bit 3 set); NORTH neighbor (1, 0) is wall. */
    for (int i = 0; i < 9; ++i) map1Squares[i] = corridor_square();
    map1Squares[1 * 3 + 1] = (unsigned char)((DUNGEON_ELEMENT_STAIRS << 5) | 0x08); /* EW */
    map1Squares[1 * 3 + 0] = wall_square(); /* NORTH = (1, 0) is wall */

    party.mapIndex = 0;
    party.mapX = 1;
    party.mapY = 1;
    party.direction = DIR_NORTH;

    int rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r);

    expect_int("f0155.ew_blocked.rc", rc, 1,
               "DUNGEON.C:F0154:1508-1558");
    expect_int("f0155.ew_blocked.stairUp", r.stairUp, 1,
               "MASK0x0004_STAIRS_UP = 1 (UP); level-1 = map 1");
    expect_int("f0155.ew_blocked.toMapIndex", r.toMapIndex, 1,
               "DUNGEON.C:F0154:1508-1558 (level-1 = map 1, level=0)");
    expect_int("f0155.ew_blocked.newDirection", r.newDirection, DIR_SOUTH,
               "DUNGEON.C:F0155:1560-1582; EW-oriented + NORTH neighbor wall = DIR_SOUTH "
               "(encode: 1<<1 | 0 = 2)");
}

/* ── Test 4: F0705 returns 0 (no transition) when party stands on a
 *       plain corridor square — the resolver must reject this and
 *       leave the result zeroed except for the bookkeeping copy. */

static void test_f0705_returns_zero_on_corridor_square(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct PartyState_Compat party;
    struct StairsTransitionResult_Compat r;
    unsigned char map0Squares[9];
    unsigned char map1Squares[9];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&party, 0, sizeof(party));
    memset(map0Squares, 0, sizeof(map0Squares));
    memset(map1Squares, 0, sizeof(map1Squares));

    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    maps[0].level = 0;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].offsetMapX = 0;
    maps[0].offsetMapY = 0;
    tiles[0].squareData = map0Squares;
    tiles[0].squareCount = 9;

    maps[1].level = 1;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].offsetMapX = 0;
    maps[1].offsetMapY = 0;
    tiles[1].squareData = map1Squares;
    tiles[1].squareCount = 9;

    for (int i = 0; i < 9; ++i) {
        map0Squares[i] = corridor_square();
        map1Squares[i] = corridor_square();
    }

    party.mapIndex = 0;
    party.mapX = 0;
    party.mapY = 0;
    party.direction = DIR_NORTH;

    int rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r);

    expect_int("f0705.corridor.rc", rc, 0,
               "M034_SQUARE_TYPE(square) = 1 (CORRIDOR), not STAIRS → no transition");
    expect_int("f0705.corridor.transitioned", r.transitioned, 0,
               "F0705 only sets transitioned=1 on DUNGEON_ELEMENT_STAIRS squares");
    expect_int("f0705.corridor.stairUp", r.stairUp, 0,
               "stairUp is only meaningful when transitioned=1");
    expect_int("f0705.corridor.fromMapIndex", r.fromMapIndex, party.mapIndex,
               "F0705 copies party.mapIndex into fromMapIndex unconditionally");
    expect_int("f0705.corridor.toMapIndex", r.toMapIndex, party.mapIndex,
               "F0705 keeps toMapIndex = party.mapIndex when no transition");
    expect_int("f0705.corridor.newMapX", r.newMapX, party.mapX,
               "F0705 keeps newMapX = party.mapX when no transition");
    expect_int("f0705.corridor.newMapY", r.newMapY, party.mapY,
               "F0705 keeps newMapY = party.mapY when no transition");
}

/* ── Test 5: F0705 must return 0 when no target map covers the global
 *       coordinate of the source stairs square (F0154 returns -1). */

static void test_f0705_returns_zero_when_no_target_map_covers_coord(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct PartyState_Compat party;
    struct StairsTransitionResult_Compat r;
    unsigned char map0Squares[9];
    unsigned char map1Squares[9];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&party, 0, sizeof(party));
    memset(map0Squares, 0, sizeof(map0Squares));
    memset(map1Squares, 0, sizeof(map1Squares));

    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    maps[0].level = 0;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].offsetMapX = 0;
    maps[0].offsetMapY = 0;
    tiles[0].squareData = map0Squares;
    tiles[0].squareCount = 9;

    /* Map 1 exists but sits at level=1 with offsetMapX=100 → its
     * global coordinate range is (100..102, 0..2).  The source
     * stairs global coordinate is (1, 1), which is NOT covered.
     * F0154 returns -1 and F0705 returns 0. */
    maps[1].level = 1;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].offsetMapX = 100;
    maps[1].offsetMapY = 0;
    tiles[1].squareData = map1Squares;
    tiles[1].squareCount = 9;

    for (int i = 0; i < 9; ++i) {
        map0Squares[i] = corridor_square();
        map1Squares[i] = corridor_square();
    }
    map0Squares[1 * 3 + 1] = stairs_square(/*dirUp=*/1, 0);

    party.mapIndex = 0;
    party.mapX = 1;
    party.mapY = 1;
    party.direction = DIR_NORTH;

    int rc = F0705_MOVEMENT_ResolveStairsTransition_Compat(&dungeon, &party, &r);

    expect_int("f0705.no_target.rc", rc, 0,
               "DUNGEON.C:F0154:1508-1558 returns -1 when no map covers target coords");
    expect_int("f0705.no_target.transitioned", r.transitioned, 0,
               "F0705 must not transition when F0154 returns -1");
    expect_int("f0705.no_target.toMapIndex", r.toMapIndex, party.mapIndex,
               "F0705 keeps toMapIndex = party.mapIndex on F0154 miss");
}

/* ── Test 6: M11_StairLevelState invariants I1..I4 across
 *       m11_stairs_use and m11_stairs_tick.  This pins the state-machine
 *       contract that the M11 launcher tick relies on. */

static void test_m11_stairs_state_machine_invariants(void)
{
    M11_StairLevelState s;
    int newX, newY, newFacing;
    int rc;

    m11_stairs_init(&s);
    m11_stairs_add_level(&s, 16, 16);
    m11_stairs_add_level(&s, 16, 16);
    rc = m11_stairs_add(&s, 5, 6, DM1_STAIR_UP, /*destLevel=*/1,
                        /*destX=*/5, /*destY=*/6, /*destFacing=*/DM1_STAIR_UP);
    expect_int("m11.add_rc", rc, 1,
               "M11_StairDef registration before transition");
    expect_int("m11.current_level_init", s.currentLevel, 0,
               "I0 — fresh init keeps currentLevel=0");
    expect_int("m11.transition_active_init", s.transitionActive, 0,
               "I0 — fresh init keeps transitionActive=0");

    /* I4 — tick is idempotent when inactive (no state mutation, no
     * underflow).  Tick with absurd values to make sure no negative
     * underflow reaches transitionActive. */
    m11_stairs_tick(&s, 1);
    m11_stairs_tick(&s, 100000);
    expect_int("m11.i4_idempotent_active", s.transitionActive, 0,
               "I4 — tick with transitionActive=0 is a no-op");
    expect_int("m11.i4_idempotent_ticks_left", s.transitionTicksLeft, 0,
               "I4 — ticksLeft stays at 0, never underflows");

    /* Use the stairs at (5, 6): triggers the transition. */
    rc = m11_stairs_use(&s, 5, 6, &newX, &newY, &newFacing);
    expect_int("m11.use_rc", rc, 1,
               "MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE:441-451 — staircase lookup");
    expect_int("m11.use_newX", newX, 5,
               "M11_StairDef.destX carry-over");
    expect_int("m11.use_newY", newY, 6,
               "M11_StairDef.destY carry-over");
    expect_int("m11.use_newFacing", newFacing, DM1_STAIR_UP,
               "M11_StairDef.destFacing carry-over");

    /* I2 — write order: fromLevel/toLevel/currentLevel/ticksLeft all
     * set BEFORE transitionActive becomes 1. */
    expect_int("m11.i2_current_level", s.currentLevel, 1,
               "I2 — currentLevel reflects destLevel after use");
    expect_int("m11.i2_from_level", s.transitionFromLevel, 0,
               "I2 — transitionFromLevel captures pre-use level");
    expect_int("m11.i2_to_level", s.transitionToLevel, 1,
               "I2 — transitionToLevel captures post-use level");
    expect_int("m11.i2_ticks_left_positive", s.transitionTicksLeft > 0, 1,
               "I2 — transitionTicksLeft set to a positive nominal value");
    expect_int("m11.i2_active_set", s.transitionActive, 1,
               "I2 — transitionActive=1 is the LAST write of the use sequence");

    /* I1 — concurrent use while in flight is rejected. */
    rc = m11_stairs_use(&s, 5, 6, &newX, &newY, &newFacing);
    expect_int("m11.i1_concurrent_use_rejected", rc, 0,
               "I1 — m11_stairs_use must reject while transitionActive=1");
    expect_int("m11.i1_state_unchanged_current_level",
               s.currentLevel, 1,
               "I1 — currentLevel not corrupted by concurrent call");
    expect_int("m11.i1_state_unchanged_active",
               s.transitionActive, 1,
               "I1 — transitionActive stays 1 across rejected concurrent call");

    /* I3 — tick clamps to 0 on overshoot, no negative underflow. */
    m11_stairs_tick(&s, 100000);
    expect_int("m11.i3_clamp_to_zero", s.transitionTicksLeft, 0,
               "I3 — overshoot clamp keeps transitionTicksLeft=0 (never negative)");
    expect_int("m11.i3_clamp_clears_active", s.transitionActive, 0,
               "I3 — once transitionTicksLeft hits 0, transitionActive clears");

    /* I4 — once cleared, tick is again a no-op (and stays at 0/0). */
    m11_stairs_tick(&s, 100000);
    expect_int("m11.i4_re_idempotent_after_clear",
               s.transitionActive, 0,
               "I4 — tick after settle stays a no-op");
    expect_int("m11.i4_re_idempotent_ticks_left",
               s.transitionTicksLeft, 0,
               "I4 — ticksLeft stays at 0 after settle");
}

/* ── main ────────────────────────────────────────────────────────────── */

int main(void)
{
    printf("probe=dm1_v1_dun06b_f0155_stairs_exit_direction_gate_pc34_compat\n");
    printf("sourceEvidence=ReDMCSB DUNGEON.C:F0154 lines 1508-1558 + DUNGEON.C:F0155 lines 1560-1582; "
           "CLIKMENU.C:F0364_COMMAND_TakeStairs lines 124-142\n");

    test_f0155_ns_blocked_east_returns_dir_west();
    test_f0155_ns_open_east_returns_dir_east();
    test_f0155_ew_blocked_north_returns_dir_south();
    test_f0705_returns_zero_on_corridor_square();
    test_f0705_returns_zero_when_no_target_map_covers_coord();
    test_m11_stairs_state_machine_invariants();

    printf("\nresult=%d/%d PASS\n", g_assertions - g_failures, g_assertions);
    if (g_failures > 0) {
        printf("FAIL: %d assertion(s) failed\n", g_failures);
        return 1;
    }
    return 0;
}
