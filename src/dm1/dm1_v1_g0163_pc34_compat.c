#include "firestaff/dm1/v1/G0163_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0163):
 * - DUNVIEW.C:440 - declaration
 * - DUNVIEW.C:581 - PC 3.4 EN init (12 rows × 8 cols)
 * - DUNVIEW.C:6206/6289/6356 - read sites (F0100_DUNGEONVIEW_DrawWallSetBitmap,
 *                                 F0113_DUNGEONVIEW_DrawField)
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kRows   = 12,
    kCols   = 8,
    kSize   = 96,    /* kRows * kCols */
    kOutOfRange = -1
};

/* G0163_aauc_Graphic558_Frame_Walls[12][8] PC 3.4 EN init (DUNVIEW.C:581).
 * Rows: 0=D0C, 1=D0L, 2=D0R, 3=D1C, 4=D1L, 5=D1R, 6=D2C, 7=D2L, 8=D2R,
 *       9=D3C, 10=D3L, 11=D3R */
static const unsigned char s_g0163[kSize] = {
    /* D0C */ 0, 223, 0, 135, 0, 0, 0, 0,
    /* D0L */ 0, 31, 0, 135, 16, 136, 0, 0,
    /* D0R */ 192, 223, 0, 135, 16, 136, 0, 0,
    /* D1C */ 32, 191, 9, 119, 128, 111, 48, 0,
    /* D1L */ 0, 63, 9, 119, 128, 111, 192, 0,
    /* D1R */ 160, 223, 9, 119, 128, 111, 0, 0,
    /* D2C */ 60, 163, 20, 90, 72, 71, 16, 0,
    /* D2L */ 0, 74, 20, 90, 72, 71, 61, 0,
    /* D2R */ 149, 223, 20, 90, 72, 71, 0, 0,
    /* D3C */ 74, 149, 25, 75, 64, 51, 18, 0,
    /* D3L */ 0, 83, 25, 75, 64, 51, 32, 0,
    /* D3R */ 139, 223, 25, 75, 64, 51, 0, 0
};

const unsigned char *
dm1_v1_g0163_table_pc34(void)
{
    return s_g0163;
}

int
dm1_v1_g0163_size_pc34(void)
{
    return kSize;
}

int
dm1_v1_g0163_get_pc34(int row_index, int col_index)
{
    if (row_index < 0 || row_index >= kRows) {
        return kOutOfRange;
    }
    if (col_index < 0 || col_index >= kCols) {
        return kOutOfRange;
    }
    return (int)s_g0163[row_index * kCols + col_index];
}

int
dm1_v1_g0163_run_pc34(
    DM1_V1_G0163ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int row_d0c_d3c_valid = 1;
    int row_d1c_d2c_valid = 1;
    int row_d3c_d0c_valid = 1;
    int all_rows_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kSize; ++i) {
        out->tableEntries[i] = (int)s_g0163[i];
    }
    out->tableSize = kSize;

    /* Spot-check D0C row (index 0): {0, 223, 0, 135, 0, 0, 0, 0}. */
    if (s_g0163[0] != 0 || s_g0163[1] != 223 || s_g0163[2] != 0 || s_g0163[3] != 135) {
        row_d0c_d3c_valid = 0;
    }
    out->rowD0CD3CValid = row_d0c_d3c_valid;

    /* Spot-check D1C row (index 3): {32, 191, 9, 119, 128, 111, 48, 0}. */
    if (s_g0163[3 * kCols + 0] != 32 || s_g0163[3 * kCols + 1] != 191) {
        row_d1c_d2c_valid = 0;
    }
    out->rowD1CD2CValid = row_d1c_d2c_valid;

    /* Spot-check D3C row (index 9): {74, 149, 25, 75, 64, 51, 18, 0}. */
    if (s_g0163[9 * kCols + 0] != 74 || s_g0163[9 * kCols + 1] != 149) {
        row_d3c_d0c_valid = 0;
    }
    out->rowD3CD0CValid = row_d3c_d0c_valid;

    /* All values fit in uint8_t. */
    for (i = 0; i < kSize; ++i) {
        if (s_g0163[i] > 255) all_rows_in_byte_range = 0;
    }
    out->allRowsInByteRange = all_rows_in_byte_range;

    {
        static const unsigned char kExpected[kSize] = {
            0, 223, 0, 135, 0, 0, 0, 0,
            0, 31, 0, 135, 16, 136, 0, 0,
            192, 223, 0, 135, 16, 136, 0, 0,
            32, 191, 9, 119, 128, 111, 48, 0,
            0, 63, 9, 119, 128, 111, 192, 0,
            160, 223, 9, 119, 128, 111, 0, 0,
            60, 163, 20, 90, 72, 71, 16, 0,
            0, 74, 20, 90, 72, 71, 61, 0,
            149, 223, 20, 90, 72, 71, 0, 0,
            74, 149, 25, 75, 64, 51, 18, 0,
            0, 83, 25, 75, 64, 51, 32, 0,
            139, 223, 25, 75, 64, 51, 0, 0
        };
        for (i = 0; i < kSize; ++i) {
            if (s_g0163[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kSize; ++i) {
        int row = i / kCols;
        int col = i % kCols;
        if (dm1_v1_g0163_get_pc34(row, col) != (int)s_g0163[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0163_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0163_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0163_get_pc34(kRows, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0163_get_pc34(0, kCols) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0163_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->rowD0CD3CValid &&
        out->rowD1CD2CValid &&
        out->rowD3CD0CValid &&
        out->allRowsInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 8;
    return out->accepted;
}
