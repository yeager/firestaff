#include "firestaff/dm1/v1/print_text_masks1_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0065_al_Graphic562_PrintTextMasksX):
 * - DATA.C:113/535/1356 - declaration + PC 3.4 init + Atari init
 * - DATA.C:113 - declaration
 * - DATA.C:535 - PC 3.4 init
 * - F0040_TEXT_Print mask-1 path
 * - DATA.C:1354-1355 - post-1.3 Atari init (same values)
 * - F0040_TEXT_Print - 32-bit text-print mask table
 *
 * Disjoint from pass784-790 + pass791-799 + pass798-863.
 * - G0065_al_Graphic562_PrintTextMasks1 — see DATA.C reference
 */

/* Variable: G0065_al_Graphic562_PrintTextMasks1 */
enum {
    kTableSize  = 4
};

static const unsigned int s_g0065[kTableSize] = {
    /* 0 */ 0x7FFF7FFFU,
    /* 1 */ 0x3FFF3FFFU,
    /* 2 */ 0x1FFF1FFFU,
    /* 3 */ 0x0FFF0FFFU
};

const unsigned int *
dm1_v1_print_text_masks1_table_pc34(void)
{
    return s_g0065;
}

int
dm1_v1_print_text_masks1_size_pc34(void)
{
    return kTableSize;
}

unsigned int
dm1_v1_print_text_masks1_get_pc34(int mask_index)
{
    if (mask_index < 0 || mask_index >= kTableSize) {
        return 0;
    }
    return s_g0065[mask_index];
}

int
dm1_v1_print_text_masks1_run_pc34(
    DM1_V1_PRINT_TEXT_MASKS1ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0065[i];
    }
    out->tableSize = kTableSize;

    {
        static const unsigned int kExpected[kTableSize] = {
            0x7FFF7FFFU, 0x3FFF3FFFU, 0x1FFF1FFFU, 0x0FFF0FFFU
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0065[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if ((int)dm1_v1_print_text_masks1_get_pc34(i) != (int)s_g0065[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_print_text_masks1_get_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_print_text_masks1_get_pc34(kTableSize) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_print_text_masks1_get_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 4;
    return out->accepted;
}
