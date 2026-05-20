/*
 * Source-lock gate for M11 starvation/dehydration HP damage integration.
 *
 * ReDMCSB evidence:
 *   CHAMPION.C F0331 lines 2380-2412: starving food/water below -512
 *     adds stamina loss, keeps depleting food/water, then calls F0325.
 *   CHAMPION.C F0325 lines 2025-2048: stamina underflow becomes pending
 *     HP damage and stamina clamps to zero.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_TRUE(cond, msg) do { \
    if (cond) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

static void seed_starving_state(M11_GameViewState* state,
                                struct DungeonDatState_Compat* dungeon,
                                struct DungeonMapDesc_Compat maps[1]) {
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat));
    maps[0].width = 1;
    maps[0].height = 1;
    dungeon->header.mapCount = 1;
    dungeon->maps = maps;

    M11_GameView_Init(state);
    state->active = 1;
    state->world.dungeon = dungeon;
    state->world.gameTick = 5; /* AdvanceIdleTick increments to 6. */
    state->world.partyMapIndex = 0;
    state->world.newPartyMapIndex = 0;
    state->world.party.mapIndex = 0;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[0].stamina.current = 4;
    state->world.party.champions[0].stamina.maximum = 1500;
    state->world.party.champions[0].food = -600;
    state->world.party.champions[0].water = -600;
    state->world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 30;
    state->world.party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 40;
}

static void test_starvation_pending_damage_reaches_runtime_hp(void) {
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];

    seed_starving_state(&state, &dungeon, maps);

    (void)M11_GameView_AdvanceIdleTick(&state);

    ASSERT_TRUE(state.world.party.champions[0].stamina.current == 0,
                "starvation underflow clamps stamina to zero");
    ASSERT_TRUE(state.world.party.champions[0].hp.current < 100,
                "starvation pending HP damage is applied in M11 runtime");
    ASSERT_TRUE(state.world.party.champions[0].food < -600,
                "source starvation path continues food depletion");
    ASSERT_TRUE(state.world.party.champions[0].water < -600,
                "source dehydration path continues water depletion");
}

int main(void) {
    printf("=== M11 Starvation Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: CHAMPION.C F0331 starvation/dehydration and F0325 underflow damage\n\n");

    test_starvation_pending_damage_reaches_runtime_hp();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
