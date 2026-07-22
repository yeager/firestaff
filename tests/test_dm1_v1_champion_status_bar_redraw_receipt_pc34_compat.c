#include "dm1_v1_champion_status_bar_redraw_receipt_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void party_with_members(struct PartyState_Compat *party)
{
    int slot;
    memset(party, 0, sizeof(*party));
    party->championCount = 2;
    party->activeChampionIndex = 0;
    for (slot = 0; slot < 2; ++slot) {
        party->champions[slot].present = 1;
        party->champions[slot].hp.current = 100;
        party->champions[slot].hp.maximum = 100;
        party->champions[slot].stamina.current = 100;
        party->champions[slot].stamina.maximum = 100;
        party->champions[slot].mana.current = 100;
        party->champions[slot].mana.maximum = 100;
    }
}

static void owned_policy(Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *policy)
{
    int slot;
    memset(policy, 0, sizeof(*policy));
    policy->valid = 1;
    for (slot = 0; slot < 2; ++slot) {
        policy->entries[slot].championIndex = slot;
        policy->entries[slot].policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34;
        policy->entries[slot].route = slot == 1
            ? DM1_V1_CHAMPION_PORTRAIT_STATUS_INVENTORY_F0292_PC34
            : DM1_V1_CHAMPION_PORTRAIT_STATUS_PRIMARY_F0296_PC34;
        policy->entries[slot].alive = 1;
    }
    for (slot = 2; slot < CHAMPION_MAX_PARTY; ++slot) {
        policy->entries[slot].championIndex = slot;
        policy->entries[slot].policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34;
    }
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionPortraitStatusRedrawReceiptPc34 policy;
    Dm1V1ChampionStatusBarRedrawReceiptPc34 receipt;
    int ok = 1;

    party_with_members(&party);
    owned_policy(&policy);
    party.champions[0].hp.current = 50;
    party.champions[0].mana.current = 0;
    ok &= check("exact clear and repaint split",
        dm1_v1_champion_status_bar_redraw_receipt_pc34(&party, &policy, &receipt) &&
        receipt.dataGateAccepted && receipt.clearCount == 2 && receipt.repaintCount == 5 &&
        receipt.operations[0].operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 &&
        receipt.operations[0].championIndex == 0 && receipt.operations[0].statIndex == 0 &&
        receipt.operations[1].operation == DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34 &&
        receipt.operations[3].operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 &&
        receipt.operations[4].route == DM1_V1_CHAMPION_PORTRAIT_STATUS_INVENTORY_F0292_PC34);

    policy.entries[1].policy = DM1_V1_CHAMPION_PORTRAIT_STATUS_CLEAR_PC34;
    ok &= check("missing portrait/status source clears all owner bars",
        dm1_v1_champion_status_bar_redraw_receipt_pc34(&party, &policy, &receipt) &&
        receipt.clearCount == 5 && receipt.repaintCount == 2 &&
        receipt.operations[4].operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34);

    owned_policy(&policy);
    policy.entries[1].alive = 0;
    party.champions[1].hp.current = 0;
    ok &= check("dead owner clears live bars",
        dm1_v1_champion_status_bar_redraw_receipt_pc34(&party, &policy, &receipt) &&
        receipt.clearCount == 5 && receipt.repaintCount == 2);

    party.champions[0].mana.maximum = 0;
    ok &= check("invalid original maximum fails closed",
        !dm1_v1_champion_status_bar_redraw_receipt_pc34(&party, &policy, &receipt));
    return ok ? 0 : 1;
}
