#include "firestaff/dm1/v1/slot_boxes_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_table_size_and_partition(void)
{
    /* DATA.C:264 — G0030 has 46 entries: 8 status hands, 30
     * inventory, 8 chest.
     */
    CHECK(dm1_v1_slot_boxes_size_pc34() == 46);
    CHECK(dm1_v1_slot_boxes_partition_status_hand_count_pc34() == 8);
    CHECK(dm1_v1_slot_boxes_partition_inventory_count_pc34() == 30);
    CHECK(dm1_v1_slot_boxes_partition_chest_count_pc34() == 8);
    CHECK(dm1_v1_slot_boxes_partition_status_hand_offset_pc34() == 0);
    CHECK(dm1_v1_slot_boxes_partition_inventory_offset_pc34() == 8);
    CHECK(dm1_v1_slot_boxes_partition_chest_offset_pc34() == 38);
}

static void test_status_box_hand_entries(void)
{
    /* DATA.C:265-272 — 8 status-box hand entries, Y=10. */
    const DM1_V1_SlotBoxPc34Compat *t = dm1_v1_slot_boxes_table_pc34();
    int i;
    int expected_x[8] = {4, 24, 73, 93, 142, 162, 211, 231};
    for (i = 0; i < 8; ++i) {
        CHECK(t[i].x == expected_x[i]);
        CHECK(t[i].y == 10);
        CHECK(t[i].zoneIndex == 0);
        CHECK(t[i].iconIndex == 0);
    }
    /* +20 stride within each pair (ready->action). */
    CHECK(t[1].x - t[0].x == 20);
    CHECK(t[3].x - t[2].x == 20);
    CHECK(t[5].x - t[4].x == 20);
    CHECK(t[7].x - t[6].x == 20);
    /* +69 stride between consecutive champions. */
    CHECK(t[2].x - t[0].x == 69);
    CHECK(t[4].x - t[2].x == 69);
    CHECK(t[6].x - t[4].x == 69);
}

static void test_inventory_entries(void)
{
    /* DATA.C:273-302 — 30 inventory slots. */
    const DM1_V1_SlotBoxPc34Compat *t = dm1_v1_slot_boxes_table_pc34();
    /* 8 : Ready Hand */
    CHECK(t[8].x == 6);
    CHECK(t[8].y == 53);
    /* 9 : Action Hand */
    CHECK(t[9].x == 62);
    CHECK(t[9].y == 53);
    /* 10 : Head */
    CHECK(t[10].x == 34);
    CHECK(t[10].y == 26);
    /* 11 : Torso */
    CHECK(t[11].x == 34);
    CHECK(t[11].y == 46);
    /* 12 : Legs */
    CHECK(t[12].x == 34);
    CHECK(t[12].y == 66);
    /* 13 : Feet */
    CHECK(t[13].x == 34);
    CHECK(t[13].y == 86);
    /* 14 : Pouch 2 */
    CHECK(t[14].x == 6);
    CHECK(t[14].y == 90);
    /* 15 : Quiver Line2 1 */
    CHECK(t[15].x == 79);
    CHECK(t[15].y == 73);
    /* 16 : Quiver Line1 2 */
    CHECK(t[16].x == 62);
    CHECK(t[16].y == 90);
    /* 17 : Quiver Line2 2 */
    CHECK(t[17].x == 79);
    CHECK(t[17].y == 90);
    /* 18 : Neck */
    CHECK(t[18].x == 6);
    CHECK(t[18].y == 33);
    /* 19 : Pouch 1 */
    CHECK(t[19].x == 6);
    CHECK(t[19].y == 73);
    /* 20 : Quiver Line1 1 */
    CHECK(t[20].x == 62);
    CHECK(t[20].y == 73);
    /* 21 : Backpack Line1 1 */
    CHECK(t[21].x == 66);
    CHECK(t[21].y == 33);
    /* 22..29 : Backpack Line2 2..9 (Y=16) */
    CHECK(t[22].x == 83);
    CHECK(t[22].y == 16);
    CHECK(t[23].x == 100);
    CHECK(t[23].y == 16);
    CHECK(t[24].x == 117);
    CHECK(t[24].y == 16);
    CHECK(t[25].x == 134);
    CHECK(t[25].y == 16);
    CHECK(t[26].x == 151);
    CHECK(t[26].y == 16);
    CHECK(t[27].x == 168);
    CHECK(t[27].y == 16);
    CHECK(t[28].x == 185);
    CHECK(t[28].y == 16);
    CHECK(t[29].x == 202);
    CHECK(t[29].y == 16);
    /* 30..37 : Backpack Line1 2..9 (Y=33) */
    CHECK(t[30].x == 83);
    CHECK(t[30].y == 33);
    CHECK(t[31].x == 100);
    CHECK(t[31].y == 33);
    CHECK(t[32].x == 117);
    CHECK(t[32].y == 33);
    CHECK(t[33].x == 134);
    CHECK(t[33].y == 33);
    CHECK(t[34].x == 151);
    CHECK(t[34].y == 33);
    CHECK(t[35].x == 168);
    CHECK(t[35].y == 33);
    CHECK(t[36].x == 185);
    CHECK(t[36].y == 33);
    CHECK(t[37].x == 202);
    CHECK(t[37].y == 33);
}

static void test_chest_entries(void)
{
    /* DATA.C:303-310 — 8 chest slots, X in [106, 196], Y in [59, 105]. */
    const DM1_V1_SlotBoxPc34Compat *t = dm1_v1_slot_boxes_table_pc34();
    int expected_x[8] = {117, 106, 111, 128, 145, 162, 179, 196};
    int expected_y[8] = { 59,  76,  93,  98, 101, 103, 104, 105};
    int i;
    for (i = 0; i < 8; ++i) {
        CHECK(t[38 + i].x == expected_x[i]);
        CHECK(t[38 + i].y == expected_y[i]);
        CHECK(t[38 + i].zoneIndex == 0);
        CHECK(t[38 + i].iconIndex == 0);
    }
}

static void test_get_x_function(void)
{
    /* OBJECT.C:435 — read X for slot-box drawing. */
    CHECK(dm1_v1_slot_boxes_get_x_pc34(0) == 4);
    CHECK(dm1_v1_slot_boxes_get_x_pc34(7) == 231);
    CHECK(dm1_v1_slot_boxes_get_x_pc34(8) == 6);
    CHECK(dm1_v1_slot_boxes_get_x_pc34(45) == 196);
    /* Out of range returns -1. */
    CHECK(dm1_v1_slot_boxes_get_x_pc34(-1) == -1);
    CHECK(dm1_v1_slot_boxes_get_x_pc34(46) == -1);
}

static void test_get_y_function(void)
{
    /* CHAMDRAW.C:557 — read Y for slot-box drawing. */
    CHECK(dm1_v1_slot_boxes_get_y_pc34(0) == 10);
    CHECK(dm1_v1_slot_boxes_get_y_pc34(38) == 59);
    CHECK(dm1_v1_slot_boxes_get_y_pc34(45) == 105);
    CHECK(dm1_v1_slot_boxes_get_y_pc34(-1) == -1);
    CHECK(dm1_v1_slot_boxes_get_y_pc34(46) == -1);
}

static void test_get_zone_index_function(void)
{
    /* CHAMDRAW.C:562 — read ZoneIndex for F0619_GetSlotBoxBorderCoordinates. */
    int i;
    for (i = 0; i < 46; ++i) {
        CHECK(dm1_v1_slot_boxes_get_zone_index_pc34(i) == 0);
    }
    CHECK(dm1_v1_slot_boxes_get_zone_index_pc34(-1) == -1);
    CHECK(dm1_v1_slot_boxes_get_zone_index_pc34(46) == -1);
}

static void test_get_icon_index_function(void)
{
    /* OBJECT.C:521 — F0488_OBJECT_GetSlotBoxIconIndex returns .IconIndex.
     * In our gate the IconIndex starts at 0 (no icon assigned yet)
     * for all 46 entries; the test verifies that the lookup wires
     * the right slot-box through to the read site.
     */
    int i;
    for (i = 0; i < 46; ++i) {
        CHECK(dm1_v1_slot_boxes_get_icon_index_pc34(i) == 0);
    }
    CHECK(dm1_v1_slot_boxes_get_icon_index_pc34(-1) == -1);
    CHECK(dm1_v1_slot_boxes_get_icon_index_pc34(46) == -1);
}

static void test_get_pointer_function(void)
{
    /* OBJECT.C:435 — L0017_ps_SlotBox = &G0030[SlotBoxIndex]. */
    const DM1_V1_SlotBoxPc34Compat *p;
    int i;
    for (i = 0; i < 46; ++i) {
        p = dm1_v1_slot_boxes_get_pc34(i);
        CHECK(p != 0);
        CHECK(p->x == dm1_v1_slot_boxes_get_x_pc34(i));
        CHECK(p->y == dm1_v1_slot_boxes_get_y_pc34(i));
    }
    /* Out of range returns NULL. */
    CHECK(dm1_v1_slot_boxes_get_pc34(-1) == 0);
    CHECK(dm1_v1_slot_boxes_get_pc34(46) == 0);
}

static void test_partition_classification(void)
{
    int i;
    /* status-hand partition [0..7] */
    for (i = 0; i < 8; ++i) {
        CHECK(dm1_v1_slot_boxes_is_status_hand_pc34(i) == 1);
        CHECK(dm1_v1_slot_boxes_is_inventory_pc34(i) == 0);
        CHECK(dm1_v1_slot_boxes_is_chest_pc34(i) == 0);
    }
    /* inventory partition [8..37] */
    for (i = 8; i < 38; ++i) {
        CHECK(dm1_v1_slot_boxes_is_status_hand_pc34(i) == 0);
        CHECK(dm1_v1_slot_boxes_is_inventory_pc34(i) == 1);
        CHECK(dm1_v1_slot_boxes_is_chest_pc34(i) == 0);
    }
    /* chest partition [38..45] */
    for (i = 38; i < 46; ++i) {
        CHECK(dm1_v1_slot_boxes_is_status_hand_pc34(i) == 0);
        CHECK(dm1_v1_slot_boxes_is_inventory_pc34(i) == 0);
        CHECK(dm1_v1_slot_boxes_is_chest_pc34(i) == 1);
    }
    /* OOB rejects all three. */
    CHECK(dm1_v1_slot_boxes_is_status_hand_pc34(-1) == 0);
    CHECK(dm1_v1_slot_boxes_is_inventory_pc34(-1) == 0);
    CHECK(dm1_v1_slot_boxes_is_chest_pc34(-1) == 0);
    CHECK(dm1_v1_slot_boxes_is_status_hand_pc34(46) == 0);
    CHECK(dm1_v1_slot_boxes_is_inventory_pc34(46) == 0);
    CHECK(dm1_v1_slot_boxes_is_chest_pc34(46) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_SlotBoxesResultPc34 r;
    int ok = dm1_v1_slot_boxes_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 19);
    CHECK(r.tableSize == 46);
    /* Spot-check tableEntries[]. */
    CHECK(r.tableEntries[0 * 4 + 0] == 4);
    CHECK(r.tableEntries[0 * 4 + 1] == 10);
    CHECK(r.tableEntries[0 * 4 + 2] == 0);
    CHECK(r.tableEntries[0 * 4 + 3] == 0);
    CHECK(r.tableEntries[7 * 4 + 0] == 231);
    CHECK(r.tableEntries[7 * 4 + 1] == 10);
    CHECK(r.tableEntries[8 * 4 + 0] == 6);
    CHECK(r.tableEntries[8 * 4 + 1] == 53);
    CHECK(r.tableEntries[21 * 4 + 0] == 66);
    CHECK(r.tableEntries[21 * 4 + 1] == 33);
    CHECK(r.tableEntries[38 * 4 + 0] == 117);
    CHECK(r.tableEntries[38 * 4 + 1] == 59);
    CHECK(r.tableEntries[45 * 4 + 0] == 196);
    CHECK(r.tableEntries[45 * 4 + 1] == 105);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.statusHandCount8 == 1);
    CHECK(r.inventoryCount30 == 1);
    CHECK(r.chestCount8 == 1);
    CHECK(r.partitionOrderingCorrect == 1);
    CHECK(r.allZoneIndexZero == 1);
    CHECK(r.allXWithinViewport == 1);
    CHECK(r.allYWithinPanel == 1);
    CHECK(r.statusBoxYIs10 == 1);
    CHECK(r.chestBoxYIs16Plus == 1);
    CHECK(r.inventoryXIsAtLeast6 == 1);
    CHECK(r.inventoryYWithinInventoryPanel == 1);
    CHECK(r.statusBoxHandXEvenOffset == 1);
    CHECK(r.chestBoxYPixelMonotonic == 1);
    CHECK(r.iconIndexLookupFunctionCorrect == 1);
    CHECK(r.statusBoxIconIndexRange == 1);
    CHECK(r.inventoryIconIndexRange == 1);
    CHECK(r.chestIconIndexRange == 1);
}

int main(void)
{
    test_table_size_and_partition();
    test_status_box_hand_entries();
    test_inventory_entries();
    test_chest_entries();
    test_get_x_function();
    test_get_y_function();
    test_get_zone_index_function();
    test_get_icon_index_function();
    test_get_pointer_function();
    test_partition_classification();
    test_run_accepted();
    printf("dm1_v1_slot_boxes: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}