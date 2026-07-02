/*
 * Source-lock gate for M11 action-menu stamina drain.
 *
 * ReDMCSB evidence:
 *   MENU.C G0491 lines 157-201: source action-disabled tick table.
 *   MENU.C G0494 lines 292-337: source action-stamina table.
 *   MENU.C G0495 lines 337-381: source action-defense table.
 *   MENU.C F0407 lines 1246-1272: action stamina is table value plus
 *     M005_RANDOM(2).
 *   MENU.C F0391 lines 829-839: action defense is applied before
 *     F0407 stores Champion.ActionIndex.
 *   MENU.C F0407 lines 1623-1624 and CHAMPION.C F0325 lines 2025-2048:
 *     the common action tail decrements stamina and clamps underflow.
 *   MENU.C F0407 lines 1620-1622 and TIMELINE.C F0253 lines 1588-1598:
 *     the common action tail disables champion actions until the enable
 *     event clears the disabled state.
 */

#include "m11_game_view.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "dm1_v1_endgame_system_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_sound_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "firestaff/dm1/v1/G0491_pc34_compat.h"
#include "firestaff/dm1/v1/G0492_pc34_compat.h"
#include "firestaff/dm1/v1/G0494_pc34_compat.h"

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

#define ASSERT_STR_EQ(actual, expected, msg) do { \
    const char* a_ = (actual); \
    const char* e_ = (expected); \
    if (a_ && e_ && strcmp(a_, e_) == 0) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got \"%s\" expected \"%s\"\n", \
                             (msg), a_ ? a_ : "(null)", e_ ? e_ : "(null)"); } \
} while (0)

static unsigned short make_thing(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static unsigned short pack_text3(int a, int b, int c) {
    return (unsigned short)(((a & 31) << 10) | ((b & 31) << 5) | (c & 31));
}

static unsigned char square_for_test(int elementType, int attributes) {
    return (unsigned char)(((elementType & 0x07) << 5) | (attributes & 0x1f));
}

static void mark_raw_object_slots_unused_for_test(unsigned char* raw, int count) {
    int i;
    for (i = 0; i < count; ++i) {
        raw[i * 4 + 0] = 0xFFu;
        raw[i * 4 + 1] = 0xFFu;
        raw[i * 4 + 2] = 0;
        raw[i * 4 + 3] = 0;
    }
}

static unsigned short object_next_for_test(
    const struct DungeonThings_Compat* things,
    unsigned short thing) {
    int type = THING_GET_TYPE(thing);
    int index = THING_GET_INDEX(thing);
    switch (type) {
        case THING_TYPE_WEAPON:
            return things->weapons[index].next;
        case THING_TYPE_ARMOUR:
            return things->armours[index].next;
        case THING_TYPE_JUNK:
            return things->junks[index].next;
        default:
            return THING_ENDOFLIST;
    }
}

static int is_melee_action_index(unsigned char actionIndex) {
    int damageFactor;
    if (actionIndex == DM1_ACTION_BLOCK) return 0;
    damageFactor = dm1_v1_graphic560_action_damage_factor_get_pc34(
        (int)actionIndex);
    return damageFactor > 0;
}

static unsigned char action_disabled_ticks_for_test(unsigned char actionIndex) {
    int ticks = dm1_v1_graphic560_action_disabled_ticks_get_pc34(actionIndex);
    return ticks < 0 ? 0u : (unsigned char)ticks;
}

static void test_melee_contact_gate_reads_g0492_with_block_exception(void) {
    ASSERT_EQ(dm1_v1_graphic560_action_damage_factor_get_pc34(DM1_ACTION_BLOCK),
              15,
              "source G0492 gives BLOCK a damage factor");
    ASSERT_EQ(is_melee_action_index(DM1_ACTION_BLOCK), 0,
              "BLOCK is excluded from F0402 melee-contact routing");
    ASSERT_EQ(dm1_v1_graphic560_action_damage_factor_get_pc34(DM1_ACTION_PARRY),
              8,
              "source G0492 gives PARRY a damage factor");
    ASSERT_EQ(is_melee_action_index(DM1_ACTION_PARRY), 1,
              "PARRY remains in the F0402 melee-contact routing");
    ASSERT_EQ(dm1_v1_graphic560_action_damage_factor_get_pc34(DM1_ACTION_DISRUPT),
              55,
              "source G0492 gives DISRUPT a damage factor");
    ASSERT_EQ(is_melee_action_index(DM1_ACTION_DISRUPT), 1,
              "DISRUPT remains in the F0402 melee-contact routing");
    ASSERT_EQ(dm1_v1_graphic560_action_damage_factor_get_pc34(DM1_ACTION_SHOOT),
              0,
              "source G0492 keeps SHOOT outside melee-contact routing");
    ASSERT_EQ(is_melee_action_index(DM1_ACTION_SHOOT), 0,
              "SHOOT remains a bounded non-melee F0407 action");
}

static void test_projectile_action_required_mana_uses_g0496_route(void) {
    DM1_ActionXpRoute route;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_FIREBALL, &route), 1,
              "FIREBALL has a source G0496 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_FIRE,
              "FIREBALL required mana uses G0496 Fire skill");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_SPIT, &route), 1,
              "SPIT has a source G0496 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_FIRE,
              "SPIT required mana uses G0496 Fire skill");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_DISPELL, &route), 1,
              "DISPELL has a source G0496 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_AIR,
              "DISPELL required mana uses G0496 Air skill");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_LIGHTNING, &route), 1,
              "LIGHTNING has a source G0496 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_AIR,
              "LIGHTNING required mana uses G0496 Air skill");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_INVOKE, &route), 1,
              "INVOKE has a source G0496 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_WIZARD,
              "INVOKE required mana uses G0496 Wizard skill");
}

static void seed_state(M11_GameViewState* state,
                       unsigned short stamina,
                       unsigned int tick) {
    memset(state, 0, sizeof(*state));
    M11_GameView_Init(state);
    state->active = 1;
    state->world.gameTick = tick;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[0].stamina.current = stamina;
    state->world.party.champions[0].stamina.maximum = 100;
    state->world.party.champions[0].food = 2048;
    state->world.party.champions[0].water = 2048;
    state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;
    state->world.party.champions[0].actionIndex = 255;
    state->world.party.champions[0].name[0] = 'H';
    state->world.party.champions[0].name[1] = 'A';
    state->world.party.champions[0].name[2] = 'L';
    state->world.party.champions[0].name[3] = 'K';
}

static void test_melee_action_row_uses_auto_target_and_action_index(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    int meleeRow = -1;
    int damageEmission = -1;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;

    weapons[0].type = 8; /* Dagger: source ActionSet contains melee rows. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "dagger champion opens action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "dagger action menu resolves source action rows");
    for (i = 0; i < 3; ++i) {
        if (is_melee_action_index(actions[i])) {
            meleeRow = i;
            break;
        }
    }
    ASSERT_EQ(meleeRow >= 0, 1, "dagger action menu exposes a melee row");
    if (meleeRow < 0) return;
    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, meleeRow), 1,
              "melee action row commits attack tick");

    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission >= 0, 1,
              "melee action row emits live damage result");
    if (damageEmission >= 0) {
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[1], 0,
                  "M11 melee row uses M10 front-square auto group target");
    }
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after melee action");
}

static void test_melee_action_row_targets_pref0407_champion_direction(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    int meleeRow = -1;
    int damageEmission = -1;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1; /* Party east: front square is empty. */
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 2; /* Champion south. */

    weapons[0].type = 8;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "champion-facing melee fixture opens action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "champion-facing melee fixture resolves source action rows");
    for (i = 0; i < 3; ++i) {
        if (is_melee_action_index(actions[i])) {
            meleeRow = i;
            break;
        }
    }
    ASSERT_EQ(meleeRow >= 0, 1,
              "champion-facing fixture exposes a melee row");
    if (meleeRow < 0) return;
    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, meleeRow), 1,
              "melee action row uses champion-facing F0407 target");

    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission >= 0, 1,
              "champion-facing melee row emits live damage result");
    if (damageEmission >= 0) {
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[1], 0,
                  "champion-facing melee row targets the south group");
    }
    ASSERT_EQ(groups[0].health[0] < 200, 1,
              "champion-facing melee row damages the south group");
    ASSERT_EQ(state.world.party.direction, 1,
              "melee action does not rewrite party direction");
    ASSERT_EQ(state.world.party.champions[0].direction, 2,
              "melee action preserves champion direction");
}

static void test_melee_action_row_closed_door_targets_pref0407_champion_direction(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonDoor_Compat doors[1];
    unsigned char actions[3];
    int swingRow = -1;
    int sawDestruction = 0;
    int sawThud = 0;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(doors, 0, sizeof(doors));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].doorSet0 = 1; /* Wooden door, defense 42 in G0254. */
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(1 * 3) + 2] =
        square_for_test(DUNGEON_ELEMENT_DOOR,
                        DUNGEON_SQUARE_MASK_THING_LIST | 4);
    squareFirstThings[0] = make_thing(THING_TYPE_DOOR, 0);

    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1; /* Party east: no door in front. */
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 2; /* Champion south. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    weapons[0].type = 2; /* ActionSet 5: SWING. */
    doors[0].next = THING_ENDOFLIST;
    doors[0].type = 0;
    doors[0].meleeDestructible = 1;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.doors = doors;
    things.doorCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "champion-facing door fixture opens action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "champion-facing door fixture resolves source action rows");
    for (i = 0; i < 3; ++i) {
        if (actions[i] == DM1_ACTION_SWING) {
            swingRow = i;
            break;
        }
    }
    ASSERT_EQ(swingRow >= 0, 1,
              "champion-facing door fixture exposes SWING row");
    if (swingRow < 0) return;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, swingRow), 1,
              "SWING row uses champion-facing F0407 door target");

    /* ReDMCSB MENU.C F0407 lines 1266-1275 computes L1251/L1252 from
     * Champion.Direction, then lines 1308-1317 route SWING against a
     * closed door through F0232. */
    for (i = 0; i < state.world.timeline.count; ++i) {
        const struct TimelineEvent_Compat* event = &state.world.timeline.events[i];
        if (event->kind == TIMELINE_EVENT_DOOR_DESTRUCTION &&
            event->mapX == 1 && event->mapY == 2 &&
            (int)event->fireAtTick == 9) {
            sawDestruction = 1;
        }
        if (event->kind == TIMELINE_EVENT_PLAY_SOUND &&
            event->mapX == 1 && event->mapY == 2 &&
            (int)event->fireAtTick == 8) {
            sawThud = 1;
        }
    }
    ASSERT_EQ(sawDestruction, 1,
              "champion-facing SWING schedules south door destruction");
    ASSERT_EQ(sawThud, 1,
              "champion-facing SWING schedules south door thud");
    ASSERT_EQ((squareData[(1 * 3) + 2] & 0x07), 4,
              "F0232 door destruction remains scheduled before dispatch");

    {
        struct TickResult_Compat dispatchResult;
        memset(&dispatchResult, 0, sizeof(dispatchResult));
        state.world.gameTick = 9;
        ASSERT_EQ(F0887_ORCH_DispatchTimelineEvents_Compat(
                      &state.world, &dispatchResult),
                  2,
                  "scheduled champion-facing door thud and destruction dispatch");
        ASSERT_EQ((squareData[(1 * 3) + 2] & 0x07), 5,
                  "scheduled champion-facing door destruction hits south door");
    }
    ASSERT_EQ(state.world.party.direction, 1,
              "door melee action does not rewrite party direction");
    ASSERT_EQ(state.world.party.champions[0].direction, 2,
              "door melee action preserves champion direction");
}

static void test_parry_action_row_routes_through_f0402_f0231(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    DM1_ActionXpRoute route;
    int parryRow = -1;
    int damageEmission = -1;
    int minActionXp;
    int i;

    seed_state(&state, 80, 11);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_PARRY, &route), 1,
              "PARRY has a source G0496/G0497 route for action-row XP");
    if (!route.valid) return;
    minActionXp = route.experienceGain * 2;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PARRY].experience = 500;

    weapons[0].type = 9; /* ActionSet 13: SWING, PARRY, CHOP. */
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "PARRY fixture opens action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "PARRY fixture resolves source action rows");
    for (i = 0; i < 3; ++i) {
        if (actions[i] == DM1_ACTION_PARRY) {
            parryRow = i;
            break;
        }
    }
    ASSERT_EQ(parryRow >= 0, 1,
              "ActionSet 13 exposes PARRY row");
    if (parryRow < 0) return;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, parryRow), 1,
              "PARRY action row routes through F0402/F0231 when target exists");

    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission >= 0, 1,
              "PARRY action row emits live F0231 damage result");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_PARRY),
              "successful action-row PARRY keeps full disabled ticks");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_PARRY,
              "successful action-row PARRY records PARRY as disabled action");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience >=
                  500 + minActionXp,
              1,
              "successful action-row PARRY awards at least full G0497 Parry XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience >=
                  500 + minActionXp,
              1,
              "successful action-row PARRY propagates at least full G0497 Fighter XP");
}

static void test_melee_action_row_halves_disable_ticks_when_f0402_fails(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned char actions[3];
    int meleeRow = -1;
    int damageEmission = -1;
    unsigned char chosen = 0xFFu;
    int staminaBefore;
    DM1_ActionXpRoute route;
    int expectedActionXp;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;

    weapons[0].type = 8;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "dagger champion opens action menu for empty-front test");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "dagger action rows resolve for empty-front test");
    for (i = 0; i < 3; ++i) {
        if (is_melee_action_index(actions[i])) {
            meleeRow = i;
            chosen = actions[i];
            break;
        }
    }
    ASSERT_EQ(meleeRow >= 0, 1, "empty-front fixture exposes a melee row");
    if (meleeRow < 0) return;
    ASSERT_EQ(dm1_v1_action_xp_route((int)chosen, &route), 1,
              "chosen melee action has a source G0496/G0497 route");
    if (!route.valid) return;

    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    expectedActionXp = (route.experienceGain >> 1) * 2;
    staminaBefore = (int)state.world.party.champions[0].stamina.current;
    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, meleeRow), 0,
              "empty-front melee row reports F0402 failure");

    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission, -1,
              "empty-front F0402 failure emits no melee damage");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(chosen) >> 1,
              "F0407 halves disabled ticks when F0402 returns false");
    ASSERT_EQ(state.actionDisabledIndex[0],
              (action_disabled_ticks_for_test(chosen) >> 1) ? chosen : 255,
              "halved F0402 failure stores matching disabled action index");
    ASSERT_EQ((int)state.world.party.champions[0].stamina.current < staminaBefore,
              1,
              "F0407 still spends action stamina after F0402 failure");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedActionXp,
              "F0407 halves G0497 action XP when F0402 returns false");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedActionXp,
              "F0304 propagates halved action XP to the base skill");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after failed melee action");
}

static void test_melee_action_row_respects_live_candidate_no_action(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    int meleeRow = -1;
    int damageEmission = -1;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;

    weapons[0].type = 8;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 9;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "candidate-panel fixture opens action menu before candidate takeover");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "candidate-panel fixture resolves source action rows");
    for (i = 0; i < 3; ++i) {
        if (is_melee_action_index(actions[i])) {
            meleeRow = i;
            break;
        }
    }
    ASSERT_EQ(meleeRow >= 0, 1, "candidate-panel fixture exposes a melee row");
    if (meleeRow < 0) return;
    state.candidateMirrorPanelActive = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorOrdinal = 1;
    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, meleeRow), 0,
              "candidate-panel melee row is rejected before action stamina");

    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission, -1,
              "candidate-panel melee row should not emit damage");
    ASSERT_EQ(groups[0].health[0], 200,
              "candidate-panel melee row leaves group HP unchanged");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 100,
              "candidate-panel melee row leaves stamina unchanged");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "candidate-panel melee row clears acting champion");
    ASSERT_EQ(state.world.candidateAttackInvulnerableEnabled, 0,
              "candidate marker is not left set when action row is rejected");
}

static void test_disrupt_action_row_rejects_material_creature(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    int disruptRow = -1;
    int damageEmission = -1;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;

    weapons[0].type = 16; /* ActionSet 18: JAB, CLEAVE, DISRUPT. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 500;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 500;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_TYPE_ANIMATED_ARMOUR;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "DISRUPT material fixture opens action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "DISRUPT material fixture resolves source action rows");
    for (i = 0; i < 3; ++i) {
        if (actions[i] == DM1_ACTION_DISRUPT) {
            disruptRow = i;
            break;
        }
    }
    ASSERT_EQ(disruptRow >= 0, 1,
              "ActionSet 18 exposes DISRUPT as an action row");
    if (disruptRow < 0) return;
    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, disruptRow), 0,
              "DISRUPT against material creature is rejected before F0231");

    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission >= 0, 1,
              "material DISRUPT emits invalid melee result through CMD_ATTACK");
    if (damageEmission >= 0) {
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[2], 0,
                  "material DISRUPT damage payload is zero");
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[3],
                  COMBAT_OUTCOME_INVALID,
                  "material DISRUPT reports invalid outcome");
    }
    ASSERT_EQ(groups[0].health[0], 200,
              "material DISRUPT leaves group HP unchanged");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_DISRUPT) >> 1,
              "material DISRUPT halves disabled ticks after F0402 failure");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_DISRUPT,
              "material DISRUPT stores disabled action index");
}

static void test_disrupt_action_row_hits_non_material_creature(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    int disruptRow = -1;
    int damageEmission = -1;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;

    weapons[0].type = 16; /* ActionSet 18: JAB, CLEAVE, DISRUPT. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 200;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 200;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 200;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = 5000;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_SWING].experience = 5000;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GHOST;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "DISRUPT ghost fixture opens action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "DISRUPT ghost fixture resolves source action rows");
    for (i = 0; i < 3; ++i) {
        if (actions[i] == DM1_ACTION_DISRUPT) {
            disruptRow = i;
            break;
        }
    }
    ASSERT_EQ(disruptRow >= 0, 1,
              "ActionSet 18 exposes DISRUPT for ghost fixture");
    if (disruptRow < 0) return;
    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, disruptRow), 1,
              "DISRUPT against non-material creature routes through F0231");

    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission >= 0, 1,
              "ghost DISRUPT emits melee damage result");
    if (damageEmission >= 0) {
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[1], 0,
                  "ghost DISRUPT targets front-square group");
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[2] > 0,
                  1,
                  "ghost DISRUPT deals positive damage");
    }
    ASSERT_EQ(groups[0].health[0] < 200, 1,
              "ghost DISRUPT reduces non-material group HP");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_DISRUPT),
              "successful DISRUPT keeps full disabled ticks");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_DISRUPT,
              "successful DISRUPT stores disabled action index");
}

static void test_candidate_panel_blocks_action_menu_open(void) {
    M11_GameViewState state;

    seed_state(&state, 100, 7);
    state.candidateMirrorPanelActive = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorOrdinal = 1;
    state.actingChampionOrdinal = 1;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 0,
              "candidate panel blocks new action-menu open");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "candidate panel clears stale acting champion on open attempt");
}

static void test_direct_non_melee_respects_candidate_panel_gate(void) {
    M11_GameViewState state;
    int staminaBefore;
    uint32_t tickBefore;

    seed_state(&state, 100, 7);
    state.candidateMirrorPanelActive = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorOrdinal = 1;
    state.actingChampionOrdinal = 1;
    staminaBefore = (int)state.world.party.champions[0].stamina.current;
    tickBefore = state.world.gameTick;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_BLOCK),
              0,
              "direct non-melee helper rejects candidate-panel actions");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "candidate-panel direct helper clears acting champion");
    ASSERT_EQ((int)state.world.party.champions[0].stamina.current,
              staminaBefore,
              "candidate-panel direct helper leaves stamina unchanged");
    ASSERT_EQ(state.actionDisabledTicks[0], 0,
              "candidate-panel direct helper does not disable action");
    ASSERT_EQ(state.world.party.champions[0].actionDefense, 0,
              "candidate-panel direct helper does not stack action defense");
    ASSERT_EQ(state.world.gameTick, tickBefore,
              "candidate-panel direct helper does not advance time");
}

static void test_empty_hand_punch_action_row_uses_live_melee(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    int damageEmission = -1;
    int punchRow = -1;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].count = 0;
    groups[0].health[0] = 200;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "empty-hand champion opens source action set 2");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "empty-hand action menu resolves PUNCH/KICK/WAR CRY");
    ASSERT_EQ(actions[0], 6, "empty-hand first row is PUNCH");
    ASSERT_EQ(actions[1], 7, "empty-hand second row is KICK");
    ASSERT_EQ(actions[2], 8, "empty-hand third row is WAR CRY");
    for (i = 0; i < 3; ++i) {
        if (actions[i] == 6) {
            punchRow = i;
            break;
        }
    }
    ASSERT_EQ(punchRow >= 0, 1, "empty-hand fixture exposes PUNCH row");
    if (punchRow < 0) return;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, punchRow), 1,
              "empty-hand PUNCH row commits attack tick");
    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_DAMAGE_DEALT) {
            damageEmission = i;
            break;
        }
    }
    ASSERT_EQ(damageEmission >= 0, 1,
              "empty-hand PUNCH row emits live damage result");
    if (damageEmission >= 0) {
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[1], 0,
                  "empty-hand PUNCH uses M10 front-square auto target");
        ASSERT_EQ(state.lastTickResult.emissions[damageEmission].payload[2] > 0,
                  1,
                  "empty-hand PUNCH produces live damage");
    }
    ASSERT_EQ(groups[0].health[0] < 200, 1,
              "empty-hand PUNCH mutates live group HP");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after empty-hand PUNCH");
}

static void test_empty_hand_war_cry_frightens_front_group(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned char actions[3];
    int warCryRow = -1;
    int i;

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 2; /* Giggler: fear resistance 0. */
    groups[0].count = 0;
    groups[0].health[0] = 25;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "empty-hand champion opens source action set 2 for WAR CRY");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "empty-hand action menu resolves WAR CRY row");
    for (i = 0; i < 3; ++i) {
        if (actions[i] == 8) {
            warCryRow = i;
            break;
        }
    }
    ASSERT_EQ(warCryRow >= 0, 1, "empty-hand fixture exposes WAR CRY row");
    if (warCryRow < 0) return;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, warCryRow), 1,
              "WAR CRY frightens front group");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_FLEE,
              "WAR CRY switches group behavior to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_FLEE,
              "WAR CRY switches active group state to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].fearCounter, 5,
              "WAR CRY sets source DelayFleeingFromTarget");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              12,
              "WAR CRY awards full Influence XP on fright success");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after WAR CRY");
}

static void test_war_cry_targets_pref0407_champion_direction(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(1 * 3) + 2] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1; /* party east: no group on that square. */
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 2; /* champion south. */
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 2; /* Giggler: fear resistance 0. */
    groups[0].count = 0;
    groups[0].health[0] = 25;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 1;
    state.world.creatureAI[0].groupMapY = 2;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 8), 1,
              "WAR CRY targets the pre-F0407 champion-facing square");
    ASSERT_EQ(state.world.party.champions[0].direction, 2,
              "WAR CRY does not run F0406 direction sync");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_FLEE,
              "champion-facing WAR CRY switches the south group to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_FLEE,
              "champion-facing WAR CRY switches the active south group");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              12,
              "champion-facing WAR CRY awards full F0401 Influence XP");
}

static void test_blow_horn_frightens_front_group_with_f0401_values(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 2;
    groups[0].count = 0;
    groups[0].health[0] = 25;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 4), 1,
              "BLOW HORN frightens front group");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_FLEE,
              "BLOW HORN switches group behavior to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_FLEE,
              "BLOW HORN switches active group state to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].fearCounter, 5,
              "BLOW HORN sets source DelayFleeingFromTarget");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              21,
              "BLOW HORN awards F0401 Influence XP plus G0497 table XP");
}

static void test_calm_frightens_front_group_with_f0401_values(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 2;
    groups[0].count = 0;
    groups[0].health[0] = 25;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 37), 1,
              "CALM frightens front group");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_FLEE,
              "CALM switches group behavior to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_FLEE,
              "CALM switches active group state to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].fearCounter, 5,
              "CALM sets source DelayFleeingFromTarget");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              36,
              "CALM awards F0401 Influence XP plus G0497 table XP");
}

static void test_brandish_frightens_front_group_with_f0401_values(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 2;
    groups[0].count = 0;
    groups[0].health[0] = 25;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 41), 1,
              "BRANDISH frightens front group");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_FLEE,
              "BRANDISH switches group behavior to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_FLEE,
              "BRANDISH switches active group state to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].fearCounter, 5,
              "BRANDISH sets source DelayFleeingFromTarget");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              33,
              "BRANDISH awards F0401 Influence XP plus G0497 table XP");
}

static void test_confuse_decrements_charges_and_frightens_front_group(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 2;
    groups[0].count = 0;
    groups[0].health[0] = 25;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 22), 1,
              "CONFUSE frightens front group");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "CONFUSE decrements action-hand weapon charges through F0405");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_FLEE,
              "CONFUSE switches group behavior to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_FLEE,
              "CONFUSE switches active group state to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].fearCounter, 5,
              "CONFUSE sets source DelayFleeingFromTarget");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              45,
              "CONFUSE awards full Influence XP on fright success");
}

static void test_war_cry_resistance_halves_xp_without_flee(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0; /* Giant Scorpion: fear resistance 9. */
    groups[0].count = 0;
    groups[0].health[0] = 80;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 8), 0,
              "WAR CRY resisted by fear resistance returns not frightened");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_ATTACK,
              "resisted WAR CRY does not switch group behavior to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_ATTACK,
              "resisted WAR CRY does not switch active group state to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].fearCounter, 0,
              "resisted WAR CRY leaves DelayFleeingFromTarget unchanged");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              6,
              "resisted WAR CRY awards half Influence XP");
}

static void test_blow_horn_immune_halves_xp_without_flee(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 9; /* Stone Golem: fear resistance 15. */
    groups[0].count = 0;
    groups[0].health[0] = 145;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 4), 0,
              "BLOW HORN against fear-immune group returns not frightened");
    ASSERT_EQ(groups[0].behavior, DM1_BEHAVIOR_ATTACK,
              "fear-immune BLOW HORN does not switch group behavior to FLEE");
    ASSERT_EQ(state.world.creatureAI[0].stateKind, AI_STATE_ATTACK,
              "fear-immune BLOW HORN does not switch active group state");
    ASSERT_EQ(state.world.creatureAI[0].fearCounter, 0,
              "fear-immune BLOW HORN leaves DelayFleeingFromTarget unchanged");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              11,
              "fear-immune BLOW HORN awards half F0401 XP plus G0497 XP");
}

static void test_blow_horn_uses_f0304_influence_xp_semantics(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];

    seed_state(&state, 100, 7);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].difficulty = 1;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(2 * 3) + 1] = DUNGEON_SQUARE_MASK_THING_LIST;
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].direction = 1;

    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PRIEST].experience = 490;
    state.world.party.champions[0].skillLevels[DM1_SKILL_IDX_PRIEST] = 1;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 100;
    state.world.lifecycle.champions[0].maxHealth = 100;
    state.world.lifecycle.champions[0].maxStamina = 100;
    state.world.lifecycle.champions[0].maxMana = 20;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 2;
    groups[0].count = 0;
    groups[0].health[0] = 25;
    groups[0].cells = 0xFF;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].reserved0 = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 4), 1,
              "BLOW HORN frightens front group before F0304 XP award");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].experience,
              21,
              "F0401 and F0407 award BLOW HORN XP to hidden Influence skill");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_INFLUENCE].temporaryExperience,
              3,
              "F0304 adds bounded temporary XP to Influence");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_PRIEST].experience,
              511,
              "F0304 propagates Influence XP to Priest base skill");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_PRIEST].temporaryExperience,
              3,
              "F0304 adds bounded temporary XP to Priest base skill");
    ASSERT_EQ(state.world.party.champions[0]
                  .skillLevels[DM1_SKILL_IDX_PRIEST],
              2,
              "F0401 F0304 level-up syncs Priest level back to party state");
}

static void test_block_action_spends_source_stamina(void) {
    M11_GameViewState state;
    seed_state(&state, 20, 1);

    (void)M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 1); /* BLOCK */

    ASSERT_EQ(state.world.party.champions[0].stamina.current, 16,
              "BLOCK spends source base stamina when jitter is zero");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 100,
              "normal action stamina drain does not damage HP");
}

static void test_flip_action_prints_source_message_and_keeps_common_tail(void) {
    M11_GameViewState state;
    DM1_ActionXpRoute route;
    const char* message;
    int expectedStaminaCost;

    seed_state(&state, 100, 62);
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 3u);
    expectedStaminaCost =
        dm1_v1_graphic560_action_stamina_get_pc34(DM1_ACTION_FLIP) +
        (int)((state.world.gameTick + (uint32_t)DM1_ACTION_FLIP) & 1u);

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_FLIP, &route), 1,
              "FLIP has a source G0496/G0497 route");
    if (!route.valid) return;
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FLIP),
              1,
              "FLIP performs the ReDMCSB F0407 coin-message branch");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "FLIP does not create a projectile");
    ASSERT_EQ(state.world.party.champions[0].stamina.current,
              100 - expectedStaminaCost,
              "FLIP spends the common G0494 stamina tail");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_FLIP),
              "FLIP keeps the common G0491 disabled-tick tail");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_FLIP,
              "FLIP records the source action index while disabled");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              route.experienceGain,
              "FLIP awards G0497 action XP to its action skill");
    if (route.baseSkillIndex != route.skillIndex) {
        ASSERT_EQ(state.world.lifecycle.champions[0]
                      .skills20[route.baseSkillIndex].experience,
                  route.experienceGain,
                  "FLIP propagates G0497 XP to the base skill");
    }
    ASSERT_EQ(M11_GameView_GetMessageLogCount(&state) >= 2, 1,
              "FLIP writes the action cue plus source coin message");
    message = M11_GameView_GetMessageLogEntry(&state, 0);
    ASSERT_EQ(message != NULL &&
                  (strstr(message, "IT COMES UP HEADS.") != NULL ||
                   strstr(message, "IT COMES UP TAILS.") != NULL),
              1,
              "FLIP prints the ReDMCSB heads/tails message");
}

static void test_throw_action_removes_action_hand_object(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned short thrownThing;
    int expectedCommonStaminaCost;

    seed_state(&state, 100, 99);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    weapons[0].type = 8; /* Dagger: weight 5, class 2, kinetic 19. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    state.world.party.champions[0].food = 0;
    state.world.party.champions[0].water = 0;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    expectedCommonStaminaCost =
        dm1_v1_graphic560_action_stamina_get_pc34(DM1_ACTION_THROW) +
        (int)((state.world.gameTick + (uint32_t)DM1_ACTION_THROW) & 1u);
    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 42), 1,
              "THROW action spawns projectile");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "THROW action creates one live projectile");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              THING_NONE,
              "THROW removes object from action hand after projectile spawn");
    ASSERT_EQ(expectedCommonStaminaCost, 1,
              "THROW fixture forces the G0494 zero-base F0407 jitter tail");
    ASSERT_EQ(state.world.party.champions[0].stamina.current,
              100 - 2 - expectedCommonStaminaCost,
              "THROW spends F0305 object-weight stamina plus F0407 jitter");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "THROW mirrors F0406 champion direction to party direction");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_THROW].experience,
              21,
              "THROW awards F0328 Throw XP plus F0407 G0497 action XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_NINJA].experience,
              21,
              "THROW propagates both XP sources to Ninja base skill");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_THROW].temporaryExperience,
              3,
              "THROW adds bounded temporary XP for both XP sources");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "THROW uses F0407 side-derived launch cell");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "THROW keeps projectile direction at party direction");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 82,
              "THROW passes F0328 kinetic energy to projectile create");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 40,
              "THROW passes F0328 bounded attack to projectile create");
    ASSERT_EQ(state.world.projectiles.entries[0].launcherStrength, 40,
              "THROW carries F0328 attack into F0217 kinetic pass-through strength");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 10,
              "THROW passes F0328 step energy to projectile create");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, thrownThing,
              "THROW preserves removed object Thing identity on projectile");
    ASSERT_EQ(state.world.projectileDisabledMovementTicks, 3,
              "THROW sets source projectile movement-disable ticks before the action tick decrements them");
    ASSERT_EQ(state.world.lastProjectileDisabledMovementDirection, 1,
              "THROW records source projectile movement-disable direction");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_COMBAT,
              "THROW requests the F0328 M563 combat sound");
    ASSERT_EQ(state.actionDisabledTicks[0], 3,
              "THROW keeps the inner F0328/F0330 disable after the action tick decrements it");
    ASSERT_EQ(state.actionDisabledIndex[0], 0xFF,
              "THROW has no F0407 disabled-action index when G0491 is zero");
    ASSERT_EQ(state.actionEnableSlotOrdinal[0], CHAMPION_SLOT_ACTION_HAND,
              "THROW stores C01 action-hand slot ordinal on the enable-action event");
}

static void test_throw_full_projectile_list_still_accepts_f0328(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned short thrownThing;
    int expectedCommonStaminaCost;

    seed_state(&state, 100, 99);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    weapons[0].type = 8; /* Dagger: weight 5, class 2, kinetic 19. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    state.world.party.champions[0].food = 0;
    state.world.party.champions[0].water = 0;
    state.world.projectiles.count = PROJECTILE_LIST_CAPACITY;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    expectedCommonStaminaCost =
        dm1_v1_graphic560_action_stamina_get_pc34(DM1_ACTION_THROW) +
        (int)((state.world.gameTick + (uint32_t)DM1_ACTION_THROW) & 1u);
    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_THROW),
              1,
              "full-list THROW still returns accepted F0328 result");
    ASSERT_EQ(state.world.projectiles.count, PROJECTILE_LIST_CAPACITY,
              "full-list THROW does not allocate past the PJE-05 cap");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              THING_NONE,
              "accepted full-list THROW still clears the action hand");
    ASSERT_EQ(expectedCommonStaminaCost, 1,
              "full-list THROW fixture forces the same F0407 jitter tail");
    ASSERT_EQ(state.world.party.champions[0].stamina.current,
              100 - 2 - expectedCommonStaminaCost,
              "full-list THROW spends F0305 stamina plus F0407 jitter");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_THROW].experience,
              21,
              "full-list THROW keeps F0328 Throw XP plus F0407 G0497 XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_NINJA].experience,
              21,
              "full-list THROW propagates both XP sources to Ninja");
    ASSERT_EQ(state.world.projectileDisabledMovementTicks, 0,
              "dropped full-list THROW does not set live-projectile movement lockout");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_COMBAT,
              "full-list THROW still requests the F0328 M563 combat sound");
    ASSERT_EQ(state.actionDisabledTicks[0], 3,
              "full-list THROW keeps the inner F0328/F0330 disable");
    ASSERT_EQ(state.actionDisabledIndex[0], 0xFF,
              "full-list THROW has no F0407 disabled-action overwrite");
    ASSERT_EQ(state.actionEnableSlotOrdinal[0], CHAMPION_SLOT_ACTION_HAND,
              "full-list THROW keeps the C01 action-hand enable slot");
}

static void test_throw_uses_post_f0304_throw_level_for_projectile(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned short thrownThing;

    seed_state(&state, 100, 70);
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    junks[0].next = THING_ENDOFLIST;
    junks[0].type = 0; /* ReDMCSB C00_JUNK_COMPASS, weight 1. */
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime =
        state.world.gameTick + 1;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    state.world.lifecycle.champions[0]
        .skills20[LIFECYCLE_SKILL_NINJA].experience = 491;
    state.world.lifecycle.champions[0]
        .skills20[LIFECYCLE_SKILL_THROW].experience = 491;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_JUNK, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_THROW),
              1,
              "THROW level-up fixture creates a projectile");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "THROW level-up fixture has one live projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 9,
              "F0328 uses post-F0304 F0303(THROW) level for same-throw step energy");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_THROW].experience >= 500,
              1,
              "F0328 Throw XP crosses the first hidden-skill threshold");
}

static void test_direct_throw_empty_action_hand_keeps_f0407_tail(void) {
    M11_GameViewState state;
    DM1_ActionXpRoute route;
    int expectedActionXp;
    int expectedStaminaCost;
    int expectedDisabledTicks;

    seed_state(&state, 100, 58);
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_THROW, &route), 1,
              "empty-hand THROW has a source G0496/G0497 route");
    if (!route.valid) return;
    expectedActionXp = route.experienceGain * 2;
    expectedStaminaCost =
        dm1_v1_graphic560_action_stamina_get_pc34(DM1_ACTION_THROW);
    expectedDisabledTicks = action_disabled_ticks_for_test(DM1_ACTION_THROW);

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_THROW),
              0,
              "empty action-hand THROW returns F0328/F0407 failure");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "empty action-hand THROW creates no projectile");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              THING_NONE,
              "empty action-hand THROW leaves the action hand empty");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "empty action-hand THROW still mirrors F0406 direction");
    ASSERT_EQ(state.actionDisabledTicks[0],
              expectedDisabledTicks,
              "empty action-hand THROW keeps the common G0491 disabled tail");
    ASSERT_EQ(state.actionDisabledIndex[0],
              expectedDisabledTicks ? DM1_ACTION_THROW : 255,
              "empty action-hand THROW records disabled action only when G0491 is nonzero");
    ASSERT_EQ(state.world.party.champions[0].stamina.current,
              100 - expectedStaminaCost,
              "empty action-hand THROW still spends common G0494 stamina");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedActionXp,
              "empty action-hand THROW still awards common G0497 XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedActionXp,
              "empty action-hand THROW still propagates common G0497 XP");
    ASSERT_EQ(state.world.projectileDisabledMovementTicks, 0,
              "empty action-hand THROW does not set F0328 movement-disable ticks");
    ASSERT_EQ(state.actionEnableSlotOrdinal[0], 0xFF,
              "empty action-hand THROW does not store an enable-action slot ordinal");
}

static void test_throw_ven_potion_launches_removepotion_projectile(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonPotion_Compat potions[1];
    unsigned short thrownThing;

    seed_state(&state, 100, 100);
    memset(&things, 0, sizeof(things));
    memset(potions, 0, sizeof(potions));
    potions[0].next = THING_ENDOFLIST;
    potions[0].power = 0;
    potions[0].type = 3; /* ReDMCSB C03_POTION_VEN_POTION. */
    things.loaded = 1;
    things.potions = potions;
    things.potionCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_POTION, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 42), 1,
              "THROW action accepts Ven potion projectile");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "THROW Ven potion creates one live projectile");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              THING_NONE,
              "THROW Ven potion removes action-hand potion");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileCategory,
              PROJECTILE_CATEGORY_KINETIC,
              "THROW Ven potion keeps thrown-object projectile category");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              PROJECTILE_SUBTYPE_POISON_CLOUD,
              "THROW Ven potion stores poison-cloud impact subtype");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 43,
              "THROW Ven potion includes F0140 potion weight in F0312 throw strength");
    ASSERT_EQ(state.world.projectiles.entries[0].associatedPotionPower, 0,
              "THROW zero-power Ven potion carries source potion power");
    ASSERT_EQ(state.world.projectiles.entries[0].poisonAttack, 0,
              "THROW zero-power Ven potion carries zero poison payload");
    ASSERT_EQ(state.world.projectiles.entries[0].flags
                  & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT,
              PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT,
              "THROW zero-power Ven potion still sets RemovePotion impact flag");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, thrownThing,
              "THROW Ven potion preserves removed potion Thing identity");
}

static void test_throw_ven_potion_advances_to_wall_impact_and_consumes(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct DungeonThings_Compat things;
    struct DungeonPotion_Compat potions[1];
    unsigned char rawPotionData[4];
    unsigned char* rawThingData[DUNGEON_THING_TYPE_COUNT];
    int thingCounts[DUNGEON_THING_TYPE_COUNT];
    unsigned short thrownThing;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(&things, 0, sizeof(things));
    memset(potions, 0, sizeof(potions));
    memset(rawPotionData, 0, sizeof(rawPotionData));
    memset(rawThingData, 0, sizeof(rawThingData));
    memset(thingCounts, 0, sizeof(thingCounts));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(3 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    potions[0].next = THING_ENDOFLIST;
    potions[0].power = 77;
    potions[0].type = 3; /* ReDMCSB C03_POTION_VEN_POTION. */
    rawPotionData[0] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawPotionData[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawPotionData[2] = 77;
    rawPotionData[3] = 3;
    rawThingData[THING_TYPE_POTION] = rawPotionData;
    thingCounts[THING_TYPE_POTION] = 1;

    things.loaded = 1;
    things.potions = potions;
    things.potionCount = 1;
    memcpy(things.rawThingData, rawThingData, sizeof(rawThingData));
    memcpy(things.thingCounts, thingCounts, sizeof(thingCounts));

    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_POTION, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 42), 1,
              "THROW Ven potion starts end-to-end projectile route");
    ASSERT_EQ(state.world.projectiles.entries[0].firstMoveGraceFlag, 1,
              "THROW Ven potion starts with first-move grace");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              PROJECTILE_SUBTYPE_POISON_CLOUD,
              "THROW Ven potion starts as poison cloud impact subtype");

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    state.world.dungeon = &dungeon;

    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "THROW Ven potion first advance keeps projectile live");
    ASSERT_EQ(state.world.projectiles.entries[0].mapX, 2,
              "THROW Ven potion first grace advance crosses to east square");
    ASSERT_EQ(state.world.projectiles.entries[0].firstMoveGraceFlag, 0,
              "THROW Ven potion first advance consumes grace flag");

    state.world.gameTick = 102;
    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "THROW Ven potion second advance stays live on intra-cell flip");
    ASSERT_EQ(state.world.projectiles.entries[0].mapX, 2,
              "THROW Ven potion second advance stays in current square");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "THROW Ven potion second advance flips cell by parity");

    state.world.gameTick = 103;
    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "THROW Ven potion third advance impacts wall and despawns");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 3, 1), 1,
              "THROW Ven potion wall impact creates explosion at wall square");
    ASSERT_EQ(state.world.explosions.entries[0].explosionType,
              C007_EXPLOSION_POISON_CLOUD,
              "THROW Ven potion wall impact maps to poison cloud");
    ASSERT_EQ(state.world.explosions.entries[0].attack, 77,
              "THROW Ven potion wall impact uses potion power");
    ASSERT_EQ(state.world.explosions.entries[0].cell,
              EXPLOSION_CELL_CENTERED,
              "THROW Ven potion wall impact creates centered cloud");
    ASSERT_EQ(potions[0].next, THING_NONE,
              "THROW Ven potion wall impact consumes decoded potion thing");
    ASSERT_EQ(rawPotionData[0] | (rawPotionData[1] << 8), THING_NONE,
              "THROW Ven potion wall impact consumes raw potion thing");
}

static void test_throw_ful_bomb_advances_to_wall_impact_and_consumes(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct DungeonThings_Compat things;
    struct DungeonPotion_Compat potions[1];
    unsigned char rawPotionData[4];
    unsigned char* rawThingData[DUNGEON_THING_TYPE_COUNT];
    int thingCounts[DUNGEON_THING_TYPE_COUNT];
    unsigned short thrownThing;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(&things, 0, sizeof(things));
    memset(potions, 0, sizeof(potions));
    memset(rawPotionData, 0, sizeof(rawPotionData));
    memset(rawThingData, 0, sizeof(rawThingData));
    memset(thingCounts, 0, sizeof(thingCounts));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(3 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    potions[0].next = THING_ENDOFLIST;
    potions[0].power = 96;
    potions[0].type = 19; /* ReDMCSB C19_POTION_FUL_BOMB. */
    rawPotionData[0] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawPotionData[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawPotionData[2] = 96;
    rawPotionData[3] = 19;
    rawThingData[THING_TYPE_POTION] = rawPotionData;
    thingCounts[THING_TYPE_POTION] = 1;

    things.loaded = 1;
    things.potions = potions;
    things.potionCount = 1;
    memcpy(things.rawThingData, rawThingData, sizeof(rawThingData));
    memcpy(things.thingCounts, thingCounts, sizeof(thingCounts));

    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_POTION, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 42), 1,
              "THROW Ful Bomb starts end-to-end projectile route");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              PROJECTILE_SUBTYPE_FIREBALL,
              "THROW Ful Bomb starts as fireball impact subtype");
    ASSERT_EQ(state.world.projectiles.entries[0].associatedPotionPower, 96,
              "THROW Ful Bomb carries potion power to projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].flags
                  & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT,
              PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT,
              "THROW Ful Bomb sets RemovePotion impact flag");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, thrownThing,
              "THROW Ful Bomb preserves removed potion Thing identity");

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    state.world.dungeon = &dungeon;

    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "THROW Ful Bomb first advance keeps projectile live");

    state.world.gameTick = 102;
    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "THROW Ful Bomb second advance stays live on intra-cell flip");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "THROW Ful Bomb second advance flips cell by parity");

    state.world.gameTick = 103;
    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "THROW Ful Bomb third advance impacts wall and despawns");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 3, 1), 1,
              "THROW Ful Bomb wall impact creates explosion at wall square");
    ASSERT_EQ(state.world.explosions.entries[0].explosionType,
              C000_EXPLOSION_FIREBALL,
              "THROW Ful Bomb wall impact maps to fireball");
    ASSERT_EQ(state.world.explosions.entries[0].attack, 96,
              "THROW Ful Bomb wall impact uses potion power");
    ASSERT_EQ(state.world.explosions.entries[0].cell, 2,
              "THROW Ful Bomb wall impact keeps projectile impact cell");
    ASSERT_EQ(potions[0].next, THING_NONE,
              "THROW Ful Bomb wall impact consumes decoded potion thing");
    ASSERT_EQ(rawPotionData[0] | (rawPotionData[1] << 8), THING_NONE,
              "THROW Ful Bomb wall impact consumes raw potion thing");
}

static void test_throw_projectile_advances_after_scheduled_tick(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned short thrownThing;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    weapons[0].type = 8; /* Dagger: weight 5, class 2, kinetic 19. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thrownThing;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 42), 1,
              "THROW action spawns projectile for scheduled F0811 advance");
    ASSERT_EQ(state.world.gameTick, 101,
              "THROW action advances one game tick before manual projectile step");
    ASSERT_EQ(state.world.projectiles.entries[0].firstMoveGraceFlag, 1,
              "THROW projectile keeps first-move grace when no dungeon is loaded");
    ASSERT_EQ(state.world.projectiles.entries[0].scheduledAtTick, 101,
              "THROW projectile is scheduled for the next source tick");
    ASSERT_EQ(state.world.projectiles.entries[0].mapX, 1,
              "THROW projectile starts on party x before manual advance");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "THROW projectile starts on side-derived cell before manual advance");

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    state.world.dungeon = &dungeon;

    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(state.world.projectiles.entries[0].firstMoveGraceFlag, 0,
              "first scheduled F0811 advance consumes first-move grace");
    ASSERT_EQ(state.world.projectiles.entries[0].mapX, 2,
              "first scheduled F0811 advance crosses to the east square");
    ASSERT_EQ(state.world.projectiles.entries[0].mapY, 1,
              "first scheduled F0811 advance preserves y");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 3,
              "first scheduled F0811 advance applies source cell parity");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 82,
              "first grace advance does not decay kinetic energy");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 40,
              "first grace advance does not decay attack");
    ASSERT_EQ(state.world.projectiles.entries[0].scheduledAtTick, 102,
              "first scheduled F0811 advance reschedules to the next party-map tick");

    state.world.gameTick = 102;
    M11_GameView_AdvanceProjectilesOnce(&state);
    ASSERT_EQ(state.world.projectiles.entries[0].mapX, 2,
              "second scheduled F0811 advance stays in square on intra-cell flip");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "second scheduled F0811 advance flips cell by source parity");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 72,
              "second scheduled F0811 advance decays kinetic energy by step");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 30,
              "second scheduled F0811 advance decays attack by step");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, thrownThing,
              "scheduled F0811 advance preserves thrown Thing identity");
}

static void test_projectile_creature_impact_at_zero_zero_applies_damage(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = 0;
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].lastSeenPartyTick = 99;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.gameTick = 100;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->poisonAttack = 3;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "F0811 creature impact at (0,0) despawns projectile");
    ASSERT_EQ(groups[0].health[0], 2,
              "M11 applies defense-scaled plus randomized resistance-adjusted poison projectile creature damage at real zero coordinate");
    ASSERT_EQ(state.world.timeline.count, 1,
              "projectile creature hit schedules one C30 reaction event");
    ASSERT_EQ(state.world.timeline.events[0].kind, TIMELINE_EVENT_CREATURE_REACTION,
              "projectile creature hit schedules a creature reaction");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 105,
              "projectile creature hit uses F0209 CM2 reaction delay");
    ASSERT_EQ(state.world.timeline.events[0].mapIndex, 0,
              "projectile creature hit reaction stores impact map");
    ASSERT_EQ(state.world.timeline.events[0].mapX, 0,
              "projectile creature hit reaction stores impact x");
    ASSERT_EQ(state.world.timeline.events[0].mapY, 0,
              "projectile creature hit reaction stores impact y");
    ASSERT_EQ(state.world.timeline.events[0].aux0, 0,
              "projectile creature hit reaction stores group index");
    ASSERT_EQ(state.world.timeline.events[0].aux1, CREATURE_TYPE_GIANT_SCORPION,
              "projectile creature hit reaction stores creature type");
    ASSERT_EQ(state.world.timeline.events[0].aux2, DM1_EVENT_REACTION_HIT_BY_PROJECTILE,
              "projectile creature hit reaction stores C30 event type");
}

static void test_projectile_creature_kill_spawns_f0190_death_smoke(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 50;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].lastSeenPartyTick = 99;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.gameTick = 100;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->poisonAttack = 3;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "killing creature projectile despawns projectile");
    ASSERT_EQ(groups[0].health[0], 0,
              "killing creature projectile zeroes group health");
    ASSERT_EQ(squareFirstThings[0] != make_thing(THING_TYPE_GROUP, 0), 1,
              "killing creature projectile removes dead group from square");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 0, 0), 1,
              "killing creature projectile creates F0190 death smoke");
    ASSERT_EQ(state.world.explosions.entries[0].explosionType,
              C040_EXPLOSION_SMOKE,
              "killing creature projectile uses C040 death smoke");
    ASSERT_EQ(state.world.explosions.entries[0].attack, 110,
              "C00 quarter-square death smoke uses F0190 attack 110");
    ASSERT_EQ(state.world.explosions.entries[0].cell,
              3,
              "creature death smoke uses killed creature cell");
    ASSERT_EQ(state.world.timeline.count, 1,
              "killing creature projectile schedules only smoke advance");
    ASSERT_EQ(state.world.timeline.events[0].kind,
              TIMELINE_EVENT_EXPLOSION_ADVANCE,
              "killing creature projectile schedules C040 advance event");
}

static void test_projectile_creature_killed_some_drops_fixed_possessions(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[12];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonArmour_Compat armours[8];
    struct DungeonJunk_Compat junks[4];
    unsigned char weaponRaw[8][4];
    unsigned char armourRaw[8][4];
    unsigned char junkRaw[4][4];
    struct ProjectileInstance_Compat* projectile;
    unsigned short firstDrop;
    int sawWeaponDrop = 0;
    int sawArmourDrop = 0;
    int sawShiftedAspect = 0;
    int sawShiftedBehavior = 0;
    int sawProjectileReaction = 0;
    int sawExplosionAdvance = 0;
    int dropCount = 0;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(weapons, 0, sizeof(weapons));
    memset(armours, 0, sizeof(armours));
    memset(junks, 0, sizeof(junks));
    memset(weaponRaw, 0, sizeof(weaponRaw));
    memset(armourRaw, 0, sizeof(armourRaw));
    memset(junkRaw, 0, sizeof(junkRaw));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        squareFirstThings[i] = THING_ENDOFLIST;
    }
    for (i = 0; i < 8; ++i) {
        weapons[i].next = THING_NONE;
        armours[i].next = THING_NONE;
    }
    for (i = 0; i < 4; ++i) {
        junks[i].next = THING_NONE;
    }
    mark_raw_object_slots_unused_for_test(&weaponRaw[0][0], 8);
    mark_raw_object_slots_unused_for_test(&armourRaw[0][0], 8);
    mark_raw_object_slots_unused_for_test(&junkRaw[0][0], 4);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    squareData[(2 * 3) + 1] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[(2 * 3) + 1] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_TYPE_ANIMATED_ARMOUR;
    groups[0].cells = 0x09u;
    groups[0].count = 1;
    groups[0].health[0] = 1;
    groups[0].health[1] = 200;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 12;
    things.groups = groups;
    things.groupCount = 1;
    things.weapons = weapons;
    things.weaponCount = 8;
    things.armours = armours;
    things.armourCount = 8;
    things.junks = junks;
    things.junkCount = 4;
    things.rawThingData[THING_TYPE_WEAPON] = &weaponRaw[0][0];
    things.rawThingData[THING_TYPE_ARMOUR] = &armourRaw[0][0];
    things.rawThingData[THING_TYPE_JUNK] = &junkRaw[0][0];
    things.thingCounts[THING_TYPE_WEAPON] = 8;
    things.thingCounts[THING_TYPE_ARMOUR] = 8;
    things.thingCounts[THING_TYPE_JUNK] = 4;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 1;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].lastSeenPartyTick = 99;
    state.world.gameTick = 100;
    state.audioState.initialized = 1;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 3;
    projectile->mapY = 1;
    projectile->cell = 0;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 80;
    projectile->stepEnergy = 5;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 7);
    projectile->reserved3 = 1;

    state.world.timeline.count = 4;
    memset(&state.world.timeline.events[0], 0,
           sizeof(state.world.timeline.events[0]) * 4);
    state.world.timeline.events[0].kind = TIMELINE_EVENT_CREATURE_REACTION;
    state.world.timeline.events[0].fireAtTick = 140;
    state.world.timeline.events[0].mapIndex = 0;
    state.world.timeline.events[0].mapX = 2;
    state.world.timeline.events[0].mapY = 1;
    state.world.timeline.events[0].aux0 = 0;
    state.world.timeline.events[0].aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    state.world.timeline.events[1] = state.world.timeline.events[0];
    state.world.timeline.events[1].aux2 = DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1;
    state.world.timeline.events[2] = state.world.timeline.events[0];
    state.world.timeline.events[2].aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    state.world.timeline.events[3] = state.world.timeline.events[0];
    state.world.timeline.events[3].aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "killed-some projectile despawns projectile");
    ASSERT_EQ(groups[0].count, 0,
              "killed-some projectile compacts group count");
    ASSERT_EQ(groups[0].health[0], 200,
              "killed-some projectile preserves surviving creature health");
    ASSERT_EQ(squareFirstThings[(2 * 3) + 1], make_thing(THING_TYPE_GROUP, 0),
              "killed-some projectile keeps surviving group on square");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 2, 1), 1,
              "killed-some projectile still creates F0190 smoke");
    ASSERT_EQ(state.world.explosions.entries[0].explosionType,
              C040_EXPLOSION_SMOKE,
              "killed-some projectile smoke uses C040");
    ASSERT_EQ(state.world.explosions.entries[0].cell, 1,
              "killed-some projectile smoke uses killed cell");

    firstDrop = groups[0].next;
    while (firstDrop != THING_ENDOFLIST && firstDrop != THING_NONE &&
           dropCount < 16) {
        int type = THING_GET_TYPE(firstDrop);
        int index = THING_GET_INDEX(firstDrop);
        if (type == THING_TYPE_ARMOUR) {
            sawArmourDrop = 1;
            ASSERT_EQ(armours[index].cursed, 1,
                      "Animated Armour fixed armour drop is cursed");
        }
        if (type == THING_TYPE_WEAPON) {
            sawWeaponDrop = 1;
            ASSERT_EQ(weapons[index].cursed, 1,
                      "Animated Armour fixed weapon drop is cursed");
        }
        ++dropCount;
        firstDrop = object_next_for_test(&things, firstDrop);
    }
    ASSERT_EQ(dropCount, 6,
              "killed-some projectile materializes six fixed possessions");
    ASSERT_EQ(sawArmourDrop, 1,
              "killed-some projectile materializes an armour drop");
    ASSERT_EQ(sawWeaponDrop, 1,
              "killed-some projectile materializes a weapon drop");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_METALLIC_THUD,
              "killed-some fixed possessions emit source metallic thud");

    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE) {
            sawExplosionAdvance = 1;
        }
        if (state.world.timeline.events[i].kind == TIMELINE_EVENT_CREATURE_REACTION) {
            if (state.world.timeline.events[i].aux2 ==
                DM1_EVENT_REACTION_HIT_BY_PROJECTILE) {
                sawProjectileReaction = 1;
            }
            if (state.world.timeline.events[i].aux2 ==
                DM1_EVENT_UPDATE_ASPECT_CREATURE_0) {
                ++sawShiftedAspect;
            }
            if (state.world.timeline.events[i].aux2 ==
                DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0) {
                ++sawShiftedBehavior;
            }
            ASSERT_EQ(state.world.timeline.events[i].aux2 !=
                      DM1_EVENT_UPDATE_ASPECT_CREATURE_0 + 1, 1,
                      "killed-some cleanup deletes old aspect event for killed slot");
            ASSERT_EQ(state.world.timeline.events[i].aux2 !=
                      DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 1, 1,
                      "killed-some cleanup deletes old behavior event for killed slot");
        }
    }
    ASSERT_EQ(sawShiftedAspect, 1,
              "killed-some cleanup shifts surviving aspect event down");
    ASSERT_EQ(sawShiftedBehavior, 1,
              "killed-some cleanup shifts surviving behavior event down");
    ASSERT_EQ(sawProjectileReaction, 1,
              "killed-some projectile still schedules C30 reaction");
    ASSERT_EQ(sawExplosionAdvance, 1,
              "killed-some projectile schedules smoke advance");
}

static int run_projectile_creature_killed_some_f0190_fear_attempt(
    unsigned int seed,
    int* outFearCounter) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    unsigned short squareFirstThings[12];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    squareData[(2 * 3) + 1] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[(2 * 3) + 1] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GIANT_SCORPION;
    groups[0].cells = 0x09u;
    groups[0].count = 1;
    groups[0].health[0] = 1;
    groups[0].health[1] = 200;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 12;
    things.groups = groups;
    things.groupCount = 1;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].lastSeenPartyTick = 99;
    state.world.gameTick = 100;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, seed);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 3;
    projectile->mapY = 1;
    projectile->cell = 0;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 80;
    projectile->stepEnergy = 5;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = THING_NONE;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    if (groups[0].behavior == DM1_BEHAVIOR_FLEE &&
        state.world.creatureAI[0].stateKind == AI_STATE_FLEE &&
        state.world.creatureAI[0].fearCounter >= 20) {
        if (outFearCounter) {
            *outFearCounter = state.world.creatureAI[0].fearCounter;
        }
        return 1;
    }
    return 0;
}

static void test_projectile_creature_killed_some_can_trigger_f0190_fear(void) {
    unsigned int seed;
    int sawFear = 0;
    int fearCounter = 0;

    for (seed = 1; seed <= 512 && !sawFear; ++seed) {
        sawFear = run_projectile_creature_killed_some_f0190_fear_attempt(
            seed, &fearCounter);
    }

    ASSERT_EQ(sawFear, 1,
              "killed-some projectile can trigger F0190 fear branch");
    ASSERT_EQ(fearCounter >= 20, 1,
              "F0190 fear stores source flee delay on active group");
}

static void test_projectile_creature_zero_scaled_attack_skips_poison_and_reaction(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_LORD_CHAOS;
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].lastSeenPartyTick = 99;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.gameTick = 100;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 1;
    projectile->stepEnergy = 1;
    projectile->poisonAttack = 50;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "zero-scaled creature impact still despawns projectile");
    ASSERT_EQ(groups[0].health[0], 100,
              "zero-scaled creature impact skips projectile poison damage");
    ASSERT_EQ(state.world.timeline.count, 0,
              "zero-scaled creature impact skips C30 reaction scheduling");
}

static void test_projectile_non_material_creature_passes_through_without_impact(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GHOST;
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.gameTick = 100;

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "non-material creature hit by normal projectile keeps projectile alive");
    ASSERT_EQ(groups[0].health[0], 100,
              "non-material creature hit by normal projectile takes no damage");
    ASSERT_EQ(state.world.timeline.count, 0,
              "non-material creature pass-through schedules no reaction");
    ASSERT_EQ(state.world.projectiles.entries[0].mapX, 0,
              "non-material creature pass-through commits destination x");
    ASSERT_EQ(state.world.projectiles.entries[0].mapY, 0,
              "non-material creature pass-through commits destination y");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "non-material creature pass-through applies F0811 cell parity");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 70,
              "non-material creature pass-through decays kinetic energy");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 30,
              "non-material creature pass-through decays attack");
    ASSERT_EQ(state.world.projectiles.entries[0].scheduledAtTick, 101,
              "non-material creature pass-through reschedules next tick");
}

static void test_projectile_harm_non_material_hits_non_material_creature(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_GHOST;
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].lastSeenPartyTick = 99;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.gameTick = 100;

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = THING_NONE;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "harm non-material projectile despawns on non-material creature hit");
    ASSERT_EQ(groups[0].health[0], 27,
              "harm non-material projectile applies defense-scaled damage to ghost");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 0, 0), 1,
              "harm non-material projectile spawns impact explosion");
    ASSERT_EQ(state.world.timeline.count, 1,
              "harm non-material creature hit schedules C30 reaction");
    ASSERT_EQ(state.world.timeline.events[0].kind, TIMELINE_EVENT_CREATURE_REACTION,
              "harm non-material creature hit schedules creature reaction");
    ASSERT_EQ(state.world.timeline.events[0].aux2, DM1_EVENT_REACTION_HIT_BY_PROJECTILE,
              "harm non-material creature hit stores C30 event type");
}

static void test_projectile_fireball_heals_black_flame_without_explosion(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_BLACK_FLAME;
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 990;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.gameTick = 100;

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = THING_NONE;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "fireball black-flame impact despawns projectile");
    ASSERT_EQ(groups[0].health[0], 1000,
              "fireball black-flame impact heals up to source cap");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 0, 0), 0,
              "fireball black-flame impact skips normal explosion spawn");
}

static void test_projectile_creature_impact_keeps_thrown_sharp_weapon(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    unsigned short squareFirstThings[6];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonWeapon_Compat weapons[1];
    unsigned char rawWeaponData[4];
    unsigned char* rawThingData[DUNGEON_THING_TYPE_COUNT];
    int thingCounts[DUNGEON_THING_TYPE_COUNT];
    struct ProjectileInstance_Compat* projectile;
    unsigned short daggerThing;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(weapons, 0, sizeof(weapons));
    memset(rawWeaponData, 0xFF, sizeof(rawWeaponData));
    memset(rawThingData, 0, sizeof(rawThingData));
    memset(thingCounts, 0, sizeof(thingCounts));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    groups[0].next = THING_ENDOFLIST;
    groups[0].slot = THING_NONE;
    groups[0].creatureType = 3; /* Wizard Eye keeps thrown sharp weapons. */
    groups[0].cells = 0x0F;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    weapons[0].next = THING_ENDOFLIST;
    weapons[0].type = 8; /* Dagger. */
    rawWeaponData[0] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawWeaponData[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawWeaponData[2] = 8;
    rawWeaponData[3] = 0;
    rawThingData[THING_TYPE_WEAPON] = rawWeaponData;
    thingCounts[THING_TYPE_WEAPON] = 1;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 6;
    things.groups = groups;
    things.groupCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    memcpy(things.rawThingData, rawThingData, sizeof(rawThingData));
    memcpy(things.thingCounts, thingCounts, sizeof(thingCounts));

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 0;
    state.world.creatureAI[0].groupMapY = 0;
    state.world.creatureAI[0].creatureType = groups[0].creatureType;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.gameTick = 100;

    daggerThing = make_thing(THING_TYPE_WEAPON, 0);
    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = daggerThing;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "kept thrown sharp weapon impact despawns projectile");
    ASSERT_EQ(groups[0].health[0], 15,
              "kept thrown sharp weapon impact applies defense-scaled damage");
    ASSERT_EQ(groups[0].slot, daggerThing,
              "surviving keep-sharp creature stores thrown dagger in group slot");
    ASSERT_EQ(weapons[0].next, THING_NONE,
              "kept dagger terminates group possession chain");
    ASSERT_EQ(rawWeaponData[0] | (rawWeaponData[1] << 8), THING_NONE,
              "kept dagger raw next mirrors decoded possession chain");
}

static void test_projectile_door_hit_schedules_and_dispatches_destruction(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileInstance_Compat* projectile;
    struct TickInput_Compat input;
    struct TickResult_Compat tickResult;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;
    squareData[0] = square_for_test(DUNGEON_ELEMENT_DOOR,
                                    PROJECTILE_DOOR_STATE_CLOSED_FULL);

    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.gameTick = 100;

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 60;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "blocking door projectile impact despawns projectile");
    /* ReDMCSB PROJEXPL.C F0217 lines 587-600 requests the
     * non-explosion impact thud before the projectile is deleted. */
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_METALLIC_THUD,
              "blocking weapon projectile door impact emits metallic thud");
    ASSERT_EQ(state.audioState.lastMarker, M11_AUDIO_MARKER_COMBAT,
              "blocking weapon projectile door impact maps thud to combat marker");
    ASSERT_EQ(state.world.timeline.count, 1,
              "M11 schedules projectile door destruction event");
    ASSERT_EQ(state.world.timeline.events[0].kind,
              TIMELINE_EVENT_DOOR_DESTRUCTION,
              "scheduled projectile door event uses destruction kind");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 101,
              "projectile door destruction fires one tick later");
    ASSERT_EQ(state.world.timeline.events[0].mapX, 0,
              "projectile door destruction targets impact x");
    ASSERT_EQ(state.world.timeline.events[0].mapY, 0,
              "projectile door destruction targets impact y");

    memset(&input, 0, sizeof(input));
    memset(&tickResult, 0, sizeof(tickResult));
    input.tick = state.world.gameTick;
    input.command = CMD_NONE;
    ASSERT_EQ(F0884_ORCH_AdvanceOneTick_Compat(
                  &state.world, &input, &tickResult) >= 0,
              1,
              "first idle tick before door destruction succeeds");
    ASSERT_EQ(squareData[0] & DUNGEON_SQUARE_MASK_ATTRIBS,
              PROJECTILE_DOOR_STATE_CLOSED_FULL,
              "door remains closed before destruction fire tick");

    memset(&input, 0, sizeof(input));
    memset(&tickResult, 0, sizeof(tickResult));
    input.tick = state.world.gameTick;
    input.command = CMD_NONE;
    ASSERT_EQ(F0884_ORCH_AdvanceOneTick_Compat(
                  &state.world, &input, &tickResult) >= 0,
              1,
              "second idle tick dispatches projectile door destruction");
    ASSERT_EQ(squareData[0] & DUNGEON_SQUARE_MASK_ATTRIBS,
              PROJECTILE_DOOR_STATE_DESTROYED,
              "projectile door destruction dispatch sets door state destroyed");
    ASSERT_EQ(state.world.timeline.count, 0,
              "projectile door destruction event is consumed after dispatch");
}

static void run_projectile_magical_door_zero_adjusted_no_sound_case(
    int projectileSubtype,
    int kineticEnergy,
    const char* label)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[12];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    squareData[(2 * 3) + 1] =
        square_for_test(DUNGEON_ELEMENT_DOOR,
                        PROJECTILE_DOOR_STATE_CLOSED_FULL);

    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 0;
    state.world.party.mapY = 0;
    state.world.party.direction = 1;
    state.world.gameTick = 100;
    state.audioState.lastSoundIndex = -1;
    state.audioState.lastMarker = M11_AUDIO_MARKER_NONE;

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    projectile->projectileSubtype = projectileSubtype;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 1;
    projectile->cell = 2;
    projectile->direction = 1;
    projectile->kineticEnergy = kineticEnergy;
    projectile->attack = 40;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 1;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = THING_NONE;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              label);
    ASSERT_EQ(state.world.explosions.count, 0,
              "zero-adjusted magical door impact creates no explosion");
    ASSERT_EQ(state.audioState.lastSoundIndex, -1,
              "zero-adjusted magical door impact skips M11 fallback sound");
    ASSERT_EQ(state.audioState.lastMarker, M11_AUDIO_MARKER_NONE,
              "zero-adjusted magical door impact leaves audio marker clear");
}

static void test_projectile_magical_door_zero_adjusted_skips_sound(void) {
    run_projectile_magical_door_zero_adjusted_no_sound_case(
        PROJECTILE_SUBTYPE_LIGHTNING_BOLT, 1,
        "zero-adjusted Lightning Bolt door impact despawns projectile");
    run_projectile_magical_door_zero_adjusted_no_sound_case(
        PROJECTILE_SUBTYPE_POISON_BOLT, 3,
        "zero-adjusted Poison Bolt door impact despawns projectile");
}

static void test_projectile_champion_hit_applies_poison_dose(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileInstance_Compat* projectile;
    struct TickInput_Compat input;
    struct TickResult_Compat tickResult;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;

    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 0;
    state.world.party.mapY = 0;
    state.world.party.direction = 1;
    state.world.party.championCount = 2;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 100;
    state.world.party.champions[1].hp.maximum = 100;
    state.world.party.champions[1].stamina.current = 100;
    state.world.party.champions[1].stamina.maximum = 100;
    state.world.party.champions[1].cell = 1;
    state.world.party.champions[1].poisonDose = 0;
    state.world.gameTick = 100;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 3u);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    projectile->ownerKind = PROJECTILE_OWNER_CREATURE;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 0;
    projectile->direction = 3;
    projectile->kineticEnergy = 80;
    projectile->attack = 32;
    projectile->stepEnergy = 10;
    projectile->poisonAttack = 12;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = THING_NONE;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "poison projectile champion impact despawns projectile");
    ASSERT_EQ(state.world.party.champions[1].hp.current, 67,
              "poison projectile champion impact applies projectile and immediate poison damage");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 0, 0), 1,
              "poison projectile champion impact spawns poison cloud explosion");
    ASSERT_EQ(state.world.explosions.entries[0].explosionType,
              C007_EXPLOSION_POISON_CLOUD,
              "poison projectile champion impact maps to poison cloud explosion");
    ASSERT_EQ(state.world.explosions.entries[0].attack, 20,
              "poison projectile champion impact uses kinetic/4 explosion attack");
    ASSERT_EQ(state.world.explosions.entries[0].cell,
              EXPLOSION_CELL_CENTERED,
              "poison projectile champion impact creates centered poison cloud");
    ASSERT_EQ(state.world.explosions.entries[0].centered, 1,
              "poison projectile champion impact marks poison cloud centered");
    ASSERT_EQ(state.world.party.champions[1].poisonDose, 12,
              "poison projectile champion impact applies poison dose after RNG gate");
    ASSERT_EQ(state.world.timeline.count, 1,
              "poison projectile champion impact schedules C75 poison event");
    ASSERT_EQ(state.world.timeline.events[0].kind, TIMELINE_EVENT_STATUS_TIMEOUT,
              "poison projectile champion impact schedules status timeout");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 136,
              "poison projectile champion impact schedules C75 after 36 ticks");
    ASSERT_EQ(state.world.timeline.events[0].aux0, LIFECYCLE_STATUS_POISON,
              "poison projectile champion impact stores C75 status kind");
    ASSERT_EQ(state.world.timeline.events[0].aux1, 11,
              "poison projectile champion impact stores remaining poison attack");
    ASSERT_EQ(state.world.timeline.events[0].aux4, 1,
              "poison projectile champion impact stores champion index in priority byte");
    ASSERT_EQ(state.world.lifecycle.champions[1].poisonEventCount, 1,
              "poison projectile champion impact increments lifecycle poison event count");

    memset(&input, 0, sizeof(input));
    memset(&tickResult, 0, sizeof(tickResult));
    state.world.gameTick = 136;
    input.tick = state.world.gameTick;
    input.command = CMD_NONE;
    ASSERT_EQ(F0884_ORCH_AdvanceOneTick_Compat(
                  &state.world, &input, &tickResult) >= 0,
              1,
              "poison projectile C75 dispatch succeeds");
    ASSERT_EQ(state.world.party.champions[1].hp.current, 66,
              "poison projectile C75 dispatch applies poison tick damage");
    ASSERT_EQ(state.world.party.champions[1].poisonDose, 10,
              "poison projectile C75 dispatch carries remaining attack");
    ASSERT_EQ(state.world.timeline.count, 1,
              "poison projectile C75 dispatch reschedules next poison event");
    ASSERT_EQ(state.world.timeline.events[0].kind, TIMELINE_EVENT_STATUS_TIMEOUT,
              "poison projectile C75 dispatch keeps status timeout kind");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 172,
              "poison projectile C75 dispatch reschedules 36 ticks later");
    ASSERT_EQ(state.world.timeline.events[0].aux0, LIFECYCLE_STATUS_POISON,
              "poison projectile C75 dispatch keeps poison status kind");
    ASSERT_EQ(state.world.timeline.events[0].aux1, 10,
              "poison projectile C75 dispatch stores decremented attack");
    ASSERT_EQ(state.world.timeline.events[0].aux4, 1,
              "poison projectile C75 dispatch preserves champion index");
    ASSERT_EQ(state.world.lifecycle.champions[1].poisonEventCount, 1,
              "poison projectile C75 dispatch keeps one active poison event");
}

static void test_projectile_champion_hit_uses_f0321_defense_scale(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;

    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 0;
    state.world.party.mapY = 0;
    state.world.party.direction = 1;
    state.world.party.championCount = 2;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 100;
    state.world.party.champions[1].hp.maximum = 100;
    state.world.party.champions[1].stamina.current = 100;
    state.world.party.champions[1].stamina.maximum = 100;
    state.world.party.champions[1].cell = 1;
    state.world.magic.partyShieldDefense = 200;
    state.world.gameTick = 100;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 5u);

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    projectile->attackTypeCode = COMBAT_ATTACK_LIGHTNING;
    projectile->ownerKind = PROJECTILE_OWNER_CREATURE;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 0;
    projectile->direction = 3;
    projectile->kineticEnergy = 60;
    projectile->attack = 30;
    projectile->stepEnergy = 5;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = THING_NONE;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "shielded projectile champion impact despawns projectile");
    ASSERT_EQ(state.world.party.champions[1].hp.current, 86,
              "shielded lightning champion impact uses F0321 defense scaling instead of raw attack");
    ASSERT_EQ(state.world.party.champions[1].wounds, COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO,
              "shielded projectile champion impact applies source wound mask when damage lands");
    ASSERT_EQ(state.partyDead, 0,
              "shielded projectile champion impact does not trip party-dead gate");
}

static void test_projectile_champion_hit_can_kill_party(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct ProjectileInstance_Compat* projectile;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;

    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 0;
    state.world.party.mapY = 0;
    state.world.party.direction = 1;
    state.world.party.championCount = 2;
    state.world.party.champions[0].present = 0;
    state.world.party.champions[0].hp.current = 0;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 20;
    state.world.party.champions[1].hp.maximum = 20;
    state.world.party.champions[1].cell = 1;
    state.world.party.champions[1].name[0] = 'S';
    state.world.party.champions[1].name[1] = 'O';
    state.world.party.champions[1].name[2] = 'N';
    state.world.party.champions[1].name[3] = 'J';
    state.world.gameTick = 100;

    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CREATURE;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 0;
    projectile->direction = 3;
    projectile->kineticEnergy = 60;
    projectile->attack = 30;
    projectile->stepEnergy = 5;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = make_thing(THING_TYPE_WEAPON, 0);
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "lethal projectile champion impact despawns projectile");
    ASSERT_EQ(state.world.party.champions[1].hp.current, 0,
              "lethal projectile champion impact clamps champion HP to zero");
    ASSERT_EQ(state.partyDead, 1,
              "lethal projectile champion impact sets M11 party-dead gate");
    ASSERT_EQ(state.world.partyDead, 1,
              "lethal projectile champion impact mirrors M10 party-dead flag");
}

static void test_thrown_potion_wall_impact_consumes_potion_thing(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[6];
    struct DungeonThings_Compat things;
    struct DungeonPotion_Compat potions[1];
    unsigned char rawPotionData[4];
    unsigned char* rawThingData[DUNGEON_THING_TYPE_COUNT];
    int thingCounts[DUNGEON_THING_TYPE_COUNT];
    struct ProjectileInstance_Compat* projectile;
    unsigned short potionThing;
    int i;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(&things, 0, sizeof(things));
    memset(potions, 0, sizeof(potions));
    memset(rawPotionData, 0, sizeof(rawPotionData));
    memset(rawThingData, 0, sizeof(rawThingData));
    memset(thingCounts, 0, sizeof(thingCounts));
    for (i = 0; i < 6; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[0] = square_for_test(DUNGEON_ELEMENT_WALL, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 2;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 6;

    potions[0].next = THING_ENDOFLIST;
    potions[0].power = 77;
    potions[0].type = 3; /* ReDMCSB C03_POTION_VEN_POTION. */
    potions[0].doNotDiscard = 0;
    rawPotionData[0] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawPotionData[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawPotionData[2] = 77;
    rawPotionData[3] = 3;
    rawThingData[THING_TYPE_POTION] = rawPotionData;
    thingCounts[THING_TYPE_POTION] = 1;

    things.loaded = 1;
    things.potions = potions;
    things.potionCount = 1;
    memcpy(things.rawThingData, rawThingData, sizeof(rawThingData));
    memcpy(things.thingCounts, thingCounts, sizeof(thingCounts));

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;
    state.world.gameTick = 100;

    potionThing = make_thing(THING_TYPE_POTION, 0);
    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 1;
    projectile->mapY = 0;
    projectile->cell = 3;
    projectile->direction = 3;
    projectile->kineticEnergy = 12;
    projectile->attack = 5;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->associatedPotionPower = 77;
    projectile->flags = PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT;
    projectile->reserved1 = potionThing;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "thrown potion wall impact despawns projectile");
    ASSERT_EQ(M11_GameView_CountCellExplosions(&state.world, 0, 0, 0), 1,
              "thrown potion wall impact spawns explosion on wall cell");
    ASSERT_EQ(state.world.explosions.entries[0].explosionType,
              C007_EXPLOSION_POISON_CLOUD,
              "thrown Ven potion impact maps to poison cloud");
    ASSERT_EQ(state.world.explosions.entries[0].attack, 77,
              "thrown potion impact explosion uses potion power");
    ASSERT_EQ(state.world.explosions.entries[0].cell,
              EXPLOSION_CELL_CENTERED,
              "thrown Ven potion impact creates centered poison cloud");
    ASSERT_EQ(potions[0].next, THING_NONE,
              "thrown potion impact consumes decoded potion thing");
    ASSERT_EQ(rawPotionData[0] | (rawPotionData[1] << 8), THING_NONE,
              "thrown potion impact consumes raw potion thing");
    ASSERT_EQ(rawPotionData[2], 77,
              "thrown potion impact preserves raw potion power byte");
    ASSERT_EQ(rawPotionData[3], 3,
              "thrown potion impact preserves raw potion type byte");
}

static void test_thrown_weapon_wall_impact_materializes_source_square(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[1];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned char rawWeaponData[4];
    unsigned char* rawThingData[DUNGEON_THING_TYPE_COUNT];
    int thingCounts[DUNGEON_THING_TYPE_COUNT];
    struct ProjectileInstance_Compat* projectile;
    unsigned short thrownThing;
    unsigned short materializedThing;

    seed_state(&state, 100, 100);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    memset(rawWeaponData, 0, sizeof(rawWeaponData));
    memset(rawThingData, 0, sizeof(rawThingData));
    memset(thingCounts, 0, sizeof(thingCounts));

    squareData[0] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    squareFirstThings[0] = THING_ENDOFLIST;
    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 1;
    maps[0].height = 1;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 1;

    weapons[0].type = 8; /* ReDMCSB C08_WEAPON_DAGGER. */
    weapons[0].next = THING_NONE;
    rawWeaponData[0] = (unsigned char)(THING_NONE & 0xFFu);
    rawWeaponData[1] = (unsigned char)((THING_NONE >> 8) & 0xFFu);
    rawWeaponData[2] = 8;
    rawWeaponData[3] = 0;
    rawThingData[THING_TYPE_WEAPON] = rawWeaponData;
    thingCounts[THING_TYPE_WEAPON] = 1;

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    memcpy(things.rawThingData, rawThingData, sizeof(rawThingData));
    memcpy(things.thingCounts, thingCounts, sizeof(thingCounts));

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 0;
    state.world.party.mapY = 0;
    state.world.party.direction = 1;

    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    materializedThing = (unsigned short)(((1 & 0x03) << 14) | thrownThing);
    ASSERT_EQ(squareFirstThings[0], THING_ENDOFLIST,
              "test fixture starts with an empty source square chain");
    state.world.projectiles.count = 1;
    projectile = &state.world.projectiles.entries[0];
    memset(projectile, 0, sizeof(*projectile));
    projectile->slotIndex = 0;
    projectile->projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile->ownerKind = PROJECTILE_OWNER_CHAMPION;
    projectile->ownerIndex = 0;
    projectile->mapIndex = 0;
    projectile->mapX = 0;
    projectile->mapY = 0;
    projectile->cell = 1;
    projectile->direction = 1;
    projectile->kineticEnergy = 12;
    projectile->attack = 5;
    projectile->stepEnergy = 10;
    projectile->firstMoveGraceFlag = 0;
    projectile->launchedAtTick = 99;
    projectile->scheduledAtTick = 100;
    projectile->reserved1 = thrownThing;
    projectile->reserved3 = 1;

    M11_GameView_AdvanceProjectilesOnce(&state);

    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "thrown dagger wall impact despawns projectile");
    ASSERT_EQ(squareFirstThings[0], materializedThing,
              "thrown dagger wall impact materializes on projectile source square");
    ASSERT_EQ(THING_GET_CELL(squareFirstThings[0]), 1,
              "materialized thrown dagger preserves projectile source cell");
}

static void test_leader_hand_throw_uses_f0328_temporary_action_hand(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned short thrownThing;
    unsigned short actionHandThing;

    seed_state(&state, 100, 100);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    weapons[0].type = 8; /* Dagger: weight 5, class 2, kinetic 19. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 0;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    actionHandThing = make_thing(THING_TYPE_JUNK, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        actionHandThing;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, thrownThing), 1,
              "leader hand accepts throw object");

    /* ReDMCSB viewport origin is x=0,y=33.  Local x=120 selects the
     * right-side F0329/F0328 throw route; local y=20 is the upper
     * C080 throw zone from CLIKVIEW.C F0375. */
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 120, 53, 1),
              M11_GAME_INPUT_REDRAW,
              "leader-hand C080 click throws through F0329/F0328");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "leader-hand throw creates one live projectile");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "accepted leader-hand throw clears leader hand");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              actionHandThing,
              "leader-hand throw restores existing action-hand object");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 98,
              "leader-hand throw spends F0305 object-weight stamina");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_THROW].experience,
              16,
              "leader-hand throw awards F0328/F0304 Throw hidden-skill XP");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "leader-hand throw uses explicit right-side launch cell");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "leader-hand throw keeps projectile direction at party direction");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 82,
              "leader-hand throw passes F0328 kinetic energy to projectile create");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 40,
              "leader-hand throw passes F0328 bounded attack to projectile create");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 10,
              "leader-hand throw passes F0328 step energy to projectile create");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, thrownThing,
              "leader-hand throw preserves Thing identity on projectile");
    ASSERT_EQ(state.world.projectileDisabledMovementTicks, 4,
              "leader-hand throw sets source movement-disable ticks without action-row tick decrement");
    ASSERT_EQ(state.world.lastProjectileDisabledMovementDirection, 1,
              "leader-hand throw records source movement-disable direction");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_COMBAT,
              "leader-hand throw requests the F0328 M563 combat sound");
    ASSERT_EQ(state.actionDisabledTicks[0], 4,
              "leader-hand throw applies F0328/F0330 four-tick action disable");
    ASSERT_EQ(state.actionDisabledIndex[0], 0xFF,
              "leader-hand throw has no F0407 action index override");
    ASSERT_EQ(state.actionEnableSlotOrdinal[0], 0,
              "leader-hand throw keeps F0330's zero enable-action slot ordinal");
}

static void test_leader_hand_throw_full_projectile_list_accepts_f0328(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned short thrownThing;
    unsigned short actionHandThing;

    seed_state(&state, 100, 100);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    weapons[0].type = 8; /* Dagger: weight 5, class 2, kinetic 19. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 0;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    state.world.projectiles.count = PROJECTILE_LIST_CAPACITY;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_WEAPON, 0);
    actionHandThing = make_thing(THING_TYPE_JUNK, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        actionHandThing;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, thrownThing), 1,
              "leader hand accepts full-list throw object");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 120, 53, 1),
              M11_GAME_INPUT_REDRAW,
              "full-list leader-hand C080 click still accepts F0329/F0328");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state),
              PROJECTILE_LIST_CAPACITY,
              "full-list leader-hand throw does not allocate past PJE-05 cap");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "accepted full-list leader-hand throw clears leader hand");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              actionHandThing,
              "full-list leader-hand throw restores existing action hand");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 98,
              "full-list leader-hand throw spends F0305 object-weight stamina");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_THROW].experience,
              16,
              "full-list leader-hand throw awards F0328 Throw XP");
    ASSERT_EQ(state.world.projectileDisabledMovementTicks, 0,
              "dropped full-list leader-hand throw creates no movement lockout");
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_COMBAT,
              "full-list leader-hand throw still requests F0328 M563 sound");
    ASSERT_EQ(state.actionDisabledTicks[0], 4,
              "full-list leader-hand throw applies F0328/F0330 disable");
    ASSERT_EQ(state.actionDisabledIndex[0], 0xFF,
              "full-list leader-hand throw has no F0407 action index override");
    ASSERT_EQ(state.actionEnableSlotOrdinal[0], 0,
              "full-list leader-hand throw keeps F0330's zero enable slot");
}

static void test_leader_hand_throw_waterskin_uses_f0140_charge_weight(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned short thrownThing;

    seed_state(&state, 100, 100);
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    junks[0].next = THING_ENDOFLIST;
    junks[0].type = 1; /* ReDMCSB C01_JUNK_WATERSKIN. */
    junks[0].chargeCount = 3;
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 0;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    thrownThing = make_thing(THING_TYPE_JUNK, 0);
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, thrownThing), 1,
              "leader hand accepts waterskin throw object");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 120, 53, 1),
              M11_GAME_INPUT_REDRAW,
              "leader-hand waterskin click throws through F0329/F0328");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "leader-hand waterskin throw creates one live projectile");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 96,
              "leader-hand waterskin throw spends F0305 from F0140 base+charge weight");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 48,
              "leader-hand waterskin throw includes F0140 charge weight in F0312 strength");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, thrownThing,
              "leader-hand waterskin throw preserves Thing identity on projectile");
}

static void test_leader_hand_throw_container_uses_f0140_recursive_weight(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonContainer_Compat containers[1];
    struct DungeonJunk_Compat junks[1];
    unsigned short chestThing;
    unsigned short waterskinThing;

    seed_state(&state, 100, 100);
    memset(&things, 0, sizeof(things));
    memset(containers, 0, sizeof(containers));
    memset(junks, 0, sizeof(junks));
    waterskinThing = make_thing(THING_TYPE_JUNK, 0);
    chestThing = make_thing(THING_TYPE_CONTAINER, 0);
    junks[0].next = THING_ENDOFLIST;
    junks[0].type = 1; /* ReDMCSB C01_JUNK_WATERSKIN. */
    junks[0].chargeCount = 3;
    containers[0].next = THING_ENDOFLIST;
    containers[0].slot = waterskinThing;
    things.loaded = 1;
    things.containers = containers;
    things.containerCount = 1;
    things.junks = junks;
    things.junkCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = 50;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 0;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 40;
    state.world.party.champions[0].maxLoad = 420;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, chestThing), 1,
              "leader hand accepts container throw object");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 120, 53, 1),
              M11_GAME_INPUT_REDRAW,
              "leader-hand container click throws through F0329/F0328");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "leader-hand container throw creates one live projectile");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 77,
              "leader-hand container throw spends F0305 from recursive F0140 weight");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 16,
              "leader-hand container throw includes recursive F0140 weight in F0312 strength");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, chestThing,
              "leader-hand container throw preserves Thing identity on projectile");
}

static void test_block_action_disables_champion_for_source_ticks(void) {
    M11_GameViewState state;
    DM1_ActionXpRoute route;
    int expectedActionXp;
    int expectedStaminaCost;
    int i;
    seed_state(&state, 30, 1);

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_BLOCK, &route), 1,
              "BLOCK has a source G0496/G0497 route");
    if (!route.valid) return;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    expectedActionXp = route.experienceGain * 2;
    expectedStaminaCost =
        dm1_v1_graphic560_action_stamina_get_pc34(DM1_ACTION_BLOCK);

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_BLOCK),
              1,
              "direct BLOCK returns F0407 ActionPerformed true");
    ASSERT_EQ(state.world.party.champions[0].stamina.current,
              30 - expectedStaminaCost,
              "BLOCK spends source G0494 action stamina before F0325 tail");
    ASSERT_EQ(state.actionDisabledTicks[0], 6,
              "BLOCK applies G0491 six-tick action disable");
    ASSERT_EQ(state.actionDisabledIndex[0], 1,
              "BLOCK records disabled action index");
    ASSERT_EQ(state.world.party.champions[0].actionDefense, 36,
              "BLOCK applies G0495 action defense modifier");
    ASSERT_EQ(state.world.party.champions[0].actionIndex, 1,
              "BLOCK stores champion action index while disabled");
    ASSERT_EQ(route.skillIndex, LIFECYCLE_SKILL_PARRY,
              "BLOCK routes source G0496 XP to Parry");
    ASSERT_EQ(route.baseSkillIndex, LIFECYCLE_SKILL_FIGHTER,
              "BLOCK Parry XP propagates to Fighter base skill");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_PARRY].experience,
              expectedActionXp,
              "direct BLOCK awards F0407 G0497 XP to Parry through F0304");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[LIFECYCLE_SKILL_FIGHTER].experience,
              expectedActionXp,
              "direct BLOCK propagates F0407 G0497 XP to Fighter through F0304");
    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 0,
              "disabled champion cannot reopen action menu");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 1), 0,
              "direct non-melee helper respects action-disabled gate");
    ASSERT_EQ(state.world.party.champions[0].actionDefense, 36,
              "rejected disabled action does not stack action defense");

    for (i = 0; i < 5; ++i) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.actionDisabledTicks[0], 1,
              "action disable counts down on accepted idle ticks");
    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 0,
              "last disabled tick still blocks action menu");

    (void)M11_GameView_AdvanceIdleTick(&state);
    ASSERT_EQ(state.actionDisabledTicks[0], 0,
              "action disable clears after source tick budget");
    ASSERT_EQ(state.actionDisabledIndex[0], 255,
              "cleared action disable resets action index");
    ASSERT_EQ(state.world.party.champions[0].actionDefense, 0,
              "action enable removes G0495 action defense modifier");
    ASSERT_EQ(state.world.party.champions[0].actionIndex, 255,
              "action enable clears champion action index");
    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "champion can reopen action menu after disable clears");
}

static void test_direct_parry_empty_front_uses_f0402_failure_tail(void) {
    M11_GameViewState state;
    DM1_ActionXpRoute route;
    int expectedXp;

    seed_state(&state, 30, 3);
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_PARRY, &route), 1,
              "PARRY has a source G0496/G0497 route");
    if (!route.valid) return;
    expectedXp = (route.experienceGain >> 1) * 2;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_PARRY),
              0,
              "direct PARRY without a melee target returns F0402 failure");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_PARRY) >> 1,
              "direct PARRY empty-front failure halves disabled ticks");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_PARRY,
              "direct PARRY empty-front failure records PARRY as disabled action");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 29,
              "direct PARRY still spends F0407 common-tail stamina");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedXp,
              "direct PARRY empty-front failure halves G0497 Parry XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedXp,
              "direct PARRY empty-front failure propagates halved XP");
}

static void test_freeze_life_common_branch_decrements_charges(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];

    seed_state(&state, 100, 31);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 3;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FREEZE_LIFE),
              1,
              "FREEZE LIFE common branch performs F0407 effect");
    ASSERT_EQ(weapons[0].chargeCount, 2,
              "FREEZE LIFE common branch decrements charges through F0405");
    ASSERT_EQ(state.world.freezeLifeTicks > 0, 1,
              "FREEZE LIFE leaves a live freeze-life tick budget");
}

static void test_freeze_life_blue_box_consumes_action_hand(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned char rawJunkData[4];

    seed_state(&state, 100, 32);
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    memset(rawJunkData, 0, sizeof(rawJunkData));
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    things.rawThingData[THING_TYPE_JUNK] = rawJunkData;
    things.thingCounts[THING_TYPE_JUNK] = 1;
    junks[0].type = 42;
    junks[0].chargeCount = 7;
    junks[0].next = THING_ENDOFLIST;
    rawJunkData[0] = 0xFEu;
    rawJunkData[1] = 0xFFu;
    rawJunkData[2] = 42u;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_JUNK, 0);

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FREEZE_LIFE),
              1,
              "FREEZE LIFE blue magical box performs F0407 effect");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              THING_NONE,
              "FREEZE LIFE blue magical box removes action-hand object");
    ASSERT_EQ(junks[0].next, THING_NONE,
              "FREEZE LIFE blue magical box unlinks removed Thing");
    ASSERT_EQ(junks[0].chargeCount, 7,
              "FREEZE LIFE blue magical box is consumed without F0405 charges");
    ASSERT_EQ(state.world.freezeLifeTicks, 29,
              "FREEZE LIFE blue magical box adds 30 ticks before CMD_NONE");
    ASSERT_EQ(rawJunkData[0], 0xFFu,
              "FREEZE LIFE blue magical box raw next low byte is none");
    ASSERT_EQ(rawJunkData[1], 0xFFu,
              "FREEZE LIFE blue magical box raw next high byte is none");
}

static void test_freeze_life_green_box_consumes_action_hand_and_caps(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned char rawJunkData[4];

    seed_state(&state, 100, 34);
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    memset(rawJunkData, 0, sizeof(rawJunkData));
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    things.rawThingData[THING_TYPE_JUNK] = rawJunkData;
    things.thingCounts[THING_TYPE_JUNK] = 1;
    junks[0].type = 43;
    junks[0].chargeCount = 9;
    junks[0].next = THING_ENDOFLIST;
    rawJunkData[0] = 0xFEu;
    rawJunkData[1] = 0xFFu;
    rawJunkData[2] = 43u;
    state.world.things = &things;
    state.world.freezeLifeTicks = 100;
    state.world.magic.freezeLifeTicks = 100;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_JUNK, 0);

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FREEZE_LIFE),
              1,
              "FREEZE LIFE green magical box performs F0407 effect");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_ACTION_HAND],
              THING_NONE,
              "FREEZE LIFE green magical box removes action-hand object");
    ASSERT_EQ(junks[0].next, THING_NONE,
              "FREEZE LIFE green magical box unlinks removed Thing");
    ASSERT_EQ(junks[0].chargeCount, 9,
              "FREEZE LIFE green magical box is consumed without F0405 charges");
    ASSERT_EQ(state.world.freezeLifeTicks, 199,
              "FREEZE LIFE green magical box caps at 200 before CMD_NONE");
    ASSERT_EQ(rawJunkData[0], 0xFFu,
              "FREEZE LIFE green magical box raw next low byte is none");
    ASSERT_EQ(rawJunkData[1], 0xFFu,
              "FREEZE LIFE green magical box raw next high byte is none");
}

static void test_light_decrements_action_hand_charges(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    int i;
    int guard;

    seed_state(&state, 100, 33);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 3;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_LIGHT, &route), 1,
              "LIGHT has a source G0496/G0497 route");
    if (!route.valid) return;
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_LIGHT),
              1,
              "LIGHT performs F0407 magical light effect");
    ASSERT_EQ(state.world.magic.magicalLightAmount, 12,
              "LIGHT adds Graphic562 power-2 light amount");
    ASSERT_EQ(M11_GameView_GetDungeonPaletteIndex(&state), 4,
              "LIGHT refreshes M11 dungeon palette through F0337");
    ASSERT_EQ(weapons[0].chargeCount, 2,
              "LIGHT decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.timeline.count, 1,
              "LIGHT schedules F0404/C70 magical light decay");
    ASSERT_EQ(state.world.timeline.events[0].kind,
              TIMELINE_EVENT_MAGIC_LIGHT_DECAY,
              "LIGHT decay uses the M10 F0257 event kind");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 2533,
              "LIGHT schedules first decay at GameTime + 2500");
    ASSERT_EQ(state.world.timeline.events[0].aux0, -2,
              "LIGHT stores negative light power for later removal");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              route.experienceGain,
              "direct LIGHT awards full F0407 G0497 XP to the action skill");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              route.experienceGain,
              "direct LIGHT propagates full F0407 G0497 XP to the base skill");

    guard = 0;
    while (state.world.gameTick <= 2533U && guard++ < 2600) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.world.magic.magicalLightAmount, 5,
              "first LIGHT decay removes power-2 to power-1 delta");
    ASSERT_EQ(M11_GameView_GetDungeonPaletteIndex(&state), 4,
              "first LIGHT decay keeps the source F0337 dim palette");
    ASSERT_EQ(state.world.timeline.count, 1,
              "first LIGHT decay schedules weaker follow-up");
    ASSERT_EQ(state.world.timeline.events[0].kind,
              TIMELINE_EVENT_MAGIC_LIGHT_DECAY,
              "weaker LIGHT decay keeps F0257 event kind");
    ASSERT_EQ((int)state.world.timeline.events[0].fireAtTick, 2537,
              "weaker LIGHT decay fires four ticks later");
    ASSERT_EQ(state.world.timeline.events[0].aux0, -1,
              "weaker LIGHT decay stores negative power 1");

    guard = 0;
    while (state.world.gameTick <= 2537U && guard++ < 16) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.world.magic.magicalLightAmount, 0,
              "final LIGHT decay removes remaining power-1 light");
    ASSERT_EQ(M11_GameView_GetDungeonPaletteIndex(&state), 5,
              "final LIGHT decay returns the M11 dungeon palette to darkest");
    for (i = 0; i < state.world.timeline.count; ++i) {
        ASSERT_EQ(state.world.timeline.events[i].kind !=
                      TIMELINE_EVENT_MAGIC_LIGHT_DECAY,
                  1,
                  "final LIGHT decay leaves no light-decay follow-up");
    }
}

static void test_heal_action_uses_hidden_heal_skill(void) {
    M11_GameViewState state;

    seed_state(&state, 100, 39);
    state.world.party.champions[0].hp.current = 80;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].mana.current = 10;
    state.world.party.champions[0].mana.maximum = 20;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PRIEST].experience = 0;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_HEAL].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_PRIEST), 1,
              "fixture keeps base Priest skill low");
    ASSERT_EQ(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_HEAL), 5,
              "fixture gives hidden Heal skill a distinct source level");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_HEAL),
              1,
              "HEAL action succeeds through F0407");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 100,
              "F0407 HEAL uses hidden Heal skill capacity, not base Priest");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 2,
              "F0407 HEAL spends two mana per source healing cycle");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_HEAL].experience,
              10010,
              "F0407 HEAL awards 2 plus 2 XP per healing cycle");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_PRIEST].experience,
              10,
              "F0407 HEAL propagates healing-loop XP to base Priest");
}

static void test_heal_no_effect_still_runs_f0407_tail(void) {
    M11_GameViewState state;
    DM1_ActionXpRoute route;

    seed_state(&state, 100, 60);
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.world.party.champions[0].mana.current = 10;
    state.world.party.champions[0].mana.maximum = 20;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_HEAL].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_HEAL, &route), 1,
              "HEAL has a source G0496/G0497 route");
    ASSERT_EQ(route.experienceGain, 5,
              "PC34 EN/I34E G0497 keeps no-effect HEAL table XP");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_HEAL),
              1,
              "no-effect HEAL still returns F0407 ActionPerformed true");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 100,
              "no-effect HEAL leaves full health unchanged");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 10,
              "no-effect HEAL does not enter the mana-spending heal loop");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_HEAL),
              "no-effect HEAL keeps the common F0407 action-disabled tail");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_HEAL].experience,
              10000 + route.experienceGain,
              "no-effect HEAL awards the common G0497 table XP to Heal");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[DM1_SKILL_IDX_PRIEST].experience,
              route.experienceGain,
              "no-effect HEAL propagates common G0497 XP to Priest");
}

static void test_window_action_schedules_thieves_eye_and_decrements_charges(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    uint32_t initialTick;
    uint32_t expiryTick = 0;
    int skillLevel;
    int expiryIndex = -1;
    int i;
    int guard;

    seed_state(&state, 100, 45);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 3;
    state.world.things = &things;
    state.world.party.mapIndex = 2;
    state.world.party.mapX = 4;
    state.world.party.mapY = 5;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_EARTH].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    initialTick = state.world.gameTick;
    skillLevel = M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_EARTH);

    ASSERT_EQ(skillLevel, 5,
              "fixture gives WINDOW/Earth skill a distinct source level");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_WINDOW, &route), 1,
              "WINDOW has a source G0496/G0497 route");
    if (!route.valid) return;
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_WINDOW),
              1,
              "WINDOW performs F0407 Thieves Eye effect");
    ASSERT_EQ(weapons[0].chargeCount, 2,
              "WINDOW decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.lifecycle.status.thievesEyeCount, 1,
              "WINDOW increments the party C73 Thieves Eye counter");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              10000 + route.experienceGain,
              "direct WINDOW awards full F0407 G0497 XP to Earth");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              route.experienceGain,
              "direct WINDOW propagates full F0407 G0497 XP to Wizard");
    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
            state.world.timeline.events[i].aux0 == LIFECYCLE_STATUS_THIEVES_EYE) {
            expiryIndex = i;
            break;
        }
    }
    ASSERT_EQ(expiryIndex >= 0, 1,
              "WINDOW schedules C73 Thieves Eye expiry event");
    if (expiryIndex >= 0) {
        expiryTick = state.world.timeline.events[expiryIndex].fireAtTick;
        ASSERT_EQ(expiryTick >= initialTick + 5U, 1,
                  "WINDOW expiry has source minimum duration");
        ASSERT_EQ(expiryTick <= initialTick + (uint32_t)(skillLevel + 12), 1,
                  "WINDOW expiry is bounded by random(skill + 8) + 5");
        ASSERT_EQ(state.world.timeline.events[expiryIndex].mapIndex, 2,
                  "WINDOW stores current party map index on C73 event");
        ASSERT_EQ(state.world.timeline.events[expiryIndex].mapX, 4,
                  "WINDOW stores current party X on C73 event");
        ASSERT_EQ(state.world.timeline.events[expiryIndex].mapY, 5,
                  "WINDOW stores current party Y on C73 event");
        ASSERT_EQ(state.world.timeline.events[expiryIndex].cell, -1,
                  "WINDOW C73 event is party-scoped");
    }

    guard = 0;
    while (expiryTick > 0 && state.world.gameTick <= expiryTick && guard++ < 64) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.world.lifecycle.status.thievesEyeCount, 0,
              "C73 expiry decrements the Thieves Eye counter");
    for (i = 0; i < state.world.timeline.count; ++i) {
        ASSERT_EQ(!(state.world.timeline.events[i].kind ==
                        TIMELINE_EVENT_STATUS_TIMEOUT &&
                    state.world.timeline.events[i].aux0 ==
                        LIFECYCLE_STATUS_THIEVES_EYE),
                  1,
                  "C73 expiry leaves no Thieves Eye follow-up");
    }
}

static void test_spit_action_launches_f0327_fireball_and_decrements_charges(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];

    seed_state(&state, 100, 47);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 3;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIRE].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_FIRE), 5,
              "fixture gives SPIT/Fire skill a distinct source level");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_SPIT),
              1,
              "SPIT performs F0407/F0327 projectile route");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 7,
              "SPIT spends 7 - min(6, Fire skill) mana");
    ASSERT_EQ(weapons[0].chargeCount, 2,
              "SPIT decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "SPIT mirrors F0406 champion direction to party direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "SPIT creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileCategory,
              PROJECTILE_CATEGORY_MAGICAL,
              "SPIT projectile is magical");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              PROJECTILE_SUBTYPE_FIREBALL,
              "SPIT projectile uses source fireball explosion thing");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 250,
              "SPIT uses F0407 kinetic energy 250");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "SPIT uses F0327 fixed projectile attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "SPIT uses F0327 step energy from maximum mana");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "SPIT launch cell follows F0326 champion Cell formula");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "SPIT launch direction follows party/champion direction");
    ASSERT_EQ(state.world.projectiles.entries[0].firstMoveGraceFlag, 1,
              "SPIT schedules first-move grace like F0212 projectile create");
    ASSERT_EQ(state.world.timeline.count, 1,
              "SPIT schedules first projectile movement event");
    ASSERT_EQ(state.world.timeline.events[0].kind,
              TIMELINE_EVENT_PROJECTILE_MOVE,
              "SPIT first event is projectile movement");
}

static void test_spit_low_mana_scales_kinetic_energy_before_f0327(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    int fireSkillLevel;
    int requiredMana;
    int expectedKineticEnergy;

    seed_state(&state, 100, 59);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 3;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    fireSkillLevel = M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_FIRE);
    if (fireSkillLevel < 0) fireSkillLevel = 0;
    requiredMana = 7 - (fireSkillLevel > 6 ? 6 : fireSkillLevel);
    if (requiredMana < 1) requiredMana = 1;
    expectedKineticEnergy = 3 * 250 / requiredMana;
    if (expectedKineticEnergy < 2) expectedKineticEnergy = 2;

    ASSERT_EQ(requiredMana > 3, 1,
              "fixture forces the SPIT CurrentMana < RequiredMana branch");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_SPIT),
              1,
              "low-mana SPIT still performs F0407/F0327 projectile route");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 0,
              "low-mana SPIT spends all available mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "low-mana SPIT decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "low-mana SPIT still mirrors F0406 direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "low-mana SPIT creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileCategory,
              PROJECTILE_CATEGORY_MAGICAL,
              "low-mana SPIT creates a magical projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              PROJECTILE_SUBTYPE_FIREBALL,
              "low-mana SPIT keeps source fireball projectile subtype");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy,
              expectedKineticEnergy,
              "F0407 scales SPIT kinetic energy as currentMana * 250 / requiredMana");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "low-mana SPIT still uses F0327 fixed attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "low-mana SPIT still uses F0327 step energy from maximum mana");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "low-mana SPIT launch cell follows champion Cell formula");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "low-mana SPIT launch direction follows party direction");
}

static void test_fireball_action_uses_f0327_and_decrements_charges(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];

    seed_state(&state, 100, 49);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIRE].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FIREBALL),
              1,
              "FIREBALL action performs F0407/F0327 projectile route");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 7,
              "FIREBALL spends 7 - min(6, Fire skill) mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "FIREBALL decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "FIREBALL mirrors F0406 champion direction to party direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "FIREBALL creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              PROJECTILE_SUBTYPE_FIREBALL,
              "FIREBALL uses source fireball projectile subtype");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 150,
              "FIREBALL uses F0407 kinetic energy 150");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "FIREBALL uses F0327 fixed projectile attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "FIREBALL uses F0327 step energy from maximum mana");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "FIREBALL launch cell follows F0326 champion Cell formula");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "FIREBALL launch direction follows party/champion direction");
}

static void test_fireball_low_mana_scales_kinetic_energy_before_f0327(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    int fireSkillLevel;
    int requiredMana;
    int expectedKineticEnergy;

    seed_state(&state, 100, 55);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 3;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    fireSkillLevel = M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_FIRE);
    if (fireSkillLevel < 0) fireSkillLevel = 0;
    requiredMana = 7 - (fireSkillLevel > 6 ? 6 : fireSkillLevel);
    if (requiredMana < 1) requiredMana = 1;
    expectedKineticEnergy = 3 * 150 / requiredMana;
    if (expectedKineticEnergy < 2) expectedKineticEnergy = 2;

    ASSERT_EQ(requiredMana > 3, 1,
              "fixture forces the F0407 CurrentMana < RequiredMana branch");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FIREBALL),
              1,
              "low-mana FIREBALL still performs F0327 projectile route");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 0,
              "low-mana FIREBALL spends all available mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "low-mana FIREBALL decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "low-mana FIREBALL still mirrors F0406 direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "low-mana FIREBALL creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              PROJECTILE_SUBTYPE_FIREBALL,
              "low-mana FIREBALL keeps source projectile subtype");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy,
              expectedKineticEnergy,
              "F0407 scales kinetic energy as currentMana * 150 / requiredMana");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "low-mana FIREBALL still uses F0327 fixed attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "low-mana FIREBALL still uses F0327 step energy from maximum mana");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "low-mana FIREBALL launch cell follows champion Cell formula");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "low-mana FIREBALL launch direction follows party direction");
}

static void run_air_projectile_action_uses_f0327_and_direction_case(
    int actionIndex,
    int expectedSubtype,
    int expectedKineticEnergy,
    int expectedAttackType,
    const char* actionName)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];

    seed_state(&state, 100, 52);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_AIR].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, actionIndex),
              1,
              actionName);
    ASSERT_EQ(state.world.party.champions[0].mana.current, 7,
              "Air projectile action spends 7 - min(6, Air skill) mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "Air projectile action decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "Air projectile action mirrors F0406 champion direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "Air projectile action creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileCategory,
              PROJECTILE_CATEGORY_MAGICAL,
              "Air projectile action creates a magical projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              expectedSubtype,
              "Air projectile action uses source projectile subtype");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy,
              expectedKineticEnergy,
              "Air projectile action uses source kinetic energy");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "Air projectile action uses F0327 fixed projectile attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].attackTypeCode,
              expectedAttackType,
              "Air projectile action uses source attack type");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "Air projectile action uses F0327 step energy from maximum mana");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "Air projectile launch cell follows F0326 champion Cell formula");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "Air projectile launch direction follows party/champion direction");
}

static void test_air_projectile_actions_use_f0327_and_f0406_direction(void) {
    run_air_projectile_action_uses_f0327_and_direction_case(
        DM1_ACTION_DISPELL, PROJECTILE_SUBTYPE_HARM_NON_MATERIAL, 150,
        COMBAT_ATTACK_MAGIC,
        "DISPELL action performs F0407/F0327 projectile route");
    run_air_projectile_action_uses_f0327_and_direction_case(
        DM1_ACTION_LIGHTNING, PROJECTILE_SUBTYPE_LIGHTNING_BOLT, 180,
        COMBAT_ATTACK_LIGHTNING,
        "LIGHTNING action performs F0407/F0327 projectile route");
}

static void run_air_projectile_low_mana_scales_kinetic_case(
    int actionIndex,
    int expectedSubtype,
    int baseKineticEnergy,
    int expectedAttackType,
    const char* actionName)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    int airSkillLevel;
    int requiredMana;
    int expectedKineticEnergy;

    seed_state(&state, 100, 56);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 3;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    airSkillLevel = M11_GameView_GetSkillLevel(&state, 0, DM1_SKILL_IDX_AIR);
    if (airSkillLevel < 0) airSkillLevel = 0;
    requiredMana = 7 - (airSkillLevel > 6 ? 6 : airSkillLevel);
    if (requiredMana < 1) requiredMana = 1;
    expectedKineticEnergy = 3 * baseKineticEnergy / requiredMana;
    if (expectedKineticEnergy < 2) expectedKineticEnergy = 2;

    ASSERT_EQ(requiredMana > 3, 1,
              "fixture forces the Air F0407 CurrentMana < RequiredMana branch");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, actionIndex),
              1,
              actionName);
    ASSERT_EQ(state.world.party.champions[0].mana.current, 0,
              "low-mana Air projectile action spends all available mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "low-mana Air projectile action decrements F0405 charges");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "low-mana Air projectile action still mirrors F0406 direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "low-mana Air projectile action creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileCategory,
              PROJECTILE_CATEGORY_MAGICAL,
              "low-mana Air projectile action creates a magical projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              expectedSubtype,
              "low-mana Air projectile action keeps source projectile subtype");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy,
              expectedKineticEnergy,
              "F0407 scales Air kinetic energy by current mana over required mana");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "low-mana Air projectile action still uses F0327 attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].attackTypeCode,
              expectedAttackType,
              "low-mana Air projectile action keeps source attack type");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "low-mana Air projectile action still uses F0327 step energy");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "low-mana Air projectile launch cell follows champion Cell");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "low-mana Air projectile launch direction follows party direction");
}

static void test_air_projectile_low_mana_scales_kinetic_energy_before_f0327(void) {
    run_air_projectile_low_mana_scales_kinetic_case(
        DM1_ACTION_DISPELL, PROJECTILE_SUBTYPE_HARM_NON_MATERIAL, 150,
        COMBAT_ATTACK_MAGIC,
        "low-mana DISPELL still performs F0407/F0327 projectile route");
    run_air_projectile_low_mana_scales_kinetic_case(
        DM1_ACTION_LIGHTNING, PROJECTILE_SUBTYPE_LIGHTNING_BOLT, 180,
        COMBAT_ATTACK_LIGHTNING,
        "low-mana LIGHTNING still performs F0407/F0327 projectile route");
}

static void test_fireball_projectile_create_failure_halves_action_xp(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    int expectedXp;

    seed_state(&state, 100, 50);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIRE].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.projectiles.count = PROJECTILE_LIST_CAPACITY;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_FIREBALL, &route), 1,
              "FIREBALL has a source G0496/G0497 route");
    if (!route.valid) return;
    expectedXp = route.experienceGain >> 1;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FIREBALL),
              0,
              "full projectile list makes F0327 projectile create fail");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "failed FIREBALL still decrements charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "failed FIREBALL still mirrors F0406 champion direction");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_FIREBALL),
              "failed FIREBALL keeps full source disabled ticks");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              10000 + expectedXp,
              "failed FIREBALL halves G0497 action XP on the action skill");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
                  expectedXp,
              "failed FIREBALL propagates halved action XP to base skill");
}

static void test_spit_projectile_create_failure_halves_action_xp(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    int expectedXp;

    seed_state(&state, 100, 61);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIRE].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.projectiles.count = PROJECTILE_LIST_CAPACITY;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_SPIT, &route), 1,
              "SPIT has a source G0496/G0497 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_FIRE,
              "SPIT failure route uses G0496 Fire skill");
    if (!route.valid) return;
    expectedXp = route.experienceGain >> 1;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_SPIT),
              0,
              "full projectile list makes SPIT F0327 projectile create fail");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 7,
              "failed SPIT still spends G0496 mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "failed SPIT still decrements charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "failed SPIT still mirrors F0406 champion direction");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_SPIT),
              "failed SPIT keeps full source disabled ticks");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              10000 + expectedXp,
              "failed SPIT halves G0497 action XP on the action skill");
    if (route.baseSkillIndex != route.skillIndex) {
        ASSERT_EQ(state.world.lifecycle.champions[0]
                      .skills20[route.baseSkillIndex].experience,
                  expectedXp,
                  "failed SPIT propagates halved action XP to base skill");
    }
}

static void run_air_projectile_create_failure_halves_action_xp_case(
    int actionIndex,
    const char* actionName)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    int expectedXp;

    seed_state(&state, 100, 54);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_AIR].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.projectiles.count = PROJECTILE_LIST_CAPACITY;

    ASSERT_EQ(dm1_v1_action_xp_route(actionIndex, &route), 1,
              actionName);
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_AIR,
              "Air projectile failure route uses G0496 Air skill");
    if (!route.valid) return;
    expectedXp = route.experienceGain >> 1;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, actionIndex),
              0,
              "full projectile list makes Air F0327 projectile create fail");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 7,
              "failed Air projectile action still spends G0496 mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "failed Air projectile action still decrements F0405 charges");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "failed Air projectile action still mirrors F0406 direction");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test((unsigned char)actionIndex),
              "failed Air projectile action keeps full source disabled ticks");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              10000 + expectedXp,
              "failed Air projectile action halves G0497 XP on action skill");
    if (route.baseSkillIndex != route.skillIndex) {
        ASSERT_EQ(state.world.lifecycle.champions[0]
                      .skills20[route.baseSkillIndex].experience,
                  expectedXp,
                  "failed Air projectile action propagates halved XP to base skill");
    }
}

static void test_air_projectile_create_failure_halves_action_xp(void) {
    run_air_projectile_create_failure_halves_action_xp_case(
        DM1_ACTION_DISPELL,
        "DISPELL has a source G0496/G0497 route");
    run_air_projectile_create_failure_halves_action_xp_case(
        DM1_ACTION_LIGHTNING,
        "LIGHTNING has a source G0496/G0497 route");
}

static void test_invoke_action_uses_f0327_and_decrements_charges(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    struct RngState_Compat expectedRng;
    int invokeSkillLevel;
    int expectedManaCost;
    int expectedEnergy;
    int expectedRoll;
    int expectedSubtype;

    seed_state(&state, 100, 51);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 7u);
    (void)F0730_COMBAT_RngInit_Compat(&expectedRng, 7u);
    /* ReDMCSB: MENU.C F0407 lines 1480-1482 draws RANDOM(128)+100 before
     * the RANDOM(6) projectile family switch. */
    expectedEnergy = F0732_COMBAT_RngRandom_Compat(&expectedRng, 128) + 100;
    expectedRoll = F0732_COMBAT_RngRandom_Compat(&expectedRng, 6);
    switch (expectedRoll) {
        case 0:
            expectedSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
            break;
        case 1:
            expectedSubtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
            break;
        case 2:
            expectedSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
            break;
        default:
            expectedSubtype = PROJECTILE_SUBTYPE_FIREBALL;
            break;
    }
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_INVOKE, &route), 1,
              "INVOKE has a source G0496/G0497 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_WIZARD,
              "INVOKE required mana uses G0496 Wizard skill");
    invokeSkillLevel = M11_GameView_GetSkillLevel(&state, 0,
                                                  route.skillIndex);
    if (invokeSkillLevel < 0) invokeSkillLevel = 0;
    expectedManaCost = 7 - (invokeSkillLevel > 6 ? 6 : invokeSkillLevel);
    if (expectedManaCost < 1) expectedManaCost = 1;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_INVOKE),
              1,
              "INVOKE action performs F0407 randomized projectile route");
    ASSERT_EQ(state.world.party.champions[0].mana.current,
              9 - expectedManaCost,
              "INVOKE spends 7 - min(6, G0496 Wizard skill) mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "INVOKE decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "INVOKE mirrors F0406 champion direction to party direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "INVOKE creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy,
              expectedEnergy,
              "INVOKE draws source RNG energy before projectile family");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileSubtype,
              expectedSubtype,
              "INVOKE draws source RNG family after kinetic energy");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy >= 100, 1,
              "INVOKE kinetic energy has source lower bound RANDOM(128)+100");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy <= 227, 1,
              "INVOKE kinetic energy has source upper bound RANDOM(128)+100");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "INVOKE uses F0327 fixed projectile attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "INVOKE uses F0327 step energy from maximum mana");
    ASSERT_EQ(state.audioState.lastMarker, M11_AUDIO_MARKER_NONE,
              "INVOKE has no ReDMCSB action-time audio marker");
    ASSERT_EQ(state.audioState.lastSoundIndex, -1,
              "INVOKE has no ReDMCSB action-time source sound");
}

static void test_invoke_low_mana_scales_random_kinetic_before_f0327(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    struct RngState_Compat expectedRng;
    int wizardSkillLevel;
    int requiredMana;
    int expectedFullEnergy;
    int expectedScaledEnergy;
    int scaledMinimum;
    int scaledMaximum;
    int subtype;

    seed_state(&state, 100, 57);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 3;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 7u);
    (void)F0730_COMBAT_RngInit_Compat(&expectedRng, 7u);
    expectedFullEnergy =
        F0732_COMBAT_RngRandom_Compat(&expectedRng, 128) + 100;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_INVOKE, &route), 1,
              "INVOKE has a source G0496/G0497 route");
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_WIZARD,
              "INVOKE low-mana route uses G0496 Wizard skill");
    wizardSkillLevel = M11_GameView_GetSkillLevel(&state, 0,
                                                  route.skillIndex);
    if (wizardSkillLevel < 0) wizardSkillLevel = 0;
    requiredMana = 7 - (wizardSkillLevel > 6 ? 6 : wizardSkillLevel);
    if (requiredMana < 1) requiredMana = 1;
    scaledMinimum = 3 * 100 / requiredMana;
    if (scaledMinimum < 2) scaledMinimum = 2;
    scaledMaximum = 3 * 227 / requiredMana;
    if (scaledMaximum < 2) scaledMaximum = 2;
    expectedScaledEnergy = 3 * expectedFullEnergy / requiredMana;
    if (expectedScaledEnergy < 2) expectedScaledEnergy = 2;

    ASSERT_EQ(requiredMana > 3, 1,
              "fixture forces the INVOKE CurrentMana < RequiredMana branch");
    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_INVOKE),
              1,
              "low-mana INVOKE still performs F0407/F0327 projectile route");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 0,
              "low-mana INVOKE spends all available mana");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "low-mana INVOKE decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "low-mana INVOKE still mirrors F0406 direction");
    ASSERT_EQ(state.world.projectiles.count, 1,
              "low-mana INVOKE creates one projectile");
    ASSERT_EQ(state.world.projectiles.entries[0].projectileCategory,
              PROJECTILE_CATEGORY_MAGICAL,
              "low-mana INVOKE creates a magical projectile");
    subtype = state.world.projectiles.entries[0].projectileSubtype;
    ASSERT_EQ((subtype == PROJECTILE_SUBTYPE_POISON_BOLT) ||
                  (subtype == PROJECTILE_SUBTYPE_POISON_CLOUD) ||
                  (subtype == PROJECTILE_SUBTYPE_HARM_NON_MATERIAL) ||
                  (subtype == PROJECTILE_SUBTYPE_FIREBALL),
              1,
              "low-mana INVOKE keeps the source randomized projectile family");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy >= scaledMinimum,
              1,
              "low-mana INVOKE kinetic energy stays above scaled source minimum");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy <= scaledMaximum,
              1,
              "low-mana INVOKE kinetic energy stays below scaled source maximum");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy,
              expectedScaledEnergy,
              "low-mana INVOKE scales the source RNG energy after drawing it");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy < expectedFullEnergy,
              1,
              "low-mana INVOKE does not keep full RANDOM(128)+100 kinetic energy");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 90,
              "low-mana INVOKE still uses F0327 attack 90");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 2,
              "low-mana INVOKE still uses F0327 step energy");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "low-mana INVOKE launch cell follows champion Cell");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "low-mana INVOKE launch direction follows party direction");
}

static void test_invoke_projectile_create_failure_halves_action_xp(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    int expectedXp;

    seed_state(&state, 100, 53);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    weapons[0].type = 1;
    weapons[0].chargeCount = 2;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 64;
    state.world.party.direction = 1;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 10000;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.projectiles.count = PROJECTILE_LIST_CAPACITY;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 7u);

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_INVOKE, &route), 1,
              "INVOKE has a source G0496/G0497 route");
    if (!route.valid) return;
    expectedXp = route.experienceGain >> 1;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_INVOKE),
              0,
              "full projectile list makes INVOKE F0327 create fail");
    ASSERT_EQ(weapons[0].chargeCount, 1,
              "failed INVOKE still decrements charges through F0405");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 8,
              "failed INVOKE still spends G0496 Wizard mana through F0327");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "failed INVOKE still mirrors F0406 champion direction");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_INVOKE),
              "failed INVOKE keeps full source disabled ticks");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              10000 + expectedXp,
              "failed INVOKE halves G0497 action XP on the action skill");
    if (route.baseSkillIndex != route.skillIndex) {
        ASSERT_EQ(state.world.lifecycle.champions[0]
                      .skills20[route.baseSkillIndex].experience,
                  expectedXp,
                  "failed INVOKE propagates halved action XP to base skill");
    }
}

static void test_cast_potion_spell_mutates_empty_flask(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonPotion_Compat potions[1];
    unsigned char rawPotionData[4];
    int sawSpellEffect;
    int i;

    seed_state(&state, 100, 41);
    memset(&things, 0, sizeof(things));
    memset(potions, 0, sizeof(potions));
    memset(rawPotionData, 0, sizeof(rawPotionData));

    potions[0].next = THING_ENDOFLIST;
    potions[0].type = 20; /* C20 empty flask / C195 icon. */
    potions[0].power = 3;
    rawPotionData[0] = 0xFEu;
    rawPotionData[1] = 0xFFu;
    rawPotionData[2] = 3u;
    rawPotionData[3] = 20u;
    things.loaded = 1;
    things.potions = potions;
    things.potionCount = 1;
    things.rawThingData[THING_TYPE_POTION] = rawPotionData;
    things.thingCounts[THING_TYPE_POTION] = 1;
    state.world.things = &things;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_POTION, 0);
    state.world.party.champions[0].mana.current = 100;
    state.world.party.champions[0].mana.maximum = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_HEAL].experience = 10000;

    ASSERT_EQ(M11_GameView_OpenSpellPanel(&state), 1,
              "potion spell opens spell panel");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1,
              "potion spell enters Lo power rune");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1,
              "potion spell enters Ya potion rune");
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1,
              "potion spell casts with empty flask in hand");

    ASSERT_EQ(potions[0].type, 11,
              "potion spell mutates empty flask to Ya stamina potion");
    ASSERT_EQ(potions[0].power >= 40 && potions[0].power <= 55, 1,
              "potion spell writes Lo power range RANDOM(16)+40");
    ASSERT_EQ(rawPotionData[2], potions[0].power,
              "potion spell updates raw potion power byte");
    ASSERT_EQ(rawPotionData[3], 11,
              "potion spell updates raw potion type byte");
    sawSpellEffect = 0;
    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_SPELL_EFFECT &&
            state.lastTickResult.emissions[i].payload[1] ==
                C1_SPELL_KIND_POTION_COMPAT &&
            state.lastTickResult.emissions[i].payload[2] == 11) {
            sawSpellEffect = 1;
        }
    }
    ASSERT_EQ(sawSpellEffect, 1,
              "potion spell emits committed potion spell effect");
}

static void test_cast_zokathra_spell_materializes_ready_hand_junk(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned char rawJunkData[4];
    int sawSpellEffect;
    int i;

    seed_state(&state, 100, 43);
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    memset(rawJunkData, 0, sizeof(rawJunkData));

    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state.world.party.champions[0].inventory[i] = THING_NONE;
    }
    junks[0].next = THING_NONE;
    rawJunkData[0] = 0xFFu;
    rawJunkData[1] = 0xFFu;
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    things.rawThingData[THING_TYPE_JUNK] = rawJunkData;
    things.thingCounts[THING_TYPE_JUNK] = 1;
    state.world.things = &things;
    state.world.party.champions[0].mana.current = 100;
    state.world.party.champions[0].mana.maximum = 100;
    state.world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 100;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_WIZARD].experience = 10000;

    ASSERT_EQ(M11_GameView_OpenSpellPanel(&state), 1,
              "Zokathra opens spell panel");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1,
              "Zokathra enters Lo power rune");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 5), 1,
              "Zokathra enters Zo element rune");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 2), 1,
              "Zokathra enters Kath form rune");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 4), 1,
              "Zokathra enters Ra class rune");
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1,
              "Zokathra spell casts");

    ASSERT_EQ(junks[0].type, 51,
              "Zokathra materializes source C51 junk type");
    ASSERT_EQ(junks[0].next, THING_ENDOFLIST,
              "Zokathra materialized junk is unlinked in hand");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_HAND_LEFT],
              make_thing(THING_TYPE_JUNK, 0),
              "Zokathra prioritizes ready hand");
    ASSERT_EQ(rawJunkData[0], 0xFE,
              "Zokathra raw junk next low byte is end-of-list");
    ASSERT_EQ(rawJunkData[1], 0xFF,
              "Zokathra raw junk next high byte is end-of-list");
    ASSERT_EQ(rawJunkData[2], 51,
              "Zokathra raw junk type byte is C51");
    sawSpellEffect = 0;
    for (i = 0; i < state.lastTickResult.emissionCount; ++i) {
        if (state.lastTickResult.emissions[i].kind == EMIT_SPELL_EFFECT &&
            state.lastTickResult.emissions[i].payload[1] ==
                C3_SPELL_KIND_OTHER_COMPAT &&
            state.lastTickResult.emissions[i].payload[2] ==
                C7_SPELL_TYPE_OTHER_ZOKATHRA_COMPAT) {
            sawSpellEffect = 1;
        }
    }
    ASSERT_EQ(sawSpellEffect, 1,
              "Zokathra emits committed other-spell effect");
}

static void test_spellshield_low_mana_halves_disable_and_quarters_xp(void) {
    M11_GameViewState state;
    DM1_ActionXpRoute route;
    int expectedActionXp;

    seed_state(&state, 100, 11);
    state.world.party.champions[0].mana.current = 2;
    state.world.party.champions[0].mana.maximum = 10;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_SPELLSHIELD, &route), 1,
              "SPELLSHIELD has a source G0496/G0497 route");
    if (!route.valid) return;
    expectedActionXp = route.experienceGain >> 2;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_SPELLSHIELD),
              0,
              "low-mana SPELLSHIELD returns false through F0403/F0407");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 0,
              "low-mana SPELLSHIELD consumes remaining mana through F0403");
    ASSERT_EQ(state.world.magic.spellShieldDefense, 4,
              "low-mana SPELLSHIELD still adds half-tick F0403 defense");
    ASSERT_EQ(state.world.magic.fireShieldDefense, 0,
              "SPELLSHIELD does not modify fire shield defense");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_SPELLSHIELD) >> 1,
              "F0407 halves SPELLSHIELD disabled ticks when F0403 fails");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_SPELLSHIELD,
              "failed SPELLSHIELD still records disabled action index");
    ASSERT_EQ(route.skillIndex, 15,
              "SPELLSHIELD routes source G0496 XP to skill 15");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedActionXp,
              "F0407 quarters SPELLSHIELD G0497 XP when F0403 fails");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedActionXp,
              "F0304 propagates quartered SPELLSHIELD XP to base skill");
}

static void test_spellshield_success_consumes_mana_charges_and_full_xp(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    unsigned short chargedThing;
    uint32_t expiryTick = 0;
    int expiryIndex = -1;
    int i;
    int guard;

    seed_state(&state, 100, 13);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    chargedThing = make_thing(THING_TYPE_WEAPON, 0);
    weapons[0].chargeCount = 3;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chargedThing;
    state.world.party.champions[0].mana.current = 9;
    state.world.party.champions[0].mana.maximum = 10;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_SPELLSHIELD, &route), 1,
              "SPELLSHIELD has a source G0496/G0497 route for success");
    if (!route.valid) return;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_SPELLSHIELD),
              1,
              "full-mana SPELLSHIELD succeeds through F0403/F0407");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 5,
              "successful SPELLSHIELD consumes four mana through F0403");
    ASSERT_EQ(state.world.magic.spellShieldDefense, 8,
              "successful SPELLSHIELD adds 280>>5 F0403 defense");
    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
            state.world.timeline.events[i].aux0 == LIFECYCLE_STATUS_SPELL_SHIELD) {
            expiryIndex = i;
            break;
        }
    }
    ASSERT_EQ(expiryIndex >= 0, 1,
              "successful SPELLSHIELD schedules C77 shield expiry event");
    if (expiryIndex >= 0) {
        expiryTick = state.world.timeline.events[expiryIndex].fireAtTick;
        ASSERT_EQ(state.world.timeline.events[expiryIndex].aux1, 8,
                  "C77 expiry event carries the applied F0403 defense");
        ASSERT_EQ(expiryTick, 13 + 280,
                  "C77 expiry event fires at source GameTime + ticks");
    }
    ASSERT_EQ(weapons[0].chargeCount, 2,
              "successful SPELLSHIELD decrements action-hand charges through F0405");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_SPELLSHIELD),
              "successful SPELLSHIELD keeps full G0491 disabled ticks");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              route.experienceGain,
              "successful SPELLSHIELD awards full G0497 XP through F0304");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              route.experienceGain,
              "successful SPELLSHIELD propagates full G0497 XP to base skill");
    guard = 0;
    while (expiryTick > 0 && state.world.gameTick <= expiryTick && guard++ < 400) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.world.gameTick >= expiryTick, 1,
              "idle ticks reach the scheduled SPELLSHIELD expiry");
    ASSERT_EQ(state.world.magic.spellShieldDefense, 0,
              "C77 timeline dispatch subtracts expired spell shield defense");
}

static void test_fireshield_success_schedules_c78_and_expires(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;
    unsigned short chargedThing;
    uint32_t expiryTick = 0;
    int expiryIndex = -1;
    int i;
    int guard;

    seed_state(&state, 100, 17);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    chargedThing = make_thing(THING_TYPE_WEAPON, 0);
    weapons[0].chargeCount = 4;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chargedThing;
    state.world.party.champions[0].mana.current = 10;
    state.world.party.champions[0].mana.maximum = 10;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_FIRESHIELD, &route), 1,
              "FIRESHIELD has a source G0496/G0497 route for success");
    if (!route.valid) return;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FIRESHIELD),
              1,
              "full-mana FIRESHIELD succeeds through F0403/F0407");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 6,
              "successful FIRESHIELD consumes four mana through F0403");
    ASSERT_EQ(state.world.magic.spellShieldDefense, 0,
              "FIRESHIELD does not modify spell shield defense");
    ASSERT_EQ(state.world.magic.fireShieldDefense, 8,
              "successful FIRESHIELD adds 280>>5 F0403 defense");
    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
            state.world.timeline.events[i].aux0 == LIFECYCLE_STATUS_FIRE_SHIELD) {
            expiryIndex = i;
            break;
        }
    }
    ASSERT_EQ(expiryIndex >= 0, 1,
              "successful FIRESHIELD schedules C78 shield expiry event");
    if (expiryIndex >= 0) {
        expiryTick = state.world.timeline.events[expiryIndex].fireAtTick;
        ASSERT_EQ(state.world.timeline.events[expiryIndex].aux1, 8,
                  "C78 expiry event carries the applied F0403 defense");
        ASSERT_EQ(expiryTick, 17 + 280,
                  "C78 expiry event fires at source GameTime + ticks");
    }
    ASSERT_EQ(weapons[0].chargeCount, 3,
              "successful FIRESHIELD decrements action-hand charges through F0405");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              route.experienceGain,
              "successful FIRESHIELD awards full G0497 XP through F0304");
    guard = 0;
    while (expiryTick > 0 && state.world.gameTick <= expiryTick && guard++ < 400) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.world.gameTick >= expiryTick, 1,
              "idle ticks reach the scheduled FIRESHIELD expiry");
    ASSERT_EQ(state.world.magic.fireShieldDefense, 0,
              "C78 timeline dispatch subtracts expired fire shield defense");
    ASSERT_EQ(state.world.magic.spellShieldDefense, 0,
              "C78 expiry leaves spell shield defense unchanged");
}

static void test_spellshield_high_defense_quarters_new_event_defense(void) {
    M11_GameViewState state;
    uint32_t expiryTick = 0;
    int expiryIndex = -1;
    int i;
    int guard;

    seed_state(&state, 100, 19);
    state.world.party.champions[0].mana.current = 8;
    state.world.party.champions[0].mana.maximum = 10;
    state.world.magic.spellShieldDefense = 51;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_SPELLSHIELD),
              1,
              "high-defense SPELLSHIELD still succeeds through F0403");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 4,
              "high-defense SPELLSHIELD consumes four mana through F0403");
    ASSERT_EQ(state.world.magic.spellShieldDefense, 53,
              "F0403 quarters only the new defense when existing spell shield is >50");
    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind == TIMELINE_EVENT_STATUS_TIMEOUT &&
            state.world.timeline.events[i].aux0 == LIFECYCLE_STATUS_SPELL_SHIELD) {
            expiryIndex = i;
            break;
        }
    }
    ASSERT_EQ(expiryIndex >= 0, 1,
              "high-defense SPELLSHIELD schedules C77 expiry event");
    if (expiryIndex >= 0) {
        expiryTick = state.world.timeline.events[expiryIndex].fireAtTick;
        ASSERT_EQ(state.world.timeline.events[expiryIndex].aux1, 2,
                  "C77 high-defense expiry event carries quartered new defense");
        ASSERT_EQ(expiryTick, 19 + 280,
                  "C77 high-defense expiry event still uses full source ticks");
    }
    guard = 0;
    while (expiryTick > 0 && state.world.gameTick <= expiryTick && guard++ < 400) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.world.magic.spellShieldDefense, 51,
              "C77 expiry subtracts only the quartered high-defense event amount");
}

static void test_shoot_no_ammunition_clears_action_xp_but_keeps_tail(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    unsigned char actions[3];
    DM1_ActionXpRoute route;

    seed_state(&state, 80, 23);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    weapons[0].type = 25; /* Bow: ObjectInfo ActionSet 27 -> SHOOT. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] =
        THING_NONE;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "bow champion opens SHOOT action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "bow action menu resolves source action set");
    ASSERT_EQ(actions[0], DM1_ACTION_SHOOT,
              "bow action set exposes SHOOT in row 0");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_SHOOT, &route), 1,
              "SHOOT has a source G0496/G0497 route");

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, 0), 0,
              "SHOOT without ready-hand ammunition returns F0407 failure");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "SHOOT without ammunition creates no projectile");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_HAND_LEFT],
              THING_NONE,
              "SHOOT failure leaves ready hand empty");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_SHOOT),
              "SHOOT failure keeps the common full disabled-tick tail");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_SHOOT,
              "SHOOT failure records SHOOT as the disabled action");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              0,
              "F0407 no-ammunition SHOOT clears G0497 Shoot XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              0,
              "F0407 no-ammunition SHOOT clears propagated Ninja XP");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after SHOOT failure");
}

static void test_direct_shoot_no_ammunition_clears_action_xp(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[1];
    DM1_ActionXpRoute route;

    seed_state(&state, 80, 24);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    weapons[0].type = 25; /* Bow: ObjectInfo ActionSet 27 -> SHOOT. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_WEAPON, 0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] =
        THING_NONE;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_SHOOT, &route), 1,
              "direct SHOOT fixture has a source G0496/G0497 route");

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_SHOOT),
              0,
              "direct SHOOT without ammunition returns F0407 failure");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 0,
              "direct SHOOT without ammunition creates no projectile");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_SHOOT),
              "direct SHOOT failure keeps the common disabled-tick tail");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_SHOOT,
              "direct SHOOT failure records SHOOT as disabled action");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              0,
              "direct F0407 no-ammunition SHOOT clears G0497 Shoot XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              0,
              "direct F0407 no-ammunition SHOOT clears propagated Ninja XP");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "direct SHOOT failure clears acting champion");
}

static void test_shoot_action_uses_champion_cell_for_f0326_launch(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    unsigned char actions[3];
    unsigned short bowThing;
    unsigned short arrowThing;

    seed_state(&state, 80, 29);
    memset(&things, 0, sizeof(things));
    memset(weapons, 0, sizeof(weapons));
    weapons[0].type = 25; /* Bow: class 20, kinetic 50, shoot attack 50. */
    weapons[1].type = 27; /* Arrow: class 10, kinetic 10. */
    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = 2;
    state.world.things = &things;
    state.world.party.direction = 1;
    state.world.party.champions[0].cell = 2;
    state.world.party.champions[0].direction = 3;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    bowThing = make_thing(THING_TYPE_WEAPON, 0);
    arrowThing = make_thing(THING_TYPE_WEAPON, 1);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        bowThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] =
        arrowThing;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "bow champion opens SHOOT action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "bow action menu resolves source action set");
    ASSERT_EQ(actions[0], DM1_ACTION_SHOOT,
              "bow action set exposes SHOOT in row 0");

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, 0), 1,
              "SHOOT with ready-hand arrow succeeds");
    ASSERT_EQ(M11_GameView_GetProjectileCount(&state), 1,
              "SHOOT creates one projectile");
    ASSERT_EQ(state.world.party.champions[0].direction, 1,
              "SHOOT mirrors F0406 champion direction to party direction");
    ASSERT_EQ(state.world.projectiles.entries[0].cell, 2,
              "SHOOT launch cell follows F0326 champion Cell formula");
    ASSERT_EQ(state.world.projectiles.entries[0].direction, 1,
              "SHOOT keeps projectile direction at party direction");
    ASSERT_EQ(state.world.projectiles.entries[0].reserved1, arrowThing,
              "SHOOT projectile carries the removed ready-hand arrow Thing");
    ASSERT_EQ(state.world.projectiles.entries[0].kineticEnergy, 60,
              "SHOOT combines launcher and ammunition kinetic energy");
    ASSERT_EQ(state.world.projectiles.entries[0].attack, 102,
              "SHOOT uses F0407 shoot attack with source Shoot skill level");
    ASSERT_EQ(state.world.projectiles.entries[0].launcherStrength, 102,
              "SHOOT carries F0407 attack into F0217 kinetic pass-through strength");
    ASSERT_EQ(state.world.projectiles.entries[0].stepEnergy, 4,
              "SHOOT derives F0326 step energy from bow action class");
    ASSERT_EQ(state.world.party.champions[0]
                  .inventory[CHAMPION_SLOT_HAND_LEFT],
              THING_NONE,
              "SHOOT removes ready-hand arrow after projectile spawn");
}

static void test_climb_down_failure_cancels_disable_but_keeps_xp(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned char actions[3];
    DM1_ActionXpRoute route;
    int expectedXp;

    seed_state(&state, 80, 31);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData; /* front square is corridor, not pit. */
    tiles[0].squareCount = 9;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;

    junks[0].type = 45; /* Rope: ObjectInfo ActionSet 39 -> CLIMB DOWN. */
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_JUNK, 0);

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "rope champion opens CLIMB DOWN action menu");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "rope action menu resolves source action set");
    ASSERT_EQ(actions[0], DM1_ACTION_CLIMB_DOWN,
              "rope action set exposes CLIMB DOWN in row 0");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_CLIMB_DOWN, &route), 1,
              "CLIMB DOWN has a source G0496/G0497 route");
    expectedXp = route.experienceGain * 2;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, 0), 0,
              "CLIMB DOWN in front of corridor returns F0407 failure");
    ASSERT_EQ(state.actionDisabledTicks[0], 0,
              "failed CLIMB DOWN cancels action-disabled ticks");
    ASSERT_EQ(state.actionDisabledIndex[0], 255,
              "failed CLIMB DOWN clears disabled action index");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedXp,
              "BUG0_79 failed CLIMB DOWN still awards G0497 XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedXp,
              "failed CLIMB DOWN XP propagates through F0304");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after CLIMB DOWN failure");
}

static void test_direct_climb_down_failure_cancels_disable_but_keeps_xp(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    DM1_ActionXpRoute route;
    int expectedXp;

    seed_state(&state, 80, 32);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData; /* front square is corridor, not pit. */
    tiles[0].squareCount = 9;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;

    junks[0].type = 45; /* Rope: ObjectInfo ActionSet 39 -> CLIMB DOWN. */
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_JUNK, 0);

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_CLIMB_DOWN, &route), 1,
              "direct CLIMB DOWN fixture has a source G0496/G0497 route");
    expectedXp = route.experienceGain * 2;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_CLIMB_DOWN),
              0,
              "direct CLIMB DOWN in front of corridor returns F0407 failure");
    ASSERT_EQ(state.actionDisabledTicks[0], 0,
              "direct failed CLIMB DOWN cancels action-disabled ticks");
    ASSERT_EQ(state.actionDisabledIndex[0], 255,
              "direct failed CLIMB DOWN clears disabled action index");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedXp,
              "direct BUG0_79 failed CLIMB DOWN still awards G0497 XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedXp,
              "direct failed CLIMB DOWN XP propagates through F0304");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "direct CLIMB DOWN failure clears acting champion");
}

static void test_climb_down_open_pit_moves_party_and_keeps_tail(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char map0[9];
    unsigned char map1[9];
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned char actions[3];
    DM1_ActionXpRoute route;
    int i;
    int expectedXp;

    seed_state(&state, 100, 37);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    for (i = 0; i < 9; ++i) {
        map0[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        map1[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    map0[2 * 3 + 1] = square_for_test(DUNGEON_ELEMENT_PIT, 0x08);

    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    maps[0].level = 0;
    maps[1].width = 3;
    maps[1].height = 3;
    maps[1].level = 1;
    tiles[0].squareData = map0;
    tiles[0].squareCount = 9;
    tiles[1].squareData = map1;
    tiles[1].squareCount = 9;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;

    junks[0].type = 45; /* Rope: ObjectInfo ActionSet 39 -> CLIMB DOWN. */
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_JUNK, 0);

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "rope champion opens CLIMB DOWN action menu for open pit");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "rope action menu resolves source action set for open pit");
    ASSERT_EQ(actions[0], DM1_ACTION_CLIMB_DOWN,
              "rope action set exposes CLIMB DOWN before open pit");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_CLIMB_DOWN, &route), 1,
              "CLIMB DOWN open pit has a source G0496/G0497 route");
    expectedXp = route.experienceGain * 2;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, 0), 1,
              "CLIMB DOWN in front of an open pit returns success");
    ASSERT_EQ(state.world.party.mapIndex, 1,
              "successful CLIMB DOWN resolves pit fall to lower map");
    ASSERT_EQ(state.world.partyMapIndex, 1,
              "successful CLIMB DOWN keeps world partyMapIndex mirror current");
    ASSERT_EQ(state.world.party.mapX, 2,
              "successful CLIMB DOWN first moves party forward onto pit x");
    ASSERT_EQ(state.world.party.mapY, 1,
              "successful CLIMB DOWN preserves pit y after fall");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 80,
              "successful CLIMB DOWN applies source pit-fall damage");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_CLIMB_DOWN),
              "successful CLIMB DOWN keeps full disabled-tick tail");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_CLIMB_DOWN,
              "successful CLIMB DOWN records disabled action index");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedXp,
              "successful CLIMB DOWN awards G0497 XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedXp,
              "successful CLIMB DOWN XP propagates through F0304");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after successful CLIMB DOWN");
}

static void test_climb_down_closed_pit_moves_forward_without_fall(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned char actions[3];
    DM1_ActionXpRoute route;
    int i;
    int expectedXp;

    seed_state(&state, 100, 39);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    for (i = 0; i < 9; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[2 * 3 + 1] = square_for_test(DUNGEON_ELEMENT_PIT, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;

    junks[0].type = 45; /* Rope: ObjectInfo ActionSet 39 -> CLIMB DOWN. */
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_JUNK, 0);

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "rope champion opens CLIMB DOWN action menu before closed pit");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "rope action menu resolves source action set before closed pit");
    ASSERT_EQ(actions[0], DM1_ACTION_CLIMB_DOWN,
              "rope action set exposes CLIMB DOWN before closed pit");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_CLIMB_DOWN, &route), 1,
              "CLIMB DOWN closed pit has a source G0496/G0497 route");
    expectedXp = route.experienceGain * 2;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, 0), 1,
              "CLIMB DOWN in front of a closed pit returns success");
    ASSERT_EQ(state.world.party.mapIndex, 0,
              "closed-pit CLIMB DOWN stays on original map");
    ASSERT_EQ(state.world.party.mapX, 2,
              "closed-pit CLIMB DOWN still moves party forward");
    ASSERT_EQ(state.world.party.mapY, 1,
              "closed-pit CLIMB DOWN preserves y");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 100,
              "closed-pit CLIMB DOWN applies no fall damage");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_CLIMB_DOWN),
              "closed-pit CLIMB DOWN keeps full disabled-tick tail");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_CLIMB_DOWN,
              "closed-pit CLIMB DOWN records disabled action index");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedXp,
              "closed-pit CLIMB DOWN awards G0497 XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedXp,
              "closed-pit CLIMB DOWN XP propagates through F0304");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after closed-pit CLIMB DOWN");
}

static void test_climb_down_group_over_pit_blocks_move_but_keeps_bug79_tail(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    unsigned short squareFirstThings[1];
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junks[1];
    unsigned char actions[3];
    DM1_ActionXpRoute route;
    int i;
    int expectedXp;

    seed_state(&state, 100, 41);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(junks, 0, sizeof(junks));
    for (i = 0; i < 9; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[2 * 3 + 1] = square_for_test(
        DUNGEON_ELEMENT_PIT, DUNGEON_SQUARE_MASK_THING_LIST | 0x08);
    squareFirstThings[0] = make_thing(THING_TYPE_GROUP, 0);

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 1;

    junks[0].type = 45; /* Rope: ObjectInfo ActionSet 39 -> CLIMB DOWN. */
    things.loaded = 1;
    things.junks = junks;
    things.junkCount = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    state.world.things = &things;
    state.world.lifecycle.lastCreatureAttackTime = state.world.gameTick;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        make_thing(THING_TYPE_JUNK, 0);

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "rope champion opens CLIMB DOWN action menu before occupied pit");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "rope action menu resolves source action set before occupied pit");
    ASSERT_EQ(actions[0], DM1_ACTION_CLIMB_DOWN,
              "rope action set exposes CLIMB DOWN before occupied pit");
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_CLIMB_DOWN, &route), 1,
              "CLIMB DOWN occupied pit has a source G0496/G0497 route");
    expectedXp = route.experienceGain * 2;

    ASSERT_EQ(M11_GameView_TriggerActionRow(&state, 0), 0,
              "CLIMB DOWN is blocked by a group levitating over the pit");
    ASSERT_EQ(state.world.party.mapIndex, 0,
              "blocked CLIMB DOWN keeps party on original map");
    ASSERT_EQ(state.world.party.mapX, 1,
              "blocked CLIMB DOWN keeps original x");
    ASSERT_EQ(state.world.party.mapY, 1,
              "blocked CLIMB DOWN keeps original y");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 100,
              "blocked CLIMB DOWN applies no fall damage");
    ASSERT_EQ(state.actionDisabledTicks[0], 0,
              "blocked CLIMB DOWN cancels action-disabled ticks");
    ASSERT_EQ(state.actionDisabledIndex[0], 255,
              "blocked CLIMB DOWN clears disabled action index");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedXp,
              "BUG0_79 occupied-pit CLIMB DOWN still awards G0497 XP");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.baseSkillIndex].experience,
              expectedXp,
              "occupied-pit CLIMB DOWN XP propagates through F0304");
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "F0391 clears acting champion after occupied-pit CLIMB DOWN");
}

static void test_action_defense_serializes_outside_v1_champion_blob(void) {
    struct ChampionState_Compat champion;
    struct ChampionState_Compat restored;
    unsigned char fullBuf[CHAMPION_SERIALIZED_SIZE];
    unsigned char oldPortraitBuf[CHAMPION_SERIALIZED_V2_PORTRAIT_SIZE];
    int rc;

    F0600_CHAMPION_InitEmpty_Compat(&champion);
    champion.present = 1;
    champion.portraitIndex = 3;
    champion.actionIndex = 1;
    champion.actionDefense = 36;
    champion.portraitBitmapValid = 1;
    champion.portraitBitmap[0] = 0xA5;

    rc = F0602_CHAMPION_Serialize_Compat(&champion, fullBuf,
                                         (int)sizeof(fullBuf));
    ASSERT_EQ(rc, CHAMPION_SERIALIZED_SIZE,
              "current champion serialization uses v3 action-defense size");
    rc = F0603_CHAMPION_Deserialize_Compat(&restored, fullBuf,
                                           (int)sizeof(fullBuf));
    ASSERT_EQ(rc, CHAMPION_SERIALIZED_SIZE,
              "current champion deserialization consumes v3 size");
    ASSERT_EQ(restored.actionIndex, 1,
              "current champion deserialization preserves action index");
    ASSERT_EQ(restored.actionDefense, 36,
              "current champion deserialization preserves action defense");
    ASSERT_EQ(restored.portraitBitmapValid, 1,
              "current champion deserialization preserves portrait flag");
    ASSERT_EQ(restored.portraitBitmap[0], 0xA5,
              "current champion deserialization preserves portrait bytes");

    memcpy(oldPortraitBuf, fullBuf, sizeof(oldPortraitBuf));
    oldPortraitBuf[CHAMPION_SERIALIZED_V1_SIZE +
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT + 1] = 0;
    oldPortraitBuf[CHAMPION_SERIALIZED_V1_SIZE +
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT + 2] = 0;
    oldPortraitBuf[CHAMPION_SERIALIZED_V1_SIZE +
                   CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT + 3] = 0;
    rc = F0603_CHAMPION_Deserialize_Compat(&restored, oldPortraitBuf,
                                           (int)sizeof(oldPortraitBuf));
    ASSERT_EQ(rc, CHAMPION_SERIALIZED_V2_PORTRAIT_SIZE,
              "old reserved-zero portrait-v2 champion blob remains readable");
    ASSERT_EQ(restored.actionIndex, 255,
              "old reserved-zero portrait-v2 blob defaults action index to none");
    ASSERT_EQ(restored.actionDefense, 0,
              "old reserved-zero portrait-v2 blob defaults action defense to zero");
    ASSERT_EQ(restored.portraitBitmapValid, 1,
              "portrait-v2 champion blob still preserves portrait flag");
    ASSERT_EQ(restored.portraitBitmap[0], 0xA5,
              "portrait-v2 champion blob still preserves portrait bytes");
}

static void test_action_stamina_underflow_clamps_and_damages(void) {
    M11_GameViewState state;
    seed_state(&state, 2, 1);

    (void)M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 1); /* BLOCK */

    ASSERT_EQ(state.world.party.champions[0].stamina.current, 0,
              "underflow action stamina clamps to zero");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 99,
              "underflow action stamina applies F0325-style HP damage");
}

static void test_fluxcage_schedules_f0224_remove_event(void) {
    M11_GameViewState state;
    int removeIndex = -1;
    int slot = -1;
    int guard;
    int i;

    seed_state(&state, 100, 41);
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 2;
    state.world.party.mapY = 2;
    state.world.party.direction = 0; /* north: target is (2,1). */
    state.world.party.champions[0].direction = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FLUXCAGE),
              1,
              "FLUXCAGE creates the F0224 fluxcage explosion");
    ASSERT_EQ(state.world.party.champions[0].direction, 0,
              "FLUXCAGE mirrors F0406 champion direction to party direction");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_FLUXCAGE),
              "FLUXCAGE runtime action-disable tail reads source G0491");
    ASSERT_EQ(state.world.explosions.count, 1,
              "FLUXCAGE leaves one live explosion instance");

    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind ==
            TIMELINE_EVENT_REMOVE_FLUXCAGE) {
            removeIndex = i;
            break;
        }
    }
    ASSERT_EQ(removeIndex >= 0, 1,
              "FLUXCAGE schedules C24 remove-fluxcage event");
    if (removeIndex < 0) return;

    slot = state.world.timeline.events[removeIndex].aux0;
    ASSERT_EQ((int)state.world.timeline.events[removeIndex].fireAtTick, 141,
              "FLUXCAGE remove event fires at GameTime + 100");
    ASSERT_EQ(state.world.timeline.events[removeIndex].mapIndex, 0,
              "FLUXCAGE remove event stores map index");
    ASSERT_EQ(state.world.timeline.events[removeIndex].mapX, 2,
              "FLUXCAGE remove event stores target x");
    ASSERT_EQ(state.world.timeline.events[removeIndex].mapY, 1,
              "FLUXCAGE remove event stores target y");
    ASSERT_EQ(slot >= 0, 1,
              "FLUXCAGE remove event stores explosion slot");
    ASSERT_EQ(state.world.explosions.entries[slot].explosionType,
              C050_EXPLOSION_FLUXCAGE,
              "FLUXCAGE explosion uses source explosion type");
    ASSERT_EQ(state.world.explosions.entries[slot].attack, 255,
              "FLUXCAGE explosion uses source attack strength");
    ASSERT_EQ(state.world.explosions.entries[slot].mapX, 2,
              "FLUXCAGE explosion stores target x");
    ASSERT_EQ(state.world.explosions.entries[slot].mapY, 1,
              "FLUXCAGE explosion stores target y");

    guard = 0;
    while (state.world.gameTick <= 141U && guard++ < 128) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.world.explosions.count, 0,
              "C24 remove-fluxcage event despawns the fluxcage");
}

static void test_fluxcage_uses_pref0406_champion_target_square(void) {
    M11_GameViewState state;
    int removeIndex = -1;
    int slot = -1;
    int i;

    seed_state(&state, 100, 41);
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 2;
    state.world.party.mapY = 2;
    state.world.party.direction = 0; /* party north. */
    state.world.party.champions[0].direction = 3; /* champion west. */

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FLUXCAGE),
              1,
              "FLUXCAGE performs with a pre-F0406 champion-facing target");
    ASSERT_EQ(state.world.party.champions[0].direction, 0,
              "FLUXCAGE still mirrors F0406 champion direction afterward");
    ASSERT_EQ(state.world.explosions.count, 1,
              "pre-F0406 FLUXCAGE creates one live explosion");

    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind ==
            TIMELINE_EVENT_REMOVE_FLUXCAGE) {
            removeIndex = i;
            break;
        }
    }
    ASSERT_EQ(removeIndex >= 0, 1,
              "pre-F0406 FLUXCAGE schedules the remove event");
    if (removeIndex < 0) return;

    slot = state.world.timeline.events[removeIndex].aux0;
    ASSERT_EQ(state.world.timeline.events[removeIndex].mapX, 1,
              "pre-F0406 FLUXCAGE remove event uses champion-facing x");
    ASSERT_EQ(state.world.timeline.events[removeIndex].mapY, 2,
              "pre-F0406 FLUXCAGE remove event uses champion-facing y");
    ASSERT_EQ(slot >= 0, 1,
              "pre-F0406 FLUXCAGE remove event stores explosion slot");
    ASSERT_EQ(state.world.explosions.entries[slot].mapX, 1,
              "pre-F0406 FLUXCAGE explosion uses champion-facing x");
    ASSERT_EQ(state.world.explosions.entries[slot].mapY, 2,
              "pre-F0406 FLUXCAGE explosion uses champion-facing y");
}

static void test_fluxcage_wall_target_keeps_f0407_tail_without_cage(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[9];
    DM1_ActionXpRoute route;
    int removeIndex = -1;
    int expectedXp;
    int i;

    seed_state(&state, 100, 43);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 9; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    squareData[(1 * 3) + 0] = square_for_test(DUNGEON_ELEMENT_WALL, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 1;
    state.world.party.direction = 0; /* north: wall target at (1,0). */
    state.world.party.champions[0].direction = 0;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_FLUXCAGE, &route), 1,
              "FLUXCAGE has a source G0496/G0497 route");
    if (!route.valid) return;
    expectedXp = route.experienceGain;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FLUXCAGE),
              1,
              "wall-target FLUXCAGE keeps F0407 action-performed state");
    ASSERT_EQ(state.world.party.champions[0].direction, 0,
              "wall-target FLUXCAGE still mirrors F0406 champion direction");
    ASSERT_EQ(state.world.explosions.count, 0,
              "wall-target F0224 creates no fluxcage explosion");
    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind ==
            TIMELINE_EVENT_REMOVE_FLUXCAGE) {
            removeIndex = i;
            break;
        }
    }
    ASSERT_EQ(removeIndex, -1,
              "wall-target F0224 schedules no remove-fluxcage event");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_FLUXCAGE),
              "wall-target FLUXCAGE keeps the common G0491 disabled tail");
    ASSERT_EQ(state.actionDisabledIndex[0], DM1_ACTION_FLUXCAGE,
              "wall-target FLUXCAGE records the disabled action index");
    ASSERT_EQ(state.world.lifecycle.champions[0]
                  .skills20[route.skillIndex].experience,
              expectedXp,
              "wall-target FLUXCAGE still awards full G0497 action XP");
    if (route.baseSkillIndex != route.skillIndex) {
        ASSERT_EQ(state.world.lifecycle.champions[0]
                      .skills20[route.baseSkillIndex].experience,
                  expectedXp,
                  "wall-target FLUXCAGE propagates full XP to the base skill");
    }
    ASSERT_EQ(M11_GameView_GetActingChampionOrdinal(&state), 0,
              "wall-target FLUXCAGE clears acting champion through F0391");
}

static void test_fluxcage_third_cage_schedules_lord_chaos_danger(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[25];
    unsigned short squareFirstThings[25];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    int dangerIndex = -1;
    int removeIndex = -1;
    int i;

    seed_state(&state, 100, 41);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    for (i = 0; i < 25; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 5;
    maps[0].height = 5;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 25;
    squareFirstThings[(2 * 5) + 1] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 2;
    state.world.party.mapY = 3;
    state.world.party.direction = 0; /* north: new cage at (2,2). */
    state.world.party.champions[0].direction = 0;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    groups[0].count = 0;
    groups[0].health[0] = 10000;
    groups[0].cells = 0xFF;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 25;
    things.groups = groups;
    things.groupCount = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    state.world.creatureAI[0].lastSeenPartyTick = 41;
    state.world.creatureAI[0].reserved0 = 0;

    state.world.explosions.count = 2;
    state.world.explosions.entries[0].reserved0 = 1;
    state.world.explosions.entries[0].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[0].mapIndex = 0;
    state.world.explosions.entries[0].mapX = 1;
    state.world.explosions.entries[0].mapY = 1;
    state.world.explosions.entries[1].reserved0 = 1;
    state.world.explosions.entries[1].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[1].mapIndex = 0;
    state.world.explosions.entries[1].mapX = 2;
    state.world.explosions.entries[1].mapY = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FLUXCAGE),
              1,
              "third FLUXCAGE near Lord Chaos performs F0224 reaction check");

    for (i = 0; i < state.world.timeline.count; ++i) {
        if (state.world.timeline.events[i].kind ==
            TIMELINE_EVENT_CREATURE_REACTION) {
            dangerIndex = i;
        }
        if (state.world.timeline.events[i].kind ==
            TIMELINE_EVENT_REMOVE_FLUXCAGE) {
            removeIndex = i;
        }
    }
    ASSERT_EQ(dangerIndex >= 0, 1,
              "third FLUXCAGE schedules C29 Lord Chaos danger reaction");
    ASSERT_EQ(removeIndex >= 0, 1,
              "third FLUXCAGE still schedules C24 remove event");
    if (dangerIndex < 0) return;
    ASSERT_EQ((int)state.world.timeline.events[dangerIndex].fireAtTick, 44,
              "C29 danger reaction uses F0209 CM3 movement delay");
    ASSERT_EQ(state.world.timeline.events[dangerIndex].mapIndex, 0,
              "C29 danger reaction stores Lord Chaos map");
    ASSERT_EQ(state.world.timeline.events[dangerIndex].mapX, 2,
              "C29 danger reaction stores Lord Chaos x");
    ASSERT_EQ(state.world.timeline.events[dangerIndex].mapY, 1,
              "C29 danger reaction stores Lord Chaos y");
    ASSERT_EQ(state.world.timeline.events[dangerIndex].aux0, 0,
              "C29 danger reaction stores Lord Chaos group index");
    ASSERT_EQ(state.world.timeline.events[dangerIndex].aux1,
              DM1_CREATURE_LORD_CHAOS_ID,
              "C29 danger reaction stores Lord Chaos creature type");
    ASSERT_EQ(state.world.timeline.events[dangerIndex].aux2,
              DM1_EVENT_REACTION_DANGER_ON_SQUARE,
              "C29 danger reaction stores danger-on-square event type");
}

static void test_fuse_incomplete_fluxcage_moves_lord_chaos_escape(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[25];
    unsigned short squareFirstThings[25];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned char rawGroupData[16];
    int i;

    seed_state(&state, 100, 41);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(rawGroupData, 0, sizeof(rawGroupData));
    for (i = 0; i < 25; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 5;
    maps[0].height = 5;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 25;
    squareFirstThings[(2 * 5) + 1] = make_thing(THING_TYPE_GROUP, 0);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 2;
    state.world.party.mapY = 2;
    state.world.party.direction = 0; /* north: target is (2,1). */
    state.world.party.champions[0].direction = 3;

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    groups[0].count = 0;
    groups[0].health[0] = 10000;
    groups[0].cells = 0xFF;
    rawGroupData[0] = 0xFEu;
    rawGroupData[1] = 0xFFu;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 25;
    things.groups = groups;
    things.groupCount = 1;
    things.rawThingData[THING_TYPE_GROUP] = rawGroupData;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    state.world.things = &things;
    state.world.creatureAICount = 1;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    state.world.creatureAI[0].reserved0 = 0;

    state.world.explosions.count = 3;
    state.world.explosions.entries[0].reserved0 = 1;
    state.world.explosions.entries[0].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[0].mapIndex = 0;
    state.world.explosions.entries[0].mapX = 1;
    state.world.explosions.entries[0].mapY = 1;
    state.world.explosions.entries[1].reserved0 = 1;
    state.world.explosions.entries[1].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[1].mapIndex = 0;
    state.world.explosions.entries[1].mapX = 2;
    state.world.explosions.entries[1].mapY = 0;
    state.world.explosions.entries[2].reserved0 = 1;
    state.world.explosions.entries[2].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[2].mapIndex = 0;
    state.world.explosions.entries[2].mapX = 2;
    state.world.explosions.entries[2].mapY = 2;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FUSE),
              1,
              "FUSE with incomplete Fluxcage lets Lord Chaos escape");
    ASSERT_EQ(state.world.party.champions[0].direction, 0,
              "FUSE mirrors F0406 champion direction to party direction");
    ASSERT_EQ(squareFirstThings[(2 * 5) + 1], THING_ENDOFLIST,
              "FUSE escape unlinks Lord Chaos from target square");
    ASSERT_EQ(squareFirstThings[(3 * 5) + 1],
              make_thing(THING_TYPE_GROUP, 0),
              "FUSE escape relinks Lord Chaos to the open east gap");
    ASSERT_EQ(groups[0].next, THING_ENDOFLIST,
              "FUSE escape keeps moved group as single square-list entry");
    ASSERT_EQ(rawGroupData[0], 0xFEu,
              "FUSE escape raw group next low byte stays end-of-list");
    ASSERT_EQ(rawGroupData[1], 0xFFu,
              "FUSE escape raw group next high byte stays end-of-list");
    ASSERT_EQ(state.world.creatureAI[0].groupMapX, 3,
              "FUSE escape updates active Lord Chaos AI x");
    ASSERT_EQ(state.world.creatureAI[0].groupMapY, 1,
              "FUSE escape updates active Lord Chaos AI y");
    ASSERT_EQ(state.world.gameWon, 0,
              "FUSE escape does not trigger the completed fuse ending");
}

static void test_fuse_without_lord_chaos_keeps_action_performed_tail(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[25];
    unsigned short squareFirstThings[25];
    struct DungeonThings_Compat things;
    int i;

    seed_state(&state, 100, 41);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    for (i = 0; i < 25; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 5;
    maps[0].height = 5;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 25;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 2;
    state.world.party.mapY = 2;
    state.world.party.direction = 0; /* north: target is (2,1). */
    state.world.party.champions[0].direction = 3;
    state.world.party.champions[0].food = 0;
    state.world.party.champions[0].water = 0;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 25;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FUSE),
              1,
              "FUSE without Lord Chaos keeps F0407 ActionPerformed true");
    ASSERT_EQ(state.world.party.champions[0].direction, 0,
              "FUSE without Lord Chaos mirrors F0406 champion direction");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_FUSE),
              "FUSE without Lord Chaos keeps the common G0491 tail");
    ASSERT_EQ(state.world.gameWon, 0,
              "FUSE without Lord Chaos does not trigger the ending");
}

static void test_fuse_out_of_bounds_keeps_action_performed_without_explosion(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[25];
    int i;

    seed_state(&state, 100, 43);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    for (i = 0; i < 25; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 5;
    maps[0].height = 5;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 25;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 0;
    state.world.party.mapY = 0;
    state.world.party.direction = 3; /* west: target is (-1,0). */
    state.world.party.champions[0].direction = 1;
    state.world.party.champions[0].food = 0;
    state.world.party.champions[0].water = 0;

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FUSE),
              1,
              "out-of-bounds FUSE keeps F0407 ActionPerformed true");
    ASSERT_EQ(state.world.party.champions[0].direction, 3,
              "out-of-bounds FUSE still mirrors F0406 direction");
    ASSERT_EQ(state.world.explosions.count, 0,
              "out-of-bounds FUSE skips the guarded F0225 explosion");
    ASSERT_EQ(state.actionDisabledTicks[0],
              action_disabled_ticks_for_test(DM1_ACTION_FUSE),
              "out-of-bounds FUSE keeps the common G0491 tail");
    ASSERT_EQ(state.world.gameWon, 0,
              "out-of-bounds FUSE does not trigger the ending");
}

static void test_fuse_complete_fluxcage_sets_m11_game_won_gate(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[25];
    unsigned short squareFirstThings[25];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[2];
    struct DungeonTextString_Compat textStrings[2];
    unsigned char rawTextStringData[8];
    unsigned short textData[6];
    int partyFluxcageCount = -1;
    int chaosFluxcageCount = -1;
    int fireballAttackSeen[6] = {0, 0, 0, 0, 0, 0};
    int harmAttackSeen[6] = {0, 0, 0, 0, 0, 0};
    int fireballBurstCount = 0;
    int harmBurstCount = 0;
    int artifactMapX = -1;
    int artifactMapY = -1;
    int artifactElement = -1;
    int artifactProjectiles = -1;
    int artifactExplosions = -1;
    int artifactProjectileGfx = -1;
    int artifactExplosionType = -1;
    int i;
    uint32_t gameTickAtWin;

    seed_state(&state, 100, 41);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0xFF, sizeof(squareFirstThings));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(textStrings, 0, sizeof(textStrings));
    memset(rawTextStringData, 0xFF, sizeof(rawTextStringData));
    memset(textData, 0, sizeof(textData));
    for (i = 0; i < 25; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        squareFirstThings[i] = THING_ENDOFLIST;
    }

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 5;
    maps[0].height = 5;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 25;
    squareFirstThings[0] = make_thing(THING_TYPE_TEXTSTRING, 1);
    squareFirstThings[(2 * 5) + 1] = make_thing(THING_TYPE_GROUP, 0);
    squareFirstThings[(4 * 5) + 4] = make_thing(THING_TYPE_GROUP, 1);
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 2;
    state.world.party.mapY = 2;
    state.world.party.direction = 0; /* north: target is (2,1). */

    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    groups[0].count = 0;
    groups[0].health[0] = 10000;
    groups[0].cells = 0x12;
    groups[0].direction = 1;
    groups[1].next = THING_ENDOFLIST;
    groups[1].creatureType = DM1_CREATURE_TYPE_SCREAMER;
    groups[1].count = 0;
    groups[1].health[0] = 37;
    groups[1].cells = 0xFF;
    groups[1].direction = 3;

    textData[0] = pack_text3(0, 5, 8);      /* AFI */
    textData[1] = pack_text3(17, 18, 19);   /* RST */
    textData[2] = pack_text3(31, 31, 31);   /* end */
    textData[3] = pack_text3(1, 18, 4);     /* BSE */
    textData[4] = pack_text3(2, 14, 13);    /* CON */
    textData[5] = pack_text3(3, 31, 31);    /* D + end */
    textStrings[0].next = THING_ENDOFLIST;
    textStrings[0].visible = 0;
    textStrings[0].textDataWordOffset = 0;
    textStrings[1].next = make_thing(THING_TYPE_TEXTSTRING, 0);
    textStrings[1].visible = 0;
    textStrings[1].textDataWordOffset = 3;
    rawTextStringData[0] = 0xFEu;
    rawTextStringData[1] = 0xFFu;
    rawTextStringData[4] = (unsigned char)(make_thing(THING_TYPE_TEXTSTRING, 0) & 0xFFu);
    rawTextStringData[5] = (unsigned char)(make_thing(THING_TYPE_TEXTSTRING, 0) >> 8);

    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 25;
    things.groups = groups;
    things.groupCount = 2;
    things.textStrings = textStrings;
    things.textStringCount = 2;
    things.textData = textData;
    things.textDataWordCount = 6;
    things.rawThingData[THING_TYPE_TEXTSTRING] = rawTextStringData;
    things.thingCounts[THING_TYPE_TEXTSTRING] = 2;
    state.world.things = &things;
    state.world.creatureAICount = 2;
    state.world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    state.world.creatureAI[0].groupMapIndex = 0;
    state.world.creatureAI[0].groupMapX = 2;
    state.world.creatureAI[0].groupMapY = 1;
    state.world.creatureAI[0].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    state.world.creatureAI[0].groupCells = 0x12;
    state.world.creatureAI[0].groupDirection = 1;
    state.world.creatureAI[0].reserved0 = 0;
    state.world.creatureAI[1].stateKind = AI_STATE_WANDER;
    state.world.creatureAI[1].groupMapIndex = 0;
    state.world.creatureAI[1].groupMapX = 4;
    state.world.creatureAI[1].groupMapY = 4;
    state.world.creatureAI[1].creatureType = DM1_CREATURE_TYPE_SCREAMER;
    state.world.creatureAI[1].groupCells = 0xFF;
    state.world.creatureAI[1].groupDirection = 3;
    state.world.creatureAI[1].reserved0 = 1;

    state.world.explosions.count = 5;
    state.world.explosions.entries[0].reserved0 = 1;
    state.world.explosions.entries[0].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[0].mapIndex = 0;
    state.world.explosions.entries[0].mapX = 1;
    state.world.explosions.entries[0].mapY = 1;
    state.world.explosions.entries[1].reserved0 = 1;
    state.world.explosions.entries[1].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[1].mapIndex = 0;
    state.world.explosions.entries[1].mapX = 2;
    state.world.explosions.entries[1].mapY = 0;
    state.world.explosions.entries[2].reserved0 = 1;
    state.world.explosions.entries[2].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[2].mapIndex = 0;
    state.world.explosions.entries[2].mapX = 2;
    state.world.explosions.entries[2].mapY = 2;
    state.world.explosions.entries[3].reserved0 = 1;
    state.world.explosions.entries[3].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[3].mapIndex = 0;
    state.world.explosions.entries[3].mapX = 3;
    state.world.explosions.entries[3].mapY = 1;
    state.world.explosions.entries[4].reserved0 = 1;
    state.world.explosions.entries[4].explosionType = C050_EXPLOSION_FLUXCAGE;
    state.world.explosions.entries[4].mapIndex = 0;
    state.world.explosions.entries[4].mapX = 2;
    state.world.explosions.entries[4].mapY = 1;

    ASSERT_EQ(M11_GameView_ProbeViewportArtifactCounts(
                  &state, 1, -1, &artifactMapX, &artifactMapY,
                  &artifactElement, &artifactProjectiles,
                  &artifactExplosions, &artifactProjectileGfx,
                  &artifactExplosionType),
              1,
              "FUSE complete fixture can sample the left-front fluxcage square");
    ASSERT_EQ(artifactMapX, 1,
              "FUSE complete fixture left-front sample uses fluxcage x");
    ASSERT_EQ(artifactMapY, 1,
              "FUSE complete fixture left-front sample uses fluxcage y");
    ASSERT_EQ(artifactExplosions, 1,
              "FUSE complete fixture sees the live fluxcage before endgame hide gate");
    ASSERT_EQ(artifactExplosionType, C050_EXPLOSION_FLUXCAGE,
              "FUSE complete fixture exposes the fluxcage type before F0446 hide gate");

    ASSERT_EQ(M11_GameView_TriggerNonMeleeActionByIndex(
                  &state, 0, DM1_ACTION_FUSE),
              1,
              "FUSE with complete Fluxcage triggers the fuse ending");
    ASSERT_EQ(groups[0].creatureType, DM1_CREATURE_GREY_LORD_ID,
              "FUSE complete turns Lord Chaos into the Grey Lord");
    ASSERT_EQ(groups[0].health[0], 10000,
              "FUSE complete heals Grey Lord per F0446");
    ASSERT_EQ(groups[0].cells, 0xFF,
              "FUSE complete centers Grey Lord per F0446");
    ASSERT_EQ(groups[0].direction, 2,
              "FUSE complete faces Grey Lord opposite the party");
    ASSERT_EQ(state.world.creatureAI[0].creatureType, DM1_CREATURE_GREY_LORD_ID,
              "FUSE complete updates active AI creature type mirror");
    ASSERT_EQ(state.world.creatureAI[0].groupCells, 0xFF,
              "FUSE complete updates active AI group-cell mirror");
    ASSERT_EQ(state.world.creatureAI[0].groupDirection, 2,
              "FUSE complete updates active AI group-direction mirror");
    ASSERT_EQ(squareFirstThings[(2 * 5) + 1], make_thing(THING_TYPE_GROUP, 0),
              "FUSE complete keeps the Grey Lord group on its square");
    ASSERT_EQ(squareFirstThings[(4 * 5) + 4], THING_ENDOFLIST,
              "FUSE complete deletes non-Grey-Lord groups from the map");
    ASSERT_EQ(state.world.creatureAICount, 1,
              "FUSE complete removes active AI entries for deleted groups");
    ASSERT_EQ(state.world.magic.magicalLightAmount, 200,
              "FUSE complete applies F0446 magical light amount");
    ASSERT_EQ(state.world.magic.fireShieldDefense, 100,
              "FUSE complete applies F0446 fire shield defense");
    ASSERT_EQ(state.world.magic.spellShieldDefense, 100,
              "FUSE complete applies F0446 spell shield defense");
    ASSERT_EQ(state.world.magic.partyShieldDefense, 100,
              "FUSE complete applies F0446 party shield defense");
    ASSERT_EQ(M11_GameView_GetEndgameDoNotDrawFluxcages(&state), 1,
              "FUSE complete sets F0446 do-not-draw-fluxcages gate");
    artifactMapX = artifactMapY = artifactElement = -1;
    artifactProjectiles = artifactExplosions = -1;
    artifactProjectileGfx = artifactExplosionType = -1;
    ASSERT_EQ(M11_GameView_ProbeViewportArtifactCounts(
                  &state, 1, -1, &artifactMapX, &artifactMapY,
                  &artifactElement, &artifactProjectiles,
                  &artifactExplosions, &artifactProjectileGfx,
                  &artifactExplosionType),
              1,
              "FUSE complete samples the left-front square after F0446 hide gate");
    ASSERT_EQ(artifactMapX, 1,
              "FUSE complete post-hide sample keeps fluxcage square x");
    ASSERT_EQ(artifactMapY, 1,
              "FUSE complete post-hide sample keeps fluxcage square y");
    ASSERT_EQ(artifactExplosions, 0,
              "FUSE complete hides surviving fluxcages from viewport sampling");
    ASSERT_EQ(artifactExplosionType, -1,
              "FUSE complete hides surviving fluxcage explosion type from viewport sampling");
    ASSERT_EQ(F0871_RUNTIME_CountFluxcagesOnSquare_Compat(
                  &state.world.explosions, 0, 2, 2, &partyFluxcageCount),
              1,
              "FUSE complete can count party-square fluxcages");
    ASSERT_EQ(partyFluxcageCount, 0,
              "FUSE complete removes F0446 party-square fluxcages");
    ASSERT_EQ(F0871_RUNTIME_CountFluxcagesOnSquare_Compat(
                  &state.world.explosions, 0, 2, 1, &chaosFluxcageCount),
              1,
              "FUSE complete can count Lord Chaos-square fluxcages");
    ASSERT_EQ(chaosFluxcageCount, 0,
              "FUSE complete removes F0446 Lord Chaos-square fluxcages");
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
        const struct ExplosionInstance_Compat* e =
            &state.world.explosions.entries[i];
        if (e->reserved0 == 0) continue;
        if (e->explosionType != C000_EXPLOSION_FIREBALL) continue;
        if (e->mapIndex != 0 || e->mapX != 2 || e->mapY != 1) continue;
        if (e->cell != EXPLOSION_CELL_CENTERED) continue;
        if (e->attack >= 55 && e->attack <= 255 &&
            ((e->attack - 55) % 40) == 0) {
            fireballAttackSeen[(e->attack - 55) / 40] += 1;
        }
        fireballBurstCount++;
    }
    ASSERT_EQ(fireballBurstCount, 7,
              "FUSE complete creates the F0446 opening fireball burst plus final fireball");
    for (i = 0; i < 6; ++i) {
        int expected = (i == 5) ? 2 : 1;
        ASSERT_EQ(fireballAttackSeen[i], expected,
                  "FUSE complete creates source fireball attacks and final 255");
    }
    fireballBurstCount = 0;
    for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
        const struct ExplosionInstance_Compat* e =
            &state.world.explosions.entries[i];
        if (e->reserved0 == 0) continue;
        if (e->explosionType != C003_EXPLOSION_HARM_NON_MATERIAL) continue;
        if (e->mapIndex != 0 || e->mapX != 2 || e->mapY != 1) continue;
        if (e->cell != EXPLOSION_CELL_CENTERED) continue;
        if (e->attack >= 55 && e->attack <= 255 &&
            ((e->attack - 55) % 40) == 0) {
            harmAttackSeen[(e->attack - 55) / 40] += 1;
        }
        harmBurstCount++;
    }
    ASSERT_EQ(harmBurstCount, 8,
              "FUSE complete keeps fuse HNM plus F0446 HNM burst and final HNM");
    for (i = 0; i < 6; ++i) {
        int expected = (i == 5) ? 3 : 1;
        ASSERT_EQ(harmAttackSeen[i], expected,
                  "FUSE complete creates source HNM attacks and final 255s");
    }
    ASSERT_EQ(state.audioState.lastSoundIndex, DM1_SND_BUZZ,
              "FUSE complete requests F0446 buzz sound");
    ASSERT_EQ(state.endgameBuzzRequestCount, 13,
              "FUSE complete requests initial plus Chaos/Order buzz sounds");
    ASSERT_EQ(state.endgameChaosOrderSwitchCount, 12,
              "FUSE complete records F0446 nested Chaos/Order switch count");
    ASSERT_EQ(state.endgameFuseSequenceUpdateTicks, 24,
              "FUSE complete records F0446 nested fuse-update cadence");
    ASSERT_EQ(state.endgameFuseSequenceTotalUpdateTicks, 45,
              "FUSE complete records all F0445 update calls plus text messages");
    ASSERT_EQ(state.endgameTextMessageDelayTicks, 1560,
              "FUSE complete records F0446 780-tick delay per text message");
    ASSERT_EQ(state.audioState.lastMusicTrackId,
              DM1_Endgame_GetEndingParams()->victoryMusicId,
              "FUSE complete requests F0446 game-won music track");
    ASSERT_EQ(state.endgameFinalDelayTicks,
              DM1_Endgame_GetEndingParams()->finalDelayTicks,
              "FUSE complete records F0446 final delay ticks");
    ASSERT_EQ(state.endgameFuseSequenceDelayTicks, 2160,
              "FUSE complete records total F0446 text plus final delay ticks");
    ASSERT_EQ(state.endgameFuseSequenceDelayRemainingTicks, 2160,
              "FUSE complete arms non-blocking F0446 delay countdown");
    ASSERT_EQ(state.endgameFinalHandoffReady, 0,
              "FUSE complete waits before marking final endgame handoff ready");
    ASSERT_EQ(M11_GameView_GetEndgameFinalHandoffReady(&state), 0,
              "FUSE complete public handoff query starts false");
    ASSERT_EQ(state.endgameRestartAllowed,
              DM1_Endgame_GetEndingParams()->restartAllowedAfterWin,
              "FUSE complete records F0446 restart disallow gate");
    ASSERT_EQ(state.endgameCalledWithTrue,
              DM1_Endgame_GetEndingParams()->endgameCalledWithTrue,
              "FUSE complete records F0444 Endgame(TRUE) handoff");
    ASSERT_EQ(state.world.explosions.count, 18,
              "FUSE complete keeps cages, fuse effect, bursts, and final pair");
    ASSERT_EQ(state.world.gameWon, 1,
              "FUSE complete sets M10 world game-won state");
    ASSERT_EQ(M11_GameView_IsGameWon(&state), 1,
              "FUSE complete sets M11 input/render game-won gate");
    ASSERT_EQ((int)M11_GameView_GetGameWonTick(&state), 41,
              "FUSE complete stores current game tick as game-won tick");
    gameTickAtWin = state.world.gameTick;
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_REDRAW,
              "FUSE complete endgame delay countdown requests redraw");
    ASSERT_EQ(state.endgameFuseSequenceDelayRemainingTicks, 2159,
              "FUSE complete endgame delay counts down one wait tick");
    ASSERT_EQ(state.endgameFinalHandoffReady, 0,
              "FUSE complete first wait tick does not mark final handoff ready");
    ASSERT_EQ(M11_GameView_GetEndgameFinalHandoffReady(&state), 0,
              "FUSE complete public handoff query stays false while waiting");
    ASSERT_EQ(state.world.gameTick, gameTickAtWin,
              "FUSE complete endgame delay does not advance source game time");
    for (i = 0; i < 2159; ++i) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.endgameFuseSequenceDelayRemainingTicks, 0,
              "FUSE complete endgame delay countdown drains exactly");
    ASSERT_EQ(state.endgameFinalHandoffReady, 1,
              "FUSE complete marks final handoff ready when delay drains");
    ASSERT_EQ(M11_GameView_GetEndgameFinalHandoffReady(&state), 1,
              "FUSE complete public handoff query flips when delay drains");
    ASSERT_EQ(M11_GameView_AdvanceIdleTick(&state), M11_GAME_INPUT_IGNORED,
              "FUSE complete completed endgame delay blocks idle gameplay ticks");
    ASSERT_EQ(state.world.gameTick, gameTickAtWin,
              "FUSE complete completed endgame still holds source game time");
    ASSERT_EQ(M11_GameView_GetMessageLogCount(&state), 3,
              "FUSE complete keeps status, final log, and source endgame text message");
    ASSERT_STR_EQ(M11_GameView_GetMessageLogEntry(&state, 2), "\nSECOND",
                  "FUSE complete prints F0446 text strings in A/B order without sort key");
}

int main(void) {
    printf("=== M11 Action Stamina Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: MENU.C G0494/F0407 and CHAMPION.C F0325\n\n");

    test_melee_contact_gate_reads_g0492_with_block_exception();
    test_projectile_action_required_mana_uses_g0496_route();
    test_block_action_spends_source_stamina();
    test_flip_action_prints_source_message_and_keeps_common_tail();
    test_throw_action_removes_action_hand_object();
    test_throw_full_projectile_list_still_accepts_f0328();
    test_throw_uses_post_f0304_throw_level_for_projectile();
    test_direct_throw_empty_action_hand_keeps_f0407_tail();
    test_throw_ven_potion_launches_removepotion_projectile();
    test_throw_ven_potion_advances_to_wall_impact_and_consumes();
    test_throw_ful_bomb_advances_to_wall_impact_and_consumes();
    test_throw_projectile_advances_after_scheduled_tick();
    test_projectile_creature_impact_at_zero_zero_applies_damage();
    test_projectile_creature_kill_spawns_f0190_death_smoke();
    test_projectile_creature_killed_some_drops_fixed_possessions();
    test_projectile_creature_killed_some_can_trigger_f0190_fear();
    test_projectile_creature_zero_scaled_attack_skips_poison_and_reaction();
    test_projectile_non_material_creature_passes_through_without_impact();
    test_projectile_harm_non_material_hits_non_material_creature();
    test_projectile_fireball_heals_black_flame_without_explosion();
    test_projectile_creature_impact_keeps_thrown_sharp_weapon();
    test_projectile_door_hit_schedules_and_dispatches_destruction();
    test_projectile_magical_door_zero_adjusted_skips_sound();
    test_projectile_champion_hit_applies_poison_dose();
    test_projectile_champion_hit_uses_f0321_defense_scale();
    test_projectile_champion_hit_can_kill_party();
    test_thrown_potion_wall_impact_consumes_potion_thing();
    test_thrown_weapon_wall_impact_materializes_source_square();
    test_leader_hand_throw_uses_f0328_temporary_action_hand();
    test_leader_hand_throw_full_projectile_list_accepts_f0328();
    test_leader_hand_throw_waterskin_uses_f0140_charge_weight();
    test_leader_hand_throw_container_uses_f0140_recursive_weight();
    test_block_action_disables_champion_for_source_ticks();
    test_direct_parry_empty_front_uses_f0402_failure_tail();
    test_freeze_life_common_branch_decrements_charges();
    test_freeze_life_blue_box_consumes_action_hand();
    test_freeze_life_green_box_consumes_action_hand_and_caps();
    test_light_decrements_action_hand_charges();
    test_heal_action_uses_hidden_heal_skill();
    test_heal_no_effect_still_runs_f0407_tail();
    test_window_action_schedules_thieves_eye_and_decrements_charges();
    test_spit_action_launches_f0327_fireball_and_decrements_charges();
    test_spit_low_mana_scales_kinetic_energy_before_f0327();
    test_fireball_action_uses_f0327_and_decrements_charges();
    test_fireball_low_mana_scales_kinetic_energy_before_f0327();
    test_air_projectile_actions_use_f0327_and_f0406_direction();
    test_air_projectile_low_mana_scales_kinetic_energy_before_f0327();
    test_fireball_projectile_create_failure_halves_action_xp();
    test_spit_projectile_create_failure_halves_action_xp();
    test_air_projectile_create_failure_halves_action_xp();
    test_invoke_action_uses_f0327_and_decrements_charges();
    test_invoke_low_mana_scales_random_kinetic_before_f0327();
    test_invoke_projectile_create_failure_halves_action_xp();
    test_cast_potion_spell_mutates_empty_flask();
    test_cast_zokathra_spell_materializes_ready_hand_junk();
    test_spellshield_low_mana_halves_disable_and_quarters_xp();
    test_spellshield_success_consumes_mana_charges_and_full_xp();
    test_fireshield_success_schedules_c78_and_expires();
    test_spellshield_high_defense_quarters_new_event_defense();
    test_shoot_no_ammunition_clears_action_xp_but_keeps_tail();
    test_direct_shoot_no_ammunition_clears_action_xp();
    test_shoot_action_uses_champion_cell_for_f0326_launch();
    test_climb_down_failure_cancels_disable_but_keeps_xp();
    test_direct_climb_down_failure_cancels_disable_but_keeps_xp();
    test_climb_down_open_pit_moves_party_and_keeps_tail();
    test_climb_down_closed_pit_moves_forward_without_fall();
    test_climb_down_group_over_pit_blocks_move_but_keeps_bug79_tail();
    test_action_defense_serializes_outside_v1_champion_blob();
    test_action_stamina_underflow_clamps_and_damages();
    test_fluxcage_schedules_f0224_remove_event();
    test_fluxcage_uses_pref0406_champion_target_square();
    test_fluxcage_wall_target_keeps_f0407_tail_without_cage();
    test_fluxcage_third_cage_schedules_lord_chaos_danger();
    test_fuse_incomplete_fluxcage_moves_lord_chaos_escape();
    test_fuse_without_lord_chaos_keeps_action_performed_tail();
    test_fuse_out_of_bounds_keeps_action_performed_without_explosion();
    test_fuse_complete_fluxcage_sets_m11_game_won_gate();
    test_melee_action_row_uses_auto_target_and_action_index();
    test_melee_action_row_targets_pref0407_champion_direction();
    test_melee_action_row_closed_door_targets_pref0407_champion_direction();
    test_parry_action_row_routes_through_f0402_f0231();
    test_melee_action_row_halves_disable_ticks_when_f0402_fails();
    test_melee_action_row_respects_live_candidate_no_action();
    test_disrupt_action_row_rejects_material_creature();
    test_disrupt_action_row_hits_non_material_creature();
    test_candidate_panel_blocks_action_menu_open();
    test_direct_non_melee_respects_candidate_panel_gate();
    test_empty_hand_punch_action_row_uses_live_melee();
    test_empty_hand_war_cry_frightens_front_group();
    test_war_cry_targets_pref0407_champion_direction();
    test_blow_horn_frightens_front_group_with_f0401_values();
    test_calm_frightens_front_group_with_f0401_values();
    test_brandish_frightens_front_group_with_f0401_values();
    test_confuse_decrements_charges_and_frightens_front_group();
    test_war_cry_resistance_halves_xp_without_flee();
    test_blow_horn_immune_halves_xp_without_flee();
    test_blow_horn_uses_f0304_influence_xp_semantics();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
