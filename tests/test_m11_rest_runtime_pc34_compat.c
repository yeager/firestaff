/*
 * Source-lock gate for M11 rest/wake runtime behavior.
 *
 * ReDMCSB evidence:
 *   COMMAND.C F0380 lines 2336-2359: C145_COMMAND_REST sets
 *     G0300_B_PartyIsResting and switches to party-resting input tables.
 *   COMMAND.C F0380 lines 2361-2363 and CHAMPION.C F0314 lines
 *     1382-1410: C146_COMMAND_WAKE_UP clears G0300_B_PartyIsResting and
 *     restores normal interface input. The wake command exits through
 *     T0380042 without advancing a resting recovery tick.
 *   PROJEXPL.C F0230 lines 1346-1373: creature damage wakes the party
 *     before applying the resting-hit path.
 */

#include "m11_game_view.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_combat_pc34_compat.h"

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

static void seed_resting_attack_state(M11_GameViewState* state,
                                      struct DungeonDatState_Compat* dungeon,
                                      struct DungeonMapDesc_Compat maps[2],
                                      struct DungeonThings_Compat* things,
                                      struct DungeonGroup_Compat groups[1],
                                      unsigned short squareFirstThings[2]) {
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat) * 2);
    memset(things, 0, sizeof(*things));
    memset(groups, 0, sizeof(struct DungeonGroup_Compat));

    maps[0].width = 1;
    maps[0].height = 1;
    maps[1].width = 1;
    maps[1].height = 1;
    maps[1].difficulty = 0;
    dungeon->header.mapCount = 2;
    dungeon->maps = maps;

    groups[0].creatureType = 12; /* Skeleton: attack cadence 6 ticks. */
    groups[0].count = 0;
    groups[0].health[0] = 90;
    things->groups = groups;
    things->groupCount = 1;
    squareFirstThings[0] = THING_ENDOFLIST;
    squareFirstThings[1] = (unsigned short)(THING_TYPE_GROUP << 10);
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 2;

    M11_GameView_Init(state);
    state->active = 1;
    state->resting = 1;
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.gameTick = 5; /* AdvanceIdleTick increments to 6 before AI. */
    state->world.partyMapIndex = 1;
    state->world.newPartyMapIndex = 1;
    state->world.partyIsResting = 1;
    state->world.lifecycle.rest.isResting = 1;
    state->world.party.mapIndex = 1;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[0].stamina.current = 100;
    state->world.party.champions[0].stamina.maximum = 100;
    state->world.party.champions[0].attributes[CHAMPION_ATTR_DEXTERITY] = 0;
}

static void test_creature_attack_wakes_resting_party(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[2];
    int i;
    int sawRedraw = 0;

    seed_resting_attack_state(&state, &dungeon, maps, &things, groups, squareFirstThings);

    for (i = 0; i < 24 && state.resting; ++i) {
        if (M11_GameView_AdvanceIdleTick(&state) == M11_GAME_INPUT_REDRAW) {
            sawRedraw = 1;
        }
    }
    ASSERT_EQ(sawRedraw, 1, "idle ticks with adjacent creature redraw");
    ASSERT_EQ(state.resting, 0, "creature attack clears resting flag");
    ASSERT_EQ(state.world.partyIsResting, 0,
              "creature attack clears world resting mirror");
    ASSERT_EQ(state.world.lifecycle.rest.isResting, 0,
              "creature attack clears lifecycle resting mirror");
    ASSERT_EQ(state.damageFlashTimer > 0, 1, "creature attack damage cue fires");
    ASSERT_EQ(state.world.party.champions[0].hp.current < 100, 1,
              "creature attack applies damage");
}

static int run_awake_creature_attack_fixture_with_parry_xp(int32_t fighterXp,
                                                           int32_t parryXp) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short squareFirstThings[2];
    int i;
    int startHp;

    seed_resting_attack_state(&state, &dungeon, maps, &things, groups,
                              squareFirstThings);
    state.resting = 0;
    state.world.partyIsResting = 0;
    state.world.lifecycle.rest.isResting = 0;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_FIGHTER].experience = fighterXp;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PARRY].experience = parryXp;
    (void)F0730_COMBAT_RngInit_Compat(&state.world.masterRng, 1u);
    startHp = (int)state.world.party.champions[0].hp.current;

    for (i = 0; i < 24; ++i) {
        (void)M11_GameView_AdvanceIdleTick(&state);
        if ((int)state.world.party.champions[0].hp.current < startHp) {
            break;
        }
    }

    return startHp - (int)state.world.party.champions[0].hp.current;
}

static void test_creature_attack_runtime_uses_f0230_parry_skill(void) {
    int noParryDamage;
    int parryDamage;

    /*
     * ReDMCSB PROJEXPL.C F0230:1390-1395 subtracts
     * (F0303_CHAMPION_GetSkillLevel(C07_SKILL_PARRY) << 1) before the
     * creature-hit damage cascade.  This keeps the M11 runtime tick path
     * pinned, not only the public F0230 probe helper.
     */
    noParryDamage = run_awake_creature_attack_fixture_with_parry_xp(0, 0);
    parryDamage = run_awake_creature_attack_fixture_with_parry_xp(8192, 8192);

    ASSERT_EQ(noParryDamage > 0, 1,
              "F0230 control fixture takes creature damage");
    ASSERT_EQ(parryDamage < noParryDamage, 1,
              "F0230 runtime parry skill lowers creature damage");
}

static void seed_rest_recovery_boundary_state(M11_GameViewState* state) {
    memset(state, 0, sizeof(*state));
    M11_GameView_Init(state);
    state->active = 1;
    state->resting = 1;
    state->world.gameTick = 3;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 9;
    state->world.party.champions[0].hp.maximum = 12;
    state->world.party.champions[0].stamina.current = 19;
    state->world.party.champions[0].stamina.maximum = 20;
    state->world.party.champions[0].mana.current = 4;
    state->world.party.champions[0].mana.maximum = 5;
}

static void test_wake_input_does_not_spend_recovery_tick(void) {
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_rest_recovery_boundary_state(&state);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_REST_TOGGLE);

    /*
     * ReDMCSB COMMAND.C F0380:2361-2363 dispatches C146_COMMAND_WAKE_UP
     * straight to CHAMPION.C F0314, which clears G0300_B_PartyIsResting
     * at lines 1393-1396.  It does not enter GAMELOOP.C's next time
     * advance, so Firestaff must not award the tick-4 rest recovery when
     * the player wakes exactly at that boundary.
     */
    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW, "wake input redraws");
    ASSERT_EQ(state.resting, 0, "wake input clears resting flag");
    ASSERT_EQ(state.world.gameTick, 3, "wake input does not advance game tick");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 9,
              "wake input does not recover hp");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 19,
              "wake input does not recover stamina");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 4,
              "wake input does not recover mana");
}

static void test_resting_idle_tick_recovers_at_boundary(void) {
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_rest_recovery_boundary_state(&state);

    result = M11_GameView_AdvanceIdleTick(&state);

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW, "resting idle tick redraws");
    ASSERT_EQ(state.resting, 1, "resting idle tick keeps rest state");
    ASSERT_EQ(state.world.gameTick, 4, "resting idle tick advances to boundary");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 10,
              "resting idle tick recovers hp at boundary");
    ASSERT_EQ(state.world.party.champions[0].stamina.current, 20,
              "resting idle tick recovers capped stamina at boundary");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 5,
              "resting idle tick recovers capped mana at boundary");
}

int main(void) {
    printf("=== M11 Rest Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: COMMAND.C F0380 rest/wake, CHAMPION.C F0314 wake, PROJEXPL.C F0230 attack wake\n\n");

    test_creature_attack_wakes_resting_party();
    test_creature_attack_runtime_uses_f0230_parry_skill();
    test_wake_input_does_not_spend_recovery_tick();
    test_resting_idle_tick_recovers_at_boundary();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
