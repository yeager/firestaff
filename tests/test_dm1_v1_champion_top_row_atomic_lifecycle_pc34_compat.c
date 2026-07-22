#include "dm1_v1_champion_top_row_atomic_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t pixels[] = { 1 };
static uint8_t surface[16];
static const uint8_t palette[] = { 1 };

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void clear_frame(Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame)
{
    memset(frame, 0, sizeof(*frame));
    frame->valid = 1;
    frame->clearOnly = 1;
    frame->operationCount = 1;
    frame->operations[0].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
}

static void complete_frame(Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *frame)
{
    memset(frame, 0, sizeof(*frame));
    frame->valid = 1;
    frame->originalMaterialsPublished = 1;
    frame->operationCount = 2;
    frame->operations[0].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34;
    frame->operations[0].sourcePixels = pixels;
    frame->operations[0].portraitPixels = pixels;
    frame->operations[1].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34;
    frame->operations[1].statusBar.width = 4;
    frame->operations[1].statusBar.height = 12;
    frame->operations[1].statusBar.originalPalette = palette;
    frame->operations[1].statusBar.originalIndexedSurface = surface;
}

int main(void)
{
    Dm1V1ChampionTopRowAtomicLifecycleStatePc34 state;
    Dm1V1ChampionTopRowAtomicLifecycleReceiptPc34 receipt;
    Dm1V1ChampionTopRowAtomicFrameReceiptPc34 frame;
    int ok = 1;

    dm1_v1_champion_top_row_atomic_lifecycle_init_pc34(&state);
    complete_frame(&frame);
    ok &= check("composition cannot precede clear",
        !dm1_v1_champion_top_row_atomic_lifecycle_step_pc34(&state, &frame, &receipt) &&
        !state.initialized && !receipt.valid);

    clear_frame(&frame);
    ok &= check("clear publishes first tick",
        dm1_v1_champion_top_row_atomic_lifecycle_step_pc34(&state, &frame, &receipt) &&
        receipt.publication == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_CLEAR_PC34 &&
        receipt.generation == 1 && state.clearPublishedSinceComposition);

    complete_frame(&frame);
    ok &= check("next complete composition publishes atomically",
        dm1_v1_champion_top_row_atomic_lifecycle_step_pc34(&state, &frame, &receipt) &&
        receipt.publication == DM1_V1_CHAMPION_TOP_ROW_ATOMIC_LIFECYCLE_COMPOSITION_PC34 &&
        receipt.frame.operationCount == 2 && !state.clearPublishedSinceComposition);

    ok &= check("second composition requires another clear",
        !dm1_v1_champion_top_row_atomic_lifecycle_step_pc34(&state, &frame, &receipt) &&
        !receipt.valid);
    clear_frame(&frame);
    frame.operations[0].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34;
    ok &= check("partial frame cannot advance lifecycle",
        !dm1_v1_champion_top_row_atomic_lifecycle_step_pc34(&state, &frame, &receipt) &&
        !receipt.valid);
    return ok ? 0 : 1;
}
