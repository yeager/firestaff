#include "dm1_v1_champion_top_row_atomic_lifecycle_pc34_compat.h"

#include <string.h>

static int frame_is_clear_only(const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame)
{
    int i;
    if (!frame || !frame->valid || !frame->clearOnly || frame->originalMaterialsPublished)
        return 0;
    for (i = 0; i < frame->operationCount; ++i) {
        if (frame->operations[i].operation != DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34)
            return 0;
    }
    return 1;
}

static int frame_is_complete_composition(
    const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame)
{
    int i;
    int hasPortraitOrDead = 0;
    int hasStatusBar = 0;
    if (!frame || !frame->valid || frame->clearOnly || !frame->originalMaterialsPublished ||
        frame->operationCount <= 0 ||
        frame->operationCount > DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < frame->operationCount; ++i) {
        const Dm1V1ChampionTopRowAtomicFrameOpPc34 *op = &frame->operations[i];
        if (op->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34 ||
            op->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_BLIT_C008_PC34) {
            if (!op->sourcePixels || (op->operation ==
                DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34 && !op->portraitPixels)) {
                return 0;
            }
            hasPortraitOrDead = 1;
        } else if (op->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34) {
            if (!op->statusBar.originalPalette || !op->statusBar.originalIndexedSurface ||
                op->statusBar.width <= 0 || op->statusBar.height <= 0) return 0;
            hasStatusBar = 1;
        } else {
            return 0;
        }
    }
    return hasPortraitOrDead && hasStatusBar;
}

void dm1_v1_champion_top_row_atomic_lifecycle_init_pc34(
    Dm1V1ChampionTopRowAtomicLifecycleStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_atomic_lifecycle_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 redraws the status top row as an "
           "ordered pass, while F0680/F0692 expose only complete logical-screen "
           "updates. A clear pass therefore precedes the next retained-material "
           "composition; no intermediate mixed frame is publishable.";
}

int dm1_v1_champion_top_row_atomic_lifecycle_step_pc34(
    Dm1V1ChampionTopRowAtomicLifecycleStatePc34 *state,
    const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame,
    Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowAtomicLifecycleStatePc34 nextState;
    Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 pending;
    if (!state || !outReceipt || !frame) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    nextState = *state;
    if (!nextState.initialized) nextState.initialized = 1;
    memset(&pending, 0, sizeof(pending));

    if (frame_is_clear_only(frame)) {
        ++nextState.clearGeneration;
        nextState.clearPublishedSinceComposition = 1;
        pending.publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34;
        pending.generation = nextState.clearGeneration;
    } else if (frame_is_complete_composition(frame)) {
        if (!nextState.clearPublishedSinceComposition) return 0;
        ++nextState.compositionGeneration;
        nextState.clearPublishedSinceComposition = 0;
        pending.publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34;
        pending.generation = nextState.compositionGeneration;
    } else {
        return 0;
    }

    pending.frame = *frame;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
