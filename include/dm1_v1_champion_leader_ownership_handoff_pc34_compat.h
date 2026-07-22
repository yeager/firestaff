#ifndef FIRESTAFF_DM1_V1_CHAMPION_LEADER_OWNERSHIP_HANDOFF_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_LEADER_OWNERSHIP_HANDOFF_PC34_COMPAT_H

#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Source-owned input for the G0299/G0423 champion-panel selection state.
 * Ordinals follow the original 0 = none, 1..4 = party slot convention. */
typedef struct Dm1V1ChampionLeaderOwnershipInputPc34 {
    int candidateChampionOrdinal;
    int inventoryChampionOrdinal;
    int inventoryPanelActive;
} Dm1V1ChampionLeaderOwnershipInputPc34;

/* Renderer-neutral ownership contract that precedes F0292/F0293 redraw.
 * A zero redrawOwnerMask is a deliberate F0296 candidate early return. */
typedef struct Dm1V1ChampionLeaderOwnershipReceiptPc34 {
    int valid;
    int partyChampionCount;
    int leaderChampionIndex;
    int leaderChampionOrdinal;
    int inventoryChampionIndex;
    int inventoryChampionOrdinal;
    unsigned int topRowChampionMask;
    unsigned int redrawOwnerMask;
    int candidateBlocksF0296;
    int inventoryOwnerSkipsPrimaryF0296;
} Dm1V1ChampionLeaderOwnershipReceiptPc34;

/* Validates the compact PartyState_Compat source representation and derives
 * F0296 ownership before any top-row or inventory redraw is considered.
 * It intentionally does not choose an alternate leader or material fallback. */
int dm1_v1_champion_leader_ownership_handoff_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipInputPc34 *input,
    Dm1V1ChampionLeaderOwnershipReceiptPc34 *outReceipt);

const char *dm1_v1_champion_leader_ownership_handoff_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
