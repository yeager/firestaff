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
    /* Cases run sequentially and retain only one live fixture. Keep column
     * storage alive through each probe without introducing heap ownership. */
    static unsigned short columns[2];
    int i;
    memset(columns, 0, sizeof(columns));
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
    dungeon->columnsCumulativeSquareFirstThingCount = columns;
    dungeon->dungeonColumnCount = 1;

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
    /* An unflagged square has no compact entry. The allocated table slot
     * is a trailing NONE, consumed by F0514 upon the first insertion. */
    squareFirstThings[0] = THING_NONE;
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

static void store_raw_next(unsigned char* raw, unsigned short next) {
    raw[0] = (unsigned char)(next & 0xFFu);
    raw[1] = (unsigned char)((next >> 8) & 0xFFu);
}

static void store_raw_group(unsigned char raw[16],
                            const struct DungeonGroup_Compat* group) {
    unsigned short bitfield;
    int slot;

    store_raw_next(raw, group->next);
    raw[2] = (unsigned char)(group->slot & 0xffu);
    raw[3] = (unsigned char)((group->slot >> 8) & 0xffu);
    raw[4] = group->creatureType;
    raw[5] = group->cells;
    for (slot = 0; slot < 4; ++slot) {
        raw[6 + slot * 2] = (unsigned char)(group->health[slot] & 0xffu);
        raw[7 + slot * 2] = (unsigned char)(group->health[slot] >> 8);
    }
    bitfield = (unsigned short)((group->behavior & 0x0fu) |
                                 ((group->count & 0x03u) << 5) |
                                 ((group->direction & 0x03u) << 8) |
                                 ((group->doNotDiscard & 0x01u) << 10));
    raw[14] = (unsigned char)(bitfield & 0xffu);
    raw[15] = (unsigned char)(bitfield >> 8);
}

static unsigned short raw_next_for_thing(const struct DungeonThings_Compat* things,
                                         unsigned short thing) {
    int type = THING_GET_TYPE(thing);
    int index = THING_GET_INDEX(thing);
    unsigned char* raw;
    if (!things || type < 0 || type >= 16 ||
        index < 0 || index >= things->thingCounts[type] ||
        !things->rawThingData[type]) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + (index * 4);
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
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

static int same_thing_identity(unsigned short left, unsigned short right) {
    return (left & 0x3fffu) == (right & 0x3fffu);
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
    /* G0253's red-dragon row (GROUP.C F0186 / DUNGEON.C G0253) is 8
     * unconditional DRAGON STEAK entries plus 2 carrying
     * MASK0x8000_RANDOM_DROP, which F0186 skips on M005_RANDOM(2) != 0.  A
     * full ten-steak drop is therefore seed-dependent, and this gate asserts
     * that maximum, so pin the draw that admits both flagged entries.  The
     * shared fixture seed admits only one, which left the chain one short and
     * shifted every later cell/index expectation by a position. */
    state.world.masterRng.seed = 2;

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

static void test_fixed_drops_use_compact_square_first_things(void) {
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
    unsigned short columns[1] = {0};
    unsigned char weaponRaw[8][4];
    unsigned char armourRaw[8][4];
    unsigned char junkRaw[12][4];

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);
    /* Same G0253 RANDOM_DROP pin as the sibling red-dragon gates: 8
     * unconditional steaks plus 2 admitted only on M005_RANDOM(2) == 0. */
    state.world.masterRng.seed = 2;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    dungeon.dungeonColumnCount = 1;
    /* F0514 consumes the sole trailing NONE and marks this source square. */
    squareFirstThings[0] = THING_NONE;
    ASSERT_EQ(M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
                  &state, DM1_CREATURE_TYPE_RED_DRAGON, 2, 0, 0, 0),
              10, "fixed drops materialize through the compact PC34 table");
    ASSERT_TRUE(mapTiles[0] & DUNGEON_SQUARE_MASK_THING_LIST,
                "F0514 marks the compact source square as owning a list");
    ASSERT_EQ(F0511_DUNGEON_GetSquareFirstThing_Compat(
                  &dungeon, &things, 0, 0, 0), squareFirstThings[0],
              "F0511 resolves the same compact head after materialization");
    ASSERT_EQ(count_square_chain(&things), 10,
              "all fixed drops remain reachable through the compact head");
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
    ASSERT_EQ(things.squareFirstThings[0], THING_NONE,
              "exhausted pool leaves square chain unchanged");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_WOODEN_THUD,
              "F0186 requests wooden sound even when all allocations fail");
    for (i = 0; i < 8; ++i) {
        weapons[i].next = THING_ENDOFLIST;
        weaponRaw[i][0] = 0xFEu;
        weaponRaw[i][1] = 0xFFu;
    }
    ASSERT_EQ(M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
                  &state, DM1_CREATURE_TYPE_TROLIN, 0, 0, 0, 0),
              0, "Trolin does not grow exhausted weapon pool");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_METALLIC_THUD,
              "F0186 classifies metallic sound before failed allocation");
    ASSERT_EQ(M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
                  &state, 10 /* Mummy */, 0, 0, 0, 0),
              0, "creature without fixed possessions has no fixed drops");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_METALLIC_THUD,
              "no fixed-drop attribute means no new fixed-drop sound");
}

static void test_dead_group_runtime_materializes_and_removes_group(void) {
    /* Cell per drop under the pinned seed-2 draw.  GROUP.C F0186 (MEDIA016
     * / PC34) computes each cell as
     *   ((Cell == C0xFF_SINGLE_CENTERED_CREATURE) || !M004_RANDOM(4))
     *       ? M004_RANDOM(4) : Cell
     * and fixed_possession_cell reproduces that exactly, including the ||
     * short-circuit that skips the first draw for a centred creature.  The
     * algorithm is source-verified; this array only records the resulting
     * sequence for this seed, and was re-derived when the seed was pinned to
     * admit both MASK0x8000_RANDOM_DROP entries. */
    static const int expectedCells[10] = {3, 2, 1, 2, 2, 2, 2, 2, 2, 2};
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
    unsigned short thing;
    int i;

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);
    /* G0253's red-dragon row (GROUP.C F0186 / DUNGEON.C G0253) is 8
     * unconditional DRAGON STEAK entries plus 2 carrying
     * MASK0x8000_RANDOM_DROP, which F0186 skips on M005_RANDOM(2) != 0.  A
     * full ten-steak drop is therefore seed-dependent, and this gate asserts
     * that maximum, so pin the draw that admits both flagged entries.  The
     * shared fixture seed admits only one, which left the chain one short and
     * shifted every later cell/index expectation by a position. */
    state.world.masterRng.seed = 2;
    memset(groups, 0, sizeof(groups));
    memset(groupRaw, 0, sizeof(groupRaw));
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    groups[0].cells = 2;
    groups[0].count = 0;
    groups[0].health[0] = 0;
    store_raw_group(groupRaw[0], &groups[0]);
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = &groupRaw[0][0];
    things.squareFirstThings[0] = groupThing;
    mapTiles[0] |= DUNGEON_SQUARE_MASK_THING_LIST;

    ASSERT_EQ(M11_GameView_ProbeCheckCreatureGroupDeathAndDrop(
                  &state, groupThing, 0, 0, 0),
              1, "dead group runtime drop/removal path accepted");
    ASSERT_EQ(count_square_chain(&things), 10,
              "dead group runtime path materializes red dragon fixed drops");
    ASSERT_EQ(THING_GET_TYPE(things.squareFirstThings[0]), THING_TYPE_JUNK,
              "dead group is unlinked before first fixed drop");
    ASSERT_EQ(groups[0].next, THING_NONE,
              "dead group slot is returned to source unused pool");

    thing = things.squareFirstThings[0];
    for (i = 0; i < 10; ++i) {
        ASSERT_EQ(THING_GET_TYPE(thing), THING_TYPE_JUNK,
                  "death/drop chain entry keeps source-generated junk type");
        ASSERT_EQ(THING_GET_INDEX(thing), i,
                  "death/drop chain entry preserves F0166 allocation order");
        ASSERT_EQ(THING_GET_CELL(thing), expectedCells[i],
                  "death/drop chain entry keeps source RNG cell");
        ASSERT_EQ(junks[i].type, 36,
                  "death/drop chain entry keeps dragon steak subtype");
        ASSERT_EQ(junks[i].cursed, 0,
                  "death/drop chain entry keeps dragon steaks uncursed");
        thing = next_for_thing(&things, thing);
    }
    ASSERT_EQ(thing, THING_ENDOFLIST,
              "death/drop chain terminates after generated steaks");
}

static void test_dead_trolin_inserts_fixed_drop_into_existing_object_chain(void) {
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
    unsigned short existingFloorJunk =
        (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    unsigned short carriedJunk =
        (unsigned short)((THING_TYPE_JUNK << 10) | 1);
    unsigned short fixedClub;

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);
    memset(groups, 0, sizeof(groups));
    memset(groupRaw, 0, sizeof(groupRaw));

    junks[0].next = THING_ENDOFLIST;
    junks[0].type = 25;
    store_raw_next(junkRaw[0], THING_ENDOFLIST);
    junkRaw[0][2] = 25;
    junks[1].next = THING_ENDOFLIST;
    junks[1].type = 33;
    store_raw_next(junkRaw[1], THING_ENDOFLIST);
    junkRaw[1][2] = 33;

    groups[0].next = existingFloorJunk;
    groups[0].slot = carriedJunk;
    groups[0].creatureType = DM1_CREATURE_TYPE_TROLIN;
    groups[0].cells = 1;
    groups[0].count = 0;
    groups[0].health[0] = 0;
    store_raw_group(groupRaw[0], &groups[0]);
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = &groupRaw[0][0];
    things.squareFirstThings[0] = groupThing;
    mapTiles[0] |= DUNGEON_SQUARE_MASK_THING_LIST;

    /* ReDMCSB GROUP.C:F0188:716-731 invokes F0186 before walking Slot.
     * F0186:610-645 allocates the fixed Trolin club, cell-tags it, and
     * inserts it through F0267. MOVESENS.C F0276:1654 calls F0163;
     * DUNGEON.C F0163:1829-1837 appends at the tail, not the head.
     * The pre-existing floor object therefore precedes fixed and carried drops. */
    ASSERT_EQ(M11_GameView_ProbeCheckCreatureGroupDeathAndDrop(
                  &state, groupThing, 0, 0, 0),
              1, "dead trolin runtime death/drop path accepted");

    fixedClub = (unsigned short)((1u << 14) | (THING_TYPE_WEAPON << 10) | 0);
    /* ReDMCSB GROUP.C F0188:728 re-cells EVERY carried possession with a
     * fresh M004_RANDOM(4) before F0267 moves it onto the square:
     *   L0365_T_CurrentThing =
     *       M015_THING_WITH_NEW_CELL(L0365_T_CurrentThing, M004_RANDOM(4));
     * so the raw THING word cannot be compared against the fixture's
     * original cell bits.  Compare identity (type + index) and let the
     * source own the cell. */
    ASSERT_EQ(things.squareFirstThings[0], existingFloorJunk,
              "pre-existing floor object remains first after group removal");
    ASSERT_EQ(raw_next_for_thing(&things, existingFloorJunk), fixedClub,
              "pre-existing floor object links to generated fixed club");
    ASSERT_TRUE(same_thing_identity(raw_next_for_thing(&things, fixedClub), carriedJunk),
                "fixed club precedes the later F0188 carried possession");
    ASSERT_EQ(raw_next_for_thing(&things, carriedJunk), THING_ENDOFLIST,
              "carried possession terminates object chain");
    ASSERT_EQ(weapons[0].type, 23, "generated Trolin fixed drop is club subtype");
    ASSERT_EQ(THING_GET_CELL(fixedClub), 1,
              "generated Trolin fixed club keeps deterministic source cell");
    ASSERT_EQ(groups[0].next, THING_NONE,
              "dead trolin group slot is returned to source unused pool");
}

static void test_dead_mummy_preserves_carried_tail_and_floor_chain(void) {
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
    unsigned short existingFloorJunk =
        (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    unsigned short carriedHead =
        (unsigned short)((THING_TYPE_JUNK << 10) | 1);
    unsigned short carriedTail =
        (unsigned short)((THING_TYPE_JUNK << 10) | 2);

    seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
                    weapons, armours, junks, squareFirstThings,
                    weaponRaw, armourRaw, junkRaw);
    memset(groups, 0, sizeof(groups));
    memset(groupRaw, 0, sizeof(groupRaw));

    junks[0].next = THING_ENDOFLIST;
    junks[0].type = 25;
    store_raw_next(junkRaw[0], THING_ENDOFLIST);
    junkRaw[0][2] = 25;
    junks[1].next = carriedTail;
    junks[1].type = 33;
    store_raw_next(junkRaw[1], carriedTail);
    junkRaw[1][2] = 33;
    junks[2].next = THING_ENDOFLIST;
    junks[2].type = 34;
    store_raw_next(junkRaw[2], THING_ENDOFLIST);
    junkRaw[2][2] = 34;

    groups[0].next = existingFloorJunk;
    groups[0].slot = carriedHead;
    groups[0].creatureType = 10; /* Mummy: no fixed-possession table. */
    groups[0].cells = 0;
    groups[0].count = 0;
    groups[0].health[0] = 0;
    store_raw_group(groupRaw[0], &groups[0]);
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = &groupRaw[0][0];
    things.squareFirstThings[0] = groupThing;
    mapTiles[0] |= DUNGEON_SQUARE_MASK_THING_LIST;

    /* ReDMCSB GROUP.C:F0188:724-731 walks the dead group's Slot chain and
     * inserts each carried object through F0267 before GROUP.C:F0189 removes
     * the dead group from the square. This pins the object-chain handoff when
     * there is no F0186 fixed-possession generation to hide a lost tail. */
    ASSERT_EQ(M11_GameView_ProbeCheckCreatureGroupDeathAndDrop(
                  &state, groupThing, 0, 0, 0),
              1, "dead mummy runtime death/drop path accepted");

    /* GROUP.C F0188:728 re-cells each item; F0276:1654 delegates to
     * DUNGEON.C F0163:1829-1837, which appends in original Slot order. */
    ASSERT_EQ(things.squareFirstThings[0], existingFloorJunk,
              "pre-existing floor object stays first after group unlink");
    ASSERT_TRUE(same_thing_identity(raw_next_for_thing(&things, existingFloorJunk),
                                    carriedHead),
                "existing floor tail links to original carried head");
    ASSERT_TRUE(same_thing_identity(raw_next_for_thing(&things, carriedHead),
                                    carriedTail),
                "carried head links to original carried tail");
    ASSERT_EQ(raw_next_for_thing(&things, carriedTail), THING_ENDOFLIST,
              "last carried object terminates floor chain");
    ASSERT_EQ(groups[0].slot, THING_ENDOFLIST,
              "dead mummy carried slot chain is consumed");
    ASSERT_EQ(groups[0].next, THING_NONE,
              "dead mummy group slot is returned to source unused pool");
}

static void test_killed_all_rejects_drifted_c04_or_wrong_source_square(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[2];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[12];
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[2];
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
    maps[0].width = 2;
    maps[0].height = 1;
    mapTiles[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                  DUNGEON_SQUARE_MASK_THING_LIST);
    mapTiles[1] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    tiles[0].squareCount = 2;
    squareFirstThings[0] = groupThing;
    squareFirstThings[1] = THING_NONE;
    dungeon.dungeonColumnCount = 2;
    dungeon.columnsCumulativeSquareFirstThingCount[1] = 1;
    things.squareFirstThingCount = 2;
    dungeon.header.squareFirstThingCount = 2;
    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_ENDOFLIST;
    groups[0].creatureType = 10; /* Mummy: no generated possession drops. */
    groups[0].count = 0;
    groups[0].health[0] = 0;
    store_raw_group(groupRaw[0], &groups[0]);
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.rawThingData[THING_TYPE_GROUP] = &groupRaw[0][0];
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].reserved0 = 0;

    groupRaw[0][4] = DM1_CREATURE_TYPE_RED_DRAGON;
    ASSERT_EQ(M11_GameView_ProbeCheckCreatureGroupDeathAndDrop(
                  &state, groupThing, 0, 0, 0),
              0, "F0190 rejects a C04 creature-type drift before side effects");
    ASSERT_EQ(squareFirstThings[0], groupThing,
              "C04 drift leaves the source square chain untouched");
    ASSERT_EQ(state.world.creatureAICount, 1,
              "C04 drift leaves active-group ownership intact");

    store_raw_group(groupRaw[0], &groups[0]);
    ASSERT_EQ(M11_GameView_ProbeCheckCreatureGroupDeathAndDrop(
                  &state, groupThing, 0, 1, 0),
              0, "F0190 rejects a group whose source square does not own its Thing");
    ASSERT_EQ(squareFirstThings[0], groupThing,
              "wrong source square does not unlink the real group");
    ASSERT_EQ(state.world.creatureAICount, 1,
              "wrong source square leaves LoS active-group ownership intact");
}

static void test_worm_exhausted_pool_rng(void) {
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
    unsigned char weaponRaw[8][4], armourRaw[8][4], junkRaw[12][4];
    int capacity, source, i;
    /* Source-shaped RAM fixture, not captured original gameplay. F0186
     * skips cell RNG when F0166 cannot reserve the next junk record. */
    for (capacity = 0; capacity <= 1; ++capacity)
    for (source = 0; source < 2; ++source) {
        struct RngState_Compat expected;
        int cell = source ? 255 : 2;
        seed_drop_state(&state, &dungeon, maps, tiles, mapTiles, &things,
            weapons, armours, junks, squareFirstThings, weaponRaw, armourRaw, junkRaw);
        for (i = capacity; i < 12; ++i) {
            junks[i].next = THING_ENDOFLIST;
            junkRaw[i][0] = 0xFE;
            junkRaw[i][1] = 0xFF;
        }
        expected = state.world.masterRng;
        if (capacity && (cell == 255 || !F0732_COMBAT_RngRandom_Compat(&expected, 4)))
            cell = F0732_COMBAT_RngRandom_Compat(&expected, 4);
        /* Both optional choices still draw even though no slots remain. */
        (void)F0732_COMBAT_RngRandom_Compat(&expected, 2);
        (void)F0732_COMBAT_RngRandom_Compat(&expected, 2);
        ASSERT_EQ(M11_GameView_ProbeMaterializeCreatureFixedPossessionDrops(
            &state, 15, source ? 255 : 2, 0, 0, 0), capacity,
            "worm obeys exhausted source pool");
        ASSERT_EQ(state.world.masterRng.seed, expected.seed, "runtime source RNG after allocation failure");
        ASSERT_EQ(count_square_chain(&things), capacity, "no invented pool records");
        if (capacity) {
            ASSERT_EQ(THING_GET_CELL(squareFirstThings[0]), cell, "runtime cell matches original order");
            ASSERT_EQ(junks[0].next, next_for_thing(&things, squareFirstThings[0]), "raw/decoded next agrees");
        }
    }
}

int main(void) {
    test_worm_exhausted_pool_rng();
    printf("M11 creature fixed possession runtime source-lock gate\n");
    printf("Source: ReDMCSB GROUP.C F0186/F0188, DUNGEON.C F0166, MOVESENS.C F0267\n\n");

    test_red_dragon_steaks_materialize_as_junk();
    test_animated_armour_materializes_cursed_armour_and_weapons();
    test_fixed_drops_use_compact_square_first_things();
    test_fixed_drops_do_not_append_when_pool_exhausted();
    test_dead_group_runtime_materializes_and_removes_group();
    test_dead_trolin_inserts_fixed_drop_into_existing_object_chain();
    test_dead_mummy_preserves_carried_tail_and_floor_chain();
    test_killed_all_rejects_drifted_c04_or_wrong_source_square();

    printf("\n--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
