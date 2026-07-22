#include "dm1_v1_champion_top_row_f0296_transition_pc34_compat.h"

#include <string.h>

static int append_operation(Dm1V1ChampionTopRowF0296TransitionReceiptPc34 *receipt,
                            const Dm1V1ChampionTopRowPresentationReceiptPc34 *presentation,
                            int operationIndex)
{
    const Dm1V1ChampionTopRowPresentationOpPc34 *source;
    Dm1V1ChampionTopRowF0296TransitionOpPc34 *dest;
    if (!receipt || !presentation || operationIndex < 0 ||
        operationIndex >= presentation->operationCount || receipt->operationCount >=
        DM1_V1_CHAMPION_TOP_ROW_F0296_MAX_OPS_PC34) return 0;
    source = &presentation->operations[operationIndex];
    if ((source->kind == DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34 ||
         source->kind == DM1_V1_CHAMPION_TOP_ROW_OP_COMPOSE_ICON_PC34 ||
         source->kind == DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34) &&
        !source->sourcePixels) return 0;
    dest = &receipt->operations[receipt->operationCount++];
    dest->presentationOperationIndex = operationIndex;
    dest->championSlot = source->championSlot;
    dest->zoneId = source->zoneId;
    dest->graphicIndex = source->graphicIndex;
    dest->sourcePixels = source->sourcePixels;
    return 1;
}

static int first_dead_slot(const Dm1V1ChampionTopRowPresentationReceiptPc34 *presentation)
{
    int i;
    for (i = 0; presentation && i < presentation->operationCount; ++i) {
        if (presentation->operations[i].kind ==
            DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34) {
            return presentation->operations[i].championSlot;
        }
    }
    return -1;
}

const char *dm1_v1_champion_top_row_f0296_transition_source_evidence_pc34(void)
{
    return "ReDMCSB CHAMDRAW.C F0296:1208-1210 candidate early return; "
           "F0296:1217-1231 primary status-hand refresh; F0296:1256-1259 "
           "inventory MASK0x4000_VIEWPORT then F0292; F0292:816-842 keeps "
           "dead C008 status presentation and skips live hands/bars.";
}

int dm1_v1_champion_top_row_f0296_transition_from_refresh_pc34(
    const Dm1V1ChampionTopRowPresentationReceiptPc34 *presentation,
    const Dm1V1ChampionPanelHandSlotRefreshStatePc34 *refreshState,
    const Dm1V1ChampionPanelHandSlotRefreshResultPc34 *refreshResult,
    Dm1V1ChampionTopRowF0296TransitionReceiptPc34 *outReceipt)
{
    int i;
    int inventorySlot;
    if (!presentation || !presentation->valid || !refreshState ||
        !refreshResult || !outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->inventoryChampionSlot = -1;
    outReceipt->deadChampionSlot = -1;

    if (refreshResult->path == DM1_V1_DMHSR_PATH_CANDIDATE_EARLY_RETURN_PC34) {
        if (refreshState->candidateChampionOrdinal == 0 ||
            refreshState->inventoryChampionOrdinal != 0) return 0;
        outReceipt->kind = DM1_V1_CHAMPION_TOP_ROW_F0296_CANDIDATE_EARLY_RETURN_PC34;
        outReceipt->candidateSuppressed = 1;
        outReceipt->valid = 1;
        return 1;
    }

    if (refreshResult->path == DM1_V1_DMHSR_PATH_REJECTED_DEAD_MEMBER_PC34) {
        int deadSlot = first_dead_slot(presentation);
        if (deadSlot < 0) return 0;
        outReceipt->kind = DM1_V1_CHAMPION_TOP_ROW_F0296_DEAD_STATUS_REPAINT_PC34;
        outReceipt->deadChampionSlot = deadSlot;
        for (i = 0; i < presentation->operationCount; ++i) {
            if (presentation->operations[i].championSlot == deadSlot &&
                !append_operation(outReceipt, presentation, i)) return 0;
        }
        outReceipt->valid = outReceipt->operationCount > 0;
        return outReceipt->valid;
    }

    if (!refreshResult->accepted) return 0;
    inventorySlot = refreshState->inventoryChampionOrdinal - 1;
    if (refreshState->inventoryChampionOrdinal != 0 &&
        refreshResult->f0292DrawStateDispatched) {
        if (inventorySlot < 0 || inventorySlot >= 4) return 0;
        outReceipt->kind = DM1_V1_CHAMPION_TOP_ROW_F0296_INVENTORY_F0292_REPAINT_PC34;
        outReceipt->inventoryChampionSlot = inventorySlot;
        for (i = 0; i < presentation->operationCount; ++i) {
            if (presentation->operations[i].championSlot == inventorySlot &&
                !append_operation(outReceipt, presentation, i)) return 0;
        }
        outReceipt->valid = outReceipt->operationCount > 0;
        return outReceipt->valid;
    }

    outReceipt->kind = DM1_V1_CHAMPION_TOP_ROW_F0296_CHANGED_HAND_PC34;
    for (i = 0; i < refreshResult->partyChampionCount && i < 4; ++i) {
        int op;
        if (!refreshState->slotBoxWalkF0295Dispatched[i] ||
            refreshState->slotBoxWalkInventorySkip[i]) continue;
        for (op = 0; op < presentation->operationCount; ++op) {
            const Dm1V1ChampionTopRowPresentationOpPc34 *candidate =
                &presentation->operations[op];
            if (candidate->championSlot == i && candidate->kind ==
                DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34 && candidate->zoneId ==
                212 + i * 2) {
                if (!append_operation(outReceipt, presentation, op)) return 0;
                break;
            }
        }
    }
    outReceipt->valid = outReceipt->operationCount > 0;
    return outReceipt->valid;
}
