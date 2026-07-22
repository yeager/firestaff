#include "dm1_v1_champion_runtime_source_m11_bridge_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int append_command(Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 *receipt,
                          Dm1V1ChampionRuntimeSourceM11CommandKindPc34 kind,
                          int champion, int zone, int graphic,
                          const uint8_t *pixels, const uint8_t *portrait,
                          const uint8_t *palette, uint8_t *surface)
{
    Dm1V1ChampionRuntimeSourceM11CommandPc34 *command;
    if (!receipt || receipt->commandCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    command = &receipt->commands[receipt->commandCount++];
    command->kind = kind;
    command->championIndex = champion;
    command->zoneId = zone;
    command->graphicIndex = graphic;
    command->originalPixels = pixels;
    command->portraitPixels = portrait;
    command->originalPalette = palette;
    command->originalSurface = surface;
    return 1;
}

static const Dm1V1ChampionTopRowSurfacePc34 *surface_for_hand(
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assets, int graphic)
{
    if (graphic == DM1_GFX_SLOT_NORMAL) return &assets->assets.slotNormal;
    if (graphic == DM1_GFX_SLOT_WOUNDED) return &assets->assets.slotWounded;
    if (graphic == DM1_GFX_SLOT_ACTING) return &assets->assets.slotActing;
    return NULL;
}

static int runtime_source_is_complete(const Dm1V1ChampionRuntimeSourceReceiptPc34 *source)
{
    int i;
    int liveCount = 0;
    int statusCount = 0;
    if (!source || !source->valid || !source->liveEvidence.valid ||
        !source->topRowAssets.valid || !source->topRowAssets.c008Accepted ||
        !source->topRowAssets.c028Accepted || !source->topRowAssets.c033Accepted ||
        !source->topRowAssets.c034Accepted || !source->topRowAssets.c035Accepted ||
        source->liveEvidence.originalMaterials.c008Pixels !=
            source->topRowAssets.assets.deadStatusBox.pixels ||
        source->liveEvidence.originalMaterials.c028Pixels !=
            source->topRowAssets.assets.championIcons.pixels) return 0;
    for (i = 0; i < source->liveEvidence.evidenceCount; ++i) {
        const Dm1V1ChampionLiveRuntimeCompositionEvidencePc34 *e =
            &source->liveEvidence.evidence[i];
        if (e->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_C008_PC34) {
            if (e->originalPixels != source->topRowAssets.assets.deadStatusBox.pixels) return 0;
            ++liveCount;
        } else if (e->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_C028_PC34) {
            if (e->originalPixels != source->topRowAssets.assets.championIcons.pixels ||
                !e->portraitPixels) return 0;
            ++liveCount;
        } else if (e->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_STATUS_BAR_PC34) {
            if (e->originalPalette != source->liveEvidence.originalMaterials.indexedPalette ||
                e->originalSurface != source->liveEvidence.originalMaterials.indexedSurface) return 0;
            ++statusCount;
        } else {
            return 0;
        }
    }
    for (i = 0; i < source->handSourceCount; ++i) {
        const Dm1V1ChampionRuntimeHandSourcePc34 *hand = &source->handSources[i];
        const Dm1V1ChampionTopRowSurfacePc34 *surface =
            surface_for_hand(&source->topRowAssets, hand->graphicIndex);
        if (!surface || !hand->originalPixels || hand->originalPixels != surface->pixels ||
            surface->width != 18 || surface->height != 18) return 0;
    }
    return liveCount > 0 && statusCount > 0 && source->handSourceCount > 0;
}

static int append_stale_clears(Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 *receipt,
                               const Dm1V1ChampionRuntimeSourceReceiptPc34 *source)
{
    int i;
    for (i = 0; i < source->liveEvidence.evidenceCount; ++i) {
        const Dm1V1ChampionLiveRuntimeCompositionEvidencePc34 *e =
            &source->liveEvidence.evidence[i];
        if (!append_command(receipt, DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34,
                            e->championIndex, e->zoneId, 0, NULL, NULL, NULL, NULL))
            return 0;
    }
    for (i = 0; i < source->handSourceCount; ++i) {
        const Dm1V1ChampionRuntimeHandSourcePc34 *hand = &source->handSources[i];
        if (!append_command(receipt, DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34,
                            hand->championIndex,
                            dm1_v1_champion_status_hand_zone_id_pc34(
                                hand->championIndex, hand->handIndex),
                            0, NULL, NULL, NULL, NULL)) return 0;
    }
    return receipt->commandCount > 0;
}

void dm1_v1_champion_runtime_source_m11_bridge_init_pc34(
    Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_runtime_source_m11_bridge_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287/F0291 binds C008, C028, portrait, "
           "statusbar, and C033/C034/C035 hand sources into a top-row update. "
           "A stale update clears only those zones; it cannot pass fallback or "
           "generated artwork across the M11 boundary.";
}

int dm1_v1_champion_runtime_source_m11_bridge_pc34(
    Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 *state,
    const Dm1V1ChampionRuntimeSourceReceiptPc34 *runtimeSource,
    Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 *outReceipt)
{
    Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 pending;
    int stale;
    int i;
    if (!state || !runtimeSource || !outReceipt || !runtime_source_is_complete(runtimeSource))
        return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    stale = runtimeSource->tick <= state->lastTick ||
            runtimeSource->generation <= state->lastGeneration;
    if (stale) {
        if (!append_stale_clears(&pending, runtimeSource)) return 0;
        pending.clearOnly = 1;
    } else {
        for (i = 0; i < runtimeSource->liveEvidence.evidenceCount; ++i) {
            const Dm1V1ChampionLiveRuntimeCompositionEvidencePc34 *e =
                &runtimeSource->liveEvidence.evidence[i];
            Dm1V1ChampionRuntimeSourceM11CommandKindPc34 kind =
                e->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_C008_PC34
                ? DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34
                : e->kind == DM1_V1_CHAMPION_LIVE_RUNTIME_C028_PC34
                ? DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34
                : DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34;
            if (!append_command(&pending, kind, e->championIndex, e->zoneId, 0,
                                e->originalPixels, e->portraitPixels,
                                e->originalPalette, e->originalSurface)) return 0;
        }
        for (i = 0; i < runtimeSource->handSourceCount; ++i) {
            const Dm1V1ChampionRuntimeHandSourcePc34 *hand =
                &runtimeSource->handSources[i];
            if (!append_command(&pending, DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34,
                                hand->championIndex,
                                dm1_v1_champion_status_hand_zone_id_pc34(
                                    hand->championIndex, hand->handIndex),
                                hand->graphicIndex, hand->originalPixels,
                                NULL, NULL, NULL)) return 0;
        }
        state->lastTick = runtimeSource->tick;
        state->lastGeneration = runtimeSource->generation;
    }
    pending.tick = runtimeSource->tick;
    pending.generation = runtimeSource->generation;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
