#include "dm1_v1_champion_final_capture_m11_gate_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static int originals_are_configured(
    const Dm1V1ChampionFinalCaptureM11GateConfigPc34 *config)
{
    const Dm1V1ChampionFinalCaptureOriginalConfigPc34 *o;
    if (!config || !config->portraitPixels) return 0;
    o = &config->originals;
    return o->c008Pixels && o->c008Width == 67 && o->c008Height == 29 &&
           o->c028Pixels && o->c028Width == 76 && o->c028Height == 14 &&
           o->c033Pixels && o->c034Pixels && o->c035Pixels &&
           o->handWidth == 18 && o->handHeight == 18 &&
           o->indexedPalette && o->indexedPaletteEntryCount >= 16 &&
           o->indexedSurface;
}

static int append_copy(Dm1V1ChampionFinalCaptureM11GateReceiptPc34 *receipt,
                       const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command)
{
    if (!receipt || !command || receipt->commandCount >=
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    receipt->commands[receipt->commandCount++] = *command;
    return 1;
}

static int append_clear(Dm1V1ChampionFinalCaptureM11GateReceiptPc34 *receipt,
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

static int command_matches_config(
    const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command,
    const Dm1V1ChampionFinalCaptureM11GateConfigPc34 *config)
{
    const Dm1V1ChampionFinalCaptureOriginalConfigPc34 *o = &config->originals;
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34)
        return command->originalPixels == o->c008Pixels && !command->portraitPixels &&
               !command->originalPalette && !command->originalSurface;
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34)
        return command->originalPixels == o->c028Pixels &&
               command->portraitPixels == config->portraitPixels &&
               !command->originalPalette && !command->originalSurface;
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34)
        return !command->originalPixels && !command->portraitPixels &&
               command->originalPalette == o->indexedPalette &&
               command->originalSurface == o->indexedSurface;
    if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34) {
        const uint8_t *expected = command->graphicIndex == DM1_GFX_SLOT_NORMAL
            ? o->c033Pixels : command->graphicIndex == DM1_GFX_SLOT_WOUNDED
            ? o->c034Pixels : command->graphicIndex == DM1_GFX_SLOT_ACTING
            ? o->c035Pixels : NULL;
        return expected && command->originalPixels == expected &&
               !command->portraitPixels && !command->originalPalette &&
               !command->originalSurface;
    }
    return 0;
}

static int clear_command_is_material_free(
    const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command)
{
    return command && command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34 &&
           !command->originalPixels && !command->portraitPixels &&
           !command->originalPalette && !command->originalSurface;
}

static int receipt_is_complete(
    const Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *evidence,
    const Dm1V1ChampionFinalCaptureM11GateConfigPc34 *config)
{
    int i;
    int c008Count = 0;
    int c028Count = 0;
    int statusCount = 0;
    int handCount = 0;
    if (!evidence || evidence->evidenceCount < 1 || evidence->evidenceCount >
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < evidence->evidenceCount; ++i) {
        const Dm1V1ChampionRuntimeSourceM11CommandPc34 *command = &evidence->evidence[i];
        if (!command_matches_config(command, config)) return 0;
        if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34) ++c008Count;
        else if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34) ++c028Count;
        else if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34) ++statusCount;
        else if (command->kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34) ++handCount;
    }
    return c008Count == 1 && c028Count == 1 && statusCount == 1 && handCount > 0;
}

static int receipt_is_clear_only(
    const Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *evidence)
{
    int i;
    if (!evidence || evidence->evidenceCount < 1 || evidence->evidenceCount >
        DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34) return 0;
    for (i = 0; i < evidence->evidenceCount; ++i)
        if (!clear_command_is_material_free(&evidence->evidence[i])) return 0;
    return 1;
}

void dm1_v1_champion_final_capture_m11_gate_init_pc34(
    Dm1V1ChampionFinalCaptureM11GateStatePc34 *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

const char *dm1_v1_champion_final_capture_m11_gate_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292/F0287/F0291 selects original C008/C028, "
           "portrait/status material and C033-C035 hand slots before F0680/F0692 "
           "capture. This final M11 admission receipt preserves that configured "
           "source boundary: unproven, stale, or partial material is clear-only.";
}

int dm1_v1_champion_final_capture_m11_gate_pc34(
    Dm1V1ChampionFinalCaptureM11GateStatePc34 *state,
    const Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *finalEvidence,
    const Dm1V1ChampionFinalCaptureM11GateConfigPc34 *config,
    Dm1V1ChampionFinalCaptureM11GateReceiptPc34 *outReceipt)
{
    Dm1V1ChampionFinalCaptureM11GateReceiptPc34 pending;
    int clearOnly;
    int i;
    if (!state || !finalEvidence || !config || !outReceipt || !finalEvidence->valid ||
        !originals_are_configured(config)) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    memset(&pending, 0, sizeof(pending));
    clearOnly = finalEvidence->clearOnly ||
                finalEvidence->tick <= state->lastTick ||
                finalEvidence->generation <= state->lastGeneration;
    if (finalEvidence->clearOnly) {
        if (!receipt_is_clear_only(finalEvidence)) return 0;
        for (i = 0; i < finalEvidence->evidenceCount; ++i)
            if (!append_copy(&pending, &finalEvidence->evidence[i])) return 0;
    } else if (!clearOnly && receipt_is_complete(finalEvidence, config)) {
        for (i = 0; i < finalEvidence->evidenceCount; ++i)
            if (!append_copy(&pending, &finalEvidence->evidence[i])) return 0;
        state->lastTick = finalEvidence->tick;
        state->lastGeneration = finalEvidence->generation;
    } else {
        for (i = 0; i < finalEvidence->evidenceCount; ++i)
            if (!append_clear(&pending, &finalEvidence->evidence[i])) return 0;
        clearOnly = 1;
    }
    if (pending.commandCount == 0) return 0;
    pending.valid = 1;
    pending.clearOnly = clearOnly;
    pending.tick = finalEvidence->tick;
    pending.generation = finalEvidence->generation;
    *outReceipt = pending;
    return 1;
}
