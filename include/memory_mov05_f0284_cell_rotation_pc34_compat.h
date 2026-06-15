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
 *  - per-present-cell Direction += delta (mod 4)
 *  - per-present-cell ordinal position in the present-list
 *    rotates by delta (empty slots stay empty; contiguous
 *    present-list ordering preserved)
 *  - activeChampionIndex follows the previously-active champion
 *    through the rotation
 *  - idempotent on no-op direction changes
 */

#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

int F0284_CHAMPION_SetPartyDirection_Compat(
    struct PartyState_Compat* party, int newDirection);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MEMORY_MOV05_F0284_CELL_ROTATION_PC34_COMPAT_H */
