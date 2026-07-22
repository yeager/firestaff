#include "dm1_v1_champion_runtime_source_receipt_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static const Dm1V1ChampionTopRowSurfacePc34 *hand_surface(
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets, int graphic)
{
    if (graphic == DM1_GFX_SLOT_NORMAL) return &assets->assets.slotNormal;
    if (graphic == DM1_GFX_SLOT_WOUNDED) return &assets->assets.slotWounded;
    if (graphic == DM1_GFX_SLOT_ACTING) return &assets->assets.slotActing;
    return NULL;
}

static int top_assets_are_exact(const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets)
{
    return assets && assets->valid && assets->c008Accepted && assets->c028Accepted &&
           assets->c033Accepted && assets->c034Accepted && assets->c035Accepted &&
           assets->assets.slotNormal.graphicIndex == DM1_GFX_SLOT_NORMAL &&
           assets->assets.slotNormal.pixels && assets->assets.slotNormal.width == 18 &&
           assets->assets.slotNormal.height == 18 &&
           assets->assets.slotWounded.graphicIndex == DM1_GFX_SLOT_WOUNDED &&
           assets->assets.slotWounded.pixels && assets->assets.slotWounded.width == 18 &&
           assets->assets.slotWounded.height == 18 &&
           assets->assets.slotActing.graphicIndex == DM1_GFX_SLOT_ACTING &&
           assets->assets.slotActing.pixels && assets->assets.slotActing.width == 18 &&
           assets->assets.slotActing.height == 18;
}

static int live_evidence_is_exact(const struct PartyState_Compat *party,
                                  const Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *evidence,
                                  unsigned int *outLiveMask)
{
    unsigned int liveMask = 0;
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!party || !evidence || !evidence->valid || !outLiveMask) return 0;
    for (i = 0; i < evidence->evidenceCount; ++i) {
        const Dm1V1ChampionLiveRuntimeCompositionEvidencePc34 *item = &evidence->evidence[i];
        if (item->championIndex < 0 || item->championIndex >= party->championCount ||
            !party->champions[item->championIndex].present) return 0;
        if (item->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_C028_PC34) {
            if (party->champions[item->championIndex].hp.current == 0 ||
                !party->champions[item->championIndex].portraitBitmapValid ||
                item->originalPixels != evidence->originalMaterials.c028Pixels ||
                item->portraitPixels != party->champions[item->championIndex].portraitBitmap)
                return 0;
            liveMask |= 1U << item->championIndex;
            ++sourceCount;
        } else if (item->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_C008_PC34) {
            if (party->champions[item->championIndex].hp.current > 0 ||
                item->originalPixels != evidence->originalMaterials.c008Pixels) return 0;
            ++sourceCount;
        } else if (item->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_STATUS_BAR_PC34) {
            if (item->originalPalette != evidence->originalMaterials.indexedPalette ||
                item->originalSurface != evidence->originalMaterials.indexedSurface) return 0;
            ++barCount;
        } else {
            return 0;
        }
    }
    *outLiveMask = liveMask;
    return sourceCount > 0 && barCount > 0;
}

const char *dm1_v1_champion_runtime_source_receipt_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292 consumes C008/C028 and champion portrait "
           "material, F0287 retains statusbar sources, and F0291 maps C033/C034/"
           "C035 hand-slot boxes. The runtime source receipt preserves only these "
           "original source identities and has no generated-art path.";
}

int dm1_v1_champion_runtime_source_receipt_pc34(
    const struct PartyState_Compat *party,
    int actingChampionOrdinal,
    const Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *liveEvidence,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *topRowAssets,
    Dm1V1ChampionRuntimeSourceReceiptPc34 *outReceipt)
{
    Dm1V1ChampionRuntimeSourceReceiptPc34 pending;
    unsigned int liveMask;
    int champion;
    if (!party || !outReceipt || actingChampionOrdinal < 0 ||
        actingChampionOrdinal > CHAMPION_MAX_PARTY || !top_assets_are_exact(topRowAssets) ||
        !live_evidence_is_exact(party, liveEvidence, &liveMask)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));

    for (champion = 0; champion < party->championCount; ++champion) {
        int hand;
        if (!(liveMask & (1U << champion))) continue;
        for (hand = DM1_SLOT_READY_HAND; hand <= DM1_SLOT_ACTION_HAND; ++hand) {
            int graphic = dm1_v1_champion_status_hand_slot_graphic_pc34(
                hand, party->champions[champion].wounds,
                hand == DM1_SLOT_ACTION_HAND && actingChampionOrdinal == champion + 1);
            const Dm1V1ChampionTopRowSurfacePc34 *surface = hand_surface(topRowAssets, graphic);
            Dm1V1ChampionRuntimeHandSourcePc34 *dest;
            if (!surface || !surface->pixels || surface->graphicIndex != graphic ||
                surface->width != 18 || surface->height != 18 ||
                pending.handSourceCount >= DM1_V1_CHAMPION_RUNTIME_SOURCE_MAX_HANDS_PC34)
                return 0;
            dest = &pending.handSources[pending.handSourceCount++];
            dest->championIndex = champion;
            dest->handIndex = hand;
            dest->graphicIndex = graphic;
            dest->originalPixels = surface->pixels;
        }
    }
    if (pending.handSourceCount == 0) return 0;
    pending.tick = liveEvidence->tick;
    pending.generation = liveEvidence->generation;
    pending.liveEvidence = *liveEvidence;
    pending.topRowAssets = *topRowAssets;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
