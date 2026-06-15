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
 *   The original ReDMCSB source uses per-champion `.cell` ordinals
 *   (CHAMPION.C:G0301_ps_Champion[i].cell) that are independent of
 *   the slot position.  A right turn rotates every present
 *   champion's .cell by +1 (mod nCells).  Empty cells (no present
 *   champion at that cell) stay empty; the slot array itself is
 *   not touched.
 *
 *   Firestaff's compat layer doesn't store per-champion `.cell`
 *   ordinals separately; it uses slot-position as a proxy.  This
 *   is a structural simplification, NOT a fix of the original
 *   invariant.  The helper here implements an approximation that
 *   works for contiguous party layouts (slot 0..3 each holding
 *   one champion) by SHIFTING the present champions in the slot
 *   array by `delta` positions in present-list order.  Empty
 *   slots stay at their array positions and do NOT get
 *   overwritten — the rotation wraps around the present-list.
 *
 *   Limitation: this approximation collapses a non-contiguous
 *   party layout (e.g. a gap at slot 1 with champions at 0, 2,
 *   3) into a different layout after rotation.  In the original
 *   ReDMCSB the empty cell stays at the same ordinal; in our
 *   approximation the empty slot may end up at a different slot
 *   position.  This is acceptable for V1 because the inventory
 *   panel sorts by cell ordinal, and we preserve present-list
 *   ordering.  A full fix would require a `cell` field in
 *   ChampionState_Compat (structural change), tracked separately
 *   in TODO.md.
 */
#include "memory_mov05_f0284_cell_rotation_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <string.h>
#include "memory_mov05_f0284_cell_rotation_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <string.h>

/* F0284 cell + direction rotation, lifted out of the tick
 * orchestrator's static helper so unit tests can drive it
 * directly.  In production the orchestrator path is the
 * caller; in probe mode the regression test drives this
 * implementation via the F0284_CHAMPION_SetPartyDirection_Compat
 * probe wrapper.
 *
 * The implementation matches the in-orchestrator helper bit-for-bit
 * (same delta calc, same present-list mapping, same
 * activeChampionIndex tracking).  Any future refactor of the
 * orchestrator helper must be mirrored here (or, better, the
 * orchestrator helper should be replaced with a call to this
 * public version, see TODO). */
static void mov05_rotate_party(
    struct PartyState_Compat* party, int newDirection)
{
    int oldDirection;
    int delta;
    int i;
    int prevActive;
    if (!party) return;
    newDirection &= 3;
    oldDirection = party->direction & 3;
    if (newDirection == oldDirection) {
        party->direction = newDirection;
        return;
    }
    delta = newDirection - oldDirection;
    if (delta < 0) delta += 4;
    prevActive = party->activeChampionIndex;

    /* Step 1: per-present-cell Direction rotation (mod 4). */
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        if (party->champions[i].present) {
            party->champions[i].direction =
                (unsigned char)((party->champions[i].direction + delta) & 3);
        }
    }

    /* Step 2: cell-rotation in present-list order. */
    if (delta > 0 && party->championCount > 0) {
        struct ChampionState_Compat rotated[CHAMPION_MAX_PARTY];
        int presentAt[CHAMPION_MAX_PARTY];
        int nPresent = 0;
        int idxInPresent = 0;
        int targetSlot;
        int k;
        for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
            presentAt[i] = party->champions[i].present;
            if (presentAt[i]) nPresent++;
        }
        memset(rotated, 0, sizeof(rotated));
        for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
            if (!presentAt[i]) continue;
            targetSlot = -1;
            {
                int wanted = (idxInPresent + delta) % nPresent;
                int seen = 0;
                for (k = 0; k < CHAMPION_MAX_PARTY; ++k) {
                    if (presentAt[k]) {
                        if (seen == wanted) { targetSlot = k; break; }
                        seen++;
                    }
                }
            }
            if (targetSlot >= 0) {
                rotated[targetSlot] = party->champions[i];
            }
            idxInPresent++;
        }
        for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
            if (!presentAt[i]) rotated[i] = party->champions[i];
        }
        memcpy(party->champions, rotated, sizeof(party->champions));
        /* Step 3: active leader index. */
        if (prevActive >= 0 && prevActive < CHAMPION_MAX_PARTY &&
            presentAt[prevActive]) {
            int prevRank = 0;
            int newRank;
            int newActive = -1;
            int seen = 0;
            for (i = 0; i < prevActive; ++i) {
                if (presentAt[i]) prevRank++;
            }
            newRank = (prevRank + delta) % nPresent;
            for (i = 0; i < CHAMPION_MAX_PARTY && newActive < 0; ++i) {
                if (presentAt[i]) {
                    if (seen == newRank) newActive = i;
                    seen++;
                }
            }
            if (newActive >= 0) {
                party->activeChampionIndex = newActive;
            }
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
