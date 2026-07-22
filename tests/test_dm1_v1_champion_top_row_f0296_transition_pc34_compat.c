#include "dm1_v1_champion_top_row_f0296_transition_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

static void presentation(Dm1V1ChampionTopRowPresentationReceiptPc34 *out)
{
    static const unsigned char pixels[] = { 1 };
    memset(out, 0, sizeof(*out));
    out->valid = 1;
    out->operationCount = 5;
    out->operations[0] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34,
        .championSlot = 0, .zoneId = 212, .graphicIndex = 35, .sourcePixels = pixels };
    out->operations[1] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34,
        .championSlot = 1, .zoneId = 214, .graphicIndex = 33, .sourcePixels = pixels };
    out->operations[2] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34,
        .championSlot = 2, .zoneId = 153, .graphicIndex = 8, .sourcePixels = pixels };
    out->operations[3] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_NAME_PC34,
        .championSlot = 2, .zoneId = 161 };
    out->operations[4] = (Dm1V1ChampionTopRowPresentationOpPc34){
        .kind = DM1_V1_CHAMPION_TOP_ROW_OP_COMPOSE_ICON_PC34,
        .championSlot = 2, .zoneId = 115, .graphicIndex = 28, .sourcePixels = pixels };
}

int main(void)
{
    Dm1V1ChampionTopRowPresentationReceiptPc34 topRow;
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 state;
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 result;
    Dm1V1ChampionTopRowF0296TransitionReceiptPc34 receipt;
    int ok = 1;

    presentation(&topRow);
    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    state.candidateChampionOrdinal = 1;
    result.path = DM1_V1_DMHSR_PATH_CANDIDATE_EARLY_RETURN_PC34;
    ok &= check("candidate early return", dm1_v1_champion_top_row_f0296_transition_from_refresh_pc34(
        &topRow, &state, &result, &receipt) && receipt.candidateSuppressed &&
        receipt.operationCount == 0);

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    result.accepted = 1; result.partyChampionCount = 2;
    state.slotBoxWalkF0295Dispatched[0] = 1;
    ok &= check("changed action hand", dm1_v1_champion_top_row_f0296_transition_from_refresh_pc34(
        &topRow, &state, &result, &receipt) && receipt.operationCount == 1 &&
        receipt.operations[0].zoneId == 212 && receipt.operations[0].graphicIndex == 35);

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    state.inventoryChampionOrdinal = 2;
    result.accepted = 1; result.f0292DrawStateDispatched = 1;
    ok &= check("inventory F0292 repaint", dm1_v1_champion_top_row_f0296_transition_from_refresh_pc34(
        &topRow, &state, &result, &receipt) && receipt.inventoryChampionSlot == 1 &&
        receipt.operationCount == 1 && receipt.operations[0].zoneId == 214);

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    result.path = DM1_V1_DMHSR_PATH_REJECTED_DEAD_MEMBER_PC34;
    ok &= check("dead C008 transition", dm1_v1_champion_top_row_f0296_transition_from_refresh_pc34(
        &topRow, &state, &result, &receipt) && receipt.deadChampionSlot == 2 &&
        receipt.operationCount == 3 && receipt.operations[0].graphicIndex == 8);
    return ok ? 0 : 1;
}
