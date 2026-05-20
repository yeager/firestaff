/*
 * Source-lock gate for the M11 normal V1 inventory full backpack panel.
 *
 * ReDMCSB evidence:
 *   DEFS.H lines 778-817: C00..C37 source slot namespace
 *   DATA.C lines 1049-1087: 30 champion inventory masks + 8 chest masks
 *   CHAMPION.C F0302 lines 677-699: inventory slot-box click resolves
 *     source slot index, reads champion slot/chest slot, no-ops empty/empty,
 *     and gates leader-hand placement through AllowedSlots & SlotMasks
 *   CHAMPION.C F0302 lines 700-712: leader hand/slot swap and redraw
 *   PANEL.C F0347 lines 1651-1691: inventory panel redraw is driven by
 *     the active inventory champion action hand and panel content route
 *   PANEL.C F0352 lines 1250-1254: weapon descriptions expose Cursed,
 *     Poisoned, Broken and ChargeCount-derived state from WEAPON data
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

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static void seed_inventory_view(M11_GameViewState* state,
                                struct DungeonThings_Compat* things,
                                struct DungeonWeapon_Compat* weapon) {
    int i;
    memset(things, 0, sizeof(*things));
    memset(weapon, 0, sizeof(*weapon));
    weapon->type = 8; /* dagger: source AllowedSlots includes backpack/container */
    things->weapons = weapon;
    things->weaponCount = 1;

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

static void test_extended_backpack_source_mapping(void) {
    ASSERT_EQ(CHAMPION_SLOT_COUNT, 30,
              "champion runtime storage keeps 30 source inventory slots");
    ASSERT_EQ(CHAMPION_SLOT_ACTION_HAND, CHAMPION_SLOT_HAND_RIGHT,
              "action hand aliases the real C01 hand storage slot");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(CHAMPION_SLOT_BACKPACK_9), 29,
              "BACKPACK_9 maps to source slot box C528");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(CHAMPION_SLOT_BACKPACK_17), 37,
              "BACKPACK_17 maps to source slot box C536");
    ASSERT_EQ(M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(29), CHAMPION_SLOT_BACKPACK_9,
              "source slot box C528 maps back to BACKPACK_9");
    ASSERT_EQ(M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(37), CHAMPION_SLOT_BACKPACK_17,
              "source slot box C536 maps back to BACKPACK_17");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxZoneCount(), 30,
              "source inventory exposes all C507..C536 slot-box zones");
}

static void test_extended_backpack_runtime_clicks(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = 0, zone = 0;

    seed_inventory_view(&state, &things, &weapon);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_9] = daggerThing;
    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(29, &sx, &sy, &sw, &sh),
                "C528 zone exists");
    ASSERT_EQ(M11_GameView_GetV1MouseCommandForPoint(M11_DM1_MOUSE_LIST_INVENTORY,
                                                     sx + sw / 2,
                                                     33 + sy + sh / 2,
                                                     M11_DM1_MOUSE_MASK_LEFT,
                                                     &space,
                                                     &zone),
              49,
              "C528 resolves to source command C049");
    ASSERT_EQ(zone, 528, "C528 resolves to source zone id 528");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C528 picks the extended backpack item");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_9], THING_NONE,
              "C528 pickup clears BACKPACK_9 storage");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), daggerThing,
              "C528 pickup moves item to leader hand");

    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(37, &sx, &sy, &sw, &sh),
                "C536 zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C536 places leader hand into BACKPACK_17");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_17], daggerThing,
              "C536 placement fills BACKPACK_17 storage");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "C536 placement clears leader hand");
}

static void test_eye_panel_weapon_attribute_flags(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    seed_inventory_view(&state, &things, &weapon);
    weapon.type = 8;
    weapon.cursed = 1;
    weapon.poisoned = 1;
    weapon.broken = 1;
    weapon.chargeCount = 7;

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, daggerThing), 1,
              "leader hand accepts source weapon thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source weapon description");
    ASSERT_TRUE(strstr(state.inspectDetail, "CURSED") != NULL,
                "weapon eye panel reports source cursed flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "POISONED") != NULL,
                "weapon eye panel reports source poisoned flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "BROKEN") != NULL,
                "weapon eye panel reports source broken flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "CHARGE 7") != NULL,
                "weapon eye panel reports source charge count");
}

int main(void) {
    printf("=== M11 Inventory Full Panel Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: DEFS.H 778-817, DATA.C 1049-1087, CHAMPION.C F0302 677-712, PANEL.C F0347 1651-1691\n\n");

    test_extended_backpack_source_mapping();
    test_extended_backpack_runtime_clicks();
    test_eye_panel_weapon_attribute_flags();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
