/*
 * csb_v1_reincarnation_penalty_pc34_compat.h
 *
 * CSB V1 Reincarnation Penalty (Champions GAP 2, CHANGE7_24).
 * Source-locked per CSB:REVIVE.C CHANGE7_24 + Character.cpp:14:
 *   - HP / MP / STA halved (right-shift by 1, source-equivalent
 *     to divide by 2 with round-down).  ReDMCSB REVIVE.C lines
 *     815-822 (MEDIA265 CHANGE7_24_IMPROVEMENT).
 *   - For each non-Luck stat (Strength, Dexterity, Wisdom,
 *     Vitality, AntiMagic, AntiFire) the current value is
 *     reduced by `cur / statPenalty` (default 8 = 1/8th per
 *     Character.cpp:14, source-equivalent to `>>3` in REVIVE.C
 *     lines 811-812) and BOTH the current and maximum rows are
 *     set to the same result, clamped at 0 (the ChampionState
 *     surface has no per-stat minimum row; the natural floor is
 *     0 because attributes never go negative in this surface).
 *   - Luck is exempt (no penalty).  ReDMCSB DEFS.H:745-750
 *     defines C1_STATISTIC_STRENGTH..C6_STATISTIC_ANTIFIRE
 *     which excludes C0_STATISTIC_LUCK; ChampionState_Compat
 *     attributes[CHAMPION_ATTR_COUNT] has 6 slots matching the
 *     same range, so Luck exemption is implicit in the loop
 *     bounds.
 *
 * ReDMCSB: REVIVE.C line ~809-822 (CHANGE7_24).
 * ReDMCSB: Character.cpp:14 (reincarnateAttributePenalty=2,
 *   reincarnateStatPenalty=8, randomPoints=3).
 * ReDMCSB: DEFS.H:338-339 (C160/C161 resurrect/reincarnate
 *   panel commands) — this shim is the ChampionState_Compat
 *   surface variant; the modern CSB_V1_Champion surface lives
 *   in `csb_v1_champion_reincarnate()` in
 *   src/csb/csb_v1_character_pc34_compat.c.
 *
 * v1 (2026-06-14): initial simplified contract.
 * v2 (2026-06-27): source-faithful port from ReDMCSB REVIVE.C
 *   CHANGE7_24 — per-stat formula now `cur - (cur/statPenalty)`
 *   with both current and max set to the same result.
 */
#ifndef REDMCSB_CSB_V1_REINCARNATION_PENALTY_PC34_COMPAT_H
#define REDMCSB_CSB_V1_REINCARNATION_PENALTY_PC34_COMPAT_H

#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration for CSB's CHANGE7_24 reincarnation penalty.
 *   attributePenalty = reserved for future per-attribute tier
 *       loss; not consumed by the current source-locked
 *       REVIVE.C 1/statPenalty formula (Character.cpp:14 default
 *       2, max 16).
 *   statPenalty = the divisor for the per-stat 1/N reduction
 *       on the six non-Luck stats (default 8 = 1/8th per
 *       Character.cpp:14; source-equivalent to `>>3` in
 *       ReDMCSB REVIVE.C lines 811-812).
 *   randomPoints = bonus points awarded on reincarnation
 *       (default 3, max 25 — Character.cpp:14).  Reserved for
 *       the post-M10 random-boost pass; this shim does not
 *       apply random +1 boosts (they live in the modern
 *       CSB_V1_Champion.reincarnate path).
 * The defaults match the ReDMCSB Character.cpp:14 globals. */
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

/* Apply the source-locked CHANGE7_24 penalty to a freshly
 * materialized candidate.  HP/MP/STA current and maximum are
 * halved with `>> 1`, and for each of the six non-Luck stats
 * both current and maximum are set to
 * `cur - (cur / statPenalty)` with a 0 floor (no per-stat
 * minimum tracking on ChampionState_Compat; ReDMCSB's
 * C2_MINIMUM row clamps to 30 in F0280; 0 is the natural
 * compat floor because ChampionState attributes never go
 * negative).  Re-applying the shim applies the source rule
 * again to the already-penalized values.  Source: ReDMCSB
 * REVIVE.C lines 809-822. */
void csb_v1_reincarnation_penalty_apply(
    struct ChampionState_Compat* champ);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_REINCARNATION_PENALTY_PC34_COMPAT_H */
