#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static M11_Item make_item(int itemType, int weight)
{
    M11_Item item;
    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static int expect_int(const char* label, int got, int want, const char* redmcsbAnchor)
{
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

int main(void)
{
    enum {
        TEST_CHEST = 0x3456
    };
    const char* f0140ContainerWeight =
        "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0333VisibleSlots =
        "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0334CloseCompacts =
        "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0301Load =
        "ReDMCSB CHAMPION.C F0301 lines 609-615";

    M11_InventoryState state;
    M11_Item linked[10];
    M11_Item closed[8];
    int snapshotWeight = 0;
    int ok = 1;

    printf("probe=dm1_v1_inventory_chest_load_pc34_compat\n");
    printf("sourceEvidence=%s\n", dm1_inventory_chest_load_source_evidence_pc34());

    memset(closed, 0, sizeof(closed));
    for (int i = 0; i < 10; ++i) {
        linked[i] = make_item(100 + i, 2 + i);
    }

    m11_inventory_init(&state, 1);

    /* ReDMCSB: CHAMPION.C F0301 lines 609-615 adds an ordinary slot object
     * weight to champion Load through F0140. */
    ok &= expect_int("base backpack load",
                     m11_inventory_set_item_in_pc34_source_slot(&state, 0,
                                                                DM1_PC34_SLOT_BACKPACK_LINE1_1,
                                                                501, 13, 0,
                                                                DM1_PC34_ALLOWED_ANY_SLOT),
                     1, f0301Load);
    /* ReDMCSB: CHAMPION.C F0301 lines 609-615 keeps the source Load value as
     * the sum of champion slot object weights. */
    ok &= expect_int("base backpack load value",
                     m11_inventory_get_load(&state, 0), 13, f0301Load);

    /* ReDMCSB: CHEST.C F0333 lines 53-76 copies only the first eight linked
     * container entries into G0425_aT_ChestSlots. */
    ok &= expect_int("open overfull chest",
                     m11_inventory_open_chest(&state, 0, TEST_CHEST, linked, 10),
                     1, f0333VisibleSlots);
    /* ReDMCSB: CHEST.C F0333 lines 53-76 excludes the ninth and later linked
     * objects from the visible open-chest slot snapshot. */
    ok &= expect_int("visible contents weight excludes hidden tail",
                     m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0),
                     44, f0333VisibleSlots);
    /* ReDMCSB: DUNGEON.C F0140 lines 1114-1120 gives a container base weight
     * of 50 before adding linked CONTENTS object weights. */
    ok &= expect_int("open chest container weight",
                     m11_inventory_pc34_open_chest_container_weight(&state, 0),
                     94, f0140ContainerWeight);
    /* ReDMCSB: CHAMPION.C F0301 lines 609-615 adds C30+ slot object weight
     * through the same F0140 path used for ordinary inventory slots. */
    ok &= expect_int("load includes visible open-chest contents",
                     m11_inventory_get_load(&state, 0), 57, f0301Load);

    /* ReDMCSB: CHAMPION.C F0301 lines 609-615 stores C30+ additions in G0425,
     * so replacing one visible slot changes the source visible contents sum. */
    ok &= expect_int("replace visible chest slot",
                     m11_inventory_set_item_in_chest_slot(&state, 0, 2,
                                                          909, 17, 0,
                                                          DM1_PC34_ALLOWED_CONTAINER),
                     1, f0301Load);
    /* ReDMCSB: DUNGEON.C F0140 lines 1117-1119 sums the current linked
     * CONTENTS weights, including the edited visible slot value. */
    ok &= expect_int("edited visible contents weight",
                     m11_inventory_pc34_open_chest_visible_contents_weight(&state, 0),
                     57, f0140ContainerWeight);
    /* ReDMCSB: DUNGEON.C F0140 lines 1114-1120 keeps the 50-unit container
     * shell separate from the edited CONTENTS sum. */
    ok &= expect_int("edited open chest container weight",
                     m11_inventory_pc34_open_chest_container_weight(&state, 0),
                     107, f0140ContainerWeight);

    state.champions[0].load = 777;
    /* ReDMCSB: CHEST.C F0334 lines 117-132 compacts non-empty G0425 slots
     * before clearing the open chest; this helper snapshots F0140 weight first. */
    ok &= expect_int("close snapshots compacted container weight",
                     m11_inventory_pc34_close_chest_with_weight_snapshot(&state, 0,
                                                                        closed, 8,
                                                                        &snapshotWeight),
                     8, f0334CloseCompacts);
    /* ReDMCSB: DUNGEON.C F0140 lines 1114-1120 reports the source container
     * weight captured before F0334 erases the transient G0425 slots. */
    ok &= expect_int("snapshot weight value",
                     snapshotWeight, 107, f0140ContainerWeight);
    /* ReDMCSB: CHEST.C F0334 lines 123-130 preserves visible slot order while
     * relinking compacted CONTENTS. */
    ok &= expect_int("compacted replacement survives close",
                     closed[2].itemType, 909, f0334CloseCompacts);
    /* ReDMCSB: CHEST.C F0334 lines 118-132 iterates exactly eight G0425 slots,
     * so hidden tail entries never enter the compacted close output. */
    ok &= expect_int("hidden tail excluded from compacted close",
                     closed[7].itemType, 107, f0334CloseCompacts);
    /* ReDMCSB: CHAMPION.C F0300/F0301 lines 582-615 refresh Load after slot
     * object removal/addition; after F0334 close, transient G0425 weight is gone. */
    ok &= expect_int("close recomputes stale champion load",
                     m11_inventory_get_load(&state, 0), 13, f0301Load);
    /* ReDMCSB: CHEST.C F0334 lines 113-114 returns immediately when no chest
     * is open, leaving no container weight snapshot. */
    ok &= expect_int("no-open close returns zero",
                     m11_inventory_pc34_close_chest_with_weight_snapshot(&state, 0,
                                                                        closed, 8,
                                                                        &snapshotWeight),
                     0, f0334CloseCompacts);
    /* ReDMCSB: CHEST.C F0334 lines 113-114 no-open guard prevents any F0140
     * container weight from being observed. */
    ok &= expect_int("no-open snapshot stays zero",
                     snapshotWeight, 0, f0334CloseCompacts);

    printf("inventoryChestLoadInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
