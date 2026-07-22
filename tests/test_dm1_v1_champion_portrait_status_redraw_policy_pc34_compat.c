#include "dm1_v1_champion_portrait_status_redraw_policy_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t pixels[76 * 29] = { 1 };

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void party_with_members(struct PartyState_Compat *party)
{
    int slot;
    memset(party, 0, sizeof(*party));
    party->championCount = 3;
    party->activeChampionIndex = 0;
    for (slot = 0; slot < 3; ++slot) {
        party->champions[slot].present = 1;
        party->champions[slot].hp.current = 100;
        party->champions[slot].portraitBitmapValid = 1;
    }
}

static void assets_with_originals(Dm1V1ChampionTopRowAssetsReceiptPc34 *assets)
{
    memset(assets, 0, sizeof(*assets));
    assets->c008Accepted = 1;
    assets->assets.deadStatusBox = (Dm1V1ChampionTopRowSurfacePc34){
        DM1_GFX_DEAD_CHAMPION, 1, pixels, 67, 29 };
    assets->c028Accepted = 1;
    assets->assets.championIcons = (Dm1V1ChampionTopRowSurfacePc34){
        DM1_GFX_CHAMPION_ICONS, 1, pixels, 76, 14 };
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionLeaderOwnershipInputPc34 input;
    Dm1V1ChampionLeaderOwnershipReceiptPc34 ownership;
    Dm1V1ChampionTopRowAssetsReceiptPc34 assets;
    Dm1V1ChampionPortraitStatusRedrawReceiptPc34 receipt;
    int ok = 1;

    party_with_members(&party);
    assets_with_originals(&assets);
    memset(&input, 0, sizeof(input));
    ok &= check("live champions own original portrait and C028 status",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &ownership) &&
        dm1_v1_champion_portrait_status_redraw_policy_pc34(&party, &ownership, &assets, &receipt) &&
        receipt.ownedCount == 3 && receipt.skipCount == 1 && receipt.clearCount == 0 &&
        receipt.entries[0].portraitPixels == party.champions[0].portraitBitmap &&
        receipt.entries[0].statusPixels == pixels);

    party.champions[1].portraitBitmapValid = 0;
    ok &= check("missing live portrait clears rather than substitutes",
        dm1_v1_champion_portrait_status_redraw_policy_pc34(&party, &ownership, &assets, &receipt) &&
        receipt.entries[1].policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_CLEAR_PC34 &&
        receipt.entries[1].portraitPixels == NULL);

    party.champions[1].hp.current = 0;
    ok &= check("dead champion owns C008 without a live portrait",
        dm1_v1_champion_portrait_status_redraw_policy_pc34(&party, &ownership, &assets, &receipt) &&
        receipt.entries[1].policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34 &&
        receipt.entries[1].statusGraphicIndex == DM1_GFX_DEAD_CHAMPION &&
        receipt.entries[1].portraitPixels == NULL);

    assets.c008Accepted = 0;
    ok &= check("missing C008 clears dead status",
        dm1_v1_champion_portrait_status_redraw_policy_pc34(&party, &ownership, &assets, &receipt) &&
        receipt.entries[1].policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_CLEAR_PC34);

    party_with_members(&party);
    assets_with_originals(&assets);
    input.inventoryChampionOrdinal = 2;
    input.inventoryPanelActive = 1;
    ok &= check("inventory champion receives F0292 ownership route",
        dm1_v1_champion_leader_ownership_handoff_pc34(&party, &input, &ownership) &&
        dm1_v1_champion_portrait_status_redraw_policy_pc34(&party, &ownership, &assets, &receipt) &&
        receipt.entries[1].policy == DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34 &&
        receipt.entries[1].route == DM1_V1_CHAMPION_PORTRAIT_STATUS_INVENTORY_F0292_PC34);
    return ok ? 0 : 1;
}
