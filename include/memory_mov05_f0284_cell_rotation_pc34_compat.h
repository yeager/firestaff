#ifndef FIRESTAFF_MEMORY_MOV05_F0284_CELL_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_MEMORY_MOV05_F0284_CELL_ROTATION_PC34_COMPAT_H

/*
 * MOV-05 (DM1 V1 functional-divergence-report.md):
 *   "F0284_CHAMPION_SetPartyDirection rotates Direction but
 *    not Cell.  ...  cell rotation affects display ordering in
 *    the inventory panel."
 *
 * Source-locked to ReDMCSB CHAMPION.C:117-130,
 * F0284_CHAMPION_SetPartyDirection.
 *
 * The F0284 impl in memory_tick_orchestrator_pc34_compat.c is
 * static; this header exposes a public wrapper
 * F0284_CHAMPION_SetPartyDirection_Compat so the rotation can
 * be unit-tested directly without the full F0884 tick path.
 *
 * Invariants pinned by the regression test:
 *  - delta = (newDirection - oldDirection) mod 4
 *  - per-present champion Cell += delta (mod 4)
 *  - per-present champion Direction += delta (mod 4)
 *  - champion array slots and activeChampionIndex are unchanged
 *  - idempotent on no-op direction changes
 */

#include "memory_champion_state_pc34_compat.h"

struct RngState_Compat;

#ifdef __cplusplus
extern "C" {
#endif

int F0284_CHAMPION_SetPartyDirection_Compat(
    struct PartyState_Compat* party, int newDirection);

/* ReDMCSB CHAMPION.C F0286: find the first living party champion in the
 * F0229/G0023 cell order for an adjacent attacker. Party slots remain in
 * original M516 order; no host-selected target is substituted. */
int F0286_CHAMPION_GetTargetChampionIndex_Compat(
    const struct PartyState_Compat* party,
    int attackerMapX,
    int attackerMapY,
    unsigned int attackerCell,
    struct RngState_Compat* rng);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MEMORY_MOV05_F0284_CELL_ROTATION_PC34_COMPAT_H */
