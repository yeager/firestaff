#include "dm1_v1_champion_top_row_m11_lifecycle_pc34_compat.h"

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
    out->c008Original = out->c028Original = out->indexedPaletteOriginal = out->indexedSurfaceOriginal = 1;
    out->c008Pixels = c008; out->c008GraphicIndex = DM1_GFX_DEAD_CHAMPION;
    out->c008Width = 67; out->c008Height = 29;
    out->c028Pixels = c028; out->c028GraphicIndex = DM1_GFX_CHAMPION_ICONS;
    out->c028Width = 76; out->c028Height = 14;
    out->indexedPalette = palette; out->indexedPaletteEntryCount = 16;
    out->indexedSurface = surface;
}

static void clear_consumption(Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *out,
                              unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34;
}

static void composition_consumption(Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 *out,
                                    unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34;
    proof(&out->originalMaterials);
    out->operationCount = 2;
    out->operations[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34;
    out->operations[0].source.championIndex = 1;
    out->operations[0].source.sourcePixels = c028;
    out->operations[0].source.portraitPixels = c028;
    out->operations[1].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34;
    out->operations[1].source.statusBar.championIndex = 1;
    out->operations[1].source.statusBar.zoneId = 196;
    out->operations[1].source.statusBar.x = 70;
    out->operations[1].source.statusBar.y = 20;
    out->operations[1].source.statusBar.width = 4;
    out->operations[1].source.statusBar.height = 12;
    out->operations[1].source.statusBar.originalPalette = palette;
    out->operations[1].source.statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowM11LifecycleStatePc34 state;
    Dm1V1ChampionTopRowM11ConsumptionReceiptPc34 consumption;
    Dm1V1ChampionTopRowM11LifecycleReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_top_row_m11_lifecycle_init_pc34(&state);
    clear_consumption(&consumption, 1, 1);
    ok &= check("clear starts lifecycle", dm1_v1_champion_top_row_m11_lifecycle_step_pc34(
        &state, &consumption, &receipt) && receipt.publication ==
        DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_CLEAR_PC34);
    composition_consumption(&consumption, 2, 1);
    ok &= check("matching proof composes", dm1_v1_champion_top_row_m11_lifecycle_step_pc34(
        &state, &consumption, &receipt) && receipt.publication ==
        DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_COMPOSITION_PC34);
    clear_consumption(&consumption, 3, 2);
    ok &= check("new clear changes generation", dm1_v1_champion_top_row_m11_lifecycle_step_pc34(
        &state, &consumption, &receipt));
    composition_consumption(&consumption, 4, 1);
    ok &= check("stale proof clears relevant zones only", dm1_v1_champion_top_row_m11_lifecycle_step_pc34(
        &state, &consumption, &receipt) && receipt.publication ==
        DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_STALE_ZONE_CLEAR_PC34 &&
        receipt.clearOperationCount == 2 && receipt.clearOperations[0].championIndex == 1 &&
        receipt.clearOperations[1].zoneId == 196 && state.pendingClearGeneration == 2);
    composition_consumption(&consumption, 5, 2);
    ok &= check("current proof composes after stale clear", dm1_v1_champion_top_row_m11_lifecycle_step_pc34(
        &state, &consumption, &receipt) && receipt.publication ==
        DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_COMPOSITION_PC34);
    return ok ? 0 : 1;
}
