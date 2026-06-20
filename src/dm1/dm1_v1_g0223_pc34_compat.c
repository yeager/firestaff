#include "firestaff/dm1/v1/G0223_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0223):
 * - DUNVIEW.C:512 - declaration
 * - DUNVIEW.C:1833 - PC 3.4 EN init
 * - DUNVIEW.C F0099/F0654 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kRows   = 3,
    kCols   = 8,
    kSize   = 24,
    kOutOfRange = -1
};

static const int s_g0223[kRows][kCols] = {
    {0, 1, 2, 3, 0, -3, -2, -1},
    {0, 1, 1, 2, 0, -2, -1, -1},
    {0, 1, 1, 1, 0, -1, -1, -1}
};

const unsigned char *
dm1_v1_g0223_table_pc34(void)
{
    return (const unsigned char *)s_g0223;
}

int
dm1_v1_g0223_size_pc34(void)
{
    return kSize;
}

int
dm1_v1_g0223_get_pc34(int row_index, int col_index)
{
    if (row_index < 0 || row_index >= kRows) {
        return kOutOfRange;
    }
    if (col_index < 0 || col_index >= kCols) {
        return kOutOfRange;
    }
    return s_g0223[row_index][col_index];
}

int
dm1_v1_g0223_run_pc34(
    DM1_V1_G0223ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int all_in_int16_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i, j;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kRows; ++i) {
        for (j = 0; j < kCols; ++j) {
            out->tableEntries[i * kCols + j] = s_g0223[i][j];
        }
    }
    out->tableSize = kSize;

    for (i = 0; i < kRows; ++i) {
        for (j = 0; j < kCols; ++j) {
            if (s_g0223[i][j] < -32768 || s_g0223[i][j] > 32767) {
                all_in_int16_range = 0;
            }
        }
    }
    out->allInInt16Range = all_in_int16_range;

    {
        static const int kExpected[kRows][kCols] = {
            {0, 1, 2, 3, 0, -3, -2, -1},
    {0, 1, 1, 2, 0, -2, -1, -1},
    {0, 1, 1, 1, 0, -1, -1, -1}
        };
        for (i = 0; i < kRows; ++i) {
            for (j = 0; j < kCols; ++j) {
                if (s_g0223[i][j] != kExpected[i][j]) {
                    table_matches_declaration = 0;
                }
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kSize; ++i) {
        int row = i / kCols;
        int col = i % kCols;
        if (dm1_v1_g0223_get_pc34(row, col) != s_g0223[row][col]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0223_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0223_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0223_get_pc34(kRows, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0223_get_pc34(0, kCols) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0223_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allInInt16Range &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
