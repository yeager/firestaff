#include "dm1_v1_champion_top_row_runtime_frame_admission_pc34_compat.h"

#include <string.h>

static int append_command(Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *receipt,
                          const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *command,
                          int clearOnly)
{
    if (!receipt || !command || receipt->commandCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    if (clearOnly) {
        if (command->kind != DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34 ||
            command->source.sourcePixels || command->source.portraitPixels ||
            command->source.statusBar.originalPalette ||
            command->source.statusBar.originalIndexedSurface) return 0;
    }
    receipt->commands[receipt->commandCount++] = *command;
    return 1;
}

static int atomic_is_clear(const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame)
{
    int i;
    if (!frame || !frame->valid || !frame->clearOnly || frame->originalMaterialsPublished ||
        frame->operationCount <= 0) return 0;
    for (i = 0; i < frame->operationCount; ++i) {
        if (frame->operations[i].operation != DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34)
            return 0;
    }
    return 1;
}

static int atomic_op_matches_host(const Dm1V1ChampionTopRowAtomicFrameOpPc34 *atomic,
                                  const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *host)
{
    if (atomic->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_BLIT_C008_PC34)
        return host->kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_BLIT_C008_PC34 &&
               atomic->sourcePixels && atomic->sourcePixels == host->source.sourcePixels;
    if (atomic->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34)
        return host->kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34 &&
               atomic->sourcePixels && atomic->portraitPixels &&
               atomic->sourcePixels == host->source.sourcePixels &&
               atomic->portraitPixels == host->source.portraitPixels;
    if (atomic->operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34)
        return host->kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34 &&
               atomic->statusBar.originalPalette && atomic->statusBar.originalIndexedSurface &&
               atomic->statusBar.originalPalette == host->source.statusBar.originalPalette &&
               atomic->statusBar.originalIndexedSurface == host->source.statusBar.originalIndexedSurface &&
               atomic->statusBar.zoneId == host->source.statusBar.zoneId;
    return 0;
}

static int composition_matches_atomic(
    const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *atomic,
    const Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *host)
{
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!atomic || !host || !atomic->valid || atomic->clearOnly ||
        !atomic->originalMaterialsPublished || !host->valid || host->clearOnly ||
        atomic->operationCount <= 0 || host->commandCount != atomic->operationCount) return 0;
    for (i = 0; i < atomic->operationCount; ++i) {
        if (!atomic_op_matches_host(&atomic->operations[i], &host->commands[i])) return 0;
        if (atomic->operations[i].operation == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34)
            ++barCount;
        else
            ++sourceCount;
    }
    return sourceCount > 0 && barCount > 0;
}

void dm1_v1_champion_top_row_runtime_frame_admission_init_pc34(
    Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_runtime_frame_admission_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292 and F0287 build one ordered top-row "
           "composition from C008/C028/statusbar sources. F0680/F0692 accepts "
           "only a coherent completed frame; stale paths clear their regions "
           "without carrying original material into runtime presentation.";
}

int dm1_v1_champion_top_row_runtime_frame_admission_pc34(
    Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34 *state,
    const Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *atomicFrame,
    const Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 *hostLifecycle,
    Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34 nextState;
    Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 pending;
    const Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *host;
    int i;
    if (!state || !atomicFrame || !hostLifecycle || !outReceipt || !hostLifecycle->valid ||
        hostLifecycle->tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;
    host = &hostLifecycle->hostRender;

    if (hostLifecycle->publication == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_CLEAR_PC34) {
        if (!atomic_is_clear(atomicFrame) || !host->clearOnly ||
            hostLifecycle->generation <= nextState.pendingClearGeneration) return 0;
        for (i = 0; i < host->commandCount; ++i)
            if (!append_command(&pending, &host->commands[i], 1)) return 0;
        nextState.pendingClearGeneration = hostLifecycle->generation;
        pending.kind = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_CLEAR_PC34;
        pending.clearOnly = 1;
    } else if (hostLifecycle->publication ==
               DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_STALE_CLEAR_PC34) {
        if (!host->clearOnly) return 0;
        for (i = 0; i < host->commandCount; ++i)
            if (!append_command(&pending, &host->commands[i], 1)) return 0;
        pending.kind = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_CLEAR_PC34;
        pending.clearOnly = 1;
    } else if (hostLifecycle->publication ==
               DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_COMPOSITION_PC34) {
        if (hostLifecycle->generation != nextState.pendingClearGeneration ||
            hostLifecycle->generation <= nextState.lastCompositionGeneration ||
            !composition_matches_atomic(atomicFrame, host)) return 0;
        for (i = 0; i < host->commandCount; ++i)
            if (!append_command(&pending, &host->commands[i], 0)) return 0;
        nextState.lastCompositionGeneration = hostLifecycle->generation;
        nextState.pendingClearGeneration = 0;
        pending.kind = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_COMPOSITION_PC34;
    } else {
        return 0;
    }
    nextState.lastTick = hostLifecycle->tick;
    pending.tick = hostLifecycle->tick;
    pending.generation = hostLifecycle->generation;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
