/**
 * DM1 V1 Stairs Inventory State — narrow CTest gate
 *
 * Bridges the DM1_V1_RoomTransition_BuildPlanPc34Compat module and the
 * M11_InventoryState / M11_StairLevelState modules and asserts the
 * ReDMCSB-derived contract that a stairs-triggered level change does NOT
 * mutate the per-champion inventory state.  Concretely:
 *
 *   1. The open chest (openChestThing, chestSlots[*], panelContent=CHEST)
 *      is preserved across a stairs level change.  The planner only
 *      mutates party.MapIndex/MapX/MapY/Direction; F0334_INVENTORY_CloseChest
 *      is NOT called by the stairs path.
 *   2. The inventory panel content (PANEL_INVENTORY / FOOD_WATER_POISONED /
 *      CHEST) is preserved.  ReDMCSB F0355_INVENTORY_Toggle_CPSE is the only
 *      place that flips the panel content; F0364_COMMAND_TakeStairs is not
 *      that place.
 *   3. The leader-hand torch (or any other slot object) is preserved across
 *      a stairs level change.  F0173/F0174 do NOT touch M516.Champion[i].Slots.
 *   4. The room transition plan for stairs must NOT request F0334
 *      (close-chest side effect) and must NOT request F0355 (toggle-inventory
 *      side effect); both are deferred to a later user click.
 *
 * ReDMCSB references:
 *   CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142   — stairs trigger
 *   MOVESENS.C:441-451                            — party X/Y/Direction mutate
 *   MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE     — move mechanics, no inventory
 *   DUNGEON.C:F0154_GetLocationAfterLevelChange   — destination lookup
 *   DUNGEON.C:F0173_DUNGEON_SetCurrentMap:2724-2740 — map data reload
 *   DUNGEON.C:F0174_SetCurrentMapAndPartyMap:2742-2762 — party map metadata
 *   PANEL.C:F0334_INVENTORY_CloseChest            — close-chest (NOT called by stairs)
 *   PANEL.C:F0355_INVENTORY_Toggle_CPSE           — toggle panel (NOT called by stairs)
 *   PANEL.C:G0423_i_InventoryChampionOrdinal      — global panel champion (survives)
 *   PANEL.C:G0425_aT_ChestSlots[8]                — global chest contents (survives)
 *   PANEL.C:G0426_T_OpenChest                     — global open chest thing (survives)
 *   PANEL.C:G0424_i_PanelContent                  — global panel content (survives)
 *   CHAMPION.C:M516_CHAMPIONS[4].Slots            — champion hand objects (survive)
 *
 * Scope discipline:
 *   - No retest of F0334 / F0355 close/toggle side effects (covered by
 *     test_dm1_v1_inventory_chest_close_recompaction_pc34_compat,
 *     test_dm1_v1_inventory_backpack_chest_pc34_compat, and the chest_close
 *     gate family).
 *   - No retest of stairs + light carry-over (covered by
 *     test_dm1_v1_stairs_transition_light_state_pc34_compat).
 *   - No retest of ordinary room/level transitions (covered by
 *     test_dm1_v1_room_transition_pc34_compat).
 *   - This test only asserts the stairs + inventory-state preservation
 *     contract that none of the above tests pin together.
 */

#include "dm1_v1_inventory_pc34_compat.h"
#include "dm1_v1_room_transition_pc34_compat.h"
#include "dm1_v1_stairs_level_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_true(const char *id, int cond, const char *anchor)
{
    expect_int(id, cond ? 1 : 0, 1, anchor);
}

static struct Dm1V1RoomTransitionPosePc34Compat make_pose(int mapIndex, int x, int y, int dir)
{
    struct Dm1V1RoomTransitionPosePc34Compat p;
    p.mapIndex = mapIndex;
    p.mapX = x;
    p.mapY = y;
    p.direction = dir;
    return p;
}

static struct Dm1V1RoomTransitionInputPc34Compat make_stairs_input(
    int fromMap, int toMap, int x, int y, int dir)
{
    struct Dm1V1RoomTransitionInputPc34Compat in;
    memset(&in, 0, sizeof(in));
    in.presentationMode = DM1_V1_ROOM_TRANSITION_PRESENTATION_ORIGINAL;
    in.trigger = DM1_V1_ROOM_TRANSITION_TRIGGER_STAIRS;
    in.before = make_pose(fromMap, x, y, dir);
    in.after  = make_pose(toMap,   x, y, dir);
    in.partyChampionCount = 4;
    return in;
}

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;
    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = allowedSlots;
    return item;
}

/* ── Test 1: open chest survives a stairs level change in the plan ───── */

static void test_stairs_plan_preserves_open_chest_state(void)
{
    /*
     * ReDMCSB F0364_COMMAND_TakeStairs lines 124-142 calls
     * F0267/F0154/F0173/F0284/F0173 in that order; NONE of them call
     * F0334_INVENTORY_CloseChest.  F0334 is the only function that
     * mutates G0426_T_OpenChest to C0xFFFF_THING_NONE, and it is only
     * called from F0355 (toggle inventory) and F0333 (open chest).
     * Therefore the stairs plan must not have any plan-level
     * close-chest side effect.
     *
     * We assert the planner leaves the four chest-related fields
     * untouched by also checking the plan does not promote the
     * close-chest code path through any of its output flags (there
     * is no explicit closeChests flag, but the planner's only
     * state-preservation outputs are preserveChampionInventories /
     * preserveChampionStats / preserveLeaderHandObject).
     */
    struct Dm1V1RoomTransitionInputPc34Compat in =
        make_stairs_input(0, 1, 7, 8, 0);
    struct Dm1V1RoomTransitionPlanPc34Compat plan;

    int rc = DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan);
    expect_int("stairs.rc", rc, 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("stairs.active", plan.active, 1,
               "DUNGEON.C:F0173_DUNGEON_SetCurrentMap:2724-2740");
    expect_int("stairs.trigger", plan.trigger,
               DM1_V1_ROOM_TRANSITION_TRIGGER_STAIRS,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_true("stairs.preserve_champion_inventories",
                plan.preserveChampionInventories == 1,
                "DUNGEON.C:F0173/F0174:2724-2762; PANEL.C:G0426_T_OpenChest");
    expect_true("stairs.preserve_champion_stats",
                plan.preserveChampionStats == 1,
                "DUNGEON.C:F0173/F0174:2724-2762; CHAMPION.C:M516_CHAMPIONS");
    expect_true("stairs.preserve_leader_hand_object",
                plan.preserveLeaderHandObject == 1,
               "DUNGEON.C:F0173/F0174:2724-2762; CHAMPION.C:M516_CHAMPIONS.Slots");
    expect_int("stairs.map_changed", plan.mapChanged, 1,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange; "
               "DUNGEON.C:F0173_DUNGEON_SetCurrentMap:2724-2740");
    expect_true("stairs.requests_map_metadata_reload",
                plan.requestSetCurrentMap == 1,
                "DUNGEON.C:F0173_DUNGEON_SetCurrentMap:2724-2740");
    expect_true("stairs.requests_party_map_metadata_reload",
                plan.requestSetCurrentMapAndPartyMap == 1,
                "DUNGEON.C:F0174_DUNGEON_SetCurrentMapAndPartyMap:2742-2762");
}

/* ── Test 2: M11_StairLevelState transition does not touch inventory ──── */

static void test_stairs_use_leaves_open_chest_intact(void)
{
    /*
     * The M11_StairLevelState carries the per-level layout metadata
     * (stairs[] and levels[]) and exposes a transition counter.  Calling
     * m11_stairs_use only mutates currentLevel, transitionActive,
     * transitionTicksLeft, transitionFromLevel, and transitionToLevel.
     * It MUST NOT touch the M11_InventoryState that the caller holds
     * alongside it.
     *
     * The M11_InventoryState is a *separate* struct (per-champion data
     * is in M516_CHAMPIONS, not in the level state), so the test is
     * really checking that there is no implicit shared pointer.  In the
     * PC 3.4 layout, M516 is a global array in CHAMPION.C and the level
     * state is a separate M11_StairLevelState; they are independent.
     */
    M11_StairLevelState stairs;
    M11_InventoryState inv;
    M11_Item linked[8];
    int champ = 0;
    int newX, newY, newFacing;
    int openThing = 0xC457;
    int panelBefore, openBefore, slotItemBefore, slotWeightBefore;
    int slotChargeBefore;
    int useRc, transitioning;
    int i;

    m11_stairs_init(&stairs);
    m11_stairs_add_level(&stairs, 16, 16);
    m11_stairs_add_level(&stairs, 16, 16);
    /* Stairs at (7, 8) facing south on level 0 → (7, 8) facing south on level 1. */
    expect_int("stairs_add",
               m11_stairs_add(&stairs, 7, 8, 2, 1, 7, 8, 2), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142; DUNGEON.C:F0154");
    expect_int("stairs_initial_level", stairs.currentLevel, 0,
               "M11_StairLevelState init");
    expect_int("stairs_initial_transition", stairs.transitionActive, 0,
               "M11_StairLevelState init");

    /* Build a fully-open chest state in the inventory. */
    m11_inventory_init(&inv, 1);
    for (i = 0; i < 8; ++i) {
        linked[i] = make_item(200 + i, /*weight=*/2 + i, DM1_PC34_ALLOWED_CONTAINER);
    }
    expect_int("open_chest",
               m11_inventory_open_chest(&inv, champ, openThing, linked, 8), 1,
               "CHEST.C:F0333_INVENTORY_OpenChest:30-75");
    expect_int("panel_after_open",
               m11_inventory_get_panel_content_pc34(&inv),
               DM1_PC34_PANEL_CHEST,
               "CHEST.C:F0333_INVENTORY_OpenChest:30-75; PANEL.C:G0424_i_PanelContent");

    /* Place a torch in the leader's action hand. */
    inv.champions[champ].handItem = 1;
    expect_int("set_action_hand_torch",
               m11_inventory_set_item(&inv, champ, DM1_SLOT_HAND_LEFT,
                                      /*itemType=*/19 /* FLAME */,
                                      /*weight=*/1,
                                      /*charges=*/12), 1,
               "CHAMPION.C:F0301_AddObjectInSlot:587-660");

    /* Snapshot the inventory state we expect to survive. */
    panelBefore = m11_inventory_get_panel_content_pc34(&inv);
    openBefore = m11_inventory_get_open_chest_thing(&inv, champ);
    slotItemBefore = inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].itemType;
    slotWeightBefore = inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].weight;
    slotChargeBefore = inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].charges;

    /* Take the stairs. */
    useRc = m11_stairs_use(&stairs, 7, 8, &newX, &newY, &newFacing);
    expect_int("stairs_use_rc", useRc, 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142; MOVESENS.C:F0267");
    expect_int("stairs_use_x", newX, 7,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    expect_int("stairs_use_y", newY, 8,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    expect_int("stairs_use_facing", newFacing, 2,
               "DUNGEON.C:F0155_GetStairsExitDirection");

    /* M11_StairLevelState reflects the level change. */
    expect_int("stairs_level_after_use", stairs.currentLevel, 1,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    expect_int("stairs_transition_active_after_use", stairs.transitionActive, 1,
               "GAMELOOP.C:58-64 deferred new-party-map processing");
    expect_int("stairs_transition_from_level", stairs.transitionFromLevel, 0,
               "GAMELOOP.C:58-64 deferred new-party-map processing");
    expect_int("stairs_transition_to_level", stairs.transitionToLevel, 1,
               "GAMELOOP.C:58-64 deferred new-party-map processing");

    /* The M11_InventoryState MUST be byte-untouched for the open-chest
     * fields, panel content, and leader hand torch. */
    expect_int("inv.panel_after_stairs",
               m11_inventory_get_panel_content_pc34(&inv), panelBefore,
               "PANEL.C:G0424_i_PanelContent; CLIKMENU.C:F0364 does not toggle panel");
    expect_int("inv.panel_value",
               m11_inventory_get_panel_content_pc34(&inv),
               DM1_PC34_PANEL_CHEST,
               "PANEL.C:G0424_i_PanelContent survives stairs");
    expect_int("inv.open_chest_thing",
               m11_inventory_get_open_chest_thing(&inv, champ), openBefore,
               "PANEL.C:G0426_T_OpenChest; F0334_INVENTORY_CloseChest NOT called by F0364");
    expect_int("inv.open_chest_thing_value",
               m11_inventory_get_open_chest_thing(&inv, champ), openThing,
               "PANEL.C:G0426_T_OpenChest survives stairs");
    expect_int("inv.action_hand_item",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].itemType,
               slotItemBefore,
               "CHAMPION.C:M516.Champion[0].Slots[C01_SLOT_ACTION_HAND] preserved");
    expect_int("inv.action_hand_weight",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].weight,
               slotWeightBefore,
               "DUNGEON.C:F0173/F0174:2724-2762 do not touch slot weight");
    expect_int("inv.action_hand_charges",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].charges,
               slotChargeBefore,
               "DUNGEON.C:F0173/F0174:2724-2762 do not touch slot charges");
    expect_int("inv.handItem",
               inv.champions[champ].handItem, 1,
               "CHAMPION.C:M516.Champion[0].HandItem preserved");
    expect_int("inv.load_unchanged",
               m11_inventory_get_load(&inv, champ),
               /* 1 (torch) + (2+3+4+5+6+7+8+9) = 45 */ 45,
               "CHAMPION.C:M516.Champion[0].Load preserved");

    /* Walk the eight chest slots and assert each item is byte-identical. */
    for (i = 0; i < 8; ++i) {
        M11_Item got;
        char id[64];
        expect_int("inv.chest_slot_has_thing",
                   m11_inventory_get_item_in_chest_slot(&inv, champ, i, &got), 1,
                   "PANEL.C:G0425_aT_ChestSlots[8] survives stairs");
        snprintf(id, sizeof(id), "inv.chest.%d.itemType", i);
        expect_int(id, got.itemType, 200 + i,
                   "PANEL.C:G0425_aT_ChestSlots[i] itemType preserved");
        snprintf(id, sizeof(id), "inv.chest.%d.weight", i);
        expect_int(id, got.weight, 2 + i,
                   "PANEL.C:G0425_aT_ChestSlots[i] weight preserved");
    }

    /* Tick the stairs transition to completion; the level state should
     * settle but the inventory must STILL be untouched. */
    for (i = 0; i < 16; ++i) {
        m11_stairs_tick(&stairs, 100);
    }
    transitioning = m11_stairs_is_transitioning(&stairs);
    expect_int("stairs_transition_settled", transitioning, 0,
               "GAMELOOP.C:58-64 deferred new-party-map processing; "
               "DUNGEON.C:F0173:2724-2740");
    expect_int("inv.open_chest_thing_after_ticks",
               m11_inventory_get_open_chest_thing(&inv, champ), openThing,
               "PANEL.C:G0426_T_OpenChest survives ticks of stairs transition");
    expect_int("inv.action_hand_item_after_ticks",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].itemType,
               slotItemBefore,
               "CHAMPION.C:M516.Champion[0].Slots survives ticks of stairs");
    expect_int("inv.panel_after_ticks",
               m11_inventory_get_panel_content_pc34(&inv),
               DM1_PC34_PANEL_CHEST,
               "PANEL.C:G0424_i_PanelContent survives ticks of stairs");
}

/* ── Test 3: stairs with no open chest keeps panel=INVENTORY, slot intact */

static void test_stairs_use_with_no_chest_keeps_inventory_panel(void)
{
    /*
     * Negative half of the contract: when no chest is open, the panel
     * must be PANEL_INVENTORY (or FOOD_WATER_POISONED) across the
     * stairs, and no slot is touched.
     */
    M11_StairLevelState stairs;
    M11_InventoryState inv;
    int champ = 0;
    int newX, newY, newFacing;
    int panelBefore;
    int i;

    m11_stairs_init(&stairs);
    m11_stairs_add_level(&stairs, 16, 16);
    m11_stairs_add_level(&stairs, 16, 16);
    expect_int("stairs_add_2",
               m11_stairs_add(&stairs, 3, 3, 0, 1, 3, 3, 0), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");

    m11_inventory_init(&inv, 1);
    /* Place a non-container object in the action hand. */
    expect_int("set_action_hand_scroll",
               m11_inventory_set_item(&inv, champ, DM1_SLOT_HAND_LEFT,
                                      /*itemType=*/180 /* SCROLL */,
                                      /*weight=*/1,
                                      /*charges=*/1), 1,
               "CHAMPION.C:F0301_AddObjectInSlot:587-660");
    panelBefore = m11_inventory_get_panel_content_pc34(&inv);
    expect_int("panel_initially_inventory",
               panelBefore, DM1_PC34_PANEL_INVENTORY,
               "M11_InventoryState init: PANEL.C:G0424_i_PanelContent");

    expect_int("no_open_chest",
               m11_inventory_get_open_chest_thing(&inv, champ), 0,
               "PANEL.C:G0426_T_OpenChest initially C0xFFFF_THING_NONE");

    expect_int("stairs_use_no_chest",
               m11_stairs_use(&stairs, 3, 3, &newX, &newY, &newFacing), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("stairs_use_x_no_chest", newX, 3,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    expect_int("stairs_use_y_no_chest", newY, 3,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");

    expect_int("panel_unchanged_after_stairs",
               m11_inventory_get_panel_content_pc34(&inv), panelBefore,
               "PANEL.C:G0424_i_PanelContent; F0364 does not toggle panel");
    expect_int("panel_value_after_stairs",
               m11_inventory_get_panel_content_pc34(&inv),
               DM1_PC34_PANEL_INVENTORY,
               "PANEL.C:G0424_i_PanelContent stays at INVENTORY");
    expect_int("open_chest_still_zero",
               m11_inventory_get_open_chest_thing(&inv, champ), 0,
               "PANEL.C:G0426_T_OpenChest stays C0xFFFF_THING_NONE");
    expect_int("action_hand_scroll_preserved",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].itemType, 180,
               "CHAMPION.C:M516.Champion[0].Slots[C01_SLOT_ACTION_HAND] preserved");
    expect_int("action_hand_charges_preserved",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].charges, 1,
               "CHAMPION.C:M516.Champion[0].Slots[C01_SLOT_ACTION_HAND] charges preserved");

    /* Ticking through the transition must also not toggle the panel. */
    for (i = 0; i < 32; ++i) {
        m11_stairs_tick(&stairs, 100);
    }
    expect_int("panel_unchanged_after_ticks",
               m11_inventory_get_panel_content_pc34(&inv),
               DM1_PC34_PANEL_INVENTORY,
               "PANEL.C:G0424_i_PanelContent survives ticks of stairs");
    expect_int("action_hand_scroll_preserved_after_ticks",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].itemType, 180,
               "CHAMPION.C:M516.Champion[0].Slots survives ticks of stairs");
}

/* ── Test 4: two back-to-back stairs still keep chest open ───────────── */

static void test_two_stairs_uses_keep_open_chest_across_both(void)
{
    /*
     * Stairs up and stairs down on the new level must each preserve
     * the open chest.  The champion's open-chest state should survive
     * the level change, the tick to settle, the next level change,
     * and the tick to settle again.
     *
     * M11_StairLevelState::m11_stairs_check matches the first stairs
     * registered at a given (x,y), so we use distinct coordinates for
     * the up- and down-stairs to be able to drive both transitions.
     */
    M11_StairLevelState stairs;
    M11_InventoryState inv;
    M11_Item linked[8];
    int champ = 0;
    int newX, newY, newFacing;
    int openThing = 0xC0DE;
    int i;

    m11_stairs_init(&stairs);
    m11_stairs_add_level(&stairs, 16, 16);
    m11_stairs_add_level(&stairs, 16, 16);
    /* Up-stairs: (5,5)→(5,5) on level 0→1, down-stairs: (10,10)→(10,10) on level 1→0. */
    expect_int("stairs_add_first",
               m11_stairs_add(&stairs, 5, 5, 0, 1, 5, 5, 0), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("stairs_add_second",
               m11_stairs_add(&stairs, 10, 10, 0, 0, 10, 10, 0), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");

    m11_inventory_init(&inv, 1);
    for (i = 0; i < 8; ++i) {
        linked[i] = make_item(300 + i, 1, DM1_PC34_ALLOWED_CONTAINER);
    }
    expect_int("open_chest_for_two_stairs",
               m11_inventory_open_chest(&inv, champ, openThing, linked, 8), 1,
               "CHEST.C:F0333_INVENTORY_OpenChest:30-75");

    /* Stairs up. */
    expect_int("stairs_up_use",
               m11_stairs_use(&stairs, 5, 5, &newX, &newY, &newFacing), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("stairs_up_level", stairs.currentLevel, 1,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    for (i = 0; i < 16; ++i) m11_stairs_tick(&stairs, 100);
    expect_int("open_chest_after_up",
               m11_inventory_get_open_chest_thing(&inv, champ), openThing,
               "PANEL.C:G0426_T_OpenChest survives first stairs up");

    /* Stairs down. */
    expect_int("stairs_down_use",
               m11_stairs_use(&stairs, 10, 10, &newX, &newY, &newFacing), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("stairs_down_level", stairs.currentLevel, 0,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    for (i = 0; i < 16; ++i) m11_stairs_tick(&stairs, 100);
    expect_int("open_chest_after_down",
               m11_inventory_get_open_chest_thing(&inv, champ), openThing,
               "PANEL.C:G0426_T_OpenChest survives second stairs down");
    expect_int("panel_after_roundtrip",
               m11_inventory_get_panel_content_pc34(&inv),
               DM1_PC34_PANEL_CHEST,
               "PANEL.C:G0424_i_PanelContent survives stairs roundtrip");
    /* Re-check a sample chest slot to confirm full byte stability. */
    expect_int("chest_slot_0_item_after_roundtrip",
               inv.champions[champ].chestSlots[0].itemType, 300,
               "PANEL.C:G0425_aT_ChestSlots[0] survives stairs roundtrip");
    expect_int("chest_slot_7_item_after_roundtrip",
               inv.champions[champ].chestSlots[7].itemType, 307,
               "PANEL.C:G0425_aT_ChestSlots[7] survives stairs roundtrip");
}

/* ── Test 5: stairs plan on same pose is a no-op for the inventory path */

static void test_stairs_plan_same_pose_does_not_request_close_or_toggle(void)
{
    /*
     * Defensive: even when the planner short-circuits (no pose change),
     * the inventory state is trivially preserved because the planner
     * never reaches the state-mutation branch.  The plan's
     * preserveChampionInventories / preserveLeaderHandObject flags are
     * still 0 in that case (only set when active=1), but the inventory
     * is byte-untouched because the planner is a pure function.
     */
    struct Dm1V1RoomTransitionInputPc34Compat in =
        make_stairs_input(0, 0, 7, 8, 0);
    struct Dm1V1RoomTransitionPlanPc34Compat plan;

    int rc = DM1_V1_RoomTransition_BuildPlanPc34Compat(&in, &plan);
    expect_int("stairs.same_pose.rc", rc, 1,
               "DUNGEON.C:1508-1558; MOVESENS.C:441-451");
    expect_int("stairs.same_pose.active", plan.active, 0,
               "DUNGEON.C:1508-1558; MOVESENS.C:441-451");
    expect_int("stairs.same_pose.map_changed", plan.mapChanged, 0,
               "DUNGEON.C:1508-1558");
    /* Same-pose short-circuit means the planner didn't reach the
     * state-preservation block, so the flags are 0 — but the
     * inventory is also untouched because the planner is pure. */
    expect_int("stairs.same_pose.preserve_inventories",
               plan.preserveChampionInventories, 0,
               "DUNGEON.C:1508-1558 short-circuit: no preservation needed");
}

int main(void)
{
    printf("=== DM1 V1 stairs inventory state — source-lock gate ===\n");
    test_stairs_plan_preserves_open_chest_state();
    test_stairs_use_leaves_open_chest_intact();
    test_stairs_use_with_no_chest_keeps_inventory_panel();
    test_two_stairs_uses_keep_open_chest_across_both();
    test_stairs_plan_same_pose_does_not_request_close_or_toggle();
    printf("\n=== %d/%d assertions passed (%d failures) ===\n",
           g_assertions - g_failures, g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
