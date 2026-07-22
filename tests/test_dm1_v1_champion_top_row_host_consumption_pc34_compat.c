#include "dm1_v1_champion_top_row_host_consumption_pc34_compat.h"

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

static void clear_receipt(Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *receipt,
                          unsigned int generation)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34;
    receipt->generation = generation;
    receipt->commandCount = 1;
    receipt->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34;
    receipt->commands[0].source.operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
}

static void composition_receipt(Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 *receipt,
                                unsigned int generation)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34;
    receipt->generation = generation;
    receipt->commandCount = 2;
    receipt->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34;
    receipt->commands[0].source.operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34;
    receipt->commands[0].source.sourcePixels = pixels;
    receipt->commands[0].source.portraitPixels = pixels;
    receipt->commands[1].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34;
    receipt->commands[1].source.operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34;
    receipt->commands[1].source.statusBar.originalPalette = palette;
    receipt->commands[1].source.statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowHostConsumptionStatePc34 state;
    Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 render;
    Dm1V1ChampionTopRowHostConsumptionReceiptPc34 host;
    int ok = 1;

    dm1_v1_champion_top_row_host_consumption_init_pc34(&state);
    composition_receipt(&render, 1);
    ok &= check("host rejects composition without prior clear",
        !dm1_v1_champion_top_row_host_consumption_pc34(&state, &render, &host) && !host.valid);
    clear_receipt(&render, 1);
    ok &= check("host consumes clear generation one",
        dm1_v1_champion_top_row_host_consumption_pc34(&state, &render, &host) &&
        host.operationCount == 1 && state.pendingClearGeneration == 1);
    composition_receipt(&render, 1);
    ok &= check("host consumes matching fresh composition",
        dm1_v1_champion_top_row_host_consumption_pc34(&state, &render, &host) &&
        host.operationCount == 2 && state.pendingClearGeneration == 0);
    clear_receipt(&render, 2);
    ok &= check("host consumes newer clear", dm1_v1_champion_top_row_host_consumption_pc34(
        &state, &render, &host));
    composition_receipt(&render, 1);
    ok &= check("host rejects stale original composition", !dm1_v1_champion_top_row_host_consumption_pc34(
        &state, &render, &host) && !host.valid);
    composition_receipt(&render, 2);
    ok &= check("host consumes current composition", dm1_v1_champion_top_row_host_consumption_pc34(
        &state, &render, &host) && host.generation == 2);
    return ok ? 0 : 1;
}
