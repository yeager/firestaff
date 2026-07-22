#include "dm1_v1_champion_top_row_m11_consumption_pc34_compat.h"

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

static void proof(Dm1V1ChampionTopRowM11OriginalMaterialsPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->c008Original = 1; out->c008Pixels = c008;
    out->c008GraphicIndex = DM1_GFX_DEAD_CHAMPION; out->c008Width = 67; out->c008Height = 29;
    out->c028Original = 1; out->c028Pixels = c028;
    out->c028GraphicIndex = DM1_GFX_CHAMPION_ICONS; out->c028Width = 76; out->c028Height = 14;
    out->indexedPaletteOriginal = 1; out->indexedPalette = palette; out->indexedPaletteEntryCount = 16;
    out->indexedSurfaceOriginal = 1; out->indexedSurface = surface;
}

static void clear_host(Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *out, unsigned int tick,
                       unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34;
    out->generation = generation;
    out->hostConsumption.valid = 1; out->hostConsumption.operationCount = 1;
    out->hostConsumption.operations[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34;
}

static void composition_host(Dm1V1ChampionTopRowHostLifecycleReceiptPc34 *out, unsigned int tick,
                             unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34;
    out->generation = generation;
    out->hostConsumption.valid = 1; out->hostConsumption.operationCount = 2;
    out->hostConsumption.operations[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34;
    out->hostConsumption.operations[0].source.graphicIndex = DM1_GFX_CHAMPION_ICONS;
    out->hostConsumption.operations[0].source.sourcePixels = c028;
    out->hostConsumption.operations[0].source.portraitPixels = c028;
    out->hostConsumption.operations[1].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34;
    out->hostConsumption.operations[1].source.statusBar.originalPalette = palette;
    out->hostConsumption.operations[1].source.statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowM11ConsumptionStatePc34 state;
    Dm1V1ChampionTopRowHostLifecycleReceiptPc34 host;
    Dm1V1ChampionTopRowM11OriginalMaterialsPc34 materials;
    Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_top_row_m11_consumption_init_pc34(&state);
    proof(&materials);
    composition_host(&host, 1, 1);
    ok &= check("M11 boundary requires prior clear", !dm1_v1_champion_top_row_m11_consumption_receipt_pc34(
        &state, &host, &materials, &receipt) && !receipt.valid);
    clear_host(&host, 1, 1);
    ok &= check("M11 boundary accepts proven clear", dm1_v1_champion_top_row_m11_consumption_receipt_pc34(
        &state, &host, &materials, &receipt) && receipt.operationCount == 1);
    composition_host(&host, 2, 1);
    ok &= check("M11 boundary accepts matching proven composition", dm1_v1_champion_top_row_m11_consumption_receipt_pc34(
        &state, &host, &materials, &receipt) && receipt.operationCount == 2 &&
        receipt.originalMaterials.c028Pixels == c028);
    clear_host(&host, 3, 2);
    ok &= check("M11 boundary advances clear generation", dm1_v1_champion_top_row_m11_consumption_receipt_pc34(
        &state, &host, &materials, &receipt));
    composition_host(&host, 4, 1);
    ok &= check("M11 boundary rejects stale composition", !dm1_v1_champion_top_row_m11_consumption_receipt_pc34(
        &state, &host, &materials, &receipt) && !receipt.valid);
    return ok ? 0 : 1;
}
