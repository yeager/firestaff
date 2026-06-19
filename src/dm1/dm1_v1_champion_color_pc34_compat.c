#include "firestaff/dm1/v1/champion_color_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:84  - declaration of G0046_auc_Graphic562_ChampionColor[4]
 * - DATA.C:423 - PC 3.4 init { 7, 11, 8, 14 }
 * - DATA.C:1095 - post-1.3 Atari init (same values)
 * - CHAMDRAW.C:48/51/60/300/342/1022 - champion-icon/portrait fill
 * - CHAMPION.C:986/1016/1052 - champion name text color
 * - REVIVE.C:868/872/887 - champion name text color in resurrect
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807 (Graphics.dat init-table gates
 * batches 1+2+3). This gate is a non-mirror-candidate contract
 * for the G0046 champion-color assignment table.
 */

enum {
    kChampionCount    = 4,
    kLeaderIndex      = 0,
    kLeaderColor      = 7,
    kFirstFollowerIdx = 1,
    kFirstFollowerClr = 11,
    kMinColor         = 0,
    kMaxColor         = 15,   /* 4-bit palette index */
    kOutOfRange       = 0
};

static const unsigned char s_g0046[kChampionCount] = {
    /* 0  */  7,  /* leader */
    /* 1  */ 11,  /* 1st follower */
    /* 2  */  8,
    /* 3  */ 14
};

const unsigned char *
dm1_v1_champion_color_table_pc34(void)
{
    return s_g0046;
}

int
dm1_v1_champion_color_size_pc34(void)
{
    return kChampionCount;
}

int
dm1_v1_champion_color_pc34(int champion_index)
{
    if (champion_index < 0 || champion_index >= kChampionCount) {
        return kOutOfRange;
    }
    return (int)s_g0046[champion_index];
}

int
dm1_v1_champion_color_leader_pc34(void)
{
    return (int)s_g0046[kLeaderIndex];
}

int
dm1_v1_champion_color_run_pc34(
    DM1_V1_ChampionColorResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int leader_color_is_7 = 1;
    int first_follower_color_is_11 = 1;
    int all_colors_distinct = 1;
    int all_colors_in_range_0to15 = 1;
    int lookup_function_in_range = 1;
    int lookup_out_of_range_returns_zero = 1;
    int dispatch_by_champion_index_correct = 1;
    static const unsigned char kExpected[kChampionCount] = { 7, 11, 8, 14 };
    int seen[16] = { 0 };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < kChampionCount; ++i) {
        out->tableEntries[i] = (int)s_g0046[i];
        if (s_g0046[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: leader color is 7 (LIGHT_GRAY). */
    if ((int)s_g0046[kLeaderIndex] != kLeaderColor) {
        leader_color_is_7 = 0;
    }
    out->leaderColorIs7 = leader_color_is_7;

    /* Phase 3: first follower color is 11 (LIGHT_CYAN). */
    if ((int)s_g0046[kFirstFollowerIdx] != kFirstFollowerClr) {
        first_follower_color_is_11 = 0;
    }
    out->firstFollowerColorIs11 = first_follower_color_is_11;

    /* Phase 4: all 4 colors distinct (so each champion is visually
     * identifiable in the panel).
     */
    for (i = 0; i < kChampionCount; ++i) {
        int v = (int)s_g0046[i];
        if (seen[v]) {
            all_colors_distinct = 0;
        }
        seen[v] = 1;
    }
    out->allColorsDistinct = all_colors_distinct;

    /* Phase 5: all values in [0, 15] (4-bit palette index). */
    for (i = 0; i < kChampionCount; ++i) {
        int v = (int)s_g0046[i];
        if (v < kMinColor || v > kMaxColor) {
            all_colors_in_range_0to15 = 0;
        }
    }
    out->allColorsInRange0to15 = all_colors_in_range_0to15;

    /* Phase 6: lookup function correctness. */
    for (i = 0; i < kChampionCount; ++i) {
        if (dm1_v1_champion_color_pc34(i) != (int)kExpected[i]) {
            lookup_function_in_range = 0;
        }
    }
    out->lookupFunctionInRange = lookup_function_in_range;

    /* Phase 7: out-of-range lookup returns 0. */
    if (dm1_v1_champion_color_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_champion_color_pc34(4) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_champion_color_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    /* Phase 8: dispatch by champion index (the canonical use site is
     * `G0046[ChampionIndex]` for champion-icon fill). Verify the
     * helper matches.
     */
    if (dm1_v1_champion_color_leader_pc34() != 7) {
        dispatch_by_champion_index_correct = 0;
    }
    if (dm1_v1_champion_color_pc34(0) != dm1_v1_champion_color_leader_pc34()) {
        dispatch_by_champion_index_correct = 0;
    }
    out->dispatchByChampionIndexCorrect = dispatch_by_champion_index_correct;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->leaderColorIs7 &&
        out->firstFollowerColorIs11 &&
        out->allColorsDistinct &&
        out->allColorsInRange0to15 &&
        out->lookupFunctionInRange &&
        out->lookupOutOfRangeReturnsZero &&
        out->dispatchByChampionIndexCorrect;
    out->assertionCount = 9;
    return out->accepted;
}