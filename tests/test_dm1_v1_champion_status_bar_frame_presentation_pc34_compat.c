#include "dm1_v1_champion_status_bar_frame_presentation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static uint8_t surface[320 * 200];
static const uint8_t palette[16 * 3] = { 1 };

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void command(Dm1V1ChampionStatusBarRedrawCommandPc34 *out,
                    Dm1V1ChampionStatusBarRedrawOperationPc34 operation,
                    int y)
{
    memset(out, 0, sizeof(*out));
    out->operation = operation;
    out->route = DM1_V1_CHAMPION_PORTRAIT_STATUS_PRIMARY_F0296_PC34;
    out->championIndex = 0;
    out->statIndex = 0;
    out->zoneId = 195;
    out->x = 1;
    out->y = y;
    out->width = 4;
    out->height = 12;
    out->colorIndex = operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 ? 12 : 7;
    out->originalPalette = palette;
}

int main(void)
{
    Dm1V1ChampionStatusBarRedrawCommandSequencePc34 commands;
    Dm1V1ChampionStatusBarFrameMaterialsPc34 materials;
    Dm1V1ChampionStatusBarFramePresentationReceiptPc34 receipt;
    int ok = 1;

    memset(&commands, 0, sizeof(commands));
    commands.valid = 1;
    commands.commandCount = 2;
    command(&commands.commands[0], DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34, 10);
    command(&commands.commands[1], DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34, 22);
    materials.originalPaletteReady = 1;
    materials.originalPalette = palette;
    materials.originalPaletteEntryCount = 16;
    materials.originalSurfaceReady = 1;
    materials.originalIndexedSurface = surface;
    materials.surfaceWidth = 320;
    materials.surfaceHeight = 200;
    materials.surfacePitch = 320;
    ok &= check("atomically publishes retained palette and surface",
        dm1_v1_champion_status_bar_frame_presentation_pc34(&commands, &materials, &receipt) &&
        receipt.valid && receipt.atomicPublish && receipt.operationCount == 2 &&
        receipt.clearCount == 1 && receipt.repaintCount == 1 &&
        receipt.operations[0].originalPalette == palette &&
        receipt.operations[0].originalIndexedSurface == surface &&
        receipt.operations[1].operation == DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34);

    materials.originalPaletteReady = 0;
    ok &= check("missing palette emits no partial frame",
        !dm1_v1_champion_status_bar_frame_presentation_pc34(&commands, &materials, &receipt) &&
        !receipt.valid && receipt.operationCount == 0);
    materials.originalPaletteReady = 1;
    commands.commands[1].y = 199;
    ok &= check("out of bounds command emits no partial frame",
        !dm1_v1_champion_status_bar_frame_presentation_pc34(&commands, &materials, &receipt) &&
        !receipt.valid && receipt.operationCount == 0);
    return ok ? 0 : 1;
}
