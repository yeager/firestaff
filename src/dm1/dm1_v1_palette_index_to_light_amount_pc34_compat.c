#include "firestaff/dm1/v1/palette_index_to_light_amount_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:46  - declaration of G0040_ai_Graphic562_PaletteIndexToLightAmount[6]
 * - DATA.C:360 - PC 3.4 init { 99, 75, 50, 25, 1, 0 }
 * - DATA.C:1089 - post-1.3 Atari init (same values)
 * - PANEL.C:419 - AL1040_pi_LightAmount = G0040_ai_Graphic562_PaletteIndexToLightAmount
 * - PANEL.C:421-423 - the walk-while-greater palette selection loop
 * - G0304_i_DungeonViewPaletteIndex - the global that the loop assigns to
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power). This gate is a non-mirror-
 * candidate contract for the G0040 palette-index threshold table.
 */

enum {
    kTableSize        = 6,
    kBrightestIndex   = 0,
    kDarkestIndex     = 5,
    kBrightestThreshold = 99,
    kDarkestThreshold   = 0,
    kOutOfRangeLightAmount = 0
};

static const int s_g0040[kTableSize] = {
    /* 0 */ 99,  /* brightest palette threshold */
    /* 1 */ 75,
    /* 2 */ 50,
    /* 3 */ 25,
    /* 4 */  1,
    /* 5 */  0   /* darkest palette threshold */
};

const int *
dm1_v1_palette_index_to_light_amount_table_pc34(void)
{
    return s_g0040;
}

int
dm1_v1_palette_index_to_light_amount_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_index_to_light_amount_pc34(int palette_index)
{
    if (palette_index < 0 || palette_index >= kTableSize) {
        return kOutOfRangeLightAmount;
    }
    return s_g0040[palette_index];
}

/* PANEL.C:421-423 — walk-while-greater palette selection.
 *
 * Source semantics (verbatim from PANEL.C:419-423):
 *   AL1040_pi_LightAmount = G0040_ai_Graphic562_PaletteIndexToLightAmount;
 *   if (L1036_i_TotalLightAmount > 0) {
 *       AL1039_ui_PaletteIndex = 0;
 *       while (*AL1040_pi_LightAmount++ > L1036_i_TotalLightAmount) {
 *           AL1039_ui_PaletteIndex++;
 *       }
 *   } else {
 *       AL1039_ui_PaletteIndex = 5;
 *   }
 *   G0304_i_DungeonViewPaletteIndex = AL1039_ui_PaletteIndex;
 *
 * Note the post-increment (*p++): the pointer advances during the
 * test, but the test itself reads the value BEFORE the increment.
 * For 6 entries, the loop runs at most 6 times; after the last
 * increment, *p reads past the end. The C source relies on the
 * memory layout to have a 7th word (likely G0041_ai_Graphic562_Box_
 * ViewportFloppyZzzCross[0] = 174) which is > any TotalLightAmount
 * so the loop exits.
 *
 * For our pure-C gate we walk a copy of the table, which is safe.
 */
int
dm1_v1_palette_index_to_light_amount_select_pc34(int total_light_amount)
{
    if (total_light_amount <= 0) {
        return kDarkestIndex;
    }
    /* Walk while *p > total_light_amount. */
    {
        int palette_index = kBrightestIndex;
        int i;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0040[i] > total_light_amount) {
                palette_index++;
            } else {
                break;
            }
        }
        return palette_index;
    }
}

int
dm1_v1_palette_index_to_light_amount_brightest_index_pc34(void)
{
    return kBrightestIndex;
}

int
dm1_v1_palette_index_to_light_amount_darkest_index_pc34(void)
{
    return kDarkestIndex;
}

int
dm1_v1_palette_index_to_light_amount_brightest_threshold_pc34(void)
{
    return kBrightestThreshold;
}

int
dm1_v1_palette_index_to_light_amount_run_pc34(
    DM1_V1_PaletteIndexToLightAmountResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int first_entry_99 = 1;
    int last_entry_0 = 1;
    int monotonically_non_increasing = 1;
    int all_within_range_0_99 = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int select_palette_index_brightest_for_99_plus = 1;
    int select_palette_index_darkest_for_0 = 1;
    int select_palette_index_boundaries_correct = 1;
    static const int kExpected[kTableSize] = { 99, 75, 50, 25, 1, 0 };
    int prev;
    int boundary_75_to_76;
    int boundary_50_to_51;
    int boundary_25_to_26;
    int boundary_2_to_1;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0040[i];
        if (s_g0040[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: first entry is 99 (brightest palette threshold). */
    if (s_g0040[kBrightestIndex] != kBrightestThreshold) {
        first_entry_99 = 0;
    }
    out->firstEntry99 = first_entry_99;

    /* Phase 3: last entry is 0 (darkest palette threshold). */
    if (s_g0040[kDarkestIndex] != kDarkestThreshold) {
        last_entry_0 = 0;
    }
    out->lastEntry0 = last_entry_0;

    /* Phase 4: monotonic non-increasing. */
    prev = s_g0040[0];
    for (i = 1; i < kTableSize; ++i) {
        if (s_g0040[i] > prev) {
            monotonically_non_increasing = 0;
        }
        prev = s_g0040[i];
    }
    out->monotonicallyNonIncreasing = monotonically_non_increasing;

    /* Phase 5: all values in [0, 99]. */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0040[i] < 0 || s_g0040[i] > 99) {
            all_within_range_0_99 = 0;
        }
    }
    out->allWithinRange0_99 = all_within_range_0_99;

    /* Phase 6: lookup function returns expected values for each index. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_index_to_light_amount_pc34(i) != kExpected[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 7: out-of-range lookup returns 0 (sentinel). */
    if (dm1_v1_palette_index_to_light_amount_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_index_to_light_amount_pc34(6) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    /* Phase 8: palette-selection function. For TotalLightAmount >= 99
     * (and 100, 200, etc.), selects palette 0 (brightest). The source
     * loop is `while (*p++ > total)` so for total = 99, *p = 99 > 99
     * is false, so palette_index stays at 0.
     */
    if (dm1_v1_palette_index_to_light_amount_select_pc34(99) != 0) {
        select_palette_index_brightest_for_99_plus = 0;
    }
    if (dm1_v1_palette_index_to_light_amount_select_pc34(100) != 0) {
        select_palette_index_brightest_for_99_plus = 0;
    }
    if (dm1_v1_palette_index_to_light_amount_select_pc34(200) != 0) {
        select_palette_index_brightest_for_99_plus = 0;
    }
    out->selectPaletteIndexBrightestFor99Plus =
        select_palette_index_brightest_for_99_plus;

    /* Phase 9: palette-selection for TotalLightAmount <= 0 selects
     * palette 5 (darkest).
     */
    if (dm1_v1_palette_index_to_light_amount_select_pc34(0) != 5) {
        select_palette_index_darkest_for_0 = 0;
    }
    if (dm1_v1_palette_index_to_light_amount_select_pc34(-1) != 5) {
        select_palette_index_darkest_for_0 = 0;
    }
    if (dm1_v1_palette_index_to_light_amount_select_pc34(-1000) != 5) {
        select_palette_index_darkest_for_0 = 0;
    }
    out->selectPaletteIndexDarkestFor0 = select_palette_index_darkest_for_0;

    /* Phase 10: palette-selection boundaries. Walk the four threshold
     * boundaries in the table.
     *
     * Thresholds (G0040):
     *   index 0 -> 99: palette 0 for total >= 99
     *   index 1 -> 75: palette 1 for total in [76, 99]
     *                  palette 0 for total >= 99 (i.e. not 75..98 -> PI=1, since 99>total)
     *   index 2 -> 50: palette 2 for total in [51, 75]
     *   index 3 -> 25: palette 3 for total in [26, 50]
     *   index 4 ->  1: palette 4 for total in [2, 25]
     *   index 5 ->  0: palette 5 for total in (-INF, 1] (or 0 explicit)
     *
     * The walk-while-greater semantics: PI starts at 0; we increment
     * PI for each threshold that is strictly greater than total.
     *
     *   total=99: 99>99 false -> PI=0
     *   total=76: 99>76 true (PI=1), 75>76 false -> PI=1
     *   total=75: 99>75 true (PI=1), 75>75 false -> PI=1
     *   total=74: 99>74 true (PI=1), 75>74 true (PI=2), 50>74 false -> PI=2
     *   total=51: 99>51 true (PI=1), 75>51 true (PI=2), 50>51 false -> PI=2
     *   total=50: ... 50>50 false -> PI=2
     *   total=49: 99>49, 75>49, 50>49 true (PI=3), 25>49 false -> PI=3
     *   total=26: ... 25>26 false -> PI=3
     *   total=25: ... 25>25 false -> PI=3
     *   total=24: ... 25>24 true (PI=4), 1>24 false -> PI=4
     *   total=2: ... 1>2 false -> PI=4
     *   total=1: ... 1>1 false -> PI=4
     *   total=0: dark branch -> PI=5
     */
    boundary_75_to_76 =
        (dm1_v1_palette_index_to_light_amount_select_pc34(75) == 1) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(76) == 1) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(74) == 2);
    boundary_50_to_51 =
        (dm1_v1_palette_index_to_light_amount_select_pc34(50) == 2) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(51) == 2) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(49) == 3);
    boundary_25_to_26 =
        (dm1_v1_palette_index_to_light_amount_select_pc34(25) == 3) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(26) == 3) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(24) == 4);
    boundary_2_to_1 =
        (dm1_v1_palette_index_to_light_amount_select_pc34(2) == 4) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(1) == 4) &&
        (dm1_v1_palette_index_to_light_amount_select_pc34(0) == 5);

    if (!boundary_75_to_76 ||
        !boundary_50_to_51 ||
        !boundary_25_to_26 ||
        !boundary_2_to_1) {
        select_palette_index_boundaries_correct = 0;
    }
    out->selectPaletteIndexBoundariesCorrect =
        select_palette_index_boundaries_correct;

    /* Phase 11: boundary test sweep (a few representative totals). */
    {
        struct {
            int total;
            int expected_pi;
        } kCases[] = {
            { 200, 0 }, /* brightest */
            { 100, 0 }, /* brightest */
            {  99, 0 }, /* brightest boundary */
            {  76, 1 }, /* PI 1 boundary high */
            {  75, 1 }, /* PI 1 boundary low */
            {  74, 2 }, /* PI 2 boundary high */
            {  51, 2 }, /* PI 2 boundary high (recheck) */
            {  50, 2 }, /* PI 2 boundary low */
            {  49, 3 }, /* PI 3 boundary high */
            {  26, 3 }, /* PI 3 boundary high (recheck) */
            {  25, 3 }, /* PI 3 boundary low */
            {  24, 4 }, /* PI 4 boundary high */
            {   2, 4 }, /* PI 4 boundary high (recheck) */
            {   1, 4 }, /* PI 4 boundary low */
            {   0, 5 }, /* darkest */
            {  -1, 5 }  /* darkest (negative) */
        };
        int ok = 1;
        int n_cases = (int)(sizeof(kCases) / sizeof(kCases[0]));
        for (i = 0; i < n_cases; ++i) {
            int got = dm1_v1_palette_index_to_light_amount_select_pc34(
                kCases[i].total);
            if (got != kCases[i].expected_pi) {
                ok = 0;
            }
        }
        out->selectPaletteIndexBoundaryTests = ok ? 1 : 0;
    }

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntry99 &&
        out->lastEntry0 &&
        out->monotonicallyNonIncreasing &&
        out->allWithinRange0_99 &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero &&
        out->selectPaletteIndexBrightestFor99Plus &&
        out->selectPaletteIndexDarkestFor0 &&
        out->selectPaletteIndexBoundariesCorrect &&
        out->selectPaletteIndexBoundaryTests;
    out->assertionCount = 12;
    return out->accepted;
}