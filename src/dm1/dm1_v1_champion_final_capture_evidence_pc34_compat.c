#include "dm1_v1_champion_final_capture_evidence_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static int config_is_original(const Dm1V1ChampionFinalCaptureOriginalConfigPc34 *config)
{
    return config && config->c008Pixels && config->c008Width == 67 && config->c008Height == 29 &&
           config->c028Pixels && config->c028Width == 76 && config->c028Height == 14 &&
           config->c033Pixels && config->c034Pixels && config->c035Pixels &&
           config->handWidth == 18 && config->handHeight == 18 &&
           config->indexedPalette && config->indexedPaletteEntryCount >= 16 &&
           config->indexedSurface;
}

static int append_copy(Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *receipt,
                       const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command)
{
    if (!receipt || !command || receipt->evidenceCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    receipt->evidence[receipt->evidenceCount++] = *command;
    return 1;
}

static int append_clear(Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *receipt,
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

static int command_matches_config(const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command,
                                  const Dm1V1ChampionFinalCaptureOriginalConfigPc34 *config)
{
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34)
        return command->originalPixels == config->c008Pixels;
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34)
        return command->originalPixels == config->c028Pixels && command->portraitPixels;
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34)
        return command->originalPalette == config->indexedPalette &&
               command->originalSurface == config->indexedSurface;
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34) {
        const uint8_t *expected = command->graphicIndex == DM1_GFX_SLOT_NORMAL
            ? config->c033Pixels : command->graphicIndex == DM1_GFX_SLOT_WOUNDED
            ? config->c034Pixels : command->graphicIndex == DM1_GFX_SLOT_ACTING
            ? config->c035Pixels : NULL;
        return expected && command->originalPixels == expected;
    }
    return 0;
}

void dm1_v1_champion_final_capture_evidence_init_pc34(
    Dm1V1ChampionFinalCaptureEvidenceStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_final_capture_evidence_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287/F0291 uses configured original "
           "C008/C028/C033-C035, portrait, palette, and logical-screen material. "
           "F0680/F0692 final capture can expose only that coherent source set; "
           "stale frames are clear-only and synthetic fallback is excluded.";
}

int dm1_v1_champion_final_capture_evidence_pc34(
    Dm1V1ChampionFinalCaptureEvidenceStatePc34 *state,
    const Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 *lifecycle,
    const Dm1V1ChampionFinalCaptureOriginalConfigPc34 *originalConfig,
    Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *outReceipt)
{
    Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 pending;
    int stale;
    int i;
    int sourceCount = 0;
    int barCount = 0;
    if (!state || !lifecycle || !outReceipt || !lifecycle->valid ||
        !config_is_original(originalConfig)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    stale = lifecycle->clearOnly || lifecycle->tick <= state->lastTick ||
            lifecycle->generation <= state->lastGeneration;
    if (!stale) {
        for (i = 0; i < lifecycle->commandCount; ++i) {
            const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command =
                &lifecycle->commands[i];
            if (!command_matches_config(command, originalConfig)) {
                stale = 1;
                break;
            }
            if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34)
                ++barCount;
            else if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34 ||
                     command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34)
                ++sourceCount;
        }
        if (sourceCount == 0 || barCount == 0) stale = 1;
    }
    if (stale) {
        for (i = 0; i < lifecycle->commandCount; ++i)
            if (!append_clear(&pending, &lifecycle->commands[i])) return 0;
        pending.clearOnly = 1;
    } else {
        for (i = 0; i < lifecycle->commandCount; ++i)
            if (!append_copy(&pending, &lifecycle->commands[i])) return 0;
        state->lastTick = lifecycle->tick;
        state->lastGeneration = lifecycle->generation;
    }
    if (pending.evidenceCount == 0) return 0;
    pending.tick = lifecycle->tick;
    pending.generation = lifecycle->generation;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
