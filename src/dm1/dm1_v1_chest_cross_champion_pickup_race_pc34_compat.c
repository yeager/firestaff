#include "dm1_v1_chest_cross_champion_pickup_race_pc34_compat.h"

#include <string.h>

/*
 * Non-overlap: existing chest gates cover capacity rejection, close/reopen
 * ordering, occupied-slot swaps, cross-champion reopen, and multi-champion
 * close isolation. This gate models a two-leader race for one visible G0425
 * chest slot: the first empty hand wins, then the second empty hand reaches
 * CHAMPION.C F0302 lines 694-695's empty-hand/empty-slot no-op.
 */

typedef struct {
    M11_Item chestSlots[DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT];
    M11_Item leaderHands[DM1_PC34_CHEST_PICKUP_RACE_CHAMPION_COUNT];
    int openChestThing;
} RaceModelPc34;

static const DM1_V1_ChestCrossChampionPickupRaceSpecPc34 s_spec = {
    "Source-locked deterministic contract gate only; no real-asset icon or DOS pixel parity claim.",
    DM1_PC34_CHEST_PICKUP_RACE_CHAMPION_COUNT,
    DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT,
    DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX,
    DM1_PC34_CHEST_PICKUP_RACE_PC34_SLOT,
    DM1_PC34_CHEST_PICKUP_RACE_CHEST_THING,
    DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM
};

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void clear_item(M11_Item* item)
{
    memset(item, 0, sizeof(*item));
}

static void copy_types(const M11_Item* items, int count, int* out)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT; ++i) {
        out[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static int count_visible(const M11_Item* items)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT; ++i) {
        if (items[i].itemType != 0) {
            ++count;
        }
    }
    return count;
}

static int count_type_in_items(const M11_Item* items, int count, int itemType)
{
    int result = 0;
    int i;

    for (i = 0; i < count; ++i) {
        if (items[i].itemType == itemType) {
            ++result;
        }
    }
    return result;
}

static int count_type_in_model(const RaceModelPc34* model, int itemType)
{
    int result = 0;
    int i;

    result += count_type_in_items(
        model->chestSlots, DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT, itemType);
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_RACE_CHAMPION_COUNT; ++i) {
        if (model->leaderHands[i].itemType == itemType) {
            ++result;
        }
    }
    return result;
}

static int open_chest_pc34(RaceModelPc34* model,
                           int openChestThing,
                           const M11_Item* linkedItems,
                           int linkedItemCount)
{
    int limit;
    int i;

    if (!model || openChestThing == 0 || linkedItemCount < 0 ||
        (linkedItemCount > 0 && !linkedItems)) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0333 lines 31-43 ignores the same G0426 chest and
     * selects the current chest; lines 53-67 copy only the visible linked
     * objects into G0425_aT_ChestSlots[0..7]. */
    model->openChestThing = openChestThing;
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT; ++i) {
        clear_item(&model->chestSlots[i]);
    }
    limit = linkedItemCount < DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT ?
        linkedItemCount : DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT;
    for (i = 0; i < limit; ++i) {
        model->chestSlots[i] = linkedItems[i];
    }
    return 1;
}

static int click_chest_slot_pc34(RaceModelPc34* model,
                                 int championIndex,
                                 int chestSlotIndex)
{
    M11_Item leaderHandObject;
    M11_Item slotObject;

    if (!model || championIndex < 0 ||
        championIndex >= DM1_PC34_CHEST_PICKUP_RACE_CHAMPION_COUNT ||
        model->openChestThing == 0 || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0302 lines 688-690 reads the global leader hand and
     * G0425 chest slot. Lines 694-695 return when both are empty; lines 700-710
     * remove the leader hand, remove the occupied slot through F0300, place the
     * slot object into the leader hand through F0297, and optionally put the
     * former leader-hand object back through F0301. CHAMDRAW.C F0291 lines
     * 551-552 draws chest slots from that same G0425 window.
     */
    leaderHandObject = model->leaderHands[championIndex];
    slotObject = model->chestSlots[chestSlotIndex];
    if (leaderHandObject.itemType == 0 && slotObject.itemType == 0) {
        return 0;
    }

    if (leaderHandObject.itemType != 0 &&
        (leaderHandObject.allowedSlots & DM1_PC34_ALLOWED_CONTAINER) == 0) {
        return 0;
    }

    if (leaderHandObject.itemType != 0) {
        clear_item(&model->leaderHands[championIndex]);
    }
    if (slotObject.itemType != 0) {
        /* ReDMCSB CHAMPION.C F0300 lines 511-515 clears the chosen G0425 slot;
         * F0297 lines 243-268 puts that thing in the active leader hand. */
        clear_item(&model->chestSlots[chestSlotIndex]);
        model->leaderHands[championIndex] = slotObject;
    }
    if (leaderHandObject.itemType != 0) {
        /* ReDMCSB CHAMPION.C F0301 lines 606-610 writes the former leader-hand
         * object back to G0425 when the click is a placement/swap. */
        model->chestSlots[chestSlotIndex] = leaderHandObject;
    }
    return 1;
}

static int close_chest_pc34(RaceModelPc34* model,
                            M11_Item* linkedItemsOut,
                            int maxItemsOut)
{
    int count = 0;
    int i;

    if (!model || maxItemsOut < 0 ||
        (maxItemsOut > 0 && !linkedItemsOut)) {
        return -1;
    }
    if (model->openChestThing == 0) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0334 lines 113-132 clears G0426, skips C0xFFFF empty
     * G0425 slots, and relinks only non-empty visible chest members. */
    model->openChestThing = 0;
    for (i = 0; i < DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT; ++i) {
        if (model->chestSlots[i].itemType != 0) {
            if (count < maxItemsOut) {
                linkedItemsOut[count] = model->chestSlots[i];
            }
            ++count;
        }
        clear_item(&model->chestSlots[i]);
    }
    return count;
}

const char*
dm1_v1_chest_cross_champion_pickup_race_source_evidence_pc34(void)
{
    return
        "CHEST.C F0333:31-67 selects G0426 and copies linked contents into the eight G0425 chest slots\n"
        "CHEST.C F0334:113-132 clears G0426 and relinks only non-empty visible G0425 slots\n"
        "CHAMPION.C F0297:243-268/F0298:270-298 maintain leader-hand object identity and weight\n"
        "CHAMPION.C F0300:511-515/F0301:606-610 clear/write G0425 slots during pickup and placement\n"
        "CHAMPION.C F0302:688-710 reads leader hand plus G0425 slot, no-ops when both are empty, then performs the swap\n"
        "CHAMDRAW.C F0291:551-552 and F0296:1184-1185 render changed open-chest slots from that same G0425 window";
}

const DM1_V1_ChestCrossChampionPickupRaceSpecPc34*
dm1_v1_chest_cross_champion_pickup_race_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_cross_champion_pickup_race_run_pc34(
    DM1_V1_ChestCrossChampionPickupRaceProbePc34* out)
{
    RaceModelPc34 model;
    M11_Item linked[4];
    M11_Item closed[DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT];
    int i;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(&model, 0, sizeof(model));
    memset(closed, 0, sizeof(closed));

    out->contractOnly = 1;
    for (i = 0; i < 4; ++i) {
        linked[i] = make_item(DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + i,
                              5 + i);
    }

    out->openResult = open_chest_pc34(
        &model, DM1_PC34_CHEST_PICKUP_RACE_CHEST_THING, linked, 4);
    out->openThing = model.openChestThing;
    out->initialVisibleCount = count_visible(model.chestSlots);
    out->initialRaceSlotType =
        model.chestSlots[DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX].itemType;
    out->initialRaceSlotWeight =
        model.chestSlots[DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX].weight;
    copy_types(model.chestSlots, DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT,
               out->initialTypes);

    out->firstClickResult = click_chest_slot_pc34(
        &model, DM1_PC34_CHEST_PICKUP_RACE_FIRST_CHAMPION,
        DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX);
    out->firstHandAfterType =
        model.leaderHands[DM1_PC34_CHEST_PICKUP_RACE_FIRST_CHAMPION].itemType;
    out->firstHandAfterWeight =
        model.leaderHands[DM1_PC34_CHEST_PICKUP_RACE_FIRST_CHAMPION].weight;
    out->firstRaceSlotAfterType =
        model.chestSlots[DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX].itemType;
    out->firstChampionWonPickup =
        out->firstClickResult &&
        out->firstHandAfterType == DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM &&
        out->firstRaceSlotAfterType == 0 ? 1 : 0;

    out->secondClickResult = click_chest_slot_pc34(
        &model, DM1_PC34_CHEST_PICKUP_RACE_SECOND_CHAMPION,
        DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX);
    out->secondHandAfterType =
        model.leaderHands[DM1_PC34_CHEST_PICKUP_RACE_SECOND_CHAMPION].itemType;
    out->secondRaceSlotAfterType =
        model.chestSlots[DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX].itemType;
    out->secondNoopPath =
        out->secondClickResult == 0 && out->secondHandAfterType == 0 ? 1 : 0;
    out->raceSlotStableAfterSecondClick =
        out->secondRaceSlotAfterType == out->firstRaceSlotAfterType ? 1 : 0;

    out->winnerCount =
        (model.leaderHands[DM1_PC34_CHEST_PICKUP_RACE_FIRST_CHAMPION].itemType ==
         DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM ? 1 : 0) +
        (model.leaderHands[DM1_PC34_CHEST_PICKUP_RACE_SECOND_CHAMPION].itemType ==
         DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM ? 1 : 0);
    out->pickedItemCopyCountAfterRace =
        count_type_in_model(&model, DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM);
    out->visibleCountAfterRace = count_visible(model.chestSlots);
    copy_types(model.chestSlots, DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT,
               out->afterRaceTypes);

    out->closeCount = close_chest_pc34(
        &model, closed, DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT);
    if (out->closeCount < 0) {
        return 0;
    }
    copy_types(closed, out->closeCount, out->closedTypes);
    out->pickedItemAbsentFromClosedLinks =
        count_type_in_items(closed, out->closeCount,
                            DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM) == 0 ? 1 : 0;
    out->closeCompactedEmptyRaceSlot =
        out->closeCount == 3 &&
        out->closedTypes[DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX - 1] ==
            DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 1 &&
        out->closedTypes[DM1_PC34_CHEST_PICKUP_RACE_SLOT_INDEX] ==
            DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 3 ? 1 : 0;

    out->reopenResult = open_chest_pc34(
        &model, DM1_PC34_CHEST_PICKUP_RACE_CHEST_THING + 1,
        closed, out->closeCount);
    if (!out->reopenResult) {
        return 0;
    }
    out->reopenedVisibleCount = count_visible(model.chestSlots);
    copy_types(model.chestSlots, DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT,
               out->reopenedTypes);
    out->pickedItemAbsentAfterReopen =
        count_type_in_items(model.chestSlots,
                            DM1_PC34_CHEST_PICKUP_RACE_SLOT_COUNT,
                            DM1_PC34_CHEST_PICKUP_RACE_PICKED_ITEM) == 0 ? 1 : 0;
    out->reopenedOrderCompacted =
        out->reopenedTypes[0] == DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM &&
        out->reopenedTypes[1] == DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 1 &&
        out->reopenedTypes[2] == DM1_PC34_CHEST_PICKUP_RACE_FIRST_ITEM + 3 &&
        out->reopenedTypes[3] == 0 ? 1 : 0;

    return 1;
}
