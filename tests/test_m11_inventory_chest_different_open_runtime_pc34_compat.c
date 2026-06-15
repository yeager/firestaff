/*
 * DM1 V1 M11 different action-hand chest runtime regression.
 *
 * ReDMCSB CHEST.C F0333 lines 30-38 first returns for the same G0426
 * chest, otherwise calls F0334 before assigning the newly requested chest.
 * CHEST.C F0334 lines 117-132 rewrites only visible C537..C544 slots, so
 * opening a different action-hand chest must close and truncate the prior
 * overfull chest before the replacement chest becomes G0426.
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

static unsigned short thing_ref(int type, int index)
{
    return (unsigned short)(((type & 0x0F) << 10) | (index & 0x03FF));
}

static void seed_inventory_view(M11_GameViewState* state,
                                struct DungeonThings_Compat* things,
                                struct DungeonWeapon_Compat* weapons,
                                int weaponCount,
                                struct DungeonContainer_Compat* containers,
                                int containerCount)
{
    int i;

    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(*weapons) * (size_t)weaponCount);
    memset(containers, 0, sizeof(*containers) * (size_t)containerCount);
    things->weapons = weapons;
    things->weaponCount = weaponCount;
    things->containers = containers;
    things->containerCount = containerCount;

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

static int weapon_chain_count(const struct DungeonWeapon_Compat* weapons,
                              int weaponCount,
                              unsigned short firstThing,
                              int maxWalk)
{
    unsigned short thing = firstThing;
    int count = 0;

    while (thing != THING_NONE && thing != THING_ENDOFLIST && count < maxWalk) {
        int index;
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) break;
        index = (int)THING_GET_INDEX(thing);
        if (index < 0 || index >= weaponCount) break;
        ++count;
        thing = weapons[index].next;
    }
    return count;
}

static int weapon_chain_contains(const struct DungeonWeapon_Compat* weapons,
                                 int weaponCount,
                                 unsigned short firstThing,
                                 unsigned short targetThing,
                                 int maxWalk)
{
    unsigned short thing = firstThing;
    int count = 0;

    while (thing != THING_NONE && thing != THING_ENDOFLIST && count < maxWalk) {
        int index;
        if (thing == targetThing) return 1;
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) break;
        index = (int)THING_GET_INDEX(thing);
        if (index < 0 || index >= weaponCount) break;
        ++count;
        thing = weapons[index].next;
    }
    return 0;
}

static void test_different_action_hand_chest_closes_previous_visible_slots(void)
{
    enum {
        CHEST_A_INDEX = 0,
        CHEST_B_INDEX = 1,
        CHEST_A_ITEM_COUNT = 9,
        CHEST_B_FIRST_ITEM = 9,
        CHEST_B_ITEM_COUNT = 3,
        WEAPON_COUNT = 12
    };
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[WEAPON_COUNT];
    struct DungeonContainer_Compat containers[2];
    unsigned short chestA = thing_ref(THING_TYPE_CONTAINER, CHEST_A_INDEX);
    unsigned short chestB = thing_ref(THING_TYPE_CONTAINER, CHEST_B_INDEX);
    unsigned short weaponThings[WEAPON_COUNT];
    int i;

    seed_inventory_view(&state, &things, weapons, WEAPON_COUNT, containers, 2);
    for (i = 0; i < WEAPON_COUNT; ++i) {
        weaponThings[i] = thing_ref(THING_TYPE_WEAPON, i);
        weapons[i].type = 8; /* DUNGEON.C G0237 line 1145: dagger-like. */
        weapons[i].next = THING_ENDOFLIST;
    }
    for (i = 0; i < CHEST_A_ITEM_COUNT - 1; ++i) {
        weapons[i].next = weaponThings[i + 1];
    }
    for (i = CHEST_B_FIRST_ITEM;
         i < CHEST_B_FIRST_ITEM + CHEST_B_ITEM_COUNT - 1;
         ++i) {
        weapons[i].next = weaponThings[i + 1];
    }
    containers[CHEST_A_INDEX].slot = weaponThings[0];
    containers[CHEST_B_INDEX].slot = weaponThings[CHEST_B_FIRST_ITEM];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestA;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "first action-hand chest opens");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestA,
              "first open records chest A as G0426");
    ASSERT_EQ(weapon_chain_count(weapons, WEAPON_COUNT,
                                 containers[CHEST_A_INDEX].slot, 12),
              9,
              "chest A starts with a hidden ninth item");
    ASSERT_EQ(weapons[7].next, weaponThings[8],
              "chest A C544-equivalent item still links to hidden ninth item while open");

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestB;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "different action-hand chest opens");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestB,
              "different open records chest B as G0426");
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(
                  &state, CHAMPION_SLOT_ACTION_HAND),
              145,
              "different open draws the open action-hand chest icon");

    ASSERT_EQ(weapon_chain_count(weapons, WEAPON_COUNT,
                                 containers[CHEST_A_INDEX].slot, 12),
              8,
              "opening chest B closes chest A from visible C537..C544 slots only");
    ASSERT_EQ(weapon_chain_contains(weapons, WEAPON_COUNT,
                                    containers[CHEST_A_INDEX].slot,
                                    weaponThings[8], 12),
              0,
              "different-open close drops chest A hidden ninth tail");
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "different-open close terminates chest A visible C544 item");
    ASSERT_EQ(weapon_chain_count(weapons, WEAPON_COUNT,
                                 containers[CHEST_B_INDEX].slot, 12),
              3,
              "chest B remains the newly materialized source chain");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "closing chest B clears G0426");
    ASSERT_EQ(weapon_chain_count(weapons, WEAPON_COUNT,
                                 containers[CHEST_B_INDEX].slot, 12),
              3,
              "closing chest B preserves its visible chain");
}

int main(void)
{
    printf("=== M11 DM1 V1 Different Chest Open Runtime Gate ===\n");
    printf("ReDMCSB: CHEST.C F0333 lines 30-38; F0334 lines 117-132\n");

    test_different_action_hand_chest_closes_previous_visible_slots();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
