#include "dm1_v1_champion_top_row_m11_lifecycle_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int append_clear(Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *receipt,
                        int champion, int zone, int x, int y, int width, int height)
{
    int i;
    Dm1V1ChampionTopRowM11LifecycleClearOpPc34 *op;
    if (!receipt || champion < 0 || champion >= CHAMPION_MAX_PARTY || zone < 0 ||
        width <= 0 || height <= 0) return 0;
    for (i = 0; i < receipt->clearOperationCount; ++i) {
        if (receipt->clearOperations[i].zoneId == zone) return 1;
    }
    if (receipt->clearOperationCount >= DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34)
        return 0;
    op = &receipt->clearOperations[receipt->clearOperationCount++];
    op->championIndex = champion;
    op->zoneId = zone;
    op->x = x;
    op->y = y;
    op->width = width;
    op->height = height;
    return 1;
}

static int proof_is_exact(const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *proof)
{
    return proof && proof->c008Original && proof->c008Pixels &&
           proof->c008GraphicIndex == DM1_GFX_DEAD_CHAMPION &&
           proof->c008Width == 67 && proof->c008Height == 29 &&
           proof->c028Original && proof->c028Pixels &&
           proof->c028GraphicIndex == DM1_GFX_CHAMPION_ICONS &&
           proof->c028Width == 76 && proof->c028Height == 14 &&
           proof->indexedPaletteOriginal && proof->indexedPalette &&
           proof->indexedPaletteEntryCount >= 16 && proof->indexedSurfaceOriginal &&
           proof->indexedSurface;
}

static int add_relevant_composition_zones(
    Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *receipt,
    const Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *consumption)
{
    int i;
    for (i = 0; i < consumption->operationCount; ++i) {
        const Dm1V1ChampionTopRowHostConsumptionOpPc34 *op = &consumption->operations[i];
        if (op->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34 ||
            op->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34) {
            DM1_V1_ChampionStatusRectPc34 rect;
            int champion = op->source.championIndex;
            if (champion < 0 || champion >= CHAMPION_MAX_PARTY ||
                !dm1_v1_champion_status_box_rect_pc34(champion, &rect) ||
                !append_clear(receipt, champion,
                    dm1_v1_champion_status_box_zone_id_pc34(champion),
                    rect.x, rect.y, rect.w, rect.h)) return 0;
        } else if (op->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34) {
            const Dm1V1ChampionStatusBarFrameOpPc34 *bar = &op->source.statusBar;
            if (!append_clear(receipt, bar->championIndex, bar->zoneId,
                              bar->x, bar->y, bar->width, bar->height)) return 0;
        } else {
            return 0;
        }
    }
    return receipt->clearOperationCount > 0;
}

static int composition_matches_proof(const Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *consumption)
{
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!proof_is_exact(&consumption->originalMaterials) || consumption->operationCount <= 0)
        return 0;
    for (i = 0; i < consumption->operationCount; ++i) {
        const Dm1V1ChampionTopRowHostConsumptionOpPc34 *op = &consumption->operations[i];
        if (op->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34) {
            if (op->source.sourcePixels != consumption->originalMaterials.c008Pixels) return 0;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34) {
            if (op->source.sourcePixels != consumption->originalMaterials.c028Pixels ||
                !op->source.portraitPixels) return 0;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34) {
            if (op->source.statusBar.originalPalette !=
                    consumption->originalMaterials.indexedPalette ||
                op->source.statusBar.originalIndexedSurface !=
                    consumption->originalMaterials.indexedSurface) return 0;
            ++barCount;
        } else {
            return 0;
        }
    }
    return sourceCount > 0 && barCount > 0;
}

void dm1_v1_champion_top_row_m11_lifecycle_init_pc34(
    Dm1V1ChampionTopRowM11LifecycleStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_m11_lifecycle_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 binds top-row and bar regions to "
           "one current logical-screen generation. F0680/F0692 cannot present "
           "stale source material after another clear; only those affected zones "
           "are cleared while the current generation remains pending.";
}

int dm1_v1_champion_top_row_m11_lifecycle_step_pc34(
    Dm1V1ChampionTopRowM11LifecycleStatePc34 *state,
    const Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *consumption,
    Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowM11LifecycleStatePc34 nextState;
    Dm1V1ChampionTopRowM11LifecycleReceiptPc34 pending;
    int compositionCurrent;
    if (!state || !consumption || !outReceipt || !consumption->valid ||
        consumption->tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (consumption->publication == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34) {
        if (consumption->generation <= nextState.lastClearGeneration) return 0;
        nextState.lastClearGeneration = consumption->generation;
        nextState.pendingClearGeneration = consumption->generation;
        pending.publication = DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_CLEAR_PC34;
    } else if (consumption->publication ==
               DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34) {
        compositionCurrent = consumption->generation == nextState.pendingClearGeneration &&
            consumption->generation > nextState.lastCompositionGeneration &&
            composition_matches_proof(consumption);
        if (!compositionCurrent) {
            if (!add_relevant_composition_zones(&pending, consumption)) return 0;
            pending.publication = DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_STALE_ZONE_CLEAR_PC34;
        } else {
            nextState.lastCompositionGeneration = consumption->generation;
            nextState.pendingClearGeneration = 0;
            pending.publication = DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_COMPOSITION_PC34;
        }
    } else {
        return 0;
    }

    nextState.lastTick = consumption->tick;
    pending.tick = consumption->tick;
    pending.generation = consumption->generation;
    pending.consumption = *consumption;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
