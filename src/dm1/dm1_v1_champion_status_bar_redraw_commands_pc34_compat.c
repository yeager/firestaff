#include "dm1_v1_champion_status_bar_redraw_commands_pc34_compat.h"

#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int materials_are_original(const Dm1V1ChampionStatusBarRedrawMaterialsPc34 *materials)
{
    return materials && materials->statusTargetReady &&
           materials->indexedPaletteOriginal && materials->indexedPalette &&
           materials->indexedPaletteEntryCount >= 16;
}

static int operation_is_exact(const Dm1V1ChampionStatusBarRedrawOpPc34 *op)
{
    DM1_V1_ChampionStatusRectPc34 rect;
    int expectedZone;
    if (!op || (op->operation != DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 &&
                op->operation != DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34) ||
        (op->route != DM1_V1_CHAMPION_PORTRAIT_STATUS_PRIMARY_F0296_PC34 &&
         op->route != DM1_V1_CHAMPION_PORTRAIT_STATUS_INVENTORY_F0292_PC34) ||
        op->championIndex < 0 || op->championIndex >= CHAMPION_MAX_PARTY ||
        op->statIndex < 0 || op->statIndex >= 3 || op->width != 4 ||
        op->height <= 0 || op->color < 0 || op->color > 15 || op->current < 0 ||
        op->maximum <= 0) return 0;
    expectedZone = dm1_v1_champion_status_bar_value_zone_id_pc34(
        op->championIndex, op->statIndex);
    if (expectedZone != op->zoneId ||
        !dm1_v1_champion_status_bar_rect_pc34(op->championIndex, op->statIndex, &rect)) {
        return 0;
    }
    return op->x == rect.x && op->y >= rect.y &&
           op->y + op->height <= rect.y + rect.h;
}

static int order_is_exact(const Dm1V1ChampionStatusBarRedrawReceiptPc34 *receipt)
{
    int i;
    int previousChampion = -1;
    int previousStat = -1;
    int previousOperation = 0;
    for (i = 0; i < receipt->operationCount; ++i) {
        const Dm1V1ChampionStatusBarRedrawOpPc34 *op = &receipt->operations[i];
        if (!operation_is_exact(op)) return 0;
        if (op->championIndex < previousChampion ||
            (op->championIndex == previousChampion && op->statIndex < previousStat)) {
            return 0;
        }
        if (op->championIndex == previousChampion && op->statIndex == previousStat) {
            if (previousOperation != DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 ||
                op->operation != DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34) return 0;
        }
        previousChampion = op->championIndex;
        previousStat = op->statIndex;
        previousOperation = op->operation;
    }
    return 1;
}

const char *dm1_v1_champion_status_bar_redraw_commands_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0287:307-346 emits the PC34 clear then "
           "fill rectangles in status-bar walk order. The indexed palette "
           "and target bitmap must be retained original render material; no "
           "host palette or replacement target is admitted.";
}

int dm1_v1_champion_status_bar_redraw_commands_pc34(
    const Dm1V1ChampionStatusBarRedrawReceiptPc34 *receipt,
    const Dm1V1ChampionStatusBarRedrawMaterialsPc34 *materials,
    Dm1V1ChampionStatusBarRedrawCommandSequencePc34 *outSequence)
{
    int i;
    if (!receipt || !receipt->valid || !receipt->dataGateAccepted ||
        !outSequence || receipt->operationCount < 0 ||
        receipt->operationCount > DM1_V1_CHAMPION_STATUS_BAR_REDRAW_MAX_OPS_PC34 ||
        !materials_are_original(materials) || !order_is_exact(receipt)) return 0;
    memset(outSequence, 0, sizeof(*outSequence));
    for (i = 0; i < receipt->operationCount; ++i) {
        const Dm1V1ChampionStatusBarRedrawOpPc34 *source = &receipt->operations[i];
        Dm1V1ChampionStatusBarRedrawCommandPc34 *command = &outSequence->commands[i];
        command->operation = source->operation;
        command->route = source->route;
        command->championIndex = source->championIndex;
        command->statIndex = source->statIndex;
        command->zoneId = source->zoneId;
        command->x = source->x;
        command->y = source->y;
        command->width = source->width;
        command->height = source->height;
        command->colorIndex = (uint8_t)source->color;
        command->originalPalette = materials->indexedPalette;
    }
    outSequence->commandCount = receipt->operationCount;
    outSequence->valid = 1;
    return 1;
}
