#include "dm1_v1_chest_multi_champion_close_pc34_compat.h"

#include <string.h>

/*
 * Non-overlap: existing gates cover same-leader reopen order, cross-champion
 * reopen hand transfer, hidden ninth-tail truncation, stale clicks, and a
 * single-leader encumbrance close. This gate keeps one deterministic three
 * champion party, mutates C537/C540/C544 for different champions, then closes
 * each chest to prove per-champion close weight and identity isolation.
 */

enum {
    CHEST_THING_A = 0x7101,
    CHEST_THING_B = 0x7102,
    CHEST_THING_C = 0x7103,
    BASE_ITEM_A = 0x7201,
    BASE_ITEM_B = 0x7202,
    BASE_ITEM_C = 0x7203,
    HAND_ITEM_A = 0x7301,
    HAND_ITEM_B = 0x7302,
    HAND_ITEM_C = 0x7303,
    LINK_ITEM_A = 0x7401,
    LINK_ITEM_B = 0x7501,
    LINK_ITEM_C = 0x7601
};

static const int kBaseWeights[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT] = {
    19, 29, 37
};

static const int kHandTypes[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT] = {
    HAND_ITEM_A, HAND_ITEM_B, HAND_ITEM_C
};

static const int kHandWeights[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT] = {
    41, 43, 47
};

static const int kChestThings[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT] = {
    CHEST_THING_A, CHEST_THING_B, CHEST_THING_C
};

static const int kTargetSlots[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT] = {
    DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_SLOT_CHEST_4,
    DM1_PC34_SLOT_CHEST_8
};

static const int kMaximumLoads[DM1_PC34_CHEST_MULTI_CHAMPION_COUNT] = {
    60, 100, 130
};

static const DM1_V1_ChestMultiChampionCloseSpecPc34
    s_spec = {
        "Source-locked deterministic contract gate only; no real-asset icon or DOS pixel parity claim.",
        DM1_PC34_CHEST_MULTI_CHAMPION_COUNT,
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_4,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT,
        DM1_PC34_ALLOWED_QUIVER_LINE1
    };

static M11_Item make_item(int itemType, int weight, int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static void seed_linked_items(int championIndex,
                              M11_Item* linked,
                              int* outCount)
{
    static const int weightsA[] = { 5, 7 };
    static const int weightsB[] = { 11, 13, 17, 19 };
    static const int weightsC[] = { 2, 3, 5, 7, 11, 13, 17, 23 };
    const int* weights = weightsA;
    int firstType = LINK_ITEM_A;
    int count = 2;
    int i;

    if (championIndex == 1) {
        weights = weightsB;
        firstType = LINK_ITEM_B;
        count = 4;
    } else if (championIndex == 2) {
        weights = weightsC;
        firstType = LINK_ITEM_C;
        count = 8;
    }

    for (i = 0; i < count; ++i) {
        linked[i] = make_item(firstType + i, weights[i],
                              DM1_PC34_ALLOWED_CONTAINER);
    }
    *outCount = count;
}

static int movement_ticks_for_load(int load, int maximumLoad)
{
    if (maximumLoad > load) {
        return ((load << 3) > (maximumLoad * 5)) ? 3 : 2;
    }
    return 4 + (((load - maximumLoad) << 2) / maximumLoad);
}

static int contains_type(const M11_Item* items, int count, int itemType)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (items[i].itemType == itemType) {
            return 1;
        }
    }
    return 0;
}

static void copy_closed_items(const M11_Item* closed,
                              int championIndex,
                              DM1_V1_ChestMultiChampionCloseProbePc34* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT; ++i) {
        out->closedTypes[championIndex][i] = closed[i].itemType;
        out->closedWeights[championIndex][i] = closed[i].weight;
    }
}

static int run_champion_case(
    M11_InventoryState* state,
    int championIndex,
    DM1_V1_ChestMultiChampionCloseProbePc34* out)
{
    M11_Item linked[DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT];
    M11_Item item;
    int linkedCount = 0;
    int targetSlot = kTargetSlots[championIndex];
    int targetIndex = targetSlot - DM1_PC34_SLOT_CHEST_1;

    memset(linked, 0, sizeof(linked));
    memset(closed, 0, sizeof(closed));
    seed_linked_items(championIndex, linked, &linkedCount);

    out->targetPc34Slots[championIndex] = targetSlot;
    out->targetSlotIndexes[championIndex] = targetIndex;
    out->linkedCounts[championIndex] = linkedCount;

    out->openResults[championIndex] = m11_inventory_open_chest(
        state, championIndex, kChestThings[championIndex], linked,
        linkedCount);
    if (!out->openResults[championIndex]) {
        return 0;
    }
    out->openThings[championIndex] =
        m11_inventory_get_open_chest_thing(state, championIndex);
    out->visibleWeightsAfterOpen[championIndex] =
        m11_inventory_pc34_open_chest_visible_contents_weight(
            state, championIndex);
    out->loadsAfterOpen[championIndex] =
        m11_inventory_get_load(state, championIndex);
    out->movementTicksAfterOpen[championIndex] = movement_ticks_for_load(
        out->loadsAfterOpen[championIndex], kMaximumLoads[championIndex]);

    out->handBeforeTypes[championIndex] = kHandTypes[championIndex];
    out->handBeforeWeights[championIndex] = kHandWeights[championIndex];
    if (!m11_inventory_set_mouse_item(
            state, championIndex, kHandTypes[championIndex],
            kHandWeights[championIndex], 0, DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }
    if (!m11_inventory_get_item_in_chest_slot(
            state, championIndex, targetIndex, &item)) {
        return 0;
    }
    out->displacedTypes[championIndex] = item.itemType;
    out->displacedWeights[championIndex] = item.weight;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 validates C537/C540/C544
     * against DATA.C G0038 slot masks, then uses F0297/F0300/F0301 weight
     * deltas while swapping the leader hand with the open G0425 chest slot. */
    out->clickResults[championIndex] = m11_inventory_click_pc34_source_slot(
        state, championIndex, targetSlot);
    if (!out->clickResults[championIndex]) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(state, championIndex, &item)) {
        return 0;
    }
    out->handAfterTypes[championIndex] = item.itemType;
    if (!m11_inventory_get_item_in_chest_slot(
            state, championIndex, targetIndex, &item)) {
        return 0;
    }
    out->slotAfterTypes[championIndex] = item.itemType;
    out->visibleWeightsAfterClick[championIndex] =
        m11_inventory_pc34_open_chest_visible_contents_weight(
            state, championIndex);
    out->loadsAfterClick[championIndex] =
        m11_inventory_get_load(state, championIndex);
    out->movementTicksAfterClick[championIndex] = movement_ticks_for_load(
        out->loadsAfterClick[championIndex], kMaximumLoads[championIndex]);

    /* ReDMCSB CHEST.C F0334 lines 117-132 rewrites the container from
     * non-empty G0425 slots; DUNGEON.C F0140 lines 1114-1120 requires the
     * close snapshot to be base 50 plus the champion's own visible contents. */
    out->closeCounts[championIndex] =
        m11_inventory_pc34_close_chest_with_weight_snapshot(
            state, championIndex, closed,
            DM1_PC34_CHEST_MULTI_CHAMPION_SLOT_COUNT,
            &out->closeContainerSnapshots[championIndex]);
    if (out->closeCounts[championIndex] < 0) {
        return 0;
    }
    copy_closed_items(closed, championIndex, out);
    out->openThingsAfterClose[championIndex] =
        m11_inventory_get_open_chest_thing(state, championIndex);
    out->loadsAfterClose[championIndex] =
        m11_inventory_get_load(state, championIndex);
    out->movementTicksAfterClose[championIndex] = movement_ticks_for_load(
        out->loadsAfterClose[championIndex], kMaximumLoads[championIndex]);
    out->replacementClosedAtTarget[championIndex] =
        closed[targetIndex].itemType == kHandTypes[championIndex];
    out->displacedAbsentFromClosed[championIndex] =
        contains_type(closed, out->closeCounts[championIndex],
                      out->displacedTypes[championIndex]) ? 0 : 1;

    return 1;
}

const char* dm1_v1_chest_multi_champion_close_source_evidence_pc34(void)
{
    return
        "CHEST.C F0333:31-67 opens the selected chest and copies linked contents into G0425 C537-C544 slots\n"
        "CHEST.C F0334:117-132 clears G0426 and rewrites the container links from non-empty G0425 slots\n"
        "DUNGEON.C F0140:1114-1120 gives containers base weight 50 plus linked contents; DUNGEON.C:108 gives Staff Of Claws AllowedSlots 0x0040\n"
        "DUNGEON.C F0163:1796-1837 clears Next and appends relinked close contents in order\n"
        "CHAMPION.C F0297:243-268, F0300:489-584, F0301:587-615 adjust per-champion Load for hand/slot moves\n"
        "CHAMPION.C F0302:688-710 rejects incompatible leader-hand items and swaps accepted C30+ chest slots\n"
        "CHAMPION.C F0309/F0310:1157-1205 consumes Load for maximum-load/movement tick encumbrance\n"
        "DATA.C G0038:1049-1087 defines C537-C544 chest slots as MASK0x0400_CONTAINER";
}

const DM1_V1_ChestMultiChampionCloseSpecPc34*
dm1_v1_chest_multi_champion_close_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_multi_champion_close_run_pc34(
    DM1_V1_ChestMultiChampionCloseProbePc34* out)
{
    M11_InventoryState state;
    M11_Item staff;
    int championIndex;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    out->contractOnly = 1;
    out->championCount = DM1_PC34_CHEST_MULTI_CHAMPION_COUNT;
    out->c537Mask = m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_CHEST_1);
    out->c540Mask = m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_CHEST_4);
    out->c544Mask = m11_inventory_pc34_slot_mask(DM1_PC34_SLOT_CHEST_8);
    out->staffOfClawsAllowedSlots = DM1_PC34_ALLOWED_QUIVER_LINE1;
    staff = make_item(DM1_PC34_CHEST_MULTI_CHAMPION_STAFF_OF_CLAWS_INFO,
                      8, DM1_PC34_ALLOWED_QUIVER_LINE1);
    out->staffRejectedFromChest =
        m11_inventory_can_equip(&staff, DM1_PC34_SLOT_CHEST_1) ? 0 : 1;
    out->staffAcceptedInQuiver =
        m11_inventory_can_equip(&staff, DM1_PC34_SLOT_QUIVER_LINE1_1);

    m11_inventory_init(&state, DM1_PC34_CHEST_MULTI_CHAMPION_COUNT);
    for (championIndex = 0;
         championIndex < DM1_PC34_CHEST_MULTI_CHAMPION_COUNT;
         ++championIndex) {
        if (!m11_inventory_set_item_in_pc34_source_slot(
                &state, championIndex, DM1_PC34_SLOT_BACKPACK_LINE1_1,
                BASE_ITEM_A + championIndex, kBaseWeights[championIndex],
                0, DM1_PC34_ALLOWED_ANY_SLOT)) {
            return 0;
        }
        out->baseLoads[championIndex] =
            m11_inventory_get_load(&state, championIndex);
    }

    for (championIndex = 0;
         championIndex < DM1_PC34_CHEST_MULTI_CHAMPION_COUNT;
         ++championIndex) {
        if (!run_champion_case(&state, championIndex, out)) {
            return 0;
        }
    }

    return 1;
}
