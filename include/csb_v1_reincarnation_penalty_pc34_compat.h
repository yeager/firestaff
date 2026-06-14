/*
 * csb_v1_reincarnation_penalty_pc34_compat.h
 *
 * CSB V1 Reincarnation Penalty (Champions GAP 2, CHANGE7_24).
 * Source-locked per CSB:REVIVE.C CHANGE7_24 + Character.cpp:14:
 *   - HP / MP / STA halved (rounding down)
 *   - other stats (Strength, Dexterity, Wisdom, Vitality,
 *     AntiMagic, AntiFire) each reduced by 1/8th (rounding
 *     up so a 16-stat champion becomes 14, not 13)
 *   - Luck is exempt (no penalty)
 *
 * v1 (2026-06-14): F0610_PARTY_AddChampionFromMirrorTextString
 * calls csb_v1_reincarnation_penalty_apply() on the freshly-
 * materialized candidate when the reincarnation flag is set.
 * The flag defaults to 0 (DM1 behaviour: stats preserved).
 */
#ifndef REDMCSB_CSB_V1_REINCARNATION_PENALTY_PC34_COMPAT_H
#define REDMCSB_CSB_V1_REINCARNATION_PENALTY_PC34_COMPAT_H

#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration for CSB's CHANGE7_24 reincarnation penalty.
 *   attributePenalty = how much is subtracted from each non-Luck
 *       stat (default 2, max 16 — Character.cpp:14)
 *   statPenalty = the divisor for the half-of-X penalty on
 *       HP/MP/STA (default 8, max 16 — Character.cpp:14)
 *   randomPoints = bonus points awarded on reincarnation
 *       (default 3, max 25 — Character.cpp:14)
 * The defaults match the ReDMCSB character.cpp globals. */
typedef struct CSB_V1_ReincarnationConfig_Compat {
    int attributePenalty;
    int statPenalty;
    int randomPoints;
} CSB_V1_ReincarnationConfig_Compat;

/* Reincarnation flag.  When set, F0610 applies the penalty on
 * the materialized candidate.  When clear, the candidate is
 * preserved as-is (resurrect / DM1 behaviour).  Default 0. */
int  csb_v1_reincarnation_mode_get(void);
void csb_v1_reincarnation_mode_set(int enabled);

/* Configure the three penalty globals.  Pass NULL to reset
 * to the Character.cpp:14 defaults. */
void csb_v1_reincarnation_config(const CSB_V1_ReincarnationConfig_Compat* cfg);

/* Apply the CHANGE7_24 penalty to a freshly materialized
 * candidate.  Halves HP/MP/STA (rounding down) and subtracts
 * attributePenalty from each non-Luck stat.  Idempotent on
 * already-penalized candidates (clamped to 0). */
void csb_v1_reincarnation_penalty_apply(
    struct ChampionState_Compat* champ);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_REINCARNATION_PENALTY_PC34_COMPAT_H */
