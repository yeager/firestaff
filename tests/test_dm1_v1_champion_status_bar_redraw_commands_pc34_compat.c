#include "dm1_v1_champion_status_bar_redraw_commands_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const uint8_t palette[16 * 3] = { 1 };

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void op(Dm1V1ChampionStatusBarRedrawOpPc34 *out,
               Dm1V1ChampionStatusBarRedrawOperationPc34 operation,
               int champion, int stat, int y, int height)
{
    DM1_V1_ChampionStatusRectPc34 rect;
    memset(out, 0, sizeof(*out));
    dm1_v1_champion_status_bar_rect_pc34(champion, stat, &rect);
    out->operation = operation;
    out->route = DM1_V1_CHAMPION_PORTRAIT_STATUS_PRIMARY_F0296_PC34;
    out->championIndex = champion;
    out->statIndex = stat;
    out->zoneId = dm1_v1_champion_status_bar_value_zone_id_pc34(champion, stat);
    out->x = rect.x;
    out->y = y;
    out->width = 4;
    out->height = height;
    out->color = operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34
        ? DM1_COLOR_DARKEST_GRAY : DM1_ChampionColor[champion];
    out->current = 50;
    out->maximum = 100;
}

int main(void)
{
    Dm1V1ChampionStatusBarRedrawReceiptPc34 receipt;
    Dm1V1ChampionStatusBarRedrawMaterialsPc34 materials;
    Dm1V1ChampionStatusBarRedrawCommandSequencePc34 commands;
    DM1_V1_ChampionStatusRectPc34 rect;
    int ok = 1;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.dataGateAccepted = 1;
    dm1_v1_champion_status_bar_rect_pc34(0, 0, &rect);
    op(&receipt.operations[0], DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34,
       0, 0, rect.y, 12);
    op(&receipt.operations[1], DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34,
       0, 0, rect.y + 12, 13);
    op(&receipt.operations[2], DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34,
       0, 1, rect.y, 25);
    receipt.operationCount = 3;
    materials.statusTargetReady = 1;
    materials.indexedPaletteOriginal = 1;
    materials.indexedPalette = palette;
    materials.indexedPaletteEntryCount = 16;
    ok &= check("commands preserve source clear repaint order",
        dm1_v1_champion_status_bar_redraw_commands_pc34(&receipt, &materials, &commands) &&
        commands.valid && commands.commandCount == 3 &&
        commands.commands[0].operation == DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34 &&
        commands.commands[1].operation == DM1_V1_CHAMPION_STATUS_BAR_REPAINT_PC34 &&
        commands.commands[2].statIndex == 1 &&
        commands.commands[0].originalPalette == palette);

    materials.indexedPaletteOriginal = 0;
    ok &= check("missing original palette fails closed",
        !dm1_v1_champion_status_bar_redraw_commands_pc34(&receipt, &materials, &commands));
    materials.indexedPaletteOriginal = 1;
    receipt.operations[1].operation = DM1_V1_CHAMPION_STATUS_BAR_CLEAR_PC34;
    ok &= check("invalid clear repaint order fails closed",
        !dm1_v1_champion_status_bar_redraw_commands_pc34(&receipt, &materials, &commands));
    return ok ? 0 : 1;
}
