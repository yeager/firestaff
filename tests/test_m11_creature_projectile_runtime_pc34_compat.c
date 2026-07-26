/*
 * Source-lock gate for M11 creature projectile live runtime insertion.
 *
 * ReDMCSB evidence:
 *   GROUP.C F0209 lines 2376-2387: visible same row/column in attack
 *     range can dispatch F0207 creature attack.
 *   GROUP.C F0207 lines 1695-1770: projectile payload resolves target
 *     cell, direction, special projectile thing, kinetic energy, dexterity
 *     attack, and step energy 8.
 *   PROJEXPL.C F0212 lines 76-92: projectile is linked live, first move
 *     is scheduled at game time + 1, and creature/champion launches use
 *     C48 ignore-impacts-first-movement rather than launcher C49.
 *   PROJEXPL.C F0219 lines 689-690 and MOVESENS.C lines 295-296: C48
 *     skips current-cell impact checks for that first movement.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
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

static uint16_t s_cumColCounts[5];

static void seed_projectile_runtime_state(M11_GameViewState* state,
                                          struct DungeonDatState_Compat* dungeon,
                                          struct DungeonMapDesc_Compat maps[2],
                                          struct DungeonMapTiles_Compat tiles[2],
                                          unsigned char map0Tiles[1],
                                          unsigned char map1Tiles[3],
                                          struct DungeonThings_Compat* things,
                                          struct DungeonGroup_Compat groups[1],
                                          unsigned short squareFirstThings[4]) {
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat) * 2);
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat) * 2);
    memset(things, 0, sizeof(*things));
    memset(groups, 0, sizeof(struct DungeonGroup_Compat));
    memset(squareFirstThings, 0, sizeof(unsigned short) * 4);

    maps[0].width = 1;
    maps[0].height = 1;
    maps[1].width = 3;
    maps[1].height = 1;
    maps[1].difficulty = 0;
    map0Tiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[1] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[2] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST);
    tiles[0].squareData = map0Tiles;
    tiles[0].squareCount = 1;
    tiles[1].squareData = map1Tiles;
    tiles[1].squareCount = 3;
    dungeon->header.mapCount = 2;
    dungeon->header.squareFirstThingCount = 1;
    /* Only map1 square (2,0) has the THING_LIST bit set.
     * cumColCounts[c] = number of thing-flagged squares before column c. */
    s_cumColCounts[0] = 0;  /* map0 col0: no thing-list */
    s_cumColCounts[1] = 0;  /* map1 col0: no thing-list */
    s_cumColCounts[2] = 0;  /* map1 col1: no thing-list */
    s_cumColCounts[3] = 0;  /* map1 col2: has thing-list */
    dungeon->columnsCumulativeSquareFirstThingCount = s_cumColCounts;
    dungeon->dungeonColumnCount = 4;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    groups[0].creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    groups[0].next = THING_ENDOFLIST;
    groups[0].cells = 0;
    groups[0].count = 0;
    groups[0].direction = 3; /* WEST, toward party at x=0. */
    groups[0].health[0] = 255;
    things->groups = groups;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->loaded = 1;
    squareFirstThings[0] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    squareFirstThings[1] = THING_NONE;
    squareFirstThings[2] = THING_NONE;
    squareFirstThings[3] = THING_NONE;
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 1;

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.gameTick = 28; /* Red dragon source attack cadence boundary. */
    state->world.masterRng.seed = 2; /* First RANDOM(16) permits attack-range gate. */
    state->world.partyMapIndex = 1;
    state->world.newPartyMapIndex = 1;
    state->world.party.mapIndex = 1;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.direction = 1;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
}

static void authenticate_group_c04(struct DungeonThings_Compat* things,
                                   const struct DungeonGroup_Compat* group,
                                   unsigned char rawGroup[16]) {
    memset(rawGroup, 0, 16);
    rawGroup[0] = (unsigned char)(group->next & 0xffu);
    rawGroup[1] = (unsigned char)(group->next >> 8);
    rawGroup[2] = (unsigned char)(group->slot & 0xffu);
    rawGroup[3] = (unsigned char)(group->slot >> 8);
    rawGroup[4] = group->creatureType;
    rawGroup[5] = group->cells;
    rawGroup[6] = (unsigned char)(group->health[0] & 0xffu);
    rawGroup[7] = (unsigned char)(group->health[0] >> 8);
    rawGroup[14] = (unsigned char)(group->behavior & 0x0fu);
    rawGroup[14] |= (unsigned char)((group->count & 0x03u) << 5);
    rawGroup[15] = (unsigned char)((group->direction & 0x03u) << 0);
    things->rawThingData[THING_TYPE_GROUP] = rawGroup;
}

static unsigned int scent_receipt_fingerprint(
    const DM1_V1_NeedsScentListPc34Compat* scents) {
    unsigned int hash = 2166136261u;
    unsigned int i;
#define SCENT_FNV_BYTE(value) do { hash ^= (unsigned char)(value); hash *= 16777619u; } while (0)
    SCENT_FNV_BYTE(scents->count);
    SCENT_FNV_BYTE(scents->count >> 8);
    SCENT_FNV_BYTE(scents->firstScentIndex);
    SCENT_FNV_BYTE(scents->firstScentIndex >> 8);
    SCENT_FNV_BYTE(scents->lastScentIndex);
    SCENT_FNV_BYTE(scents->lastScentIndex >> 8);
    for (i = 0; i < scents->count; ++i) {
        const DM1_V1_NeedsScentPc34Compat* scent = &scents->entries[i];
        SCENT_FNV_BYTE(scent->mapX);
        SCENT_FNV_BYTE(scent->mapX >> 8);
        SCENT_FNV_BYTE(scent->mapY);
        SCENT_FNV_BYTE(scent->mapY >> 8);
        SCENT_FNV_BYTE(scent->mapIndex);
        SCENT_FNV_BYTE(scent->mapIndex >> 8);
        SCENT_FNV_BYTE(scent->strength);
    }
#undef SCENT_FNV_BYTE
    return hash;
}

static void test_probe_inserts_creature_projectile_slot(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[4];
    const struct ProjectileInstance_Compat* p;

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);

    ASSERT_EQ(M11_GameView_ProbeCreatureProjectileRuntimeLaunch(
                  &state, (unsigned short)(THING_TYPE_GROUP << 10), 0, 2, 0),
              1, "probe creature launch accepted");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1, "live projectile slot inserted");

    p = &state.world.projectiles.entries[0];
    ASSERT_EQ(p->ownerKind, PROJECTILE_OWNER_CREATURE, "owner kind is creature");
    ASSERT_EQ(p->ownerIndex, 0, "owner index is group index");
    ASSERT_EQ(p->projectileSubtype, PROJECTILE_SUBTYPE_FIREBALL, "red dragon launches fireball");
    ASSERT_EQ(p->mapIndex, 1, "projectile starts on creature map");
    ASSERT_EQ(p->mapX, 2, "projectile starts at creature x");
    ASSERT_EQ(p->mapY, 0, "projectile starts at creature y");
    ASSERT_EQ(p->direction, 3, "projectile direction faces party");
    ASSERT_EQ(p->stepEnergy, 8, "source step energy retained");
    ASSERT_EQ(p->firstMoveGraceFlag, 1, "first move ignores current-cell impact");
    ASSERT_EQ(p->scheduledAtTick, 29, "first move scheduled at current tick plus one");
}

static void test_live_idle_tick_inserts_creature_projectile(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[4];
    unsigned char rawGroup[16];

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    state.world.gameTick = 27; /* AdvanceIdleTick moves to tick 28, then creature AI runs. */

    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "idle tick accepted");
    ASSERT_EQ(state.world.gameTick, 28, "idle tick advanced to attack cadence");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "live creature tick inserted projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].firstMoveGraceFlag, 1,
              "live creature tick projectile has first-move grace");
}

static void test_live_tick_rejects_c04_position_and_route_drift(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[4];
    unsigned char rawGroup[16];

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    rawGroup[4] = (unsigned char)(groups[0].creatureType - 1u);
    state.world.gameTick = 27;
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "C04-drift tick is handled without mutation");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "mutated C04 type cannot launch a projectile");
    ASSERT_EQ(F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 1, 2, 0),
              (THING_TYPE_GROUP << 10),
              "mutated C04 leaves the authenticated source square unchanged");

    authenticate_group_c04(&things, &groups[0], rawGroup);
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].groupMapIndex = 1;
    state.world.creatureAI[0].groupMapX = 1;
    state.world.creatureAI[0].groupMapY = 0;
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "position-drift tick is handled without mutation");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "wrong live group position cannot launch a projectile");

    state.world.creatureAI[0].groupMapX = 2;
    map1Tiles[1] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "blocked-route tick is handled");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "raw wall route blocks F0200 visibility and launch");
    ASSERT_EQ(F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 1, 2, 0),
              (THING_TYPE_GROUP << 10),
              "blocked route leaves raw group chain in its source square");
}

static void test_live_tick_moves_authenticated_group_and_ai_position(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[4];
    unsigned char rawGroup[16];

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].groupMapIndex = 1;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.gameTick = 11; /* M11's C24 profile moves on tick 12. */

    {
        M11_GameInputResult tickRes = M11_GameView_AdvanceIdleTick(&state);
        unsigned short sftAtSource = F0511_DUNGEON_GetSquareFirstThing_Compat(
            &dungeon, &things, 1, 2, 0);
        unsigned short sftAtDest = F0511_DUNGEON_GetSquareFirstThing_Compat(
            &dungeon, &things, 1, 1, 0);
        ASSERT_EQ(tickRes, M11_GAME_INPUT_REDRAW,
                  "authenticated C04 move tick is handled");
        ASSERT_EQ(sftAtSource, THING_ENDOFLIST,
                  "F0209 unlinks the group from its authenticated source square");
        ASSERT_EQ(sftAtDest, (THING_TYPE_GROUP << 10),
                  "F0209 links the group to the source-selected destination square");
    }
    ASSERT_EQ(state.world.creatureAI[0].groupMapX, 1,
              "live active-group position follows the raw C04 movement");
    ASSERT_EQ(state.world.creatureAI[0].groupMapY, 0,
              "live active-group Y remains source-consistent after movement");
}

static void test_live_tick_uses_f0201_direct_smell_route(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[4] = { 0, 0, 0, 0 };
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonDoor_Compat door;
    unsigned short squareFirstThings[5] = { 0, 0, 0, 0, 0 };
    unsigned char rawGroup[16];
    unsigned char rawDoor[4];

    /* A closed ordinary door is opaque to F0197/F0200, but F0198's smell
     * predicate only blocks walls and real closed fake walls.  The group
     * therefore smells the party through it and takes the still-passable
     * first step at x=3 -> x=2. */
    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles,
                                  map1Tiles, &things, groups, squareFirstThings);
    maps[1].width = 4;
    tiles[1].squareCount = 4;
    /* 5 columns: map0(1) + map1(4). Door at x=1, group at x=3.
     * cumColCounts[c] = thing-flagged squares before column c. */
    s_cumColCounts[0] = 0;
    s_cumColCounts[1] = 0;
    s_cumColCounts[2] = 0;  /* door is here */
    s_cumColCounts[3] = 1;  /* 1 before (door) */
    s_cumColCounts[4] = 1;  /* group is here */
    dungeon.dungeonColumnCount = 5;
    dungeon.header.squareFirstThingCount = 2;
    things.squareFirstThingCount = 2;
    map1Tiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[2] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = (unsigned short)(THING_TYPE_DOOR << 10);
    squareFirstThings[1] = (unsigned short)(THING_TYPE_GROUP << 10);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    memset(&door, 0, sizeof(door));
    memset(rawDoor, 0, sizeof(rawDoor));
    door.next = THING_ENDOFLIST;
    rawDoor[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawDoor[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    things.doors = &door;
    things.doorCount = 1;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    things.rawThingData[THING_TYPE_DOOR] = rawDoor;
    map1Tiles[1] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) |
                                   DUNGEON_SQUARE_MASK_THING_LIST | 4u);
    state.world.gameTick = 11;

    {
        unsigned short sftAtSource, sftAtDest;
        ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
                  "F0201 direct-smell tick is handled");
        ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
                  "opaque C00 blocks F0200 projectile sight");
        sftAtSource = F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 1, 3, 0);
        sftAtDest = F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 1, 2, 0);
        ASSERT_EQ(sftAtSource, THING_ENDOFLIST,
                  "F0201 unlinks group from the authenticated source square");
        ASSERT_EQ(sftAtDest, (THING_TYPE_GROUP << 10),
                  "F0198/F0199 direct smell route moves group one source step");
    }

    /* The direct route remains bound to raw C04 identity. */
    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles,
                                  map1Tiles, &things, groups, squareFirstThings);
    maps[1].width = 4;
    tiles[1].squareCount = 4;
    s_cumColCounts[0] = 0;
    s_cumColCounts[1] = 0;
    s_cumColCounts[2] = 0;
    s_cumColCounts[3] = 0;
    s_cumColCounts[4] = 0;
    dungeon.dungeonColumnCount = 5;
    dungeon.header.squareFirstThingCount = 1;
    things.squareFirstThingCount = 1;
    map1Tiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[2] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    rawGroup[4] = (unsigned char)(groups[0].creatureType - 1u);
    map1Tiles[1] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    state.world.gameTick = 11;
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "drifted C04 smell tick is handled");
    ASSERT_EQ(F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 1, 3, 0),
              (THING_TYPE_GROUP << 10),
              "drifted C04 cannot materialize a smell movement");

    /* A raw wall rejects the F0198/F0199 route before mutation; no decoded
     * scent entry is consulted as a fallback. */
    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles,
                                  map1Tiles, &things, groups, squareFirstThings);
    maps[1].width = 4;
    tiles[1].squareCount = 4;
    s_cumColCounts[0] = 0;
    s_cumColCounts[1] = 0;
    s_cumColCounts[2] = 0;
    s_cumColCounts[3] = 0;
    s_cumColCounts[4] = 0;
    dungeon.dungeonColumnCount = 5;
    dungeon.header.squareFirstThingCount = 1;
    things.squareFirstThingCount = 1;
    map1Tiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[2] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    map1Tiles[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    map1Tiles[1] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    state.world.gameTick = 11;
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "wall-blocked smell tick is handled");
    ASSERT_EQ(F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 1, 3, 0),
              (THING_TYPE_GROUP << 10),
              "F0198 raw wall rejects F0201 movement without a scent fallback");
}

static void test_m10_f0201_stored_scent_receipt(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[4];
    struct DM1GroupBehaviorContext_Compat ctx;
    struct DM1GroupSmellDirectionPlan_Compat plan;
    DM1_V1_NeedsScentListPc34Compat scents;
    unsigned int fingerprint;

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles,
                                  map1Tiles, &things, groups, squareFirstThings);
    memset(&ctx, 0, sizeof(ctx));
    memset(&plan, 0, sizeof(plan));
    memset(&scents, 0, sizeof(scents));
    ctx.currentMapIndex = 1;
    ctx.currentGroupMapX = 2;
    ctx.currentGroupMapY = 0;
    ctx.currentGroupDistanceToParty = 2;
    ctx.currentGroupPrimaryDirToParty = 3;
    ctx.currentGroupSecondaryDirToParty = 0;
    ctx.partyMapIndex = 1;
    ctx.partyMapX = 0;
    ctx.partyMapY = 0;
    ctx.creatureInfo.ranges = 0x3645; /* C24 Red Dragon: smell 6. */
    map1Tiles[1] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);

    scents.count = 1;
    scents.entries[0].mapIndex = 1;
    scents.entries[0].mapX = 0;
    scents.entries[0].mapY = 0;
    scents.entries[0].strength = 30;
    fingerprint = scent_receipt_fingerprint(&scents);
    ASSERT_EQ(F0890d_ORCH_PublishPartyScentReceipt_Compat(
                  &state.world, &scents, fingerprint), 1,
              "M10 accepts bounded source G0407 scent receipt");
    ASSERT_EQ(F0890e_ORCH_BuildGroupSmellDirectionPlan_Compat(
                  &state.world, &ctx, &plan), 1,
              "M10 F0201 resolves authenticated stored scent");
    ASSERT_EQ(plan.usedDirectPartyRoute, 0,
              "raw wall rejects the direct F0198/F0199 branch");
    ASSERT_EQ(plan.usedStoredScent, 1,
              "authenticated party-square scent is used after blocked route");
    ASSERT_EQ(plan.directionOrdinal, 4,
              "stored scent keeps source westward F0228 direction");

    state.world.pc34PartyScentReceipt.entries[0].strength--;
    memset(&plan, 0, sizeof(plan));
    ASSERT_EQ(F0890e_ORCH_BuildGroupSmellDirectionPlan_Compat(
                  &state.world, &ctx, &plan), 1,
              "receipt drift is handled without synthetic direction");
    ASSERT_EQ(plan.directionOrdinal, 0,
              "mutated receipt cannot drive stored-scent movement");
    ASSERT_EQ(plan.usedStoredScent, 0,
              "mutated receipt remains fail-closed");

    scents.entries[0].mapIndex = 7;
    fingerprint = scent_receipt_fingerprint(&scents);
    ASSERT_EQ(F0890d_ORCH_PublishPartyScentReceipt_Compat(
                  &state.world, &scents, fingerprint), 0,
              "out-of-map source scent is rejected before publication");
}

static void test_live_tick_uses_authenticated_door_info_for_sight(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonDoor_Compat door;
    unsigned short squareFirstThings[4];
    unsigned char rawGroup[16];
    unsigned char rawDoor[4];

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);
    /* Add door at (1,0): need 2 SFT entries (door + group). */
    s_cumColCounts[0] = 0;
    s_cumColCounts[1] = 0;
    s_cumColCounts[2] = 0;  /* door at map1 x=1 */
    s_cumColCounts[3] = 1;  /* group at map1 x=2 */
    dungeon.header.squareFirstThingCount = 2;
    things.squareFirstThingCount = 2;
    squareFirstThings[0] = (unsigned short)(THING_TYPE_DOOR << 10);
    squareFirstThings[1] = (unsigned short)(THING_TYPE_GROUP << 10);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    memset(&door, 0, sizeof(door));
    memset(rawDoor, 0, sizeof(rawDoor));
    door.next = THING_ENDOFLIST;
    door.type = 0; /* Map.DoorSet0: portcullis G0254 entry. */
    rawDoor[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawDoor[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    things.doors = &door;
    things.doorCount = 1;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    things.rawThingData[THING_TYPE_DOOR] = rawDoor;
    maps[1].doorSet0 = 0;
    map1Tiles[1] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) |
                                   DUNGEON_SQUARE_MASK_THING_LIST | 4u);
    state.world.gameTick = 27;

    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "portcullis LoS tick is handled");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "authenticated C00 portcullis admits F0200 sight and launch");

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);
    s_cumColCounts[0] = 0;
    s_cumColCounts[1] = 0;
    s_cumColCounts[2] = 0;
    s_cumColCounts[3] = 1;
    dungeon.header.squareFirstThingCount = 2;
    things.squareFirstThingCount = 2;
    squareFirstThings[0] = (unsigned short)(THING_TYPE_DOOR << 10);
    squareFirstThings[1] = (unsigned short)(THING_TYPE_GROUP << 10);
    authenticate_group_c04(&things, &groups[0], rawGroup);
    memset(&door, 0, sizeof(door));
    memset(rawDoor, 0, sizeof(rawDoor));
    door.next = THING_ENDOFLIST;
    door.type = 0;
    rawDoor[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawDoor[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawDoor[2] = 1; /* Raw C00 selects set1 while decoded owner claims set0. */
    things.doors = &door;
    things.doorCount = 1;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    things.rawThingData[THING_TYPE_DOOR] = rawDoor;
    maps[1].doorSet0 = 0;
    map1Tiles[1] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) |
                                   DUNGEON_SQUARE_MASK_THING_LIST | 4u);
    state.world.gameTick = 27;

    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "drifted C00 LoS tick is handled");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "drifted C00 owner remains fail-closed for F0200 sight");
}

static void test_probe_rejects_unscheduled_creature_projectile(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[4];

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);
    state.world.timeline.count = TIMELINE_QUEUE_CAPACITY;

    ASSERT_EQ(M11_GameView_ProbeCreatureProjectileRuntimeLaunch(
                  &state, (unsigned short)(THING_TYPE_GROUP << 10), 0, 2, 0),
              0, "F0212 rejects when its first C48/C49 cannot be scheduled");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "F0212 queue rejection restores the newly allocated projectile slot");
    ASSERT_EQ(state.world.projectiles.entries[0].slotIndex, -1,
              "F0813 clears the rejected projectile slot identity");
    ASSERT_EQ(state.world.timeline.count, TIMELINE_QUEUE_CAPACITY,
              "F0212 queue rejection does not alter authenticated pending events");
}


static void test_black_flame_fireball_impact_heals_and_caps(void) {
    struct DungeonGroup_Compat group;
    struct ProjectileInstance_Compat projectile;

    memset(&group, 0, sizeof(group));
    memset(&projectile, 0, sizeof(projectile));

    group.creatureType = 11; /* ReDMCSB C11_CREATURE_BLACK_FLAME */
    group.count = 0;
    group.cells = 0xFF;
    group.health[0] = 990;

    projectile.projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    projectile.attack = 15;

    ASSERT_EQ(F0890a_ORCH_ApplyProjectileCreatureImpact_Compat(&group, 0, &projectile),
              0, "black flame fireball impact does not kill");
    ASSERT_EQ(group.health[0], 1000,
              "black flame fireball impact heals and caps at 1000");

    group.health[0] = 120;
    projectile.attack = 30;
    ASSERT_EQ(F0890a_ORCH_ApplyProjectileCreatureImpact_Compat(&group, 0, &projectile),
              0, "black flame fireball impact uses heal branch");
    ASSERT_EQ(group.health[0], 150,
              "black flame fireball impact adds projectile attack to health");
}

static void test_first_move_grace_skips_source_square_impact(void) {
    struct ProjectileCreateInput_Compat input;
    struct ProjectileList_Compat list;
    struct TimelineEvent_Compat firstMove;
    struct CellContentDigest_Compat digest;
    struct ProjectileInstance_Compat next;
    struct ProjectileTickResult_Compat result;
    int slot = -1;

    memset(&input, 0, sizeof(input));
    memset(&list, 0, sizeof(list));
    memset(&digest, 0, sizeof(digest));
    input.category = PROJECTILE_CATEGORY_MAGICAL;
    input.subtype = PROJECTILE_SUBTYPE_FIREBALL;
    input.ownerKind = PROJECTILE_OWNER_CREATURE;
    input.ownerIndex = 0;
    input.mapIndex = 1;
    input.mapX = 2;
    input.mapY = 0;
    input.cell = 0;
    input.direction = 3;
    input.kineticEnergy = 40;
    input.attack = 70;
    input.stepEnergy = 8;
    input.currentTick = 12;
    input.attackTypeCode = COMBAT_ATTACK_FIRE;
    input.firstMoveGraceFlag = 1;

    ASSERT_EQ(F0810_PROJECTILE_Create_Compat(&input, &list, &slot, &firstMove), 1,
              "projectile create succeeds");
    digest.sourceMapIndex = 1;
    digest.sourceMapX = 2;
    digest.sourceMapY = 0;
    digest.destMapIndex = 1;
    digest.destMapX = 2;
    digest.destMapY = 0;
    digest.destSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    digest.destHasChampion = 1;
    digest.destChampionCellMask = 1 << input.cell;

    ASSERT_EQ(F0811_PROJECTILE_Advance_Compat(&list.entries[slot], &digest, 13,
                                              NULL, &next, &result),
              1, "first projectile advance succeeds");
    ASSERT_EQ(result.resultKind, PROJECTILE_RESULT_FLEW,
              "first move grace skips source champion impact");
    ASSERT_EQ(result.despawn, 0, "first move grace does not despawn on source impact");
    ASSERT_EQ(next.firstMoveGraceFlag, 0, "first move grace clears after first advance");
}

int main(void) {
    printf("=== M11 Creature Projectile Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: GROUP.C F0207/F0209, PROJEXPL.C F0212/F0219, MOVESENS.C C48 impact gate\n\n");

    test_probe_inserts_creature_projectile_slot();
    test_live_idle_tick_inserts_creature_projectile();
    test_live_tick_rejects_c04_position_and_route_drift();
    test_live_tick_moves_authenticated_group_and_ai_position();
    test_live_tick_uses_f0201_direct_smell_route();
    test_m10_f0201_stored_scent_receipt();
    test_live_tick_uses_authenticated_door_info_for_sight();
    test_probe_rejects_unscheduled_creature_projectile();
    test_black_flame_fireball_impact_heals_and_caps();
    test_first_move_grace_skips_source_square_impact();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
