#include "dm1_v1_champion_top_row_m11_host_render_lifecycle_pc34_compat.h"

#include <string.h>

static int clear_only_is_valid(const Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *receipt)
{
    int i;
    if (!receipt || !receipt->valid || !receipt->clearOnly || receipt->commandCount <= 0 ||
        receipt->commandCount > DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < receipt->commandCount; ++i) {
        if (receipt->commands[i].kind != DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34 ||
            receipt->commands[i].source.sourcePixels ||
            receipt->commands[i].source.portraitPixels) return 0;
    }
    return 1;
}

static int composition_is_valid(const Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *receipt)
{
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!receipt || !receipt->valid || receipt->clearOnly || receipt->commandCount <= 0 ||
        receipt->commandCount > DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < receipt->commandCount; ++i) {
        const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *command =
            &receipt->commands[i];
        if (command->kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_BLIT_C008_PC34) {
            if (!command->source.sourcePixels) return 0;
            ++sourceCount;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34) {
            if (!command->source.sourcePixels || !command->source.portraitPixels) return 0;
            ++sourceCount;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34) {
            if (!command->source.statusBar.originalPalette ||
                !command->source.statusBar.originalIndexedSurface) return 0;
            ++barCount;
        } else {
            return 0;
        }
    }
    return sourceCount > 0 && barCount > 0;
}

void dm1_v1_champion_top_row_m11_host_render_lifecycle_init_pc34(
    Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_m11_host_render_lifecycle_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 separates ordered redraw phases, "
           "and F0680/F0692 publishes only current completed material. A stale "
           "generation at the host-render boundary is therefore clear-only; it "
           "cannot retain any C008/C028/statusbar source pointers.";
}

int dm1_v1_champion_top_row_m11_host_render_lifecycle_step_pc34(
    Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34 *state,
    const Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *hostRender,
    Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34 nextState;
    Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 pending;
    if (!state || !hostRender || !outReceipt || !hostRender->valid ||
        hostRender->tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (hostRender->clearOnly) {
        if (!clear_only_is_valid(hostRender)) return 0;
        if (hostRender->generation > nextState.lastClearGeneration) {
            nextState.lastClearGeneration = hostRender->generation;
            nextState.pendingClearGeneration = hostRender->generation;
            pending.publication = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_CLEAR_PC34;
        } else {
            pending.publication = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_STALE_CLEAR_PC34;
        }
    } else {
        if (!composition_is_valid(hostRender) ||
            hostRender->generation != nextState.pendingClearGeneration ||
            hostRender->generation <= nextState.lastCompositionGeneration) return 0;
        nextState.lastCompositionGeneration = hostRender->generation;
        nextState.pendingClearGeneration = 0;
        pending.publication = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_COMPOSITION_PC34;
    }

    nextState.lastTick = hostRender->tick;
    pending.tick = hostRender->tick;
    pending.generation = hostRender->generation;
    pending.hostRender = *hostRender;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
