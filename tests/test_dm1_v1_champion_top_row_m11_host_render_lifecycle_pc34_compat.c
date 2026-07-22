#include "dm1_v1_champion_top_row_m11_host_render_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t pixels[] = { 1 };
static const uint8_t palette[] = { 1 };
static uint8_t surface[16];

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void clear_only(Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *out,
                       unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->clearOnly = 1; out->tick = tick; out->generation = generation;
    out->commandCount = 1;
    out->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34;
    out->commands[0].source.operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
}

static void composition(Dm1V1ChampionTopRowM11HostRenderReceiptPc34 *out,
                        unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->commandCount = 2;
    out->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34;
    out->commands[0].source.sourcePixels = pixels;
    out->commands[0].source.portraitPixels = pixels;
    out->commands[1].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34;
    out->commands[1].source.statusBar.originalPalette = palette;
    out->commands[1].source.statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowM11HostRenderLifecycleStatePc34 state;
    Dm1V1ChampionTopRowM11HostRenderReceiptPc34 host;
    Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_top_row_m11_host_render_lifecycle_init_pc34(&state);
    clear_only(&host, 1, 1);
    ok &= check("current clear starts host lifecycle", dm1_v1_champion_top_row_m11_host_render_lifecycle_step_pc34(
        &state, &host, &receipt) && receipt.publication ==
        DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_CLEAR_PC34);
    clear_only(&host, 2, 0);
    ok &= check("stale path remains clear only", dm1_v1_champion_top_row_m11_host_render_lifecycle_step_pc34(
        &state, &host, &receipt) && receipt.publication ==
        DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_STALE_CLEAR_PC34 &&
        receipt.hostRender.clearOnly && receipt.hostRender.commands[0].source.sourcePixels == NULL &&
        state.pendingClearGeneration == 1);
    composition(&host, 3, 1);
    ok &= check("matching composition follows retained clear", dm1_v1_champion_top_row_m11_host_render_lifecycle_step_pc34(
        &state, &host, &receipt) && receipt.publication ==
        DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_COMPOSITION_PC34);
    composition(&host, 4, 1);
    ok &= check("stale full composition is rejected", !dm1_v1_champion_top_row_m11_host_render_lifecycle_step_pc34(
        &state, &host, &receipt) && !receipt.valid);
    return ok ? 0 : 1;
}
