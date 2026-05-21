/*
 * Source-lock gate for M11 creature fixed-possession runtime materialization.
 *
 * ReDMCSB evidence:
 *   GROUP.C F0186 lines 610-645 resolves random fixed possessions, allocates
 *     each object via F0166_DUNGEON_GetUnusedThing, writes Type/Cursed through
 *     the object data pointer, cell-tags the thing, and calls F0267 from
 *     CM1_MAPX_NOT_ON_A_SQUARE to materialize it on the destination square.
 *   GROUP.C F0188 lines 716-731 calls F0186 for each creature before dropping
 *     the existing group possession chain.
 *   DUNGEON.C F0166 lines 2077-2137 scans fixed thing arrays for Next=NONE,
 *     clears the slot, and marks Next=ENDOFLIST; it does not grow pools.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"

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

static void mark_raw_unused(unsigned char* raw, int count) {
    int i;
    for (i = 0; i < count; ++i) {
        raw[i * 4 + 0] = 0xFFu;
        raw[i * 4 + 1] = 0xFFu;
        raw[i * 4 + 2] = 0;
        raw[i * 4 + 3] = 0;
    }
}

static void seed_drop_state(M11_GameViewState* state,
                            struct DungeonDatState_Compat* dungeon,
                            struct DungeonMapDesc_Compat maps[1],
                            struct DungeonMapTiles_Compat tiles[1],
                            unsigned char mapTiles[1],
                            struct DungeonThings_Compat* things,
                            struct DungeonWeapon_Compat weapons[8],
                            struct DungeonArmour_Compat armours[8],
                            struct DungeonJunk_Compat junks[12],
                            unsigned short squareFirstThings[1],
                            unsigned char weaponRaw[8][4],
                            unsigned char armourRaw[8][4],
                            unsigned char junkRaw[12][4]) {
    int i;
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat));
    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(struct DungeonWeapon_Compat) * 8);
    memset(armours, 0, sizeof(struct DungeonArmour_Compat) * 8);
    memset(junks, 0, sizeof(struct DungeonJunk_Compat) * 12);
    memset(weaponRaw, 0, 8 * 4);
    memset(armourRaw, 0, 8 * 4);
    memset(junkRaw, 0, 12 * 4);

    maps[0].width = 1;
    maps[0].height = 1;
    mapTiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    tiles[0].squareData = mapTiles;
    tiles[0].squareCount = 1;
    dungeon->header.mapCount = 1;
    dungeon->header.squareFirstThingCount = 1;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    for (i = 0; i < 8; ++i) {
        weapons[i].next = THING_NONE;
        armours[i].next = THING_NONE;
    }
    for (i = 0; i < 12; ++i) {
        junks[i].next = THING_NONE;
    }
    mark_raw_unused(&weaponRaw[0][0], 8);
    mark_raw_unused(&armourRaw[0][0], 8);
    mark_raw_unused(&junkRaw[0][0], 12);

    things->weapons = weapons;
    things->weaponCount = 8;
    things->armours = armours;
    things->armourCount = 8;
    things->junks = junks;
    things->junkCount = 12;
    things->thingCounts[THING_TYPE_WEAPON] = 8;
    things->thingCounts[THING_TYPE_ARMOUR] = 8;
    things->thingCounts[THING_TYPE_JUNK] = 12;
    things->rawThingData[THING_TYPE_WEAPON] = &weaponRaw[0][0];
    things->rawThingData[THING_TYPE_ARMOUR] = &armourRaw[0][0];
    things->rawThingData[THING_TYPE_JUNK] = &junkRaw[0][0];
    squareFirstThings[0] = THING_ENDOFLIST;
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 1;
    things->loaded = 1;

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.masterRng.seed = 8;
    state->audioState.initialized = 1;
}

static unsigned short next_for_thing(const struct DungeonThings_Compat* things,
                                     unsigned short thing) {
    int type = THING_GET_TYPE(thing);
    int index = THING_GET_INDEX(thing);
    switch (type) {
    case THING_TYPE_WEAPON: return things->weapons[index].next;
    case THING_TYPE_ARMOUR: return things->armours[index].next;
    case THING_TYPE_JUNK: return things->junks[index].next;
    default: return THING_ENDOFLIST;
    }
}

static int count_square_chain(const struct DungeonThings_Compat* things) {
    unsigned short thing = things->squareFirstThings[0];
    int count = 0;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && count < 32) {
        ++count;
        thing = next_for_thing(things, thing);
    }
    return count;
}

static void test_red_dragon_steaks_materialize_as_junk(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[12];
    unsigned short squareFirstThings[1];
    unsigned char weaponRaw[8][4];
    unsigned char armourRaw[8][4];
    unsigned char junkRaw[12][4];
    unsigned short thing;
    int i;

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);

    ASSERT_EQ(M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
                  &state, DM1_CREATURE_TYPE_RED_DRAGON, 2, 0, 0, 0),
              10, "red dragon materializes ten fixed possession drops");
    ASSERT_EQ(count_square_chain(&things), 10, "red dragon links ten things to square");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_WOODEN_THUD,
              "red dragon fixed possessions emit wooden thud source sound");

    thing = things.squareFirstThings[0];
    for (i = 0; i < 10; ++i) {
        ASSERT_EQ(THING_GET_TYPE(thing), THING_TYPE_JUNK,
                  "red dragon fixed drop thing type is junk");
        ASSERT_EQ(junks[THING_GET_INDEX(thing)].type, 36,
                  "red dragon fixed drop subtype is dragon steak");
        ASSERT_TRUE(THING_GET_CELL(thing) <= 3,
                    "red dragon fixed drop has encoded cell");
        thing = next_for_thing(&things, thing);
    }
}

static void test_animated_armour_materializes_cursed_armour_and_weapons(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[12];
    unsigned short squareFirstThings[1];
    unsigned char weaponRaw[8][4];
    unsigned char armourRaw[8][4];
    unsigned char junkRaw[12][4];

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);
    state.world.masterRng.seed = 1;

    ASSERT_EQ(M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
                  &state, DM1_CREATURE_TYPE_ANIMATED_ARMOUR, 2, 0, 0, 0),
              6, "animated armour materializes six fixed possession drops");
    ASSERT_EQ(count_square_chain(&things), 6, "animated armour links six things to square");
    ASSERT_EQ(armours[0].type, 41, "first armour slot is foot plate");
    ASSERT_EQ(armours[0].cursed, 1, "fixed armour drop is cursed");
    ASSERT_EQ(weapons[0].type, 10, "first weapon slot is sword");
    ASSERT_EQ(weapons[0].cursed, 1, "fixed weapon drop is cursed");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_METALLIC_THUD,
              "animated armour fixed possessions emit metallic thud source sound");
}

static void test_fixed_drops_do_not_append_when_pool_exhausted(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[12];
    unsigned short squareFirstThings[1];
    unsigned char weaponRaw[8][4];
    unsigned char armourRaw[8][4];
    unsigned char junkRaw[12][4];
    int i;

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);
    for (i = 0; i < 12; ++i) {
        junks[i].next = THING_ENDOFLIST;
        junkRaw[i][0] = 0xFEu;
        junkRaw[i][1] = 0xFFu;
    }

    ASSERT_EQ(M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
                  &state, DM1_CREATURE_TYPE_RED_DRAGON,
                  DM1_SINGLE_CENTERED_CREATURE_CELL, 0, 0, 0),
              0, "red dragon does not grow exhausted junk pool");
    ASSERT_EQ(things.squareFirstThings[0], THING_ENDOFLIST,
              "exhausted pool leaves square chain unchanged");
}

static void test_dead_group_runtime_materializes_and_removes_group(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[12];
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[1];
    unsigned char weaponRaw[8][4];
    unsigned char armourRaw[8][4];
    unsigned char junkRaw[12][4];
    unsigned char groupRaw[1][16];
    unsigned short groupThing = (unsigned short)(THING_TYPE_GROUP << 10);

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);
    memset(groups, 0, sizeof(groups));
    memset(groupRaw, 0, sizeof(groupRaw));
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    groups[0].cells = 2;
    groups[0].count = 0;
    groups[0].health[0] = 0;
    groupRaw[0][0] = 0xFEu;
    groupRaw[0][1] = 0xFFu;
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = &groupRaw[0][0];
    things.squareFirstThings[0] = groupThing;

    ASSERT_EQ(M11_GameView_ProbeCheckCreatureGroupDeathAndDrop(
                  &state, groupThing, 0, 0, 0),
              1, "dead group runtime drop/removal path accepted");
    ASSERT_EQ(count_square_chain(&things), 10,
              "dead group runtime path materializes red dragon fixed drops");
    ASSERT_EQ(THING_GET_TYPE(things.squareFirstThings[0]), THING_TYPE_JUNK,
              "dead group is unlinked before first fixed drop");
    ASSERT_EQ(groups[0].next, THING_NONE,
              "dead group slot is returned to source unused pool");
}

int main(void) {
    printf("M11 creature fixed possession runtime source-lock gate\n");
    printf("Source: ReDMCSB GROUP.C F0186/F0188, DUNGEON.C F0166, MOVESENS.C F0267\n\n");

    test_red_dragon_steaks_materialize_as_junk();
    test_animated_armour_materializes_cursed_armour_and_weapons();
    test_fixed_drops_do_not_append_when_pool_exhausted();
    test_dead_group_runtime_materializes_and_removes_group();

    printf("\n--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
