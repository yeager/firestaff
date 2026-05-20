/*
 * Source-lock gate for M11 action-menu stamina drain.
 *
 * ReDMCSB evidence:
 *   MENU.C G0494 lines 292-337: source action-stamina table.
 *   MENU.C F0407 lines 1246-1272: action stamina is table value plus
 *     M005_RANDOM(2).
 *   MENU.C F0407 lines 1623-1624 and CHAMPION.C F0325 lines 2025-2048:
 *     the common action tail decrements stamina and clamps underflow.
 */

#include "m11_game_view.h"

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
    state->world.party.champions[0].name[0] = 'H';
    state->world.party.champions[0].name[1] = 'A';
    state->world.party.champions[0].name[2] = 'L';
    state->world.party.champions[0].name[3] = 'K';
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

static void test_action_stamina_underflow_clamps_and_damages(void) {
    M11_GameViewState state;
    seed_state(&state, 2, 1);

    (void)M11_GameView_TriggerNonMeleeActionByIndex(&state, 0, 1); /* BLOCK */

    ASSERT_EQ(state.world.party.champions[0].stamina.current, 0,
              "underflow action stamina clamps to zero");
    ASSERT_EQ(state.world.party.champions[0].hp.current, 99,
              "underflow action stamina applies F0325-style HP damage");
}

int main(void) {
    printf("=== M11 Action Stamina Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: MENU.C G0494/F0407 and CHAMPION.C F0325\n\n");

    test_block_action_spends_source_stamina();
    test_action_stamina_underflow_clamps_and_damages();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
