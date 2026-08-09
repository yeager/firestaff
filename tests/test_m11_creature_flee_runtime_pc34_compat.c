/*
 * Source-lock gate for the M11 creature flee runtime route.
 *
 * ReDMCSB evidence:
 *   GROUP.C F0190 GetDamageCreatureOutcome: a fear test that succeeds sets
 *     the group behaviour to 5 (DM1_BEHAVIOR_FLEE).
 *   GROUP.C F0209 label T0209094_FleeFromTarget: a fleeing group takes
 *     M018_OPPOSITE of both the primary and the secondary toward-party
 *     direction, then moves along that inverted pair.
 *
 * Before this route existed, M11's tick loop had no DM1_BEHAVIOR_FLEE
 * branch: a frightened group kept approaching the party, so F0821's fear
 * outcome was invisible in the live runtime. These assertions pin the
 * inversion (the group must retreat, never step toward the party) and the
 * blocked-retreat case (no move, origin preserved).
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static uint16_t s_cumColCounts[6];
static unsigned char s_rawGroup[16];

/* One 5x1 corridor map. Party sits at x=0; the group sits at x=2 with a
 * free corridor square at x=3 behind it, so a correct flee moves it AWAY
 * (x=3) and an incorrect approach would move it toward the party (x=1). */
static void seed_flee_state(M11_GameViewState* state,
                            struct DungeonDatState_Compat* dungeon,
                            struct DungeonMapDesc_Compat maps[1],
                            struct DungeonMapTiles_Compat tiles[1],
                            unsigned char mapTiles[5],
                            struct DungeonThings_Compat* things,
                            struct DungeonGroup_Compat groups[1],
                            unsigned short squareFirstThings[2],
                            int retreatSquareIsWall) {
    int i;
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat));
    memset(things, 0, sizeof(*things));
    memset(groups, 0, sizeof(struct DungeonGroup_Compat));
    memset(squareFirstThings, 0, sizeof(unsigned short) * 2);
    memset(s_cumColCounts, 0, sizeof(s_cumColCounts));

    maps[0].width = 5;
    maps[0].height = 1;
    for (i = 0; i < 5; ++i) {
        mapTiles[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    if (retreatSquareIsWall) {
        /* Seal x=3 so the source retreat square is not walkable. */
        mapTiles[3] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    }
    /* Only the group square (x=2) carries a thing list. */
    mapTiles[2] |= DUNGEON_SQUARE_MASK_THING_LIST;

    tiles[0].squareData = mapTiles;
    tiles[0].squareCount = 5;
    dungeon->header.mapCount = 1;
    dungeon->header.squareFirstThingCount = 1;
    /* cumColCounts[c] = thing-flagged squares before column c; only col2. */
    s_cumColCounts[0] = 0;
    s_cumColCounts[1] = 0;
    s_cumColCounts[2] = 0;
    s_cumColCounts[3] = 1;
    s_cumColCounts[4] = 1;
    s_cumColCounts[5] = 1;
    dungeon->columnsCumulativeSquareFirstThingCount = s_cumColCounts;
    dungeon->dungeonColumnCount = 5;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    groups[0].creatureType = DM1_CREATURE_TYPE_SCREAMER;
    groups[0].next = THING_ENDOFLIST;
    groups[0].cells = 0;
    groups[0].count = 0;
    groups[0].direction = 3; /* WEST, still facing the party. */
    groups[0].behavior = DM1_BEHAVIOR_FLEE;
    groups[0].health[0] = 40;
    things->groups = groups;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->loaded = 1;
    squareFirstThings[0] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    squareFirstThings[1] = THING_NONE;
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 1;

    /* Authenticate the C04 group record so the live-position source guard
     * accepts the group at (2,0). */
    memset(s_rawGroup, 0, sizeof(s_rawGroup));
    s_rawGroup[0] = (unsigned char)(groups[0].next & 0xffu);
    s_rawGroup[1] = (unsigned char)(groups[0].next >> 8);
    s_rawGroup[2] = (unsigned char)(groups[0].slot & 0xffu);
    s_rawGroup[3] = (unsigned char)(groups[0].slot >> 8);
    s_rawGroup[4] = groups[0].creatureType;
    s_rawGroup[5] = groups[0].cells;
    s_rawGroup[6] = (unsigned char)(groups[0].health[0] & 0xffu);
    s_rawGroup[7] = (unsigned char)(groups[0].health[0] >> 8);
    s_rawGroup[14] = (unsigned char)(groups[0].behavior & 0x0fu);
    s_rawGroup[14] |= (unsigned char)((groups[0].count & 0x03u) << 5);
    s_rawGroup[15] = (unsigned char)((groups[0].direction & 0x03u) << 0);
    things->rawThingData[THING_TYPE_GROUP] = s_rawGroup;

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.gameTick = 8;
    state->world.masterRng.seed = 1;
    state->world.partyMapIndex = 0;
    state->world.newPartyMapIndex = 0;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.direction = 1;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
}

/* F0820 pure-contract check: the flee pair must be the exact inversion of
 * the toward-party pair, independent of any M11 state. */
static void test_f0820_inverts_toward_party_pair(void) {
    struct DM1GroupBehaviorContext_Compat ctx;
    int primary = -1;
    int secondary = -1;
    int d;

    for (d = 0; d < 4; ++d) {
        memset(&ctx, 0, sizeof(ctx));
        ctx.currentGroupPrimaryDirToParty = d;
        ctx.currentGroupSecondaryDirToParty = (d + 1) & 3;
        ASSERT_EQ(F0820_DM1_GROUP_GetFleeDirection_Compat(
                      &ctx, &primary, &secondary),
                  1, "F0820 resolves a flee pair");
        ASSERT_EQ(primary, (d + 2) & 3, "flee primary is opposite of toward");
        ASSERT_EQ(secondary, (((d + 1) & 3) + 2) & 3,
                  "flee secondary is opposite of toward secondary");
    }
    /* Null gates fail closed. */
    memset(&ctx, 0, sizeof(ctx));
    ASSERT_EQ(F0820_DM1_GROUP_GetFleeDirection_Compat(NULL, &primary, &secondary),
              0, "F0820 rejects a null context");
    ASSERT_EQ(F0820_DM1_GROUP_GetFleeDirection_Compat(&ctx, NULL, &secondary),
              0, "F0820 rejects a null primary out");
    ASSERT_EQ(F0820_DM1_GROUP_GetFleeDirection_Compat(&ctx, &primary, NULL),
              0, "F0820 rejects a null secondary out");
}

/* The live M11 route must retreat the group away from the party. */
static void test_flee_step_moves_away_from_party(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[5];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[2];
    int newX = -1;
    int newY = -1;
    int moved;

    seed_flee_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    groups, squareFirstThings, 0);

    moved = M11_GameView_ProbeCreatureFleeStep(
        &state, (unsigned short)(THING_TYPE_GROUP << 10), 2, 0, &newX, &newY);

    ASSERT_EQ(moved, 1, "fleeing group relocates");
    ASSERT_EQ(newY, 0, "flee stays on the corridor row");
    /* The party is at x=0, so a correct retreat increases x. A regression
     * that reused the approach route would land on x=1. */
    ASSERT_EQ(newX, 3, "flee moves away from the party, not toward it");
    ASSERT_EQ(newX > 2, 1, "flee never decreases distance to the party");
}

/* A sealed retreat square must leave the group in place, not fall back to
 * an approach step. */
static void test_flee_blocked_leaves_group_in_place(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[5];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[2];
    int newX = -1;
    int newY = -1;
    int moved;

    seed_flee_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    groups, squareFirstThings, 1);

    moved = M11_GameView_ProbeCreatureFleeStep(
        &state, (unsigned short)(THING_TYPE_GROUP << 10), 2, 0, &newX, &newY);

    ASSERT_EQ(moved, 0, "blocked retreat does not move the group");
    ASSERT_EQ(newX, 2, "blocked retreat preserves the origin x");
    ASSERT_EQ(newY, 0, "blocked retreat preserves the origin y");
}

/* Null-state gate. */
static void test_flee_probe_null_state(void) {
    int newX = 7;
    int newY = 9;
    ASSERT_EQ(M11_GameView_ProbeCreatureFleeStep(
                  NULL, (unsigned short)(THING_TYPE_GROUP << 10), 2, 0,
                  &newX, &newY),
              0, "flee probe rejects a null state");
}

int main(void) {
    test_f0820_inverts_toward_party_pair();
    test_flee_step_moves_away_from_party();
    test_flee_blocked_leaves_group_in_place();
    test_flee_probe_null_state();

    printf("m11 creature flee runtime: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
