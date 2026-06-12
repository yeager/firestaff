/*
 * DM1 V1 chest-eye open-to-action-hand-switch runtime gate.
 *
 * Lane: while a different container (chest A) is the action-hand open chest
 * (G0426 names chest A) and a separate leader-hand container (chest B) is
 * held, the C071 eye click on chest B routes through PANEL.C F0352 ->
 * F0342 -> F0333, which first calls F0334 to close chest A and then opens
 * chest B with P0694_B_PressingEye = 1. The transition has three pieces
 * the related gates do not pin together:
 *
 *   1. Chest A close (F0334) is performed on chest A's visible C537..C544
 *      chain only, so its hidden ninth tail is dropped at close time even
 *      though the leader hand is the input target, not the action hand.
 *
 *   2. Chest B open (F0333) sets v1OpenChestThing = chest B and
 *      v1OpenChestOpenedByEye = 1, so chest B's C025 panel is rendered
 *      without the C145 action-hand open-chest icon suppression in the
 *      F0333 lines 43-46 P0694_B_PressingEye branch.
 *
 *   3. The action-hand slot still holds chest A (the eye route does not
 *      swap the action-hand container), so the action-hand icon must
 *      revert to C144 (closed container) and chest A's slot chain must
 *      end with the eighth visible weapon instead of THING_ENDOFLIST.
 *
 * This is intentionally non-duplicative with:
 *   - test_m11_inventory_eye_closes_open_chest_runtime_pc34_compat
 *       (leader-hand weapon / scroll eye route replaces the panel)
 *   - test_m11_inventory_eye_stats_closes_open_chest_runtime_pc34_compat
 *       (empty leader hand routes to F0351 champion-stats panel)
 *   - test_m11_inventory_full_panel_runtime_pc34_compat
 *       (leader-hand container eye route, but no prior open chest)
 *   - test_dm1_v1_chest_close_stack_merge_pc34_compat
 *       (F0334 close rewire for sparse G0425, no eye route)
 *   - test_dm1_v1_chest_open_mirror_rotation_three_way_pc34_compat
 *       (mirror-candidate chest open race, no eye route)
 *
 * Source evidence (verbatim ReDMCSB anchors):
 *   PANEL.C F0352 lines 2111-2159 dispatches C071 eye clicks.
 *   PANEL.C F0342 lines 1119-1124 closes G0426 through F0334 when
 *     G0331_B_PressingEye or G0333_B_PressingMouth is set.
 *   PANEL.C F0342 lines 1132-1135 routes a CONTAINER-typed leader-hand
 *     thing to F0333_INVENTORY_OpenAndDrawChest with P0694_B_PressingEye.
 *   CHEST.C F0333 lines 30-32 returns early when G0426 already names the
 *     same chest, leaving v1OpenChestOpenedByEye untouched.
 *   CHEST.C F0333 lines 35-38 closes a different already-open G0426 chest
 *     before opening the requested container, populating G0425_aT_ChestSlots
 *     with the first eight visible things only.
 *   CHEST.C F0333 lines 43-46 suppresses the C145 action-hand open-chest
 *     icon when P0694_B_PressingEye is set.
 *   CHEST.C F0334 lines 117-132 rewrites the container from the
 *     G0425_aT_ChestSlots only, dropping any hidden ninth-tail link.
 *   COMMAND.C F0380 lines 2316-2320 routes C071_COMMAND_CLICK_ON_EYE to
 *     F0352.
 */
#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* Mirror the strong symbols that the M11 chest tests link against so the
 * probe does not need the full Firestaff frontend. The values are unused
 * for this contract-only gate; they just satisfy the linker. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass;
static int g_fail;

#define ASSERT_EQ(actual, expected, msg) do { \
    long long a_ = (long long)(actual); \
    long long e_ = (long long)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %lld expected %lld\n", (msg), a_, e_); } \
} while (0)

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

enum {
    EYE_SCREEN_X = 12 + 8,
    EYE_SCREEN_Y = 33 + 13 + 8,

    CHEST_A_INDEX = 0,
    CHEST_B_INDEX = 1,
    CHEST_A_CHAIN_COUNT = 9, /* 8 visible + 1 hidden tail */
    CHEST_B_CHAIN_COUNT = 4, /* 4 visible weapons */
    WEAPON_BASE_FOR_CHEST_A = 0,
    WEAPON_BASE_FOR_CHEST_B = 9
};

static unsigned short make_weapon_thing(int baseIndex)
{
    return (unsigned short)((THING_TYPE_WEAPON << 10) | baseIndex);
}

static unsigned short make_container_thing(int containerIndex)
{
    return (unsigned short)((THING_TYPE_CONTAINER << 10) | containerIndex);
}

static void seed_champion(struct ChampionState_Compat* champ)
{
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memcpy(champ->name, "TIGGY   ", 8);
    champ->hp.current = 100;
    champ->hp.maximum = 100;
    champ->stamina.current = 90;
    champ->stamina.maximum = 90;
    champ->mana.current = 33;
    champ->mana.maximum = 33;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_world_for_test(M11_GameViewState* state,
                                struct DungeonThings_Compat* things,
                                struct DungeonWeapon_Compat* weapons,
                                int weaponCount,
                                struct DungeonContainer_Compat* containers,
                                int containerCount)
{
    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(*weapons) * (size_t)weaponCount);
    memset(containers, 0, sizeof(*containers) * (size_t)containerCount);
    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 1;
    state->world.things = things;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    seed_champion(&state->world.party.champions[0]);
}

static void seed_chest_a_chain(struct DungeonWeapon_Compat* weapons,
                               int weaponCount)
{
    int i;
    for (i = 0; i < weaponCount; ++i) {
        unsigned short thing = make_weapon_thing(WEAPON_BASE_FOR_CHEST_A + i);
        weapons[WEAPON_BASE_FOR_CHEST_A + i].type = 8; /* DAGGER (object info 31) */
        weapons[WEAPON_BASE_FOR_CHEST_A + i].next =
            (i + 1 < CHEST_A_CHAIN_COUNT)
                ? make_weapon_thing(WEAPON_BASE_FOR_CHEST_A + i + 1)
                : THING_ENDOFLIST;
        (void)thing;
    }
}

static void seed_chest_b_chain(struct DungeonWeapon_Compat* weapons,
                               int weaponCount)
{
    int i;
    for (i = 0; i < CHEST_B_CHAIN_COUNT; ++i) {
        weapons[WEAPON_BASE_FOR_CHEST_B + i].type = 2; /* SWORD (object info 24) */
        weapons[WEAPON_BASE_FOR_CHEST_B + i].next =
            (i + 1 < CHEST_B_CHAIN_COUNT)
                ? make_weapon_thing(WEAPON_BASE_FOR_CHEST_B + i + 1)
                : THING_ENDOFLIST;
        (void)weaponCount;
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

static unsigned short weapon_chain_last_visible(const struct DungeonWeapon_Compat* weapons,
                                                int weaponCount,
                                                unsigned short firstThing,
                                                int maxWalk)
{
    unsigned short thing = firstThing;
    unsigned short prev = THING_NONE;
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
        prev = thing;
        ++count;
        thing = weapons[index].next;
    }
    return prev;
}

static void test_eye_routes_open_chest_a_to_leader_hand_chest_b(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[WEAPON_BASE_FOR_CHEST_B + CHEST_B_CHAIN_COUNT];
    struct DungeonContainer_Compat containers[2];
    unsigned short chestA = make_container_thing(CHEST_A_INDEX);
    unsigned short chestB = make_container_thing(CHEST_B_INDEX);
    unsigned short firstChestAWeapon = make_weapon_thing(WEAPON_BASE_FOR_CHEST_A);
    unsigned short firstChestBWeapon = make_weapon_thing(WEAPON_BASE_FOR_CHEST_B);
    unsigned short lastVisibleChestA = make_weapon_thing(WEAPON_BASE_FOR_CHEST_A + 7);
    unsigned short lastVisibleChestB = make_weapon_thing(WEAPON_BASE_FOR_CHEST_B + 3);

    memset(weapons, 0, sizeof(weapons));
    seed_world_for_test(&state, &things, weapons,
                        (int)(sizeof(weapons) / sizeof(weapons[0])),
                        containers, 2);

    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = (int)(sizeof(weapons) / sizeof(weapons[0]));
    things.containers = containers;
    things.containerCount = 2;

    seed_chest_a_chain(weapons, things.weaponCount);
    seed_chest_b_chain(weapons, things.weaponCount);
    containers[CHEST_A_INDEX].slot = firstChestAWeapon;
    containers[CHEST_B_INDEX].slot = firstChestBWeapon;

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestA;

    /* Sanity: chest A opens via the action-hand F0333 route with
     * v1OpenChestOpenedByEye = 0 (F0333 lines 30-32 + lines 43-46 do not
     * fire when P0694_B_PressingEye is unset). */
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest A opens before eye route");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestA,
              "G0426 names chest A before eye route");
    ASSERT_EQ(state.v1OpenChestOpenedByEye, 0,
              "F0333 non-eye path leaves v1OpenChestOpenedByEye cleared");
    ASSERT_EQ(weapon_chain_count(weapons, things.weaponCount,
                                 containers[CHEST_A_INDEX].slot, 12),
              CHEST_A_CHAIN_COUNT,
              "chest A starts with nine linked weapons (8 visible + 1 hidden tail)");
    ASSERT_EQ(weapon_chain_last_visible(weapons, things.weaponCount,
                                        containers[CHEST_A_INDEX].slot, 12),
              make_weapon_thing(WEAPON_BASE_FOR_CHEST_A + CHEST_A_CHAIN_COUNT - 1),
              "chest A chain walk reaches the ninth hidden tail link");

    /* Place chest B in the leader hand. F0333 does not run yet because
     * the eye click has not happened. */
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, chestB), 1,
              "leader hand accepts chest B before eye route");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), chestB,
              "leader hand holds chest B before eye click");

    /* The C071 eye click on a leader-hand container routes through
     * PANEL.C F0352 -> F0342 -> F0333 with P0694_B_PressingEye = 1.
     * F0333 lines 35-38 first closes chest A through F0334, which
     * rewrites containers[0].slot from the visible C537..C544 chain
     * only. F0333 then opens chest B and sets v1OpenChestOpenedByEye.
     */
    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click on leader-hand chest B redraws the source chest panel");

    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestB,
              "F0333 lines 35-38 + 58-75 switch G0426 from chest A to chest B");
    ASSERT_EQ(state.v1OpenChestOpenedByEye, 1,
              "F0333 lines 43-46 set v1OpenChestOpenedByEye on eye-route open");

    /* Chest A close: F0334 rewrites containers[0].slot from the visible
     * C537..C544 chain only, dropping the ninth hidden tail link and
     * relinking the last visible item to THING_ENDOFLIST. */
    ASSERT_EQ(containers[CHEST_A_INDEX].slot, firstChestAWeapon,
              "F0334 close keeps the first visible chest A weapon as the head");
    ASSERT_EQ(weapons[WEAPON_BASE_FOR_CHEST_A + 7].next, THING_ENDOFLIST,
              "F0334 close rewrites chest A's last visible weapon to THING_ENDOFLIST");
    ASSERT_EQ(weapon_chain_count(weapons, things.weaponCount,
                                 containers[CHEST_A_INDEX].slot, 12),
              8,
              "F0334 close compacts chest A to eight visible weapons");
    ASSERT_EQ(weapon_chain_count(weapons, things.weaponCount,
                                 firstChestAWeapon, 12),
              8,
              "F0334 close drops chest A's hidden ninth tail weapon link");

    /* Chest B open: F0333 lines 58-75 populate G0425 with the first
     * eight visible chest B items, the ninth-and-later tail of chest B
     * is not visible because chest B has fewer than eight items. */
    ASSERT_EQ(containers[CHEST_B_INDEX].slot, firstChestBWeapon,
              "F0333 open does not perturb chest B's source slot chain");
    ASSERT_EQ(weapon_chain_count(weapons, things.weaponCount,
                                 containers[CHEST_B_INDEX].slot, 12),
              CHEST_B_CHAIN_COUNT,
              "F0333 open leaves chest B's full source chain reachable");
    ASSERT_EQ(weapon_chain_last_visible(weapons, things.weaponCount,
                                        containers[CHEST_B_INDEX].slot, 12),
              lastVisibleChestB,
              "F0333 open shows the last visible chest B weapon at C544");

    /* The action-hand slot still holds chest A; the eye route does not
     * move chest A out of the action hand. The action-hand icon
     * therefore must show C144 (closed container) instead of C145
     * (open container), because chest A is no longer G0426. */
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND],
              chestA,
              "eye route does not move chest A out of the action-hand slot");
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(
                  &state, CHAMPION_SLOT_ACTION_HAND),
              144,
              "closed action-hand chest A icon is C144 after eye route switch");
    ASSERT_EQ(weapons[WEAPON_BASE_FOR_CHEST_A + 6].next, lastVisibleChestA,
              "F0334 close relinks the last visible chest A pair before truncation");

    /* Leader hand still holds chest B; the eye route only changes the
     * panel state and does not consume the leader-hand object. */
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), chestB,
              "leader hand still holds chest B after eye route switch");

    /* Object-description / scroll / stats panels must be cleared by the
     * container eye branch in m11_process_v1_eye_click so the F0333
     * chest panel becomes the active inventory detail. */
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "container eye route clears object-description panel state");
    ASSERT_EQ(state.v1ObjectDescriptionThing, THING_NONE,
              "container eye route clears object-description thing");
    ASSERT_EQ(state.v1ObjectDescriptionIconIndex, -1,
              "container eye route clears object-description icon");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "container eye route clears scroll panel state");
    ASSERT_EQ(state.v1ScrollPanelThing, THING_NONE,
              "container eye route clears scroll panel thing");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 0,
              "container eye route clears champion-stats panel state");
    ASSERT_EQ(state.v1FoodWaterPanelActive, 0,
              "container eye route clears food/water panel state");
}

static void test_eye_switch_is_idempotent_when_already_open(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[WEAPON_BASE_FOR_CHEST_B + CHEST_B_CHAIN_COUNT];
    struct DungeonContainer_Compat containers[2];
    unsigned short chestA = make_container_thing(CHEST_A_INDEX);
    unsigned short chestB = make_container_thing(CHEST_B_INDEX);
    unsigned short firstChestAWeapon = make_weapon_thing(WEAPON_BASE_FOR_CHEST_A);
    unsigned short firstChestBWeapon = make_weapon_thing(WEAPON_BASE_FOR_CHEST_B);

    memset(weapons, 0, sizeof(weapons));
    seed_world_for_test(&state, &things, weapons,
                        (int)(sizeof(weapons) / sizeof(weapons[0])),
                        containers, 2);

    things.loaded = 1;
    things.weapons = weapons;
    things.weaponCount = (int)(sizeof(weapons) / sizeof(weapons[0]));
    things.containers = containers;
    things.containerCount = 2;
    seed_chest_a_chain(weapons, things.weaponCount);
    seed_chest_b_chain(weapons, things.weaponCount);
    containers[CHEST_A_INDEX].slot = firstChestAWeapon;
    containers[CHEST_B_INDEX].slot = firstChestBWeapon;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestA;
    (void)chestA;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "chest A opens before same-chest eye idempotence check");
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, chestB), 1,
              "leader hand holds chest B for the same-chest idempotence check");

    /* First eye click switches G0426 to chest B. */
    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "first eye click on chest B redraws the panel");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestB,
              "G0426 names chest B after the first eye click");
    ASSERT_EQ(state.v1OpenChestOpenedByEye, 1,
              "v1OpenChestOpenedByEye is set after the first eye click");
    ASSERT_EQ(weapon_chain_count(weapons, things.weaponCount,
                                 containers[CHEST_A_INDEX].slot, 12),
              8,
              "first eye click closed chest A and dropped the hidden tail");

    /* A second eye click on chest B (already G0426) must hit the
     * CHEST.C F0333 lines 30-32 early-return branch: G0426 already names
     * the same chest, so the panel is unchanged and the v1OpenChestOpenedByEye
     * flag is left at its current value (1 from the first click). */
    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "second eye click on chest B is a redraw (no-op open path)");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestB,
              "G0426 still names chest B after the second eye click");
    ASSERT_EQ(state.v1OpenChestOpenedByEye, 1,
              "F0333 lines 30-32 short-circuit keeps v1OpenChestOpenedByEye set");
    ASSERT_EQ(containers[CHEST_B_INDEX].slot, firstChestBWeapon,
              "second eye click does not perturb chest B's source slot chain");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), chestB,
              "leader hand still holds chest B after the second eye click");
}

int main(void)
{
    printf("=== DM1 V1 Chest Eye Open To Action Hand Switch Runtime Gate ===\n");
    printf("ReDMCSB: PANEL.C F0352 2111-2159, F0342 1119-1135, "
           "CHEST.C F0333 30-75, F0334 117-132, "
           "COMMAND.C F0380 2316-2320\n\n");

    test_eye_routes_open_chest_a_to_leader_hand_chest_b();
    test_eye_switch_is_idempotent_when_already_open();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
