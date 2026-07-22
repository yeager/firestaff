#include "dm1_v1_champion_top_row_runtime_host_output_pc34_compat.h"

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

static void clear_admission(Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *out,
                            unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->clearOnly = 1; out->tick = tick; out->generation = generation;
    out->kind = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_CLEAR_PC34;
    out->commandCount = 1;
    out->commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34;
}

static void composition_admission(Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 *out,
                                  unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->kind = DM1_V1_CHAMPION_TOP_ROW_RUNTIME_FRAME_COMPOSITION_PC34;
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
    Dm1V1ChampionTopRowRuntimeHostOutputStatePc34 state;
    Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 admission;
    Dm1V1ChampionTopRowRuntimeHostOutputReceiptPc34 output;
    int ok = 1;

    dm1_v1_champion_top_row_runtime_host_output_init_pc34(&state);
    clear_admission(&admission, 1, 1);
    ok &= check("first clear reaches host", dm1_v1_champion_top_row_runtime_host_output_step_pc34(
        &state, &admission, &output) && output.action ==
        DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_CLEAR_PC34 && output.clearOnly);
    composition_admission(&admission, 2, 1);
    ok &= check("matching composition publishes", dm1_v1_champion_top_row_runtime_host_output_step_pc34(
        &state, &admission, &output) && output.action ==
        DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_PUBLISH_PC34 && state.hostCompositionActive);
    clear_admission(&admission, 3, 2);
    ok &= check("new clear revokes active host output", dm1_v1_champion_top_row_runtime_host_output_step_pc34(
        &state, &admission, &output) && output.action ==
        DM1_V1_CHAMPION_TOP_ROW_RUNTIME_HOST_OUTPUT_REVOKE_CLEAR_PC34 &&
        output.clearOnly && !state.hostCompositionActive);
    composition_admission(&admission, 4, 1);
    ok &= check("old composition cannot republish", !dm1_v1_champion_top_row_runtime_host_output_step_pc34(
        &state, &admission, &output) && !output.valid);
    composition_admission(&admission, 4, 2);
    ok &= check("new generation publishes after clear", dm1_v1_champion_top_row_runtime_host_output_step_pc34(
        &state, &admission, &output) && output.generation == 2);
    return ok ? 0 : 1;
}
