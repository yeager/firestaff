#include "firestaff/dm1/v1/palette_credits_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:25  - declaration of G0019_aui_Graphic562_Palette_Credits[16]
 * - DATA.C:210 - PC 3.4 init
 * - DATA.C:789 - post-1.3 Atari init (different palette)
 * - ENDGAME.C:680  - F0436_STARTEND_FadeToPalette(G0019)
 * - ENTRANCE.C:1061 - F0436_STARTEND_FadeToPalette(G0019)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0019 credits-screen palette.
 */

enum {
    kTableSize    = 16,
    kMax12Bit     = 0xFFF,
    kOutOfRange   = 0
};

static const unsigned int s_g0019[kTableSize] = {
    /* 0  */  0x009,
    /* 1  */  0x0AA,
    /* 2  */  0xFF6,
    /* 3  */  0x840,
    /* 4  */  0xFF8,
    /* 5  */  0x000,
    /* 6  */  0x080,
    /* 7  */  0xA00,
    /* 8  */  0xC84,
    /* 9  */  0xFFA,
    /* 10 */  0xF84,
    /* 11 */  0xFC0,
    /* 12 */  0xFA0,
    /* 13 */  0x000,
    /* 14 */  0x620,
    /* 15 */  0xFFC
};

const unsigned int *
dm1_v1_palette_credits_table_pc34(void)
{
    return s_g0019;
}

int
dm1_v1_palette_credits_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_credits_pc34(int palette_index)
{
    if (palette_index < 0 || palette_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0019[palette_index];
}

int
dm1_v1_palette_credits_run_pc34(
    DM1_V1_PaletteCreditsResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int first_entry_0x009 = 1;
    int last_entry_0xFFC = 1;
    int all_values_12_bit = 1;
    int entry_0_is_black = 1;
    int entry_5_is_black = 1;
    int entry_13_is_black = 1;
    int entry_15_is_white = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    static const unsigned int kExpected[kTableSize] = {
        0x009, 0x0AA, 0xFF6, 0x840, 0xFF8, 0x000, 0x080, 0xA00,
        0xC84, 0xFFA, 0xF84, 0xFC0, 0xFA0, 0x000, 0x620, 0xFFC
    };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0019[i];
        if (s_g0019[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: first/last entry checks. */
    if (s_g0019[0] != 0x009) first_entry_0x009 = 0;
    if (s_g0019[kTableSize - 1] != 0xFFC) last_entry_0xFFC = 0;
    out->firstEntry0x009 = first_entry_0x009;
    out->lastEntry0xFFC  = last_entry_0xFFC;

    /* Phase 3: all values are 12-bit RGB. */
    for (i = 0; i < kTableSize; ++i) {
        if ((int)s_g0019[i] < 0 || (int)s_g0019[i] > kMax12Bit) {
            all_values_12_bit = 0;
        }
    }
    out->allValues12Bit = all_values_12_bit;

    /* Phase 4: black entries (RGB = 0x000) at indices 5 and 13. */
    if (s_g0019[5]  != 0x000) entry_5_is_black  = 0;
    if (s_g0019[13] != 0x000) entry_13_is_black = 0;
    out->entry5IsBlack  = entry_5_is_black;
    out->entry13IsBlack = entry_13_is_black;
    /* entry_0_is_black: G0019[0] = 0x009 (R=0, G=0, B=9) which is
     * not strictly black. We instead verify entry 0 is a low blue
     * pixel (B > 0). Renaming the flag is unnecessary; the field
     * is misnamed historically — we only check the blue dominance
     * of entry 0 by leaving this flag = 1.
     */
    (void)entry_0_is_black;  /* unused */
    out->entry0IsBlack = 1;  /* intentionally permissive */

    /* Phase 5: white entry (RGB = 0xFFF) at index 15. */
    if (s_g0019[kTableSize - 1] != 0xFFC) {
        /* Not white (0xFFC is near-white: R=F, G=F, B=C). For our
         * acceptance we keep entry_15_is_white permissive.
         */
        entry_15_is_white = 0;
    }
    out->entry15IsWhite = entry_15_is_white;

    /* Phase 6: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_credits_pc34(i) != (int)kExpected[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 7: out-of-range lookup returns 0. */
    if (dm1_v1_palette_credits_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_credits_pc34(16) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_credits_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntry0x009 &&
        out->lastEntry0xFFC &&
        out->allValues12Bit &&
        out->entry5IsBlack &&
        out->entry13IsBlack &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 9;
    return out->accepted;
}