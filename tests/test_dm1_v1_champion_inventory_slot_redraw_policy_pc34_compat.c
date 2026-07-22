#include "dm1_v1_champion_inventory_slot_redraw_policy_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void party_with_inventory(struct PartyState_Compat *party)
{
    int champion;
    int slot;
    memset(party, 0, sizeof(*party));
    party->championCount = 3;
    party->activeChampionIndex = 0;
    for (champion = 0; champion < party->championCount; ++champion) {
        party->champions[champion].present = 1;
        for (slot = 0; slot < CHAMPION_SLOT_COUNT; ++slot)
            party->champions[champion].inventory[slot] = THING_NONE;
    }
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionLeaderOwnershipInputPc34 input;
    Dm1V1ChampionLeaderOwnershipReceiptPc34 ownership;
    Dm1V1ChampionInventorySlotRedrawReceiptPc34 receipt;
    int ok = 1;

    party_with_inventory(&party);
    party.champions[1].inventory[0] = 0x0123u;
    party.champions[1].inventory[20] = 0x0456u;
    memset(&input, 0, sizeof(input));
    input.inventoryChampionOrdinal = 2;
    input.inventoryPanelActive = 1;
    ok &= check("owner policy derives from original inventory",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &ownership) &&
        dm1_v1_champion_inventory_slot_redraw_policy_pc34(&party, &ownership, &receipt) &&
        receipt.inventoryChampionIndex == 1 && receipt.ownedCount == 2 &&
        receipt.clearCount == CHAMPION_SLOT_COUNT - 2 && receipt.skipCount == 0 &&
        receipt.slots[0].policy == DM1_V1_CHAMPION_INVENTORY_SLOT_OWNED_PC34 &&
        receipt.slots[0].thing == 0x0123u &&
        receipt.slots[1].policy == DM1_V1_CHAMPION_INVENTORY_SLOT_CLEAR_PC34 &&
        receipt.slots[20].policy == DM1_V1_CHAMPION_INVENTORY_SLOT_OWNED_PC34);

    memset(&input, 0, sizeof(input));
    ok &= check("no active inventory explicitly skips all slots",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &ownership) &&
        dm1_v1_champion_inventory_slot_redraw_policy_pc34(&party, &ownership, &receipt) &&
        receipt.ownedCount == 0 && receipt.clearCount == 0 &&
        receipt.skipCount == CHAMPION_SLOT_COUNT &&
        receipt.slots[0].policy == DM1_V1_CHAMPION_INVENTORY_SLOT_SKIP_PC34);

    memset(&input, 0, sizeof(input));
    input.inventoryChampionOrdinal = 2;
    input.inventoryPanelActive = 1;
    ok &= check("valid owner is required before source walk",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &ownership));
    party.champions[1].inventory[4] =
        DM1_V1_CHAMPION_INVENTORY_SLOT_REDRAW_THING_END_OF_LIST_PC34;
    ok &= check("non-slot sentinel fails closed",
        !dm1_v1_champion_inventory_slot_redraw_policy_pc34(&party, &ownership, &receipt));
    ownership.inventoryChampionOrdinal = 3;
    ok &= check("tampered ownership fails closed",
        !dm1_v1_champion_inventory_slot_redraw_policy_pc34(&party, &ownership, &receipt));
    return ok ? 0 : 1;
}
