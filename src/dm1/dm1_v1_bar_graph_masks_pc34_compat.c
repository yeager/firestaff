#include "firestaff/dm1/v1/bar_graph_masks_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0055_aaaui_Graphic562_BarGraphMasks):
 * - DATA.C:93  - declaration of G0055_aaaui_Graphic562_BarGraphMasks[4][3][2]
 * - DATA.C:93-98 - PC 3.4 EN init (4 champions × 3 bars × 2 masks)
 * - DATA.C:98  - last entry of bar-graph-masks init block
 * - CHAMDRAW.C:204 - lea G0056_aaui_Graphic562_BarGraphByteOffsets(A4),A1
 * - CHAMDRAW.C:207 - lea G0055_aaaui_Graphic562_BarGraphMasks(A4),A1
 * - CHAMDRAW.C:208 - inline comment listing the 12 mask pairs
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-859 (Graphics.dat init-table gates batches 1-11). This
 * gate is a non-mirror-candidate contract for the G0055
 * bar-graph bitmask table.
 */

enum {
    kChampionCount = 4,
    kGraphCount    = 3,
    kMaskPair      = 2,
    kTableSize     = 24,    /* 4 * 3 * 2 */
    kIndexOOR      = -1
};

static const unsigned int s_g0055[kTableSize] = {
    /* Champion 0 */ 0x0003, 0xC000,  0x0780, 0x0000,  0x000F, 0x0000,
    /* Champion 1 */ 0x1E00, 0x0000,  0x003C, 0x0000,  0x7800, 0x0000,
    /* Champion 2 */ 0x00F0, 0x0000,  0x0001, 0xE000,  0x03C0, 0x0000,
    /* Champion 3 */ 0x0007, 0x8000,  0x0F00, 0x0000,  0x001E, 0x0000
};

const unsigned int *
dm1_v1_bar_graph_masks_table_pc34(void)
{
    return s_g0055;
}

int
dm1_v1_bar_graph_masks_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_bar_graph_masks_get_pc34(int champion_index, int graph_index, int mask_index)
{
    int flat_index;
    if (champion_index < 0 || champion_index >= kChampionCount) {
        return kIndexOOR;
    }
    if (graph_index < 0 || graph_index >= kGraphCount) {
        return kIndexOOR;
    }
    if (mask_index < 0 || mask_index >= kMaskPair) {
        return kIndexOOR;
    }
    flat_index = (champion_index * kGraphCount + graph_index) * kMaskPair + mask_index;
    return (int)s_g0055[flat_index];
}

int
dm1_v1_bar_graph_masks_run_pc34(
    DM1_V1_BarGraphMasksResultPc34 *out)
{
    int table_matches_declaration = 1;
    int c0_health_mask_pair_0003_C000 = 1;
    int c1_health_mask_pair_1E00_0000 = 1;
    int c2_health_mask_pair_00F0_0000 = 1;
    int c3_health_mask_pair_0007_8000 = 1;
    int all_masks_non_zero = 1;
    int all_masks_in_uint16_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0055[i];
    }
    out->tableSize = kTableSize;

    /* Phase 1: per-champion health-bar mask pair assertions. */
    if (s_g0055[0] != 0x0003 || s_g0055[1] != 0xC000) {
        c0_health_mask_pair_0003_C000 = 0;
    }
    if (s_g0055[6] != 0x1E00 || s_g0055[7] != 0x0000) {
        c1_health_mask_pair_1E00_0000 = 0;
    }
    if (s_g0055[12] != 0x00F0 || s_g0055[13] != 0x0000) {
        c2_health_mask_pair_00F0_0000 = 0;
    }
    if (s_g0055[18] != 0x0007 || s_g0055[19] != 0x8000) {
        c3_health_mask_pair_0007_8000 = 0;
    }
    out->champion0HealthMaskPair0003C000 = c0_health_mask_pair_0003_C000;
    out->champion1HealthMaskPair1E000000 = c1_health_mask_pair_1E00_0000;
    out->champion2HealthMaskPair00F00000 = c2_health_mask_pair_00F0_0000;
    out->champion3HealthMaskPair00078000 = c3_health_mask_pair_0007_8000;

    /* Phase 2: all masks are non-zero OR have a partner (some pairs are
     * "split 16-bit" so one half is 0x0000 while the other carries the bits).
     * We accept all_masks_non_zero=1 only if at least one half of each pair
     * is non-zero.
     */
    {
        int c, g;
        for (c = 0; c < kChampionCount; ++c) {
            for (g = 0; g < kGraphCount; ++g) {
                int m1 = (int)s_g0055[(c * kGraphCount + g) * kMaskPair + 0];
                int m2 = (int)s_g0055[(c * kGraphCount + g) * kMaskPair + 1];
                if (m1 == 0 && m2 == 0) {
                    all_masks_non_zero = 0;
                }
            }
        }
    }
    out->allMasksNonZero = all_masks_non_zero;

    /* Phase 3: all masks fit in uint16_t (0..0xFFFF). */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0055[i] > 0xFFFF) {
            all_masks_in_uint16_range = 0;
        }
    }
    out->allMasksInUint16Range = all_masks_in_uint16_range;

    /* Phase 4: full table matches declared order. */
    {
        static const unsigned int kExpected[kTableSize] = {
            0x0003, 0xC000,  0x0780, 0x0000,  0x000F, 0x0000,
            0x1E00, 0x0000,  0x003C, 0x0000,  0x7800, 0x0000,
            0x00F0, 0x0000,  0x0001, 0xE000,  0x03C0, 0x0000,
            0x0007, 0x8000,  0x0F00, 0x0000,  0x001E, 0x0000
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0055[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 5: lookup function correctness. */
    {
        int c, g, m;
        for (c = 0; c < kChampionCount; ++c) {
            for (g = 0; g < kGraphCount; ++g) {
                for (m = 0; m < kMaskPair; ++m) {
                    int flat = (c * kGraphCount + g) * kMaskPair + m;
                    if (dm1_v1_bar_graph_masks_get_pc34(c, g, m) != (int)s_g0055[flat]) {
                        lookup_function_correct = 0;
                    }
                }
            }
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 6: out-of-range lookup returns -1. */
    if (dm1_v1_bar_graph_masks_get_pc34(-1, 0, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_masks_get_pc34(0, -1, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_masks_get_pc34(0, 0, -1) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_masks_get_pc34(kChampionCount, 0, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_masks_get_pc34(0, kGraphCount, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_masks_get_pc34(0, 0, kMaskPair) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_bar_graph_masks_get_pc34(999, 999, 999) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->champion0HealthMaskPair0003C000 &&
        out->champion1HealthMaskPair1E000000 &&
        out->champion2HealthMaskPair00F00000 &&
        out->champion3HealthMaskPair00078000 &&
        out->allMasksNonZero &&
        out->allMasksInUint16Range &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 9;
    return out->accepted;
}