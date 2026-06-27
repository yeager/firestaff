/*
 * csb_v1_reincarnation_penalty_pc34_compat.c
 *
 * Source-locked per CSB:REVIVE.C CHANGE7_24 + Character.cpp:14.
 *
 * ReDMCSB: REVIVE.C line ~809-822 (MEDIA332 + MEDIA265
 * CHANGE7_24_IMPROVEMENT, "New reincarnation rules: Health,
 * Mana and Stamina values are halved.  The current and maximum
 * values of each statistic except Luck are decreased by
 * 1/8th of their value without going below their minimum
 * value").
 *
 * ReDMCSB: REVIVE.C lines 811-812 (per-stat 1/8th loop):
 *   one_eighth = Statistics[i][C1_CURRENT] >> 3;
 *   Statistics[i][C1_CURRENT] = Statistics[i][C0_MAXIMUM]
 *       = GetMaximumValue(Statistics[i][C2_MINIMUM],
 *                          Statistics[i][C1_CURRENT]
 *                          - one_eighth);
 *
 * ReDMCSB: REVIVE.C lines 815-822 (MEDIA265 halving):
 *   CurrentHealth   >>= 1; MaximumHealth   >>= 1;
 *   CurrentStamina  >>= 1; MaximumStamina  >>= 1;
 *   CurrentMana     >>= 1; MaximumMana     >>= 1;
 *
 * ReDMCSB: Character.cpp:14 (reincarnateAttributePenalty=2,
 *   reincarnateStatPenalty=8, randomPoints=3).  The default
 *   statPenalty divisor 8 corresponds to `>>3` in REVIVE.C.
 *   When the per-champion `statPenalty` differs from 8, we
 *   scale by `cur / statPenalty` to honor the Character.cpp
 *   scaling global.
 *
 * ReDMCSB: DEFS.H:745-750 (C1_STATISTIC_STRENGTH..C6_STATISTIC_ANTIFIRE
 *   loop excludes C0_STATISTIC_LUCK).  ChampionState_Compat
 *   stores the 6 non-Luck stats directly in `attributes[]`
 *   (CHAMPION_ATTR_COUNT = 6), so the Luck exemption is
 *   implicit in the loop bounds.
 *
 * v1 (2026-06-14): initial simplified contract.
 * v2 (2026-06-27): source-faithful port from ReDMCSB REVIVE.C
 *   CHANGE7_24.  Per-stat reduction now follows `cur -
 * (cur / statPenalty)` with both current and max set to the
 * same value, clamped at 0 (ChampionState_Compat has no
 * per-stat minimum row; ReDMCSB's C2_MINIMUM row clamps to 30
 * in F0280; 0 is the natural compat floor since ChampionState
 * attributes never go negative in the surface model).
 */
#include "csb_v1_reincarnation_penalty_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <string.h>

static int g_csb_v1_reincarnation_enabled = 0;
static CSB_V1_ReincarnationConfig_Compat g_csb_v1_reincarnation_cfg = {
    /* attributePenalty = 2, statPenalty = 8, randomPoints = 3
     * per Character.cpp:14 defaults. */
    2, 8, 3
};

int csb_v1_reincarnation_mode_get(void) {
    return g_csb_v1_reincarnation_enabled;
}

void csb_v1_reincarnation_mode_set(int enabled) {
    g_csb_v1_reincarnation_enabled = enabled ? 1 : 0;
}

void csb_v1_reincarnation_config(const CSB_V1_ReincarnationConfig_Compat* cfg) {
    if (cfg) {
        g_csb_v1_reincarnation_cfg = *cfg;
    } else {
        g_csb_v1_reincarnation_cfg.attributePenalty = 2;
        g_csb_v1_reincarnation_cfg.statPenalty       = 8;
        g_csb_v1_reincarnation_cfg.randomPoints      = 3;
    }
}

void csb_v1_reincarnation_penalty_apply(struct ChampionState_Compat* champ) {
    int statDivisor;
    int i;
    int cur;
    int reduction;
    int new_val;
    if (!champ) return;
    if (!g_csb_v1_reincarnation_enabled) return;

    /* ReDMCSB REVIVE.C lines 815-821: current and maximum
     * HP/Mana/Stamina are all halved with `>> 1`.  Clamp only
     * malformed compat inputs where current still exceeds max
     * after both source-locked rows have been shifted. */
    champ->hp.current = (unsigned short)(champ->hp.current >> 1);
    champ->hp.maximum = (unsigned short)(champ->hp.maximum >> 1);
    if (champ->hp.current > champ->hp.maximum) {
        champ->hp.current = champ->hp.maximum;
    }
    champ->stamina.current = (unsigned short)(champ->stamina.current >> 1);
    champ->stamina.maximum = (unsigned short)(champ->stamina.maximum >> 1);
    if (champ->stamina.current > champ->stamina.maximum) {
        champ->stamina.current = champ->stamina.maximum;
    }
    champ->mana.current = (unsigned short)(champ->mana.current >> 1);
    champ->mana.maximum = (unsigned short)(champ->mana.maximum >> 1);
    if (champ->mana.current > champ->mana.maximum) {
        champ->mana.current = champ->mana.maximum;
    }

    /* ReDMCSB Character.cpp:14 default statPenalty=8 corresponds
     * to the `>>3` shift in REVIVE.C.  We honor the global with
     * a divide so callers can override the divisor.  Avoid
     * divide-by-zero by clamping to 1 minimum. */
    statDivisor = g_csb_v1_reincarnation_cfg.statPenalty;
    if (statDivisor <= 0) statDivisor = 1;

    /* ReDMCSB REVIVE.C lines 809-815 (CHANGE7_24 per-stat loop):
     *   for (i = C1_STATISTIC_STRENGTH; i <= C6_STATISTIC_ANTIFIRE; i++) {
     *       one_eighth = Statistics[i][C1_CURRENT] >> 3;
     *       Statistics[i][C1_CURRENT] = Statistics[i][C0_MAXIMUM]
     *           = GetMaximumValue(
     *               Statistics[i][C2_MINIMUM],
     *               Statistics[i][C1_CURRENT] - one_eighth);
     *   }
     *
     * ChampionState_Compat.attributes[] has 6 slots (CHAMPION_ATTR_COUNT)
     * covering Strength/Dex/Wisdom/Vitality/AntiMagic/AntiFire, which is
     * exactly the REVIVE.C C1..C6 range.  There is no Luck slot in
     * ChampionState_Compat, so the Luck-exempt clause from
     * Character.cpp:14 is implicit in the loop bounds.
     *
     * ChampionState_Compat has no per-attribute minimum tracking
     * (attributeMinimums[] is not a field); we use 0 as the floor
     * since attribute values never go negative in this surface. */
    for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        cur = (int)champ->attributes[i];
        reduction = cur / statDivisor;
        new_val = cur - reduction;
        if (new_val < 0) new_val = 0;
        champ->attributes[i] = (unsigned short)new_val;
        /* Mirror current->max so a future READ of attributeMaximums[]
         * matches the new current value, matching REVIVE.C line 812
         * (Statistics[i][C1_CURRENT] = Statistics[i][C0_MAXIMUM]). */
        champ->attributeMaximums[i] = (unsigned short)new_val;
    }
}
