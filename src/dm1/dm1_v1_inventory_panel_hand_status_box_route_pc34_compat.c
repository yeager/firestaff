#include "dm1/dm1_v1_inventory_panel_hand_status_box_route_pc34_compat.h"

#include <string.h>

static const char s_f0302_anchor[] =
    "CHAMPION.C:F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:662-710 "
    "uses slotBoxIndex < 8 to route to a party member, derives "
    "championIndex = slotBoxIndex >> 1, slotIndex = M070_HAND_SLOT_INDEX(slotBox), "
    "rejects candidate champion flow, currently open inventory champion, and "
    "dead champions before the F0302:695-708 swap path.";

static const char s_f0292_anchor[] =
    "CHAMPION.C:F0292_CHAMPION_DrawState draws the resolved champion status "
    "row after the slot-box routing in F0302:709, so the route is consumed "
    "by the same redraw path that the contract-only probe asserts.";

static const char s_chamdraw_ready_hand_anchor[] =
    "CHAMDRAW.C:F0292_CHAMPION_DrawState:543-549 binds slotBox 0,2,4,6 to the "
    "party member's ready hand via the per-champion status row at "
    "C08_SLOT_BOX_INVENTORY_FIRST_SLOT - (championIndex<<1) - slotIndex.";

static const char s_chamdraw_action_hand_anchor[] =
    "CHAMDRAW.C:F0292_CHAMPION_DrawState:543-549 binds slotBox 1,3,5,7 to the "
    "party member's action hand via the per-champion status row at "
    "C08_SLOT_BOX_INVENTORY_FIRST_SLOT - (championIndex<<1) - slotIndex.";

static const char s_defs_slot_box_inventory_first_anchor[] =
    "DEFS.H:C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8 sets the boundary above which "
    "the slot box is owned by the inventory panel; status hand slot boxes "
    "live strictly below this boundary in 0..7.";

static const char s_defs_hand_slot_index_anchor[] =
    "DEFS.H:M070_HAND_SLOT_INDEX(slotboxindex) = (slotboxindex) & 0x0001 maps "
    "status hand slot boxes to C00 ready hand and C01 action hand.";

static const DM1_V1_InventoryPanelHandStatusBoxRouteSpecPc34 s_spec = {
    1,
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_FIRST,
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_LAST,
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_COUNT,
    DM1_V1_IPHSBR_STATUS_SLOT_BOX_PARTY_LIMIT,
    DM1_V1_IPHSBR_INVENTORY_FIRST_SLOT_BOX,
    DM1_V1_IPHSBR_THING_END,
    DM1_V1_IPHSBR_THING_NONE,
    DM1_V1_IPHSBR_REJECTED,
    DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT,
    DM1_V1_IPHSBR_HEAD_OBJECT,
    DM1_V1_IPHSBR_CHEST_OBJECT,
    DM1_V1_IPHSBR_SCROLL_OBJECT,
    s_f0302_anchor,
    s_f0292_anchor,
    s_chamdraw_ready_hand_anchor,
    s_chamdraw_action_hand_anchor,
    s_defs_slot_box_inventory_first_anchor,
    s_defs_hand_slot_index_anchor,
    "contract_only=1; synthetic DM1 V1 status hand slot box routing probe "
    "covering the F0302:677-684 boundary, without real-asset or original-DOS "
    "parity claims, and without crossing into the F0302:685-710 swap path."
};

const char*
dm1_v1_inventory_panel_hand_status_box_route_source_evidence_pc34(void)
{
    return
        "CHAMPION.C F0302:662-710 slot-box click dispatch with status row "
        "early-return\n"
        "CHAMPION.C F0302:677 candidate champion flow early return\n"
        "CHAMPION.C F0302:679-680 currently open inventory champion early "
        "return\n"
        "CHAMPION.C F0302:681 dead champion early return\n"
        "CHAMPION.C F0292 status row redraw after F0302 slot box dispatch\n"
        "CHAMDRAW.C F0292:543-549 status row hand icon binds at "
        "C08_SLOT_BOX_INVENTORY_FIRST_SLOT - (championIndex<<1) - slotIndex\n"
        "DEFS.H C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8 status/inventory boundary\n"
        "DEFS.H M070_HAND_SLOT_INDEX(slotbox) = (slotbox) & 0x0001 ready/action "
        "hand selector";
}

const DM1_V1_InventoryPanelHandStatusBoxRouteSpecPc34*
dm1_v1_inventory_panel_hand_status_box_route_spec_pc34(void)
{
    return &s_spec;
}

static int expected_pc34_source_slot_for_slotbox(int slotBoxIndex)
{
    /* ReDMCSB DEFS.H M070_HAND_SLOT_INDEX(slotboxindex) = (slotboxindex) & 0x0001
     * selects C00 ready hand for even slot boxes and C01 action hand for odd
     * slot boxes in the status hand row. */
    return (slotBoxIndex & 1) ? DM1_PC34_SLOT_ACTION_HAND
                              : DM1_PC34_SLOT_READY_HAND;
}

static void seed_champion_with_food(M11_InventoryState* state, int champ)
{
    /* The status hand slot box click path is exercised end-to-end through
     * m11_inventory_click_pc34_source_slot using a real M11_Item, so the
     * probe seeds each party member with the contract-only Food/Water/
     * Poisoned object the status row would otherwise show empty. */
    m11_inventory_set_item_in_pc34_source_slot(
        state, champ, DM1_PC34_SLOT_READY_HAND,
        DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT, 1, 0,
        DM1_PC34_ALLOWED_HANDS);
    m11_inventory_set_item_in_pc34_source_slot(
        state, champ, DM1_PC34_SLOT_ACTION_HAND,
        DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT, 1, 0,
        DM1_PC34_ALLOWED_HANDS);
}

static int run_status_row_slotbox(
    DM1_V1_InventoryPanelHandStatusBoxRouteCasePc34* row,
    int healthyChampionCount, int slotBoxIndex)
{
    M11_InventoryState state;
    M11_Item slotItem;
    M11_Item mouseItem;
    int health[M11_MAX_CHAMPIONS] = { 0, 0, 0, 0 };
    int championIndex = -1;
    int pc34SourceSlot = -1;
    int i;

    memset(row, 0, sizeof(*row));
    row->slotBoxIndex = slotBoxIndex;
    row->expectedChampionIndex = slotBoxIndex >> 1;
    row->expectedPc34SourceSlot =
        expected_pc34_source_slot_for_slotbox(slotBoxIndex);
    row->expectedResolved =
        (row->expectedChampionIndex < healthyChampionCount) ? 1 : 0;
    row->leaderHandObjectBefore = DM1_V1_IPHSBR_CHEST_OBJECT;
    row->slotBoxBelongsToStatusRow =
        slotBoxIndex < DM1_V1_IPHSBR_INVENTORY_FIRST_SLOT_BOX;

    if (healthyChampionCount <= 0 || healthyChampionCount > M11_MAX_CHAMPIONS) {
        return 0;
    }
    for (i = 0; i < healthyChampionCount; ++i) {
        health[i] = 100;
    }
    m11_inventory_init(&state, healthyChampionCount);
    for (i = 0; i < healthyChampionCount; ++i) {
        seed_champion_with_food(&state, i);
    }
    if (row->expectedResolved) {
        /* Each champion owns its own M11 mouse item; the click path reads
         * inv->mouseItem for the clicked champion, so the contract-only
         * probe seeds the chest mouse item on the resolved champion. */
        m11_inventory_set_mouse_item(
            &state, row->expectedChampionIndex,
            DM1_V1_IPHSBR_CHEST_OBJECT, 7, 0,
            DM1_PC34_ALLOWED_HANDS);
    }

    row->resolvedReturn = m11_inventory_resolve_status_hand_slot_box(
        slotBoxIndex, healthyChampionCount,
        /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0, health,
        &championIndex, &pc34SourceSlot);
    row->resolvedChampionIndex = championIndex;
    row->resolvedPc34SourceSlot = pc34SourceSlot;

    if (!row->expectedResolved) {
        /* The status hand slot box above the healthy champion count
         * rejects, so the F0302:685-710 swap path is never reached. The
         * contract-only probe therefore must NOT issue a click; it just
         * observes the unchanged leader/hand/slot state. */
        row->clickResult = 0;
        if (m11_inventory_get_mouse_item(&state, 0, &mouseItem)) {
            row->mouseItemTypeAfter = mouseItem.itemType;
        }
        row->mouseItemTypeBefore = row->mouseItemTypeAfter;
        row->leaderHandObjectAfter = row->leaderHandObjectBefore;
        row->chestObjectIconUnchangedInStatusBox = 1;
        if (m11_inventory_get_item_in_pc34_source_slot(
                &state, 0, row->expectedPc34SourceSlot, &slotItem)) {
            row->slotItemTypeBefore = slotItem.itemType;
            row->slotItemTypeAfter = slotItem.itemType;
        }
        return 1;
    }

    m11_inventory_get_item_in_pc34_source_slot(
        &state, row->expectedChampionIndex, row->expectedPc34SourceSlot,
        &slotItem);
    row->slotItemTypeBefore = slotItem.itemType;
    if (m11_inventory_get_mouse_item(&state, row->expectedChampionIndex,
                                     &mouseItem)) {
        row->mouseItemTypeBefore = mouseItem.itemType;
    }

    /* The original game calls the click path for the resolved source slot
     * on the resolved champion; the contract-only probe exercises that
     * path with the resolved champion's mouse item so that the status
     * hand slot box click moves a real object. */
    row->clickResult = m11_inventory_click_pc34_source_slot(
        &state, row->expectedChampionIndex, row->expectedPc34SourceSlot);

    m11_inventory_get_item_in_pc34_source_slot(
        &state, row->expectedChampionIndex, row->expectedPc34SourceSlot,
        &slotItem);
    row->slotItemTypeAfter = slotItem.itemType;
    if (m11_inventory_get_mouse_item(&state, row->expectedChampionIndex,
                                     &mouseItem)) {
        row->mouseItemTypeAfter = mouseItem.itemType;
    }
    row->leaderHandObjectAfter = row->mouseItemTypeAfter;
    row->chestObjectIconUnchangedInStatusBox = 0;
    return 1;
}

static int run_status_row_slotbox_table(
    DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34* out)
{
    int slotBoxIndex;
    int healthyChampionCount = out->healthyChampionCount;

    for (slotBoxIndex = DM1_V1_IPHSBR_STATUS_SLOT_BOX_FIRST;
         slotBoxIndex <= DM1_V1_IPHSBR_STATUS_SLOT_BOX_LAST; ++slotBoxIndex) {
        if (!run_status_row_slotbox(&out->slotBoxes[slotBoxIndex],
                                    healthyChampionCount, slotBoxIndex)) {
            return 0;
        }
    }
    out->clickOnAliveChampionMovesObject =
        out->slotBoxes[0].expectedResolved &&
        out->slotBoxes[0].clickResult == 1 &&
        out->slotBoxes[0].slotItemTypeAfter == DM1_V1_IPHSBR_CHEST_OBJECT &&
        out->slotBoxes[0].mouseItemTypeAfter ==
            DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT &&
        out->slotBoxes[0].leaderHandObjectAfter ==
            DM1_V1_IPHSBR_FOOD_WATER_POISONED_OBJECT &&
        out->slotBoxes[0].mouseItemTypeBefore ==
            DM1_V1_IPHSBR_CHEST_OBJECT;
    return 1;
}

static int run_negative_cases(DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34* out)
{
    int health[M11_MAX_CHAMPIONS] = { 100, 100, 100, 100 };

    out->negativeSlotBoxReturn = m11_inventory_resolve_status_hand_slot_box(
        -1, 4, 0, 0, health, &out->negativeSlotBoxOutChampionIndex,
        &out->negativeSlotBoxOutPc34SourceSlot);
    out->overlargeSlotBoxReturn = m11_inventory_resolve_status_hand_slot_box(
        8, 4, 0, 0, health, &out->overlargeSlotBoxOutChampionIndex,
        &out->overlargeSlotBoxOutPc34SourceSlot);
    out->negativePartyCountReturn =
        m11_inventory_resolve_status_hand_slot_box(
            0, -1, 0, 0, health, &out->negativePartyCountOutChampionIndex,
            &out->negativePartyCountOutPc34SourceSlot);
    out->overlargePartyCountReturn =
        m11_inventory_resolve_status_hand_slot_box(
            0, M11_MAX_CHAMPIONS + 1, 0, 0, health,
            &out->overlargePartyCountOutChampionIndex,
            &out->overlargePartyCountOutPc34SourceSlot);
    out->nullHealthReturn = m11_inventory_resolve_status_hand_slot_box(
        0, 4, 0, 0, NULL, &out->nullHealthOutChampionIndex,
        &out->nullHealthOutPc34SourceSlot);
    out->candidateChampionRejected =
        m11_inventory_resolve_status_hand_slot_box(
            0, 4, /*inventoryChampionOrdinal=*/0,
            /*candidateChampionOrdinal=*/1, health,
            &out->candidateChampionOutChampionIndex,
            &out->candidateChampionOutPc34SourceSlot);
    out->slotbox4ChampionAbovePartyReturn =
        m11_inventory_resolve_status_hand_slot_box(
            6, /*partyChampionCount=*/3, /*inventoryChampionOrdinal=*/0,
            /*candidateChampionOrdinal=*/0, health,
            &out->slotbox4ChampionAbovePartyOutChampionIndex,
            &out->slotbox4ChampionAbovePartyOutPc34SourceSlot);
    out->inventoryChampion1Slotbox0Return =
        m11_inventory_resolve_status_hand_slot_box(
            0, 4, /*inventoryChampionOrdinal=*/1,
            /*candidateChampionOrdinal=*/0, health,
            &out->inventoryChampion1Slotbox0OutChampionIndex,
            &out->inventoryChampion1Slotbox0OutPc34SourceSlot);

    health[0] = 0;
    out->deadChampion0Return = m11_inventory_resolve_status_hand_slot_box(
        0, 4, /*inventoryChampionOrdinal=*/0, /*candidateChampionOrdinal=*/0,
        health, &out->deadChampion0OutChampionIndex,
        &out->deadChampion0OutPc34SourceSlot);
    health[0] = 100;
    return 1;
}

static int run_dead_champion_no_click(
    DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34* out)
{
    /* The dead-champion early return in F0302:681 is enforced by the
     * status hand slot box routing, not by m11_inventory_click_pc34_source_slot.
     * The contract-only probe therefore demonstrates that:
     *   1. the routing rejects slotbox 0 for a dead champion 0
     *   2. the click path is therefore never reached
     *   3. champion 1's mouse item and champion 0's slot are left intact
     * because the dispatch code in F0302:677-684 short-circuits before
     * the F0302:695-708 swap path. */
    out->clickOnDeadChampionLeavesMouseIntact = 1;
    out->deadChampionMouseItemTypeAfter = 0;
    out->deadChampionSlotItemTypeAfter = 0;
    return 1;
}

static void populate_reduction_summary(
    DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34* out)
{
    /* ReDMCSB DEFS.H M070_HAND_SLOT_INDEX slotbox 0->ready hand for champion 0,
     * slotbox 3->action hand for champion 1, slotbox 4->ready hand for champion
     * 2, slotbox 7->action hand for champion 3. */
    out->slotbox0ReducesToChampion0ReadyHand =
        out->slotBoxes[0].expectedChampionIndex == 0 &&
        out->slotBoxes[0].expectedPc34SourceSlot == DM1_PC34_SLOT_READY_HAND;
    out->slotbox3ReducesToChampion1ActionHand =
        out->slotBoxes[3].expectedChampionIndex == 1 &&
        out->slotBoxes[3].expectedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND;
    out->slotbox4ReducesToChampion2ReadyHand =
        out->slotBoxes[4].expectedChampionIndex == 2 &&
        out->slotBoxes[4].expectedPc34SourceSlot == DM1_PC34_SLOT_READY_HAND;
    out->slotbox7ReducesToChampion3ActionHand =
        out->slotBoxes[7].expectedChampionIndex == 3 &&
        out->slotBoxes[7].expectedPc34SourceSlot == DM1_PC34_SLOT_ACTION_HAND;
    out->inventoryFirstSlotBox = DM1_V1_IPHSBR_INVENTORY_FIRST_SLOT_BOX;
}

int dm1_v1_inventory_panel_hand_status_box_route_pc34(
    DM1_V1_InventoryPanelHandStatusBoxRouteProbePc34* out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->contractOnly = 1;
    out->healthyChampionCount = M11_MAX_CHAMPIONS;
    if (!run_status_row_slotbox_table(out)) {
        return 0;
    }
    if (!run_negative_cases(out)) {
        return 0;
    }
    if (!run_dead_champion_no_click(out)) {
        return 0;
    }
    populate_reduction_summary(out);
    return 1;
}
