#include "dm1_v1_f0354_portrait_material_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static unsigned char c028[76 * 14];

static int check(int value, const char *message)
{
    if (!value) fprintf(stderr, "FAIL: %s\n", message);
    return value;
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionLeaderOwnershipInputPc34 input;
    Dm1V1ChampionLeaderOwnershipReceiptPc34 ownership;
    Dm1V1ChampionTopRowAssetsReceiptPc34 assets;
    DM1_V1_F0354PortraitMaterialReceiptPc34 receipt;
    int ok = 1;

    memset(&party, 0, sizeof(party));
    memset(&input, 0, sizeof(input));
    memset(&assets, 0, sizeof(assets));
    party.championCount = 1;
    party.activeChampionIndex = 0;
    party.champions[0].present = 1;
    party.champions[0].hp.current = 100;
    party.champions[0].portraitBitmapValid = 1;
    party.champions[0].portraitBitmap[0] = 0x31u;
    party.champions[0].portraitBitmap[CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT - 1] = 0x9cu;
    c028[0] = 0x7du;
    c028[sizeof(c028) - 1u] = 0x04u;
    input.inventoryChampionOrdinal = 1;
    input.inventoryPanelActive = 1;
    assets.c028Accepted = 1;
    assets.assets.championIcons.graphicIndex = DM1_GFX_CHAMPION_ICONS;
    assets.assets.championIcons.width = 76;
    assets.assets.championIcons.height = 14;
    assets.assets.championIcons.pixels = c028;

    ok &= check(dm1_v1_champion_leader_ownership_handoff_pc34(
                    &party, &input, &ownership), "ownership receipt is source-valid");
    ok &= check(dm1_v1_f0354_portrait_material_receipt_pc34(
                    &party, &ownership, &assets, 0, &receipt) && receipt.valid &&
                    receipt.statusGraphicIndex == DM1_GFX_CHAMPION_ICONS &&
                    receipt.portraitFingerprint != 0u && receipt.c028Fingerprint != 0u,
                "F0354 binds raw portrait to C028 GRAPHICS.DAT");
    party.champions[0].portraitBitmapValid = 0;
    ok &= check(!dm1_v1_f0354_portrait_material_receipt_pc34(
                    &party, &ownership, &assets, 0, &receipt) && !receipt.valid,
                "F0354 fails closed for unavailable PC34 portrait data");
    party.champions[0].portraitBitmapValid = 1;
    assets.c028Accepted = 0;
    ok &= check(!dm1_v1_f0354_portrait_material_receipt_pc34(
                    &party, &ownership, &assets, 0, &receipt) && !receipt.valid,
                "F0354 fails closed for unavailable C028 material");
    if (!ok) return 1;
    puts("PASS: DM1 F0354 raw portrait and C028 material admission");
    return 0;
}
