#include "firestaff/dm1/v1/wound_defense_factor_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0050_auc_Graphic562_WoundDefenseFactor):
 * - DATA.C:88  - declaration of G0050_auc_Graphic562_WoundDefenseFactor[6]
 * - DATA.C:88/427/1103 - declaration + PC 3.4 init + Atari init
 * - DATA.C:427 - PC 3.4 init { 5, 5, 4, 6, 3, 1 }
 * - DATA.C:1103 - post-1.3 Atari init (same values)
 * - CHAMPION.C:1346 - F0312_CHAMPION_GetStrength * G0050[P0653_ui_WoundIndex]
 *                     ((STRENGTH + ARMOUR_DEFENSE) * WOUND_DEFENSE_FACTOR) >> 4/5
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-852 (Graphics.dat init-table gates batches 1-10). This
 * gate is a non-mirror-candidate contract for the G0050 wound-defense
 * factor table.
 */

enum {
    kTableSize  = 6,
    kOutOfRange = 0,
    kHeadIdx    = 0,
    kTorsoIdx   = 1,
    kLegsIdx    = 2,
    kArmsIdx    = 3,
    kFeetIdx    = 4,
    kHandIdx    = 5
};

static const unsigned char s_g0050[kTableSize] = {
    /* 0 */  5,   /* HEAD defense factor */
    /* 1 */  5,   /* TORSO defense factor */
    /* 2 */  4,   /* LEGS defense factor */
    /* 3 */  6,   /* ARMS defense factor */
    /* 4 */  3,   /* FEET defense factor */
    /* 5 */  1    /* HAND defense factor */
};

const unsigned char *
dm1_v1_wound_defense_factor_table_pc34(void)
{
    return s_g0050;
}

int
dm1_v1_wound_defense_factor_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_wound_defense_factor_get_pc34(int wound_index)
{
    if (wound_index < 0 || wound_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_g0050[wound_index];
}

int
dm1_v1_wound_defense_factor_run_pc34(
    DM1_V1_WoundDefenseFactorResultPc34 *out)
{
    int table_matches_declaration = 1;
    int head_factor_5 = 1;
    int torso_factor_5 = 1;
    int legs_factor_4 = 1;
    int arms_factor_6 = 1;
    int feet_factor_3 = 1;
    int hand_factor_1 = 1;
    int all_factors_positive = 1;
    int all_factors_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0050[i];
    }
    out->tableSize = kTableSize;

    /* Phase 1: per-index factor checks. */
    if (s_g0050[kHeadIdx]  != 5) head_factor_5  = 0;
    if (s_g0050[kTorsoIdx] != 5) torso_factor_5 = 0;
    if (s_g0050[kLegsIdx]  != 4) legs_factor_4  = 0;
    if (s_g0050[kArmsIdx]  != 6) arms_factor_6  = 0;
    if (s_g0050[kFeetIdx]  != 3) feet_factor_3  = 0;
    if (s_g0050[kHandIdx]  != 1) hand_factor_1  = 0;
    out->headFactor5 = head_factor_5;
    out->torsoFactor5 = torso_factor_5;
    out->legsFactor4 = legs_factor_4;
    out->armsFactor6 = arms_factor_6;
    out->feetFactor3 = feet_factor_3;
    out->handFactor1 = hand_factor_1;

    /* Phase 2: all factors > 0 (defense factor must be positive). */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0050[i] == 0) {
            all_factors_positive = 0;
        }
    }
    out->allFactorsPositive = all_factors_positive;

    /* Phase 3: all factors in byte range [0, 255]. */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0050[i] > 255) {
            all_factors_in_byte_range = 0;
        }
    }
    out->allFactorsInByteRange = all_factors_in_byte_range;

    /* Phase 4: table matches declared order. */
    {
        static const unsigned char kExpected[kTableSize] = {5, 5, 4, 6, 3, 1};
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0050[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 5: lookup function correctness for each index. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_wound_defense_factor_get_pc34(i) != (int)s_g0050[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 6: out-of-range lookup returns 0. */
    if (dm1_v1_wound_defense_factor_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_wound_defense_factor_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_wound_defense_factor_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->headFactor5 &&
        out->torsoFactor5 &&
        out->legsFactor4 &&
        out->armsFactor6 &&
        out->feetFactor3 &&
        out->handFactor1 &&
        out->allFactorsPositive &&
        out->allFactorsInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 12;
    return out->accepted;
}