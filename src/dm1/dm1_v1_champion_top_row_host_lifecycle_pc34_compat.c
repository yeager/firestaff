#include "dm1_v1_champion_top_row_host_lifecycle_pc34_compat.h"

#include <string.h>

static int clear_host_receipt_is_valid(
    const Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *receipt)
{
    int i;
    if (!receipt || !receipt->valid || receipt->operationCount <= 0 ||
        receipt->operationCount > DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34)
        return 0;
    for (i = 0; i < receipt->operationCount; ++i) {
        if (receipt->operations[i].kind !=
                DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34 ||
            receipt->operations[i].source.operation !=
                DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34) return 0;
    }
    return 1;
}

static int composition_host_receipt_is_valid(
    const Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *receipt)
{
    int i;
    int sourceCount = 0;
    int statusCount = 0;
    if (!receipt || !receipt->valid || receipt->operationCount <= 0 ||
        receipt->operationCount > DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34)
        return 0;
    for (i = 0; i < receipt->operationCount; ++i) {
        const Dm1V1ChampionTopRowHostConsumptionOpPc34 *op = &receipt->operations[i];
        if (op->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34) {
            if (!op->source.sourcePixels) return 0;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34) {
            if (!op->source.sourcePixels || !op->source.portraitPixels) return 0;
            ++sourceCount;
        } else if (op->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34) {
            if (!op->source.statusBar.originalPalette ||
                !op->source.statusBar.originalIndexedSurface) return 0;
            ++statusCount;
        } else {
            return 0;
        }
    }
    return sourceCount > 0 && statusCount > 0;
}

void dm1_v1_champion_top_row_host_lifecycle_init_pc34(
    Dm1V1ChampionTopRowHostLifecycleStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_host_lifecycle_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 updates the top row in frame order; "
           "F0680/F0692 consume the completed logical screen on a later present "
           "step. Once a new clear generation is consumed, an older retained "
           "C008/C028/statusbar composition is stale and cannot be presented.";
}

int dm1_v1_champion_top_row_host_lifecycle_step_pc34(
    Dm1V1ChampionTopRowHostLifecycleStatePc34 *state,
    unsigned int tick,
    const Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *hostConsumption,
    Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowHostLifecycleStatePc34 nextState;
    Dm1V1ChampionTopRowHostLifecycleReceiptPc34 pending;
    if (!state || !hostConsumption || !outReceipt || tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (hostConsumption->publication == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34) {
        if (hostConsumption->generation <= nextState.lastClearGeneration ||
            !clear_host_receipt_is_valid(hostConsumption)) return 0;
        nextState.lastClearGeneration = hostConsumption->generation;
        nextState.pendingClearGeneration = hostConsumption->generation;
    } else if (hostConsumption->publication ==
               DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34) {
        if (hostConsumption->generation <= nextState.lastCompositionGeneration ||
            hostConsumption->generation != nextState.pendingClearGeneration ||
            !composition_host_receipt_is_valid(hostConsumption)) return 0;
        nextState.lastCompositionGeneration = hostConsumption->generation;
        nextState.pendingClearGeneration = 0;
    } else {
        return 0;
    }

    nextState.lastTick = tick;
    pending.tick = tick;
    pending.publication = hostConsumption->publication;
    pending.generation = hostConsumption->generation;
    pending.hostConsumption = *hostConsumption;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
