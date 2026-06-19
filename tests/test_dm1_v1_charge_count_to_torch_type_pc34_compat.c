#include "firestaff/dm1/v1/charge_count_to_torch_type_pc34_compat.h"

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
    /* DATA.C:263 G0029 init:
     *   { 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 }
     */
    const int *t = dm1_v1_charge_count_to_torch_type_table_pc34();
    int n = dm1_v1_charge_count_to_torch_type_size_pc34();
    int i;
    static const int kExpected[16] = {
        0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
    };
    CHECK(t != 0);
    CHECK(n == 16);
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] == kExpected[i]);
    }
}

static void test_lookup_function(void)
{
    /* OBJECT.C:178 dispatch — for each valid charge count, the lookup
     * function returns the expected torch type.
     */
    int i;
    static const int kExpected[16] = {
        0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
    };
    for (i = 0; i < 16; ++i) {
        CHECK(dm1_v1_charge_count_to_torch_type_pc34(i) == kExpected[i]);
    }
    /* Out-of-range returns 0 (sentinel). */
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(-1) == 0);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(16) == 0);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(999) == 0);
}

static void test_bucket_boundaries(void)
{
    /* Bucket design (DATA.C:263 + DATA.C:926):
     *   0 charges      -> type 0
     *   1..3 charges   -> type 1
     *   4..7 charges   -> type 2
     *   8..15 charges  -> type 3
     */
    /* Type 0 bucket: {0}. */
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(0) == 0);
    /* Type 1 bucket: {1, 2, 3}. */
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(1) == 1);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(2) == 1);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(3) == 1);
    /* Type 2 bucket: {4, 5, 6, 7}. */
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(4) == 2);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(5) == 2);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(6) == 2);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(7) == 2);
    /* Type 3 bucket: {8..15}. */
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(8) == 3);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(9) == 3);
    CHECK(dm1_v1_charge_count_to_torch_type_pc34(15) == 3);
}

static void test_first_last_count_for_type(void)
{
    /* The bucket boundaries can also be queried via the first/last
     * count helpers (the inverse mapping).
     */
    /* Type 0: charge count 0..0 */
    CHECK(dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(0) == 0);
    CHECK(dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(0) == 0);
    /* Type 1: 1..3 */
    CHECK(dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(1) == 1);
    CHECK(dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(1) == 3);
    /* Type 2: 4..7 */
    CHECK(dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(2) == 4);
    CHECK(dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(2) == 7);
    /* Type 3: 8..15 */
    CHECK(dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(3) == 8);
    CHECK(dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(3) == 15);
    /* Out-of-range torch type returns -1. */
    CHECK(dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(-1) == -1);
    CHECK(dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(4) == -1);
    CHECK(dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(99) == -1);
}

static void test_monotonic_and_range(void)
{
    /* The values form a monotonically non-decreasing curve
     * (each bucket boundary is a non-decrease step, within-bucket
     * values are equal).
     */
    const int *t = dm1_v1_charge_count_to_torch_type_table_pc34();
    int i;
    for (i = 0; i < 15; ++i) {
        CHECK(t[i] <= t[i + 1]);
    }
    for (i = 0; i < 16; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 3);
    }
}

static void test_run_accepted(void)
{
    DM1_V1_ChargeCountToTorchTypeResultPc34 r;
    int ok = dm1_v1_charge_count_to_torch_type_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 16);
    CHECK(r.tableEntries[0] == 0);
    CHECK(r.tableEntries[1] == 1);
    CHECK(r.tableEntries[3] == 1);
    CHECK(r.tableEntries[4] == 2);
    CHECK(r.tableEntries[7] == 2);
    CHECK(r.tableEntries[8] == 3);
    CHECK(r.tableEntries[15] == 3);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.tableIsMonotonic == 1);
    CHECK(r.tableHas4DistinctValues == 1);
    CHECK(r.firstEntry0 == 1);
    CHECK(r.lastEntry3 == 1);
    CHECK(r.allWithinRange0to3 == 1);
    CHECK(r.bucketBoundariesCorrect == 1);
    CHECK(r.lookupFunctionInRange == 1);
    CHECK(r.lookupOutOfRangeReturnsZero == 1);
    CHECK(r.bucketBoundaries0183ToType0123Correct == 1);
    CHECK(r.dispatchFunctionCorrect == 1);
    /* Cross-check that the table entries match the lookup function. */
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] ==
              dm1_v1_charge_count_to_torch_type_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_bucket_boundaries();
    test_first_last_count_for_type();
    test_monotonic_and_range();
    test_run_accepted();
    printf("dm1_v1_charge_count_to_torch_type: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}