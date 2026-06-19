#include "firestaff/dm1/v1/icon_graphic_first_icon_index_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:32 - declaration of G0026_ai_Graphic562_IconGraphicFirstIconIndex[7]
 * - DATA.C:253-260 - PC 3.4 init values
 * - DATA.C:911-918 - post-1.3 Atari init values (same 32-stride pattern)
 * - OBJECT.C:312-319 - F0489_MEMORY_GetNativeBitmapOrGraphicIcon_Loop
 *                       walks G0026 to find IconGraphicIndex, then
 *                       subtracts G0026[IconGraphicIndex] from IconIndex
 * - OBJECT.C:455-467 - same walk for the slot-box icon path
 * - OBJECT.C:521 - G0030_as_Graphic562_SlotBoxes[P0049_ui_SlotBoxIndex]
 *                   .IconIndex -> P0048 (used as IconIndex in 455-467)
 * - DEFS.H - C042_GRAPHIC_OBJECT_ICONS_000_TO_031 constant (the base
 *            graphic id added to IconGraphicIndex)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-796 (champion-panel/leader/mirror). This gate is a
 * non-mirror-candidate contract for the OBJECT.C icon resolution path.
 */

enum {
    kIconBlockSize      = 32,
    kIconGraphCount     = 7,
    kFirstGraphOffset   = 0,    /* C042_GRAPHIC_OBJECT_ICONS_000_TO_031 */
    kMaxIconIndex       = 223,  /* (kIconGraphCount * kIconBlockSize) - 1 */
    kIndexOutOfRange    = -1,
    kFirstBlockStart    = 0,
    kLastBlockStart     = 192   /* (kIconGraphCount - 1) * kIconBlockSize */
};

static const int s_g0026[kIconGraphCount] = {
    /* 0 */   0,  /* first icon in graphic #42 */
    /* 1 */  32,  /* first icon in graphic #43 */
    /* 2 */  64,  /* first icon in graphic #44 */
    /* 3 */  96,  /* first icon in graphic #45 */
    /* 4 */ 128,  /* first icon in graphic #46 */
    /* 5 */ 160,  /* first icon in graphic #47 */
    /* 6 */ 192   /* first icon in graphic #48 */
};

const int *
dm1_v1_icon_graphic_first_icon_index_table_pc34(void)
{
    return s_g0026;
}

int
dm1_v1_icon_graphic_first_icon_index_size_pc34(void)
{
    return kIconGraphCount;
}

int
dm1_v1_icon_graphic_first_icon_index_pc34(int index)
{
    if (index < 0 || index >= kIconGraphCount) {
        return kIndexOutOfRange;
    }
    return s_g0026[index];
}

int
dm1_v1_icon_graphic_first_icon_index_block_size_pc34(void)
{
    return kIconBlockSize;
}

int
dm1_v1_icon_graphic_first_icon_index_first_graph_pc34(void)
{
    return kFirstGraphOffset;
}

int
dm1_v1_icon_graphic_first_icon_index_graph_count_pc34(void)
{
    return kIconGraphCount;
}

/* OBJECT.C:312-319 + OBJECT.C:455-467 — resolve IconIndex to
 * (graph_index, within_block_index) using the G0026 stride.
 *
 * The original loop is:
 *     for (c = 0; c < 7; ++c)
 *         if (G0026[c] > IconIndex) break;
 *     IconGraphicIndex = --c;
 *     IconIndex -= G0026[IconGraphicIndex];
 *
 * For IconIndex in [0, 31]  -> c hits 1, returns graph 0, offset = IconIndex
 * For IconIndex in [32, 63] -> c hits 2, returns graph 1, offset = IconIndex - 32
 * ...
 * For IconIndex = 223       -> c hits 7, exits loop, IconGraphicIndex = 6 (correct)
 * For IconIndex < 0 or IconIndex > 223 -> returns -1/-1.
 */
int
dm1_v1_icon_graphic_first_icon_index_resolve_pc34(
    int icon_index,
    int *out_graph_index,
    int *out_within_block_index)
{
    int c;
    int graph_index;
    int within_block_index;

    if (out_graph_index) {
        *out_graph_index = kIndexOutOfRange;
    }
    if (out_within_block_index) {
        *out_within_block_index = kIndexOutOfRange;
    }
    if (icon_index < 0 || icon_index > kMaxIconIndex) {
        return 0;
    }

    /* Replicate the C `for` loop exactly: stops at the first
     * G0026[c] > icon_index, then --c (post-decrement). For the
     * last valid index (223), the loop runs all 7 iterations and
     * exits with c == 7; --c == 6 (the last block).
     */
    c = 0;
    while (c < kIconGraphCount) {
        if (s_g0026[c] > icon_index) {
            break;
        }
        ++c;
    }
    graph_index = c - 1;
    within_block_index = icon_index - s_g0026[graph_index];

    if (out_graph_index) {
        *out_graph_index = graph_index;
    }
    if (out_within_block_index) {
        *out_within_block_index = within_block_index;
    }
    return 1;
}

int
dm1_v1_icon_graphic_first_icon_index_run_pc34(
    DM1_V1_IconGraphicFirstIconIndexResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int first_block_start_zero = 1;
    int last_block_start_192 = 1;
    int monotonic_increasing_32 = 1;
    int all_within_icon_range = 1;
    int lookup_graph42_index0 = 1;
    int lookup_graph43_index32 = 1;
    int lookup_graph44_index64 = 1;
    int lookup_graph45_index96 = 1;
    int lookup_graph46_index128 = 1;
    int lookup_graph47_index160 = 1;
    int lookup_graph48_index192 = 1;
    int out_of_range_returns_minus_one = 1;
    int prev_start;
    int last_start;
    int g;
    int w;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-element cross-check. */
    for (i = 0; i < kIconGraphCount; ++i) {
        int expected = i * kIconBlockSize;
        out->tableEntries[i] = s_g0026[i];
        if (s_g0026[i] != expected) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kIconGraphCount;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: first-block start is 0 (C042_GRAPHIC_OBJECT_ICONS_000_TO_031). */
    if (s_g0026[0] != kFirstBlockStart) {
        first_block_start_zero = 0;
    }
    out->firstBlockStartZero = first_block_start_zero;

    /* Phase 3: last-block start is 192 (graphic #48 base). */
    last_start = s_g0026[kIconGraphCount - 1];
    if (last_start != kLastBlockStart) {
        last_block_start_192 = 0;
    }
    out->lastBlockStart192 = last_block_start_192;

    /* Phase 4: monotonic increasing by kIconBlockSize (32). */
    prev_start = s_g0026[0];
    for (i = 1; i < kIconGraphCount; ++i) {
        if (s_g0026[i] != prev_start + kIconBlockSize) {
            monotonic_increasing_32 = 0;
        }
        prev_start = s_g0026[i];
    }
    out->monotonicIncreasing32 = monotonic_increasing_32;

    /* Phase 5: all starts are within icon-index range [0, kMaxIconIndex]. */
    for (i = 0; i < kIconGraphCount; ++i) {
        if (s_g0026[i] < 0 || s_g0026[i] > kMaxIconIndex) {
            all_within_icon_range = 0;
        }
    }
    out->allWithinIconRange = all_within_icon_range;

    /* Phase 6: lookup function returns the exact expected value for
     * each of the seven indices.
     */
    if (dm1_v1_icon_graphic_first_icon_index_pc34(0) != 0)   lookup_graph42_index0   = 0;
    if (dm1_v1_icon_graphic_first_icon_index_pc34(1) != 32)  lookup_graph43_index32  = 0;
    if (dm1_v1_icon_graphic_first_icon_index_pc34(2) != 64)  lookup_graph44_index64  = 0;
    if (dm1_v1_icon_graphic_first_icon_index_pc34(3) != 96)  lookup_graph45_index96  = 0;
    if (dm1_v1_icon_graphic_first_icon_index_pc34(4) != 128) lookup_graph46_index128 = 0;
    if (dm1_v1_icon_graphic_first_icon_index_pc34(5) != 160) lookup_graph47_index160 = 0;
    if (dm1_v1_icon_graphic_first_icon_index_pc34(6) != 192) lookup_graph48_index192 = 0;
    out->lookupGraph42_Index0   = lookup_graph42_index0;
    out->lookupGraph43_Index32  = lookup_graph43_index32;
    out->lookupGraph44_Index64  = lookup_graph44_index64;
    out->lookupGraph45_Index96  = lookup_graph45_index96;
    out->lookupGraph46_Index128 = lookup_graph46_index128;
    out->lookupGraph47_Index160 = lookup_graph47_index160;
    out->lookupGraph48_Index192 = lookup_graph48_index192;

    /* Phase 7: out-of-range lookup returns -1 (the sentinel we use to
     * keep this gate's API pure-C and self-contained; ReDMCSB callers
     * gate on index < kIconGraphCount before reading G0026).
     */
    if (dm1_v1_icon_graphic_first_icon_index_pc34(-1) != kIndexOutOfRange) {
        out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_icon_graphic_first_icon_index_pc34(7) != kIndexOutOfRange) {
        out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_icon_graphic_first_icon_index_pc34(223) != kIndexOutOfRange) {
        out_of_range_returns_minus_one = 0;
    }
    out->outOfRangeReturnsMinusOne = out_of_range_returns_minus_one;

    /* Phase 8: full resolve() sweep — for one canonical index in each
     * block, assert (graph, within_block) matches the OBJECT.C walk.
     * This catches stride drift, off-by-one, and walk-termination bugs.
     */
    {
        static const int kCanonical[kIconGraphCount] = {
            0, 32, 64, 96, 128, 160, 223
        };
        int ok = 1;
        for (i = 0; i < kIconGraphCount; ++i) {
            int rc = dm1_v1_icon_graphic_first_icon_index_resolve_pc34(
                kCanonical[i], &g, &w);
            if (!rc || g != i || w != (kCanonical[i] - s_g0026[i])) {
                ok = 0;
            }
        }
        out->declarationMatchesInit = ok;
    }

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstBlockStartZero &&
        out->lastBlockStart192 &&
        out->monotonicIncreasing32 &&
        out->allWithinIconRange &&
        out->lookupGraph42_Index0 &&
        out->lookupGraph43_Index32 &&
        out->lookupGraph44_Index64 &&
        out->lookupGraph45_Index96 &&
        out->lookupGraph46_Index128 &&
        out->lookupGraph47_Index160 &&
        out->lookupGraph48_Index192 &&
        out->outOfRangeReturnsMinusOne &&
        out->declarationMatchesInit;
    out->assertionCount = 14;
    return out->accepted;
}
