/*
 * Source-lock gate for the DM1 creature wander runtime route in M11.
 *
 * ReDMCSB evidence:
 *   GROUP.C F0209, behavior == C0 (DM1_BEHAVIOR_WANDER): a group that can
 *     neither see nor smell the party rolls RANDOM(2) for a 50% move chance,
 *     picks a start direction with RANDOM(4), and validates each candidate
 *     with F0811_GROUP_IsMovementPossible before settling.
 *   F0180_GROUP_StartWandering sets that behaviour.
 *
 * Before this route existed, M11's tick loop simply returned when F0200
 * (visible) and F0201 (smell) both failed, so an unaware DM1 creature stood
 * perfectly still forever. F0810_DM1_GROUP_DispatchBehavior_Compat already
 * carried the whole source WANDER branch; only the runtime application of
 * its DM1_ACTION_MOVE was missing.
 *
 * Non-claim: this pins that the wander MOVEMENT happens and stays inside
 * the source's legal-square rules. It does not claim full F0209 path
 * parity — see m11_creature_try_wander for the prior-square limitation.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
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

#define ASSERT_TRUE(cond, msg) do { \
    if (cond) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define MAP_W 7
static uint16_t s_cumColCounts[MAP_W + 1];
static unsigned char s_rawGroup[16];

/* A 7x1 corridor. The party sits far away at x=0; the group starts at x=4
 * with open corridor on both sides, so a wander step is legal in either
 * direction and the party is neither adjacent nor reachable by the step. */
static void seed_wander_state(M11_GameViewState* state,
                              struct DungeonDatState_Compat* dungeon,
                              struct DungeonMapDesc_Compat maps[1],
                              struct DungeonMapTiles_Compat tiles[1],
                              unsigned char mapTiles[MAP_W],
                              struct DungeonThings_Compat* things,
                              struct DungeonGroup_Compat groups[1],
                              unsigned short squareFirstThings[1],
                              int sealNeighbours) {
    int i;
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat));
    memset(things, 0, sizeof(*things));
    memset(groups, 0, sizeof(struct DungeonGroup_Compat));
    memset(squareFirstThings, 0, sizeof(unsigned short));
    memset(s_cumColCounts, 0, sizeof(s_cumColCounts));

    maps[0].width = MAP_W;
    maps[0].height = 1;
    for (i = 0; i < MAP_W; ++i) {
        mapTiles[i] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    if (sealNeighbours) {
        /* Wall off both neighbours of the group square so no wander step is
         * legal; F0811 must reject every candidate. */
        mapTiles[3] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
        mapTiles[5] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    }
    mapTiles[4] |= DUNGEON_SQUARE_MASK_THING_LIST;

    tiles[0].squareData = mapTiles;
    tiles[0].squareCount = MAP_W;
    dungeon->header.mapCount = 1;
    dungeon->header.squareFirstThingCount = 1;
    for (i = 0; i <= MAP_W; ++i) {
        s_cumColCounts[i] = (uint16_t)((i > 4) ? 1 : 0);
    }
    dungeon->columnsCumulativeSquareFirstThingCount = s_cumColCounts;
    dungeon->dungeonColumnCount = MAP_W;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    groups[0].creatureType = DM1_CREATURE_TYPE_SCREAMER;
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_ENDOFLIST;
    groups[0].cells = 0;
    groups[0].count = 0;
    groups[0].direction = 0;
    groups[0].behavior = DM1_BEHAVIOR_WANDER;
    groups[0].health[0] = 40;
    things->groups = groups;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->loaded = 1;

    squareFirstThings[0] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 1;

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
    things->rawThingData[THING_TYPE_GROUP] = s_rawGroup;

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.gameTick = 16;
    state->world.masterRng.seed = 7;
    state->world.partyMapIndex = 0;
    state->world.newPartyMapIndex = 0;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.direction = 0;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
}

/* Across many RNG draws the group must actually move at least once — the
 * whole point of the fix — and every move must be a single legal step. */
static void test_wander_moves_and_stays_legal(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[MAP_W];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[1];
    int moves = 0;
    int trial;
    int x = 4;
    int y = 0;

    seed_wander_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                      groups, squareFirstThings, 0);

    for (trial = 0; trial < 64; ++trial) {
        int newX = x;
        int newY = y;
        int moved = M11_GameView_ProbeCreatureWanderStep(
            &state, (unsigned short)(THING_TYPE_GROUP << 10), 0, x, y,
            &newX, &newY);
        if (moved) {
            int dx = newX - x;
            int dy = newY - y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            ASSERT_EQ(dx + dy, 1, "wander move is exactly one square");
            ASSERT_TRUE(newX >= 0 && newX < MAP_W,
                        "wander stays inside the map");
            ASSERT_TRUE(!(newX == state.world.party.mapX &&
                          newY == state.world.party.mapY),
                        "wander never steps onto the party square");
            ++moves;
            x = newX;
            y = newY;
        } else {
            ASSERT_EQ(newX, x, "no-move keeps origin x");
            ASSERT_EQ(newY, y, "no-move keeps origin y");
        }
    }

    /* F0209 rolls a 50% move chance, so over 64 draws a total freeze is not
     * a plausible outcome — it would mean the route is still inert. */
    ASSERT_TRUE(moves > 0,
                "a wandering group actually moves (was frozen before)");
}

/* With both neighbours walled, F0811 rejects every candidate and the group
 * must stay exactly where it is. */
static void test_wander_sealed_group_cannot_move(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[MAP_W];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[1];
    int trial;

    seed_wander_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                      groups, squareFirstThings, 1);

    for (trial = 0; trial < 32; ++trial) {
        int newX = -1;
        int newY = -1;
        ASSERT_EQ(M11_GameView_ProbeCreatureWanderStep(
                      &state, (unsigned short)(THING_TYPE_GROUP << 10), 0,
                      4, 0, &newX, &newY),
                  0, "sealed group cannot wander");
        ASSERT_EQ(newX, 4, "sealed group keeps origin x");
        ASSERT_EQ(newY, 0, "sealed group keeps origin y");
    }
}

/* Gates. */
static void test_wander_probe_gates(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[MAP_W];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[1];
    int newX = 9;
    int newY = 9;

    ASSERT_EQ(M11_GameView_ProbeCreatureWanderStep(
                  NULL, (unsigned short)(THING_TYPE_GROUP << 10), 0, 4, 0,
                  &newX, &newY),
              0, "null state is rejected");

    seed_wander_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                      groups, squareFirstThings, 0);
    ASSERT_EQ(M11_GameView_ProbeCreatureWanderStep(
                  &state, (unsigned short)(THING_TYPE_GROUP << 10), -1, 4, 0,
                  &newX, &newY),
              0, "negative group index is rejected");
    ASSERT_EQ(M11_GameView_ProbeCreatureWanderStep(
                  &state, (unsigned short)(THING_TYPE_GROUP << 10), 99, 4, 0,
                  &newX, &newY),
              0, "out-of-range group index is rejected");
}

int main(void) {
    test_wander_moves_and_stays_legal();
    test_wander_sealed_group_cannot_move();
    test_wander_probe_gates();

    printf("m11 creature wander runtime: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
