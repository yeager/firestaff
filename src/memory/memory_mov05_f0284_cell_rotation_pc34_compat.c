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
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

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

int F0286_CHAMPION_GetTargetChampionIndex_Compat(
    const struct PartyState_Compat* party,
    int attackerMapX,
    int attackerMapY,
    unsigned int attackerCell,
    struct RngState_Compat* rng)
{
    int orderedCells[4];
    int distanceX;
    int distanceY;
    int orderIndex;
    int championIndex;

    if (!party || !rng || party->championCount <= 0) return -1;
    distanceX = attackerMapX - party->mapX;
    distanceY = attackerMapY - party->mapY;
    if (distanceX < 0) distanceX = -distanceX;
    if (distanceY < 0) distanceY = -distanceY;
    if (distanceX + distanceY > 1) return -1;

    /* CHAMPION.C F0286 calls F0229 with the party square as target and
     * the attacking square as source, then calls F0285 in that order. */
    if (!F0229_DM1_GROUP_SetOrderedCellsToAttack_Compat(
            orderedCells, party->mapX, party->mapY, attackerMapX,
            attackerMapY, attackerCell, rng)) {
        return -1;
    }
    for (orderIndex = 0; orderIndex < 4; ++orderIndex) {
        int cell = orderedCells[orderIndex];
        for (championIndex = 0;
             championIndex < party->championCount &&
             championIndex < CHAMPION_MAX_PARTY;
             ++championIndex) {
            if (party->champions[championIndex].present &&
                party->champions[championIndex].hp.current > 0 &&
                (party->champions[championIndex].cell & 3) == cell) {
                return championIndex;
            }
        }
    }
    return -1;
}
