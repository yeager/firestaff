#include "dm1_v1_champion_top_row_runtime_m11_bridge_pc34_compat.h"

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

static void clear_output(Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *out,
                         unsigned int tick, unsigned int generation, int revoke)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->clearOnly = 1; out->tick = tick; out->generation = generation;
    out->action = revoke ? DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_REVOKE_CLEAR_PC34
                         : DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_CLEAR_PC34;
    out->commandCount = 1;
    out->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34;
}

static void publish_output(Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 *out,
                           unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->action = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34;
    out->commandCount = 2;
    out->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34;
    out->commands[0].source.sourcePixels = c028; out->commands[0].source.portraitPixels = c028;
    out->commands[1].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34;
    out->commands[1].source.statusBar.originalPalette = palette;
    out->commands[1].source.statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowRuntimeM11BridgeStatePc34 state;
    Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 host;
    Dm1V1ChampionTopRowM11OriginalMaterialsPc34 materials;
    Dm1V1ChampionTopRowRuntimeM11BridgeReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_top_row_runtime_m11_bridge_init_pc34(&state);
    proof(&materials);
    clear_output(&host, 1, 1, 0);
    ok &= check("clear bridges without materials", dm1_v1_champion_top_row_runtime_m11_bridge_pc34(
        &state, &host, &materials, &receipt) && receipt.clearOnly &&
        receipt.originalMaterials.c028Pixels == NULL);
    publish_output(&host, 2, 1);
    ok &= check("proven composition bridges", dm1_v1_champion_top_row_runtime_m11_bridge_pc34(
        &state, &host, &materials, &receipt) && !receipt.clearOnly &&
        receipt.originalMaterials.c028Pixels == c028 && state.m11CompositionActive);
    clear_output(&host, 3, 2, 1);
    ok &= check("revoke remains clear only", dm1_v1_champion_top_row_runtime_m11_bridge_pc34(
        &state, &host, &materials, &receipt) && receipt.clearOnly &&
        receipt.action == DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_REVOKE_CLEAR_PC34 &&
        !state.m11CompositionActive);
    publish_output(&host, 4, 1);
    ok &= check("stale publish rejected", !dm1_v1_champion_top_row_runtime_m11_bridge_pc34(
        &state, &host, &materials, &receipt) && !receipt.valid);
    return ok ? 0 : 1;
}
