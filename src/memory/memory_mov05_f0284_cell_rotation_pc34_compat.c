/*
 * memory_mov05_f0284_cell_rotation_pc34_compat.c
 *
 * MOV-05 (DM1 V1 functional-divergence-report.md): the static
 * set party_direction_redmcsb_compat inside
 * memory_tick_orchestrator_pc34_compat.c rotates only Direction,
 * not Cell.  This file provides a public F0284 wrapper that
 * callers and tests can drive directly.
 *
 * Source-locked to ReDMCSB CHAMPION.C:117-130,
 * F0284_CHAMPION_SetPartyDirection.
 *
 * Cell-rotation strategy (MOV-05 fix):
 *
 *   ReDMCSB stores each party member's Cell separately from its
 *   champion-array slot. F0284 rotates every present champion's
 *   Cell and Direction by the party-direction delta; it does not
 *   shuffle champion structs in memory or change the active leader
 *   index. Firestaff now stores ChampionState_Compat.cell, so this
 *   wrapper can keep the same invariant as the tick orchestrator.
 */
#include "memory_mov05_f0284_cell_rotation_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

/* F0284 cell + direction rotation, lifted out of the tick
 * orchestrator's static helper so unit tests can drive it
 * directly.  In production the orchestrator path is the
 * caller; in probe mode the regression test drives this
 * implementation via the F0284_CHAMPION_SetPartyDirection_Compat
 * probe wrapper.
 *
 * The implementation matches the in-orchestrator helper bit-for-bit
 * (same delta calc, same per-present champion Cell/Direction update).
 * Any future refactor of the
 * orchestrator helper must be mirrored here (or, better, the
 * orchestrator helper should be replaced with a call to this
 * public version, see TODO). */
static void mov05_rotate_party(
    struct PartyState_Compat* party, int newDirection)
{
    int oldDirection;
    int delta;
    int i;
    if (!party) return;
    newDirection &= 3;
    oldDirection = party->direction & 3;
    if (newDirection == oldDirection) {
        party->direction = newDirection;
        return;
    }
    delta = newDirection - oldDirection;
    if (delta < 0) delta += 4;

    /* ReDMCSB CHAMPION.C F0284 lines 117-130: rotate each present
     * champion's Cell and Direction by delta; do not reorder M516_CHAMPIONS. */
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (party->champions[i].present) {
            party->champions[i].cell =
                (unsigned char)((party->champions[i].cell + delta) & 3);
            party->champions[i].direction =
                (unsigned char)((party->champions[i].direction + delta) & 3);
        }
    }

    party->direction = newDirection;
}

int F0284_CHAMPION_SetPartyDirection_Compat(
    struct PartyState_Compat* party, int newDirection)
{
    if (!party) return 0;
    int old = party->direction & 3;
    mov05_rotate_party(party, newDirection);
    return ((party->direction & 3) != old) ? 1 : 0;
}
