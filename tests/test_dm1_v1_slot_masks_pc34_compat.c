#include "firestaff/dm1/v1/slot_masks_pc34_compat.h"

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

static void test_table_values(void)
{
    /* DATA.C:320-358 G0038 init:
     *   0..1 (Ready/Action Hand):    0xFFFF (ANY)
     *   2..5 (Head/Torso/Legs/Feet): 0x0002/0x0008/0x0010/0x0020
     *   6 (Pouch 2):                 0x0100
     *   7 (Quiver Line2 1):          0x0080
     *   8 (Quiver Line1 2):          0x0080
     *   9 (Quiver Line2 2):          0x0080
     *   10 (Neck):                   0x0004
     *   11 (Pouch 1):                0x0100
     *   12 (Quiver Line1 1):         0x0040
     *   13..29 (Backpack):           0xFFFF (ANY)
     *   30..37 (Chest 1..8):         0x0400 (CONTAINER)
     */
    const int *t = dm1_v1_slot_masks_table_pc34();
    int n = dm1_v1_slot_masks_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 38);
    /* Ready/Action hand. */
    CHECK(t[0]  == 0xFFFF);
    CHECK(t[1]  == 0xFFFF);
    /* Head/Torso/Legs/Feet. */
    CHECK(t[2]  == 0x0002);
    CHECK(t[3]  == 0x0008);
    CHECK(t[4]  == 0x0010);
    CHECK(t[5]  == 0x0020);
    /* Pouch 2, Quiver Line2 1, Quiver Line1 2, Quiver Line2 2. */
    CHECK(t[6]  == 0x0100);
    CHECK(t[7]  == 0x0080);
    CHECK(t[8]  == 0x0080);
    CHECK(t[9]  == 0x0080);
    /* Neck, Pouch 1, Quiver Line1 1. */
    CHECK(t[10] == 0x0004);
    CHECK(t[11] == 0x0100);
    CHECK(t[12] == 0x0040);
    /* Backpack line1 1..9 + line2 2..9 = 18 entries. */
    for (i = 13; i < 30; ++i) {
        CHECK(t[i] == 0xFFFF);
    }
    /* Chest 1..8. */
    for (i = 30; i < 38; ++i) {
        CHECK(t[i] == 0x0400);
    }
}

static void test_lookup_function(void)
{
    /* All 38 valid indices return the expected mask. */
    int i;
    const int *t = dm1_v1_slot_masks_table_pc34();
    for (i = 0; i < 38; ++i) {
        CHECK(dm1_v1_slot_masks_pc34(i) == t[i]);
    }
    /* OOB returns 0. */
    CHECK(dm1_v1_slot_masks_pc34(-1) == 0);
    CHECK(dm1_v1_slot_masks_pc34(38) == 0);
    CHECK(dm1_v1_slot_masks_pc34(999) == 0);
}

static void test_compatibility_helper(void)
{
    /* CHAMPION.C:697 / REVIVE.C:307 semantic:
     *   compatible = (slot_mask & G0038[slot_index]) != 0
     *
     * The thing's AllowedSlots bitmask is tested for intersection
     * with the slot's mask.
     */
    /* A head-only thing (mask=0x0002) is compatible with the head
     * slot (index 2, mask=0x0002) but NOT with the torso slot.
     */
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0002, 2) == 1);
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0002, 3) == 0);
    /* A torso-only thing is compatible with the torso slot only. */
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0008, 3) == 1);
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0008, 2) == 0);
    /* A neck-only thing (mask=0x0004) is compatible with the neck
     * slot (index 10).
     */
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0004, 10) == 1);
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0004, 2) == 0);
    /* A container-only thing (mask=0x0400) is compatible with all
     * 8 chest slots (30..37).
     */
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0400, 30) == 1);
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0400, 37) == 1);
    /* The mask=0x0400 (CONTAINER) is NOT compatible with the
     * pouch slot (index 6, mask=0x0100) since they have no
     * overlapping bits.
     */
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0400, 6) == 0);
    /* Backpack accepts anything (mask=0xFFFF). A head-only thing
     * is compatible with backpack.
     */
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0x0002, 13) == 1);
    /* Out-of-range slot index returns 0 (not compatible). */
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0xFFFF, -1) == 0);
    CHECK(dm1_v1_slot_masks_is_compatible_pc34(0xFFFF, 38) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_SlotMasksResultPc34 r;
    int ok = dm1_v1_slot_masks_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 13);
    CHECK(r.tableSize == 38);
    /* Spot-check first/last entries. */
    CHECK(r.tableEntries[0] == 0xFFFF);
    CHECK(r.tableEntries[1] == 0xFFFF);
    CHECK(r.tableEntries[10] == 0x0004);
    CHECK(r.tableEntries[30] == 0x0400);
    CHECK(r.tableEntries[37] == 0x0400);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.readyHandMaskIsAny == 1);
    CHECK(r.actionHandMaskIsAny == 1);
    CHECK(r.bodyPartMasksSingleBit == 1);
    CHECK(r.neckMaskIsNeck == 1);
    CHECK(r.quiverLine1MaskIsQuiverLine1 == 1);
    CHECK(r.quiverLine2MaskIsQuiverLine2 == 1);
    CHECK(r.pouchMaskIsPouch == 1);
    CHECK(r.backpackMasksAreAny == 1);
    CHECK(r.chestMasksAreContainer == 1);
    CHECK(r.lookupFunctionInRange == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    /* Cross-check the result struct's tableEntries match the
     * source-of-truth lookup function.
     */
    for (i = 0; i < 38; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_slot_masks_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_compatibility_helper();
    test_run_accepted();
    printf("dm1_v1_slot_masks: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}