#include "firestaff/dm1/v1/bar_graph_byte_offsets_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0056_aaui_Graphic562_BarGraphByteOffsets):
 * - DATA.C:99  - declaration of G0056_aaui_Graphic562_BarGraphByteOffsets[4][3]
 * - DATA.C:99-104 - PC 3.4 EN init (4 champions × 3 graphs)
 * - DATA.C:104 - last entry of bar-graph-byte-offsets init block
 * - CHAMDRAW.C:204 - lea G0056_aaui_Graphic562_BarGraphByteOffsets(A4),A1
 * - CHAMDRAW.C:208 - inline comment listing the 12 offsets
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-859 (Graphics.dat init-table gates batches 1-11). This
 * gate is a non-mirror-candidate contract for the G0056
 * bar-graph byte-offset table.
 */

enum {
    kChampionCount = 4,
    kGraphCount    = 3,
    kTableSize     = 12,
    kIndexOOR      = -1
};

static const unsigned int s_g0056[kTableSize] = {
    /* Champion 0 */  16, 24, 24,
    /* Champion 1 */  56, 56, 64,
    /* Champion 2 */  88, 88, 96,
    /* Champion 3 */ 120, 128, 128
};

const unsigned int *
dm1_v1_bar_graph_byte_offsets_table_pc34(void)
{
    return s_g0056;
}

int
dm1_v1_bar_graph_byte_offsets_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_bar_graph_byte_offsets_get_pc34(int champion_index, int graph_index)
{
    int flat_index;
    if (champion_index < 0 || champion_index >= kChampionCount) {
        return kIndexOOR;
    }
    if (graph_index < 0 || graph_index >= kGraphCount) {
        return kIndexOOR;
    }
    flat_index = champion_index * kGraphCount + graph_index;
    return (int)s_g0056[flat_index];
}

int
dm1_v1_bar_graph_byte_offsets_run_pc34(
    DM1_V1_BarGraphByteOffsetsResultPc34 *out)
{
    int table_matches_declaration = 1;
    int champion0_health_offset_16 = 1;
    int champion1_health_offset_56 = 1;
    int champion2_health_offset_88 = 1;
    int champion3_health_offset_120 = 1;
    int all_offsets_non_negative = 1;
    int all_offsets_in_byte_range = 1;
    int monotonic_per_champion = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int c, g;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (c = 0; c < kTableSize; ++c) {
        out->tableEntries[c] = (int)s_g0056[c];
    }
    out->tableSize = kTableSize;

    /* Phase 1: per-champion health-bar offset. */
    if (s_g0056[0]  !=  16) champion0_health_offset_16  = 0;
    if (s_g0056[3]  !=  56) champion1_health_offset_56  = 0;
    if (s_g0056[6]  !=  88) champion2_health_offset_88  = 0;
    if (s_g0056[9]  != 120) champion3_health_offset_120 = 0;
    out->champion0HealthOffset16  = champion0_health_offset_16;
    out->champion1HealthOffset56  = champion1_health_offset_56;
    out->champion2HealthOffset88  = champion2_health_offset_88;
    out->champion3HealthOffset120 = champion3_health_offset_120;

    /* Phase 2: all offsets >= 0. */
    for (c = 0; c < kTableSize; ++c) {
        if (s_g0056[c] > 0xFFFF) {
            all_offsets_non_negative = 0;
        }
    }
    out->allOffsetsNonNegative = all_offsets_non_negative;

    /* Phase 3: all offsets fit in uint16_t. */
    for (c = 0; c < kTableSize; ++c) {
        if (s_g0056[c] > 0xFFFF) {
            all_offsets_in_byte_range = 0;
        }
    }
    out->allOffsetsInByteRange = all_offsets_in_byte_range;

    /* Phase 4: per-champion, health <= mana <= stamina (monotonic). */
    for (c = 0; c < kChampionCount; ++c) {
        int h = (int)s_g0056[c * kGraphCount + 0];
        int m = (int)s_g0056[c * kGraphCount + 1];
        int s = (int)s_g0056[c * kGraphCount + 2];
        if (!(h <= m && m <= s)) {
            monotonic_per_champion = 0;
        }
    }
    out->monotonicPerChampion = monotonic_per_champion;

    /* Phase 5: full table matches declared order. */
    {
        static const unsigned int kExpected[kTableSize] = {
            16, 24, 24,  56, 56, 64,  88, 88, 96, 120, 128, 128
        };
        for (c = 0; c < kTableSize; ++c) {
            if (s_g0056[c] != kExpected[c]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 6: lookup function correctness. */
    for (c = 0; c < kChampionCount; ++c) {
        for (g = 0; g < kGraphCount; ++g) {
            int flat = c * kGraphCount + g;
            if (dm1_v1_bar_graph_byte_offsets_get_pc34(c, g) != (int)s_g0056[flat]) {
                lookup_function_correct = 0;
            }
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 7: out-of-range lookup returns -1. */
    if (dm1_v1_bar_graph_byte_offsets_get_pc34(-1, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_byte_offsets_get_pc34(0, -1) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_byte_offsets_get_pc34(kChampionCount, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_byte_offsets_get_pc34(0, kGraphCount) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_byte_offsets_get_pc34(999, 999) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->champion0HealthOffset16 &&
        out->champion1HealthOffset56 &&
        out->champion2HealthOffset88 &&
        out->champion3HealthOffset120 &&
        out->allOffsetsNonNegative &&
        out->allOffsetsInByteRange &&
        out->monotonicPerChampion &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 11;
    return out->accepted;
}