#include "dm1_v1_chest_reopen_contents_order_pc34_compat.h"

#include <string.h>

enum {
    DM1_PC34_REOPEN_CHEST_THING_BASE = 0x7100
};

static const char s_source_evidence[] =
    "CHEST.C F0333:53-67 materializes linked chest contents into G0425_aT_ChestSlots\n"
    "CHEST.C F0334:117-132 rewrites the container Slot/Next chain from non-empty G0425 slots\n"
    "DUNGEON.C F0163:1796-1799 clears linked item's Next before relinking; 1832-1837 appends after current tail";

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

static void clear_items(M11_Item* items, int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        memset(&items[i], 0, sizeof(items[i]));
    }
}

static int copy_open_types(const M11_InventoryState* state,
                           int* typesOut)
{
    int i;

    if (!state || !typesOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
    }
    return 1;
}

static void copy_item_types(const M11_Item* items,
                            int count,
                            int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT; ++i) {
        typesOut[i] =
            (i < count && items[i].itemType != 0) ? items[i].itemType : 0;
    }
}

static int count_visible_types(const int* types)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT; ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_type(const int* types, int count, int itemType)
{
    int i;

    for (i = 0;
         i < count && i < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT;
         ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int unique_visible_count(const int* types)
{
    int unique = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT; ++i) {
        int seen = 0;
        int j;

        if (types[i] == 0) {
            continue;
        }
        for (j = 0; j < i; ++j) {
            if (types[j] == types[i]) {
                seen = 1;
            }
        }
        if (!seen) {
            ++unique;
        }
    }
    return unique;
}

static int contains_every_visible_input(
    const M11_GameView_ChestReopenContentsOrderCasePc34* out)
{
    int visibleInputCount =
        out->inputCount < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT ?
        out->inputCount : DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT;
    int i;

    for (i = 0; i < visibleInputCount; ++i) {
        if (!contains_type(out->reopenedTypes,
                           DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT,
                           out->inputTypes[i])) {
            return 0;
        }
    }
    return 1;
}

static int run_case(int caseIndex,
                    const M11_Item* inputItems,
                    int inputCount,
                    int leaderHandItemType,
                    M11_GameView_ChestReopenContentsOrderCasePc34* out)
{
    M11_InventoryState state;
    M11_Item closed[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT];
    M11_Item item;
    int i;

    if (!inputItems || !out || inputCount <= 0 ||
        inputCount > DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_MAX_INPUT) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    clear_items(closed, DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT);
    m11_inventory_init(&state, 1);
    out->inputCount = inputCount;

    for (i = 0; i < inputCount; ++i) {
        out->inputTypes[i] = inputItems[i].itemType;
    }
    out->originalHead = out->inputTypes[0];
    if (inputCount >= 3) {
        out->originalMiddle = out->inputTypes[1];
    }
    out->originalTail =
        out->inputTypes[
            inputCount < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT ?
            inputCount - 1 : DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT - 1];

    if (leaderHandItemType != 0 &&
        !m11_inventory_set_mouse_item(
            &state, 0, leaderHandItemType, 9, 0,
            DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER)) {
        return 0;
    }
    if (!m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandBeforeOpen = item.itemType;

    /* ReDMCSB CHEST.C F0333 lines 53-67 follows F0159/F0163-style linked
     * order and copies at most the first eight chest contents into G0425. */
    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_REOPEN_CHEST_THING_BASE + caseIndex,
        inputItems, inputCount);
    if (!out->openResult || !copy_open_types(&state, out->openedTypes) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterOpen = item.itemType;

    /* ReDMCSB CHEST.C F0334 lines 117-132 makes the first non-empty G0425
     * entry the container head and appends later entries with DUNGEON.C F0163
     * lines 1796-1799 and 1832-1837, preserving the visible order. */
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed,
        DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT);
    if (out->closeCount < 0 ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterClose = item.itemType;
    copy_item_types(closed, out->closeCount, out->closedTypes);
    out->closedHead = out->closedTypes[0];
    if (out->closeCount >= 3) {
        out->closedMiddle = out->closedTypes[1];
    }
    if (out->closeCount > 0) {
        out->closedTail = out->closedTypes[out->closeCount - 1];
    }
    out->closedContainsHiddenTail =
        contains_type(out->closedTypes, out->closeCount,
                      DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HIDDEN_TAIL);

    /* ReDMCSB CHEST.C F0333 lines 53-67 rematerializes the F0334-produced
     * head/tail list, proving the close-rewrite to reopen round trip. */
    out->reopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_REOPEN_CHEST_THING_BASE + caseIndex,
        closed, out->closeCount);
    if (!out->reopenResult || !copy_open_types(&state, out->reopenedTypes) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandAfterReopen = item.itemType;
    out->reopenedVisibleCount = count_visible_types(out->reopenedTypes);
    out->reopenedHead = out->reopenedTypes[0];
    if (out->reopenedVisibleCount >= 3) {
        out->reopenedMiddle = out->reopenedTypes[1];
    }
    if (out->reopenedVisibleCount > 0) {
        out->reopenedTail = out->reopenedTypes[out->reopenedVisibleCount - 1];
    }
    out->reopenedContainsHiddenTail =
        contains_type(out->reopenedTypes, out->reopenedVisibleCount,
                      DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HIDDEN_TAIL);
    out->reopenedContainsEveryVisibleInput =
        contains_every_visible_input(out);
    out->reopenedUniqueVisibleCount =
        unique_visible_count(out->reopenedTypes);
    out->noDroppedOrDuplicatedVisibleItems =
        out->reopenedContainsEveryVisibleInput &&
        out->reopenedUniqueVisibleCount == out->reopenedVisibleCount ? 1 : 0;
    out->leaderHandUnchangedAcrossCycle =
        out->leaderHandBeforeOpen == out->leaderHandAfterOpen &&
        out->leaderHandBeforeOpen == out->leaderHandAfterClose &&
        out->leaderHandBeforeOpen == out->leaderHandAfterReopen ? 1 : 0;

    return 1;
}

const char* M11_GameView_ChestReopenContentsOrderSourceEvidencePc34(void)
{
    return s_source_evidence;
}

int M11_GameView_ChestReopenContentsOrderRunPc34(
    M11_GameView_ChestReopenContentsOrderProbePc34* out)
{
    M11_Item caseOne[1];
    M11_Item caseThree[3];
    M11_Item caseFull[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT];
    M11_Item caseHidden[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_MAX_INPUT];
    M11_Item caseLeader[3];
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->sourceLockedContractOnly = 1;
    out->c537Pc34Slot = DM1_PC34_SLOT_CHEST_1;
    out->c544Pc34Slot = DM1_PC34_SLOT_CHEST_8;
    out->chestSlotCount = DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT;

    caseOne[0] = make_item(701, 2, DM1_PC34_ALLOWED_CONTAINER);

    caseThree[0] = make_item(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_WEAPON,
                             12, DM1_PC34_ALLOWED_CONTAINER);
    caseThree[1] = make_item(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_POTION,
                             3, DM1_PC34_ALLOWED_CONTAINER);
    caseThree[2] = make_item(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_JUNK,
                             1, DM1_PC34_ALLOWED_CONTAINER);

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT; ++i) {
        caseFull[i] = make_item(900 + i, 2 + i,
                                DM1_PC34_ALLOWED_CONTAINER);
    }

    for (i = 0; i < DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_MAX_INPUT; ++i) {
        caseHidden[i] = make_item(800 + i, 3 + i,
                                  DM1_PC34_ALLOWED_CONTAINER);
    }

    for (i = 0; i < 3; ++i) {
        caseLeader[i] = make_item(1000 + i, 4 + i,
                                  DM1_PC34_ALLOWED_CONTAINER);
    }

    if (!run_case(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_ONE,
                  caseOne, 1, 0,
                  &out->cases[
                      DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_ONE]) ||
        !run_case(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_THREE,
                  caseThree, 3, 0,
                  &out->cases[
                      DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_THREE]) ||
        !run_case(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_FULL,
                  caseFull, DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT, 0,
                  &out->cases[
                      DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_FULL]) ||
        !run_case(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_HIDDEN_TAIL,
                  caseHidden, DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_MAX_INPUT, 0,
                  &out->cases[
                      DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_HIDDEN_TAIL]) ||
        !run_case(DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_LEADER_HELMET,
                  caseLeader, 3,
                  DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HELMET,
                  &out->cases[
                      DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_LEADER_HELMET])) {
        return 0;
    }

    return 1;
}
