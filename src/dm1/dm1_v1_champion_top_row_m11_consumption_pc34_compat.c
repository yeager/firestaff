#include "dm1_v1_champion_top_row_m11_consumption_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static int material_proof_is_exact(const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *proof)
{
    return proof && proof->c008Original && proof->c008Pixels &&
           proof->c008GraphicIndex == DM1_GFX_DEAD_CHAMPION &&
           proof->c008Width == 67 && proof->c008Height == 29 &&
           proof->c028Original && proof->c028Pixels &&
           proof->c028GraphicIndex == DM1_GFX_CHAMPION_ICONS &&
           proof->c028Width == 76 && proof->c028Height == 14 &&
           proof->indexedPaletteOriginal && proof->indexedPalette &&
           proof->indexedPaletteEntryCount >= 16 &&
           proof->indexedSurfaceOriginal && proof->indexedSurface;
}

static int host_clear_is_valid(const Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *host)
{
    int i;
    if (!host || !host->valid || host->hostConsumption.operationCount <= 0) return 0;
    for (i = 0; i < host->hostConsumption.operationCount; ++i) {
        if (host->hostConsumption.operations[i].kind !=
                DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34) return 0;
    }
    return 1;
}

static int host_composition_matches_proof(
    const Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *host,
    const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *proof)
{
    int i;
    int sourceCount = 0;
    int statusCount = 0;
    if (!host || !host->valid || host->hostConsumption.operationCount <= 0) return 0;
    for (i = 0; i < host->hostConsumption.operationCount; ++i) {
        const Dm1V1ChampionTopRowHostConsumptionOpPc34 *op =
            &host->hostConsumption.operations[i];
        if (op->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34) {
            if (op->source.sourcePixels != proof->c008Pixels ||
                op->source.graphicIndex != DM1_GFX_DEAD_CHAMPION) return 0;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34) {
            if (op->source.sourcePixels != proof->c028Pixels ||
                !op->source.portraitPixels ||
                op->source.graphicIndex != DM1_GFX_CHAMPION_ICONS) return 0;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34) {
            if (op->source.statusBar.originalPalette != proof->indexedPalette ||
                op->source.statusBar.originalIndexedSurface != proof->indexedSurface) return 0;
            ++statusCount;
        } else {
            return 0;
        }
    }
    return sourceCount > 0 && statusCount > 0;
}

void dm1_v1_champion_top_row_m11_consumption_init_pc34(
    Dm1V1ChampionTopRowM11ConsumptionStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_m11_consumption_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292 admits C008/C028 and F0287 bar material "
           "from retained original graphics/palette state; F0680/F0692 later "
           "consume the same logical screen. This receipt makes those original "
           "pointers explicit before an M11 host adapter may consume them.";
}

int dm1_v1_champion_top_row_m11_consumption_receipt_pc34(
    Dm1V1ChampionTopRowM11ConsumptionStatePc34 *state,
    const Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *hostLifecycle,
    const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *originalMaterials,
    Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowM11ConsumptionStatePc34 nextState;
    Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 pending;
    int i;
    if (!state || !hostLifecycle || !outReceipt || !material_proof_is_exact(originalMaterials) ||
        !hostLifecycle->valid || hostLifecycle->tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (hostLifecycle->publication == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34) {
        if (!host_clear_is_valid(hostLifecycle) ||
            hostLifecycle->generation <= nextState.pendingClearGeneration) return 0;
        nextState.pendingClearGeneration = hostLifecycle->generation;
    } else if (hostLifecycle->publication ==
               DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34) {
        if (hostLifecycle->generation != nextState.pendingClearGeneration ||
            hostLifecycle->generation <= nextState.lastCompositionGeneration ||
            !host_composition_matches_proof(hostLifecycle, originalMaterials)) return 0;
        nextState.lastCompositionGeneration = hostLifecycle->generation;
        nextState.pendingClearGeneration = 0;
    } else {
        return 0;
    }

    for (i = 0; i < hostLifecycle->hostConsumption.operationCount; ++i)
        pending.operations[pending.operationCount++] = hostLifecycle->hostConsumption.operations[i];
    nextState.lastTick = hostLifecycle->tick;
    pending.tick = hostLifecycle->tick;
    pending.publication = hostLifecycle->publication;
    pending.generation = hostLifecycle->generation;
    pending.originalMaterials = *originalMaterials;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
