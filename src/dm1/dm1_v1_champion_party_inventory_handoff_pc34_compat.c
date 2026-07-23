#include "dm1_v1_champion_party_inventory_handoff_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include <string.h>

static int surface_is_exact(const Dm1V1ChampionRedrawSurfacePc34 *surface,
                            int graphicIndex, int width, int height)
{
    return surface && DM1_ChampionPanel_AssetSurfaceAccepted(
        surface->graphicIndex, graphicIndex, surface->loaded,
        surface->pixels != NULL, surface->width, surface->height, width, height);
}

static int append_op(Dm1V1ChampionPartyInventoryHandoffReceiptPc34 *receipt,
                     Dm1V1ChampionPartyInventoryHandoffKindPc34 kind,
                     int sourceOperationIndex, int championSlot, int zoneId,
                     int graphicIndex, const uint8_t *sourcePixels,
                     int pendingDamageAmount)
{
    Dm1V1ChampionPartyInventoryHandoffOpPc34 *op;
    if (!receipt || receipt->operationCount >=
        DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_MAX_OPS_PC34) return 0;
    if ((graphicIndex == DM1_GFX_DEAD_CHAMPION ||
         graphicIndex == DM1_GFX_CHAMPION_ICONS ||
         graphicIndex == DM1_GFX_POISONED_LABEL ||
         graphicIndex == 15 || graphicIndex == 16 ||
         graphicIndex == DM1_GFX_SLOT_NORMAL || graphicIndex == DM1_GFX_SLOT_WOUNDED ||
         graphicIndex == DM1_GFX_SLOT_ACTING) && !sourcePixels) return 0;
    op = &receipt->operations[receipt->operationCount++];
    op->kind = kind;
    op->sourceOperationIndex = sourceOperationIndex;
    op->championSlot = championSlot;
    op->zoneId = zoneId;
    op->graphicIndex = graphicIndex;
    op->sourcePixels = sourcePixels;
    op->pendingDamageAmount = pendingDamageAmount;
    return 1;
}

static int redraw_zone_id(const Dm1V1ChampionRedrawPriorityOpPc34 *source)
{
    if (!source) return 0;
    if (source->kind == DM1_V1_CHAMPION_REDRAW_POISON_PC34) {
        return DM1_ZONE_POISONED;
    }
    if (source->kind == DM1_V1_CHAMPION_REDRAW_DAMAGE_PC34) {
        return source->graphicIndex == 16
            ? dm1_v1_champion_inventory_damage_indicator_zone_id_pc34(source->championSlot)
            : dm1_v1_champion_damage_indicator_zone_id_pc34(source->championSlot);
    }
    return 0;
}

const char *dm1_v1_champion_party_inventory_handoff_source_evidence_pc34(void)
{
    return "ReDMCSB INVNTORY.C F0355 toggles inventory ownership; CHAMDRAW.C "
           "F0293:1117-1139 restores party champion states in order; F0292 "
           "owns C008/C028 status material and PANEL.C F0345 plus CHAMPION.C "
           "F0320 own C032/C015/C016 overlays. No source surface may be replaced.";
}

int dm1_v1_champion_party_inventory_handoff_pc34(
    const Dm1V1ChampionTopRowPresentationReceiptPc34 *topRow,
    const Dm1V1ChampionRedrawPriorityReceiptPc34 *redraw,
    const Dm1V1ChampionPartyInventorySwitchPc34 *transition,
    const Dm1V1ChampionRedrawMaterialsPc34 *redrawMaterials,
    Dm1V1ChampionPartyInventoryHandoffReceiptPc34 *outReceipt)
{
    int i;
    if (!topRow || !topRow->valid || !topRow->assetReceipt.valid ||
        !redraw || !redraw->valid || !transition || !redrawMaterials ||
        !outReceipt || transition->partyChampionCount < 0 ||
        transition->partyChampionCount > 4 ||
        transition->inventoryChampionBefore < -1 || transition->inventoryChampionBefore >= 4 ||
        transition->inventoryChampionAfter < -1 || transition->inventoryChampionAfter >= 4 ||
        !surface_is_exact(&redrawMaterials->poisonLabel, DM1_GFX_POISONED_LABEL, 96, 15) ||
        !surface_is_exact(&redrawMaterials->damageSmall, 15, 45, 7) ||
        !surface_is_exact(&redrawMaterials->damageBig, 16, 32, 29)) return 0;

    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->transition = *transition;
    outReceipt->topRowAssets = topRow->assetReceipt;
    outReceipt->redrawMaterials = *redrawMaterials;
    for (i = 0; i < topRow->operationCount; ++i) {
        const Dm1V1ChampionTopRowPresentationOpPc34 *source = &topRow->operations[i];
        if (source->championSlot >= transition->partyChampionCount) continue;
        if (!append_op(outReceipt, DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_TOP_ROW_PC34,
                       i, source->championSlot, source->zoneId, source->graphicIndex,
                       source->sourcePixels, 0)) return 0;
    }
    for (i = 0; i < redraw->operationCount; ++i) {
        const Dm1V1ChampionRedrawPriorityOpPc34 *source = &redraw->operations[i];
        if (source->championSlot >= transition->partyChampionCount) continue;
        if (!append_op(outReceipt, DM1_V1_CHAMPION_PARTY_INVENTORY_HANDOFF_REDRAW_PC34,
                       i, source->championSlot, redraw_zone_id(source), source->graphicIndex,
                       source->sourcePixels, source->pendingDamageAmount)) return 0;
    }
    outReceipt->valid = outReceipt->operationCount > 0;
    return outReceipt->valid;
}
