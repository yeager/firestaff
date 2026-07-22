#include "dm1_v1_champion_top_row_lifecycle_render_bridge_pc34_compat.h"

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

static void clear_lifecycle(Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 *receipt)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34;
    receipt->generation = 1;
    receipt->frame.valid = 1;
    receipt->frame.clearOnly = 1;
    receipt->frame.operationCount = 1;
    receipt->frame.operations[0].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
}

static void composition_lifecycle(Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 *receipt)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->publication = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34;
    receipt->generation = 1;
    receipt->frame.valid = 1;
    receipt->frame.originalMaterialsPublished = 1;
    receipt->frame.operationCount = 2;
    receipt->frame.operations[0].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34;
    receipt->frame.operations[0].sourcePixels = pixels;
    receipt->frame.operations[0].portraitPixels = pixels;
    receipt->frame.operations[1].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34;
    receipt->frame.operations[1].statusBar.width = 4;
    receipt->frame.operations[1].statusBar.height = 12;
    receipt->frame.operations[1].statusBar.originalPalette = palette;
    receipt->frame.operations[1].statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowLifecycleRenderBridgeStatePc34 state;
    Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 lifecycle;
    Dm1V1ChampionTopRowLifecycleRenderReceiptPc34 render;
    int ok = 1;

    dm1_v1_champion_top_row_lifecycle_render_bridge_init_pc34(&state);
    composition_lifecycle(&lifecycle);
    ok &= check("composition cannot bridge before clear",
        !dm1_v1_champion_top_row_lifecycle_render_bridge_pc34(&state, &lifecycle, &render) &&
        !render.valid);

    clear_lifecycle(&lifecycle);
    ok &= check("clear bridges first",
        dm1_v1_champion_top_row_lifecycle_render_bridge_pc34(&state, &lifecycle, &render) &&
        render.commandCount == 1 &&
        render.commands[0].kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_CLEAR_PC34);

    composition_lifecycle(&lifecycle);
    ok &= check("complete composition bridges after clear",
        dm1_v1_champion_top_row_lifecycle_render_bridge_pc34(&state, &lifecycle, &render) &&
        render.commandCount == 2 &&
        render.commands[0].kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_COMPOSE_C028_PC34 &&
        render.commands[1].kind == DM1_V1_CHAMPION_TOP_ROW_LIFECYCLE_RENDER_STATUS_BAR_PC34);

    ok &= check("second composition is rejected until next clear",
        !dm1_v1_champion_top_row_lifecycle_render_bridge_pc34(&state, &lifecycle, &render) &&
        !render.valid);
    return ok ? 0 : 1;
}
