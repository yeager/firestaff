/**
 * DM1 V1 Stairs Inventory State Gate — narrow CTest gate
 *
 * Bridges the M11_StairLevelState I1/I2/I3 invariants and the
 * M11_InventoryState to lock down the previously-uncovered boundary
 * that the "stairs while already transitioning" path is a true no-op:
 *
 *   1. The I1 rejection path (m11_stairs_use while transitionActive==1)
 *      returns 0 AND does not mutate M11_StairLevelState, does not write
 *      *newX / *newY / *newFacing, AND does not touch the caller's
 *      M11_InventoryState (open chest, panel content, leader hand).
 *   2. The mid-tick window between m11_stairs_use returning and
 *      m11_stairs_tick consuming the full transitionTicksLeft is a
 *      "datafreeze" window: every M11_InventoryState field we care
 *      about stays byte-identical until the transition settles.
 *   3. After the rejected second call, the original transition still
 *      settles to currentLevel=1, transitionActive=0, and a follow-up
 *      m11_stairs_use on a valid stair still works.
 *
 * ReDMCSB references:
 *   CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142 — stairs trigger
 *   MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE    — move mechanics, no inventory
 *   DUNGEON.C:F0173_DUNGEON_SetCurrentMap:2724-2740 — map data reload
 *   DUNGEON.C:F0174_SetCurrentMapAndPartyMap:2742-2762 — party map metadata
 *   PANEL.C:G0426_T_OpenChest                     — global open chest thing
 *   PANEL.C:G0424_i_PanelContent                  — global panel content
 *   PANEL.C:G0425_aT_ChestSlots[8]                — global chest contents
 *   CHAMPION.C:M516_CHAMPIONS[4].Slots            — champion hand objects
 *   GAMELOOP.C:58-64                              — deferred new-party-map proc
 *
 * Scope discipline:
 *   - No retest of stairs + open chest across a settled transition
 *     (covered by test_dm1_v1_stairs_inventory_state_pc34_compat).
 *   - No retest of stairs + light state (covered by
 *     test_dm1_v1_stairs_transition_light_state_pc34_compat).
 *   - No retest of the planner's preserve_* flags (covered by
 *     test_dm1_v1_stairs_inventory_state_pc34_compat Test 1).
 *   - No retest of M11_StairLevelState invariants I2/I3/I4 in isolation
 *     (covered by test_dm1_v1_stairs_inventory_state_pc34_compat).
 *   - This test only asserts the I1 rejection path against an open
 *     chest, the mid-tick inventory freeze, and the rejected-then-
 *     follow-up stairs path.  None of those rows are pinned by any
 *     existing CTest in this lane.
 */

#include "dm1_v1_inventory_pc34_compat.h"
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

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;
    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = allowedSlots;
    return item;
}

/* ── Test 1: I1 rejection path leaves level state and inventory untouched */

static void test_stairs_use_during_transition_is_a_pure_no_op(void)
{
    /*
     * ReDMCSB src/dm1/dm1_v1_stairs_level_pc34_compat.c invariant I1:
     * "reject any stair step while a transition is already in flight."
     * The function returns 0 *before* touching transitionFromLevel,
     * transitionToLevel, currentLevel, transitionTicksLeft, or
     * transitionActive, and *before* writing *newX / *newY / *newFacing.
     *
     * The M11_InventoryState lives in CHAMPION.C M516_CHAMPIONS and is
     * independent of M11_StairLevelState, so a rejected call must not
     * mutate either the level state or any chest / hand / load state.
     */
    M11_StairLevelState stairs;
    M11_InventoryState inv;
    M11_Item linked[8];
    int champ = 0;
    int newX, newY, newFacing;
    int rc, i;
    int openThing = 0xA53A;

    m11_stairs_init(&stairs);
    m11_stairs_add_level(&stairs, 16, 16);
    m11_stairs_add_level(&stairs, 16, 16);
    /* Two stairs: (4,4)→level 1 + (8,8)→level 2 (sentinel for the rejected call). */
    expect_int("add_stairs_first",
               m11_stairs_add(&stairs, 4, 4, 2, 1, 4, 4, 2), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("add_stairs_second",
               m11_stairs_add(&stairs, 8, 8, 2, 2, 8, 8, 2), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");

    /* Build a fully-open chest state. */
    m11_inventory_init(&inv, 1);
    for (i = 0; i < 8; ++i) {
        linked[i] = make_item(400 + i, /*weight=*/3 + i, DM1_PC34_ALLOWED_CONTAINER);
    }
    expect_int("open_chest_i1",
               m11_inventory_open_chest(&inv, champ, openThing, linked, 8), 1,
               "CHEST.C:F0333_INVENTORY_OpenChest:30-75");
    expect_int("set_action_hand_torch_i1",
               m11_inventory_set_item(&inv, champ, DM1_SLOT_HAND_LEFT,
                                      /*itemType=*/19 /* FLAME */,
                                      /*weight=*/1,
                                      /*charges=*/12), 1,
               "CHAMPION.C:F0301_AddObjectInSlot:587-660");

    /* Take the stairs so a transition is pending. */
    newX = newY = newFacing = -1;
    expect_int("first_stairs_use_rc",
               m11_stairs_use(&stairs, 4, 4, &newX, &newY, &newFacing), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142; "
               "MOVESENS.C:F0267");
    expect_int("first_stairs_x", newX, 4,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    expect_int("first_stairs_y", newY, 4,
               "DUNGEON.C:F0154_GetLocationAfterLevelChange");
    expect_int("first_stairs_facing", newFacing, 2,
               "DUNGEON.C:F0155_GetStairsExitDirection");
    expect_int("transition_active_after_first_use",
               stairs.transitionActive, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:91 (I2 IRED last-write)");
    expect_int("transition_from_level_after_first_use",
               stairs.transitionFromLevel, 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:84 (I2 IRED first-write)");
    expect_int("transition_to_level_after_first_use",
               stairs.transitionToLevel, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:85 (I2 IRED second-write)");
    expect_int("current_level_after_first_use", stairs.currentLevel, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:86 (I2 IRED third-write)");
    expect_int("transition_ticks_left_after_first_use",
               stairs.transitionTicksLeft, 500,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:87 nominal 500ms");

    /* Snapshot the inventory before the rejected call. */
    const int panelBefore      = m11_inventory_get_panel_content_pc34(&inv);
    const int openBefore       = m11_inventory_get_open_chest_thing(&inv, champ);
    const int loadBefore       = m11_inventory_get_load(&inv, champ);
    const int itemTypeBefore   = inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].itemType;
    const int weightBefore     = inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].weight;
    const int chargesBefore    = inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].charges;
    const int handItemBefore   = inv.champions[champ].handItem;
    const int chestItem0Before = inv.champions[champ].chestSlots[0].itemType;
    const int chestItem7Before = inv.champions[champ].chestSlots[7].itemType;
    const int chestWeight0Before = inv.champions[champ].chestSlots[0].weight;

    /* Snapshot the level state before the rejected call. */
    const int transitionActiveBefore    = stairs.transitionActive;
    const int transitionFromLevelBefore = stairs.transitionFromLevel;
    const int transitionToLevelBefore   = stairs.transitionToLevel;
    const int currentLevelBefore        = stairs.currentLevel;
    const int transitionTicksLeftBefore = stairs.transitionTicksLeft;

    /* Attempt to take the second stairs WHILE the first is still pending.
     * Pre-seed *newX / *newY / *newFacing to sentinel values so we can
     * prove the rejected call does not write to them either. */
    newX = -77;
    newY = -77;
    newFacing = -77;
    rc = m11_stairs_use(&stairs, 8, 8, &newX, &newY, &newFacing);
    expect_int("i1_rejected_rc", rc, 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 rejection)");
    /* ReDMCSB says the rejection path returns 0 BEFORE writing newX/Y/F,
     * so the sentinel values must survive verbatim. */
    expect_int("i1_rejected_x_unchanged", newX, -77,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65-67 (rejection no-write)");
    expect_int("i1_rejected_y_unchanged", newY, -77,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65-67 (rejection no-write)");
    expect_int("i1_rejected_facing_unchanged", newFacing, -77,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65-67 (rejection no-write)");

    /* M11_StairLevelState is byte-identical to the snapshot. */
    expect_int("i1_transition_active_unchanged",
               stairs.transitionActive, transitionActiveBefore,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 no-mutation)");
    expect_int("i1_transition_from_level_unchanged",
               stairs.transitionFromLevel, transitionFromLevelBefore,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 no-mutation)");
    expect_int("i1_transition_to_level_unchanged",
               stairs.transitionToLevel, transitionToLevelBefore,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 no-mutation)");
    expect_int("i1_current_level_unchanged",
               stairs.currentLevel, currentLevelBefore,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 no-mutation)");
    expect_int("i1_transition_ticks_left_unchanged",
               stairs.transitionTicksLeft, transitionTicksLeftBefore,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 no-mutation)");

    /* M11_InventoryState is byte-identical to the snapshot. */
    expect_int("i1_panel_unchanged",
               m11_inventory_get_panel_content_pc34(&inv), panelBefore,
               "PANEL.C:G0424_i_PanelContent; I1 rejection never invokes F0355");
    expect_int("i1_open_chest_unchanged",
               m11_inventory_get_open_chest_thing(&inv, champ), openBefore,
               "PANEL.C:G0426_T_OpenChest; I1 rejection never invokes F0334");
    expect_int("i1_load_unchanged",
               m11_inventory_get_load(&inv, champ), loadBefore,
               "CHAMPION.C:M516.Champion[0].Load preserved");
    expect_int("i1_action_hand_item_unchanged",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].itemType,
               itemTypeBefore,
               "CHAMPION.C:M516.Champion[0].Slots[C01_SLOT_ACTION_HAND]");
    expect_int("i1_action_hand_weight_unchanged",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].weight,
               weightBefore,
               "CHAMPION.C:M516.Champion[0].Slots[C01_SLOT_ACTION_HAND] weight");
    expect_int("i1_action_hand_charges_unchanged",
               inv.champions[champ].slots[DM1_SLOT_HAND_LEFT].charges,
               chargesBefore,
               "CHAMPION.C:M516.Champion[0].Slots[C01_SLOT_ACTION_HAND] charges");
    expect_int("i1_hand_item_unchanged",
               inv.champions[champ].handItem, handItemBefore,
               "CHAMPION.C:M516.Champion[0].HandItem preserved");
    expect_int("i1_chest_slot_0_item_unchanged",
               inv.champions[champ].chestSlots[0].itemType, chestItem0Before,
               "PANEL.C:G0425_aT_ChestSlots[0] preserved");
    expect_int("i1_chest_slot_0_weight_unchanged",
               inv.champions[champ].chestSlots[0].weight, chestWeight0Before,
               "PANEL.C:G0425_aT_ChestSlots[0] weight preserved");
    expect_int("i1_chest_slot_7_item_unchanged",
               inv.champions[champ].chestSlots[7].itemType, chestItem7Before,
               "PANEL.C:G0425_aT_ChestSlots[7] preserved");
}

/* ── Test 2: inventory is a datafreeze during the pending transition ─── */

static void test_inventory_is_datafreeze_during_pending_transition(void)
{
    /*
     * Between m11_stairs_use returning (transitionActive=1,
     * transitionTicksLeft=500) and m11_stairs_tick consuming the full
     * 500 ms, the transition is "pending".  During this window the
     * M11_InventoryState must remain byte-identical to the pre-use
     * snapshot — neither the level state ticks (I4) nor any future
     * stairs call (I1) should reach into M516_CHAMPIONS or the
     * chest-slot / open-chest-thing globals.
     *
     * The existing test_dm1_v1_stairs_inventory_state_pc34_compat
     * tests settle the transition to completion; it does NOT pin the
     * mid-tick (still-pending) inventory freeze.
     */
    M11_StairLevelState stairs;
    M11_InventoryState inv;
    M11_Item linked[8];
    int champ = 0;
    int newX, newY, newFacing;
    int i, transitioning;

    m11_stairs_init(&stairs);
    m11_stairs_add_level(&stairs, 16, 16);
    m11_stairs_add_level(&stairs, 16, 16);
    m11_stairs_add_level(&stairs, 16, 16);
    /* (6,6) → level 1; (12,12) → level 2; (3,3) → level 0 (return stair). */
    expect_int("datafreeze_add_up1",
               m11_stairs_add(&stairs, 6, 6, 0, 1, 6, 6, 0), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("datafreeze_add_up2",
               m11_stairs_add(&stairs, 12, 12, 0, 2, 12, 12, 0), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("datafreeze_add_return",
               m11_stairs_add(&stairs, 3, 3, 0, 0, 3, 3, 0), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");

    m11_inventory_init(&inv, 1);
    for (i = 0; i < 8; ++i) {
        linked[i] = make_item(500 + i, /*weight=*/2 + i, DM1_PC34_ALLOWED_CONTAINER);
    }
    expect_int("datafreeze_open_chest",
               m11_inventory_open_chest(&inv, champ, 0xBEEF, linked, 8), 1,
               "CHEST.C:F0333_INVENTORY_OpenChest:30-75");
    expect_int("datafreeze_set_torch",
               m11_inventory_set_item(&inv, champ, DM1_SLOT_HAND_LEFT,
                                      /*itemType=*/19 /* FLAME */,
                                      /*weight=*/1,
                                      /*charges=*/10), 1,
               "CHAMPION.C:F0301_AddObjectInSlot:587-660");

    const int panelBefore = m11_inventory_get_panel_content_pc34(&inv);
    const int openBefore  = m11_inventory_get_open_chest_thing(&inv, champ);
    const int loadBefore  = m11_inventory_get_load(&inv, champ);

    /* Take the stairs so a transition is pending. */
    expect_int("datafreeze_first_use_rc",
               m11_stairs_use(&stairs, 6, 6, &newX, &newY, &newFacing), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("datafreeze_transition_active",
               stairs.transitionActive, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:91 (I2 IRED)");
    expect_int("datafreeze_transition_ticks_left",
               stairs.transitionTicksLeft, 500,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:87 nominal 500ms");

    /* Tick once (100ms) — transition is still active, transitionTicksLeft=400. */
    m11_stairs_tick(&stairs, 100);
    expect_int("datafreeze_one_tick_active",
               m11_stairs_is_transitioning(&stairs), 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:108-117 (I3 clamp + I4)");
    expect_int("datafreeze_one_tick_ticks_left",
               stairs.transitionTicksLeft, 400,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:111 (I3 clamp)");
    expect_int("datafreeze_one_tick_panel",
               m11_inventory_get_panel_content_pc34(&inv), panelBefore,
               "PANEL.C:G0424_i_PanelContent frozen mid-transition");
    expect_int("datafreeze_one_tick_open_chest",
               m11_inventory_get_open_chest_thing(&inv, champ), openBefore,
               "PANEL.C:G0426_T_OpenChest frozen mid-transition");
    expect_int("datafreeze_one_tick_load",
               m11_inventory_get_load(&inv, champ), loadBefore,
               "CHAMPION.C:M516.Champion[0].Load frozen mid-transition");

    /* Try the rejected stairs call mid-tick — must still be a no-op
     * for both the level state and the inventory. */
    newX = -55;
    newY = -55;
    newFacing = -55;
    expect_int("datafreeze_rejected_mid_tick_rc",
               m11_stairs_use(&stairs, 12, 12, &newX, &newY, &newFacing), 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 mid-tick)");
    expect_int("datafreeze_rejected_mid_tick_x", newX, -55,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65-67 (I1 no-write)");
    expect_int("datafreeze_rejected_mid_tick_panel",
               m11_inventory_get_panel_content_pc34(&inv), panelBefore,
               "PANEL.C:G0424_i_PanelContent; I1 mid-tick never invokes F0355");
    expect_int("datafreeze_rejected_mid_tick_open_chest",
               m11_inventory_get_open_chest_thing(&inv, champ), openBefore,
               "PANEL.C:G0426_T_OpenChest; I1 mid-tick never invokes F0334");

    /* Tick through the rest of the transition (5 more ticks * 100ms = 500ms). */
    for (i = 0; i < 5; ++i) {
        m11_stairs_tick(&stairs, 100);
    }
    transitioning = m11_stairs_is_transitioning(&stairs);
    expect_int("datafreeze_settled_transitioning", transitioning, 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:114 (settle)");
    expect_int("datafreeze_settled_panel",
               m11_inventory_get_panel_content_pc34(&inv), panelBefore,
               "PANEL.C:G0424_i_PanelContent survives to settle");
    expect_int("datafreeze_settled_open_chest",
               m11_inventory_get_open_chest_thing(&inv, champ), openBefore,
               "PANEL.C:G0426_T_OpenChest survives to settle");
}

/* ── Test 3: rejected call leaves a valid follow-up stairs use intact ── */

static void test_rejected_call_does_not_corrupt_followup_stairs_use(void)
{
    /*
     * The I1 rejection path must not only be a no-op for the current
     * state but also leave the level state machine ready for the next
     * successful m11_stairs_use.  After the rejected call, ticking the
     * transition to completion, then taking the next stairs, must work
     * exactly as if the rejected call had never happened.
     */
    M11_StairLevelState stairs;
    M11_InventoryState inv;
    M11_Item linked[8];
    int champ = 0;
    int newX, newY, newFacing;
    int i, rc;

    m11_stairs_init(&stairs);
    m11_stairs_add_level(&stairs, 16, 16);
    m11_stairs_add_level(&stairs, 16, 16);
    expect_int("followup_add_up",
               m11_stairs_add(&stairs, 7, 7, 2, 1, 7, 7, 2), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("followup_add_down",
               m11_stairs_add(&stairs, 9, 9, 2, 0, 9, 9, 2), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");

    m11_inventory_init(&inv, 1);
    for (i = 0; i < 8; ++i) {
        linked[i] = make_item(600 + i, /*weight=*/4, DM1_PC34_ALLOWED_CONTAINER);
    }
    expect_int("followup_open_chest",
               m11_inventory_open_chest(&inv, champ, 0xC0DE, linked, 8), 1,
               "CHEST.C:F0333_INVENTORY_OpenChest:30-75");
    expect_int("followup_set_torch",
               m11_inventory_set_item(&inv, champ, DM1_SLOT_HAND_LEFT,
                                      /*itemType=*/19 /* FLAME */,
                                      /*weight=*/1,
                                      /*charges=*/8), 1,
               "CHAMPION.C:F0301_AddObjectInSlot:587-660");

    /* Take stairs up. */
    expect_int("followup_first_use_rc",
               m11_stairs_use(&stairs, 7, 7, &newX, &newY, &newFacing), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("followup_first_current_level", stairs.currentLevel, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:86 (I2 IRED)");

    /* Rejected call mid-transition. */
    rc = m11_stairs_use(&stairs, 9, 9, &newX, &newY, &newFacing);
    expect_int("followup_rejected_rc", rc, 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1)");
    expect_int("followup_rejected_current_level", stairs.currentLevel, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:65 (I1 no-mutation)");

    /* Tick through to settle. */
    for (i = 0; i < 8; ++i) {
        m11_stairs_tick(&stairs, 100);
    }
    expect_int("followup_settled_transitioning",
               m11_stairs_is_transitioning(&stairs), 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:114 (settle)");
    expect_int("followup_settled_current_level", stairs.currentLevel, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:114 currentLevel preserved");

    /* Inventory still byte-identical (open chest + torch + load). */
    expect_int("followup_settled_panel",
               m11_inventory_get_panel_content_pc34(&inv),
               DM1_PC34_PANEL_CHEST,
               "PANEL.C:G0424_i_PanelContent survives rejected + settled");
    expect_int("followup_settled_open_chest",
               m11_inventory_get_open_chest_thing(&inv, champ), 0xC0DE,
               "PANEL.C:G0426_T_OpenChest survives rejected + settled");
    expect_int("followup_settled_chest_0_item",
               inv.champions[champ].chestSlots[0].itemType, 600,
               "PANEL.C:G0425_aT_ChestSlots[0] survives rejected + settled");

    /* Now take the down-stairs successfully. */
    expect_int("followup_second_use_rc",
               m11_stairs_use(&stairs, 9, 9, &newX, &newY, &newFacing), 1,
               "CLIKMENU.C:F0364_COMMAND_TakeStairs:124-142");
    expect_int("followup_second_current_level", stairs.currentLevel, 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:86 (I2 IRED)");
    expect_int("followup_second_transition_active", stairs.transitionActive, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:91 (I2 IRED last-write)");
    expect_int("followup_second_from_level", stairs.transitionFromLevel, 1,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:84 (I2 IRED first-write)");
    expect_int("followup_second_to_level", stairs.transitionToLevel, 0,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:85 (I2 IRED second-write)");
    expect_int("followup_second_ticks_left", stairs.transitionTicksLeft, 500,
               "src/dm1/dm1_v1_stairs_level_pc34_compat.c:87 nominal 500ms");
    expect_int("followup_second_open_chest",
               m11_inventory_get_open_chest_thing(&inv, champ), 0xC0DE,
               "PANEL.C:G0426_T_OpenChest survives second stairs use");
}

int main(void)
{
    printf("=== DM1 V1 stairs inventory state gate — I1 rejection path ===\n");
    test_stairs_use_during_transition_is_a_pure_no_op();
    test_inventory_is_datafreeze_during_pending_transition();
    test_rejected_call_does_not_corrupt_followup_stairs_use();
    printf("\n=== %d/%d assertions passed (%d failures) ===\n",
           g_assertions - g_failures, g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
