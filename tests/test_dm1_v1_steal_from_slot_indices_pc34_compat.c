#include "firestaff/dm1/v1/steal_from_slot_indices_pc34_compat.h"

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
    /* DATA.C:244-251 G0025 init:
     *   C10_SLOT_NECK, C11_SLOT_POUCH_1, C13_SLOT_BACKPACK_LINE1_1,
     *   C12_SLOT_QUIVER_LINE1_1, C10_SLOT_NECK, C13_SLOT_BACKPACK_LINE1_1,
     *   C06_SLOT_POUCH_2, C13_SLOT_BACKPACK_LINE1_1
     */
    const unsigned char *t =
        dm1_v1_steal_from_slot_indices_table_pc34();
    int n = dm1_v1_steal_from_slot_indices_size_pc34();
    CHECK(t != 0);
    CHECK(n == 8);
    CHECK(t[0] == 10);  /* NECK */
    CHECK(t[1] == 11);  /* POUCH_1 */
    CHECK(t[2] == 13);  /* BACKPACK_LINE1_1 */
    CHECK(t[3] == 12);  /* QUIVER_LINE1_1 */
    CHECK(t[4] == 10);  /* NECK */
    CHECK(t[5] == 13);  /* BACKPACK_LINE1_1 */
    CHECK(t[6] == 6);   /* POUCH_2 */
    CHECK(t[7] == 13);  /* BACKPACK_LINE1_1 */
}

static void test_lookup_function(void)
{
    /* GROUP.C:1041 G0394[counter] (or G0025[counter] in post-1.3 Atari) */
    CHECK(dm1_v1_steal_from_slot_indices_pc34(0) == 10);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(1) == 11);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(2) == 13);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(3) == 12);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(4) == 10);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(5) == 13);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(6) == 6);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(7) == 13);
    /* Out of range returns 0. */
    CHECK(dm1_v1_steal_from_slot_indices_pc34(8) == 0);
    CHECK(dm1_v1_steal_from_slot_indices_pc34(-1) == 0);
}

static void test_backpack_detection(void)
{
    /* GROUP.C:1041-1045: if (StealFromSlotIndex == C13_SLOT_BACKPACK_LINE1_1)
     *     StealFromSlotIndex += M002_RANDOM(17);
     */
    CHECK(dm1_v1_steal_from_slot_indices_is_backpack_pc34(13) == 1);
    CHECK(dm1_v1_steal_from_slot_indices_is_backpack_pc34(10) == 0);
    CHECK(dm1_v1_steal_from_slot_indices_is_backpack_pc34(11) == 0);
    CHECK(dm1_v1_steal_from_slot_indices_is_backpack_pc34(12) == 0);
    CHECK(dm1_v1_steal_from_slot_indices_is_backpack_pc34(6) == 0);
    CHECK(dm1_v1_steal_from_slot_indices_backpack_random_range_pc34() == 17);
}

static void test_counter_mod_eight(void)
{
    /* GROUP.C:1075 ++Counter &= 0x0007 (mod 8 loop). */
    CHECK(dm1_v1_steal_from_slot_indices_counter_mod_mask_pc34() == 0x0007);
    /* Counter loops 0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7. */
    int i;
    for (i = 0; i < 17; ++i) {
        int counter = i & 0x0007;
        CHECK(counter >= 0);
        CHECK(counter < 8);
        CHECK(dm1_v1_steal_from_slot_indices_pc34(counter) != 0);
    }
}

static void test_run_accepted(void)
{
    DM1_V1_StealFromSlotIndicesResultPc34 r;
    int ok = dm1_v1_steal_from_slot_indices_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 6);
    CHECK(r.tableSize == 8);
    CHECK(r.tableEntries[0] == 10);
    CHECK(r.tableEntries[1] == 11);
    CHECK(r.tableEntries[2] == 13);
    CHECK(r.tableEntries[3] == 12);
    CHECK(r.tableEntries[4] == 10);
    CHECK(r.tableEntries[5] == 13);
    CHECK(r.tableEntries[6] == 6);
    CHECK(r.tableEntries[7] == 13);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.counterModEightLoopCorrect == 1);
    CHECK(r.allWithinSlotRange == 1);
    CHECK(r.backpackSlotsUseRandom == 1);
    CHECK(r.nonBackpackSlotsPassthrough == 1);
    CHECK(r.initialCounterRandomInRange == 1);
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_backpack_detection();
    test_counter_mod_eight();
    test_run_accepted();
    printf("dm1_v1_steal_from_slot_indices: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
