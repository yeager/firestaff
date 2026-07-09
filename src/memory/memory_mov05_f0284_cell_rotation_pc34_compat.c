/*
 * memory_mov05_f0284_cell_rotation_pc34_compat.c
 *
 * MOV-05 (DM1 V1 functional-divergence-report.md): F0284 rotates
 * both champion Direction and Cell. This file provides the shared
 * public F0284 wrapper used by runtime callers and tests.
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

/* F0284 cell + direction rotation, shared by M10 tick orchestration,
 * M11 turn presentation, and the MOV-05 regression probe. */
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
