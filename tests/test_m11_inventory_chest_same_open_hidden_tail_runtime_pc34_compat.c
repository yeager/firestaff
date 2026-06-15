/*
 * DM1 V1 M11 chest same-open hidden-tail runtime regression.
 *
 * ReDMCSB CHEST.C F0333 lines 30-32 returns immediately when the requested
 * chest is already G0426_T_OpenChest.  That same-open path must not run
 * CHEST.C F0334 lines 117-132, because F0334 rewrites only visible C537..C544
 * slots and would prematurely sever the hidden 9th+ linked tail.
 */
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass;
static int g_fail;

#define ASSERT_EQ(got, want, msg) do { \
    unsigned long _got = (unsigned long)(got); \
    unsigned long _want = (unsigned long)(want); \
    if (_got != _want) { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s got=%lu want=%lu\n", msg, _got, _want); \
    } else { \
        ++g_pass; \
    } \
} while (0)

static void seed_inventory_view(M11_GameViewState* state,
                                struct DungeonThings_Compat* things,
                                struct DungeonWeapon_Compat* firstWeapon)
{
    int i;

    memset(things, 0, sizeof(*things));
    memset(firstWeapon, 0, sizeof(*firstWeapon));
    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 1;
    state->world.things = things;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state->world.party.champions[0].inventory[i] = THING_NONE;
    }
}

static void test_same_action_hand_open_preserves_hidden_tail_until_close(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[10];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing =
        (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short weaponThings[10];
    int i;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 10;
    things.containers = containers;
    things.containerCount = 1;

    for (i = 0; i < 10; ++i) {
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 2;
        weapons[i].next = (i < 9) ? weaponThings[i + 1] : THING_ENDOFLIST;
    }
    containers[0].slot = weaponThings[0];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "first action-hand chest open succeeds");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "first open records G0426 open chest thing");
    ASSERT_EQ(weapons[7].next, weaponThings[8],
              "first open leaves C544 linked to hidden 9th object");
    ASSERT_EQ(weapons[8].next, weaponThings[9],
              "first open leaves hidden 9th object linked to 10th object");

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "same action-hand chest open returns successfully");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "same-open no-op keeps the same G0426 chest active");
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "same-open no-op preserves source container head");
    ASSERT_EQ(weapons[7].next, weaponThings[8],
              "same-open no-op preserves hidden 9th tail link");
    ASSERT_EQ(weapons[8].next, weaponThings[9],
              "same-open no-op preserves hidden 10th tail link");
    ASSERT_EQ(weapons[9].next, THING_ENDOFLIST,
              "same-open no-op preserves hidden tail terminator");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "explicit close clears G0426 open chest thing");
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "explicit close rewrites visible C544 as terminator");
    ASSERT_EQ(weapons[8].next, weaponThings[9],
              "explicit close leaves detached hidden tail internally intact");
}

int main(void)
{
    printf("=== M11 DM1 V1 Chest Same-Open Hidden-Tail Runtime Gate ===\n");
    printf("ReDMCSB: CHEST.C F0333 lines 30-32; F0334 lines 117-132\n");

    test_same_action_hand_open_preserves_hidden_tail_until_close();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
