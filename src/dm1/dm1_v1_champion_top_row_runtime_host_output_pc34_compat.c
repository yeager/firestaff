#include "dm1_v1_champion_top_row_runtime_host_output_pc34_compat.h"

#include <string.h>

static int append_command(Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *receipt,
                          const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *command,
                          int clearOnly)
{
    if (!receipt || !command || receipt->commandCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    if (clearOnly && (command->kind != DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34 ||
                      command->source.sourcePixels || command->source.portraitPixels ||
                      command->source.statusBar.originalPalette ||
                      command->source.statusBar.originalIndexedSurface)) return 0;
    receipt->commands[receipt->commandCount++] = *command;
    return 1;
}

static int clear_admission_is_valid(const Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *admission)
{
    int i;
    if (!admission || !admission->valid || !admission->clearOnly ||
        admission->kind != DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_CLEAR_PC34 ||
        admission->commandCount <= 0) return 0;
    for (i = 0; i < admission->commandCount; ++i) {
        if (admission->commands[i].kind != DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34)
            return 0;
    }
    return 1;
}

static int composition_admission_is_valid(
    const Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *admission)
{
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!admission || !admission->valid || admission->clearOnly ||
        admission->kind != DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_COMPOSITION_PC34 ||
        admission->commandCount <= 0) return 0;
    for (i = 0; i < admission->commandCount; ++i) {
        const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *command =
            &admission->commands[i];
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

void dm1_v1_champion_top_row_runtime_host_output_init_pc34(
    Dm1V1ChampionTopRowRuntimeHostOutputStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_runtime_host_output_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 presents one top-row state per "
           "logical-screen generation. A later clear invalidates that state before "
           "the host can retain it; F0680/F0692 therefore receives clear-only "
           "output until the next complete generation is admitted.";
}

int dm1_v1_champion_top_row_runtime_host_output_step_pc34(
    Dm1V1ChampionTopRowRuntimeHostOutputStatePc34 *state,
    const Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *admission,
    Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowRuntimeHostOutputStatePc34 nextState;
    Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 pending;
    int i;
    if (!state || !admission || !outReceipt || !admission->valid ||
        admission->tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (admission->clearOnly) {
        if (!clear_admission_is_valid(admission)) return 0;
        for (i = 0; i < admission->commandCount; ++i)
            if (!append_command(&pending, &admission->commands[i], 1)) return 0;
        if (admission->generation > nextState.lastClearGeneration) {
            nextState.lastClearGeneration = admission->generation;
            nextState.pendingClearGeneration = admission->generation;
        }
        pending.action = nextState.hostCompositionActive
            ? DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_REVOKE_CLEAR_PC34
            : DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_CLEAR_PC34;
        nextState.hostCompositionActive = 0;
        pending.clearOnly = 1;
    } else {
        if (!composition_admission_is_valid(admission) ||
            admission->generation != nextState.pendingClearGeneration ||
            admission->generation <= nextState.lastCompositionGeneration) return 0;
        for (i = 0; i < admission->commandCount; ++i)
            if (!append_command(&pending, &admission->commands[i], 0)) return 0;
        nextState.lastCompositionGeneration = admission->generation;
        nextState.pendingClearGeneration = 0;
        nextState.hostCompositionActive = 1;
        pending.action = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34;
    }
    nextState.lastTick = admission->tick;
    pending.tick = admission->tick;
    pending.generation = admission->generation;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
