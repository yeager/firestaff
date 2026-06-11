/*
 * DM1 V1 inventory eye route closes an already-open chest.
 *
 * Runtime regression: open an action-hand chest with nine linked weapon
 * records, then inspect a separate leader-hand weapon through the real M11
 * eye pointer route. ReDMCSB closes G0426 before drawing replacement eye
 * panels, so Firestaff must compact only the visible C537..C544 slots and
 * drop the hidden ninth tail while activating the object-description panel.
 *
 * Source evidence:
 *   ReDMCSB PANEL.C F0352 lines 2153-2158 dispatches a non-empty leader
 *   hand to F0342.
 *   ReDMCSB PANEL.C F0342 lines 1119-1124 closes G0426 through F0334 before
 *   scroll/container/object-description eye routes draw their panel.
 *   ReDMCSB CHEST.C F0334 lines 117-132 rewrites the container from the
 *   current visible G0425_aT_ChestSlots only.
 */
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass;
static int g_fail;

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

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
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
            break;
        }
        index = (int)THING_GET_INDEX(thing);
        if (index < 0 || index >= weaponCount) {
            break;
        }
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
        if (thing == targetThing) {
            return 1;
        }
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
            break;
        }
        index = (int)THING_GET_INDEX(thing);
        if (index < 0 || index >= weaponCount) {
            break;
        }
        ++count;
        thing = weapons[index].next;
    }
    return 0;
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[10];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short weaponThings[10];
    int i;

    printf("=== M11 Inventory Eye Closes Open Chest Runtime Gate ===\n");
    printf("ReDMCSB: PANEL.C F0352 2153-2158, F0342 1119-1124, CHEST.C F0334 117-132\n\n");

    seed_inventory_view(&state, &things, weapons, 10, containers, 1);
    for (i = 0; i < 10; ++i) {
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 8; /* DUNGEON.C object-info index 31: dagger-like. */
        weapons[i].next = (i < 8) ? weaponThings[i + 1] : THING_ENDOFLIST;
    }
    containers[0].slot = weaponThings[0];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before leader-hand object eye route");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "open chest state names the action-hand container before eye route");
    ASSERT_EQ(weapon_chain_count(weapons, 10, containers[0].slot, 12), 9,
              "overfull chest starts with a hidden ninth tail item");

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, weaponThings[9]), 1,
              "leader hand accepts separate weapon for object-description route");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click on leader-hand weapon replaces the open chest panel");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "object-description eye route closes the previously open chest");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 1,
              "object-description route becomes the active inventory detail panel");
    ASSERT_EQ(state.v1ObjectDescriptionThing, weaponThings[9],
              "object-description panel tracks the inspected leader-hand weapon");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), weaponThings[9],
              "eye inspection leaves the leader-hand weapon in hand");
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(
                  &state, CHAMPION_SLOT_ACTION_HAND),
              144,
              "closed action-hand chest icon is restored after eye route switch");
    ASSERT_EQ(weapon_chain_count(weapons, 10, containers[0].slot, 12), 8,
              "eye route close compacts the chest to its visible eight slots");
    ASSERT_EQ(weapon_chain_contains(weapons, 10, containers[0].slot,
                                    weaponThings[8], 12),
              0,
              "eye route close drops the hidden ninth chest tail");

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
