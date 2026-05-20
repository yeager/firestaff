/*
 * Source-lock gate for live DM1 V1 Open Door spell door-impact routing.
 *
 * ReDMCSB evidence:
 *   MENU.C lines 66 and 1867-1870: ZO / Open Door is projectile spell type 4
 *     and doubles skill before F0327 projectile creation.
 *   PROJEXPL.C lines 471-489: C0xFF84 OPEN_DOOR impacts any non-destroyed
 *     door before normal open/pass-through checks; button doors enqueue
 *     C10_EVENT_DOOR / C02_EFFECT_TOGGLE for GameTime+1.
 *   PROJEXPL.C lines 491-508 and 1554-1599: the normal door-destruction
 *     branch is separate and is not used by OPEN_DOOR.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_door_action_pc34_compat.h"

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

static void seed_open_door_spell_state(M11_GameViewState* state,
                                       struct DungeonDatState_Compat* dungeon,
                                       struct DungeonMapDesc_Compat maps[1],
                                       struct DungeonMapTiles_Compat tiles[1],
                                       unsigned char mapTiles[2],
                                       struct DungeonThings_Compat* things,
                                       struct DungeonDoor_Compat doors[1],
                                       unsigned short squareFirstThings[2]) {
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat firstMove;
    int slot = -1;

    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat));
    memset(things, 0, sizeof(*things));
    memset(doors, 0, sizeof(struct DungeonDoor_Compat));
    memset(squareFirstThings, 0, sizeof(unsigned short) * 2);

    maps[0].width = 2;
    maps[0].height = 1;
    mapTiles[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    mapTiles[1] = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 4);
    tiles[0].squareData = mapTiles;
    tiles[0].squareCount = 2;
    dungeon->header.mapCount = 1;
    dungeon->header.squareFirstThingCount = 2;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    doors[0].next = THING_ENDOFLIST;
    doors[0].button = 1;
    things->doors = doors;
    things->doorCount = 1;
    things->thingCounts[THING_TYPE_DOOR] = 1;
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 2;
    things->loaded = 1;
    squareFirstThings[0] = THING_ENDOFLIST;
    squareFirstThings[1] = (unsigned short)((THING_TYPE_DOOR << 10) | 0);

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.gameTick = 10;
    state->world.partyMapIndex = 0;
    state->world.newPartyMapIndex = 0;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.direction = 1;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;

    memset(&input, 0, sizeof(input));
    input.category = PROJECTILE_CATEGORY_MAGICAL;
    input.subtype = PROJECTILE_SUBTYPE_OPEN_DOOR;
    input.ownerKind = PROJECTILE_OWNER_CHAMPION;
    input.ownerIndex = 0;
    input.mapIndex = 0;
    input.mapX = 0;
    input.mapY = 0;
    input.cell = 1;
    input.direction = 1;
    input.kineticEnergy = 90;
    input.attack = 80;
    input.stepEnergy = 1;
    input.currentTick = 9;
    input.attackTypeCode = COMBAT_ATTACK_MAGIC;
    input.firstMoveGraceFlag = 0;
    ASSERT_EQ(F0810_PROJECTILE_Create_Compat(&input, &state->world.projectiles,
                                             &slot, &firstMove), 1,
              "open-door projectile seeded");
    state->world.projectiles.entries[slot].scheduledAtTick = 10;
}

static void test_open_door_projectile_schedules_delayed_toggle_and_animates(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char mapTiles[2];
    struct DungeonThings_Compat things;
    struct DungeonDoor_Compat doors[1];
    unsigned short squareFirstThings[2];

    seed_open_door_spell_state(&state, &dungeon, maps, tiles, mapTiles,
                               &things, doors, squareFirstThings);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1, "projectile is live before impact");
    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0, "open-door projectile is consumed on door impact");
    ASSERT_EQ(state.world.timeline.count, 1, "door toggle animation event scheduled");
    ASSERT_EQ(state.world.timeline.events[0].kind, TIMELINE_EVENT_DOOR_ANIMATE,
              "scheduled event is door animation");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 11,
              "door animation starts at GameTime+1");
    ASSERT_EQ(state.world.timeline.events[0].aux1, DOOR_EFFECT_SET,
              "closed button door resolves toggle to opening effect");
    ASSERT_EQ(mapTiles[1] & 0x07, 4, "door remains closed until delayed event fires");

    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "first idle tick reaches delayed event tick");
    ASSERT_EQ(mapTiles[1] & 0x07, 4,
              "door is still closed until the queued tick is dispatched");
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "second idle tick dispatches delayed door animation");
    ASSERT_EQ(mapTiles[1] & 0x07, 3, "first delayed animation step opens door one state");
}

int main(void) {
    printf("=== M11 Open Door Spell Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: MENU.C Open Door projectile; PROJEXPL.C door impact C10/C02 toggle branch\n\n");

    test_open_door_projectile_schedules_delayed_toggle_and_animates();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
