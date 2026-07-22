#include "dm1_v1_champion_leader_ownership_handoff_pc34_compat.h"

#include <string.h>

static int ordinal_is_valid(int ordinal)
{
    return ordinal >= 0 && ordinal <= CHAMPION_MAX_PARTY;
}

static unsigned int party_mask(int championCount)
{
    return championCount == 0 ? 0U : ((1U << championCount) - 1U);
}

static int party_is_compact(const struct PartyState_Compat *party)
{
    int i;
    if (!party || party->championCount < 0 ||
        party->championCount > CHAMPION_MAX_PARTY) return 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        int expectedPresent = i < party->championCount;
        if (!!party->champions[i].present != expectedPresent) return 0;
    }
    return 1;
}

const char *dm1_v1_champion_leader_ownership_handoff_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H G0299 candidate champion, G0423 inventory "
           "champion and G0305 party state; CHAMDRAW.C F0296:1208-1210 "
           "candidate early return, F0296:1217-1219 primary slot walk, "
           "F0292:771-815 and F0293:1117-1139 champion redraw ownership; "
           "M000_INDEX_TO_ORDINAL preserves 0 = none, 1..4 = party slot.";
}

int dm1_v1_champion_leader_ownership_handoff_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipInputPc34 *input,
    Dm1V1ChampionLeaderOwnershipReceiptPc34 *outReceipt)
{
    int inventoryIndex;
    unsigned int topRowMask;

    if (!party || !input || !outReceipt || !party_is_compact(party) ||
        !ordinal_is_valid(input->candidateChampionOrdinal) ||
        !ordinal_is_valid(input->inventoryChampionOrdinal)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->inventoryChampionIndex = -1;
    outReceipt->leaderChampionIndex = -1;

    if (party->championCount == 0) {
        if (party->activeChampionIndex != -1 ||
            input->inventoryChampionOrdinal != 0 || input->inventoryPanelActive) return 0;
    } else if (party->activeChampionIndex < 0 ||
               party->activeChampionIndex >= party->championCount) {
        return 0;
    }

    topRowMask = party_mask(party->championCount);
    outReceipt->partyChampionCount = party->championCount;
    outReceipt->topRowChampionMask = topRowMask;
    outReceipt->leaderChampionIndex = party->activeChampionIndex;
    outReceipt->leaderChampionOrdinal = party->championCount == 0 ? 0 :
        party->activeChampionIndex + 1;

    inventoryIndex = input->inventoryChampionOrdinal - 1;
    if (input->inventoryChampionOrdinal != 0) {
        if (!input->inventoryPanelActive || inventoryIndex < 0 ||
            inventoryIndex >= party->championCount) return 0;
        outReceipt->inventoryChampionIndex = inventoryIndex;
        outReceipt->inventoryChampionOrdinal = input->inventoryChampionOrdinal;
        outReceipt->inventoryOwnerSkipsPrimaryF0296 = 1;
    } else if (input->inventoryPanelActive) {
        return 0;
    }

    if (input->candidateChampionOrdinal != 0 &&
        input->inventoryChampionOrdinal == 0) {
        outReceipt->candidateBlocksF0296 = 1;
        outReceipt->valid = 1;
        return 1;
    }

    outReceipt->redrawOwnerMask = topRowMask;
    if (inventoryIndex >= 0)
        outReceipt->redrawOwnerMask &= ~(1U << inventoryIndex);
    outReceipt->valid = 1;
    return 1;
}
