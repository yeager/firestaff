#include "firestaff/dm1/v1/ordered_cells_to_attack_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:29  - declaration of G0023_aac_Graphic562_OrderedCellsToAttack[8][4]
 * - DATA.C:234-243 - PC 3.4 init
 * - DATA.C:878-887 - post-1.3 Atari init (same 8 rows)
 * - PROJEXPL.C:1302 - F0229_GROUP_SetOrderedCellsToAttack reads G0023
 * - DEFS.H:7640-7660 - F0228/F0229 declarations
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index).
 * This gate is a non-mirror-candidate contract for the G0023
 * creature-AI attack-direction priority table.
 */

enum {
    kRowCount = 8,
    kColCount = 4,
    kDirSouth = 0,
    kDirEast  = 1,
    kDirNorth = 2,
    kDirWest  = 3,
    kMinDirection = 0,
    kMaxDirection = 3,
    kOutOfRangeDirection = -1
};

/* PC 3.4 init per DATA.C:234-243. Row direction labels:
 *   0: South from NW or SW    -> {0, 1, 3, 2}
 *   1: South from NE or SE    -> {1, 0, 2, 3}
 *   2: West from NW or NE     -> {1, 2, 0, 3}
 *   3: West from SE or SW     -> {2, 1, 3, 0}
 *   4: North from NW or SW    -> {3, 2, 0, 1}
 *   5: North from SE or NE    -> {2, 3, 1, 0}
 *   6: East from NW or NE     -> {0, 3, 1, 2}
 *   7: East from SE or SW     -> {3, 0, 2, 1}
 */
static const int s_g0023[kRowCount][kColCount] = {
    /* 0  S from NW or SW */ { 0, 1, 3, 2 },
    /* 1  S from NE or SE */ { 1, 0, 2, 3 },
    /* 2  W from NW or NE */ { 1, 2, 0, 3 },
    /* 3  W from SE or SW */ { 2, 1, 3, 0 },
    /* 4  N from NW or SW */ { 3, 2, 0, 1 },
    /* 5  N from SE or NE */ { 2, 3, 1, 0 },
    /* 6  E from NW or NE */ { 0, 3, 1, 2 },
    /* 7  E from SE or SW */ { 3, 0, 2, 1 }
};

const int *
dm1_v1_ordered_cells_to_attack_table_pc34(void)
{
    return &s_g0023[0][0];
}

int
dm1_v1_ordered_cells_to_attack_row_count_pc34(void)
{
    return kRowCount;
}

int
dm1_v1_ordered_cells_to_attack_col_count_pc34(void)
{
    return kColCount;
}

int
dm1_v1_ordered_cells_to_attack_pc34(int row, int col)
{
    if (row < 0 || row >= kRowCount ||
        col < 0 || col >= kColCount) {
        return kOutOfRangeDirection;
    }
    return s_g0023[row][col];
}

/* Each row must be a permutation of {0, 1, 2, 3} (all 4 directions
 * exactly once) — this is the structural invariant for an attack-
 * direction priority list. Return 1 if row is a permutation, 0
 * otherwise. Out-of-range rows return 0.
 */
int
dm1_v1_ordered_cells_to_attack_row_is_permutation_pc34(int row)
{
    int seen[4] = { 0, 0, 0, 0 };
    int i;
    if (row < 0 || row >= kRowCount) {
        return 0;
    }
    for (i = 0; i < kColCount; ++i) {
        int v = s_g0023[row][i];
        if (v < kMinDirection || v > kMaxDirection) {
            return 0;
        }
        if (seen[v]) {
            return 0;  /* duplicate */
        }
        seen[v] = 1;
    }
    return 1;
}

/* PROJEXPL.C:1302 dispatch: F0007_MAIN_CopyBytes(G0023[row], out, 4).
 * The wrapper writes the 4 ordered-direction values to out_first..out_
 * fourth and returns 1 on success. Returns 0 (and writes -1 sentinels)
 * for any OOB row or NULL output buffer.
 */
int
dm1_v1_ordered_cells_to_attack_dispatch_pc34(
    int row,
    int *out_first,
    int *out_second,
    int *out_third,
    int *out_fourth)
{
    if (out_first)   *out_first   = kOutOfRangeDirection;
    if (out_second)  *out_second  = kOutOfRangeDirection;
    if (out_third)   *out_third   = kOutOfRangeDirection;
    if (out_fourth)  *out_fourth  = kOutOfRangeDirection;
    if (!out_first || !out_second || !out_third || !out_fourth) {
        return 0;
    }
    if (row < 0 || row >= kRowCount) {
        return 0;
    }
    *out_first  = s_g0023[row][0];
    *out_second = s_g0023[row][1];
    *out_third  = s_g0023[row][2];
    *out_fourth = s_g0023[row][3];
    return 1;
}

int
dm1_v1_ordered_cells_to_attack_run_pc34(
    DM1_V1_OrderedCellsToAttackResultPc34 *out)
{
    int i;
    int j;
    int table_matches_declaration = 1;
    int row_count_is_8 = 1;
    int col_count_is_4 = 1;
    int all_values_in_range_0to3 = 1;
    int each_row_permutation_of_4_directions = 1;
    int all_8_rows_distinct = 1;
    int dispatch_function_correct = 1;
    int dispatch_out_of_range_rejects = 1;
    int dispatch_null_safe = 1;
    static const int kExpected[kRowCount][kColCount] = {
        { 0, 1, 3, 2 },
        { 1, 0, 2, 3 },
        { 1, 2, 0, 3 },
        { 2, 1, 3, 0 },
        { 3, 2, 0, 1 },
        { 2, 3, 1, 0 },
        { 0, 3, 1, 2 },
        { 3, 0, 2, 1 }
    };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values (8 rows x 4 cols = 32 ints) and
     * per-entry cross-check against the expected init.
     */
    for (i = 0; i < kRowCount; ++i) {
        for (j = 0; j < kColCount; ++j) {
            out->tableEntries[i * kColCount + j] = s_g0023[i][j];
            if (s_g0023[i][j] != kExpected[i][j]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableRows = kRowCount;
    out->tableCols = kColCount;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: partition sizes per DATA.C:234 (the comment says
     * "8 attack-direction rows of 4 ordered cells each").
     */
    if (kRowCount != 8) row_count_is_8 = 0;
    if (kColCount != 4) col_count_is_4 = 0;
    out->rowCountIs8 = row_count_is_8;
    out->colCountIs4 = col_count_is_4;

    /* Phase 3: all entries in [0, 3] (the four valid direction
     * constants: SOUTH=0, EAST=1, NORTH=2, WEST=3).
     */
    for (i = 0; i < kRowCount; ++i) {
        for (j = 0; j < kColCount; ++j) {
            int v = s_g0023[i][j];
            if (v < kMinDirection || v > kMaxDirection) {
                all_values_in_range_0to3 = 0;
            }
        }
    }
    out->allValuesInRange0to3 = all_values_in_range_0to3;

    /* Phase 4: each row is a permutation of {0, 1, 2, 3}. */
    for (i = 0; i < kRowCount; ++i) {
        if (!dm1_v1_ordered_cells_to_attack_row_is_permutation_pc34(i)) {
            each_row_permutation_of_4_directions = 0;
        }
    }
    out->eachRowPermutationOf4Directions = each_row_permutation_of_4_directions;

    /* Phase 5: all 8 rows distinct. Each row is an ordered direction
     * tuple for one (attack-direction, attacker-position) pair, so
     * the 8 rows must all be distinct permutations of {0,1,2,3}.
     * Compare each pair of rows (8*7/2 = 28 pairs).
     */
    for (i = 0; i < kRowCount; ++i) {
        for (j = i + 1; j < kRowCount; ++j) {
            int match = 1;
            int k;
            for (k = 0; k < kColCount; ++k) {
                if (s_g0023[i][k] != s_g0023[j][k]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                all_8_rows_distinct = 0;
            }
        }
    }
    out->all8RowsDistinct = all_8_rows_distinct;

    /* Phase 6: dispatch function correctness. For each valid row,
     * dispatch_pc34 must write the same 4 values as the source
     * table.
     */
    for (i = 0; i < kRowCount; ++i) {
        int a, b, c, d;
        int rc = dm1_v1_ordered_cells_to_attack_dispatch_pc34(i, &a, &b, &c, &d);
        if (!rc ||
            a != s_g0023[i][0] || b != s_g0023[i][1] ||
            c != s_g0023[i][2] || d != s_g0023[i][3]) {
            dispatch_function_correct = 0;
        }
    }
    out->dispatchFunctionCorrect = dispatch_function_correct;

    /* Phase 7: dispatch OOB rejection. */
    if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(-1, &i, &i, &i, &i) != 0) {
        dispatch_out_of_range_rejects = 0;
    }
    if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(8, &i, &i, &i, &i) != 0) {
        dispatch_out_of_range_rejects = 0;
    }
    if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(999, &i, &i, &i, &i) != 0) {
        dispatch_out_of_range_rejects = 0;
    }
    out->dispatchOutOfRangeRejects = dispatch_out_of_range_rejects;

    /* Phase 8: dispatch null-safety. Passing NULL for any out param
     * returns 0 and does not crash.
     */
    {
        int dummy;
        if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, 0, &dummy, &dummy, &dummy) != 0) {
            dispatch_null_safe = 0;
        }
        if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, &dummy, 0, &dummy, &dummy) != 0) {
            dispatch_null_safe = 0;
        }
        if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, &dummy, &dummy, &dummy, 0) != 0) {
            dispatch_null_safe = 0;
        }
        if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, &dummy, &dummy, 0, &dummy) != 0) {
            dispatch_null_safe = 0;
        }
        if (dm1_v1_ordered_cells_to_attack_dispatch_pc34(0, 0, 0, 0, 0) != 0) {
            dispatch_null_safe = 0;
        }
    }
    out->dispatchNullSafe = dispatch_null_safe;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->rowCountIs8 &&
        out->colCountIs4 &&
        out->allValuesInRange0to3 &&
        out->eachRowPermutationOf4Directions &&
        out->all8RowsDistinct &&
        out->dispatchFunctionCorrect &&
        out->dispatchOutOfRangeRejects &&
        out->dispatchNullSafe;
    out->assertionCount = 12;
    return out->accepted;
}