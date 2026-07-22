#include "dm1_v1_champion_top_row_m11_host_render_pc34_compat.h"

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
    out->indexedPalette = palette; out->indexedPaletteEntryCount = 16; out->indexedSurface = surface;
}

static void normal_clear(Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = 1; out->generation = 1;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_CLEAR_PC34;
    out->consumption.operationCount = 1;
    out->consumption.operations[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34;
    out->consumption.operations[0].source.operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
}

static void composition(Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *out, unsigned int tick,
                        unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_COMPOSITION_PC34;
    proof(&out->consumption.originalMaterials);
    out->consumption.operationCount = 2;
    out->consumption.operations[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34;
    out->consumption.operations[0].source.sourcePixels = c028;
    out->consumption.operations[0].source.portraitPixels = c028;
    out->consumption.operations[1].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34;
    out->consumption.operations[1].source.statusBar.originalPalette = palette;
    out->consumption.operations[1].source.statusBar.originalIndexedSurface = surface;
}

static void stale_clear(Dm1V1ChampionTopRowM11LifecycleReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = 2; out->generation = 0;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_M11_LIFECYCLE_STALE_ZONE_CLEAR_PC34;
    out->clearOperationCount = 2;
    out->clearOperations[0] = (Dm1V1ChampionTopRowM11LifecycleClearOpPc34){ 1, 152, 69, 0, 67, 29 };
    out->clearOperations[1] = (Dm1V1ChampionTopRowM11LifecycleClearOpPc34){ 1, 196, 70, 20, 4, 12 };
}

int main(void)
{
    Dm1V1ChampionTopRowM11HostRenderStatePc34 state;
    Dm1V1ChampionTopRowM11LifecycleReceiptPc34 lifecycle;
    Dm1V1ChampionTopRowM11HostRenderReceiptPc34 host;
    int ok = 1;

    dm1_v1_champion_top_row_m11_host_render_init_pc34(&state);
    normal_clear(&lifecycle);
    ok &= check("normal lifecycle clear produces clear only", dm1_v1_champion_top_row_m11_host_render_receipt_pc34(
        &state, &lifecycle, &host) && host.clearOnly && host.commandCount == 1 &&
        host.commands[0].kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34);
    stale_clear(&lifecycle);
    ok &= check("stale proof produces only relevant clear commands", dm1_v1_champion_top_row_m11_host_render_receipt_pc34(
        &state, &lifecycle, &host) && host.clearOnly && host.commandCount == 2 &&
        host.commands[0].kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34 &&
        host.commands[0].source.sourcePixels == NULL && host.commands[1].source.zoneId == 196);
    composition(&lifecycle, 3, 1);
    ok &= check("current proof produces full host composition", dm1_v1_champion_top_row_m11_host_render_receipt_pc34(
        &state, &lifecycle, &host) && !host.clearOnly && host.commandCount == 2 &&
        host.commands[0].kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34 &&
        host.commands[1].kind == DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34);
    return ok ? 0 : 1;
}
