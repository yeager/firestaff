#include "dm1_v1_champion_top_row_runtime_m11_bridge_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static int proof_is_exact(const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *proof)
{
    return proof && proof->c008Original && proof->c008Pixels &&
           proof->c008GraphicIndex == DM1_GFX_DEAD_CHAMPION && proof->c008Width == 67 &&
           proof->c008Height == 29 && proof->c028Original && proof->c028Pixels &&
           proof->c028GraphicIndex == DM1_GFX_CHAMPION_ICONS && proof->c028Width == 76 &&
           proof->c028Height == 14 && proof->indexedPaletteOriginal && proof->indexedPalette &&
           proof->indexedPaletteEntryCount >= 16 && proof->indexedSurfaceOriginal &&
           proof->indexedSurface;
}

static int append_command(Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 *receipt,
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

static int full_commands_match_proof(
    const Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *host,
    const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *proof)
{
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!host || host->clearOnly || !proof_is_exact(proof) || host->commandCount <= 0)
        return 0;
    for (i = 0; i < host->commandCount; ++i) {
        const Dm1V1ChampionTopRowM11HostRenderCommandPc34 *command = &host->commands[i];
        if (command->kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_BLIT_C008_PC34) {
            if (command->source.sourcePixels != proof->c008Pixels) return 0;
            ++sourceCount;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34) {
            if (command->source.sourcePixels != proof->c028Pixels ||
                !command->source.portraitPixels) return 0;
            ++sourceCount;
        } else if (command->kind ==
                   DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34) {
            if (command->source.statusBar.originalPalette != proof->indexedPalette ||
                command->source.statusBar.originalIndexedSurface != proof->indexedSurface) return 0;
            ++barCount;
        } else {
            return 0;
        }
    }
    return sourceCount > 0 && barCount > 0;
}

void dm1_v1_champion_top_row_runtime_m11_bridge_init_pc34(
    Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_top_row_runtime_m11_bridge_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287 admits C008/C028 and statusbar "
           "material as one frame, while F0680/F0692 presents only the current "
           "logical screen. The bridge preserves complete source proof for publish "
           "and keeps clear/revoke output free of original render material.";
}

int dm1_v1_champion_top_row_runtime_m11_bridge_pc34(
    Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34 *state,
    const Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *hostOutput,
    const Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *originalMaterials,
    Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 *outReceipt)
{
    Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34 nextState;
    Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 pending;
    int i;
    if (!state || !hostOutput || !outReceipt || !hostOutput->valid ||
        hostOutput->tick <= state->lastTick) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    nextState = *state;

    if (hostOutput->clearOnly) {
        if (hostOutput->action != DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_CLEAR_PC34 &&
            hostOutput->action != DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_REVOKE_CLEAR_PC34)
            return 0;
        for (i = 0; i < hostOutput->commandCount; ++i)
            if (!append_command(&pending, &hostOutput->commands[i], 1)) return 0;
        if (pending.commandCount == 0) return 0;
        if (hostOutput->generation > nextState.pendingClearGeneration)
            nextState.pendingClearGeneration = hostOutput->generation;
        nextState.m11CompositionActive = 0;
        pending.clearOnly = 1;
        pending.action = hostOutput->action;
    } else {
        if (hostOutput->action != DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34 ||
            hostOutput->generation != nextState.pendingClearGeneration ||
            hostOutput->generation <= nextState.lastCompositionGeneration ||
            !full_commands_match_proof(hostOutput, originalMaterials)) return 0;
        for (i = 0; i < hostOutput->commandCount; ++i)
            if (!append_command(&pending, &hostOutput->commands[i], 0)) return 0;
        nextState.lastCompositionGeneration = hostOutput->generation;
        nextState.pendingClearGeneration = 0;
        nextState.m11CompositionActive = 1;
        pending.action = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34;
        pending.originalMaterials = *originalMaterials;
    }
    nextState.lastTick = hostOutput->tick;
    pending.tick = hostOutput->tick;
    pending.generation = hostOutput->generation;
    pending.valid = 1;
    *state = nextState;
    *outReceipt = pending;
    return 1;
}
