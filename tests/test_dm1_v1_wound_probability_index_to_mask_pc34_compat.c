#include "firestaff/dm1/v1/wound_probability_index_to_mask_pc34_compat.h"

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
    const unsigned char *t =
        dm1_v1_wound_probability_index_to_mask_table_pc34();
    int n = dm1_v1_wound_probability_index_to_mask_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    /* DATA.C:243 G0024 = { MASK0x0020_WOUND_FEET (0x20),
     *                      MASK0x0010_WOUND_LEGS (0x10),
     *                      MASK0x0008_WOUND_TORSO (0x08),
     *                      MASK0x0004_WOUND_HEAD (0x04) }
     */
    CHECK(t[0] == 0x20);
    CHECK(t[1] == 0x10);
    CHECK(t[2] == 0x08);
    CHECK(t[3] == 0x04);
}

static void test_lookup_function(void)
{
    /* PROJEXPL.C:1386 AllowedWound = G0024[WoundProbabilityIndex] */
    CHECK(dm1_v1_wound_probability_index_to_mask_pc34(0) == 0x20);
    CHECK(dm1_v1_wound_probability_index_to_mask_pc34(1) == 0x10);
    CHECK(dm1_v1_wound_probability_index_to_mask_pc34(2) == 0x08);
    CHECK(dm1_v1_wound_probability_index_to_mask_pc34(3) == 0x04);
    CHECK(dm1_v1_wound_probability_index_to_mask_pc34(4) == 0);
    CHECK(dm1_v1_wound_probability_index_to_mask_pc34(-1) == 0);
}

static void test_test_mask_constants(void)
{
    /* DEFS.H:736-741
     * MASK0x0001_WOUND_READY_HAND = 0x0001
     * MASK0x0004_WOUND_HEAD       = 0x0004
     * MASK0x0008_WOUND_TORSO      = 0x0008
     * MASK0x0010_WOUND_LEGS       = 0x0010
     * MASK0x0020_WOUND_FEET       = 0x0020
     */
    CHECK(dm1_v1_wound_probability_ready_hand_mask_pc34() == 0x0001);
    CHECK(dm1_v1_wound_probability_test_mask_pc34() == 0x0070);
    CHECK(dm1_v1_wound_probability_index_count_pc34() == 4);
}

static void test_branch_decision(void)
{
    /* PROJEXPL.C:1378: if (WoundTest & 0x0070) -> lookup;
     *                  else                -> READY_HAND.
     * Branch returns 1 (use READY_HAND fallback) iff the test mask
     * bits 4,5,6 are all clear.
     */
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0x0000) == 1);
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0x000F) == 1);
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0x0010) == 0);
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0x0020) == 0);
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0x0040) == 0);
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0x0070) == 0);
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0x00FF) == 0);
    CHECK(dm1_v1_wound_probability_test_branch_pc34(0xFFFF) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_WoundProbabilityIndexToMaskResultPc34 r;
    int ok = dm1_v1_wound_probability_index_to_mask_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 7);
    CHECK(r.tableEntries[0] == 0x20);
    CHECK(r.tableEntries[1] == 0x10);
    CHECK(r.tableEntries[2] == 0x08);
    CHECK(r.tableEntries[3] == 0x04);
    CHECK(r.allUnique == 1);
    CHECK(r.correctOrdering == 1);
    CHECK(r.allMasksInDefs == 1);
    CHECK(r.lookupBranchCorrect == 1);
    CHECK(r.fallbackBranchCorrect == 1);
    CHECK(r.lookupBranchGuardCorrect == 1);
    CHECK(r.declarationMatchesInit == 1);
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_test_mask_constants();
    test_branch_decision();
    test_run_accepted();
    printf("dm1_v1_wound_probability_index_to_mask: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
