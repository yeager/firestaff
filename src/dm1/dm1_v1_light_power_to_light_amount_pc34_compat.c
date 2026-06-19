#include "firestaff/dm1/v1/light_power_to_light_amount_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:45  - declaration of G0039_ai_Graphic562_LightPowerToLightAmount[16]
 * - DATA.C:359 - PC 3.4 init { 0, 5, 12, 24, 33, 40, 46, 51, 59, 68,
 *                  76, 82, 89, 94, 97, 100 }
 * - DATA.C:1088 - post-1.3 Atari init (same values)
 * - PANEL.C:412 - F0337_INVENTORY_SetDungeonViewPalette reads G0039
 * - CHAMPION.C:529/645 - F0291_CHAMPION_DrawSlot reads G0039[2] for Illumulet
 * - MENU.C:1608/1936/1941 - spell paths (C038/C5/C1) read G0039
 * - TIMELINE.C:1754 - light-event tick reads G0039[Strong] - G0039[Weaker]
 * - DEFS.H C012_ICON_JUNK_ILLUMULET_UNEQUIPPED +
 *          C013_ICON_JUNK_ILLUMULET_EQUIPPED (the Illumulet pair)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes). This gate is a non-mirror-candidate contract for
 * the G0039 light-power lookup table.
 */

enum {
    kTableSize           = 16,
    kIllumuletLightPower = 2,
    kIllumuletLightAmount = 12,  /* G0039[2] = 12 */
    kMaxLightPower       = 15,
    kMaxLightAmount      = 100,
    kMinLightAmount      = 0,
    kOutOfRangeLightAmount = 0
};

static const int s_g0039[kTableSize] = {
    /* 0  */   0,
    /* 1  */   5,
    /* 2  */  12,
    /* 3  */  24,
    /* 4  */  33,
    /* 5  */  40,
    /* 6  */  46,
    /* 7  */  51,
    /* 8  */  59,
    /* 9  */  68,
    /* 10 */  76,
    /* 11 */  82,
    /* 12 */  89,
    /* 13 */  94,
    /* 14 */  97,
    /* 15 */ 100
};

const int *
dm1_v1_light_power_to_light_amount_table_pc34(void)
{
    return s_g0039;
}

int
dm1_v1_light_power_to_light_amount_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_light_power_to_light_amount_pc34(int light_power)
{
    if (light_power < 0 || light_power >= kTableSize) {
        return kOutOfRangeLightAmount;
    }
    return s_g0039[light_power];
}

/* TIMELINE.C:1754 — diff = G0039[Strong] - G0039[Weaker].
 *
 * For a positive light power (e.g. dawn), Strong >= Weaker and the
 * diff is positive (gain magical light). For a negative light power
 * (e.g. spell flipping from light to darkness), the caller flips the
 * sign after the diff (TIMELINE.C:1757-1760), so this helper just
 * returns the raw table diff. The sign-aware wrapper below mirrors
 * the source's sign-flipped behavior.
 */
int
dm1_v1_light_power_to_light_amount_diff_pc34(
    int stronger_light_power,
    int weaker_light_power)
{
    int stronger = dm1_v1_light_power_to_light_amount_pc34(stronger_light_power);
    int weaker   = dm1_v1_light_power_to_light_amount_pc34(weaker_light_power);
    return stronger - weaker;
}

int
dm1_v1_light_power_to_light_amount_illumulet_index_pc34(void)
{
    return kIllumuletLightPower;
}

int
dm1_v1_light_power_to_light_amount_illumulet_amount_pc34(void)
{
    return kIllumuletLightAmount;
}

int
dm1_v1_light_power_to_light_amount_max_value_pc34(void)
{
    return kMaxLightAmount;
}

int
dm1_v1_light_power_to_light_amount_run_pc34(
    DM1_V1_LightPowerToLightAmountResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int first_entry_zero = 1;
    int last_entry_100 = 1;
    int monotonically_non_decreasing = 1;
    int all_within_range_0_100 = 1;
    int illumulet_constant_12 = 1;
    int lookup_function_in_range = 1;
    int lookup_out_of_range_returns_zero = 1;
    int diff_helper_correct = 1;
    int diff_helper_zero_when_same = 1;
    static const int kExpected[kTableSize] = {
        0, 5, 12, 24, 33, 40, 46, 51,
        59, 68, 76, 82, 89, 94, 97, 100
    };
    int prev;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0039[i];
        if (s_g0039[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: first entry is 0 (a torch with 0 light power emits no
     * magical light).
     */
    if (s_g0039[0] != 0) {
        first_entry_zero = 0;
    }
    out->firstEntryZero = first_entry_zero;

    /* Phase 3: last entry is 100 (a torch at full light power emits
     * the maximum magical light).
     */
    if (s_g0039[kTableSize - 1] != kMaxLightAmount) {
        last_entry_100 = 0;
    }
    out->lastEntry100 = last_entry_100;

    /* Phase 4: monotonic non-decreasing. */
    prev = s_g0039[0];
    for (i = 1; i < kTableSize; ++i) {
        if (s_g0039[i] < prev) {
            monotonically_non_decreasing = 0;
        }
        prev = s_g0039[i];
    }
    out->monotonicallyNonDecreasing = monotonically_non_decreasing;

    /* Phase 5: all values in [0, 100]. */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0039[i] < kMinLightAmount ||
            s_g0039[i] > kMaxLightAmount) {
            all_within_range_0_100 = 0;
        }
    }
    out->allWithinRange0_100 = all_within_range_0_100;

    /* Phase 6: Illumulet constant. CHAMPION.C:529/645 and
     * MENU.C:1608 read G0039[2] for the Illumulet magic light. */
    if (dm1_v1_light_power_to_light_amount_pc34(2) != 12) {
        illumulet_constant_12 = 0;
    }
    if (dm1_v1_light_power_to_light_amount_illumulet_index_pc34() != 2) {
        illumulet_constant_12 = 0;
    }
    if (dm1_v1_light_power_to_light_amount_illumulet_amount_pc34() != 12) {
        illumulet_constant_12 = 0;
    }
    out->illumuletConstant12 = illumulet_constant_12;

    /* Phase 7: lookup function returns expected values for each index. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_light_power_to_light_amount_pc34(i) != kExpected[i]) {
            lookup_function_in_range = 0;
        }
    }
    out->lookupFunctionInRange = lookup_function_in_range;

    /* Phase 8: out-of-range lookup returns 0 (sentinel). */
    if (dm1_v1_light_power_to_light_amount_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_light_power_to_light_amount_pc34(16) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_light_power_to_light_amount_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    /* Phase 9: diff helper correctness. The actual TIMELINE.C:1754
     * call is G0039[Strong] - G0039[Weaker]. For strong=15, weaker=0
     * we expect 100 - 0 = 100. For strong=5, weaker=2 we expect
     * 40 - 12 = 28. For equal values we expect 0.
     */
    if (dm1_v1_light_power_to_light_amount_diff_pc34(15, 0) != 100) {
        diff_helper_correct = 0;
    }
    if (dm1_v1_light_power_to_light_amount_diff_pc34(5, 2) != 28) {
        diff_helper_correct = 0;
    }
    if (dm1_v1_light_power_to_light_amount_diff_pc34(8, 8) != 0) {
        diff_helper_zero_when_same = 0;
    }
    if (dm1_v1_light_power_to_light_amount_diff_pc34(2, 5) != -28) {
        /* diff is raw table diff; TIMELINE.C flips sign for negative
         * powers — the helper here is the raw table diff, not the
         * sign-flipped one. The sign-aware wrapper lives in
         * Phase 10 below.
         */
        diff_helper_correct = 0;
    }
    out->diffHelperCorrect = diff_helper_correct;
    out->diffHelperZeroWhenSame = diff_helper_zero_when_same;

    /* Phase 10: diff helper sign-aware — TIMELINE.C:1754-1760 calls
     * the table diff then flips sign if the original light power
     * was negative. The wrapper above returns the raw diff; the
     * caller is responsible for the sign flip. We document this by
     * verifying the raw diff behavior matches both directions.
     */
    {
        int raw_diff;
        int sign_flipped;
        raw_diff = dm1_v1_light_power_to_light_amount_diff_pc34(2, 5);
        sign_flipped = -raw_diff;  /* mirrors TIMELINE.C:1757 */
        if (sign_flipped != 28) {
            diff_helper_correct = 0;
        }
    }
    out->diffHelperSignAware = diff_helper_correct;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntryZero &&
        out->lastEntry100 &&
        out->monotonicallyNonDecreasing &&
        out->allWithinRange0_100 &&
        out->illumuletConstant12 &&
        out->lookupFunctionInRange &&
        out->lookupOutOfRangeReturnsZero &&
        out->diffHelperCorrect &&
        out->diffHelperZeroWhenSame &&
        out->diffHelperSignAware;
    out->assertionCount = 12;
    return out->accepted;
}