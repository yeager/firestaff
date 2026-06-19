#include "firestaff/dm1/v1/charge_count_to_torch_type_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:35  - declaration of G0029_auc_Graphic562_ChargeCountToTorchType[16]
 * - DATA.C:263 - PC 3.4 init { 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 }
 * - DATA.C:926 - post-1.3 Atari init (same values)
 * - OBJECT.C:178 - F0486_OBJECT_DrawObjectIcon reads G0029 for lit torches
 * - DEFS.H:      - C004_ICON_WEAPON_TORCH_UNLIT (the base icon index)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index),
 * pass803 (ordered-cells). This gate is a non-mirror-candidate
 * contract for the G0029 torch-charge-count -> torch-icon-type
 * mapping.
 */

enum {
    kTableSize      = 16,
    kTorchTypeCount = 4,
    kMinTorchType   = 0,
    kMaxTorchType   = 3,
    kOutOfRange     = 0,

    /* Bucket boundaries: {0}, {1, 2, 3}, {4, 5, 6, 7}, {8..15}. */
    kBucket1Low     = 1,
    kBucket1High    = 3,
    kBucket2Low     = 4,
    kBucket2High    = 7,
    kBucket3Low     = 8,
    kBucket3High    = 15
};

static const int s_g0029[kTableSize] = {
    /* 0  */ 0,  /* type 0 */
    /* 1  */ 1,
    /* 2  */ 1,
    /* 3  */ 1,  /* type 1 covers charges 1..3 */
    /* 4  */ 2,
    /* 5  */ 2,
    /* 6  */ 2,
    /* 7  */ 2,  /* type 2 covers charges 4..7 */
    /* 8  */ 3,
    /* 9  */ 3,
    /* 10 */ 3,
    /* 11 */ 3,
    /* 12 */ 3,
    /* 13 */ 3,
    /* 14 */ 3,
    /* 15 */ 3   /* type 3 covers charges 8..15 */
};

const int *
dm1_v1_charge_count_to_torch_type_table_pc34(void)
{
    return s_g0029;
}

int
dm1_v1_charge_count_to_torch_type_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_charge_count_to_torch_type_pc34(int charge_count)
{
    if (charge_count < 0 || charge_count >= kTableSize) {
        return kOutOfRange;
    }
    return s_g0029[charge_count];
}

/* Bucket boundaries in the source init:
 *   - 0 charges      -> type 0
 *   - 1..3 charges   -> type 1
 *   - 4..7 charges   -> type 2
 *   - 8..15 charges  -> type 3
 */
int
dm1_v1_charge_count_to_torch_type_bucket_pc34(int charge_count)
{
    return dm1_v1_charge_count_to_torch_type_pc34(charge_count);
}

/* First charge count that maps to the given torch type.
 * Returns -1 for invalid torch_type. */
int
dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(int torch_type)
{
    int i;
    if (torch_type < kMinTorchType || torch_type > kMaxTorchType) {
        return -1;
    }
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0029[i] == torch_type) {
            return i;
        }
    }
    return -1;
}

/* Last charge count that maps to the given torch type.
 * Returns -1 for invalid torch_type or no match. */
int
dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(int torch_type)
{
    int i;
    int last = -1;
    if (torch_type < kMinTorchType || torch_type > kMaxTorchType) {
        return -1;
    }
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0029[i] == torch_type) {
            last = i;
        }
    }
    return last;
}

int
dm1_v1_charge_count_to_torch_type_run_pc34(
    DM1_V1_ChargeCountToTorchTypeResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int table_is_monotonic = 1;
    int table_has_4_distinct_values = 1;
    int first_entry_0 = 1;
    int last_entry_3 = 1;
    int all_within_range_0to3 = 1;
    int bucket_boundaries_correct = 1;
    int lookup_function_in_range = 1;
    int lookup_out_of_range_returns_zero = 1;
    int bucket_boundaries_0183_to_type_0123_correct = 1;
    int dispatch_function_correct = 1;
    static const int kExpected[kTableSize] = {
        0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
    };
    int seen_torch_types[kTorchTypeCount] = { 0, 0, 0, 0 };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0029[i];
        if (s_g0029[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: monotonic non-decreasing (each bucket boundary is a
     * non-decrease step, and within-bucket values are equal).
     */
    for (i = 1; i < kTableSize; ++i) {
        if (s_g0029[i] < s_g0029[i - 1]) {
            table_is_monotonic = 0;
        }
    }
    out->tableIsMonotonic = table_is_monotonic;

    /* Phase 3: 4 distinct values across the 16 entries. */
    for (i = 0; i < kTableSize; ++i) {
        int v = s_g0029[i];
        if (v >= 0 && v < kTorchTypeCount) {
            seen_torch_types[v] = 1;
        }
    }
    for (i = 0; i < kTorchTypeCount; ++i) {
        if (!seen_torch_types[i]) {
            table_has_4_distinct_values = 0;
        }
    }
    out->tableHas4DistinctValues = table_has_4_distinct_values;

    /* Phase 4: first entry is 0 (an empty torch with 0 charges). */
    if (s_g0029[0] != 0) {
        first_entry_0 = 0;
    }
    out->firstEntry0 = first_entry_0;

    /* Phase 5: last entry is 3 (a full torch with 15 charges). */
    if (s_g0029[kTableSize - 1] != 3) {
        last_entry_3 = 0;
    }
    out->lastEntry3 = last_entry_3;

    /* Phase 6: all values in [0, 3]. */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0029[i] < 0 || s_g0029[i] > 3) {
            all_within_range_0to3 = 0;
        }
    }
    out->allWithinRange0to3 = all_within_range_0to3;

    /* Phase 7: bucket boundaries per the source design. */
    if (s_g0029[kBucket1Low] != 1) bucket_boundaries_correct = 0;
    if (s_g0029[kBucket1High] != 1) bucket_boundaries_correct = 0;
    if (s_g0029[kBucket2Low] != 2) bucket_boundaries_correct = 0;
    if (s_g0029[kBucket2High] != 2) bucket_boundaries_correct = 0;
    if (s_g0029[kBucket3Low] != 3) bucket_boundaries_correct = 0;
    if (s_g0029[kBucket3High] != 3) bucket_boundaries_correct = 0;
    out->bucketBoundariesCorrect = bucket_boundaries_correct;

    /* Phase 8: lookup function returns expected value for each index. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_charge_count_to_torch_type_pc34(i) != kExpected[i]) {
            lookup_function_in_range = 0;
        }
    }
    out->lookupFunctionInRange = lookup_function_in_range;

    /* Phase 9: out-of-range lookup returns 0. */
    if (dm1_v1_charge_count_to_torch_type_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_pc34(16) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    /* Phase 10: bucket mapping (the 0->0, 1..3->1, 4..7->2,
     * 8..15->3 design).
     */
    {
        int ok = 1;
        for (i = 0; i <= 0; ++i) {
            if (dm1_v1_charge_count_to_torch_type_pc34(i) != 0) ok = 0;
        }
        for (i = 1; i <= 3; ++i) {
            if (dm1_v1_charge_count_to_torch_type_pc34(i) != 1) ok = 0;
        }
        for (i = 4; i <= 7; ++i) {
            if (dm1_v1_charge_count_to_torch_type_pc34(i) != 2) ok = 0;
        }
        for (i = 8; i <= 15; ++i) {
            if (dm1_v1_charge_count_to_torch_type_pc34(i) != 3) ok = 0;
        }
        bucket_boundaries_0183_to_type_0123_correct = ok ? 1 : 0;
    }
    out->bucketBoundaries0183ToType0123Correct =
        bucket_boundaries_0183_to_type_0123_correct;

    /* Phase 11: dispatch function correctness (first/last for type). */
    if (dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(0) != 0) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(0) != 0) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(1) != 1) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(1) != 3) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(2) != 4) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(2) != 7) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(3) != 8) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(3) != 15) {
        dispatch_function_correct = 0;
    }
    /* OOB torch_type returns -1. */
    if (dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(-1) != -1) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(4) != -1) {
        dispatch_function_correct = 0;
    }
    if (dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(99) != -1) {
        dispatch_function_correct = 0;
    }
    out->dispatchFunctionCorrect = dispatch_function_correct;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->tableIsMonotonic &&
        out->tableHas4DistinctValues &&
        out->firstEntry0 &&
        out->lastEntry3 &&
        out->allWithinRange0to3 &&
        out->bucketBoundariesCorrect &&
        out->lookupFunctionInRange &&
        out->lookupOutOfRangeReturnsZero &&
        out->bucketBoundaries0183ToType0123Correct &&
        out->dispatchFunctionCorrect;
    out->assertionCount = 12;
    return out->accepted;
}