#include "firestaff/dm1/v1/palette_changes_no_changes_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:23  - declaration of G0017_auc_Graphic562_PaletteChanges_NoChanges[16]
 * - DATA.C:136 - PC 3.4 init { 0, 1, 2, ..., 15 } (identity)
 * - DATA.C:590 - Atari init { 0, 10, 20, ..., 150 } (10-step ramp)
 * - ACTIDRAW.C:160 - creature action icon blit
 * - BLTSHRNK.C:530 - identity path for VIDEO_BlitShrinkWithPaletteChanges
 * - DUNVIEW.C:4518/5973 - explosion/projectile blit
 * - STARTUP2.C:822 - G0075 initialization
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806 (Graphics.dat init-table gates batches
 * 1+2). This gate is a non-mirror-candidate contract for the G0017
 * identity palette-changes table.
 */

enum {
    kTableSize   = 16,
    kMaxByte     = 255,
    kOutOfRange  = 0,
    kIdentityLen = 16
};

static const unsigned char s_g0017[kTableSize] = {
    /* 0  */  0,
    /* 1  */  1,
    /* 2  */  2,
    /* 3  */  3,
    /* 4  */  4,
    /* 5  */  5,
    /* 6  */  6,
    /* 7  */  7,
    /* 8  */  8,
    /* 9  */  9,
    /* 10 */ 10,
    /* 11 */ 11,
    /* 12 */ 12,
    /* 13 */ 13,
    /* 14 */ 14,
    /* 15 */ 15
};

const unsigned char *
dm1_v1_palette_changes_no_changes_table_pc34(void)
{
    return s_g0017;
}

int
dm1_v1_palette_changes_no_changes_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_changes_no_changes_pc34(int palette_index)
{
    if (palette_index < 0 || palette_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0017[palette_index];
}

/* Returns 1 iff the table is the bytewise-identity permutation
 * {0, 1, 2, ..., 15}, which is the PC 3.4 init. The Atari init is
 * a 10-step ramp and would return 0.
 */
int
dm1_v1_palette_changes_no_changes_is_identity_pc34(void)
{
    int i;
    for (i = 0; i < kIdentityLen; ++i) {
        if ((int)s_g0017[i] != i) {
            return 0;
        }
    }
    return 1;
}

int
dm1_v1_palette_changes_no_changes_run_pc34(
    DM1_V1_PaletteChangesNoChangesResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int first_entry_0 = 1;
    int last_entry_15 = 1;
    int all_values_in_range_0to255 = 1;
    int is_identity_permutation = 1;
    int monotonic_strictly_increasing = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int is_same_as_identity_helper = 1;
    static const unsigned char kExpected[kTableSize] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0017[i];
        if (s_g0017[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: first entry is 0, last entry is 15. */
    if (s_g0017[0] != 0) first_entry_0 = 0;
    if (s_g0017[kTableSize - 1] != 15) last_entry_15 = 0;
    out->firstEntry0 = first_entry_0;
    out->lastEntry15 = last_entry_15;

    /* Phase 3: all values in [0, 255]. */
    for (i = 0; i < kTableSize; ++i) {
        if ((int)s_g0017[i] < 0 || (int)s_g0017[i] > kMaxByte) {
            all_values_in_range_0to255 = 0;
        }
    }
    out->allValuesInRange0to255 = all_values_in_range_0to255;

    /* Phase 4: identity permutation (s_g0017[i] == i for all i). */
    for (i = 0; i < kTableSize; ++i) {
        if ((int)s_g0017[i] != i) {
            is_identity_permutation = 0;
        }
    }
    out->isIdentityPermutation = is_identity_permutation;

    /* Phase 5: strictly increasing (since identity permutation
     * implies this).
     */
    for (i = 1; i < kTableSize; ++i) {
        if ((int)s_g0017[i] <= (int)s_g0017[i - 1]) {
            monotonic_strictly_increasing = 0;
        }
    }
    out->monotonicStrictlyIncreasing = monotonic_strictly_increasing;

    /* Phase 6: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_changes_no_changes_pc34(i) !=
            (int)kExpected[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 7: out-of-range lookup returns 0. */
    if (dm1_v1_palette_changes_no_changes_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_changes_no_changes_pc34(16) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_changes_no_changes_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    /* Phase 8: identity helper consistency. */
    if (dm1_v1_palette_changes_no_changes_is_identity_pc34() !=
        is_identity_permutation) {
        is_same_as_identity_helper = 0;
    }
    out->isSameAsIdentityHelper = is_same_as_identity_helper;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntry0 &&
        out->lastEntry15 &&
        out->allValuesInRange0to255 &&
        out->isIdentityPermutation &&
        out->monotonicStrictlyIncreasing &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero &&
        out->isSameAsIdentityHelper;
    out->assertionCount = 10;
    return out->accepted;
}