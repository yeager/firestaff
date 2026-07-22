#include "dm1_v1_champion_leader_ownership_handoff_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void party_with_members(struct PartyState_Compat *party, int count, int leader)
{
    int i;
    memset(party, 0, sizeof(*party));
    party->championCount = count;
    party->activeChampionIndex = leader;
    for (i = 0; i < count; ++i) party->champions[i].present = 1;
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionLeaderOwnershipInputPc34 input;
    Dm1V1ChampionLeaderOwnershipReceiptPc34 receipt;
    int ok = 1;

    party_with_members(&party, 3, 1);
    memset(&input, 0, sizeof(input));
    ok &= check("normal leader owns compact top row",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &receipt) &&
        receipt.leaderChampionIndex == 1 && receipt.leaderChampionOrdinal == 2 &&
        receipt.topRowChampionMask == 0x7U && receipt.redrawOwnerMask == 0x7U &&
        receipt.inventoryChampionIndex == -1);

    input.inventoryChampionOrdinal = 3;
    input.inventoryPanelActive = 1;
    ok &= check("inventory owner is removed from primary F0296 redraw",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &receipt) &&
        receipt.inventoryChampionIndex == 2 && receipt.inventoryOwnerSkipsPrimaryF0296 &&
        receipt.topRowChampionMask == 0x7U && receipt.redrawOwnerMask == 0x3U);

    memset(&input, 0, sizeof(input));
    input.candidateChampionOrdinal = 1;
    ok &= check("candidate blocks F0296 only without inventory",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &receipt) &&
        receipt.candidateBlocksF0296 && receipt.redrawOwnerMask == 0U &&
        receipt.topRowChampionMask == 0x7U);

    input.inventoryChampionOrdinal = 2;
    input.inventoryPanelActive = 1;
    ok &= check("candidate with inventory retains primary ownership rule",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &receipt) &&
        !receipt.candidateBlocksF0296 && receipt.redrawOwnerMask == 0x5U &&
        receipt.inventoryChampionIndex == 1);

    party.activeChampionIndex = 3;
    ok &= check("invalid active leader fails closed",
        !dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &receipt));
    party.activeChampionIndex = 1;
    input.inventoryChampionOrdinal = 4;
    ok &= check("missing inventory owner fails closed",
        !dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &receipt));
    input.inventoryChampionOrdinal = 0;
    input.inventoryPanelActive = 0;
    party.champions[1].present = 0;
    ok &= check("noncompact party fails closed",
        !dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &receipt));
    return ok ? 0 : 1;
}
