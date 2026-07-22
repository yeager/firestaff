#include "dm1_v1_champion_top_row_runtime_frame_admission_pc34_compat.h"

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

static void atomic_clear(Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->clearOnly = 1; out->operationCount = 1;
    out->operations[0].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_CLEAR_STATUS_PC34;
}

static void host_clear(Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = 1; out->generation = 1;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_CLEAR_PC34;
    out->hostRender.valid = 1; out->hostRender.clearOnly = 1; out->hostRender.commandCount = 1;
    out->hostRender.commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34;
}

static void atomic_composition(Dm1V1ChampionTopRowAtomicFrameReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->originalMaterialsPublished = 1; out->operationCount = 2;
    out->operations[0].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_COMPOSE_C028_PC34;
    out->operations[0].sourcePixels = pixels; out->operations[0].portraitPixels = pixels;
    out->operations[1].operation = DM1_V1_CHAMPION_TOP_ROW_ATOMIC_STATUS_BAR_PC34;
    out->operations[1].statusBar.zoneId = 195;
    out->operations[1].statusBar.originalPalette = palette;
    out->operations[1].statusBar.originalIndexedSurface = surface;
}

static void host_composition(Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 *out,
                             unsigned int tick, unsigned int generation)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = tick; out->generation = generation;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_COMPOSITION_PC34;
    out->hostRender.valid = 1; out->hostRender.commandCount = 2;
    out->hostRender.commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_COMPOSE_C028_PC34;
    out->hostRender.commands[0].source.sourcePixels = pixels;
    out->hostRender.commands[0].source.portraitPixels = pixels;
    out->hostRender.commands[1].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_STATUS_BAR_PC34;
    out->hostRender.commands[1].source.statusBar.zoneId = 195;
    out->hostRender.commands[1].source.statusBar.originalPalette = palette;
    out->hostRender.commands[1].source.statusBar.originalIndexedSurface = surface;
}

static void host_stale_clear(Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 *out)
{
    memset(out, 0, sizeof(*out));
    out->valid = 1; out->tick = 4;
    out->publication = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_LIFECYCLE_STALE_CLEAR_PC34;
    out->hostRender.valid = 1; out->hostRender.clearOnly = 1; out->hostRender.commandCount = 1;
    out->hostRender.commands[0].kind = DM1_V1_CHAMPION_TOP_ROW_M11_HOST_RENDER_CLEAR_PC34;
}

int main(void)
{
    Dm1V1ChampionTopRowRuntimeFrameAdmissionStatePc34 state;
    Dm1V1ChampionTopRowAtomicFrameReceiptPc34 atomic;
    Dm1V1ChampionTopRowM11HostRenderLifecycleReceiptPc34 host;
    Dm1V1ChampionTopRowRuntimeFrameAdmissionReceiptPc34 receipt;
    int ok = 1;

    dm1_v1_champion_top_row_runtime_frame_admission_init_pc34(&state);
    atomic_clear(&atomic); host_clear(&host);
    ok &= check("atomic clear is admitted", dm1_v1_champion_top_row_runtime_frame_admission_pc34(
        &state, &atomic, &host, &receipt) && receipt.clearOnly && receipt.commandCount == 1);
    atomic_composition(&atomic); host_composition(&host, 3, 1);
    ok &= check("matching atomic composition is admitted", dm1_v1_champion_top_row_runtime_frame_admission_pc34(
        &state, &atomic, &host, &receipt) && !receipt.clearOnly && receipt.commandCount == 2);
    host_stale_clear(&host);
    ok &= check("stale lifecycle remains material-free clear", dm1_v1_champion_top_row_runtime_frame_admission_pc34(
        &state, &atomic, &host, &receipt) && receipt.clearOnly &&
        receipt.commands[0].source.sourcePixels == NULL);
    atomic_composition(&atomic); host_composition(&host, 5, 2);
    ok &= check("composition without matching clear is rejected", !dm1_v1_champion_top_row_runtime_frame_admission_pc34(
        &state, &atomic, &host, &receipt) && !receipt.valid);
    return ok ? 0 : 1;
}
