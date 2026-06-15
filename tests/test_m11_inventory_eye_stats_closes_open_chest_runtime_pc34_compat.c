/*
 * DM1 V1 empty-hand eye stats panel closes an open chest.
 *
 * Source evidence:
 *   ReDMCSB PANEL.C F0352 lines 2123-2159 routes C071 eye clicks.
 *   ReDMCSB PANEL.C F0351 lines 2013-2015 closes any open G0426 chest
 *     before switching G0424 to C02_PANEL_SKILLS_AND_STATISTICS.
 *   ReDMCSB CHEST.C F0334 lines 117-132 rewrites the container from only
 *     the non-empty visible G0425/C537..C544 slots, dropping hidden tail
 *     links beyond the eighth visible object.
 */
#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

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
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", \
                             (msg), a_, e_); } \
} while (0)

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

enum {
    EYE_SCREEN_X = 12 + 8,
    EYE_SCREEN_Y = 33 + 13 + 8,
    SEEDED_VISIBLE_CHEST_SLOTS = 8,
    SEEDED_CHEST_CHAIN_COUNT = 9
};

static void seed_champion(struct ChampionState_Compat* champ)
{
    int i;

    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memcpy(champ->name, "TIGGY   ", 8);
    champ->hp.current = 77;
    champ->hp.maximum = 100;
    champ->stamina.current = 66;
    champ->stamina.maximum = 90;
    champ->mana.current = 12;
    champ->mana.maximum = 33;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_open_chest_world(M11_GameViewState* state,
                                  struct DungeonThings_Compat* things,
                                  struct DungeonWeapon_Compat weapons[SEEDED_CHEST_CHAIN_COUNT],
                                  struct DungeonContainer_Compat containers[1],
                                  unsigned short chestThing,
                                  unsigned short weaponThings[SEEDED_CHEST_CHAIN_COUNT])
{
    int i;

    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(struct DungeonWeapon_Compat) * SEEDED_CHEST_CHAIN_COUNT);
    memset(containers, 0, sizeof(struct DungeonContainer_Compat));

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 1;
    state->world.things = things;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    seed_champion(&state->world.party.champions[0]);

    things->loaded = 1;
    things->weapons = weapons;
    things->weaponCount = SEEDED_CHEST_CHAIN_COUNT;
    things->containers = containers;
    things->containerCount = 1;

    for (i = 0; i < SEEDED_CHEST_CHAIN_COUNT; ++i) {
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 8; /* Dagger row: container-compatible object. */
        weapons[i].next = (i + 1 < SEEDED_CHEST_CHAIN_COUNT)
            ? weaponThings[i + 1]
            : THING_ENDOFLIST;
    }
    containers[0].slot = weaponThings[0];
    state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestThing;
}

static void test_empty_hand_eye_stats_closes_open_chest_and_truncates_tail(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[SEEDED_CHEST_CHAIN_COUNT];
    struct DungeonContainer_Compat containers[1];
    unsigned short weaponThings[SEEDED_CHEST_CHAIN_COUNT];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);

    seed_open_chest_world(&state, &things, weapons, containers,
                          chestThing, weaponThings);

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before empty-hand eye stats route");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "open-chest sentinel names the action-hand chest before C071");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "leader hand is empty so C071 must take PANEL.C F0351");
    ASSERT_EQ(weapons[SEEDED_VISIBLE_CHEST_SLOTS - 1].next,
              weaponThings[SEEDED_VISIBLE_CHEST_SLOTS],
              "fixture has hidden ninth tail beyond visible C537..C544");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "C071 empty-hand eye click redraws the champion stats panel");

    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "PANEL.C F0351 closes G0426 before stats panel activation");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 1,
              "C071 empty-hand eye click activates the stats panel");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "F0351 close does not activate scroll panel state");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "F0351 close does not activate object-description state");
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "F0334 close keeps the first visible chest object as head");
    ASSERT_EQ(weapons[6].next, weaponThings[7],
              "F0334 close relinks the last visible pair before truncation");
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "F0334 close drops the hidden ninth tail beyond C544");
    ASSERT_TRUE(strstr(state.inspectDetail, "MANA 12/33") != NULL,
                "F0351 stats detail remains active after chest close");
}

int main(void)
{
    printf("=== DM1 V1 Empty-Hand Eye Stats Closes Open Chest Runtime Gate ===\n");
    printf("ReDMCSB: PANEL.C F0352 2123-2159, F0351 2013-2015, "
           "CHEST.C F0334 117-132\n\n");

    test_empty_hand_eye_stats_closes_open_chest_and_truncates_tail();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
