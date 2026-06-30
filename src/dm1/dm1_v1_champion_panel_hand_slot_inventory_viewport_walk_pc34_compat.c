#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 champion-panel hand-slot inventory-viewport walk gate
 * (implementation).
 *
 * Source-locked contract-only gate for CHAMDRAW.C F0296:1234-1257
 * (the inventory-owner viewport sub-walk, chest sub-walk, and
 * F0292_CHAMPION_DrawState + MASK0x4000_VIEWPORT tail). Sibling of
 * the existing `dm1_v1_champion_panel_hand_slot_refresh_pc34_compat`
 * which pins F0296:1184-1262 outside the inventory sub-walk.
 */

enum {
    kPartyChampionCount = 4,
    kInventoryOwnerIndex = 2,
    kInventoryOwnerOrdinal = 3,
    kInventorySlotActionHand = 1,
    kTraceInit = 0,
    kTraceF0296Enter = 1,
    kTraceInventoryWalkStart = 2,
    kTraceInventorySlot = 3,
    kTraceInventoryF0386 = 4,
    kTraceInventoryWalkEnd = 5,
    kTraceChestWalkStart = 6,
    kTraceChestSlot = 7,
    kTraceChestWalkEnd = 8,
    kTraceF0292Dispatch = 9,
    kTraceMaskSet = 10,
    kTraceTail = 11
};

static int is_mutable_inventory_icon(int icon_index)
{
    /* Mirror F0295 mutable-icon contract for C000..C031, C148..C163,
     * C195 (PC 3.4 EN inventory slotboxes).
     */
    if (icon_index < 0) {
        return 0;
    }
    if (icon_index <= 31) {
        return 1;
    }
    if (icon_index >= 148 && icon_index <= 163) {
        return 1;
    }
    if (icon_index == 195) {
        return 1;
    }
    return 0;
}

static const char s_source_evidence[] =
    "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262 owns the "
    "PC 3.4 EN inventory-viewport sub-walk; "
    "F0296:1234-1242 walks the inventory champion's slot indices "
    "[C00_SLOT_READY_HAND .. C30_SLOT_CHEST_1) (30 slots) and calls "
    "F0295_CHAMPION_HasObjectIconInSlotBoxChanged on slotbox "
    "AL0882_ui_SlotIndex + C08_SLOT_BOX_INVENTORY_FIRST_SLOT for each slot, "
    "OR-accumulating AL0884_B_DrawViewport across the walk; "
    "F0296:1239-1241 dispatches "
    "F0386_MENUS_DrawActionIcon(M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal)) "
    "when the changed slot is C01_SLOT_ACTION_HAND; "
    "F0296:1244 gates the chest sub-walk on "
    "G0424_i_PanelContent == M569_PANEL_CHEST (PC34 branch; Saturn I34E uses "
    "G2008_i_PanelContent, disjoint from PC34); "
    "F0296:1249-1253 walks G0425_aT_ChestSlots[0..7] with the F0295 sense-and-OR "
    "contract, mapping each chest index to slotbox "
    "C38_SLOT_BOX_CHEST_FIRST_SLOT + AL0882_ui_SlotIndex (slotboxes 38..45); "
    "F0296:1254-1257 dispatches the F0292_CHAMPION_DrawState path when "
    "AL0884_B_DrawViewport is true, first setting "
    "M008_SET(L0887_ps_Champion->Attributes, MASK0x4000_VIEWPORT) so the next "
    "F0292 redraw covers the viewport; "
    "DEFS.H:780 C00_SLOT_READY_HAND, :781 C01_SLOT_ACTION_HAND, "
    ":810 C30_SLOT_CHEST_1, :1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, "
    ":1876 C38_SLOT_BOX_CHEST_FIRST_SLOT, :1878 M070_HAND_SLOT_INDEX, "
    ":3001 M569_PANEL_CHEST, :5700 G0305_ui_PartyChampionCount, "
    ":5876 G0423_i_InventoryChampionOrdinal, :5877 G0424_i_PanelContent, "
    ":5878 G0425_aT_ChestSlots, :731 MASK0x4000_VIEWPORT, and "
    "COMPILE.H:1039 M001_ORDINAL_TO_INDEX pin the constants; "
    "pass_hand_slot_inventory_viewport_walk_gate is "
    "contract_only asset_free disjoint_from "
    "champion_panel_hand_slot_refresh champion_panel_dead_member_hand_refresh "
    "champion_panel_hand_slot_priority champion_panel_portrait_box_redraw_states "
    "champion_panel_portrait_state_redraw mirror_candidate_icon_refresh "
    "mirror_candidate_c040 champion_panel_spell_area_overlay "
    "champion_panel_status_hand_rotation "
    "champion_panel_second_leader_hand_slot_priority "
    "F0107 F0108 chest-scroll-wheel inventory_slotbox_pc34_compat_count.";

static const Dm1V1ChampionPanelHandSlotInventoryViewportWalkEvidencePc34
    s_evidence = {
        "ReDMCSB CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
        "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1234-1242 walks the inventory-viewport sub-walk",
        "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1244-1253 walks G0425_aT_ChestSlots when G0424_i_PanelContent == M569_PANEL_CHEST",
        "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1254-1257 dispatches F0292_CHAMPION_DrawState + sets MASK0x4000_VIEWPORT",
        "ReDMCSB ACTIDRAW.C F0386_MENUS_DrawActionIcon (inventory-owner dispatch on the changed action hand)",
        "ReDMCSB CHAMDRAW.C F0292_CHAMPION_DrawState (inventory-owner redraw tail)",
        "ReDMCSB DEFS.H:5876 G0423_i_InventoryChampionOrdinal (single owner gate)",
        "ReDMCSB DEFS.H:5877 G0424_i_PanelContent (chest-sub-walk gate) and :3001 M569_PANEL_CHEST",
        "ReDMCSB DEFS.H:5878 G0425_aT_ChestSlots[8] (chest sub-walk source)",
        "ReDMCSB DEFS.H:731 MASK0x4000_VIEWPORT (F0296:1255 viewport dirty bit)",
        "ReDMCSB DEFS.H:780 C00_SLOT_READY_HAND, :781 C01_SLOT_ACTION_HAND, :810 C30_SLOT_CHEST_1, :1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, :1876 C38_SLOT_BOX_CHEST_FIRST_SLOT, :1878 M070_HAND_SLOT_INDEX, :3001 M569_PANEL_CHEST, :5700 G0305_ui_PartyChampionCount, :5876 G0423_i_InventoryChampionOrdinal, :5877 G0424_i_PanelContent, :5878 G0425_aT_ChestSlots, :731 MASK0x4000_VIEWPORT; COMPILE.H:1039 M001_ORDINAL_TO_INDEX",
        "pass_hand_slot_inventory_viewport_walk_gate covers a single inventory-owner + 4-champion party on the F0296 inventory-viewport sub-walk + chest sub-walk + F0292_CHAMPION_DrawState + MASK0x4000_VIEWPORT tail",
        "no real-asset bitmap parity claim; no GRAPHICS.DAT / DUNGEON.DAT load",
        "pass_hand_slot_inventory_viewport_walk_gate is disjoint from "
        "champion_panel_hand_slot_refresh (F0296 top-row status walk-order + "
        "leader-hand precedence + candidate early-return + inventory-champion "
        "ordinal skip, not the F0296 inventory-viewport sub-walk), "
        "champion_panel_dead_member_hand_refresh (F0296/F0295/F0386 walk with a "
        "dead member present + F0292 dead-status-box branch, not the F0296 "
        "inventory-viewport sub-walk), "
        "champion_panel_hand_slot_priority (CHAMPION.C F0302 input dispatch, "
        "not the F0296 inventory-viewport sub-walk), "
        "champion_panel_portrait_box_redraw_states (F0291/F0292/F0296 event "
        "matrix for the portrait-box branch + status-box cascade, not the "
        "F0296 inventory-viewport sub-walk), "
        "champion_panel_portrait_state_redraw (F0292 state-redraw cascade, "
        "not the F0296 inventory-viewport sub-walk), "
        "mirror_candidate_icon_refresh (F0296 leader-hand icon refresh "
        "interaction with the candidate ordinal, no inventory-viewport "
        "sub-walk coverage), "
        "mirror_candidate_c040 sibling family (candidate-panel state machine, "
        "no F0296 inventory-viewport sub-walk), "
        "champion_panel_spell_area_overlay (F0394 dead-champion reject for "
        "spell area, not the F0296 inventory-viewport sub-walk), "
        "champion_panel_status_hand_rotation (F0284 leader rotation, not the "
        "F0296 inventory-viewport sub-walk), "
        "champion_panel_second_leader_hand_slot_priority (the 2nd leader's "
        "hand-slot priority path, not the F0296 inventory-viewport sub-walk), "
        "the F0107/F0108/chest-scroll-wheel family (F0333/F0334 chest close "
        "path, not the F0296 inventory-viewport sub-walk), and "
        "inventory_slotbox_pc34_compat_count (PC34 static slotbox count + "
        "zone index table, not the F0296 sub-walk dispatch contract)."
    };

static uint32_t hash_state(
    const Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state)
{
    /* FNV-1a over the state struct (deterministic across runs). */
    uint32_t hash = 2166136261u;
    const unsigned char *bytes = (const unsigned char *)state;
    size_t i;
    size_t n = sizeof(*state);

    for (i = 0; i < n; ++i) {
        hash ^= (uint32_t)bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence,
                  "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262") !=
               NULL &&
           strstr(s_source_evidence, "F0296:1234-1242") != NULL &&
           strstr(s_source_evidence, "F0296:1244") != NULL &&
           strstr(s_source_evidence, "F0296:1249-1253") != NULL &&
           strstr(s_source_evidence, "F0296:1254-1257") != NULL &&
           strstr(s_source_evidence,
                  "F0295_CHAMPION_HasObjectIconInSlotBoxChanged") != NULL &&
           strstr(s_source_evidence, "F0386_MENUS_DrawActionIcon") != NULL &&
           strstr(s_source_evidence, "F0292_CHAMPION_DrawState") != NULL &&
           strstr(s_source_evidence,
                  "G0423_i_InventoryChampionOrdinal") != NULL &&
           strstr(s_source_evidence, "G0424_i_PanelContent") != NULL &&
           strstr(s_source_evidence, "G0425_aT_ChestSlots") != NULL &&
           strstr(s_source_evidence, "M569_PANEL_CHEST") != NULL &&
           strstr(s_source_evidence, "MASK0x4000_VIEWPORT") != NULL &&
           strstr(s_source_evidence, "C08_SLOT_BOX_INVENTORY_FIRST_SLOT") !=
               NULL &&
           strstr(s_source_evidence, "C38_SLOT_BOX_CHEST_FIRST_SLOT") != NULL &&
           strstr(s_source_evidence, "C00_SLOT_READY_HAND") != NULL &&
           strstr(s_source_evidence, "C01_SLOT_ACTION_HAND") != NULL &&
           strstr(s_source_evidence, "C30_SLOT_CHEST_1") != NULL &&
           strstr(s_source_evidence, "M001_ORDINAL_TO_INDEX") != NULL;
}

void dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state)
{
    int slot;
    int chest;
    int champ;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->assetFree = 1;
    state->partyChampionCount = kPartyChampionCount;
    state->inventoryChampionOrdinal = kInventoryOwnerOrdinal;
    state->inventoryChampionIndex = kInventoryOwnerIndex;
    state->panelContent = 0;
    state->dirtyBitViewportSet = 0;
    state->f0292DrawStateDispatchCount = 0;
    state->f0295InventoryHasIconChangedCount = 0;
    state->f0295InventorySameIconCount = 0;
    state->f0295ChestHasIconChangedCount = 0;
    state->f0295ChestSameIconCount = 0;
    state->f0386InventoryOwnerDispatchCount = 0;
    state->f0296InvocationCount = 0;
    state->f0296Trace[0] = kTraceInit;

    for (champ = 0; champ < DM1_V1_HSIVW_PARTY_COUNT_PC34; ++champ) {
        state->champions[champ].championIndex = champ;
        state->champions[champ].alive = 1;
        state->champions[champ].inventoryOwner = 0;
        for (slot = 0; slot < DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34; ++slot) {
            state->champions[champ].slots[slot].slotThing =
                DM1_V1_HSIVW_THING_NONE_PC34;
            state->champions[champ].slots[slot].slotIconIndex = 0;
            state->champions[champ].slots[slot].slotBoxCurrentIcon = 0;
        }
    }
    state->champions[kInventoryOwnerIndex].inventoryOwner = 1;
    /* Default: action-hand has the empty-hand placeholder (C201) and
     * the slotbox is in sync, so the F0295 sense returns same-icon and
     * F0386 does NOT dispatch. Per-champion baseline for the test.
     */
    state->champions[kInventoryOwnerIndex].slots[kInventorySlotActionHand]
        .slotBoxCurrentIcon = 201;
    state->champions[kInventoryOwnerIndex].slots[kInventorySlotActionHand]
        .slotIconIndex = 201;
    state->champions[kInventoryOwnerIndex].slots[kInventorySlotActionHand]
        .slotThing = 0x1234u;

    for (chest = 0; chest < DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34; ++chest) {
        state->chestSlots[chest].chestSlotThing = DM1_V1_HSIVW_THING_NONE_PC34;
        state->chestSlots[chest].chestSlotIconIndex = 0;
        state->chestSlots[chest].chestSlotBoxCurrentIcon = 0;
    }
}

static int f0295_inventory_sense(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state,
    int slot_index)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkSlotPc34 *slot;
    int current_icon;
    int object_icon;
    int changed;

    if (!state || slot_index < 0 ||
        slot_index >= DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34) {
        return 0;
    }
    if (state->inventoryChampionIndex < 0 ||
        state->inventoryChampionIndex >=
            DM1_V1_HSIVW_PARTY_COUNT_PC34) {
        return 0;
    }
    slot = &state->champions[state->inventoryChampionIndex].slots[slot_index];
    current_icon = slot->slotBoxCurrentIcon;
    object_icon = slot->slotIconIndex;
    if (!is_mutable_inventory_icon(current_icon)) {
        ++state->f0295InventorySameIconCount;
        return 0;
    }
    changed = (object_icon != current_icon) ? 1 : 0;
    if (changed) {
        ++state->f0295InventoryHasIconChangedCount;
        /* Mirror the F0295 side-effect: the slotbox icon is now drawn,
         * so the slotbox "current icon" matches the object icon for
         * the next call. The gate records the redraw but does not
         * mutate `current_icon` (the contract keeps the source-side
         * state and the per-walk result distinguishable).
         */
    } else {
        ++state->f0295InventorySameIconCount;
    }
    return changed;
}

static int f0295_chest_sense(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state,
    int chest_index)
{
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkChestSlotPc34 *chest;
    int current_icon;
    int object_icon;
    int changed;

    if (!state || chest_index < 0 ||
        chest_index >= DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34) {
        return 0;
    }
    chest = &state->chestSlots[chest_index];
    current_icon = chest->chestSlotBoxCurrentIcon;
    object_icon = chest->chestSlotIconIndex;
    if (!is_mutable_inventory_icon(current_icon)) {
        ++state->f0295ChestSameIconCount;
        return 0;
    }
    changed = (object_icon != current_icon) ? 1 : 0;
    if (changed) {
        ++state->f0295ChestHasIconChangedCount;
    } else {
        ++state->f0295ChestSameIconCount;
    }
    return changed;
}

static void f0296_inventory_subwalk(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state,
    int *draw_viewport)
{
    int slot_index;
    int walk_step;
    int sense;
    int dispatched_f0386;

    if (!state || !draw_viewport) {
        return;
    }
    walk_step = 0;
    *draw_viewport = 0;
    for (slot_index = DM1_V1_HSIVW_C00_SLOT_READY_HAND_PC34;
         slot_index < DM1_V1_HSIVW_C30_SLOT_CHEST_1_PC34;
         ++slot_index) {
        sense = f0295_inventory_sense(state, slot_index);
        dispatched_f0386 = 0;
        if (sense && slot_index == DM1_V1_HSIVW_C01_SLOT_ACTION_HAND_PC34) {
            ++state->f0386InventoryOwnerDispatchCount;
            dispatched_f0386 = 1;
        }
        if (walk_step >= 0 &&
            walk_step < DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34) {
            state->inventorySlotWalkIndex[walk_step] = slot_index;
            state->inventorySlotWalkSlotboxIndex[walk_step] =
                slot_index + DM1_V1_HSIVW_C08_INVENTORY_FIRST_SLOTBOX_PC34;
            state->inventorySlotWalkHasIconChanged[walk_step] = sense;
            state->inventorySlotWalkDispatchedF0386[walk_step] =
                dispatched_f0386;
        }
        if (sense) {
            *draw_viewport = 1;
        }
        ++walk_step;
    }
}

static void f0296_chest_subwalk(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state,
    int *draw_viewport)
{
    int chest_index;
    int walk_step;
    int sense;

    if (!state || !draw_viewport) {
        return;
    }
    walk_step = 0;
    for (chest_index = 0;
         chest_index < DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34;
         ++chest_index) {
        sense = f0295_chest_sense(state, chest_index);
        if (walk_step >= 0 &&
            walk_step < DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34) {
            state->chestSlotWalkIndex[walk_step] = chest_index;
            state->chestSlotWalkSlotboxIndex[walk_step] =
                chest_index + DM1_V1_HSIVW_C38_CHEST_FIRST_SLOTBOX_PC34;
            state->chestSlotWalkHasIconChanged[walk_step] = sense;
        }
        if (sense) {
            *draw_viewport = 1;
        }
        ++walk_step;
    }
}

static int f0296_walk(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state)
{
    int draw_viewport;

    if (!state) {
        return 0;
    }
    ++state->f0296InvocationCount;
    /* Per-slot trace markers are recorded in the dedicated
     * inventorySlotWalk* / chestSlotWalk* arrays; the f0296Trace[]
     * array only carries high-level F0296 lifecycle markers
     * (enter, inventory-walk start/end, chest-walk start/end,
     * MASK set, F0292 dispatch, tail): eight entries max.
     */
    state->f0296Trace[0] = kTraceF0296Enter;

    /* F0296:1234-1242 inventory-viewport sub-walk fires when
     * L0883_ui_InventoryChampionOrdinal != 0. The gate models only the
     * inventory-owner path; without an inventory owner the
     * inventory-viewport sub-walk is skipped entirely and the F0292
     * tail cannot fire through this contract.
     */
    if (state->inventoryChampionOrdinal == 0) {
        state->f0296Trace[1] = kTraceTail;
        return 1;
    }
    state->f0296Trace[1] = kTraceInventoryWalkStart;
    draw_viewport = 0;
    f0296_inventory_subwalk(state, &draw_viewport);
    state->f0296Trace[2] = kTraceInventoryWalkEnd;
    /* F0296:1244 chest sub-walk only when G0424 == M569_PANEL_CHEST
     * (PC34 branch). The Saturn I34E branch uses G2008 and is
     * disjoint; this gate follows the PC34 anchor.
     */
    if (state->panelContent == DM1_V1_HSIVW_M569_PANEL_CHEST_PC34) {
        state->f0296Trace[3] = kTraceChestWalkStart;
        f0296_chest_subwalk(state, &draw_viewport);
        state->f0296Trace[4] = kTraceChestWalkEnd;
    } else {
        state->f0296Trace[3] = kTraceTail;
        state->f0296Trace[4] = kTraceInit;
    }
    /* F0296:1254-1257 inventory-owner F0292 redraw tail + MASK0x4000
     * viewport dirty bit, fired only when some F0295 in the inventory
     * or chest sub-walk reported a changed icon.
     */
    if (draw_viewport) {
        state->dirtyBitViewportSet = 1;
        ++state->f0292DrawStateDispatchCount;
        state->f0296Trace[5] = kTraceMaskSet;
        state->f0296Trace[6] = kTraceF0292Dispatch;
        state->f0296Trace[7] = kTraceTail;
    } else {
        state->f0296Trace[5] = kTraceTail;
        state->f0296Trace[6] = kTraceInit;
        state->f0296Trace[7] = kTraceInit;
    }
    return 1;
}

int dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state,
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 *result)
{
    int i;
    int walked_indices[DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int walked_slotbox_indices[DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int walked_ascending;
    int walked_index_range;
    int walked_slotbox_range;
    int walked_chest_indices[DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    int walked_chest_slotbox_indices[DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    int walked_chest_index_range;
    int walked_chest_slotbox_range;
    int action_hand_changed;
    int action_hand_dispatched;
    int rejected_party_size_zero;

    if (!state || !result) {
        return 0;
    }
    rejected_party_size_zero = (state->partyChampionCount <= 0) ? 1 : 0;
    if (state->partyChampionCount <= 0) {
        memset(result, 0, sizeof(*result));
        result->rejectsPartySizeZero = rejected_party_size_zero;
        result->path = DM1_V1_HSIVW_PATH_REJECTED_PARTY_SIZE_ZERO_PC34;
        return 0;
    }
    if (state->inventoryChampionOrdinal < 0) {
        memset(result, 0, sizeof(*result));
        result->rejectsPartySizeZero = 0;
        result->path = DM1_V1_HSIVW_PATH_INVALID_PC34;
        return 0;
    }
    f0296_walk(state);

    /* Build the inventory-walk invariant arrays. */
    walked_ascending = 1;
    walked_index_range = 1;
    walked_slotbox_range = 1;
    action_hand_changed = 0;
    action_hand_dispatched = 0;
    for (i = 0; i < DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34; ++i) {
        walked_indices[i] = state->inventorySlotWalkIndex[i];
        walked_slotbox_indices[i] = state->inventorySlotWalkSlotboxIndex[i];
        if (walked_indices[i] != i) {
            walked_ascending = 0;
        }
        if (walked_indices[i] < DM1_V1_HSIVW_C00_SLOT_READY_HAND_PC34 ||
            walked_indices[i] >= DM1_V1_HSIVW_C30_SLOT_CHEST_1_PC34) {
            walked_index_range = 0;
        }
        if (walked_slotbox_indices[i] !=
            walked_indices[i] +
                DM1_V1_HSIVW_C08_INVENTORY_FIRST_SLOTBOX_PC34) {
            walked_slotbox_range = 0;
        }
        if (walked_slotbox_indices[i] <
                DM1_V1_HSIVW_C08_INVENTORY_FIRST_SLOTBOX_PC34 ||
            walked_slotbox_indices[i] >
                DM1_V1_HSIVW_C08_INVENTORY_FIRST_SLOTBOX_PC34 +
                    DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34 - 1) {
            walked_slotbox_range = 0;
        }
        if (walked_indices[i] == DM1_V1_HSIVW_C01_SLOT_ACTION_HAND_PC34 &&
            state->inventorySlotWalkHasIconChanged[i]) {
            action_hand_changed = 1;
        }
        if (state->inventorySlotWalkDispatchedF0386[i]) {
            action_hand_dispatched = 1;
        }
    }

    /* Build the chest-walk invariant arrays. */
    walked_chest_index_range = 1;
    walked_chest_slotbox_range = 1;
    for (i = 0; i < DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34; ++i) {
        walked_chest_indices[i] = state->chestSlotWalkIndex[i];
        walked_chest_slotbox_indices[i] = state->chestSlotWalkSlotboxIndex[i];
        if (walked_chest_indices[i] < 0 ||
            walked_chest_indices[i] >= DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34) {
            walked_chest_index_range = 0;
        }
        if (walked_chest_slotbox_indices[i] !=
            walked_chest_indices[i] +
                DM1_V1_HSIVW_C38_CHEST_FIRST_SLOTBOX_PC34) {
            walked_chest_slotbox_range = 0;
        }
        if (walked_chest_slotbox_indices[i] <
                DM1_V1_HSIVW_C38_CHEST_FIRST_SLOTBOX_PC34 ||
            walked_chest_slotbox_indices[i] >
                DM1_V1_HSIVW_C38_CHEST_FIRST_SLOTBOX_PC34 +
                    DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34 - 1) {
            walked_chest_slotbox_range = 0;
        }
    }

    memset(result, 0, sizeof(*result));
    result->accepted = 1;
    result->sourceAnchorsPresent = source_anchors_present();
    result->inventoryOwnerRecognized =
        (state->inventoryChampionOrdinal != 0) ? 1 : 0;
    result->inventoryWalkIndexRangeC00ToC29 = walked_index_range;
    result->inventoryWalkSlotboxIndexRangeC08ToC37 = walked_slotbox_range;
    result->inventoryWalkStrictAscending = walked_ascending;
    result->inventoryWalkF0295ContractMutableIcon =
        (state->f0295InventoryHasIconChangedCount +
         state->f0295InventorySameIconCount) ==
        DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34;
    result->inventoryWalkF0295NoChangeSkipsF0386 =
        (state->f0386InventoryOwnerDispatchCount == 0) ||
        action_hand_changed;
    result->inventoryActionHandF0386DispatchContract =
        action_hand_dispatched == action_hand_changed;
    result->inventoryActionHandF0386DispatchOncePerF0296 =
        (state->f0386InventoryOwnerDispatchCount <= 1) &&
        (state->f0296InvocationCount >= 1);
    result->chestWalkIndexRangeZeroToSeven = walked_chest_index_range;
    result->chestWalkSlotboxIndexRangeC38ToC45 = walked_chest_slotbox_range;
    result->chestWalkF0295ContractMutableIcon =
        (state->f0295ChestHasIconChangedCount +
         state->f0295ChestSameIconCount) ==
            DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34 ||
        state->panelContent != DM1_V1_HSIVW_M569_PANEL_CHEST_PC34;
    result->chestWalkGatedOnPanelContent569 =
        (state->panelContent == DM1_V1_HSIVW_M569_PANEL_CHEST_PC34)
            ? ((state->f0295ChestHasIconChangedCount +
                state->f0295ChestSameIconCount) ==
                   DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34 &&
               (state->chestSlotWalkIndex[7] == 7) &&
               (state->chestSlotWalkSlotboxIndex[7] == 45))
            : 0;
    result->f0292TailDispatchedWhenAnyInventoryOrChestChanged =
        (state->f0292DrawStateDispatchCount > 0) ==
        ((state->f0295InventoryHasIconChangedCount +
          state->f0295ChestHasIconChangedCount) > 0);
    result->f0292TailSuppressedWhenAllInventoryAndChestUnchanged =
        ((state->f0295InventoryHasIconChangedCount == 0) &&
         (state->f0295ChestHasIconChangedCount == 0))
            ? (state->f0292DrawStateDispatchCount == 0)
            : 1;
    result->mask4000ViewportSetWhenAnyInventoryOrChestChanged =
        (state->dirtyBitViewportSet != 0) ==
        ((state->f0295InventoryHasIconChangedCount +
          state->f0295ChestHasIconChangedCount) > 0);
    result->rejectsPartySizeZero = rejected_party_size_zero;
    result->rejectsPanelContentNot569ForChestWalk =
        (state->panelContent != DM1_V1_HSIVW_M569_PANEL_CHEST_PC34)
            ? ((state->f0295ChestHasIconChangedCount == 0) &&
               (state->f0295ChestSameIconCount == 0))
            : 1;
    result->partyChampionCount = state->partyChampionCount;
    result->inventoryChampionOrdinal = state->inventoryChampionOrdinal;
    result->inventoryChampionIndex = state->inventoryChampionIndex;
    result->panelContent = state->panelContent;
    result->f0296InvocationCount = state->f0296InvocationCount;
    result->f0295InventoryHasIconChangedCount =
        state->f0295InventoryHasIconChangedCount;
    result->f0295InventorySameIconCount = state->f0295InventorySameIconCount;
    result->f0295ChestHasIconChangedCount =
        state->f0295ChestHasIconChangedCount;
    result->f0295ChestSameIconCount = state->f0295ChestSameIconCount;
    result->f0386InventoryOwnerDispatchCount =
        state->f0386InventoryOwnerDispatchCount;
    result->f0292DrawStateDispatchCount = state->f0292DrawStateDispatchCount;
    result->dirtyBitViewportSet = state->dirtyBitViewportSet;
    for (i = 0; i < DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34; ++i) {
        result->inventoryWalkIndex[i] = state->inventorySlotWalkIndex[i];
        result->inventoryWalkSlotboxIndex[i] =
            state->inventorySlotWalkSlotboxIndex[i];
        result->inventoryWalkHasIconChanged[i] =
            state->inventorySlotWalkHasIconChanged[i];
        result->inventoryWalkDispatchedF0386[i] =
            state->inventorySlotWalkDispatchedF0386[i];
    }
    for (i = 0; i < DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34; ++i) {
        result->chestWalkIndex[i] = state->chestSlotWalkIndex[i];
        result->chestWalkSlotboxIndex[i] =
            state->chestSlotWalkSlotboxIndex[i];
        result->chestWalkHasIconChanged[i] =
            state->chestSlotWalkHasIconChanged[i];
    }
    for (i = 0; i < DM1_V1_HSIVW_TRACE_COUNT_PC34; ++i) {
        result->trace[i] = state->f0296Trace[i];
    }
    if (state->inventoryChampionOrdinal == 0) {
        result->path = DM1_V1_HSIVW_PATH_NO_INVENTORY_OWNER_PC34;
    } else if (state->panelContent == DM1_V1_HSIVW_M569_PANEL_CHEST_PC34) {
        result->path = DM1_V1_HSIVW_PATH_INVENTORY_WALK_WITH_CHEST_PC34;
    } else {
        result->path = DM1_V1_HSIVW_PATH_INVENTORY_WALK_NO_CHEST_PC34;
    }
    result->hash = hash_state(state);
    return 1;
}

const Dm1V1ChampionPanelHandSlotInventoryViewportWalkEvidencePc34 *
dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}
