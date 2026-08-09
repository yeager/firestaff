/*
 * Source-lock gate for the DM1 Giggler steal runtime route in M11.
 *
 * ReDMCSB evidence:
 *   GROUP.C F0193_GROUP_StealFromChampion: a Giggler (C02) attempts a
 *     dexterity-gated steal instead of resolving an ordinary melee attack,
 *     walking the G0025 Graphic562_StealFromSlotIndices slot table
 *     (DATA.C:244-251), then flees on success.
 *   PROJEXPL.C F0215 / DUNGEON.C F0163: possession things attach to the
 *     group's GROUP.Slot chain through the shared tail-link policy.
 *
 * Before this route existed, DM1's C02 Giggler dealt plain melee damage and
 * never took an item, even though F0822 was already source-locked and wired
 * for CSB. These assertions pin: the non-Giggler rejection, the stolen thing
 * leaving the champion's inventory and entering the group's possession
 * chain, and the flee behaviour handoff to the F0820 branch.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
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

/* DEFS.H:786 C10_SLOT_NECK — the first entry of the G0025 steal table. */
#define GIGGLER_TEST_NECK_SLOT 10

static uint16_t s_cumColCounts[2];
static unsigned char s_rawGroup[16];
static unsigned char s_rawWeapon[4];

/* Single 1x1 corridor. Party and the Giggler group share the square. The
 * champion carries one weapon thing in an occupied inventory slot. */
static void seed_steal_state(M11_GameViewState* state,
                             struct DungeonDatState_Compat* dungeon,
                             struct DungeonMapDesc_Compat maps[1],
                             struct DungeonMapTiles_Compat tiles[1],
                             unsigned char mapTiles[1],
                             struct DungeonThings_Compat* things,
                             struct DungeonGroup_Compat groups[1],
                             struct DungeonWeapon_Compat weapons[1],
                             unsigned short squareFirstThings[1],
                             int creatureType,
                             int championDexterity) {
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat));
    memset(things, 0, sizeof(*things));
    memset(groups, 0, sizeof(struct DungeonGroup_Compat));
    memset(weapons, 0, sizeof(struct DungeonWeapon_Compat));
    memset(squareFirstThings, 0, sizeof(unsigned short));
    memset(s_cumColCounts, 0, sizeof(s_cumColCounts));

    maps[0].width = 1;
    maps[0].height = 1;
    mapTiles[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                  DUNGEON_SQUARE_MASK_THING_LIST);
    tiles[0].squareData = mapTiles;
    tiles[0].squareCount = 1;
    dungeon->header.mapCount = 1;
    dungeon->header.squareFirstThingCount = 1;
    s_cumColCounts[0] = 0;
    s_cumColCounts[1] = 1;
    dungeon->columnsCumulativeSquareFirstThingCount = s_cumColCounts;
    dungeon->dungeonColumnCount = 1;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    groups[0].creatureType = (unsigned char)creatureType;
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_ENDOFLIST;   /* empty possession chain */
    groups[0].cells = 0;
    groups[0].count = 0;
    groups[0].direction = 0;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    groups[0].health[0] = 30;
    things->groups = groups;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;

    weapons[0].next = THING_ENDOFLIST;
    weapons[0].type = 2;
    things->weapons = weapons;
    things->weaponCount = 1;
    things->thingCounts[THING_TYPE_WEAPON] = 1;
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

    memset(s_rawWeapon, 0, sizeof(s_rawWeapon));
    s_rawWeapon[0] = (unsigned char)(weapons[0].next & 0xffu);
    s_rawWeapon[1] = (unsigned char)(weapons[0].next >> 8);
    s_rawWeapon[2] = weapons[0].type;
    things->rawThingData[THING_TYPE_WEAPON] = s_rawWeapon;

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.gameTick = 4;
    state->world.masterRng.seed = 3;
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
    state->world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] =
        (unsigned short)championDexterity;
    /* All slots empty except the NECK slot, which holds the weapon thing.
     * The G0025 Graphic562_StealFromSlotIndices table (DATA.C:244-251) only
     * ever names NECK(10), POUCH_1(11), QUIVER_LINE1_1(12),
     * BACKPACK_LINE1_1(13) and POUCH_2(6); a thing in any other slot is by
     * source design unstealable, so the fixture must use a listed slot. */
    {
        int s;
        for (s = 0; s < CHAMPION_SLOT_COUNT; ++s) {
            state->world.party.champions[0].inventory[s] = THING_NONE;
        }
        state->world.party.champions[0].inventory[GIGGLER_TEST_NECK_SLOT] =
            (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    }
}

/* Count things reachable from the group's GROUP.Slot possession chain. */
static int group_possession_count(const struct DungeonThings_Compat* things,
                                  const struct DungeonGroup_Compat* group) {
    unsigned short current;
    int count = 0;
    if (!things || !group) return 0;
    current = group->slot;
    while (current != THING_NONE && current != THING_ENDOFLIST && count < 64) {
        ++count;
        if (THING_GET_TYPE(current) == THING_TYPE_WEAPON) {
            int idx = THING_GET_INDEX(current);
            if (idx < 0 || idx >= things->weaponCount) break;
            current = things->weapons[idx].next;
        } else {
            break;
        }
    }
    return count;
}

/* A non-Giggler group must be rejected outright — it keeps the ordinary
 * melee route. */
static void test_non_giggler_is_rejected(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short squareFirstThings[1];

    seed_steal_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                     groups, weapons, squareFirstThings,
                     DM1_CREATURE_TYPE_SCREAMER, 20);

    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, 0, 0),
              0, "non-Giggler group is rejected");
    ASSERT_EQ(state.world.party.champions[0].inventory[GIGGLER_TEST_NECK_SLOT],
              (unsigned short)((THING_TYPE_WEAPON << 10) | 0),
              "non-Giggler leaves the inventory slot untouched");
    ASSERT_EQ(groups[0].slot, THING_ENDOFLIST,
              "non-Giggler leaves the possession chain empty");
}

/* Seed 2 with dexterity 1 is the pinned source draw that makes F0822 both
 * steal the NECK slot (mask 0x400, count 1) and set the flee behaviour
 * (newBehavior 5 = DM1_BEHAVIOR_FLEE). This exercises the whole route:
 * inventory -> GROUP.Slot possession chain -> F0820 flee handoff. */
static void test_giggler_steal_moves_thing_and_flees(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short squareFirstThings[1];
    seed_steal_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                     groups, weapons, squareFirstThings,
                     DM1_CREATURE_TYPE_GIGGLER, 1);
    state.world.masterRng.seed = 2;

    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, 0, 0),
              1, "Giggler steal receipt resolves");

    /* The thing left the champion. */
    ASSERT_EQ(state.world.party.champions[0].inventory[GIGGLER_TEST_NECK_SLOT],
              THING_NONE, "stolen thing leaves the champion NECK slot");
    /* ...and arrived in the group's possession chain, not nowhere. */
    ASSERT_EQ(group_possession_count(&things, &groups[0]), 1,
              "stolen thing enters the group possession chain");
    ASSERT_EQ(groups[0].slot,
              (unsigned short)((THING_TYPE_WEAPON << 10) | 0),
              "stolen weapon becomes the group possession head");
    /* F0193 flees after this successful steal, handing off to F0820. */
    ASSERT_EQ((int)groups[0].behavior, DM1_BEHAVIOR_FLEE,
              "successful steal sets the F0820 flee behaviour");
}

/* A thing in a slot the G0025 table never names must not be stealable,
 * whatever the dexterity or RNG draw. */
static void test_giggler_cannot_steal_unlisted_slot(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short squareFirstThings[1];

    seed_steal_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                     groups, weapons, squareFirstThings,
                     DM1_CREATURE_TYPE_GIGGLER, 1);
    /* Move the weapon to slot 0, which the G0025 table never names. */
    state.world.party.champions[0].inventory[GIGGLER_TEST_NECK_SLOT] =
        THING_NONE;
    state.world.party.champions[0].inventory[0] =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    state.world.masterRng.seed = 2;

    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, 0, 0),
              1, "unlisted-slot steal still resolves a receipt");
    ASSERT_EQ(state.world.party.champions[0].inventory[0],
              (unsigned short)((THING_TYPE_WEAPON << 10) | 0),
              "a thing outside the G0025 table is never stolen");
    ASSERT_EQ(group_possession_count(&things, &groups[0]), 0,
              "unlisted slot yields no group possession");
}

/* A champion with nothing to take must not yield a possession. */
static void test_giggler_empty_inventory_takes_nothing(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short squareFirstThings[1];
    int s;

    seed_steal_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                     groups, weapons, squareFirstThings,
                     DM1_CREATURE_TYPE_GIGGLER, 1);
    for (s = 0; s < CHAMPION_SLOT_COUNT; ++s) {
        state.world.party.champions[0].inventory[s] = THING_NONE;
    }

    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, 0, 0),
              1, "empty-inventory steal still resolves a receipt");
    ASSERT_EQ(group_possession_count(&things, &groups[0]), 0,
              "empty inventory yields no group possession");
}

/* Gates. */
static void test_probe_gates(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short squareFirstThings[1];

    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(NULL, 0, 0),
              0, "null state is rejected");

    seed_steal_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                     groups, weapons, squareFirstThings,
                     DM1_CREATURE_TYPE_GIGGLER, 10);
    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, -1, 0),
              0, "negative group index is rejected");
    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, 99, 0),
              0, "out-of-range group index is rejected");
    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, 0, -1),
              0, "negative champion index is rejected");
    ASSERT_EQ(M11_GameView_ProbeGigglerStealFromChampion(&state, 0, 99),
              0, "out-of-range champion index is rejected");
}

int main(void) {
    test_non_giggler_is_rejected();
    test_giggler_steal_moves_thing_and_flees();
    test_giggler_cannot_steal_unlisted_slot();
    test_giggler_empty_inventory_takes_nothing();
    test_probe_gates();

    printf("m11 giggler steal runtime: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
