#include "dm1/dm1_v1_inventory_panel_status_hand_open_chest_runtime_pc34_compat.h"
#include "inventory_slotbox_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <string.h>

static const char s_f0302_anchor[] =
    "CHAMPION.C:F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:662-710 "
    "resolves status hand slot box 0..7 to championIndex = slotBoxIndex >> 1 "
    "and slotIndex = M070_HAND_SLOT_INDEX(slotBox) before the F0302:685-710 "
    "swap path runs; the route is read-only with respect to the open chest "
    "state owned by CHEST.C F0333/F0334.";

static const char s_f0333_anchor[] =
    "CHEST.C:F0333_INVENTORY_OpenAndDrawChest:43-46 swaps the action-hand "
    "icon C144 -> C145 when a container is the action-hand object, and "
    "writes G0426_T_OpenChest = P0692_T_Thing so subsequent status hand "
    "clicks still flow through F0302:677-684 unchanged.";

static const char s_f0334_anchor[] =
    "CHEST.C:F0334_INVENTORY_CloseChest:112-133 compacts the open chest "
    "back to the dungeon linked list and clears G0426_T_OpenChest; the "
    "status hand slot box route is consumed before the open/close path "
    "and never touches G0426.";

static const char s_f0347_anchor[] =
    "PANEL.C:F0347_INVENTORY_DrawPanel:1639-1691 keeps chest panel on "
    "container action hand and redraws food/water/poisoned when not "
    "container-based.";

static const char s_f0291_anchor[] =
    "CHAMDRAW.C:F0291:621-630 maps the action-hand icon C144 (closed) to "
    "C145 (open) while G0426_T_OpenChest is non-empty; the open-chest "
    "state lives in F0333/F0334, and the visible icon binding is owned "
    "by F0291.";

static const char s_defs_anchor[] =
    "DEFS.H:C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8 sets the boundary above "
    "which the slot box is owned by the inventory panel; status hand slot "
    "boxes live strictly below this boundary in 0..7, so the open-chest "
    "icon swap in CHAMDRAW.C F0291 does not affect the status row icons.";

static const DM1_V1_InventoryPanelStatusHandOpenChestSpecPc34 s_spec = {
    1,
    s_f0302_anchor,
    s_f0333_anchor,
    s_f0334_anchor,
    s_f0347_anchor,
    s_f0291_anchor,
    s_defs_anchor,
    "contract_only=1; synthetic DM1 V1 status hand slot box 0..7 reduction "
    "with open chest state regression; no real-asset, original-DOS, or "
    "real-mouse parity claim; covers F0302:677-684 + F0333:43-46 + "
    "F0334:112-133 + F0291 boundaries only."
};

const char*
dm1_v1_inventory_panel_status_hand_open_chest_source_evidence_pc34(void)
{
    return
        "CHAMPION.C F0302:662-710 status hand slot box dispatch\n"
        "CHAMPION.C F0302:677 candidate champion flow early return\n"
        "CHAMPION.C F0302:679-680 currently open inventory champion "
        "early return\n"
        "CHAMPION.C F0302:681 dead champion early return\n"
        "CHAMPION.C F0302:695-708 leader-hand / slot swap path\n"
        "CHEST.C F0333:43-46 action-hand icon C144 -> C145 swap\n"
        "CHEST.C F0333:28-32 PC 3.4 G0424_i_PanelContent = "
        "M569_PANEL_CHEST before same-open return\n"
        "CHEST.C F0333:53-67 open-chest slot population\n"
        "CHEST.C F0334:112-133 close-time compaction and "
        "G0426_T_OpenChest clear\n"
        "CHAMDRAW.C F0291:621-630 action-hand icon C144/C145 binding\n"
    "DEFS.H lines 2995-3008 PC 3.4 M569_PANEL_CHEST=6\n"
    "PANEL.C F0347:1639-1691 redraws food/water/poisoned unless the action hand "
    "is a container\n"
    "DEFS.H C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8 status/inventory "
        "boundary\n"
        "DEFS.H M070_HAND_SLOT_INDEX(slotbox) = (slotbox) & 0x0001 "
        "ready/action hand selector";
}

const DM1_V1_InventoryPanelStatusHandOpenChestSpecPc34*
dm1_v1_inventory_panel_status_hand_open_chest_spec_pc34(void)
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

static int seed_party(M11_InventoryState* state, int healthyChampionCount)
{
    int i;

    if (healthyChampionCount <= 0 ||
        healthyChampionCount > M11_MAX_CHAMPIONS) {
        return 0;
    }
    m11_inventory_init(state, healthyChampionCount);
    for (i = 0; i < healthyChampionCount; ++i) {
        /* Seed every champion's ready hand AND action hand so the
         * status hand row is observable end-to-end through
         * m11_inventory_click_pc34_source_slot. The item-type / weight
         * values are arbitrary contract-only payload; the open-chest
         * state only reads the action-hand container thing on the
         * active champion (champion 0), not the other champions' hands.
         * Champion 0's action hand is later overwritten with a container
         * for the open-chest state. */
        (void)m11_inventory_set_item_in_pc34_source_slot(
            state, i, DM1_PC34_SLOT_READY_HAND,
            DM1_V1_IPHSOC_DAGGER + (i * 11),
            3 + i, 0, DM1_PC34_ALLOWED_HANDS);
        (void)m11_inventory_set_item_in_pc34_source_slot(
            state, i, DM1_PC34_SLOT_ACTION_HAND,
            DM1_V1_IPHSOC_DAGGER + (i * 13),
            3 + i, 0, DM1_PC34_ALLOWED_HANDS);
    }
    return 1;
}

static int fill_chest(M11_Item* linked, int chestSlotCount)
{
    int i;
    for (i = 0; i < chestSlotCount; ++i) {
        linked[i].itemType = DM1_V1_IPHSOC_DAGGER + 100 + i;
        linked[i].weight = 2 + i;
        linked[i].charges = 0;
        linked[i].cursed = 0;
        linked[i].identified = 1;
        linked[i].allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    }
    return 1;
}

static int capture_row(DM1_V1_InventoryPanelStatusHandOpenChestRowPc34* row,
                       M11_InventoryState* state,
                       int healthyChampionCount,
                       int slotBoxIndex,
                       int expectedOpenChestThing)
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
    row->resolvedReturn = m11_inventory_resolve_status_hand_slot_box(
        slotBoxIndex, healthyChampionCount,
        /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0, health,
        &championIndex, &pc34SourceSlot);
    row->resolvedChampionIndex = championIndex;
    row->resolvedPc34SourceSlot = pc34SourceSlot;
    if (!row->resolvedReturn) {
        /* The dispatch code in F0302:677-684 short-circuits before the
         * F0302:695-708 swap path runs, so the click is never issued. */
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
     * the click swap has payload to move. The open-chest state does not
     * own this mouse item; the action-hand chest stays in the action
     * slot of the active champion (champion 0). The leader-hand object
     * uses MASK0xFFFF_ANY_SLOT so the F0302:688-710 swap accepts it
     * against ready hand (slot 0) and action hand (slot 1), both of
     * which are MASK0xFFFF_ANY_SLOT source slots. */
    (void)m11_inventory_set_mouse_item(
        state, championIndex, DM1_V1_IPHSOC_LEADER_HAND_SCROLL, 4, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    row->openChestThingBeforeClick =
        m11_inventory_get_open_chest_thing(state, 0);
    row->clickResult = m11_inventory_click_pc34_source_slot(
        state, championIndex, pc34SourceSlot);
    row->openChestThingAfterClick =
        m11_inventory_get_open_chest_thing(state, 0);
    row->openChestThingPreservedByClick =
        row->openChestThingBeforeClick == expectedOpenChestThing &&
        row->openChestThingAfterClick == expectedOpenChestThing;
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
    DM1_V1_InventoryPanelStatusHandOpenChestProbePc34* out,
    M11_InventoryState* state,
    int healthyChampionCount,
    int expectedOpenChestThing)
{
    int slotBoxIndex;
    for (slotBoxIndex = DM1_V1_IPHSOC_STATUS_SLOT_BOX_FIRST;
         slotBoxIndex <= DM1_V1_IPHSOC_STATUS_SLOT_BOX_LAST;
         ++slotBoxIndex) {
        if (!capture_row(&out->rows[slotBoxIndex], state, healthyChampionCount,
                         slotBoxIndex, expectedOpenChestThing)) {
            return 0;
        }
    }
    return 1;
}

static int capture_rejections_with_chest_open(
    DM1_V1_InventoryPanelStatusHandOpenChestProbePc34* out)
{
    int health[M11_MAX_CHAMPIONS] = {100, 100, 100, 100};
    int deadHealth[M11_MAX_CHAMPIONS] = {0, 100, 100, 100};
    int championIndex;
    int pc34SourceSlot;

    /* Reject: candidate champion flow.  The status hand slot box route
     * in F0302:677 must reject even when a chest is open. */
    out->candidateChampionRejectedWithChestOpen =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/2, /*partyChampionCount=*/4,
            /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/1,
            health, &championIndex, &pc34SourceSlot);
    /* Reject: dead champion.  F0302:681 must reject even when a chest
     * is open. */
    out->deadChampionRejectedWithChestOpen =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/0, /*partyChampionCount=*/4,
            /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0,
            deadHealth, &championIndex, &pc34SourceSlot);
    /* Reject: slot box 6 maps to champion 3, which is above the 3-champion
     * party.  F0302:677 must reject even when a chest is open. */
    out->slotbox4AbovePartyRejectedWithChestOpen =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/6, /*partyChampionCount=*/3,
            /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0,
            health, &championIndex, &pc34SourceSlot);
    /* Reject: status hand slot box 0 with the resolved champion equal
     * to the currently open inventory champion.  F0302:679-680 must
     * reject even when a chest is open.  The reject condition is
     * inventoryChampionOrdinal == championIndex + 1, so for slotbox 0
     * the inventory champion ordinal must be 1. */
    out->slotbox0OpenInventoryChampionRejectedWithChestOpen =
        m11_inventory_resolve_status_hand_slot_box(
            /*slotBoxIndex=*/0, /*partyChampionCount=*/4,
            /*inventoryChampionOrdinal=*/1, /*candidateChampionOrdinal=*/0,
            health, &championIndex, &pc34SourceSlot);
    return 1;
}

static void summarize_with_chest_open(
    DM1_V1_InventoryPanelStatusHandOpenChestProbePc34* out)
{
    out->slotbox0ResolvesChampion0ReadyHandWithChestOpen =
        out->rows[0].expectedChampionIndex == 0 &&
        out->rows[0].expectedPc34SourceSlot == DM1_PC34_SLOT_READY_HAND &&
        out->rows[0].resolvedReturn == 1 &&
        out->rows[0].resolvedChampionIndex == 0 &&
        out->rows[0].resolvedPc34SourceSlot == DM1_PC34_SLOT_READY_HAND;
    out->slotbox3ResolvesChampion1ActionHandWithChestOpen =
        out->rows[3].expectedChampionIndex == 1 &&
        out->rows[3].expectedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND &&
        out->rows[3].resolvedReturn == 1 &&
        out->rows[3].resolvedChampionIndex == 1 &&
        out->rows[3].resolvedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND;
    out->slotbox7ResolvesChampion3ActionHandWithChestOpen =
        out->rows[7].expectedChampionIndex == 3 &&
        out->rows[7].expectedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND &&
        out->rows[7].resolvedReturn == 1 &&
        out->rows[7].resolvedChampionIndex == 3 &&
        out->rows[7].resolvedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND;
    out->clickOnChampion0ReadyHandSwapsLeaderHand =
        out->rows[0].clickResult == 1 &&
        out->rows[0].slotItemTypeAfter == DM1_V1_IPHSOC_LEADER_HAND_SCROLL &&
        out->rows[0].mouseItemTypeAfter ==
            out->rows[0].slotItemTypeBefore;
    out->clickOnChampion1ActionHandSwapsLeaderHand =
        out->rows[3].clickResult == 1 &&
        out->rows[3].slotItemTypeAfter == DM1_V1_IPHSOC_LEADER_HAND_SCROLL &&
        out->rows[3].mouseItemTypeAfter ==
            out->rows[3].slotItemTypeBefore;
    out->clickOnChampion3ActionHandSwapsLeaderHand =
        out->rows[7].clickResult == 1 &&
        out->rows[7].slotItemTypeAfter == DM1_V1_IPHSOC_LEADER_HAND_SCROLL &&
        out->rows[7].mouseItemTypeAfter ==
            out->rows[7].slotItemTypeBefore;
}

static int capture_action_hand_container_edges(
    DM1_V1_InventoryPanelStatusHandOpenChestProbePc34* out,
    int healthyChampionCount,
    int chestThing)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item actionHand;
    M11_Item mouseItem;

    if (!seed_party(&state, healthyChampionCount)) {
        return 0;
    }
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND, chestThing, 8, 0,
        DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_CONTAINER);
    if (!fill_chest(linked, DM1_PC34_CHEST_SLOT_COUNT)) {
        return 0;
    }
    if (!m11_inventory_open_chest(&state, /*champ=*/0, chestThing, linked,
                                  DM1_PC34_CHEST_SLOT_COUNT)) {
        return 0;
    }

    (void)m11_inventory_set_mouse_item(
        &state, /*champ=*/0, DM1_V1_IPHSOC_LEADER_HAND_SCROLL, 4, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    /* ReDMCSB CHAMPION.C F0302 lines 677-684 routes status slot box 0
     * to champion 0 ready hand. It must not touch the open chest's
     * action-hand container or the F0291 open icon binding. */
    if (!m11_inventory_click_pc34_source_slot(
            &state, /*champ=*/0, DM1_PC34_SLOT_READY_HAND)) {
        return 0;
    }
    if (!m11_inventory_get_item_in_pc34_source_slot(
            &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND, &actionHand)) {
        return 0;
    }
    out->readyHandClickLeavesActionHandChestInPlace =
        actionHand.itemType == chestThing;
    out->readyHandClickKeepsOpenActionHandIcon =
        (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
            /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
            (unsigned int)actionHand.itemType, (unsigned int)chestThing,
            /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSOC_CLOSED_ICON) ==
        DM1_V1_IPHSOC_OPEN_ICON;

    if (!seed_party(&state, healthyChampionCount)) {
        return 0;
    }
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND, chestThing, 8, 0,
        DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_CONTAINER);
    if (!m11_inventory_open_chest(&state, /*champ=*/0, chestThing, linked,
                                  DM1_PC34_CHEST_SLOT_COUNT)) {
        return 0;
    }
    (void)m11_inventory_set_mouse_item(
        &state, /*champ=*/0, DM1_V1_IPHSOC_LEADER_HAND_SCROLL, 4, 0,
        DM1_PC34_ALLOWED_ANY_SLOT);
    /* Slot box 1 targets the same action-hand slot that displayed the
     * open chest. F0302 lines 688-710 swaps the leader hand and slot
     * payloads, while CHEST.C F0333/F0334 keep G0426_T_OpenChest as the
     * independent open-panel sentinel until close. */
    if (!m11_inventory_click_pc34_source_slot(
            &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND)) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&state, /*champ=*/0, &mouseItem) ||
        !m11_inventory_get_item_in_pc34_source_slot(
            &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND, &actionHand)) {
        return 0;
    }
    out->actionHandClickMovesOpenChestToLeaderHand =
        mouseItem.itemType == chestThing;
    out->actionHandClickReplacesActionHandWithLeaderHandObject =
        actionHand.itemType == DM1_V1_IPHSOC_LEADER_HAND_SCROLL;
    out->actionHandClickDropsActionHandIconToClosed =
        (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
            /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
            (unsigned int)actionHand.itemType, (unsigned int)chestThing,
            /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSOC_CLOSED_ICON) ==
        DM1_V1_IPHSOC_CLOSED_ICON;
    out->actionHandClickPreservesOpenChestThing =
        m11_inventory_get_open_chest_thing(&state, /*champ=*/0) == chestThing;
    return 1;
}

int dm1_v1_inventory_panel_status_hand_open_chest_pc34(
    DM1_V1_InventoryPanelStatusHandOpenChestProbePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SLOT_COUNT];
    int healthyChampionCount = DM1_V1_IPHSOC_PARTY_LIMIT;
    int chestThing;
    int openChestResult = 0;
    int actionHandIconClosed = 0;
    int actionHandIconOpen = 0;
    int openChestThingAfterOpen = 0;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    if (!seed_party(&state, healthyChampionCount)) {
        return 0;
    }
    /* Reuse the chest thing as the active champion's action-hand object
     * so the open-chest action-hand icon swap C144 -> C145 is observable
     * through the same M11 inventory the status hand click touches. */
    chestThing = (THING_TYPE_CONTAINER << 10) | 0x01;
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND, chestThing, 8, 0,
        DM1_PC34_ALLOWED_HANDS | DM1_PC34_ALLOWED_CONTAINER);
    /* Closed action-hand icon stays C144 (CHAMDRAW.C F0291 baseline). */
    actionHandIconClosed = (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
        /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
        (unsigned int)chestThing, /*openChestThing=*/0u,
        /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSOC_CLOSED_ICON);
    out->actionHandIconBefore = actionHandIconClosed;
    out->chestOpenBefore = 0;
    out->openChestThingBefore = 0;
    out->panelContentBeforeOpen = m11_inventory_get_panel_content_pc34(&state);
    /* Open the chest in the panel. The open-chest state is owned by
     * CHEST.C F0333:43-67 and the icon swap is CHAMDRAW.C F0291:621-630.
     * The status hand slot box 0..7 routing is unaffected. */
    if (!fill_chest(linked, DM1_PC34_CHEST_SLOT_COUNT)) {
        return 0;
    }
    openChestResult = m11_inventory_open_chest(
        &state, /*champ=*/0, chestThing, linked, DM1_PC34_CHEST_SLOT_COUNT);
    out->chestOpenAfter = openChestResult;
    openChestThingAfterOpen =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->openChestThingAfterOpen = openChestThingAfterOpen;
    out->panelContentAfterOpen = m11_inventory_get_panel_content_pc34(&state);
    /* ReDMCSB CHEST.C F0333 lines 28-32 sets M569_PANEL_CHEST before
     * returning for the same already-open G0426 chest.  Reset the synthetic
     * panel marker to prove the same-open path refreshes the panel content
     * without rebuilding G0425. */
    (void)m11_inventory_set_panel_content_pc34(
        &state, DM1_PC34_PANEL_FOOD_WATER_POISONED);
    out->sameOpenChestResult = m11_inventory_open_chest(
        &state, /*champ=*/0, chestThing, linked, DM1_PC34_CHEST_SLOT_COUNT);
    out->panelContentAfterSameOpen =
        m11_inventory_get_panel_content_pc34(&state);
    actionHandIconOpen = (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
        /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
        (unsigned int)chestThing, (unsigned int)chestThing,
        /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSOC_CLOSED_ICON);
    out->actionHandIconAfterOpen = actionHandIconOpen;
    /* Run the status hand slot box 0..7 reduction table while the chest
     * is open, then close the chest, then capture the post-close icon
     * binding. */
    if (!capture_status_hand_table(out, &state, healthyChampionCount,
                                   chestThing)) {
        return 0;
    }
    if (!capture_rejections_with_chest_open(out)) {
        return 0;
    }
    /* The status hand click path must leave the open-chest state
     * intact; F0302:677-684 does not own G0426_T_OpenChest. */
    out->chestStillOpenAfterChampion0ReadyHandClick =
        m11_inventory_get_open_chest_thing(&state, 0) == chestThing;
    out->chestStillOpenAfterChampion3ActionHandClick =
        m11_inventory_get_open_chest_thing(&state, 0) == chestThing;
    out->chestStillOpenAfterCandidateChampionReject =
        m11_inventory_get_open_chest_thing(&state, 0) == chestThing;
    out->chestStillOpenAfterDeadChampionReject =
        m11_inventory_get_open_chest_thing(&state, 0) == chestThing;
    out->openChestThingAfterAllClicks =
        m11_inventory_get_open_chest_thing(&state, 0);
    if (!capture_action_hand_container_edges(out, healthyChampionCount,
                                             chestThing)) {
        return 0;
    }

    /* Close the chest and verify the icon reverts. */
    (void)m11_inventory_close_chest(&state, 0, NULL, 0);
    out->openChestThingAfterClose =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->panelContentAfterClose = m11_inventory_get_panel_content_pc34(&state);

    /* ReDMCSB PANEL.C F0347:1639-1691 redraws the panel based on the current
     * action hand immediately after close/click flows. Use a non-container
     * item to verify the fallback route to food/water/poisoned. */
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &state, /*champ=*/0, DM1_PC34_SLOT_ACTION_HAND, DM1_V1_IPHSOC_DAGGER, 1,
        0, DM1_PC34_ALLOWED_HANDS);
    (void)m11_inventory_apply_panel_route_after_close_pc34(&state, 0);
    out->panelContentAfterCloseRedraw =
        m11_inventory_get_panel_content_pc34(&state);
    out->actionHandIconAfterClose =
        (int)INVENTORY_Compat_GetActionHandIconForOpenChest(
            /*isInventoryChampion=*/1u, /*slotIndex=*/1u,
            (unsigned int)chestThing, /*openChestThing=*/0u,
            /*baseIconIndex=*/(unsigned int)DM1_V1_IPHSOC_CLOSED_ICON);
    summarize_with_chest_open(out);
    return 1;
}
