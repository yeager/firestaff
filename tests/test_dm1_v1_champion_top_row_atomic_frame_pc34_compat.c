#include "dm1_v1_champion_top_row_atomic_frame_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t pixels[76 * 29] = { 1 };
static uint8_t surface[320 * 200];
static const uint8_t palette[16 * 3] = { 1 };

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
        party->champions[slot].portraitBitmapValid = 1;
    }
}

static void policy_with_originals(Dm1V1ChampionPortraitStatusRedrawReceiptPc34 *policy,
                                  const struct PartyState_Compat *party)
{
    int slot;
    memset(policy, 0, sizeof(*policy));
    policy->valid = 1;
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        policy->entries[slot].championIndex = slot;
        policy->entries[slot].policy = slot < 2
            ? DM1_V1_CHAMPION_PORTRAIT_STATUS_OWNED_PC34
            : DM1_V1_CHAMPION_PORTRAIT_STATUS_SKIP_PC34;
        policy->entries[slot].route = DM1_V1_CHAMPION_PORTRAIT_STATUS_PRIMARY_F0296_PC34;
        policy->entries[slot].alive = 1;
        policy->entries[slot].portraitPixels = party->champions[slot].portraitBitmap;
        policy->entries[slot].statusPixels = pixels;
    }
}

static void original_assets(Dm1V1ChampionTopRowAssetsReceiptPc34 *assets)
{
    memset(assets, 0, sizeof(*assets));
    assets->c008Accepted = 1;
    assets->c028Accepted = 1;
    assets->assets.deadStatusBox = (Dm1V1ChampionTopRowSurfacePc34){
        DM1_GFX_DEAD_CHAMPION, 1, pixels, 67, 29 };
    assets->assets.championIcons = (Dm1V1ChampionTopRowSurfacePc34){
        DM1_GFX_CHAMPION_ICONS, 1, pixels, 76, 14 };
}

static void atomic_status_bars(Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *bars)
{
    memset(bars, 0, sizeof(*bars));
    bars->valid = 1;
    bars->atomicPublish = 1;
    bars->operationCount = 1;
    bars->operations[0].operation = DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34;
    bars->operations[0].championIndex = 0;
    bars->operations[0].zoneId = 195;
    bars->operations[0].originalPalette = palette;
    bars->operations[0].originalIndexedSurface = surface;
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionPortraitStatusRedrawReceiptPc34 policy;
    Dm1V1ChampionTopRowAssetsReceiptPc34 assets;
    Dm1V1ChampionStatusBarFramePresentationReceiptPc34 bars;
    Dm1V1ChampionTopRowAtomicFrameReceiptPc34 receipt;
    int ok = 1;

    party_with_members(&party);
    policy_with_originals(&policy, &party);
    original_assets(&assets);
    atomic_status_bars(&bars);
    ok &= check("C028 and statusbars publish together",
        dm1_v1_champion_top_row_atomic_frame_pc34(&party, &policy, &assets, &bars, &receipt) &&
        receipt.valid && receipt.originalMaterialsPublished && !receipt.clearOnly &&
        receipt.operationCount == 3 &&
        receipt.operations[0].operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34 &&
        receipt.operations[2].operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34);

    assets.c028Accepted = 0;
    ok &= check("missing C028 clears entire selected top row",
        dm1_v1_champion_top_row_atomic_frame_pc34(&party, &policy, &assets, &bars, &receipt) &&
        receipt.clearOnly && !receipt.originalMaterialsPublished && receipt.operationCount == 8 &&
        receipt.operations[0].operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34);
    original_assets(&assets);
    bars.atomicPublish = 0;
    ok &= check("missing statusbar frame clears instead of mixing sources",
        dm1_v1_champion_top_row_atomic_frame_pc34(&party, &policy, &assets, &bars, &receipt) &&
        receipt.clearOnly && receipt.operationCount == 8);
    return ok ? 0 : 1;
}
