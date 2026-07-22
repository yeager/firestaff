#include "dm1_v1_champion_status_bar_frame_presentation_pc34_compat.h"

#include <string.h>

static int materials_are_ready(const Dm1V1ChampionStatusBarFrameMaterialsPc34 *materials)
{
    return materials && materials->originalPaletteReady && materials->originalPalette &&
           materials->originalPaletteEntryCount >= 16 && materials->originalSurfaceReady &&
           materials->originalIndexedSurface && materials->surfaceWidth > 0 &&
           materials->surfaceHeight > 0 && materials->surfacePitch >= materials->surfaceWidth;
}

static int command_fits_surface(const Dm1V1ChampionStatusBarRedrawCommandPc34 *command,
                                const Dm1V1ChampionStatusBarFrameMaterialsPc34 *materials)
{
    if (!command || command->x < 0 || command->y < 0 || command->width <= 0 ||
        command->height <= 0 || command->x + command->width > materials->surfaceWidth ||
        command->y + command->height > materials->surfaceHeight ||
        command->colorIndex >= materials->originalPaletteEntryCount ||
        command->originalPalette != materials->originalPalette) return 0;
    return command->operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 ||
           command->operation == DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34;
}

const char *dm1_v1_champion_status_bar_frame_presentation_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0287:307-346 performs ordered indexed-color "
           "bar writes into the current logical screen. F0680/F0692 consume "
           "the retained screen and palette as one presentation unit; a mixed "
           "palette or substitute surface is not an original frame.";
}

int dm1_v1_champion_status_bar_frame_presentation_pc34(
    const Dm1V1ChampionStatusBarRedrawCommandSequencePc34 *commands,
    const Dm1V1ChampionStatusBarFrameMaterialsPc34 *materials,
    Dm1V1ChampionStatusBarFramePresentationReceiptPc34 *outReceipt)
{
    Dm1V1ChampionStatusBarFramePresentationReceiptPc34 pending;
    int i;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!commands || !commands->valid || commands->commandCount < 0 ||
        commands->commandCount > DM1_V1_CHAMPION_STATUS_BAR_REDRAW_MAX_OPS_PC34 ||
        !materials_are_ready(materials)) return 0;

    memset(&pending, 0, sizeof(pending));
    for (i = 0; i < commands->commandCount; ++i) {
        const Dm1V1ChampionStatusBarRedrawCommandPc34 *source = &commands->commands[i];
        Dm1V1ChampionStatusBarFrameOpPc34 *dest;
        if (!command_fits_surface(source, materials)) return 0;
        dest = &pending.operations[pending.operationCount++];
        dest->operation = source->operation;
        dest->route = source->route;
        dest->championIndex = source->championIndex;
        dest->statIndex = source->statIndex;
        dest->zoneId = source->zoneId;
        dest->x = source->x;
        dest->y = source->y;
        dest->width = source->width;
        dest->height = source->height;
        dest->colorIndex = source->colorIndex;
        dest->originalPalette = materials->originalPalette;
        dest->originalIndexedSurface = materials->originalIndexedSurface;
        dest->surfacePitch = materials->surfacePitch;
        if (source->operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34)
            ++pending.clearCount;
        else
            ++pending.repaintCount;
    }
    pending.atomicPublish = 1;
    pending.valid = 1;
    *outReceipt = pending;
    return 1;
}
