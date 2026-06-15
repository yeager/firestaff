#include "dm1_v1_chest_drop_to_floor_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "COMMAND.C F0360:489-506,2174-2177 maps C021/C058..C065 slot commands into F0302\n"
    "PANEL.C F0342/F0347:1119-1133,1651-1669 closes current chest state and opens action-hand containers through F0333\n"
    "CHEST.C F0333:53-75 copies only the first eight linked objects into G0425_aT_ChestSlots\n"
    "CHAMPION.C F0302:688-710 deposits compatible leader-hand objects into C30..C37/G0425 slots\n"
    "CHEST.C F0334:113-132 closes the chest by relinking only actually deposited visible G0425 slots\n"
    "DUNGEON.C F0163:1800-1837 links overflow objects to a map square when MapX >= 0";

static const DM1_V1_ChestDropToFloorSpecPc34 s_spec = {
    "Source-locked contract gate only; not full real-asset chest/floor runtime parity.",
    DM1_PC34_SLOT_ACTION_HAND,
    DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_SLOT_CHEST_8,
    DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT,
    DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT,
    DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT,
    DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_X,
    DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_Y
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

static int copy_visible_types(const M11_InventoryState* state, int* typesOut)
{
    int i;

    if (!state || !typesOut) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT; ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(state, 0, i, &item)) {
            return 0;
        }
        typesOut[i] = item.itemType;
    }
    return 1;
}

static void copy_closed_types(const M11_Item* items, int count, int* typesOut)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT; ++i) {
        typesOut[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static int count_nonempty_types(const int* types, int maxCount)
{
    int count = 0;
    int i;

    if (!types || maxCount < 0) {
        return 0;
    }
    for (i = 0; i < maxCount; ++i) {
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
    for (i = 0; i < count; ++i) {
        if (types[i] == itemType) {
            return 1;
        }
    }
    return 0;
}

static int chest_prefix_order_preserved(const int* types)
{
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT; ++i) {
        if (types[i] != DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_CHEST_ITEM + i) {
            return 0;
        }
    }
    return 1;
}

static int deposit_order_preserved(const int* types, int depositCount)
{
    int i;
    const int start = DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT;

    if (!types || depositCount < 0) {
        return 0;
    }
    for (i = 0; i < depositCount; ++i) {
        if (types[start + i] !=
            DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_HAND_ITEM + i) {
            return 0;
        }
    }
    return 1;
}

static int floor_order_preserved(const int* floorTypes,
                                 int floorCount,
                                 int depositedCount)
{
    int i;

    if (!floorTypes || floorCount < 0 || depositedCount < 0) {
        return 0;
    }
    for (i = 0; i < floorCount; ++i) {
        if (floorTypes[i] !=
            DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_HAND_ITEM +
            depositedCount + i) {
            return 0;
        }
    }
    return 1;
}

const char* dm1_v1_chest_drop_to_floor_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestDropToFloorSpecPc34*
dm1_v1_chest_drop_to_floor_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_drop_to_floor_pc34_compat_run(
    DM1_V1_ChestDropToFloorProbePc34* out)
{
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT];
    M11_Item handStack[DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT];
    M11_Item closed[DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT];
    M11_Item item;
    int handIndex = 0;
    int floorIndex = 0;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(closed, 0, sizeof(closed));

    m11_inventory_init(&state, 1);
    for (i = 0; i < DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT; ++i) {
        linked[i] =
            make_item(DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_CHEST_ITEM + i,
                      2 + i);
    }
    for (i = 0; i < DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT; ++i) {
        handStack[i] =
            make_item(DM1_PC34_CHEST_DROP_TO_FLOOR_FIRST_HAND_ITEM + i,
                      20 + i);
    }

    out->sourceLockedContractOnly = 1;
    out->initialChestCount = DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT;
    out->handStackCountBefore = DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT;
    out->frontMapX = DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_X;
    out->frontMapY = DM1_PC34_CHEST_DROP_TO_FLOOR_FRONT_MAP_Y;

    /* ReDMCSB PANEL.C F0342 lines 1119-1133 routes a container action-hand
     * panel to CHEST.C F0333, whose lines 53-75 materialize only C30..C37. */
    out->openResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_DROP_TO_FLOOR_CHEST_THING, linked,
        DM1_PC34_CHEST_DROP_TO_FLOOR_INITIAL_CHEST_COUNT);
    out->openThing = m11_inventory_get_open_chest_thing(&state, 0);
    if (!out->openResult || !copy_visible_types(&state, out->openedTypes)) {
        return 0;
    }
    out->freeChestSlotsBefore =
        DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT -
        count_nonempty_types(out->openedTypes,
                             DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT);

    /* ReDMCSB COMMAND.C F0360 lines 2174-2177 dispatches slot clicks to
     * CHAMPION.C F0302, and F0302 lines 688-710 deposits accepted leader-hand
     * objects into C30..C37.  When no C30..C37 slot remains, DUNGEON.C F0163
     * lines 1800-1837 is the source-locked square-link path for overflow. */
    while (handIndex < DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT) {
        int deposited = 0;
        int slot;

        for (slot = 0; slot < DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT; ++slot) {
            if (!m11_inventory_get_item_in_chest_slot(&state, 0, slot, &item)) {
                return 0;
            }
            if (item.itemType == 0) {
                if (!m11_inventory_set_mouse_item(
                        &state, 0, handStack[handIndex].itemType,
                        handStack[handIndex].weight,
                        handStack[handIndex].charges,
                        handStack[handIndex].allowedSlots) ||
                    !m11_inventory_click_pc34_source_slot(
                        &state, 0, DM1_PC34_SLOT_CHEST_1 + slot)) {
                    return 0;
                }
                ++out->depositAttempts;
                ++out->depositedCount;
                ++handIndex;
                deposited = 1;
                break;
            }
        }
        if (!deposited) {
            out->floorTypes[floorIndex++] = handStack[handIndex].itemType;
            ++handIndex;
        }
    }

    if (!copy_visible_types(&state, out->afterDepositTypes) ||
        !m11_inventory_get_mouse_item(&state, 0, &item)) {
        return 0;
    }
    out->leaderHandEmptyAfterRun = item.itemType == 0 ? 1 : 0;
    out->handStackCountAfterDeposit =
        DM1_PC34_CHEST_DROP_TO_FLOOR_HAND_STACK_COUNT - out->depositedCount;
    out->overflowCount = out->handStackCountAfterDeposit;
    out->floorDropCount = floorIndex;

    /* ReDMCSB CHEST.C F0334 lines 113-132 closes by relinking only the
     * actually deposited visible G0425 slots; overflow stays outside the
     * chest and is represented here by the F0163 MapX>=0 floor-drop list. */
    out->closeCount = m11_inventory_close_chest(
        &state, 0, closed, DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT);
    if (out->closeCount < 0) {
        return 0;
    }
    copy_closed_types(closed, out->closeCount, out->closedTypes);
    out->closedChestCount =
        count_nonempty_types(out->closedTypes,
                             DM1_PC34_CHEST_DROP_TO_FLOOR_SLOT_COUNT);
    out->openThingAfterClose = m11_inventory_get_open_chest_thing(&state, 0);

    out->chestOrderPreserved =
        chest_prefix_order_preserved(out->closedTypes);
    out->depositOrderPreserved =
        deposit_order_preserved(out->closedTypes, out->depositedCount);
    out->floorOverflowOrderPreserved =
        floor_order_preserved(out->floorTypes, out->floorDropCount,
                              out->depositedCount);
    out->handCountReducedByDeposits =
        out->handStackCountAfterDeposit ==
        out->handStackCountBefore - out->depositedCount;
    out->overflowAbsentFromChest = 1;
    for (i = 0; i < out->floorDropCount; ++i) {
        if (contains_type(out->closedTypes, out->closeCount,
                          out->floorTypes[i])) {
            out->overflowAbsentFromChest = 0;
        }
    }

    return 1;
}
