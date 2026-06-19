#include "firestaff/dm1/v1/ordered_cells_to_attack_pc34_compat.h"

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

static void test_table_dimensions(void)
{
    /* DATA.C:234 — G0023 is 8 rows x 4 cols. */
    CHECK(dm1_v1_ordered_cells_to_attack_row_count_pc34() == 8);
    CHECK(dm1_v1_ordered_cells_to_attack_col_count_pc34() == 4);
}

static void test_table_values(void)
{
    /* DATA.C:234-243 G0023 init:
     *   row 0 (S from NW or SW): { 0, 1, 3, 2 }
     *   row 1 (S from NE or SE): { 1, 0, 2, 3 }
     *   row 2 (W from NW or NE): { 1, 2, 0, 3 }
     *   row 3 (W from SE or SW): { 2, 1, 3, 0 }
     *   row 4 (N from NW or SW): { 3, 2, 0, 1 }
     *   row 5 (N from SE or NE): { 2, 3, 1, 0 }
     *   row 6 (E from NW or NE): { 0, 3, 1, 2 }
     *   row 7 (E from SE or SW): { 3, 0, 2, 1 }
     */
    const int *t = dm1_v1_ordered_cells_to_attack_table_pc34();
    int row;
    static const int kExpected[8][4] = {
        { 0, 1, 3, 2 },
        { 1, 0, 2, 3 },
        { 1, 2, 0, 3 },
        { 2, 1, 3, 0 },
        { 3, 2, 0, 1 },
        { 2, 3, 1, 0 },
        { 0, 3, 1, 2 },
        { 3, 0, 2, 1 }
    };
    CHECK(t != 0);
    for (row = 0; row < 8; ++row) {
        CHECK(dm1_v1_ordered_cells_to_attack_pc34(row, 0) == kExpected[row][0]);
        CHECK(dm1_v1_ordered_cells_to_attack_pc34(row, 1) == kExpected[row][1]);
        CHECK(dm1_v1_ordered_cells_to_attack_pc34(row, 2) == kExpected[row][2]);
        CHECK(dm1_v1_ordered_cells_to_attack_pc34(row, 3) == kExpected[row][3]);
    }
}

static void test_value_range(void)
{
    /* All values must be in {0, 1, 2, 3} (the 4 direction constants). */
    int row, col;
    for (row = 0; row < 8; ++row) {
        for (col = 0; col < 4; ++col) {
            int v = dm1_v1_ordered_cells_to_attack_pc34(row, col);
            CHECK(v >= 0);
            CHECK(v <= 3);
        }
    }
}

static void test_each_row_permutation(void)
{
    /* Each row is a permutation of {0, 1, 2, 3}. */
    int row;
    for (row = 0; row < 8; ++row) {
        CHECK(dm1_v1_ordered_cells_to_attack_row_is_permutation_pc34(row) == 1);
    }
    /* OOB rejects. */
    CHECK(dm1_v1_ordered_cells_to_attack_row_is_permutation_pc34(-1) == 0);
    CHECK(dm1_v1_ordered_cells_to_attack_row_is_permutation_pc34(8) == 0);
    CHECK(dm1_v1_ordered_cells_to_attack_row_is_permutation_pc34(999) == 0);
}

static void test_dispatch_function(void)
{
    /* PROJEXPL.C:1302 — F0229_GROUP_SetOrderedCellsToAttack calls
     * CopyBytes(G0023[row], out, 4). Our dispatch wrapper writes the
     * 4 ordered-direction values and returns 1.
     */
    int a, b, c, d;
    int rc;
    int row;
    static const int kExpected[8][4] = {
        { 0, 1, 3, 2 },
        { 1, 0, 2, 3 },
        { 1, 2, 0, 3 },
        { 2, 1, 3, 0 },
        { 3, 2, 0, 1 },
        { 2, 3, 1, 0 },
        { 0, 3, 1, 2 },
        { 3, 0, 2, 1 }
    };
    for (row = 0; row < 8; ++row) {
        rc = dm1_v1_ordered_cells_to_attack_dispatch_pc34(row, &a, &b, &c, &d);
        CHECK(rc == 1);
        CHECK(a == kExpected[row][0]);
        CHECK(b == kExpected[row][1]);
        CHECK(c == kExpected[row][2]);
        CHECK(d == kExpected[row][3]);
    }
}

static void test_dispatch_oob(void)
{
    /* OOB rows return 0; out params are set to -1 sentinel. */
    int a = 0, b = 0, c = 0, d = 0;
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(-1, &a, &b, &c, &d) == 0);
    CHECK(a == -1);
    CHECK(b == -1);
    CHECK(c == -1);
    CHECK(d == -1);
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(8, &a, &b, &c, &d) == 0);
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(999, &a, &b, &c, &d) == 0);
}

static void test_dispatch_null_safe(void)
{
    /* NULL out params return 0 and don't crash. */
    int dummy = 0;
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, 0, &dummy, &dummy, &dummy) == 0);
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, &dummy, 0, &dummy, &dummy) == 0);
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, &dummy, &dummy, &dummy, 0) == 0);
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, &dummy, &dummy, 0, &dummy) == 0);
    CHECK(dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, 0, 0, 0, 0) == 0);
}

static void test_lookup_oob(void)
{
    /* The lookup function returns -1 for any OOB row/col. */
    CHECK(dm1_v1_ordered_cells_to_attack_pc34(-1, 0) == -1);
    CHECK(dm1_v1_ordered_cells_to_attack_pc34(8, 0) == -1);
    CHECK(dm1_v1_ordered_cells_to_attack_pc34(0, -1) == -1);
    CHECK(dm1_v1_ordered_cells_to_attack_pc34(0, 4) == -1);
    CHECK(dm1_v1_ordered_cells_to_attack_pc34(999, 999) == -1);
}

static void test_row_count_summary(void)
{
    /* 8 rows x 4 cols = 32 entries. Sanity-check that the table size
     * matches the source declaration in DATA.C:29.
     */
    CHECK(dm1_v1_ordered_cells_to_attack_row_count_pc34() * 
          dm1_v1_ordered_cells_to_attack_col_count_pc34() == 32);
}

static void test_run_accepted(void)
{
    DM1_V1_OrderedCellsToAttackResultPc34 r;
    int ok = dm1_v1_ordered_cells_to_attack_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableRows == 8);
    CHECK(r.tableCols == 4);
    /* Spot-check first + last rows. */
    CHECK(r.tableEntries[0] == 0);
    CHECK(r.tableEntries[1] == 1);
    CHECK(r.tableEntries[2] == 3);
    CHECK(r.tableEntries[3] == 2);
    CHECK(r.tableEntries[7 * 4 + 0] == 3);
    CHECK(r.tableEntries[7 * 4 + 1] == 0);
    CHECK(r.tableEntries[7 * 4 + 2] == 2);
    CHECK(r.tableEntries[7 * 4 + 3] == 1);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.rowCountIs8 == 1);
    CHECK(r.colCountIs4 == 1);
    CHECK(r.allValuesInRange0to3 == 1);
    CHECK(r.eachRowPermutationOf4Directions == 1);
    CHECK(r.all8RowsDistinct == 1);
    CHECK(r.dispatchFunctionCorrect == 1);
    CHECK(r.dispatchOutOfRangeRejects == 1);
    CHECK(r.dispatchNullSafe == 1);
    /* Quirk: also spot-check the full copy from tableEntries matches
     * the source-of-truth lookup function.
     */
    for (i = 0; i < 8 * 4; ++i) {
        int row = i / 4;
        int col = i % 4;
        CHECK(r.tableEntries[i] ==
              dm1_v1_ordered_cells_to_attack_pc34(row, col));
    }
}

int main(void)
{
    test_table_dimensions();
    test_table_values();
    test_value_range();
    test_each_row_permutation();
    test_dispatch_function();
    test_dispatch_oob();
    test_dispatch_null_safe();
    test_lookup_oob();
    test_row_count_summary();
    test_run_accepted();
    printf("dm1_v1_ordered_cells_to_attack: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}