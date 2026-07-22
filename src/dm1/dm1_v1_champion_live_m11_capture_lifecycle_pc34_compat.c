#include "dm1_v1_champion_live_m11_capture_lifecycle_pc34_compat.h"

#include <string.h>

static int append_copy(Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 *receipt,
                       const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command)
{
    if (!receipt || !command || receipt->commandCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    receipt->commands[receipt->commandCount++] = *command;
    return 1;
}

static int append_clear(Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 *receipt,
                        const Dm1V1ChampionRuntimeSourceM11CommandPc34 *source)
{
    Dm1V1ChampionRuntimeSourceM11CommandPc34 clear;
    if (!source) return 0;
    memset(&clear, 0, sizeof(clear));
    clear.kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34;
    clear.championIndex = source->championIndex;
    clear.zoneId = source->zoneId;
    return append_copy(receipt, &clear);
}

static int bridge_matches_capture(const Dm1V1ChampionLiveM11BridgeReceiptPc34 *bridge,
                                  const Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *capture)
{
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!bridge || !capture || !bridge->valid || bridge->clearOnly || !capture->valid ||
        bridge->m11.tick != capture->tick || bridge->m11.generation != capture->generation)
        return 0;
    for (i = 0; i < bridge->m11.commandCount; ++i) {
        const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command = &bridge->m11.commands[i];
        if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34) {
            if (command->originalPixels != capture->originalMaterials.c008Pixels) return 0;
            ++sourceCount;
        } else if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34) {
            if (command->originalPixels != capture->originalMaterials.c028Pixels ||
                !command->portraitPixels) return 0;
            ++sourceCount;
        } else if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34) {
            if (command->originalPalette != capture->originalMaterials.indexedPalette ||
                command->originalSurface != capture->originalMaterials.indexedSurface) return 0;
            ++barCount;
        } else if (command->kind != DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34) {
            return 0;
        }
    }
    return sourceCount > 0 && barCount > 0;
}

static int bridge_clear_only_is_valid(const Dm1V1ChampionLiveM11BridgeReceiptPc34 *bridge)
{
    int i;
    if (!bridge || !bridge->valid || !bridge->clearOnly || bridge->m11.commandCount <= 0)
        return 0;
    for (i = 0; i < bridge->m11.commandCount; ++i) {
        const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command = &bridge->m11.commands[i];
        if (command->kind != DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34 ||
            command->originalPixels || command->portraitPixels ||
            command->originalPalette || command->originalSurface) return 0;
    }
    return 1;
}

void dm1_v1_champion_live_m11_capture_lifecycle_init_pc34(
    Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_live_m11_capture_lifecycle_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287/F0291 binds source material to the "
           "current frame, while F0680/F0692 exposes only a coherent capture. "
           "Across ticks, stale capture proof resolves to clear-only zones rather "
           "than permitting old C008/C028/portrait/statusbar/hand material.";
}

int dm1_v1_champion_live_m11_capture_lifecycle_step_pc34(
    Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 *state,
    const Dm1V1ChampionLiveM11BridgeReceiptPc34 *liveBridge,
    const Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *captureEvidence,
    Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 *outReceipt)
{
    Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 pending;
    int stale;
    int i;
    if (!state || !liveBridge || !captureEvidence || !outReceipt || !liveBridge->valid)
        return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    if (liveBridge->clearOnly) {
        if (!bridge_clear_only_is_valid(liveBridge)) return 0;
        for (i = 0; i < liveBridge->m11.commandCount; ++i)
            if (!append_copy(&pending, &liveBridge->m11.commands[i])) return 0;
        pending.clearOnly = 1;
    } else {
        stale = liveBridge->m11.tick <= state->lastTick ||
                liveBridge->m11.generation <= state->lastGeneration ||
                !bridge_matches_capture(liveBridge, captureEvidence);
        if (stale) {
            for (i = 0; i < liveBridge->m11.commandCount; ++i)
                if (!append_clear(&pending, &liveBridge->m11.commands[i])) return 0;
            pending.clearOnly = 1;
        } else {
            for (i = 0; i < liveBridge->m11.commandCount; ++i)
                if (!append_copy(&pending, &liveBridge->m11.commands[i])) return 0;
            state->lastTick = liveBridge->m11.tick;
            state->lastGeneration = liveBridge->m11.generation;
        }
    }
    pending.tick = liveBridge->m11.tick;
    pending.generation = liveBridge->m11.generation;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
