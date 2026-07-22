#include "dm1_v1_champion_top_row_host_lifecycle_pc34_compat.h"

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

static void clear_host(Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *receipt,
                       unsigned int generation)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34;
    receipt->generation = generation;
    receipt->operationCount = 1;
    receipt->operations[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34;
    receipt->operations[0].source.operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
}

static void composition_host(Dm1V1ChampionTopRowHostConsumptionReceiptPc34 *receipt,
                             unsigned int generation)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34;
    receipt->generation = generation;
    receipt->operationCount = 2;
    receipt->operations[0].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34;
    receipt->operations[0].source.sourcePixels = pixels;
    receipt->operations[0].source.portraitPixels = pixels;
    receipt->operations[1].kind = DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34;
    receipt->operations[1].source.statusBar.originalPalette = palette;
    receipt->operations[1].source.statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowHostLifecycleStatePc34 state;
    Dm1V1ChampionTopRowHostConsumptionReceiptPc34 host;
    Dm1V1ChampionTopRowHostLifecycleReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_top_row_host_lifecycle_init_pc34(&state);
    composition_host(&host, 1);
    ok &= check("composition requires clear tick", !dm1_v1_champion_top_row_host_lifecycle_step_pc34(
        &state, 1, &host, &receipt) && !receipt.valid);
    clear_host(&host, 1);
    ok &= check("clear tick one accepted", dm1_v1_champion_top_row_host_lifecycle_step_pc34(
        &state, 1, &host, &receipt) && receipt.tick == 1);
    composition_host(&host, 1);
    ok &= check("matching composition tick two accepted", dm1_v1_champion_top_row_host_lifecycle_step_pc34(
        &state, 2, &host, &receipt) && receipt.generation == 1);
    clear_host(&host, 2);
    ok &= check("new clear invalidates earlier host composition", dm1_v1_champion_top_row_host_lifecycle_step_pc34(
        &state, 3, &host, &receipt));
    composition_host(&host, 1);
    ok &= check("stale host composition rejected after clear change", !dm1_v1_champion_top_row_host_lifecycle_step_pc34(
        &state, 4, &host, &receipt) && !receipt.valid && state.lastTick == 3);
    composition_host(&host, 2);
    ok &= check("current composition is accepted", dm1_v1_champion_top_row_host_lifecycle_step_pc34(
        &state, 4, &host, &receipt) && receipt.tick == 4);
    return ok ? 0 : 1;
}
