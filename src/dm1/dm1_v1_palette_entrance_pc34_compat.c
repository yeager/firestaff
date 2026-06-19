#include "firestaff/dm1/v1/palette_entrance_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:26  - declaration of G0020_aui_Graphic562_Palette_Entrance[16]
 * - DATA.C:213 - PC 3.4 init
 * - DATA.C:792 - Atari ST init (different values)
 * - ENTRANCE.C:595 - F0436_STARTEND_FadeToPalette(G0020)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819
 * (Graphics.dat init-table gates batches 1+2+3+4+5+6+7). This gate is
 * a non-mirror-candidate contract for the G0020 entrance-screen
 * palette.
 */

enum {
    kTableSize  = 16,
    kMax12Bit   = 0xFFF,
    kOutOfRange = 0
};

static const unsigned int s_g0020[kTableSize] = {
    /* 0  */  0x000,
    /* 1  */  0x666,
    /* 2  */  0x888,
    /* 3  */  0x840,
    /* 4  */  0xCA8,
    /* 5  */  0x0C0,
    /* 6  */  0x080,
    /* 7  */  0x0A0,
    /* 8  */  0x864,
    /* 9  */  0xF00,
    /* 10 */  0xA86,
    /* 11 */  0x642,
    /* 12 */  0x444,
    /* 13 */  0xAAA,
    /* 14 */  0x620,
    /* 15 */  0xFFF
};

const unsigned int *
dm1_v1_palette_entrance_table_pc34(void)
{
    return s_g0020;
}

int
dm1_v1_palette_entrance_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_entrance_pc34(int palette_index)
{
    if (palette_index < 0 || palette_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0020[palette_index];
}

int
dm1_v1_palette_entrance_run_pc34(
    DM1_V1_PaletteEntranceResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int first_entry_0x000 = 1;
    int last_entry_0xFFF = 1;
    int all_values_12_bit = 1;
    int entry_0_is_black = 1;
    int entry_15_is_white = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    static const unsigned int kExpected[kTableSize] = {
        0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0,
        0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF
    };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0020[i];
        if (s_g0020[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: first/last entry checks. */
    if (s_g0020[0] != 0x000) first_entry_0x000 = 0;
    if (s_g0020[kTableSize - 1] != 0xFFF) last_entry_0xFFF = 0;
    out->firstEntry0x000 = first_entry_0x000;
    out->lastEntry0xFFF  = last_entry_0xFFF;

    /* Phase 3: all values are 12-bit RGB. */
    for (i = 0; i < kTableSize; ++i) {
        if ((int)s_g0020[i] < 0 || (int)s_g0020[i] > kMax12Bit) {
            all_values_12_bit = 0;
        }
    }
    out->allValues12Bit = all_values_12_bit;

    /* Phase 4: black entry (RGB = 0x000) at index 0. */
    if (s_g0020[0] != 0x000) entry_0_is_black = 0;
    out->entry0IsBlack = entry_0_is_black;

    /* Phase 5: white entry (RGB = 0xFFF) at index 15. */
    if (s_g0020[kTableSize - 1] != 0xFFF) entry_15_is_white = 0;
    out->entry15IsWhite = entry_15_is_white;

    /* Phase 6: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_entrance_pc34(i) != (int)kExpected[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 7: out-of-range lookup returns 0. */
    if (dm1_v1_palette_entrance_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_entrance_pc34(16) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_entrance_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntry0x000 &&
        out->lastEntry0xFFF &&
        out->allValues12Bit &&
        out->entry0IsBlack &&
        out->entry15IsWhite &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 9;
    return out->accepted;
}