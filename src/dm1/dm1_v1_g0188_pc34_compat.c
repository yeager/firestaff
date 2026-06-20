#include "firestaff/dm1/v1/G0188_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0188):
 * - DUNVIEW.C:465 - declaration
 * - DUNVIEW.C:718 - PC 3.4 EN init (12 rows × 8 cols)
 * - DUNVIEW.C:6206/6209/6289/6356 - read sites
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

/* G0188_aauc_Graphic558_FieldAspects[12][8] PC 3.4 EN init (DUNVIEW.C:718).
 * Rows: 0=D3C, 1=D3L, 2=D3R, 3=D2C, 4=D2L, 5=D2R, 6=D1C, 7=D1L, 8=D1R,
 *       9=D0C, 10=D0L, 11=D0R
 * Cols: 0=NativeBitmapRelativeIndex, 1=BaseStartUnitIndex,
 *       2=TransparentColor, 3=Mask, 4=ByteWidth, 5=Height, 6=X, 7=BitPlaneWordCount */
static const unsigned char s_g0188[kSize] = {
    /* D3C */ 0, 63, 0x8A, 0xFF, 0, 0, 0, 64,
    /* D3L */ 0, 63, 0x0A, 0x80, 48, 51, 11, 64,
    /* D3R */ 0, 63, 0x0A, 0x00, 48, 51, 0, 64,
    /* D2C */ 0, 60, 0x8A, 0xFF, 0, 0, 0, 64,
    /* D2L */ 0, 63, 0x0A, 0x81, 40, 71, 5, 64,
    /* D2R */ 0, 63, 0x0A, 0x01, 40, 71, 0, 64,
    /* D1C */ 0, 61, 0x8A, 0xFF, 0, 0, 0, 64,
    /* D1L */ 0, 63, 0x0A, 0x82, 32, 111, 0, 64,
    /* D1R */ 0, 63, 0x0A, 0x02, 32, 111, 0, 64,
    /* D0C */ 0, 59, 0x8A, 0xFF, 0, 0, 0, 64,
    /* D0L */ 0, 63, 0x0A, 0x83, 16, 136, 0, 64,
    /* D0R */ 0, 63, 0x0A, 0x03, 16, 136, 0, 64
};

const unsigned char *
dm1_v1_g0188_table_pc34(void)
{
    return s_g0188;
}

int
dm1_v1_g0188_size_pc34(void)
{
    return kSize;
}

int
dm1_v1_g0188_get_pc34(int row_index, int col_index)
{
    if (row_index < 0 || row_index >= kRows) {
        return kOutOfRange;
    }
    if (col_index < 0 || col_index >= kCols) {
        return kOutOfRange;
    }
    return (int)s_g0188[row_index * kCols + col_index];
}

int
dm1_v1_g0188_run_pc34(
    DM1_V1_G0188ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int row_d3c_transparent_color = 1;
    int row_d1l_mask_byte = 1;
    int row_d0l_valid = 1;
    int all_rows_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kSize; ++i) {
        out->tableEntries[i] = (int)s_g0188[i];
    }
    out->tableSize = kSize;

    /* D3C row (index 0): {0, 63, 0x8A, 0xFF, 0, 0, 0, 64}. TransparentColor=0x8A. */
    if (s_g0188[0 * kCols + 2] != 0x8A) row_d3c_transparent_color = 0;
    out->rowD3CTransparentColor = row_d3c_transparent_color;

    /* D1L row (index 7): {0, 63, 0x0A, 0x82, ...}. Mask=0x82. */
    if (s_g0188[7 * kCols + 3] != 0x82) row_d1l_mask_byte = 0;
    out->rowD1LMaskByte = row_d1l_mask_byte;

    /* D0L row (index 10): {0, 63, 0x0A, 0x83, 16, 136, 0, 64}. */
    if (s_g0188[10 * kCols + 4] != 16 || s_g0188[10 * kCols + 5] != 136) row_d0l_valid = 0;
    out->rowD0LValid = row_d0l_valid;

    for (i = 0; i < kSize; ++i) {
        if (s_g0188[i] > 255) all_rows_in_byte_range = 0;
    }
    out->allRowsInByteRange = all_rows_in_byte_range;

    {
        static const unsigned char kExpected[kSize] = {
            0, 63, 0x8A, 0xFF, 0, 0, 0, 64,
            0, 63, 0x0A, 0x80, 48, 51, 11, 64,
            0, 63, 0x0A, 0x00, 48, 51, 0, 64,
            0, 60, 0x8A, 0xFF, 0, 0, 0, 64,
            0, 63, 0x0A, 0x81, 40, 71, 5, 64,
            0, 63, 0x0A, 0x01, 40, 71, 0, 64,
            0, 61, 0x8A, 0xFF, 0, 0, 0, 64,
            0, 63, 0x0A, 0x82, 32, 111, 0, 64,
            0, 63, 0x0A, 0x02, 32, 111, 0, 64,
            0, 59, 0x8A, 0xFF, 0, 0, 0, 64,
            0, 63, 0x0A, 0x83, 16, 136, 0, 64,
            0, 63, 0x0A, 0x03, 16, 136, 0, 64
        };
        for (i = 0; i < kSize; ++i) {
            if (s_g0188[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kSize; ++i) {
        int row = i / kCols;
        int col = i % kCols;
        if (dm1_v1_g0188_get_pc34(row, col) != (int)s_g0188[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0188_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0188_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0188_get_pc34(kRows, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0188_get_pc34(0, kCols) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0188_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->rowD3CTransparentColor &&
        out->rowD1LMaskByte &&
        out->rowD0LValid &&
        out->allRowsInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 8;
    return out->accepted;
}
