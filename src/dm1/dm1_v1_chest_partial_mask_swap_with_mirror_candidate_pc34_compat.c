#include "dm1/dm1_v1_chest_partial_mask_swap_with_mirror_candidate_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors:
 * CHEST.C F0333:30-67 opens/materializes G0425 and guards transitive close.
 * CHEST.C F0334:113-132 clears G0426, clears G0425, and relinks non-empty
 * entries in order.
 * CHAMPION.C F0297/F0298:243-298 refresh leader hand, pointer, name, load,
 * and champion state.
 * CHAMPION.C F0300/F0301:511-515,606-614 route C30+ slots through G0425.
 * CHAMPION.C F0302:688-710 owns the leader-hand/slot swap and load refresh.
 * COMMAND.C F0359:1985-1990 gives the C040 candidate panel the input only
 * when the leader hand is empty.
 * CHAMDRAW.C F0293:1117-1143 redraws champion states at candidate boundary.
 * REVIVE.C F0282:744-806 clears pending candidate confirm/cancel state.
 * OBJECT.C F0033:147-212 refreshes object icon/name data after swaps.
 * BLITMASK.C F0133:30-33 is the partial-mask icon blit dispatch boundary.
 */

static const Dm1V1ChestPartialMaskSwapWithMirrorCandidateEvidencePc34
    s_evidence = {
        "ReDMCSB CHEST.C F0333:30-67 open dispatch, leader-hand pickup, "
        "and cross-chest transitive close materialize G0425.",
        "ReDMCSB CHEST.C F0334:113-132 clears G0426, clears non-empty "
        "G0425 entries, and recompacts the linked container chain.",
        "ReDMCSB CHAMPION.C F0297/F0298:243-298 put/remove the leader-hand "
        "object and refresh pointer, object name, load, and champion state.",
        "ReDMCSB CHAMPION.C F0300:511-515 removes C30+ chest slots through "
        "G0425_aT_ChestSlots.",
        "ReDMCSB CHAMPION.C F0301:606-614 adds C30+ chest slots through "
        "G0425_aT_ChestSlots and updates load.",
        "ReDMCSB CHAMPION.C F0302:688-710 checks slot/hand, mask overlap, "
        "swap order, and encumbrance refresh.",
        "ReDMCSB COMMAND.C F0359:1985-1990 M568/C040 consumes panel input "
        "only while the leader hand is empty.",
        "ReDMCSB CHAMDRAW.C F0293:1117-1143 redraws all champion states at "
        "the candidate icon click boundary.",
        "ReDMCSB REVIVE.C F0282:744-806 clears pending candidates on "
        "confirm/cancel.",
        "ReDMCSB OBJECT.C F0033:147-212 refreshes object pointer/icon/name "
        "identity for the swapped thing.",
        "ReDMCSB BLITMASK.C F0133:30-33 dispatches partial masked bitmap "
        "drawing for icon updates.",
        "contract-only synthetic runtime regression; no real assets, "
        "savegame, pixel parity, dungeon sensors, or UI pixels are loaded."
    };

static int item_for_slot(int slotIndex)
{
    return DM1_V1_CPSWMC_SLOT0_ITEM_PC34 + slotIndex;
}

static int weight_for_slot(int slotIndex)
{
    return 2 + slotIndex;
}

void dm1_v1_chest_partial_mask_swap_with_mirror_candidate_init_pc34(
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelContent = DM1_V1_CPSWMC_PANEL_M568_C040_PC34;
    state->c040PanelOpen = 1;
    state->candidateChampionOrdinal =
        DM1_V1_CPSWMC_CANDIDATE_ORDINAL_PC34;
    state->inventoryChampionOrdinal =
        DM1_V1_CPSWMC_INVENTORY_ORDINAL_PC34;
    state->partyChampionCount =
        DM1_V1_CPSWMC_PARTY_COUNT_WITH_CANDIDATE_PC34;
    state->leaderIndex = 0;
    state->selectedChampionIndex = 0;
    state->leaderHandThing = DM1_V1_CPSWMC_LEADER_ITEM_PC34;
    state->leaderHandAllowedMask = DM1_V1_CPSWMC_PARTIAL_ALLOWED_MASK_PC34;
    state->leaderHandWeight = 13;
    state->openChestThing = DM1_V1_CPSWMC_CHEST_THING_PC34;
    state->championLoads[0] = state->leaderHandWeight;
    for (i = 0; i < DM1_V1_CPSWMC_SLOT_COUNT_PC34; ++i) {
        state->g0425[i] = item_for_slot(i);
        state->g0425AllowedMasks[i] = DM1_PC34_ALLOWED_CONTAINER;
        state->g0425Weights[i] = weight_for_slot(i);
    }
}

static int g0425_equal(const int *a, const int *b)
{
    int i;

    for (i = 0; i < DM1_V1_CPSWMC_SLOT_COUNT_PC34; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int chest_open(const Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34
                          *state)
{
    return state && state->openChestThing != DM1_V1_CPSWMC_NONE_PC34;
}

static int partial_mask_can_enter_chest_slot(
    const Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state)
{
    const int slotMask =
        m11_inventory_pc34_slot_mask(DM1_V1_CPSWMC_TARGET_PC34_SLOT_PC34);

    return state &&
           (state->leaderHandAllowedMask & slotMask) == slotMask;
}

static void close_and_recompact_chest(
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state,
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 *result)
{
    int i;

    if (!state || !result || !chest_open(state)) {
        return;
    }

    ++state->f0334CloseCount;
    state->openChestThing = DM1_V1_CPSWMC_NONE_PC34;
    result->closedChainCount = 0;
    for (i = 0; i < DM1_V1_CPSWMC_SLOT_COUNT_PC34; ++i) {
        if (state->g0425[i] != DM1_V1_CPSWMC_NONE_PC34) {
            result->closedChain[result->closedChainCount++] = state->g0425[i];
            state->g0425[i] = DM1_V1_CPSWMC_NONE_PC34;
            ++state->g0425RecompactCount;
        }
    }
}

static void clear_candidate_confirm_or_cancel(
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state)
{
    if (!state || state->candidateChampionOrdinal == 0u) {
        return;
    }
    state->candidateChampionOrdinal = 0u;
    ++state->candidateClearCount;
    if (state->partyChampionCount > 0u) {
        --state->partyChampionCount;
    }
    state->panelContent = 0;
    state->c040PanelOpen = 0;
    ++state->f0293RedrawAllCount;
    ++state->f0292DrawStateCount;
}

static int attempt_partial_mask_swap(
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state,
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 *result)
{
    const int slotIndex = DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34;
    const int selected = state ? state->selectedChampionIndex : 0;
    int slotThing;
    int slotWeight;
    int leaderThing;
    int leaderWeight;

    if (!state || !result) {
        return 0;
    }

    ++state->f0359PanelGateCount;
    if (state->candidateChampionOrdinal != 0u &&
        state->leaderHandThing == DM1_V1_CPSWMC_NONE_PC34) {
        result->candidateOwnedInput = 1;
        result->rejected = 1;
        return 0;
    }
    if (!chest_open(state)) {
        result->candidateOwnedInput =
            state->candidateChampionOrdinal != 0u ? 1 : 0;
        result->rejected = 1;
        return 0;
    }
    if (state->leaderHandThing == DM1_V1_CPSWMC_NONE_PC34 ||
        state->g0425[slotIndex] == DM1_V1_CPSWMC_NONE_PC34 ||
        !partial_mask_can_enter_chest_slot(state)) {
        result->rejected = 1;
        return 0;
    }

    ++state->f0133PartialMaskDispatchCount;
    ++state->f0302DispatchCount;
    result->partialMaskDispatched = 1;
    slotThing = state->g0425[slotIndex];
    slotWeight = state->g0425Weights[slotIndex];
    leaderThing = state->leaderHandThing;
    leaderWeight = state->leaderHandWeight;

    ++state->f0298LeaderHandRemoveCount;
    ++state->f0300ChestSlotRemoveCount;
    state->leaderHandThing = slotThing;
    state->leaderHandAllowedMask = DM1_PC34_ALLOWED_CONTAINER;
    state->leaderHandWeight = slotWeight;
    ++state->f0297LeaderHandPutCount;
    state->g0425[slotIndex] = leaderThing;
    state->g0425AllowedMasks[slotIndex] =
        DM1_V1_CPSWMC_PARTIAL_ALLOWED_MASK_PC34;
    state->g0425Weights[slotIndex] = leaderWeight;
    ++state->f0301ChestSlotAddCount;
    ++state->f0302SwapCount;
    ++state->f0302EncumbranceRefreshCount;
    ++state->objectPointerRefreshCount;
    ++state->objectNameRefreshCount;
    ++state->f0292DrawStateCount;
    if (selected >= 0 && selected < 4) {
        state->championLoads[selected] =
            state->championLoads[selected] - slotWeight + leaderWeight;
    }
    if (state->leaderIndex >= 0 && state->leaderIndex < 4) {
        state->championLoads[state->leaderIndex] =
            state->championLoads[state->leaderIndex] - leaderWeight +
            slotWeight;
    }
    result->accepted = 1;
    return 1;
}

static void capture_before(
    const Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state,
    int caseId,
    int command,
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->caseId = caseId;
    result->command = command;
    result->requestedPc34Slot = DM1_V1_CPSWMC_TARGET_PC34_SLOT_PC34;
    result->requestedChestSlotIndex = DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34;
    result->maskOverlap =
        DM1_V1_CPSWMC_PARTIAL_ALLOWED_MASK_PC34 &
        m11_inventory_pc34_slot_mask(DM1_V1_CPSWMC_TARGET_PC34_SLOT_PC34);
    result->maskExactMatch =
        DM1_V1_CPSWMC_PARTIAL_ALLOWED_MASK_PC34 ==
        m11_inventory_pc34_slot_mask(DM1_V1_CPSWMC_TARGET_PC34_SLOT_PC34);
    if (!state) {
        return;
    }
    result->candidateWasActive =
        state->candidateChampionOrdinal != 0u ? 1 : 0;
    result->chestWasOpen = chest_open(state);
    result->slotBefore = state->g0425[DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34];
    result->leaderHandBefore = state->leaderHandThing;
    result->openChestBefore = state->openChestThing;
    result->candidateBefore = state->candidateChampionOrdinal;
    result->partyCountBefore = state->partyChampionCount;
    result->selectedChampionBefore = state->selectedChampionIndex;
    result->selectedLoadBefore =
        state->championLoads[state->selectedChampionIndex];
    result->leaderLoadBefore = state->championLoads[state->leaderIndex];
}

static void capture_after(
    const Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state,
    const Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *before,
    const int beforeG0425[DM1_V1_CPSWMC_SLOT_COUNT_PC34],
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 *result)
{
    if (!state || !before || !result) {
        return;
    }
    result->slotAfter = state->g0425[DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34];
    result->leaderHandAfter = state->leaderHandThing;
    result->openChestAfter = state->openChestThing;
    result->candidateAfter = state->candidateChampionOrdinal;
    result->partyCountAfter = state->partyChampionCount;
    result->selectedChampionAfter = state->selectedChampionIndex;
    result->selectedLoadAfter =
        state->championLoads[state->selectedChampionIndex];
    result->leaderLoadAfter = state->championLoads[state->leaderIndex];
    result->f0359PanelGateDelta =
        state->f0359PanelGateCount - before->f0359PanelGateCount;
    result->f0133PartialMaskDispatchDelta =
        state->f0133PartialMaskDispatchCount -
        before->f0133PartialMaskDispatchCount;
    result->f0302DispatchDelta =
        state->f0302DispatchCount - before->f0302DispatchCount;
    result->f0302SwapDelta = state->f0302SwapCount - before->f0302SwapCount;
    result->f0297LeaderHandPutDelta =
        state->f0297LeaderHandPutCount - before->f0297LeaderHandPutCount;
    result->f0298LeaderHandRemoveDelta =
        state->f0298LeaderHandRemoveCount -
        before->f0298LeaderHandRemoveCount;
    result->f0300ChestSlotRemoveDelta =
        state->f0300ChestSlotRemoveCount -
        before->f0300ChestSlotRemoveCount;
    result->f0301ChestSlotAddDelta =
        state->f0301ChestSlotAddCount - before->f0301ChestSlotAddCount;
    result->f0302EncumbranceRefreshDelta =
        state->f0302EncumbranceRefreshCount -
        before->f0302EncumbranceRefreshCount;
    result->f0334CloseDelta =
        state->f0334CloseCount - before->f0334CloseCount;
    result->g0425RecompactDelta =
        state->g0425RecompactCount - before->g0425RecompactCount;
    result->candidateClearDelta =
        state->candidateClearCount - before->candidateClearCount;
    result->f0293RedrawAllDelta =
        state->f0293RedrawAllCount - before->f0293RedrawAllCount;
    result->f0292DrawStateDelta =
        state->f0292DrawStateCount - before->f0292DrawStateCount;
    result->objectPointerRefreshDelta =
        state->objectPointerRefreshCount -
        before->objectPointerRefreshCount;
    result->objectNameRefreshDelta =
        state->objectNameRefreshCount - before->objectNameRefreshCount;
    result->g0425Unchanged = g0425_equal(beforeG0425, state->g0425);
    result->leaderHandPreserved =
        result->leaderHandBefore == result->leaderHandAfter;
    result->noSideEffects =
        result->g0425Unchanged && result->leaderHandPreserved &&
        result->f0133PartialMaskDispatchDelta == 0 &&
        result->f0302DispatchDelta == 0 &&
        result->f0302SwapDelta == 0 &&
        result->objectPointerRefreshDelta == 0 &&
        result->objectNameRefreshDelta == 0;
}

int dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_case_pc34(
    int caseId,
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 *outResult)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 state;
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 before;
    int beforeG0425[DM1_V1_CPSWMC_SLOT_COUNT_PC34];
    int command = DM1_V1_CPSWMC_C38_CHEST_SLOT_1_PC34 +
                  DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34;

    if (!outResult) {
        return 0;
    }
    dm1_v1_chest_partial_mask_swap_with_mirror_candidate_init_pc34(&state);
    if (caseId == DM1_V1_CPSWMC_CASE_CANCEL_PC34) {
        command = DM1_V1_CPSWMC_COMMAND_CANCEL_PC34;
    } else if (caseId == DM1_V1_CPSWMC_CASE_NO_CANDIDATE_PC34) {
        state.candidateChampionOrdinal = 0u;
        state.c040PanelOpen = 0;
        state.panelContent = 0;
        state.partyChampionCount = 1u;
    } else if (caseId == DM1_V1_CPSWMC_CASE_CROSS_CHAMPION_PC34) {
        state.selectedChampionIndex = 1;
        state.inventoryChampionOrdinal = 2u;
        state.championLoads[1] = 21;
    } else if (caseId == DM1_V1_CPSWMC_CASE_EMPTY_HAND_CANDIDATE_PC34) {
        state.leaderHandThing = DM1_V1_CPSWMC_NONE_PC34;
        state.leaderHandWeight = 0;
        state.championLoads[0] = 0;
    } else if (caseId == DM1_V1_CPSWMC_CASE_CLOSED_CHEST_CANDIDATE_PC34) {
        state.openChestThing = DM1_V1_CPSWMC_NONE_PC34;
    }

    before = state;
    memcpy(beforeG0425, state.g0425, sizeof(beforeG0425));
    capture_before(&state, caseId, command, outResult);

    if (caseId == DM1_V1_CPSWMC_CASE_CANCEL_PC34) {
        ++state.f0359PanelGateCount;
        outResult->candidateOwnedInput = 1;
        clear_candidate_confirm_or_cancel(&state);
        outResult->accepted = 1;
    } else {
        (void)attempt_partial_mask_swap(&state, outResult);
        if (caseId == DM1_V1_CPSWMC_CASE_CONFIRM_PC34 &&
            outResult->accepted) {
            clear_candidate_confirm_or_cancel(&state);
            close_and_recompact_chest(&state, outResult);
        }
    }

    if (caseId == DM1_V1_CPSWMC_CASE_NO_CANDIDATE_PC34) {
        outResult->ordinaryRoute = outResult->accepted;
    }
    if (caseId == DM1_V1_CPSWMC_CASE_CROSS_CHAMPION_PC34) {
        outResult->crossChampionRoute =
            outResult->accepted && state.selectedChampionIndex == 1;
    }
    capture_after(&state, &before, beforeG0425, outResult);
    return outResult->accepted ? 1 : 0;
}

const Dm1V1ChestPartialMaskSwapWithMirrorCandidateEvidencePc34 *
dm1_v1_chest_partial_mask_swap_with_mirror_candidate_evidence_pc34(void)
{
    return &s_evidence;
}

static void self_check(int condition, int *passed, int *failed)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
}

int dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_pc34(
    int *passed,
    int *failed)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result;

    if (!passed || !failed) {
        return 0;
    }
    *passed = 0;
    *failed = 0;

    (void)dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_case_pc34(
        DM1_V1_CPSWMC_CASE_CONFIRM_PC34, &result);
    self_check(result.partialMaskDispatched == 1, passed, failed);
    self_check(result.candidateClearDelta == 1, passed, failed);
    self_check(result.g0425RecompactDelta == DM1_V1_CPSWMC_SLOT_COUNT_PC34,
               passed,
               failed);

    (void)dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_case_pc34(
        DM1_V1_CPSWMC_CASE_CANCEL_PC34, &result);
    self_check(result.candidateOwnedInput == 1, passed, failed);
    self_check(result.g0425Unchanged == 1, passed, failed);
    self_check(result.leaderHandPreserved == 1, passed, failed);

    (void)dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_case_pc34(
        DM1_V1_CPSWMC_CASE_NO_CANDIDATE_PC34, &result);
    self_check(result.ordinaryRoute == 1, passed, failed);
    self_check(result.f0302SwapDelta == 1, passed, failed);

    (void)dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_case_pc34(
        DM1_V1_CPSWMC_CASE_EMPTY_HAND_CANDIDATE_PC34, &result);
    self_check(result.rejected == 1, passed, failed);
    self_check(result.noSideEffects == 1, passed, failed);
    return *failed == 0;
}
