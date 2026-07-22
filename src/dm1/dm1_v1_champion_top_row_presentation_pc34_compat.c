#include "dm1_v1_champion_top_row_presentation_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_layout_zones_pc34_compat.h"

#include <string.h>

static int append_op(Dm1V1ChampionTopRowPresentationReceiptPc34 *receipt,
                     Dm1V1ChampionTopRowPresentationOpKindPc34 kind,
                     int championSlot, int zoneId, int graphicIndex,
                     const uint8_t *sourcePixels, int sourceX, int sourceY,
                     int sourceWidth, int sourceHeight,
                     int x, int y, int width, int height, int color)
{
    Dm1V1ChampionTopRowPresentationOpPc34 *op;
    if (!receipt || receipt->operationCount >=
        DM1_V1_CHAMPION_TOP_ROW_PRESENTATION_MAX_OPS_PC34) return 0;
    op = &receipt->operations[receipt->operationCount++];
    op->kind = kind;
    op->championSlot = championSlot;
    op->zoneId = zoneId;
    op->graphicIndex = graphicIndex;
    op->sourcePixels = sourcePixels;
    op->sourceX = sourceX;
    op->sourceY = sourceY;
    op->sourceWidth = sourceWidth;
    op->sourceHeight = sourceHeight;
    op->x = x;
    op->y = y;
    op->width = width;
    op->height = height;
    op->color = color;
    return 1;
}

static const Dm1V1ChampionTopRowSurfacePc34 *surface_for_graphic(
    const Dm1V1ChampionTopRowAssetsPc34 *assets, int graphicIndex)
{
    if (!assets) return NULL;
    if (graphicIndex == DM1_GFX_DEAD_CHAMPION) return &assets->deadStatusBox;
    if (graphicIndex == DM1_GFX_CHAMPION_ICONS) return &assets->championIcons;
    if (graphicIndex == DM1_GFX_SLOT_NORMAL) return &assets->slotNormal;
    if (graphicIndex == DM1_GFX_SLOT_WOUNDED) return &assets->slotWounded;
    if (graphicIndex == DM1_GFX_SLOT_ACTING) return &assets->slotActing;
    return NULL;
}

const char *dm1_v1_champion_top_row_presentation_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0292:771-905 orders status fill/dead C008, "
           "name C159/C163, F0287 bars C195/C199/C203, then F0291 C033/C034/C035; "
           "F0622 composes C028 champion icon frames. Missing original material "
           "rejects the complete receipt.";
}

int dm1_v1_champion_top_row_presentation_from_frame_pc34(
    const Dm1V1ChampionTopRowFramePc34 *frame,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assetReceipt,
    Dm1V1ChampionTopRowPresentationReceiptPc34 *outReceipt)
{
    int slot;
    if (!frame || !frame->valid || !assetReceipt || !assetReceipt->valid ||
        !outReceipt || frame->partyChampionCount < 0 ||
        frame->partyChampionCount > DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->assetReceipt = *assetReceipt;

    for (slot = 0; slot < DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34; ++slot) {
        const Dm1V1ChampionTopRowSlotPc34 *plan = &frame->slots[slot];
        DM1_V1_ChampionStatusRectPc34 nameRect;
        DM1_V1_ChampionStatusRectPc34 nameTextRect;
        DM1_V1_LayoutZoneRectPc34 iconRect;
        const Dm1V1ChampionTopRowSurfacePc34 *surface;
        int stat;
        int hand;

        if (!plan->present) continue;
        if (!dm1_v1_champion_status_name_rect_pc34(slot, &nameRect) ||
            !dm1_v1_champion_status_name_text_rect_pc34(slot, &nameTextRect) ||
            !dm1_v1_champion_icon_rect_pc34(slot, &iconRect)) return 0;

        if (plan->alive) {
            if (!append_op(outReceipt, DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_STATUS_PC34,
                           slot, plan->statusBoxZoneId, 0, NULL, 0, 0, 0, 0,
                           plan->statusX, plan->statusY, plan->statusWidth,
                           plan->statusHeight, DM1_COLOR_DARKEST_GRAY)) return 0;
        } else {
            surface = surface_for_graphic(&assetReceipt->assets, DM1_GFX_DEAD_CHAMPION);
            if (!surface || !surface->pixels ||
                !append_op(outReceipt, DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34,
                           slot, plan->statusBoxZoneId, DM1_GFX_DEAD_CHAMPION,
                           surface->pixels, 0, 0, surface->width, surface->height,
                           plan->statusX, plan->statusY, plan->statusWidth,
                           plan->statusHeight, -1)) return 0;
        }
        if (!append_op(outReceipt, DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_NAME_PC34,
                       slot, plan->nameClearZoneId, 0, NULL, 0, 0, 0, 0,
                       nameRect.x, nameRect.y, nameRect.w, nameRect.h,
                       DM1_COLOR_DARK_GRAY) ||
            !append_op(outReceipt, DM1_V1_CHAMPION_TOP_ROW_OP_DRAW_NAME_PC34,
                       slot, plan->nameTextZoneId, 0, NULL, 0, 0, 0, 0,
                       nameTextRect.x, nameTextRect.y, nameTextRect.w, nameTextRect.h,
                       plan->nameColor)) return 0;

        surface = surface_for_graphic(&assetReceipt->assets, DM1_GFX_CHAMPION_ICONS);
        if (!surface || !surface->pixels ||
            !append_op(outReceipt, DM1_V1_CHAMPION_TOP_ROW_OP_COMPOSE_ICON_PC34,
                       slot, 113 + slot, DM1_GFX_CHAMPION_ICONS, surface->pixels,
                       plan->iconSourceX, 0, DM1_CHAMPION_ICON_WIDTH,
                       DM1_CHAMPION_ICON_HEIGHT, iconRect.x, iconRect.y,
                       iconRect.w, iconRect.h, plan->iconFillColor)) return 0;

        if (!plan->alive) continue;
        for (stat = 0; stat < DM1_V1_CHAMPION_TOP_ROW_STAT_COUNT_PC34; ++stat) {
            const Dm1V1ChampionTopRowStatPc34 *bar = &plan->stats[stat];
            if (bar->blankHeight && !append_op(outReceipt,
                                                DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_BAR_PC34,
                                                slot, bar->zoneId, 0, NULL, 0, 0, 0, 0,
                                                bar->x, bar->y, bar->width, bar->blankHeight,
                                                bar->blankColor)) return 0;
            if (bar->fillHeight && !append_op(outReceipt,
                                               DM1_V1_CHAMPION_TOP_ROW_OP_FILL_BAR_PC34,
                                               slot, bar->zoneId, 0, NULL, 0, 0, 0, 0,
                                               bar->x, bar->y + bar->blankHeight, bar->width,
                                               bar->fillHeight, bar->fillColor)) return 0;
        }
        for (hand = 0; hand < DM1_V1_CHAMPION_TOP_ROW_HAND_COUNT_PC34; ++hand) {
            const Dm1V1ChampionTopRowHandPc34 *handPlan = &plan->hands[hand];
            surface = surface_for_graphic(&assetReceipt->assets, handPlan->graphicIndex);
            if (!surface || !surface->pixels ||
                !append_op(outReceipt, DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34,
                           slot, handPlan->zoneId, handPlan->graphicIndex,
                           surface->pixels, 0, 0, surface->width, surface->height,
                           handPlan->x, handPlan->y, handPlan->width, handPlan->height,
                           -1)) return 0;
        }
    }
    outReceipt->valid = 1;
    return 1;
}
