#include "dm1_v1_champion_inventory_slot_redraw_policy_pc34_compat.h"

#include <string.h>

static int ownership_is_consistent(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership)
{
    if (!party || !ownership || !ownership->valid ||
        ownership->partyChampionCount != party->championCount) return 0;
    if (ownership->inventoryChampionIndex < -1 ||
        ownership->inventoryChampionIndex >= party->championCount) return 0;
    if (ownership->inventoryChampionIndex == -1) {
        return ownership->inventoryChampionOrdinal == 0 &&
               !ownership->inventoryOwnerSkipsPrimaryF0296;
    }
    return ownership->inventoryChampionOrdinal ==
               ownership->inventoryChampionIndex + 1 &&
           ownership->inventoryOwnerSkipsPrimaryF0296 &&
           party->champions[ownership->inventoryChampionIndex].present;
}

const char *dm1_v1_champion_inventory_slot_redraw_policy_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0296:1234-1242 walks the selected "
           "G0423 inventory champion slots; F0295 compares the original "
           "slot thing and F0386 redraws changed non-empty slots. "
           "DEFS.H C0xFFFF_THING_NONE is the explicit empty-slot state; "
           "F0296:1256-1259 dispatches the owner redraw after the walk.";
}

int dm1_v1_champion_inventory_slot_redraw_policy_pc34(
    const struct PartyState_Compat *party,
    const Dm1V1ChampionLeaderOwnershipReceiptPc34 *ownership,
    Dm1V1ChampionInventorySlotRedrawReceiptPc34 *outReceipt)
{
    int slot;
    int owner;

    if (!outReceipt || !ownership_is_consistent(party, ownership)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->inventoryChampionIndex = ownership->inventoryChampionIndex;
    owner = ownership->inventoryChampionIndex;

    for (slot = 0; slot < CHAMPION_SLOT_COUNT; ++slot) {
        Dm1V1ChampionInventorySlotRedrawEntryPc34 *entry = &outReceipt->slots[slot];
        entry->championIndex = owner;
        entry->slotIndex = slot;

        if (owner < 0 || ownership->candidateBlocksF0296) {
            entry->policy = DM1_V1_CHAMPION_INVENTORY_SLOT_SKIP_PC34;
            ++outReceipt->skipCount;
            continue;
        }

        entry->thing = party->champions[owner].inventory[slot];
        if (entry->thing ==
            DM1_V1_CHAMPION_INVENTORY_SLOT_REDRAW_THING_END_OF_LIST_PC34) return 0;
        if (entry->thing == THING_NONE) {
            entry->policy = DM1_V1_CHAMPION_INVENTORY_SLOT_CLEAR_PC34;
            ++outReceipt->clearCount;
        } else {
            entry->policy = DM1_V1_CHAMPION_INVENTORY_SLOT_OWNED_PC34;
            ++outReceipt->ownedCount;
        }
    }
    outReceipt->valid = 1;
    return 1;
}
