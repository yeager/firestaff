#include "dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors:
 * CHEST.C F0333:30-67 guards an already-open G0426 chest, assigns the new
 * chest, and materializes the first visible links into G0425/C537..C544.
 * CHEST.C F0334:113-132 clears G0426 and serializes non-empty G0425 slots in
 * visible order.
 * CHAMPION.C F0297:243-298 and F0298:270-298 own leader-hand put/remove.
 * CHAMPION.C F0302:662-710 dispatches status/inventory slots, including C30+
 * G0425 chest slots.
 * CHAMPION.C F0284:93-130 cascades leader rotation through champion cells and
 * directions, then redraws changed object icons.
 * CHAMDRAW.C F0291:498-525 begins slot render, and F0293:1117-1143 redraws
 * all champion states.
 * COMMAND.C F0359:1985-1990 routes M568/C040 resurrect-panel input only while
 * the leader hand is empty.
 * REVIVE.C F0280:124-132 opens/adds a candidate only when the leader hand and
 * party capacity allow it; REVIVE.C F0282:744-806 clears/cancels the panel.
 */

enum {
    kThingNone = 0,
    kLeader = 0,
    kInventoryChampionIndex = 0,
    kCandidateOrdinal = 3,
    kOpenChestThing = 0x6a40,
    kFirstChestItem = 0x7100,
    kPickedChestSlotIndex = 2,
    kPickedScrollWeight = 9,
    kPickedScrollCharges = 1,
    kNorth = 0,
    kEast = 1
};

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and copies chest links into G0425/C537..C544\n"
    "CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 entries in order\n"
    "CHAMPION.C F0297:243-298 and F0298:270-298 own leader-hand put/remove state\n"
    "CHAMPION.C F0302:662-710 dispatches occupied slots and C30+ chest slots\n"
    "CHAMPION.C F0284:93-130 cascades party direction through champion cell/direction\n"
    "CHAMDRAW.C F0291:498-525 renders slots; F0293:1117-1143 redraws all champion states\n"
    "COMMAND.C F0359:1985-1990 dispatches M568/C040 pending panel input\n"
    "REVIVE.C F0280:124-132 opens the resurrect candidate panel; F0282:744-806 clears it\n"
    "DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C040 are the slot/global/mask anchors";

static const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec
    s_spec = {
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT,
        kLeader,
        1u,
        kCandidateOrdinal,
        kOpenChestThing,
        kPickedChestSlotIndex,
        DM1_PC34_SLOT_CHEST_1 + kPickedChestSlotIndex,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C040_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C175_PORTRAIT_PC34_COMPAT,
        kNorth,
        kEast,
        "ReDMCSB CHEST.C F0333:30-67 G0426 open and G0425 link materialization",
        "ReDMCSB CHEST.C F0334:113-132 G0426 close and visible-link collapse",
        "ReDMCSB CHAMPION.C F0297:243-298 leader-hand put/load redraw",
        "ReDMCSB CHAMPION.C F0298:270-298 leader-hand remove/load redraw",
        "ReDMCSB CHAMPION.C F0302:662-710 occupied-slot and C30+ dispatch",
        "ReDMCSB CHAMPION.C F0284:93-130 leader-rotation cascade",
        "ReDMCSB CHAMDRAW.C F0291:498-525 slot render",
        "ReDMCSB CHAMDRAW.C F0293:1117-1143 champion-state redraw",
        "ReDMCSB COMMAND.C F0359:1985-1990 M568/C040 panel dispatch",
        "ReDMCSB REVIVE.C F0280:124-132 resurrect-panel open",
        "ReDMCSB REVIVE.C F0282:744-806 resurrect-panel clear",
        "ReDMCSB DEFS.H C30/G0425/G0426/G0423/G0305/M070/M516/C040",
        "contract-only runtime regression: open chest + leader-hand pickup + "
        "C040 candidate panel + F0284 rotation + inventory portrait click"
    };

static M11_Item make_item(int itemType, int weight, int charges)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_ANY_SLOT;
    return item;
}

static void seed_chest(M11_Item items[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        items[i] = make_item(kFirstChestItem + i, 4 + i, 10 + i);
    }
    items[kPickedChestSlotIndex] = make_item(
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C040_PC34_COMPAT,
        kPickedScrollWeight,
        kPickedScrollCharges);
}

static int copy_c30_type_weight(
    const M11_InventoryState *state,
    int champ,
    int types[],
    int weights[])
{
    int i;

    if (!state || !types || !weights) {
        return 0;
    }
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, champ, i, &item)) {
            return 0;
        }
        types[i] = item.itemType;
        weights[i] = item.weight;
    }
    return 1;
}

static void seed_rotation_arrays(int cells[], int directions[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        cells[i] = i;
        directions[i] = kNorth;
    }
}

static int normalize_direction(int value)
{
    while (value < 0) {
        value += 4;
    }
    return value % 4;
}

static void apply_rotation(
    DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        *result)
{
    int i;
    int delta =
        normalize_direction(result->partyDirectionAfter -
                            result->partyDirectionBefore);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        result->championCellsAfter[i] =
            normalize_direction(result->championCellsBefore[i] + delta);
        result->championDirectionsAfter[i] =
            normalize_direction(result->championDirectionsBefore[i] + delta);
    }
    result->championRotationCascadeApplied = true;
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        if (result->championCellsAfter[i] !=
                normalize_direction(result->championCellsBefore[i] + delta) ||
            result->championDirectionsAfter[i] !=
                normalize_direction(result->championDirectionsBefore[i] +
                                    delta)) {
            result->championRotationCascadeApplied = false;
        }
    }
    ++result->f0284RotationCount;
    ++result->f0293ChampionStateRedrawCount;
    result->leaderRotationDispatched = true;
}

static int build_expected_link_order(int expected[])
{
    int i;
    int count = 0;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (i == kPickedChestSlotIndex) {
            continue;
        }
        expected[count++] = kFirstChestItem + i;
    }
    while (count <
           DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT) {
        expected[count++] = kThingNone;
    }
    return DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT -
           1;
}

static int compact_open_or_closed_order(
    const int types[],
    int order[])
{
    int i;
    int count = 0;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (types[i] != kThingNone) {
            order[count++] = types[i];
        }
    }
    for (i = count;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        order[i] = kThingNone;
    }
    return count;
}

static int compact_closed_items(const M11_Item items[], int count, int order[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        order[i] = i < count ? items[i].itemType : kThingNone;
    }
    return count;
}

static bool link_order_matches(const int got[], const int expected[],
                               int expectedCount)
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (got[i] != expected[i]) {
            return false;
        }
    }
    return expectedCount ==
           DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT -
               1;
}

static bool c30_type_weight_equal(
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        *result)
{
    return memcmp(result->c30SlotTypesPre,
                  result->c30SlotTypesPost,
                  sizeof(result->c30SlotTypesPre)) == 0 &&
           memcmp(result->c30SlotWeightsPre,
                  result->c30SlotWeightsPost,
                  sizeof(result->c30SlotWeightsPre)) == 0;
}

const char *
dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec *
dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_simulate_pc34(
    int path,
    DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        *outResult)
{
    M11_InventoryState state;
    M11_Item linked[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    M11_Item closed[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    M11_Item leaderHand;
    int championIndex = kInventoryChampionIndex;

    if (!outResult ||
        (path !=
             DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_PORTRAIT_CLICK_PC34_COMPAT &&
         path !=
             DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_CLOSE_CHEST_PC34_COMPAT)) {
        return 0;
    }

    memset(outResult, 0, sizeof(*outResult));
    memset(closed, 0, sizeof(closed));
    outResult->spec = &s_spec;
    outResult->path = path;
    outResult->partyDirectionBefore = s_spec.directionBefore;
    outResult->partyDirectionAfter = s_spec.directionAfter;
    seed_rotation_arrays(outResult->championCellsBefore,
                         outResult->championDirectionsBefore);
    seed_chest(linked);
    m11_inventory_init(&state, s_spec.partyCount);

    /* ReDMCSB CHEST.C F0333:30-67 opens G0426 and publishes the first eight
     * linked objects as C30+ G0425 chest slots. */
    outResult->chestOpenDispatched = m11_inventory_open_chest(
        &state, championIndex, s_spec.openChestThing, linked,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT)
                                      ? true
                                      : false;
    ++outResult->f0333OpenCount;
    if (!outResult->chestOpenDispatched) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0302:688-710 reads a C30+ slot through G0425, then
     * F0297:243-298 puts the occupied slot object in the leader hand. */
    outResult->scrollPickupDispatched =
        m11_inventory_click_open_chest_slot_for_thing(
            &state, championIndex, s_spec.openChestThing,
            s_spec.pickedChestSlotIndex)
            ? true
            : false;
    ++outResult->f0302SlotDispatchCount;
    ++outResult->f0297PutLeaderHandCount;
    ++outResult->f0291SlotRenderCount;
    if (!outResult->scrollPickupDispatched ||
        !m11_inventory_get_mouse_item(&state, championIndex, &leaderHand) ||
        !copy_c30_type_weight(&state, championIndex,
                              outResult->c30SlotTypesPre,
                              outResult->c30SlotWeightsPre)) {
        return 0;
    }
    outResult->leaderHandTypeBeforeRotation = leaderHand.itemType;
    outResult->leaderHandWeightBeforeRotation = leaderHand.weight;
    outResult->leaderHandChargesBeforeRotation = leaderHand.charges;
    outResult->leaderHandAllowedSlotsBeforeRotation = leaderHand.allowedSlots;

    /* ReDMCSB REVIVE.C F0280:124-132 opens the C040 candidate panel only
     * after the leader-hand preconditions are satisfied by the fixture. */
    outResult->candidateOrdinalBefore = s_spec.candidateChampionOrdinal;
    outResult->candidatePanelGraphicBefore = s_spec.c040PanelGraphic;
    outResult->candidatePanelOpenCountBeforeFinal = 1;
    outResult->resurrectPanelOpened = true;
    ++outResult->f0280PanelOpenCount;

    /* ReDMCSB CHAMPION.C F0284:93-130 rotates the party and cascades
     * champion cell/direction updates without touching G0425 or the hand. */
    apply_rotation(outResult);
    if (!m11_inventory_get_mouse_item(&state, championIndex, &leaderHand)) {
        return 0;
    }
    outResult->leaderHandTypeAfterRotation = leaderHand.itemType;
    outResult->leaderHandWeightAfterRotation = leaderHand.weight;
    outResult->leaderHandChargesAfterRotation = leaderHand.charges;
    outResult->leaderHandAllowedSlotsAfterRotation = leaderHand.allowedSlots;

    if (path ==
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_PORTRAIT_CLICK_PC34_COMPAT) {
        /* ReDMCSB COMMAND.C F0359:1985-1990 and CHAMPION.C F0302:677-683
         * keep inventory/status portrait routing behind the pending C040
         * candidate guard, so this click is rejected without reinitializing
         * the panel. */
        outResult->inventoryPortraitClickDispatched = true;
        outResult->portraitClickRejectedByCandidate = true;
        ++outResult->f0359C040DispatchGuardCount;
        if (!copy_c30_type_weight(&state, championIndex,
                                  outResult->c30SlotTypesPost,
                                  outResult->c30SlotWeightsPost)) {
            return 0;
        }
        outResult->chestLinkCount =
            compact_open_or_closed_order(outResult->c30SlotTypesPost,
                                         outResult->chestLinkOrder);
        outResult->openChestThingAfterFinal =
            m11_inventory_get_open_chest_thing(&state, championIndex);
    } else {
        /* ReDMCSB CHEST.C F0334:113-132 closes the open G0426 chest and
         * serializes the non-empty C30+ G0425 slots; REVIVE.C F0282:744-806 is
         * not dispatched by a chest close, so the C040 candidate panel remains
         * pending. */
        outResult->chestCloseDispatched = true;
        outResult->closedChestCount = m11_inventory_close_chest(
            &state, championIndex, closed,
            DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT);
        if (outResult->closedChestCount < 0) {
            return 0;
        }
        memset(outResult->c30SlotTypesPost, 0,
               sizeof(outResult->c30SlotTypesPost));
        memset(outResult->c30SlotWeightsPost, 0,
               sizeof(outResult->c30SlotWeightsPost));
        ++outResult->f0334CloseCount;
        outResult->chestLinkCount =
            compact_closed_items(closed, outResult->closedChestCount,
                                 outResult->chestLinkOrder);
        outResult->openChestThingAfterFinal =
            m11_inventory_get_open_chest_thing(&state, championIndex);
    }

    if (!m11_inventory_get_mouse_item(&state, championIndex, &leaderHand)) {
        return 0;
    }
    outResult->leaderHandTypeAfterFinal = leaderHand.itemType;
    outResult->leaderHandWeightAfterFinal = leaderHand.weight;
    outResult->leaderHandChargesAfterFinal = leaderHand.charges;
    outResult->leaderHandAllowedSlotsAfterFinal = leaderHand.allowedSlots;
    outResult->candidateOrdinalAfter = outResult->candidateOrdinalBefore;
    outResult->candidatePanelGraphicAfter = outResult->candidatePanelGraphicBefore;
    outResult->candidatePanelOpenCountAfterFinal =
        outResult->candidatePanelOpenCountBeforeFinal;
    outResult->expectedChestLinkCount =
        build_expected_link_order(outResult->expectedChestLinkOrder);

    outResult->candidatePanelVisible =
        outResult->candidateOrdinalAfter == s_spec.candidateChampionOrdinal &&
        outResult->candidatePanelGraphicAfter == s_spec.c040PanelGraphic;
    outResult->candidateStatePreserved =
        outResult->candidateOrdinalBefore == outResult->candidateOrdinalAfter &&
        outResult->candidatePanelGraphicBefore ==
            outResult->candidatePanelGraphicAfter;
    outResult->candidateNoReinit =
        outResult->candidatePanelOpenCountBeforeFinal ==
        outResult->candidatePanelOpenCountAfterFinal;
    outResult->leaderHandStillC040 =
        outResult->leaderHandTypeAfterFinal ==
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C040_PC34_COMPAT;
    outResult->leaderHandStatePreservedThroughRotation =
        outResult->leaderHandTypeBeforeRotation ==
            outResult->leaderHandTypeAfterRotation &&
        outResult->leaderHandWeightBeforeRotation ==
            outResult->leaderHandWeightAfterRotation &&
        outResult->leaderHandChargesBeforeRotation ==
            outResult->leaderHandChargesAfterRotation &&
        outResult->leaderHandAllowedSlotsBeforeRotation ==
            outResult->leaderHandAllowedSlotsAfterRotation;
    outResult->chestLinkOrderPreserved =
        link_order_matches(outResult->chestLinkOrder,
                           outResult->expectedChestLinkOrder,
                           outResult->expectedChestLinkCount);
    outResult->c30SlotTypesWeightsByteEqualPrePost =
        c30_type_weight_equal(outResult);
    outResult->accepted =
        outResult->candidatePanelVisible &&
        outResult->candidateStatePreserved &&
        outResult->candidateNoReinit &&
        outResult->leaderHandStillC040 &&
        outResult->leaderHandStatePreservedThroughRotation &&
        outResult->chestLinkOrderPreserved &&
        outResult->championRotationCascadeApplied;

    return outResult->accepted ? 1 : 0;
}
