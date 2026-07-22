#include "dm1_v1_champion_live_runtime_composition_evidence_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t c008[67 * 29] = { 1 };
static const uint8_t c028[76 * 14] = { 1 };
static const uint8_t palette[16 * 3] = { 1 };
static uint8_t surface[320 * 200];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void party_data(struct PartyState_Compat *party)
{
    memset(party, 0, sizeof(*party));
    party->championCount = 2;
    party->champions[0].present = party->champions[1].present = 1;
    party->champions[0].hp.current = 100;
    party->champions[0].portraitBitmapValid = 1;
}

static void assets(Dm1V1ChampionTopRowAssetsReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->c008Accepted = out->c028Accepted = 1;
    out->assets.deadStatusBox = (Dm1V1ChampionTopRowSurfacePc34){
        DM1_GFX_DEAD_CHAMPION, 1, c008, 67, 29 };
    out->assets.championIcons = (Dm1V1ChampionTopRowSurfacePc34){
        DM1_GFX_CHAMPION_ICONS, 1, c028, 76, 14 };
}

static void status_bars(Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = out->atomicPublish = 1;
    out->operationCount = 2;
    out->operations[0].championIndex = 0; out->operations[0].zoneId = 195;
    out->operations[0].originalPalette = palette; out->operations[0].originalIndexedSurface = surface;
    out->operations[1].championIndex = 1; out->operations[1].zoneId = 196;
    out->operations[1].originalPalette = palette; out->operations[1].originalIndexedSurface = surface;
}

static void bridge(Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 *out,
                   const struct PartyState_Compat *party)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = 2; out->generation = 1;
    out->action = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34;
    out->originalMaterials.c008Original = out->originalMaterials.c028Original = 1;
    out->originalMaterials.indexedPaletteOriginal = out->originalMaterials.indexedSurfaceOriginal = 1;
    out->originalMaterials.c008Pixels = c008; out->originalMaterials.c008GraphicIndex = DM1_GFX_DEAD_CHAMPION;
    out->originalMaterials.c008Width = 67; out->originalMaterials.c008Height = 29;
    out->originalMaterials.c028Pixels = c028; out->originalMaterials.c028GraphicIndex = DM1_GFX_CHAMPION_ICONS;
    out->originalMaterials.c028Width = 76; out->originalMaterials.c028Height = 14;
    out->originalMaterials.indexedPalette = palette; out->originalMaterials.indexedPaletteEntryCount = 16;
    out->originalMaterials.indexedSurface = surface;
    out->commandCount = 4;
    out->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34;
    out->commands[0].source.championIndex = 0; out->commands[0].source.sourcePixels = c028;
    out->commands[0].source.portraitPixels = party->champions[0].portraitBitmap;
    out->commands[1].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_BLIT_C008_PC34;
    out->commands[1].source.championIndex = 1; out->commands[1].source.sourcePixels = c008;
    out->commands[2].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34;
    out->commands[2].source.championIndex = 0; out->commands[2].source.statusBar.championIndex = 0;
    out->commands[2].source.statusBar.zoneId = 195; out->commands[2].source.statusBar.originalPalette = palette;
    out->commands[2].source.statusBar.originalIndexedSurface = surface;
    out->commands[3].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34;
    out->commands[3].source.championIndex = 1; out->commands[3].source.statusBar.championIndex = 1;
    out->commands[3].source.statusBar.zoneId = 196; out->commands[3].source.statusBar.originalPalette = palette;
    out->commands[3].source.statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionTopRowAssetsReceiptPc34 topAssets;
    Dm1V1ChampionStatusBarFramePresentationReceiptPc34 bars;
    Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 runtime;
    Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 receipt;
    int ok = 1;

    party_data(&party); assets(&topAssets); status_bars(&bars); bridge(&runtime, &party);
    ok &= check("live composition has only original evidence", dm1_v1_champion_live_runtime_composition_evidence_pc34(
        &party, &topAssets, &bars, &runtime, &receipt) && receipt.evidenceCount == 4 &&
        receipt.evidence[0].portraitPixels == party.champions[0].portraitBitmap &&
        receipt.evidence[1].originalPixels == c008 && receipt.evidence[2].originalPalette == palette);
    runtime.commands[0].source.portraitPixels = c028;
    ok &= check("generated or substituted portrait is rejected", !dm1_v1_champion_live_runtime_composition_evidence_pc34(
        &party, &topAssets, &bars, &runtime, &receipt));
    return ok ? 0 : 1;
}
