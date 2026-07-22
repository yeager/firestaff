#include "dm1_v1_champion_final_capture_evidence_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t c008[67 * 29] = { 1 };
static const uint8_t c028[76 * 14] = { 1 };
static const uint8_t c033[18 * 18] = { 1 };
static const uint8_t c034[18 * 18] = { 1 };
static const uint8_t c035[18 * 18] = { 1 };
static const uint8_t palette[16 * 3] = { 1 };
static uint8_t surface[320 * 200];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void config(Dm1V1ChampionFinalCaptureOriginalConfigPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->c008Pixels = c008; out->c008Width = 67; out->c008Height = 29;
    out->c028Pixels = c028; out->c028Width = 76; out->c028Height = 14;
    out->c033Pixels = c033; out->c034Pixels = c034; out->c035Pixels = c035;
    out->handWidth = out->handHeight = 18;
    out->indexedPalette = palette; out->indexedPaletteEntryCount = 16; out->indexedSurface = surface;
}

static void lifecycle(Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 *out,
                      unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->commandCount = 3;
    out->commands[0].kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34;
    out->commands[0].originalPixels = c028; out->commands[0].portraitPixels = c028;
    out->commands[1].kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34;
    out->commands[1].originalPalette = palette; out->commands[1].originalSurface = surface;
    out->commands[2].kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34;
    out->commands[2].graphicIndex = DM1_GFX_SLOT_ACTING; out->commands[2].originalPixels = c035;
}

int main(void)
{
    Dm1V1ChampionFinalCaptureEvidenceStatePc34 state;
    Dm1V1ChampionFinalCaptureOriginalConfigPc34 original;
    Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 live;
    Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_final_capture_evidence_init_pc34(&state);
    config(&original); lifecycle(&live, 1, 1);
    ok &= check("configured originals publish final evidence", dm1_v1_champion_final_capture_evidence_pc34(
        &state, &live, &original, &receipt) && !receipt.clearOnly && receipt.evidenceCount == 3 &&
        receipt.evidence[2].originalPixels == c035);
    ok &= check("repeated capture becomes clear only", dm1_v1_champion_final_capture_evidence_pc34(
        &state, &live, &original, &receipt) && receipt.clearOnly &&
        receipt.evidence[0].kind == DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34 &&
        receipt.evidence[0].originalPixels == NULL);
    lifecycle(&live, 2, 2);
    live.commands[2].originalPixels = c034;
    ok &= check("wrong configured hand source clears only", dm1_v1_champion_final_capture_evidence_pc34(
        &state, &live, &original, &receipt) && receipt.clearOnly && receipt.evidence[2].originalPixels == NULL);
    return ok ? 0 : 1;
}
