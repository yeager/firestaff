#include "dm1_v1_champion_top_row_lifecycle_render_bridge_pc34_compat.h"

#include <string.h>

static int append_command(Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *receipt,
                          Dm1V1ChampionTopRowLifecycleRenderCommandKindPc34 kind,
                          const Dm1V1ChampionTopRowAtomicFrameOpPc34 *source)
{
    if (!receipt || !source || receipt->commandCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    receipt->commands[receipt->commandCount].kind = kind;
    receipt->commands[receipt->commandCount].source = *source;
    ++receipt->commandCount;
    return 1;
}

static int clear_frame_is_renderable(const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame)
{
    int i;
    if (!frame || !frame->valid || !frame->clearOnly || frame->originalMaterialsPublished ||
        frame->operationCount <= 0 || frame->operationCount >
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < frame->operationCount; ++i) {
        if (frame->operations[i].operation != DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34)
            return 0;
    }
    return 1;
}

static int composition_frame_is_renderable(
    const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame)
{
    int i;
    int sourceCount = 0;
    int statusCount = 0;
    if (!frame || !frame->valid || frame->clearOnly || !frame->originalMaterialsPublished ||
        frame->operationCount <= 0 || frame->operationCount >
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < frame->operationCount; ++i) {
        const Dm1V1ChampionTopRowAtomicFrameOpPc34 *op = &frame->operations[i];
        if (op->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_BLIT_C008_PC34) {
            if (!op->sourcePixels) return 0;
            ++sourceCount;
        } else if (op->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34) {
            if (!op->sourcePixels || !op->portraitPixels) return 0;
            ++sourceCount;
        } else if (op->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34) {
            if (!op->statusBar.originalPalette || !op->statusBar.originalIndexedSurface ||
                op->statusBar.width <= 0 || op->statusBar.height <= 0) return 0;
            ++statusCount;
        } else {
            return 0;
        }
    }
    return sourceCount > 0 && statusCount > 0;
}

void dm1_v1_champion_top_row_lifecycle_render_bridge_init_pc34(
    Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_lifecycle_render_bridge_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 emits ordered top-row operations, "
           "and F0680/F0692 consume only a complete retained logical-screen "
           "frame. The bridge preserves the prior clear before forwarding C008, "
           "C028, and statusbar operations to a renderer.";
}

int dm1_v1_champion_top_row_lifecycle_render_bridge_pc34(
    Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34 *state,
    const Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 *lifecycle,
    Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34 nextState;
    Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 pending;
    int i;
    if (!state || !lifecycle || !outReceipt || !lifecycle->valid) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (lifecycle->publication == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34) {
        if (lifecycle->generation <= nextState.lastClearGeneration ||
            !clear_frame_is_renderable(&lifecycle->frame)) return 0;
        for (i = 0; i < lifecycle->frame.operationCount; ++i) {
            if (!append_command(&pending, DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34,
                                &lifecycle->frame.operations[i])) return 0;
        }
        nextState.lastClearGeneration = lifecycle->generation;
        nextState.clearPublishedSinceComposition = 1;
    } else if (lifecycle->publication ==
               DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34) {
        if (!nextState.clearPublishedSinceComposition ||
            lifecycle->generation <= nextState.lastCompositionGeneration ||
            !composition_frame_is_renderable(&lifecycle->frame)) return 0;
        for (i = 0; i < lifecycle->frame.operationCount; ++i) {
            const Dm1V1ChampionTopRowAtomicFrameOpPc34 *source =
                &lifecycle->frame.operations[i];
            Dm1V1ChampionTopRowLifecycleRenderCommandKindPc34 kind =
                source->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_BLIT_C008_PC34
                ? DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34
                : source->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34
                ? DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34
                : DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34;
            if (!append_command(&pending, kind, source)) return 0;
        }
        nextState.lastCompositionGeneration = lifecycle->generation;
        nextState.clearPublishedSinceComposition = 0;
    } else {
        return 0;
    }

    pending.publication = lifecycle->publication;
    pending.generation = lifecycle->generation;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
