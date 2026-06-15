#include "dm1_v1_chest_occupied_slot_swap_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 sets G0426 and materializes C537..C544 into G0425\n"
    "CHEST.C F0334:113-132 clears G0426 and rewrites non-empty G0425 entries\n"
    "CHAMPION.C F0297:243-268 puts the removed occupied stack in G4055\n"
    "CHAMPION.C F0298:270-298 removes the previous leader-hand stack\n"
    "CHAMPION.C F0300:489-584 clears the occupied C30+ G0425 slot\n"
    "CHAMPION.C F0301:587-660 writes the previous leader-hand stack to C30+\n"
    "CHAMPION.C F0302:662-713 dispatches C538 through F0300/F0297/F0301\n"
    "PANEL.C F0354:2299-2322 closes inventory via F0334 on champion changes\n"
    "UTAMSCR.C F0077/F0078:141-150 bracket screen updates during hand swaps\n"
    "BLITMASK.C F0133:30-33 documents masked bitmap blit routing for icons\n"
    "OBJECT.C F0033:147-212 resolves object icons for the swapped stacks\n"
    "DEFS.H:810-817 C30..C37; 1878 M070; 5700/5876/5878/5881 G0305/G0423/G0425/G0426; 3906-3913 C537..C544";

const DM1_V1_ChestOccupiedSlotSwapSpecPc34
    dm1_v1_chest_occupied_slot_swap_pc34_spec = {
        "pass706 contract-only C538 occupied-slot swap runtime gate",
        1,
        1,
        DM1_PC34_CHEST_OCCUPIED_SWAP_C537_ORDINAL,
        DM1_PC34_CHEST_OCCUPIED_SWAP_C538_ORDINAL,
        DM1_PC34_CHEST_OCCUPIED_SWAP_C544_ORDINAL,
        DM1_PC34_SLOT_CHEST_2,
        DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX,
        0,
        1,
        DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT,
        DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK,
        DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT,
        DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK,
        DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT
    };

static DM1_V1_ChestOccupiedSlotSwapAssertionsPc34 s_assertions;

static M11_Item make_item(int itemType,
                          int weight,
                          int charges,
                          int allowedSlots)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.charges = charges;
    item.identified = 1;
    item.allowedSlots = allowedSlots;
    return item;
}

static int copy_g0425(DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state)
{
    int i;

    if (!state) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(
                &state->runtime, 0, i, &item)) {
            return 0;
        }
        state->g0425Types[i] = item.itemType;
        state->g0425Weights[i] = item.weight;
        state->g0425Counts[i] = item.charges;
    }
    state->g0426OpenChestThing =
        m11_inventory_get_open_chest_thing(&state->runtime, 0);
    state->m516LeaderLoad = m11_inventory_get_load(&state->runtime, 0);
    return 1;
}

static int visible_count(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const int* types, int itemType)
{
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static void copy_snapshot(const DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state,
                          int* types,
                          int* weights,
                          int* counts)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        types[i] = state->g0425Types[i];
        weights[i] = state->g0425Weights[i];
        counts[i] = state->g0425Counts[i];
    }
}

static int source_equivalent_load(const DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state)
{
    M11_Item hand;

    if (!state ||
        !m11_inventory_get_mouse_item(&state->runtime, 0, &hand)) {
        return 0;
    }
    return m11_inventory_get_load(&state->runtime, 0) + hand.weight;
}

static void record_check(int condition)
{
    ++s_assertions.totalAssertions;
    if (condition) {
        ++s_assertions.passedAssertions;
    } else {
        ++s_assertions.failedAssertions;
    }
}

static void record_assertions(
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* p)
{
    int i;

    memset(&s_assertions, 0, sizeof(s_assertions));
    if (!p) {
        record_check(0);
        return;
    }

    record_check(p->initResult == 1);
    record_check(p->exerciseResult == 1);
    record_check(p->sourceLockedContractOnly == 1);
    record_check(p->g0305PartyChampionCount == 1);
    record_check(p->g0423InventoryChampionOrdinal == 1);
    record_check(p->m070ReadyHandSlotIndex == 0);
    record_check(p->m070ActionHandSlotIndex == 1);
    record_check(p->openChestThingBefore ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_OPEN_CHEST_THING);
    record_check(p->openChestThingAfterClick ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_OPEN_CHEST_THING);
    record_check(p->c538Pc34Slot == DM1_PC34_SLOT_CHEST_2);
    record_check(p->c538Ordinal ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_C538_ORDINAL);
    record_check(p->visibleCountBefore == 3);
    record_check(p->visibleCountAfterClick == 3);
    record_check(p->replacementAllowedInC538 == 1);
    record_check(p->f0302Accepted == 1);
    record_check(p->f0300RemovedOccupiedC538 == 1);
    record_check(p->f0297PlacedOldC538InLeaderHand == 1);
    record_check(p->f0301StoredLeaderObjectInC538 == 1);
    record_check(p->c537StableAfterClick == 1);
    record_check(p->c539StableAfterClick == 1);
    record_check(p->oldStackNoLongerInChestAfterClick == 1);
    record_check(p->replacementNoLongerInLeaderHandAfterClick == 1);
    record_check(p->sourceEquivalentLoadUnchanged == 1);
    record_check(p->closedCount == 3);
    record_check(p->closeClearedG0426 == 1);
    record_check(p->closeRewroteVisibleOnly == 1);
    record_check(p->reopenResult == 1);
    record_check(p->reopenedCount == 3);
    record_check(p->reopenPreservedC538Replacement == 1);
    record_check(p->leaderHandBeforeType ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK);
    record_check(p->leaderHandAfterType ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK);
    record_check(p->leaderHandAfterCount ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT);
    record_check(p->c538BeforeType ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK);
    record_check(p->c538BeforeCount ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT);
    record_check(p->c538AfterType ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK);
    record_check(p->c538AfterCount ==
                 DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT);
    for (i = 3; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        record_check(p->afterTypes[i] == 0);
        record_check(p->closedTypes[i] == 0);
        record_check(p->reopenedTypes[i] == 0);
    }
}

const char* dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestOccupiedSlotSwapSpecPc34*
dm1_v1_chest_occupied_slot_swap_spec_pc34(void)
{
    return &dm1_v1_chest_occupied_slot_swap_pc34_spec;
}

int dm1_v1_chest_occupied_slot_swap_init_pc34(
    DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state)
{
    M11_Item linked[3];

    if (!state) {
        return 0;
    }
    memset(state, 0, sizeof(*state));
    m11_inventory_init(&state->runtime, 1);
    state->g0305PartyChampionCount = 1;
    state->g0423InventoryChampionOrdinal = 1;
    state->m070ReadyHandSlotIndex = 0;
    state->m070ActionHandSlotIndex = 1;

    linked[0] = make_item(DM1_PC34_CHEST_OCCUPIED_SWAP_C537_STACK,
                          DM1_PC34_CHEST_OCCUPIED_SWAP_C537_WEIGHT, 1,
                          DM1_PC34_ALLOWED_CONTAINER);
    linked[1] = make_item(DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK,
                          DM1_PC34_CHEST_OCCUPIED_SWAP_C538_WEIGHT,
                          DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT,
                          DM1_PC34_ALLOWED_CONTAINER);
    linked[2] = make_item(DM1_PC34_CHEST_OCCUPIED_SWAP_C539_STACK,
                          DM1_PC34_CHEST_OCCUPIED_SWAP_C539_WEIGHT, 1,
                          DM1_PC34_ALLOWED_CONTAINER);

    /* ReDMCSB CHEST.C F0333 lines 43-67 sets G0426, draws the open chest,
     * then copies linked objects into G0425 C537/C538/C539. */
    if (!m11_inventory_open_chest(
            &state->runtime, 0,
            DM1_PC34_CHEST_OCCUPIED_SWAP_OPEN_CHEST_THING,
            linked, 3)) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0297/F0298 lines 243-298 make G4055 the leader
     * hand object that F0302 later swaps with occupied C538. */
    if (!m11_inventory_set_mouse_item(
            &state->runtime, 0,
            DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK,
            DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_WEIGHT,
            DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT,
            DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }

    if (!copy_g0425(state)) {
        return 0;
    }
    state->m516LeaderSourceEquivalentLoad = source_equivalent_load(state);
    return 1;
}

int dm1_v1_chest_occupied_slot_swap_exercise_pc34(
    DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state,
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out)
{
    M11_Item hand;
    M11_Item slot;
    M11_Item closed[DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT];
    int i;

    if (!state || !out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));
    out->initResult = 1;
    out->sourceLockedContractOnly = 1;
    out->g0305PartyChampionCount = state->g0305PartyChampionCount;
    out->g0423InventoryChampionOrdinal = state->g0423InventoryChampionOrdinal;
    out->m070ReadyHandSlotIndex = state->m070ReadyHandSlotIndex;
    out->m070ActionHandSlotIndex = state->m070ActionHandSlotIndex;
    out->c538Pc34Slot = DM1_PC34_SLOT_CHEST_2;
    out->c538Ordinal = DM1_PC34_CHEST_OCCUPIED_SWAP_C538_ORDINAL;
    out->openChestThingBefore = state->g0426OpenChestThing;
    copy_snapshot(state, out->beforeTypes, out->beforeWeights,
                  out->beforeCounts);
    out->visibleCountBefore = visible_count(out->beforeTypes);
    out->c538BeforeType =
        out->beforeTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX];
    out->c538BeforeWeight =
        out->beforeWeights[DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX];
    out->c538BeforeCount =
        out->beforeCounts[DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX];
    if (!m11_inventory_get_mouse_item(&state->runtime, 0, &hand)) {
        return 0;
    }
    out->leaderHandBeforeType = hand.itemType;
    out->leaderHandBeforeWeight = hand.weight;
    out->leaderHandBeforeCount = hand.charges;
    out->replacementAllowedInC538 =
        m11_inventory_can_equip(&hand, DM1_PC34_SLOT_CHEST_2);
    out->sourceEquivalentLoadBefore = source_equivalent_load(state);

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 reads G0425[C538-C537],
     * runs F0300/F0297 on the occupied stack, then F0301 stores G4055. */
    out->f0302Accepted = m11_inventory_click_pc34_source_slot(
        &state->runtime, 0, DM1_PC34_SLOT_CHEST_2);
    if (!out->f0302Accepted ||
        !m11_inventory_get_mouse_item(&state->runtime, 0, &hand) ||
        !m11_inventory_get_item_in_chest_slot(
            &state->runtime, 0,
            DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX, &slot) ||
        !copy_g0425(state)) {
        record_assertions(out);
        return 0;
    }
    out->openChestThingAfterClick = state->g0426OpenChestThing;
    copy_snapshot(state, out->afterTypes, out->afterWeights,
                  out->afterCounts);
    out->visibleCountAfterClick = visible_count(out->afterTypes);
    out->leaderHandAfterType = hand.itemType;
    out->leaderHandAfterWeight = hand.weight;
    out->leaderHandAfterCount = hand.charges;
    out->c538AfterType = slot.itemType;
    out->c538AfterWeight = slot.weight;
    out->c538AfterCount = slot.charges;
    out->f0300RemovedOccupiedC538 =
        hand.itemType == DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK ? 1 : 0;
    out->f0297PlacedOldC538InLeaderHand =
        out->f0300RemovedOccupiedC538 &&
        hand.charges == DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT ? 1 : 0;
    out->f0301StoredLeaderObjectInC538 =
        slot.itemType == DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK &&
        slot.charges == DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT ? 1 : 0;
    out->c537StableAfterClick =
        out->afterTypes[0] == DM1_PC34_CHEST_OCCUPIED_SWAP_C537_STACK ? 1 : 0;
    out->c539StableAfterClick =
        out->afterTypes[2] == DM1_PC34_CHEST_OCCUPIED_SWAP_C539_STACK ? 1 : 0;
    out->oldStackNoLongerInChestAfterClick =
        !contains_type(out->afterTypes,
                       DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK);
    out->replacementNoLongerInLeaderHandAfterClick =
        hand.itemType != DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK ? 1 : 0;
    out->sourceEquivalentLoadAfterClick = source_equivalent_load(state);
    out->sourceEquivalentLoadUnchanged =
        out->sourceEquivalentLoadBefore ==
        out->sourceEquivalentLoadAfterClick ? 1 : 0;

    /* ReDMCSB CHEST.C F0334 lines 113-132 closes G0426 and compacts only
     * non-empty G0425 slots back into the source container chain. */
    out->closedCount = m11_inventory_close_chest(
        &state->runtime, 0, closed,
        DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT);
    if (out->closedCount < 0) {
        record_assertions(out);
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        out->closedTypes[i] =
            i < out->closedCount ? closed[i].itemType : 0;
    }
    out->closeClearedG0426 =
        m11_inventory_get_open_chest_thing(&state->runtime, 0) == 0 ? 1 : 0;
    out->closeRewroteVisibleOnly =
        out->closedCount == 3 &&
        out->closedTypes[0] == DM1_PC34_CHEST_OCCUPIED_SWAP_C537_STACK &&
        out->closedTypes[1] == DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK &&
        out->closedTypes[2] == DM1_PC34_CHEST_OCCUPIED_SWAP_C539_STACK ? 1 : 0;

    /* ReDMCSB CHEST.C F0333 lines 53-76 rematerializes the compacted chain
     * into C537..C544, keeping the C538 replacement visible. */
    out->reopenResult = m11_inventory_open_chest(
        &state->runtime, 0,
        DM1_PC34_CHEST_OCCUPIED_SWAP_OPEN_CHEST_THING,
        closed, out->closedCount);
    if (!out->reopenResult || !copy_g0425(state)) {
        record_assertions(out);
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        out->reopenedTypes[i] = state->g0425Types[i];
    }
    out->reopenedCount = visible_count(out->reopenedTypes);
    out->reopenPreservedC538Replacement =
        out->reopenedTypes[DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX] ==
        DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK ? 1 : 0;
    out->exerciseResult = 1;
    record_assertions(out);
    return s_assertions.failedAssertions == 0 ? 1 : 0;
}

const DM1_V1_ChestOccupiedSlotSwapAssertionsPc34*
dm1_v1_chest_occupied_slot_swap_assertions_pc34(void)
{
    return &s_assertions;
}

int dm1_v1_chest_occupied_slot_swap_pc34(
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out)
{
    DM1_V1_ChestOccupiedSlotSwapRuntimePc34 state;

    if (!out) {
        return 0;
    }
    if (!dm1_v1_chest_occupied_slot_swap_init_pc34(&state)) {
        memset(out, 0, sizeof(*out));
        record_assertions(out);
        return 0;
    }
    return dm1_v1_chest_occupied_slot_swap_exercise_pc34(&state, out);
}
