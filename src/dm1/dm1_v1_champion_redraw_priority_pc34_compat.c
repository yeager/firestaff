#include "dm1_v1_champion_redraw_priority_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int material_is_exact(const Dm1V1ChampionRedrawSurfacePc34 *surface,
                             int graphicIndex, int width, int height)
{
    return surface && DM1_ChampionPanel_AssetSurfaceAccepted(
        surface->graphicIndex, graphicIndex, surface->loaded,
        surface->pixels != NULL, surface->width, surface->height, width, height);
}

static int append_op(Dm1V1ChampionRedrawPriorityReceiptPc34 *receipt,
                     Dm1V1ChampionRedrawPriorityKindPc34 kind,
                     int championSlot, int presentationIndex, int graphicIndex,
                     const uint8_t *pixels, int pendingDamageAmount,
                     int x, int y, int width, int height)
{
    Dm1V1ChampionRedrawPriorityOpPc34 *op;
    if (!receipt || receipt->operationCount >=
        DM1_V1_CHAMPION_REDRAW_PRIORITY_MAX_OPS_PC34) return 0;
    op = &receipt->operations[receipt->operationCount];
    op->kind = kind;
    op->championSlot = championSlot;
    op->priority = receipt->operationCount;
    op->presentationOperationIndex = presentationIndex;
    op->graphicIndex = graphicIndex;
    op->sourcePixels = pixels;
    op->pendingDamageAmount = pendingDamageAmount;
    op->x = x; op->y = y; op->width = width; op->height = height;
    ++receipt->operationCount;
    return 1;
}

const char *dm1_v1_champion_redraw_priority_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0293:1117-1139 iterates party champion "
           "states in order; F0292:771-905 draws status/name/bars/hands and "
           "F0292:816-842 terminates live lanes for dead champions; CHAMPION.C "
           "F0320:1744-1775 selects C015 top-row or C016 inventory damage; "
           "PANEL.C F0345:1598-1606 selects C032 POISONED.";
}

int dm1_v1_champion_redraw_priority_from_top_row_pc34(
    const Dm1V1ChampionTopRowPresentationReceiptPc34 *topRow,
    const Dm1V1ChampionRedrawStatePc34 *state,
    const Dm1V1ChampionRedrawMaterialsPc34 *materials,
    Dm1V1ChampionRedrawPriorityReceiptPc34 *outReceipt)
{
    int slot;
    if (!topRow || !topRow->valid || !state || !materials || !outReceipt ||
        state->partyChampionCount < 0 || state->partyChampionCount > 4 ||
        state->inventoryChampionIndex < -1 || state->inventoryChampionIndex >= 4) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    for (slot = 0; slot < state->partyChampionCount; ++slot) {
        int op;
        int alive;
        if (!state->present[slot]) continue;
        alive = state->currentHealth[slot] > 0;
        for (op = 0; op < topRow->operationCount; ++op) {
            const Dm1V1ChampionTopRowPresentationOpPc34 *source =
                &topRow->operations[op];
            if (source->championSlot != slot) continue;
            if ((source->kind == DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34 ||
                 source->kind == DM1_V1_CHAMPION_TOP_ROW_OP_COMPOSE_ICON_PC34 ||
                 source->kind == DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34) &&
                !source->sourcePixels) return 0;
            if (!append_op(outReceipt, DM1_V1_CHAMPION_REDRAW_STATUS_PC34, slot,
                           op, source->graphicIndex, source->sourcePixels,
                           0,
                           source->x, source->y, source->width, source->height)) return 0;
        }
        if (!alive) continue;
        if (state->poisonDose[slot] > 0) {
            DM1_V1_ChampionStatusRectPc34 box;
            if (!material_is_exact(&materials->poisonLabel, DM1_GFX_POISONED_LABEL,
                                   96, 15) ||
                !dm1_v1_champion_poison_label_rect_pc34(slot, 96, 15, &box) ||
            !append_op(outReceipt, DM1_V1_CHAMPION_REDRAW_POISON_PC34, slot,
                           -1, DM1_GFX_POISONED_LABEL, materials->poisonLabel.pixels,
                           0,
                           box.x, box.y, box.w, box.h)) return 0;
        }
        if (state->pendingDamage[slot] > 0) {
            int inventory = slot == state->inventoryChampionIndex;
            int graphic = inventory ? 16 : 15;
            int width = inventory ? 32 : 45;
            int height = inventory ? 29 : 7;
            const Dm1V1ChampionRedrawSurfacePc34 *surface = inventory
                ? &materials->damageBig : &materials->damageSmall;
            DM1_V1_ChampionStatusRectPc34 rect;
            int geometryOk = inventory
                ? dm1_v1_champion_inventory_damage_indicator_rect_pc34(
                    slot, width, height, &rect)
                : dm1_v1_champion_damage_indicator_rect_pc34(slot, width, height, &rect);
            if (state->pendingDamageAmount[slot] <= 0 ||
                !material_is_exact(surface, graphic, width, height) || !geometryOk ||
                !append_op(outReceipt, DM1_V1_CHAMPION_REDRAW_DAMAGE_PC34, slot,
                           -1, graphic, surface->pixels,
                           state->pendingDamageAmount[slot],
                           rect.x, rect.y, rect.w, rect.h)) return 0;
        }
    }
    outReceipt->valid = outReceipt->operationCount > 0;
    return outReceipt->valid;
}
