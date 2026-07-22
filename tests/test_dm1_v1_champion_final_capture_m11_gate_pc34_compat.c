#include "dm1_v1_champion_final_capture_m11_gate_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t c008[67 * 29] = { 1 };
static const uint8_t c028[76 * 14] = { 2 };
static const uint8_t c033[18 * 18] = { 3 };
static const uint8_t c034[18 * 18] = { 4 };
static const uint8_t c035[18 * 18] = { 5 };
static const uint8_t portrait[32 * 29] = { 6 };
static const uint8_t palette[16 * 3] = { 7 };
static uint8_t surface[320 * 200];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void configured(Dm1V1ChampionFinalCaptureM11GateConfigPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->originals.c008Pixels = c008; out->originals.c008Width = 67; out->originals.c008Height = 29;
    out->originals.c028Pixels = c028; out->originals.c028Width = 76; out->originals.c028Height = 14;
    out->originals.c033Pixels = c033; out->originals.c034Pixels = c034; out->originals.c035Pixels = c035;
    out->originals.handWidth = 18; out->originals.handHeight = 18;
    out->originals.indexedPalette = palette; out->originals.indexedPaletteEntryCount = 16;
    out->originals.indexedSurface = surface; out->portraitPixels = portrait;
}

static void evidence(Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *out,
                     unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation; out->evidenceCount = 4;
    out->evidence[0].kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34;
    out->evidence[0].originalPixels = c008;
    out->evidence[1].kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34;
    out->evidence[1].originalPixels = c028; out->evidence[1].portraitPixels = portrait;
    out->evidence[2].kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34;
    out->evidence[2].originalPalette = palette; out->evidence[2].originalSurface = surface;
    out->evidence[3].kind = DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34;
    out->evidence[3].graphicIndex = DM1_GFX_SLOT_ACTING; out->evidence[3].originalPixels = c035;
}

int main(void)
{
    Dm1V1ChampionFinalCaptureM11GateStatePc34 state;
    Dm1V1ChampionFinalCaptureM11GateConfigPc34 config;
    Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 input;
    Dm1V1ChampionFinalCaptureM11GateReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_final_capture_m11_gate_init_pc34(&state);
    configured(&config); evidence(&input, 1, 1);
    ok &= check("exact configured originals reach M11 capture gate",
                dm1_v1_champion_final_capture_m11_gate_pc34(&state, &input, &config, &receipt) &&
                !receipt.clearOnly && receipt.commandCount == 4 &&
                receipt.commands[1].portraitPixels == portrait);
    ok &= check("repeated evidence is clear only",
                dm1_v1_champion_final_capture_m11_gate_pc34(&state, &input, &config, &receipt) &&
                receipt.clearOnly && receipt.commands[0].kind ==
                    DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34 &&
                !receipt.commands[0].originalPixels);
    evidence(&input, 2, 2); input.evidence[1].portraitPixels = c028;
    ok &= check("unconfigured portrait is clear only",
                dm1_v1_champion_final_capture_m11_gate_pc34(&state, &input, &config, &receipt) &&
                receipt.clearOnly && !receipt.commands[1].portraitPixels);
    evidence(&input, 3, 3); input.evidenceCount = 3;
    ok &= check("missing configured hand proof is clear only",
                dm1_v1_champion_final_capture_m11_gate_pc34(&state, &input, &config, &receipt) &&
                receipt.clearOnly && receipt.commands[2].kind ==
                    DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34);
    return ok ? 0 : 1;
}
