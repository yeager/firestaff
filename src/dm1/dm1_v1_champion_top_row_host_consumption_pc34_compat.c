#include "dm1_v1_champion_top_row_host_consumption_pc34_compat.h"

#include <string.h>

static int clear_commands_are_valid(const Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *receipt)
{
    int i;
    if (!receipt || receipt->commandCount <= 0 || receipt->commandCount >
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < receipt->commandCount; ++i) {
        if (receipt->commands[i].kind != DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34 ||
            receipt->commands[i].source.operation !=
                DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34) return 0;
    }
    return 1;
}

static int composition_commands_are_fresh(
    const Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *receipt)
{
    int i;
    int materialCommands = 0;
    int statusCommands = 0;
    if (!receipt || receipt->commandCount <= 0 || receipt->commandCount >
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < receipt->commandCount; ++i) {
        const Dm1V1ChampionTopRowLifecycleRenderCommandPc34 *command =
            &receipt->commands[i];
        if (command->kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_BLIT_C008_PC34) {
            if (command->source.operation != DM1_V1_CHAMPION_TOP_ROW_ATOMIC_BLIT_C008_PC34 ||
                !command->source.sourcePixels) return 0;
            ++materialCommands;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34) {
            if (command->source.operation != DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34 ||
                !command->source.sourcePixels || !command->source.portraitPixels) return 0;
            ++materialCommands;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34) {
            if (command->source.operation != DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34 ||
                !command->source.statusBar.originalPalette ||
                !command->source.statusBar.originalIndexedSurface) return 0;
            ++statusCommands;
        } else {
            return 0;
        }
    }
    return materialCommands > 0 && statusCommands > 0;
}

void dm1_v1_champion_top_row_host_consumption_init_pc34(
    Dm1V1ChampionTopRowHostConsumptionStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_host_consumption_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 produces one ordered status frame, "
           "and F0680/F0692 presents retained logical-screen material only after "
           "the preceding clear/update phase. Generation matching prevents a stale "
           "C008/C028 or palette/surface composition from reaching the host.";
}

int dm1_v1_champion_top_row_host_consumption_pc34(
    Dm1V1ChampionTopRowHostConsumptionStatePc34 *state,
    const Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *renderReceipt,
    Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowHostConsumptionStatePc34 nextState;
    Dm1V1ChampionTopRowHostConsumptionReceiptPc34 pending;
    int i;
    if (!state || !renderReceipt || !outReceipt || !renderReceipt->valid) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    nextState = *state;
    memset(&pending, 0, sizeof(pending));

    if (renderReceipt->publication == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34) {
        if (renderReceipt->generation <= nextState.lastClearGeneration ||
            !clear_commands_are_valid(renderReceipt)) return 0;
        nextState.lastClearGeneration = renderReceipt->generation;
        nextState.pendingClearGeneration = renderReceipt->generation;
    } else if (renderReceipt->publication ==
               DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34) {
        if (renderReceipt->generation <= nextState.lastCompositionGeneration ||
            renderReceipt->generation != nextState.pendingClearGeneration ||
            !composition_commands_are_fresh(renderReceipt)) return 0;
        nextState.lastCompositionGeneration = renderReceipt->generation;
        nextState.pendingClearGeneration = 0;
    } else {
        return 0;
    }

    for (i = 0; i < renderReceipt->commandCount; ++i) {
        pending.operations[pending.operationCount].kind = renderReceipt->commands[i].kind;
        pending.operations[pending.operationCount].source = renderReceipt->commands[i].source;
        ++pending.operationCount;
    }
    pending.publication = renderReceipt->publication;
    pending.generation = renderReceipt->generation;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
