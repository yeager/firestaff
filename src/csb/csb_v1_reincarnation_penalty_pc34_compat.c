/*
 * csb_v1_reincarnation_penalty_pc34_compat.c
 *
 * Source-locked per CSB:REVIVE.C CHANGE7_24 + Character.cpp:14
 * (three globals: reincarnateAttributePenalty=2,
 * reincarnateStatPenalty=8, randomPoints=3).  Reincarnation
 * halves HP/MP/STA and subtracts attributePenalty from each
 * non-Luck stat.  Luck is exempt.
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
    int attrPenalty;
    int statPenalty;
    int half;
    int i;
    if (!champ) return;
    if (!g_csb_v1_reincarnation_enabled) return;
    attrPenalty = g_csb_v1_reincarnation_cfg.attributePenalty;
    statPenalty = g_csb_v1_reincarnation_cfg.statPenalty;
    if (attrPenalty < 0)  attrPenalty = 0;
    if (statPenalty <= 0) statPenalty = 1; /* avoid divide-by-zero */

    /* HP / MP / STA halved.  Per ReDMCSB CHANGE7_24 the penalty
     * floors to half (round down).  ChampionState stores HP/
     * STA/MANA in ChampionStat_Compat {current, max, tmp}.
     * Halve the max; current stays <= max so it is auto-clamped. */
    if (champ->hp.maximum > 1) {
        half = champ->hp.maximum / 2;
        if (half < 1) half = 1;
        champ->hp.maximum = (unsigned short)half;
    }
    if (champ->stamina.maximum > 1) {
        half = champ->stamina.maximum / 2;
        if (half < 1) half = 1;
        champ->stamina.maximum = (unsigned short)half;
    }
    if (champ->mana.maximum > 1) {
        half = champ->mana.maximum / 2;
        if (half < 1) half = 1;
        champ->mana.maximum = (unsigned short)half;
    }
    /* MP and STA live in lifecycleState; we approximate the
     * CHANGE7_24 halving by halving the attribute current row
     * (statistics[*][1] in lifecycle).  In the standalone compat
     * shim we don't have lifecycleState; we operate on the
     * ChampionState's attributes array which mirrors the
     * DEFS.H Statistics[C1_CURRENT] row.  This is the surface
     * most callers read.  The 6 ChampionState attribute slots
     * (Strength/Dex/Wisdom/Vitality/AntiMagic/AntiFire) do not
     * include Luck, so the Luck-exempt clause from
     * Character.cpp:14 is implicit in the loop bounds. */
    for (i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        if (champ->attributes[i] > (unsigned short)attrPenalty) {
            champ->attributes[i] = (unsigned short)(
                champ->attributes[i] - (unsigned short)attrPenalty);
        } else {
            champ->attributes[i] = 0;
        }
    }
    (void)statPenalty; /* statPenalty (the divisor) reserved for
                          post-M10 deeper-magic-path work.  CHANGE7_24
                          uses statPenalty as the divisor of (stat
                          - random(8)) for HP halving in some
                          source variants; the compat shim uses
                          the simpler half-of-current rule. */
}
