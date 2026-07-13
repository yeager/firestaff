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
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

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
    map1Tiles[2] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    tiles[0].squareData = map0Tiles;
    tiles[0].squareCount = 1;
    tiles[1].squareData = map1Tiles;
    tiles[1].squareCount = 3;
    dungeon->header.mapCount = 2;
    dungeon->header.squareFirstThingCount = 4;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    groups[0].creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    groups[0].cells = 0;
    groups[0].count = 0;
    groups[0].direction = 3; /* WEST, toward party at x=0. */
    groups[0].health[0] = 255;
    things->groups = groups;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    squareFirstThings[0] = THING_ENDOFLIST;
    squareFirstThings[1] = THING_ENDOFLIST;
    squareFirstThings[2] = THING_ENDOFLIST;
    squareFirstThings[3] = (unsigned short)((THING_TYPE_GROUP << 10) | 0);
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 4;

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

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles, map1Tiles,
                                  &things, groups, squareFirstThings);
    state.world.gameTick = 27; /* AdvanceIdleTick moves to tick 28, then creature AI runs. */

    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "idle tick accepted");
    ASSERT_EQ(state.world.gameTick, 28, "idle tick advanced to attack cadence");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "live creature tick inserted projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].firstMoveGraceFlag, 1,
              "live creature tick projectile has first-move grace");
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

static void test_direct_f0811_impact_cleans_c14_and_c49(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonProjectile_Compat projectiles[1];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short squareFirstThings[4];
    struct TimelineEvent_Compat moveEvent;
    unsigned short projectileThing = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short weaponThing = (unsigned short)(THING_TYPE_WEAPON << 10);

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles,
                                  map1Tiles, &things, groups, squareFirstThings);
    /* F0219 sees the north/westward blocker before it can commit movement.
     * The live C14 begins at map 1, x 2; F0217 must unlink it, F0215 drops
     * its actual Slot back at x 2, and F0214 removes the queued C49. */
    map1Tiles[1] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    memset(projectiles, 0, sizeof(projectiles));
    memset(weapons, 0, sizeof(weapons));
    projectiles[0].slot = weaponThing;
    projectiles[0].next = THING_ENDOFLIST;
    weapons[0].type = 27; /* ReDMCSB WEAPON_ARROW. */
    weapons[0].next = THING_NONE;
    things.projectiles = projectiles;
    things.projectileCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    squareFirstThings[3] = projectileThing;
    memset(&state.world.projectiles, 0, sizeof(state.world.projectiles));
    state.world.projectiles.count = 1;
    state.world.projectiles.entries[0].slotIndex = 0;
    state.world.projectiles.entries[0].projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    state.world.projectiles.entries[0].projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    state.world.projectiles.entries[0].mapIndex = 1;
    state.world.projectiles.entries[0].mapX = 2;
    state.world.projectiles.entries[0].mapY = 0;
    state.world.projectiles.entries[0].cell = 0;
    state.world.projectiles.entries[0].direction = 3;
    state.world.projectiles.entries[0].kineticEnergy = 20;
    state.world.projectiles.entries[0].attack = 20;
    state.world.projectiles.entries[0].stepEnergy = 1;
    state.world.projectiles.entries[0].scheduledAtTick = 29;
    state.world.projectiles.entries[0].attackTypeCode = COMBAT_ATTACK_BLUNT;
    state.world.projectiles.entries[0].reserved1 = weaponThing;
    state.world.projectiles.entries[0].reserved3 = 1;
    memset(&moveEvent, 0, sizeof(moveEvent));
    moveEvent.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    moveEvent.fireAtTick = 29;
    moveEvent.aux0 = 0;
    state.world.timeline.events[0] = moveEvent;
    state.world.timeline.count = 1;
    state.world.gameTick = 29;

    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(state.world.projectiles.count, 0,
              "F0811 impact despawns the live runtime projectile");
    ASSERT_EQ(state.world.timeline.count, 0,
              "F0214 removes stale C48/C49 after direct M11 impact");
    ASSERT_EQ(squareFirstThings[3], weaponThing,
              "F0215 replaces C14 with the thrown weapon at the source square");
    ASSERT_EQ(projectiles[0].next, THING_NONE,
              "F0215 clears the raw C14 next link after impact");
    ASSERT_EQ(weapons[0].next, THING_ENDOFLIST,
              "F0163 makes the dropped weapon a terminal F0115 floor item");
    ASSERT_EQ((int)dm1_item_sprite_index(THING_TYPE_WEAPON, 27) >=
                  DM1_GRAPHIC_FIRST_OBJECT,
              1, "F0115 drop resolves the original M612 object graphic bank");
}

static void test_direct_f0811_multi_projectile_impact_ownership(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0Tiles[1];
    unsigned char map1Tiles[3];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonProjectile_Compat projectiles[2];
    struct DungeonWeapon_Compat weapons[1];
    unsigned short squareFirstThings[4];
    unsigned short projectile0 = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short projectile1 = (unsigned short)((THING_TYPE_PROJECTILE << 10) | 1);
    unsigned short weaponThing = (unsigned short)(THING_TYPE_WEAPON << 10);
    unsigned short fireballThing = (unsigned short)(THING_TYPE_EXPLOSION << 10);
    struct TimelineEvent_Compat* event;

    seed_projectile_runtime_state(&state, &dungeon, maps, tiles, map0Tiles,
                                  map1Tiles, &things, groups, squareFirstThings);
    map1Tiles[1] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    memset(projectiles, 0, sizeof(projectiles));
    memset(weapons, 0, sizeof(weapons));
    projectiles[0].slot = weaponThing;
    projectiles[0].next = THING_ENDOFLIST;
    projectiles[1].slot = fireballThing;
    projectiles[1].next = THING_ENDOFLIST;
    weapons[0].type = 27;
    weapons[0].next = THING_NONE;
    things.projectiles = projectiles;
    things.projectileCount = 2;
    things.weapons = weapons;
    things.weaponCount = 1;
    squareFirstThings[1] = projectile1;
    squareFirstThings[3] = projectile0;
    memset(&state.world.projectiles, 0, sizeof(state.world.projectiles));
    state.world.projectiles.count = 2;
    state.world.gameTick = 29;

    state.world.projectiles.entries[0].slotIndex = 0;
    state.world.projectiles.entries[0].projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    state.world.projectiles.entries[0].projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    state.world.projectiles.entries[0].mapIndex = 1;
    state.world.projectiles.entries[0].mapX = 2;
    state.world.projectiles.entries[0].mapY = 0;
    state.world.projectiles.entries[0].cell = 0;
    state.world.projectiles.entries[0].direction = 3;
    state.world.projectiles.entries[0].kineticEnergy = 20;
    state.world.projectiles.entries[0].attack = 20;
    state.world.projectiles.entries[0].stepEnergy = 1;
    state.world.projectiles.entries[0].scheduledAtTick = 29;
    state.world.projectiles.entries[0].attackTypeCode = COMBAT_ATTACK_BLUNT;
    state.world.projectiles.entries[0].reserved1 = weaponThing;
    state.world.projectiles.entries[0].reserved3 = 1;
    state.world.projectiles.entries[1] = state.world.projectiles.entries[0];
    state.world.projectiles.entries[1].slotIndex = 1;
    state.world.projectiles.entries[1].projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    state.world.projectiles.entries[1].projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    state.world.projectiles.entries[1].kineticEnergy = 40;
    state.world.projectiles.entries[1].attack = 40;
    state.world.projectiles.entries[1].attackTypeCode = COMBAT_ATTACK_FIRE;
    state.world.projectiles.entries[1].reserved1 = fireballThing;
    state.world.projectiles.entries[1].mapX = 0;

    memset(state.world.timeline.events, 0, sizeof(state.world.timeline.events));
    event = &state.world.timeline.events[0];
    event->kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event->fireAtTick = 29;
    event->aux0 = 0;
    event = &state.world.timeline.events[1];
    event->kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event->fireAtTick = 29;
    event->aux0 = 1;
    event = &state.world.timeline.events[2];
    event->kind = TIMELINE_EVENT_ENABLE_CHAMPION_ACTION;
    event->fireAtTick = 35;
    event->aux0 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    event->aux2 = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    event->aux4 = 0;
    state.world.timeline.count = 3;

    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(state.world.projectiles.count, 0,
              "same-tick F0811 impacts consume both runtime projectiles");
    ASSERT_EQ(squareFirstThings[3], weaponThing,
              "only the thrown weapon materializes after mixed impacts");
    ASSERT_EQ(squareFirstThings[1], THING_ENDOFLIST,
              "fireball leaves no static Thing after its separate impact");
    ASSERT_EQ(projectiles[0].next, THING_NONE,
              "thrown route C14 is cleared after its own F0215");
    ASSERT_EQ(projectiles[1].next, THING_NONE,
              "fireball route C14 is cleared after its own F0215");
    ASSERT_EQ(weapons[0].next, THING_ENDOFLIST,
              "thrown weapon remains terminal after both C14 cleanups");
    ASSERT_EQ(state.world.timeline.count, 1,
              "both C48/C49 events are removed without broad queue cleanup");
    ASSERT_EQ(state.world.timeline.events[0].kind,
              TIMELINE_EVENT_ENABLE_CHAMPION_ACTION,
              "unrelated C11 action receipt survives projectile cleanup");
    ASSERT_EQ(state.world.timeline.events[0].aux4, 0,
              "surviving C11 retains its champion owner");
}

int main(void) {
    printf("=== M11 Creature Projectile Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: GROUP.C F0207/F0209, PROJEXPL.C F0212/F0219, MOVESENS.C C48 impact gate\n\n");

    test_probe_inserts_creature_projectile_slot();
    test_live_idle_tick_inserts_creature_projectile();
    test_black_flame_fireball_impact_heals_and_caps();
    test_first_move_grace_skips_source_square_impact();
    test_direct_f0811_impact_cleans_c14_and_c49();
    test_direct_f0811_multi_projectile_impact_ownership();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
