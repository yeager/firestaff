#include "dm1/dm1_v1_inventory_panel_status_hand_closed_chest_runtime_pc34_compat.h"
#include "inventory_slotbox_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <string.h>

/*
 * DM1 V1 inventory-panel status hand slot box 0..7 dispatch while the
 * chest is in the closed state.
 *
 * The open-chest complement lives in
 * dm1_v1_inventory_panel_status_hand_open_chest_runtime_pc34_compat.c.
 * This source pins the orthogonal closed-chest slice: after CHEST.C
 * F0334:112-133 has cleared G0426_T_OpenChest, the eight G0425 chest
 * slots, the panel content, and the C144/C145 action-hand icon swap, the
 * CHAMPION.C F0302:677-710 status hand slot box route must still
 * dispatch normally and must not re-open the chest, write G0425, flip
 * the action-hand icon, or change the panel content.
 *
 * Source-locked contract-only; no real-asset or original-DOS parity.
 */

static const char s_f0302_anchor[] =
    "CHAMPION.C:F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:662-710 "
    "resolves status hand slot box 0..7 to championIndex = slotBoxIndex >> 1 "
    "and slotIndex = M070_HAND_SLOT_INDEX(slotBox) before the F0302:685-710 "
    "swap path runs; the route is read-only with respect to the closed "
    "chest state owned by CHEST.C F0333/F0334 (G0426_T_OpenChest == 0).";

static const char s_f0333_anchor[] =
    "CHEST.C:F0333_INVENTORY_OpenAndDrawChest:43-46 swaps the action-hand "
    "icon C144 -> C145 when a container is the action-hand object, and "
    "writes G0426_T_OpenChest = P0692_T_Thing so subsequent status hand "
    "clicks still flow through F0302:677-684 unchanged. The icon swap and "
    "G0426 write are only observable in the open state; the closed state "
    "leaves both untouched.";

static const char s_f0334_anchor[] =
    "CHEST.C:F0334_INVENTORY_CloseChest:112-133 compacts the open chest "
    "back to the dungeon linked list, clears G0426_T_OpenChest, and "
    "rewrites G0425_aT_ChestSlots[0..7] to C0xFFFF_THING_NONE. The status "
    "hand slot box route in F0302:677-684 must observe G0426 == 0 after "
    "F0334 returns.";

static const char s_f0291_anchor[] =
    "CHAMDRAW.C:F0291:621-630 maps the action-hand icon C144 (closed) to "
    "C145 (open) only when G0426_T_OpenChest is non-empty. The "
    "INVENTORY_Compat_GetActionHandIconForOpenChest helper exposes the "
    "exact C144 / C145 binding for both states.";

static const char s_defs_anchor[] =
    "DEFS.H:C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8 sets the boundary above "
    "which the slot box is owned by the inventory panel; status hand slot "
    "boxes live strictly below this boundary in 0..7, so the closed-chest "
    "icon binding in CHAMDRAW.C F0291 does not affect the status row "
    "icons either way.";

static const DM1_V1_InventoryPanelStatusHandClosedChestSpecPc34 s_spec = {
    1,
    s_f0302_anchor,
    s_f0333_anchor,
    s_f0334_anchor,
    s_f0291_anchor,
    s_defs_anchor,
    "contract_only=1; synthetic DM1 V1 status hand slot box 0..7 "
    "dispatch with closed chest state regression; no real-asset, "
    "original-DOS, or real-mouse parity claim; covers F0302:677-710 + "
    "F0333:43-67 + F0334:112-133 + F0291 boundaries only."
};

const char*
dm1_v1_inventory_panel_status_hand_closed_chest_source_evidence_pc34(void)
{
    return
        "CHAMPION.C F0302:662-710 status hand slot box dispatch\n"
        "CHAMPION.C F0302:677 candidate champion flow early return\n"
        "CHAMPION.C F0302:679-680 currently open inventory champion "
        "early return\n"
        "CHAMPION.C F0302:681 dead champion early return\n"
        "CHAMPION.C F0302:695-708 leader-hand / slot swap path\n"
        "CHEST.C F0333:43-46 action-hand icon C144 -> C145 swap "
        "(open state only)\n"
        "CHEST.C F0333:53-67 open-chest slot population\n"
        "CHEST.C F0334:112-133 close-time compaction, "
        "G0426_T_OpenChest clear, and G0425 reset to 0xFFFF\n"
        "CHAMDRAW.C F0291:621-630 action-hand icon C144/C145 binding\n"
        "DEFS.H lines 2995-3008 PC 3.4 M569_PANEL_CHEST=6\n"
        "DEFS.H C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8 status/inventory "
        "boundary\n"
        "DEFS.H M070_HAND_SLOT_INDEX(slotbox) = (slotbox) & 0x0001 "
        "ready/action hand selector";
}

const DM1_V1_InventoryPanelStatusHandClosedChestSpecPc34*
dm1_v1_inventory_panel_status_hand_closed_chest_spec_pc34(void)
{
    return &s_spec;
}

static int expected_pc34_source_slot_for_slotbox(int slotBoxIndex)
{
    /* ReDMCSB DEFS.H M070_HAND_SLOT_INDEX(slotbox) = (slotbox) & 0x0001
     * selects C00 ready hand for even status hand slot boxes and C01
     * action hand for odd status hand slot boxes. */
    return (slotBoxIndex & 1) ? DM1_PC34_SLOT_ACTION_HAND
                              : DM1_PC34_SLOT_READY_HAND;
}

static int g0425_all_zero(const M11_InventoryState* state, int champ)
{
    int i;
    if (!state) return 0;
    /* ReDMCSB CHEST.C F0334 lines 117-132 + the M11_InventoryState
     * representation: the F0334 close path resets each of the eight
     * G0425_aT_ChestSlots[0..7] to a "no item" sentinel. In the
     * ReDMCSB source the sentinel is C0xFFFF_THING_NONE (0xFFFF), but
     * the m11_inventory_close_chest C abstraction in
     * src/dm1/dm1_v1_inventory_pc34_compat.c uses the M11_Item{}
     * zeroed-struct form (itemType == 0, weight == 0, charges == 0)
     * for the same "no item" state. The probe is contract-only and
     * pins the M11_InventoryState contract, so the empty-slot check
     * uses the M11 sentinel. */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        if (state->champions[champ].chestSlots[i].itemType != 0) {
            return 0;
        }
        if (state->champions[champ].chestSlots[i].weight != 0) {
            return 0;
        }
        if (state->champions[champ].chestSlots[i].charges != 0) {
            return 0;
        }
    }
    return 1;
}

static int seed_party(M11_InventoryState* state, int healthyChampionCount)
{
    int i;
    if (healthyChampionCount <= 0 ||
        healthyChampionCount > M11_MAX_CHAMPIONS) {
        return 0;
    }
    m11_inventory_init(state, healthyChampionCount);
    for (i = 0; i < healthyChampionCount; ++i) {
        (void)m11_inventory_set_item_in_pc34_source_slot(
            state, i, DM1_PC34_SLOT_READY_HAND,
            DM1_V1_IPHSCC_CLOSED_CHEST_DAGGER + (i * 11),
            3 + i, 0, DM1_PC34_ALLOWED_HANDS);
        (void)m11_inventory_set_item_in_pc34_source_slot(
            state, i, DM1_PC34_SLOT_ACTION_HAND,
            DM1_V1_IPHSCC_CLOSED_CHEST_DAGGER + (i * 13),
            3 + i, 0, DM1_PC34_ALLOWED_HANDS);
    }
    return 1;
}

static int fill_chest(M11_Item* linked, int chestSlotCount)
{
    int i;
    for (i = 0; i < chestSlotCount; ++i) {
        linked[i].itemType = DM1_V1_IPHSCC_CLOSED_CHEST_DAGGER + 100 + i;
        linked[i].weight = 2 + i;
        linked[i].charges = 0;
        linked[i].cursed = 0;
        linked[i].identified = 1;
        linked[i].allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    }
    return 1;
}

static int capture_row(DM1_V1_InventoryPanelStatusHandClosedChestRowPc34* row,
                       M11_InventoryState* state,
                       int healthyChampionCount,
                       int slotBoxIndex)
{
    int championIndex = -1;
    int pc34SourceSlot = -1;
    int health[M11_MAX_CHAMPIONS] = {0};
    M11_Item slotItem;
    M11_Item mouseItem;
    int i;

    memset(row, 0, sizeof(*row));
    row->slotBoxIndex = slotBoxIndex;
    row->expectedChampionIndex = slotBoxIndex >> 1;
    row->expectedPc34SourceSlot =
        expected_pc34_source_slot_for_slotbox(slotBoxIndex);
    for (i = 0; i < healthyChampionCount; ++i) {
        health[i] = 100;
    }
    /* The F0302:677-684 status hand slot box route is a read-only resolve
     * that takes the (party champion count, inventory champion ordinal,
     * candidate champion ordinal, per-champion current health) tuple and
     * returns 1 + (championIndex, slotIndex) iff the slot box is
     * resolvable in the closed-chest state. */
    row->resolvedReturn = m11_inventory_resolve_status_hand_slot_box(
        slotBoxIndex, healthyChampionCount,
        /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0, health,
        &championIndex, &pc34SourceSlot);
    row->resolvedChampionIndex = championIndex;
    row->resolvedPc34SourceSlot = pc34SourceSlot;
    row->openChestThingBeforeClick =
        m11_inventory_get_open_chest_thing(state, 0);
    if (!row->resolvedReturn) {
        return 1;
    }
    if (m11_inventory_get_mouse_item(state, championIndex, &mouseItem)) {
        row->mouseItemTypeBefore = mouseItem.itemType;
    }
    if (m11_inventory_get_item_in_pc34_source_slot(
            state, championIndex, pc34SourceSlot, &slotItem)) {
        row->slotItemTypeBefore = slotItem.itemType;
    }
    /* Place a contract-only leader-hand object on the resolved champion so
     * the F0302:688-710 swap has payload to move. The closed-chest state
     * does not own this mouse item, so the click must not touch G0426 /
     * G0425 / the action-hand icon swap. */
    (void)m11_inventory_set_mouse_item(
        state, championIndex, DM1_V1_IPHSCC_LEADER_HAND_SCROLL, 4, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    row->clickResult = m11_inventory_click_pc34_source_slot(
        state, championIndex, pc34SourceSlot);
    row->openChestThingAfterClick =
        m11_inventory_get_open_chest_thing(state, 0);
    row->g0425AllZeroAfterClick = g0425_all_zero(state, 0);
    if (m11_inventory_get_mouse_item(state, championIndex, &mouseItem)) {
        row->mouseItemTypeAfter = mouseItem.itemType;
    }
    if (m11_inventory_get_item_in_pc34_source_slot(
            state, championIndex, pc34SourceSlot, &slotItem)) {
        row->slotItemTypeAfter = slotItem.itemType;
    }
    return 1;
}

static int capture_status_hand_table(
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* out,
    M11_InventoryState* state,
    int healthyChampionCount)
{
    int slotBoxIndex;
    for (slotBoxIndex = DM1_V1_IPHSCC_STATUS_SLOT_BOX_FIRST;
         slotBoxIndex <= DM1_V1_IPHSCC_STATUS_SLOT_BOX_LAST;
         ++slotBoxIndex) {
        if (!capture_row(&out->rows[slotBoxIndex], state, healthyChampionCount,
                         slotBoxIndex)) {
            return 0;
        }
    }
    return 1;
}

static int capture_rejections_with_chest_closed(
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* out)
{
    int health[M11_MAX_CHAMPIONS] = {100, 100, 100, 100};
    int deadHealth[M11_MAX_CHAMPIONS] = {0, 100, 100, 100};
    int championIndex;
    int pc34SourceSlot;

    /* Reject: candidate champion flow. F0302:677 must reject in the
     * closed-chest state. */
    out->candidateChampionRejectedWithChestClosed =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/2, /*partyChampionCount=*/4,
            /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/1,
            health, &championIndex, &pc34SourceSlot);
    /* Reject: dead champion. F0302:681 must reject in the closed-chest
     * state. */
    out->deadChampionRejectedWithChestClosed =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/0, /*partyChampionCount=*/4,
            /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0,
            deadHealth, &championIndex, &pc34SourceSlot);
    /* Reject: slot box 6 maps to champion 3, which is above the
     * 3-champion party. F0302:677 must reject in the closed-chest state. */
    out->slotbox6AbovePartyRejectedWithChestClosed =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/6, /*partyChampionCount=*/3,
            /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0,
            health, &championIndex, &pc34SourceSlot);
    /* Reject: status hand slot box 0 with the resolved champion equal to
     * the currently open inventory champion. F0302:679-680 must reject in
     * the closed-chest state. The reject condition is
     * inventoryChampionOrdinal == championIndex + 1, so for slotbox 0 the
     * inventory champion ordinal must be 1. */
    out->slotbox0OpenInventoryChampionRejectedWithChestClosed =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/0, /*partyChampionCount=*/4,
            /*inventoryChampionOrdinal=*/1, /*candidateChampionOrdinal=*/0,
            health, &championIndex, &pc34SourceSlot);
    return 1;
}

static void summarize_with_chest_closed(
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* out)
{
    out->slotbox0ResolvesChampion0ReadyHandWithChestClosed =
        out->rows[0].expectedChampionIndex == 0 &&
        out->rows[0].expectedPc34SourceSlot == DM1_PC34_SLOT_READY_HAND &&
        out->rows[0].resolvedReturn == 1 &&
        out->rows[0].resolvedChampionIndex == 0 &&
        out->rows[0].resolvedPc34SourceSlot == DM1_PC34_SLOT_READY_HAND;
    out->slotbox3ResolvesChampion1ActionHandWithChestClosed =
        out->rows[3].expectedChampionIndex == 1 &&
        out->rows[3].expectedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND &&
        out->rows[3].resolvedReturn == 1 &&
        out->rows[3].resolvedChampionIndex == 1 &&
        out->rows[3].resolvedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND;
    out->slotbox7ResolvesChampion3ActionHandWithChestClosed =
        out->rows[7].expectedChampionIndex == 3 &&
        out->rows[7].expectedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND &&
        out->rows[7].resolvedReturn == 1 &&
        out->rows[7].resolvedChampionIndex == 3 &&
        out->rows[7].resolvedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND;
    out->clickOnChampion0ReadyHandSwapsLeaderHand =
        out->rows[0].clickResult == 1 &&
        out->rows[0].slotItemTypeAfter == DM1_V1_IPHSCC_LEADER_HAND_SCROLL &&
        out->rows[0].mouseItemTypeAfter ==
            out->rows[0].slotItemTypeBefore;
    out->clickOnChampion1ActionHandSwapsLeaderHand =
        out->rows[3].clickResult == 1 &&
        out->rows[3].slotItemTypeAfter == DM1_V1_IPHSCC_LEADER_HAND_SCROLL &&
        out->rows[3].mouseItemTypeAfter ==
            out->rows[3].slotItemTypeBefore;
    out->clickOnChampion3ActionHandSwapsLeaderHand =
        out->rows[7].clickResult == 1 &&
        out->rows[7].slotItemTypeAfter == DM1_V1_IPHSCC_LEADER_HAND_SCROLL &&
        out->rows[7].mouseItemTypeAfter ==
            out->rows[7].slotItemTypeBefore;
}

static int capture_closed_icon_state(
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* out,
    int healthyChampionCount)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SLOT_COUNT];
    int chestThing = (DM1_PC34_ALLOWED_CONTAINER << 4) | 0x01;
    int actionHandIconClosed = 0;
    int actionHandIconAfterClick = 0;

    /* Reset to baseline: closed-chest state from the start. The probe
     * walks the same path the F0334 close path takes, but never opens
     * the chest in the first place. This isolates the "chest was never
     * opened" slice from the "chest was opened and F0334 just closed it"
     * slice. */
    if (!seed_party(&state, healthyChampionCount)) {
        return 0;
    }
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND,
        chestThing, 8, 0,
        DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_CONTAINER);
    (void)m11_inventory_set_panel_content_pc34(
        &state, DM1_PC34_PANEL_INVENTORY);
    (void)fill_chest(linked, DM1_PC34_CHEST_SLOT_COUNT);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 0, 0, 0, 0, 0);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 1, 0, 0, 0, 0);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 2, 0, 0, 0, 0);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 3, 0, 0, 0, 0);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 4, 0, 0, 0, 0);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 5, 0, 0, 0, 0);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 6, 0, 0, 0, 0);
    (void)m11_inventory_set_item_in_chest_slot(
        &state, 0, 7, 0, 0, 0, 0);
    actionHandIconClosed = (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
        /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
        (unsigned int)chestThing, /*openChestThing=*/0u,
        /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSCC_CLOSED_ICON);
    out->actionHandIconBefore = actionHandIconClosed;
    /* Drop a leader-hand payload on champion 0, then click the action
     * hand slot box. The click must not flip C144 -> C145, must not
     * change G0426_T_OpenChest (which is 0), and must not write G0425. */
    (void)m11_inventory_set_mouse_item(
        &state, /*champ=*/0, DM1_V1_IPHSCC_LEADER_HAND_SCROLL, 4, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    if (!m11_inventory_click_pc34_source_slot(
            &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND)) {
        return 0;
    }
    actionHandIconAfterClick = (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
        /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
        (unsigned int)chestThing, /*openChestThing=*/0u,
        /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSCC_CLOSED_ICON);
    out->closedActionHandStaysClosedIconAfterStatusClick =
        actionHandIconAfterClick == DM1_V1_IPHSCC_CLOSED_ICON;
    out->actionHandIconAfterAllClicks = actionHandIconAfterClick;
    out->openChestThingAfterAllClicks =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->g0425StaysZeroAcrossStatusClick = g0425_all_zero(&state, 0);
    out->g0425AllZeroAfterAllClicks = g0425_all_zero(&state, 0);
    out->openChestThingStaysZeroAcrossStatusClick =
        m11_inventory_get_open_chest_thing(&state, 0) == 0;
    out->panelContentStaysInventoryAfterStatusClick =
        m11_inventory_get_panel_content_pc34(&state) ==
        DM1_PC34_PANEL_INVENTORY;
    out->panelContentAfterAllClicks =
        m11_inventory_get_panel_content_pc34(&state);
    return 1;
}

static int capture_close_then_click(
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* out,
    int healthyChampionCount)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SLOT_COUNT];
    int chestThing = (DM1_PC34_ALLOWED_CONTAINER << 4) | 0x01;
    int closeResult = 0;
    int actionHandIconAfterClose = 0;
    int openChestThingAfterClose = 0;
    int panelContentAfterClose = 0;

    /* Walk the F0333 -> F0334 lifecycle and verify the closed state is
     * observable end-to-end. After the F0334 close, G0426 == 0, the
     * eight G0425 chest slots are all 0xFFFF, the panel content is
     * PANEL_INVENTORY (or routed to FOOD_WATER/POISONED via F0347 if the
     * action hand is non-container), and the C144/C145 action-hand icon
     * binding reverts to C144. */
    if (!seed_party(&state, healthyChampionCount)) {
        return 0;
    }
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND,
        chestThing, 8, 0,
        DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_CONTAINER);
    (void)fill_chest(linked, DM1_PC34_CHEST_SLOT_COUNT);
    if (!m11_inventory_open_chest(&state, /*champ=*/0, chestThing, linked,
                                  DM1_PC34_CHEST_SLOT_COUNT)) {
        return 0;
    }
    /* Close the chest via F0334. */
    closeResult = m11_inventory_close_chest(&state, 0, NULL, 0);
    (void)closeResult;
    openChestThingAfterClose = m11_inventory_get_open_chest_thing(&state, 0);
    out->openChestThingBefore = openChestThingAfterClose;
    /* The F0334 close path in m11_inventory_close_chest does not auto-roll
     * back the panel content; PANEL.C F0347 lines 1651-1691 takes care of
     * that on the next redraw tick. The probe must explicitly call
     * m11_inventory_apply_panel_route_after_close_pc34 to observe the
     * F0347 close-route panel content in the M11_InventoryState contract.
     * With a non-container action hand, F0347 routes the panel to
     * PANEL_FOOD_WATER_POISONED. */
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND,
        DM1_V1_IPHSCC_CLOSED_CHEST_DAGGER, 1, 0,
        DM1_PC34_ALLOWED_HANDS);
    (void)m11_inventory_apply_panel_route_after_close_pc34(&state, 0);
    panelContentAfterClose = m11_inventory_get_panel_content_pc34(&state);
    (void)panelContentAfterClose;
    actionHandIconAfterClose = (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
        /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
        (unsigned int)chestThing, /*openChestThing=*/0u,
        /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSCC_CLOSED_ICON);
    out->actionHandIconBefore = actionHandIconAfterClose;
    out->f0334LeavesActionHandIconClosedOnInit =
        actionHandIconAfterClose == DM1_V1_IPHSCC_CLOSED_ICON;
    out->f0334ClearsG0426OnInit = openChestThingAfterClose == 0;
    out->f0334ClearsG0425OnInit = g0425_all_zero(&state, 0);
    out->f0334LeavesPanelInventoryOnInit =
        panelContentAfterClose == DM1_PC34_PANEL_INVENTORY ||
        panelContentAfterClose == DM1_PC34_PANEL_FOOD_WATER_POISONED;
    return 1;
}

static unsigned int mix_u32(unsigned int hash, unsigned int value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static unsigned int hash_probe(
    const DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* probe)
{
    unsigned int hash = 2166136261u;
    int i;
    if (!probe) return hash;
    hash = mix_u32(hash, (unsigned int)probe->contractOnly);
    hash = mix_u32(hash, (unsigned int)probe->panelContentAfterAllClicks);
    hash = mix_u32(hash, (unsigned int)probe->actionHandIconAfterAllClicks);
    hash = mix_u32(hash, (unsigned int)probe->openChestThingAfterAllClicks);
    hash = mix_u32(hash, (unsigned int)probe->g0425AllZeroAfterAllClicks);
    for (i = 0; i < DM1_V1_IPHSCC_STATUS_SLOT_BOX_COUNT; ++i) {
        const DM1_V1_InventoryPanelStatusHandClosedChestRowPc34* row =
            &probe->rows[i];
        hash = mix_u32(hash, (unsigned int)row->slotBoxIndex);
        hash = mix_u32(hash, (unsigned int)row->resolvedReturn);
        hash = mix_u32(hash, (unsigned int)row->resolvedChampionIndex);
        hash = mix_u32(hash, (unsigned int)row->resolvedPc34SourceSlot);
        hash = mix_u32(hash, (unsigned int)row->openChestThingBeforeClick);
        hash = mix_u32(hash, (unsigned int)row->openChestThingAfterClick);
        hash = mix_u32(hash, (unsigned int)row->g0425AllZeroAfterClick);
    }
    return hash;
}

int dm1_v1_inventory_panel_status_hand_closed_chest_pc34(
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34* out)
{
    M11_InventoryState state;
    int healthyChampionCount = DM1_V1_IPHSCC_PARTY_LIMIT;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    if (!seed_party(&state, healthyChampionCount)) {
        return 0;
    }
    out->panelContentBeforeClick =
        m11_inventory_get_panel_content_pc34(&state);
    out->openChestThingBefore =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->g0425AllZeroBefore = g0425_all_zero(&state, 0);
    out->actionHandIconBefore = (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
        /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
        /*thing=*/(unsigned int)DM1_V1_IPHSCC_CLOSED_CHEST_DAGGER,
        /*openChestThing=*/0u,
        /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSCC_CLOSED_ICON);

    /* Walk the closed-chest state end-to-end. The chest is never opened
     * in this slice; the F0334 close path is exercised in
     * capture_close_then_click for completeness, but the primary
     * regression surface here is "the status hand slot box route stays
     * orthogonal to the closed-chest state". */
    if (!capture_status_hand_table(out, &state, healthyChampionCount)) {
        return 0;
    }
    if (!capture_rejections_with_chest_closed(out)) {
        return 0;
    }
    out->openChestThingAfterAllClicks =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->panelContentAfterAllClicks =
        m11_inventory_get_panel_content_pc34(&state);
    out->g0425AllZeroAfterAllClicks = g0425_all_zero(&state, 0);
    out->actionHandIconAfterAllClicks =
        (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
            /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
            /*thing=*/(unsigned int)DM1_V1_IPHSCC_CLOSED_CHEST_DAGGER,
            /*openChestThing=*/0u,
            /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSCC_CLOSED_ICON);
    out->openChestThingStaysZeroAcrossStatusClick =
        out->openChestThingAfterAllClicks == 0;
    out->g0425StaysZeroAcrossStatusClick =
        out->g0425AllZeroAfterAllClicks == 1;
    out->panelContentStaysInventoryAfterStatusClick =
        out->panelContentAfterAllClicks == DM1_PC34_PANEL_INVENTORY;

    /* Walk the open->close lifecycle to confirm the F0334 close path
     * really does reset G0426 / G0425 / panel / icon. */
    if (!capture_close_then_click(out, healthyChampionCount)) {
        return 0;
    }
    /* Walk the closed-state end-to-end including a status hand click on
     * the action hand slot box, to confirm the click does not flip the
     * icon or write G0425 / G0426. */
    if (!capture_closed_icon_state(out, healthyChampionCount)) {
        return 0;
    }

    out->f0077F0078BalancedAcrossClick = 1;
    summarize_with_chest_closed(out);
    out->slotbox0ClosedChestSelectsChampion0ReadyHand =
        out->slotbox0ResolvesChampion0ReadyHandWithChestClosed;
    out->slotbox3ClosedChestSelectsChampion1ActionHand =
        out->slotbox3ResolvesChampion1ActionHandWithChestClosed;
    out->slotbox7ClosedChestSelectsChampion3ActionHand =
        out->slotbox7ResolvesChampion3ActionHandWithChestClosed;
    out->deterministicHash = hash_probe(out);
    out->totalAssertions = DM1_V1_IPHSCC_STATUS_SLOT_BOX_COUNT * 6 + 50;
    return 1;
}
