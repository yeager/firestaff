#include "dm1_v1_champion_top_row_m11_host_render_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static int append_command(Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *receipt,
                          Dm1V1ChampionTopRowM11HostRenderCommandKindPc34 kind,
                          const Dm1V1ChampionTopRowAtomicFrameOpPc34 *source)
{
    if (!receipt || !source || receipt->commandCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    receipt->commands[receipt->commandCount].kind = kind;
    receipt->commands[receipt->commandCount].source = *source;
    ++receipt->commandCount;
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

static int append_stale_clears(Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *receipt,
                               const Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *lifecycle)
{
    int i;
    for (i = 0; i < lifecycle->clearOperationCount; ++i) {
        Dm1V1ChampionTopRowAtomicFrameOpPc34 source;
        memset(&source, 0, sizeof(source));
        source.operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
        source.championIndex = lifecycle->clearOperations[i].championIndex;
        source.zoneId = lifecycle->clearOperations[i].zoneId;
        source.x = lifecycle->clearOperations[i].x;
        source.y = lifecycle->clearOperations[i].y;
        source.width = lifecycle->clearOperations[i].width;
        source.height = lifecycle->clearOperations[i].height;
        if (!append_command(receipt, DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34,
                            &source)) return 0;
    }
    return receipt->commandCount > 0;
}

static int append_composition(Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *receipt,
                              const Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *lifecycle)
{
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!proof_is_exact(&lifecycle->consumption.originalMaterials)) return 0;
    for (i = 0; i < lifecycle->consumption.operationCount; ++i) {
        const Dm1V1ChampionTopRowHostConsumptionOpPc34 *op =
            &lifecycle->consumption.operations[i];
        Dm1V1ChampionTopRowM11HostRenderCommandKindPc34 kind;
        if (op->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34) {
            if (op->source.sourcePixels != lifecycle->consumption.originalMaterials.c008Pixels)
                return 0;
            kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_BLIT_C008_PC34;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34) {
            if (op->source.sourcePixels != lifecycle->consumption.originalMaterials.c028Pixels ||
                !op->source.portraitPixels) return 0;
            kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34) {
            if (op->source.statusBar.originalPalette !=
                    lifecycle->consumption.originalMaterials.indexedPalette ||
                op->source.statusBar.originalIndexedSurface !=
                    lifecycle->consumption.originalMaterials.indexedSurface) return 0;
            kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34;
            ++barCount;
        } else {
            return 0;
        }
        if (!append_command(receipt, kind, &op->source)) return 0;
    }
    return sourceCount > 0 && barCount > 0;
}

void dm1_v1_champion_top_row_m11_host_render_init_pc34(
    Dm1V1ChampionTopRowM11HostRenderStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_m11_host_render_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 yields top-row material for the "
           "present path, whereas a stale generation must only clear its affected "
           "zones. F0680/F0692 host presentation therefore receives either a full "
           "proven composition or a clear-only command sequence.";
}

int dm1_v1_champion_top_row_m11_host_render_receipt_pc34(
    Dm1V1ChampionTopRowM11HostRenderStatePc34 *state,
    const Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *lifecycle,
    Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowM11HostRenderStatePc34 nextState;
    Dm1V1ChampionTopRowM11HostRenderReceiptPc34 pending;
    int i;
    if (!state || !lifecycle || !outReceipt || !lifecycle->valid ||
        lifecycle->tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (lifecycle->publication == DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_CLEAR_PC34) {
        if (lifecycle->generation <= nextState.pendingClearGeneration) return 0;
        for (i = 0; i < lifecycle->consumption.operationCount; ++i) {
            if (lifecycle->consumption.operations[i].kind !=
                    DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34 ||
                !append_command(&pending, DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34,
                    &lifecycle->consumption.operations[i].source)) return 0;
        }
        if (pending.commandCount == 0) return 0;
        nextState.pendingClearGeneration = lifecycle->generation;
        pending.clearOnly = 1;
    } else if (lifecycle->publication ==
               DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_STALE_ZONE_CLEAR_PC34) {
        if (!append_stale_clears(&pending, lifecycle)) return 0;
        pending.clearOnly = 1;
    } else if (lifecycle->publication ==
               DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_COMPOSITION_PC34) {
        if (lifecycle->generation != nextState.pendingClearGeneration ||
            lifecycle->generation <= nextState.lastCompositionGeneration ||
            !append_composition(&pending, lifecycle)) return 0;
        nextState.lastCompositionGeneration = lifecycle->generation;
        nextState.pendingClearGeneration = 0;
    } else {
        return 0;
    }
    nextState.lastTick = lifecycle->tick;
    pending.tick = lifecycle->tick;
    pending.generation = lifecycle->generation;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
