#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n", message, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s [%s]\n", message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void test_evidence(void)
{
    const Dm1V1ChampionPanelHandSlotInventoryViewportWalkEvidencePc34 *e =
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_evidence_pc34();
    const char *text =
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_source_evidence_pc34();
    const char *siblings[] = {
        "champion_panel_hand_slot_refresh",
        "champion_panel_dead_member_hand_refresh",
        "champion_panel_hand_slot_priority",
        "champion_panel_portrait_box_redraw_states",
        "champion_panel_portrait_state_redraw",
        "mirror_candidate_icon_refresh",
        "mirror_candidate_c040",
        "champion_panel_spell_area_overlay",
        "champion_panel_status_hand_rotation",
        "champion_panel_second_leader_hand_slot_priority",
        "F0107",
        "F0108",
        "chest-scroll-wheel",
        "inventory_slotbox_pc34_compat_count",
    };
    int i;

    check_true(e != NULL, "evidence accessor",
               "inventory_viewport_walk_evidence");
    check_contains(e->walkF0295Anchor, "1153-1182",
                   "F0295 anchor", e->walkF0295Anchor);
    check_contains(e->walkF0296InventoryAnchor, "1234-1242",
                   "F0296 inventory-walk anchor",
                   e->walkF0296InventoryAnchor);
    check_contains(e->walkF0296ChestAnchor, "1244-1253",
                   "F0296 chest-walk anchor", e->walkF0296ChestAnchor);
    check_contains(e->walkF0296TailAnchor, "1254-1257",
                   "F0296 tail anchor", e->walkF0296TailAnchor);
    check_contains(e->walkF0386Anchor, "F0386_MENUS_DrawActionIcon",
                   "F0386 anchor", e->walkF0386Anchor);
    check_contains(e->walkF0292Anchor, "F0292_CHAMPION_DrawState",
                   "F0292 anchor", e->walkF0292Anchor);
    check_contains(e->inventoryChampionOrdinalAnchor,
                   "G0423_i_InventoryChampionOrdinal",
                   "G0423 inventory ordinal anchor",
                   e->inventoryChampionOrdinalAnchor);
    check_contains(e->panelContentAnchor, "G0424_i_PanelContent",
                   "G0424 panel content anchor", e->panelContentAnchor);
    check_contains(e->panelContentAnchor, "M569_PANEL_CHEST",
                   "M569 chest anchor", e->panelContentAnchor);
    check_contains(e->chestSlotsAnchor, "G0425_aT_ChestSlots",
                   "G0425 chest slots anchor", e->chestSlotsAnchor);
    check_contains(e->dirtyBitAnchor, "MASK0x4000_VIEWPORT",
                   "MASK0x4000_VIEWPORT anchor", e->dirtyBitAnchor);
    check_contains(e->defsAnchor, "C08_SLOT_BOX_INVENTORY_FIRST_SLOT",
                   "DEFS.H C08 inventory first slotbox anchor",
                   e->defsAnchor);
    check_contains(e->defsAnchor, "C38_SLOT_BOX_CHEST_FIRST_SLOT",
                   "DEFS.H C38 chest first slotbox anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C00_SLOT_READY_HAND",
                   "DEFS.H C00 ready hand anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C01_SLOT_ACTION_HAND",
                   "DEFS.H C01 action hand anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "C30_SLOT_CHEST_1",
                   "DEFS.H C30 chest-1 anchor", e->defsAnchor);
    check_contains(e->defsAnchor, "M001_ORDINAL_TO_INDEX",
                   "DEFS.H M001 ordinal-to-index anchor", e->defsAnchor);

    check_contains(text,
                   "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262",
                   "F0296 source text", text);
    check_contains(text, "F0296:1234-1242",
                   "F0296 inventory-walk source text", text);
    check_contains(text, "F0296:1244",
                   "F0296 chest-gate source text", text);
    check_contains(text, "F0296:1249-1253",
                   "F0296 chest-walk source text", text);
    check_contains(text, "F0296:1254-1257",
                   "F0296 tail source text", text);
    check_contains(text, "MASK0x4000_VIEWPORT",
                   "MASK0x4000_VIEWPORT source text", text);

    for (i = 0; i < (int)(sizeof(siblings) / sizeof(siblings[0])); ++i) {
        char id[64];
        const char *s = siblings[i];
        snprintf(id, sizeof(id), "sibling.%s", s);
        check_contains(e->nonOverlap, s, id, e->nonOverlap);
    }

    check_contains(e->noRealGraphicsClaim, "no real-asset bitmap parity claim",
                   "no parity claim", e->noRealGraphicsClaim);
    check_contains(e->contractScope,
                   "single inventory-owner + 4-champion party",
                   "single-inventory-owner scope", e->contractScope);
}

static void test_inventory_walk_default_no_change(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);

    check_int_eq(state.partyChampionCount, 4,
                 "party count = 4", "DM1_V1_HSIVW_PARTY_COUNT_PC34");
    check_int_eq(state.inventoryChampionOrdinal, 3,
                 "inventory champion ordinal = 3",
                 "owner index 2 -> ordinal 3");
    check_int_eq(state.inventoryChampionIndex, 2,
                 "inventory champion index = 2",
                 "owner ordinal 3 -> index 2");
    check_int_eq(state.champions[2].inventoryOwner, 1,
                 "champion 2 marked as inventory owner",
                 "single inventory owner");
    check_int_eq(state.champions[0].inventoryOwner, 0,
                 "champion 0 not inventory owner", "single inventory owner");
    check_int_eq(state.champions[3].inventoryOwner, 0,
                 "champion 3 not inventory owner", "single inventory owner");

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        1, "run returns success", "single inventory-owner baseline");

    check_int_eq(result.accepted, 1, "result.accepted",
                 "F0296 inventory-viewport walk returned");
    check_int_eq(result.inventoryOwnerRecognized, 1,
                 "inventory owner recognized",
                 "F0296:1234 inventory-owner branch");
    check_int_eq(result.inventoryWalkIndexRangeC00ToC29, 1,
                 "inventory walk index range [C00..C29]",
                 "F0296:1236 AL0882_ui_SlotIndex range");
    check_int_eq(result.inventoryWalkSlotboxIndexRangeC08ToC37, 1,
                 "inventory walk slotbox range [C08..C37]",
                 "F0296:1238 slotbox = slot + C08");
    check_int_eq(result.inventoryWalkStrictAscending, 1,
                 "inventory walk strict ascending 0..29",
                 "F0296:1237 for-loop ascending");
    check_int_eq(result.inventoryWalkF0295ContractMutableIcon, 1,
                 "F0295 sense contract fires for every inventory slot",
                 "F0296:1238 + F0295:1153-1182");
    check_int_eq(result.inventoryWalkF0295NoChangeSkipsF0386, 1,
                 "F0386 not dispatched when action-hand icon unchanged",
                 "F0296:1239-1241 L0889 changed-action-hand guard");
    check_int_eq(result.inventoryActionHandF0386DispatchContract, 1,
                 "F0386 dispatch contract on action hand",
                 "F0296:1239-1241");
    check_int_eq(result.inventoryActionHandF0386DispatchOncePerF0296, 1,
                 "F0386 dispatched at most once per F0296",
                 "F0296:1239-1241 single dispatch");
    check_int_eq(result.chestWalkGatedOnPanelContent569, 0,
                 "chest walk gated off when panel content != 569",
                 "F0296:1244 G0424 != M569");
    check_int_eq(result.f0292TailDispatchedWhenAnyInventoryOrChestChanged, 1,
                 "F0292 tail dispatched iff any change (no change -> no dispatch)",
                 "F0296:1254 draw_viewport == 0");
    check_int_eq(result.f0292TailSuppressedWhenAllInventoryAndChestUnchanged,
                 1, "F0292 tail suppressed when nothing changed",
                 "F0296:1254-1257 AL0884_B_DrawViewport == 0");
    check_int_eq(result.mask4000ViewportSetWhenAnyInventoryOrChestChanged, 1,
                 "MASK0x4000_VIEWPORT iff any change (no change -> not set)",
                 "F0296:1255 draw_viewport == 0");
    check_int_eq(result.path,
                 DM1_V1_HSIVW_PATH_INVENTORY_WALK_NO_CHEST_PC34,
                 "path = INVENTORY_WALK_NO_CHEST (default panel content)",
                 "F0296:1244 panel != 569");

    check_int_eq(result.f0295InventoryHasIconChangedCount, 0,
                 "default no inventory-icon changes",
                 "F0296:1238 + F0295 same-icon path");
    check_int_eq(result.f0295InventorySameIconCount, 30,
                 "all 30 inventory slots reported same-icon",
                 "F0296:1237 slot range [0..29]");
    check_int_eq(result.f0295ChestHasIconChangedCount, 0,
                 "no chest sub-walk fired (panel != 569)",
                 "F0296:1244 chest gate");
    check_int_eq(result.f0295ChestSameIconCount, 0,
                 "no chest sub-walk fired (panel != 569)",
                 "F0296:1244 chest gate");
    check_int_eq(result.f0386InventoryOwnerDispatchCount, 0,
                 "no F0386 dispatch (action-hand unchanged)",
                 "F0296:1239-1241 guard");
    check_int_eq(result.f0292DrawStateDispatchCount, 0,
                 "no F0292 dispatch (no icon change)",
                 "F0296:1254 AL0884_B_DrawViewport == 0");
    check_int_eq(result.dirtyBitViewportSet, 0,
                 "MASK0x4000_VIEWPORT not set (no icon change)",
                 "F0296:1255");
}

static void test_inventory_walk_action_hand_changed(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);

    /* Change the inventory owner's action-hand slot icon so F0295
     * reports has-icon-changed and F0386 dispatches + F0292 fires.
     */
    state.champions[2].slots[1].slotBoxCurrentIcon = 22;
    state.champions[2].slots[1].slotIconIndex = 25;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        1, "run returns success (action-hand changed)",
        "F0296 inventory-viewport walk + F0386 dispatch");

    check_int_eq(result.inventoryOwnerRecognized, 1,
                 "inventory owner recognized",
                 "F0296:1234 inventory-owner branch");
    check_int_eq(result.inventoryWalkF0295ContractMutableIcon, 1,
                 "F0295 sense contract fires for every inventory slot",
                 "F0296:1238 + F0295:1153-1182");
    check_int_eq(result.inventoryWalkF0295NoChangeSkipsF0386, 1,
                 "F0386 dispatch aligned with action-hand change",
                 "F0296:1239-1241 L0889 changed-action-hand guard");
    check_int_eq(result.inventoryActionHandF0386DispatchContract, 1,
                 "F0386 dispatched iff action-hand changed",
                 "F0296:1239-1241");
    check_int_eq(result.inventoryActionHandF0386DispatchOncePerF0296, 1,
                 "F0386 dispatched exactly once per F0296",
                 "F0296:1239-1241 single dispatch");
    check_int_eq(result.f0295InventoryHasIconChangedCount, 1,
                 "one inventory has-icon-changed (action hand)",
                 "F0296:1238 + F0295 mutable changed");
    check_int_eq(result.f0295InventorySameIconCount, 29,
                 "29 inventory slots reported same-icon",
                 "F0296:1237 slot range minus the action-hand change");
    check_int_eq(result.f0386InventoryOwnerDispatchCount, 1,
                 "F0386 dispatched exactly once for inventory-owner action hand",
                 "F0296:1239-1241");
    check_int_eq(result.f0292DrawStateDispatchCount, 1,
                 "F0292 dispatched exactly once when action-hand changed",
                 "F0296:1254-1257 AL0884_B_DrawViewport != 0");
    check_int_eq(result.dirtyBitViewportSet, 1,
                 "MASK0x4000_VIEWPORT set when action-hand changed",
                 "F0296:1255");
    check_int_eq(result.f0292TailDispatchedWhenAnyInventoryOrChestChanged, 1,
                 "F0292 tail fires when at least one slot changed",
                 "F0296:1254 AL0884_B_DrawViewport != 0");
    check_int_eq(result.mask4000ViewportSetWhenAnyInventoryOrChestChanged, 1,
                 "MASK0x4000_VIEWPORT set when at least one slot changed",
                 "F0296:1255");
    check_int_eq(result.inventoryWalkDispatchedF0386[1], 1,
                 "walk[1] dispatched F0386 (action-hand slot)",
                 "F0296:1239-1241 action-hand dispatch");
    check_int_eq(result.inventoryWalkDispatchedF0386[0], 0,
                 "walk[0] did not dispatch F0386 (ready-hand slot)",
                 "F0296:1239-1241 slot != action-hand");
    check_int_eq(result.inventoryWalkDispatchedF0386[29], 0,
                 "walk[29] did not dispatch F0386 (chest-1 boundary)",
                 "F0296:1239-1241 slot != action-hand");
    check_int_eq(result.inventoryWalkSlotboxIndex[1], 9,
                 "walk[1] slotbox index = 9 (slot 1 + C08)",
                 "F0296:1238 slotbox = slot + C08");
    check_int_eq(result.inventoryWalkSlotboxIndex[0], 8,
                 "walk[0] slotbox index = 8 (slot 0 + C08)",
                 "F0296:1238 slotbox = slot + C08");
    check_int_eq(result.inventoryWalkSlotboxIndex[29], 37,
                 "walk[29] slotbox index = 37 (slot 29 + C08)",
                 "F0296:1238 slotbox = slot + C08");
    check_int_eq(result.f0292TailSuppressedWhenAllInventoryAndChestUnchanged,
                 1,
                 "F0292-tail-suppressed invariant is true when something did change",
                 "F0296:1254-1257 AL0884_B_DrawViewport != 0");
}

static void test_inventory_walk_chest_subwalk_panel_569(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);

    state.panelContent = DM1_V1_HSIVW_M569_PANEL_CHEST_PC34;
    /* Mutate one chest slot to trigger a chest has-icon-changed. */
    state.chestSlots[3].chestSlotBoxCurrentIcon = 22;
    state.chestSlots[3].chestSlotIconIndex = 30;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        1, "run returns success (chest sub-walk)",
        "F0296 inventory-viewport + chest sub-walk + F0292 tail");

    check_int_eq(result.inventoryOwnerRecognized, 1,
                 "inventory owner recognized",
                 "F0296:1234 inventory-owner branch");
    check_int_eq(result.chestWalkGatedOnPanelContent569, 1,
                 "chest walk fires when panel content == 569",
                 "F0296:1244 G0424 == M569");
    check_int_eq(result.f0295ChestHasIconChangedCount, 1,
                 "one chest has-icon-changed (chest slot 3)",
                 "F0296:1251 + F0295 mutable changed");
    check_int_eq(result.f0295ChestSameIconCount, 7,
                 "seven chest slots reported same-icon",
                 "F0296:1250 chest range [0..7] minus slot 3");
    check_int_eq(result.chestWalkHasIconChanged[3], 1,
                 "chest walk[3] has-icon-changed",
                 "F0296:1251 chest[3] icon mismatch");
    check_int_eq(result.chestWalkHasIconChanged[0], 0,
                 "chest walk[0] same-icon",
                 "F0296:1251 chest[0] icon match");
    check_int_eq(result.chestWalkHasIconChanged[7], 0,
                 "chest walk[7] same-icon",
                 "F0296:1251 chest[7] icon match");
    check_int_eq(result.chestWalkIndexRangeZeroToSeven, 1,
                 "chest walk index range [0..7]",
                 "F0296:1250 chest range");
    check_int_eq(result.chestWalkSlotboxIndexRangeC38ToC45, 1,
                 "chest walk slotbox range [C38..C45]",
                 "F0296:1251 slotbox = chestIndex + C38");
    check_int_eq(result.chestWalkSlotboxIndex[0], 38,
                 "chest walk[0] slotbox index = 38",
                 "F0296:1251 slotbox = chestIndex + C38");
    check_int_eq(result.chestWalkSlotboxIndex[3], 41,
                 "chest walk[3] slotbox index = 41",
                 "F0296:1251 slotbox = chestIndex + C38");
    check_int_eq(result.chestWalkSlotboxIndex[7], 45,
                 "chest walk[7] slotbox index = 45",
                 "F0296:1251 slotbox = chestIndex + C38");
    check_int_eq(result.f0292DrawStateDispatchCount, 1,
                 "F0292 dispatched once when chest sub-walk changed",
                 "F0296:1254-1257 AL0884_B_DrawViewport != 0");
    check_int_eq(result.dirtyBitViewportSet, 1,
                 "MASK0x4000_VIEWPORT set when chest sub-walk changed",
                 "F0296:1255");
    check_int_eq(result.path,
                 DM1_V1_HSIVW_PATH_INVENTORY_WALK_WITH_CHEST_PC34,
                 "path = INVENTORY_WALK_WITH_CHEST (panel == 569)",
                 "F0296:1244 G0424 == M569");
    check_int_eq(result.f0386InventoryOwnerDispatchCount, 0,
                 "F0386 not dispatched (chest slots never fire F0386)",
                 "F0296:1249-1253 chest sub-walk does not call F0386");
}

static void test_inventory_walk_action_hand_plus_chest(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);

    state.panelContent = DM1_V1_HSIVW_M569_PANEL_CHEST_PC34;
    state.champions[2].slots[1].slotBoxCurrentIcon = 22;
    state.champions[2].slots[1].slotIconIndex = 25;
    state.chestSlots[5].chestSlotBoxCurrentIcon = 22;
    state.chestSlots[5].chestSlotIconIndex = 30;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        1, "run returns success (action-hand + chest both changed)",
        "F0296 inventory-viewport + chest sub-walk");

    check_int_eq(result.f0295InventoryHasIconChangedCount, 1,
                 "one inventory has-icon-changed (action hand)",
                 "F0296:1238 + F0295 mutable changed");
    check_int_eq(result.f0295ChestHasIconChangedCount, 1,
                 "one chest has-icon-changed (chest slot 5)",
                 "F0296:1251 + F0295 mutable changed");
    check_int_eq(result.f0386InventoryOwnerDispatchCount, 1,
                 "F0386 dispatched for inventory-owner action hand",
                 "F0296:1239-1241");
    check_int_eq(result.f0292DrawStateDispatchCount, 1,
                 "F0292 dispatched once (any change)",
                 "F0296:1254 AL0884_B_DrawViewport != 0");
    check_int_eq(result.dirtyBitViewportSet, 1,
                 "MASK0x4000_VIEWPORT set (any change)",
                 "F0296:1255");
    check_int_eq(result.path,
                 DM1_V1_HSIVW_PATH_INVENTORY_WALK_WITH_CHEST_PC34,
                 "path = INVENTORY_WALK_WITH_CHEST",
                 "F0296:1244 G0424 == M569");
}

static void test_no_inventory_owner_skips_sub_walks(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);

    /* No inventory owner: F0296:1234 guard skips the entire
     * inventory-viewport sub-walk + chest sub-walk + F0292 tail.
     */
    state.inventoryChampionOrdinal = 0;
    state.inventoryChampionIndex = -1;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        1, "run returns success (no inventory owner)",
        "F0296:1234 guard");
    check_int_eq(result.inventoryOwnerRecognized, 0,
                 "inventory owner NOT recognized (ordinal == 0)",
                 "F0296:1234 guard");
    check_int_eq(result.f0295InventoryHasIconChangedCount, 0,
                 "no inventory-walk F0295 (ordinal == 0)",
                 "F0296:1234 guard");
    check_int_eq(result.f0295ChestHasIconChangedCount, 0,
                 "no chest sub-walk (ordinal == 0)",
                 "F0296:1234 guard");
    check_int_eq(result.f0386InventoryOwnerDispatchCount, 0,
                 "no F0386 dispatch (ordinal == 0)",
                 "F0296:1234 guard");
    check_int_eq(result.f0292DrawStateDispatchCount, 0,
                 "no F0292 dispatch (ordinal == 0)",
                 "F0296:1234 guard");
    check_int_eq(result.dirtyBitViewportSet, 0,
                 "MASK0x4000_VIEWPORT NOT set (ordinal == 0)",
                 "F0296:1234 guard");
    check_int_eq(result.path, DM1_V1_HSIVW_PATH_NO_INVENTORY_OWNER_PC34,
                 "path = NO_INVENTORY_OWNER",
                 "F0296:1234 guard");
}

static void test_panel_content_569_with_chest_changed(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);

    state.panelContent = DM1_V1_HSIVW_M569_PANEL_CHEST_PC34;
    /* Mutate three chest slots to exercise a multi-slot chest change. */
    state.chestSlots[0].chestSlotBoxCurrentIcon = 22;
    state.chestSlots[0].chestSlotIconIndex = 24;
    state.chestSlots[2].chestSlotBoxCurrentIcon = 22;
    state.chestSlots[2].chestSlotIconIndex = 25;
    state.chestSlots[7].chestSlotBoxCurrentIcon = 22;
    state.chestSlots[7].chestSlotIconIndex = 28;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        1, "run returns success (multi chest changed)",
        "F0296 chest sub-walk");
    check_int_eq(result.chestWalkGatedOnPanelContent569, 1,
                 "chest sub-walk fired (panel == 569)",
                 "F0296:1244 G0424 == M569");
    check_int_eq(result.f0295ChestHasIconChangedCount, 3,
                 "three chest has-icon-changed",
                 "F0296:1251 + F0295 mutable changed");
    check_int_eq(result.f0295ChestSameIconCount, 5,
                 "five chest same-icon",
                 "F0296:1251 chest range minus three");
    check_int_eq(result.f0292DrawStateDispatchCount, 1,
                 "F0292 dispatched once (any chest change)",
                 "F0296:1254 AL0884_B_DrawViewport != 0");
    check_int_eq(result.dirtyBitViewportSet, 1,
                 "MASK0x4000_VIEWPORT set (any chest change)",
                 "F0296:1255");
}

static void test_rejects_party_size_zero(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);
    state.partyChampionCount = 0;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        0, "run returns 0 for party size 0",
        "F0296 party-size guard");
    check_int_eq(result.rejectsPartySizeZero, 1,
                 "rejectsPartySizeZero flag",
                 "F0296 party-size guard");
    check_int_eq(result.path,
                 DM1_V1_HSIVW_PATH_REJECTED_PARTY_SIZE_ZERO_PC34,
                 "path = REJECTED_PARTY_SIZE_ZERO",
                 "F0296 party-size guard");
}

static void test_rejects_negative_inventory_ordinal(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);
    state.inventoryChampionOrdinal = -1;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        0, "run returns 0 for negative inventory ordinal",
        "F0296 inventory-ordinal guard");
    check_int_eq(result.path, DM1_V1_HSIVW_PATH_INVALID_PC34,
                 "path = INVALID (negative ordinal)",
                 "F0296 inventory-ordinal guard");
}

static void test_guards_match_expectations_on_default(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result;

    memset(&state, 0, sizeof(state));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state);

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state, &result),
        1, "run returns success on default state",
        "default state baseline");
    check_int_eq(result.rejectsPartySizeZero, 0,
                 "rejectsPartySizeZero unflagged on default",
                 "default state baseline");
    check_int_eq(result.rejectsPanelContentNot569ForChestWalk, 1,
                 "panel-content-guard unflagged on default (panel != 569)",
                 "F0296:1244 guard");
}

static void test_deterministic_hash(void)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state_a;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 state_b;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result_a;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 result_b;

    memset(&state_a, 0, sizeof(state_a));
    memset(&state_b, 0, sizeof(state_b));
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state_a);
    dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(&state_b);

    state_a.panelContent = DM1_V1_HSIVW_M569_PANEL_CHEST_PC34;
    state_a.champions[2].slots[1].slotBoxCurrentIcon = 22;
    state_a.champions[2].slots[1].slotIconIndex = 25;
    state_a.chestSlots[0].chestSlotBoxCurrentIcon = 22;
    state_a.chestSlots[0].chestSlotIconIndex = 30;

    state_b.panelContent = DM1_V1_HSIVW_M569_PANEL_CHEST_PC34;
    state_b.champions[2].slots[1].slotBoxCurrentIcon = 22;
    state_b.champions[2].slots[1].slotIconIndex = 25;
    state_b.chestSlots[0].chestSlotBoxCurrentIcon = 22;
    state_b.chestSlots[0].chestSlotIconIndex = 30;

    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state_a, &result_a),
        1, "hash run A returns success", "deterministic hash");
    check_int_eq(
        dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
            &state_b, &result_b),
        1, "hash run B returns success", "deterministic hash");
    check_int_eq(result_a.hash, result_b.hash,
                 "FNV-1a hash matches across independent runs",
                 "deterministic hash");
    check_true(result_a.hash != 0u,
               "hash is non-zero on a non-trivial state",
               "FNV-1a non-zero invariant");
}

int main(void)
{
    test_evidence();
    test_inventory_walk_default_no_change();
    test_inventory_walk_action_hand_changed();
    test_inventory_walk_chest_subwalk_panel_569();
    test_inventory_walk_action_hand_plus_chest();
    test_no_inventory_owner_skips_sub_walks();
    test_panel_content_569_with_chest_changed();
    test_rejects_party_size_zero();
    test_rejects_negative_inventory_ordinal();
    test_guards_match_expectations_on_default();
    test_deterministic_hash();

    printf("%s dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_pc34_"
           "compat assertions=%d failures=%d\n",
           g_failures == 0 ? "PASS" : "FAIL", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
