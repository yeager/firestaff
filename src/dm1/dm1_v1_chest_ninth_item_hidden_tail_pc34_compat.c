#include "dm1_v1_chest_ninth_item_hidden_tail_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:58-67 opens a chest and copies at most the first eight linked objects into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:113-132 closes a chest by rewriting only non-empty visible G0425_aT_ChestSlots\n"
    "CHAMPION.C F0297/F0298/F0302:250-298,688-710 owns leader-hand put/remove state and occupied-slot swap routing\n"
    "DUNGEON.C F0163:1796-1837 clears Next and appends linked visible-input returns; hidden tail input 808 is not in that visible return list\n"
    "OBJECT.C F0031:25-120 loads deterministic object names; this gate uses unique itemType values as object-id sentinels";

const DM1_V1_ChestNinthItemHiddenTailSpecPc34
    dm1_v1_chest_ninth_item_hidden_tail_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_SLOT_CHEST_1,
        DM1_PC34_SLOT_CHEST_8,
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT,
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED,
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST,
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT
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

static int copy_open_types(const M11_InventoryState* state,
                           int* typesOut)
{
    int i;

    if (!state || !typesOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
    }
    return 1;
}

static void copy_closed_types(const M11_Item* items,
                              int count,
                              int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        typesOut[i] =
            (items && i < count && items[i].itemType != 0) ?
            items[i].itemType : 0;
    }
}

static int count_visible(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const int* types, int count, int itemType)
{
    int i;

    if (!types || count < 0) {
        return 0;
    }
    for (i = 0;
         i < count && i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT;
         ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int order_matches_visible_input(const int* types)
{
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        if (types[i] !=
            DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST + i) {
            return 0;
        }
    }
    return 1;
}

const char* dm1_v1_chest_ninth_item_hidden_tail_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestNinthItemHiddenTailSpecPc34*
dm1_v1_chest_ninth_item_hidden_tail_spec_pc34(void)
{
    return &dm1_v1_chest_ninth_item_hidden_tail_pc34_spec;
}

int dm1_v1_chest_ninth_item_hidden_tail_pc34(
    DM1_V1_ChestNinthItemHiddenTailProbePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED];
    M11_Item hiddenTail[1];
    M11_Item closed[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT];
    M11_Item reopenInput[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED];
    M11_Item item;
    int hiddenTailCount = 0;
    int reopenInputCount;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(hiddenTail, 0, sizeof(hiddenTail));
    memset(closed, 0, sizeof(closed));
    memset(reopenInput, 0, sizeof(reopenInput));

    out->sourceLockedContractOnly = 1;
    out->chestThing = DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_CHEST_THING;
    out->reopenedChestThing =
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_REOPEN_THING;

    m11_inventory_init(&state, 1);
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED; ++i) {
        linked[i] =
            make_item(DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST + i,
                      2 + i,
                      DM1_PC34_ALLOWED_CONTAINER);
    }

    /* ReDMCSB CHEST.C F0333 lines 58-67 materializes at most the first eight
     * linked objects into C537..C544/G0425 in input order; OBJECT.C F0031
     * lines 25-120 is represented by unique deterministic itemType sentinels.
     * The ninth linked input remains a hidden tail outside the visible panel. */
    out->linkedInputCount = DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED;
    out->openResult = m11_inventory_open_chest(
        &state, 0, out->chestThing, linked,
        out->linkedInputCount);
    out->openThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->openResult || !copy_open_types(&state, out->openedTypes)) {
        return 0;
    }
    out->openedVisibleCount = count_visible(out->openedTypes);
    out->openedHiddenTailVisible =
        contains_type(out->openedTypes,
                      DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT,
                      DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT);
    out->openedHiddenTailType =
        linked[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT].itemType;
    out->openedHiddenTailPreserved =
        out->openedHiddenTailType ==
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT ? 1 : 0;
    out->openedOrderMatchesInput =
        order_matches_visible_input(out->openedTypes);

    item = make_item(DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, 17,
                     DM1_PC34_ALLOWED_CONTAINER);
    if (!m11_inventory_set_mouse_item(&state, 0, item.itemType,
                                      item.weight, item.charges,
                                      item.allowedSlots) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBeforePut = item.itemType;
    out->leaderHandCanEnterContainer =
        m11_inventory_can_equip(&item, DM1_PC34_SLOT_CHEST_8);

    /* ReDMCSB CHAMPION.C F0297/F0298/F0302 lines 250-298,688-710 owns the
     * leader-hand put state.  With all C537..C544 slots occupied, this source
     * lock records the ninth object as the hidden tail input 808 and leaves
     * G0425 unchanged for CHEST.C F0334/DUNGEON.C F0163 visible relinking. */
    out->hiddenTailPutResult =
        out->leaderHandCanEnterContainer &&
        out->openedVisibleCount ==
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT &&
        out->leaderHandBeforePut ==
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT ? 1 : 0;
    if (!out->hiddenTailPutResult) {
        return 0;
    }
    hiddenTail[hiddenTailCount++] =
        linked[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT];
    if (!copy_open_types(&state, out->afterPutTypes) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterPut = item.itemType;
    out->hiddenTailChainCountAfterPut = hiddenTailCount;
    out->hiddenTailTypeAfterPut = hiddenTail[0].itemType;
    out->hiddenTailVisibleAfterPut =
        contains_type(out->afterPutTypes,
                      DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT,
                      DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT);
    out->visibleCountAfterPut = count_visible(out->afterPutTypes);
    out->afterPutOrderMatchesInput =
        order_matches_visible_input(out->afterPutTypes);

    /* ReDMCSB CHEST.C F0334 lines 113-132 rewrites only the eight non-empty
     * visible G0425 slots, while DUNGEON.C F0163 lines 1796-1837 appends only
     * those visible-input returns and excludes hidden tail input 808. */
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed,
        DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT);
    if (out->closeCount < 0 ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    copy_closed_types(closed, out->closeCount, out->closedTypes);
    out->hiddenTailReturnedByVisibleClose =
        contains_type(out->closedTypes, out->closeCount,
                      DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT);
    out->leaderHandAfterClose = item.itemType;

    for (i = 0; i < out->closeCount &&
         i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        reopenInput[i] = closed[i];
    }
    reopenInputCount = out->closeCount;
    if (hiddenTailCount > 0 &&
        reopenInputCount < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED) {
        reopenInput[reopenInputCount++] = hiddenTail[0];
    }

    /* ReDMCSB CHEST.C F0333 lines 31-67 reopens from the relinked visible
     * head plus the preserved hidden tail chain; only the first eight linked
     * objects rematerialize in C537..C544. */
    out->reopenResult = m11_inventory_open_chest(
        &state, 0, out->reopenedChestThing, reopenInput, reopenInputCount);
    if (!out->reopenResult || !copy_open_types(&state, out->reopenedTypes) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterReopen = item.itemType;
    out->reopenedVisibleCount = count_visible(out->reopenedTypes);
    out->hiddenTailVisibleAfterReopen =
        contains_type(out->reopenedTypes, out->reopenedVisibleCount,
                      DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT);
    out->hiddenTailChainCountAfterReopen = hiddenTailCount;
    out->hiddenTailTypeAfterReopen = hiddenTail[0].itemType;
    out->reopenedOrderMatchesInput =
        order_matches_visible_input(out->reopenedTypes);

    return 1;
}
