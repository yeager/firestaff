#include "dm1_v1_champion_live_m11_bridge_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t pixels[76 * 29] = { 1 };
static const uint8_t palette[16 * 3] = { 1 };
static uint8_t surface[320 * 200];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void party_data(struct PartyState_Compat *party)
{
    memset(party, 0, sizeof(*party));
    party->championCount = 1;
    party->champions[0].present = 1;
    party->champions[0].hp.current = 100;
    party->champions[0].portraitBitmapValid = 1;
}

static void assets(Dm1V1ChampionTopRowAssetsReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = out->c008Accepted = out->c028Accepted = out->c033Accepted = 1;
    out->c034Accepted = out->c035Accepted = 1;
    out->assets.deadStatusBox.pixels = pixels;
    out->assets.championIcons.pixels = pixels;
    out->assets.slotNormal = (Dm1V1ChampionTopRowSurfacePc34){ DM1_GFX_SLOT_NORMAL, 1, pixels, 18, 18 };
    out->assets.slotWounded = (Dm1V1ChampionTopRowSurfacePc34){ DM1_GFX_SLOT_WOUNDED, 1, pixels, 18, 18 };
    out->assets.slotActing = (Dm1V1ChampionTopRowSurfacePc34){ DM1_GFX_SLOT_ACTING, 1, pixels, 18, 18 };
}

static void evidence(Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *out,
                     const struct PartyState_Compat *party)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = 1; out->generation = 1;
    out->originalMaterials.c008Pixels = pixels; out->originalMaterials.c028Pixels = pixels;
    out->originalMaterials.indexedPalette = palette; out->originalMaterials.indexedSurface = surface;
    out->evidenceCount = 2;
    out->evidence[0] = (Dm1V1ChampionLiveRuntimeCompositionEvidencePc34){
        DM1_V1_CHAMPION_LIVE_RUNTIME_C028_PC34, 0, 113, pixels,
        party->champions[0].portraitBitmap, NULL, NULL };
    out->evidence[1] = (Dm1V1ChampionLiveRuntimeCompositionEvidencePc34){
        DM1_V1_CHAMPION_LIVE_RUNTIME_STATUS_BAR_PC34, 0, 195, NULL, NULL, palette, surface };
}

int main(void)
{
    Dm1V1ChampionLiveM11BridgeStatePc34 state;
    struct PartyState_Compat party;
    Dm1V1ChampionTopRowAssetsReceiptPc34 topAssets;
    Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 live;
    Dm1V1ChampionLiveM11BridgeReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_live_m11_bridge_init_pc34(&state);
    party_data(&party); assets(&topAssets); evidence(&live, &party);
    ok &= check("live C008 C028 status and hand proof publishes", dm1_v1_champion_live_m11_bridge_pc34(
        &state, &party, 1, &live, &topAssets, &receipt) && !receipt.clearOnly &&
        receipt.m11.commandCount == 4 && receipt.runtimeSource.handSourceCount == 2);
    ok &= check("stale live receipt clears only", dm1_v1_champion_live_m11_bridge_pc34(
        &state, &party, 1, &live, &topAssets, &receipt) && receipt.clearOnly &&
        receipt.m11.commands[0].originalPixels == NULL);
    return ok ? 0 : 1;
}
