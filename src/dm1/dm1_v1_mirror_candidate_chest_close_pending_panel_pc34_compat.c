#include "dm1_v1_mirror_candidate_chest_close_pending_panel_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors:
 * COMMAND.C F0380:2174-2183 routes C028..C065 slot boxes through F0302 but
 * gates C007..C011 inventory toggles on !G0299_ui_CandidateChampionOrdinal.
 * CHAMPION.C F0302:688-710 reads G0425 chest slots and swaps the leader hand.
 * PANEL.C F0355:2314-2318 clears G0423 and calls F0334 when inventory closes.
 * CHEST.C F0334:113-132 clears G0426 and relinks non-empty G0425 slots.
 * REVIVE.C F0282:744-757 C162 calls F0355 before clearing G0299/decrementing.
 */

enum {
    kCandidateOrdinal = 2,
    kInventoryOrdinal = 2,
    kPartyCountWithCandidate = 2,
    kLeaderIndex = 0
};

static const Dm1V1MirrorCandidateChestClosePendingPanelEvidencePc34Compat
    s_evidence = {
        "ReDMCSB COMMAND.C F0380:2180-2183 C007..C011 inventory toggle "
        "guarded by !G0299",
        "ReDMCSB COMMAND.C F0380:2174-2178 C028..C065 slot-box dispatch to "
        "F0302 while a leader exists",
        "ReDMCSB CHAMPION.C F0302:688-710 reads G0425 C30+ chest slots and "
        "swaps with the leader hand",
        "ReDMCSB PANEL.C F0355:2314-2318 close inventory clears G0423 and "
        "invokes F0334",
        "ReDMCSB CHEST.C F0334:113-132 no-open return, G0426 clear, visible "
        "slot clear, first-slot write, and relink order",
        "ReDMCSB REVIVE.C F0282:744-757 C162 cancel calls F0355 before "
        "clearing G0299 and decrementing party count",
        "contract-only synthetic runtime regression; no real chest, dungeon, "
        "bitmap, savegame, or asset data is loaded",
        "non-overlap: covers an open G0426/G0425 chest surviving blocked "
        "C011 while C040/G0299 is pending, distinct from portrait clicks, "
        "inventory toggles, close button, occupied hand, right-click pickup, "
        "spell rune, skills, and plain chest relink gates"
    };

static void fill_fixture_slots(int slots[
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT_COUNT_PC34_COMPAT])
{
    slots[0] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT0_THING_PC34_COMPAT;
    slots[1] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    slots[2] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT2_THING_PC34_COMPAT;
    slots[3] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT3_THING_PC34_COMPAT;
    slots[4] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    slots[5] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    slots[6] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    slots[7] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
}

static int valid_slot_index(int chestSlotIndex)
{
    return chestSlotIndex >= 0 &&
           chestSlotIndex <
               DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT_COUNT_PC34_COMPAT;
}

static void capture_before(
    const Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    int command,
    int requestedChestSlotIndex,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->command = command;
    result->requestedChestSlotIndex = requestedChestSlotIndex;
    result->slot1Before =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    result->slot1After = result->slot1Before;
    if (!state) {
        result->ignored = 1;
        return;
    }
    result->openChestBefore = state->openChestThing;
    result->openChestAfter = state->openChestThing;
    result->containerHeadBefore = state->containerHeadThing;
    result->containerHeadAfter = state->containerHeadThing;
    result->slot0Before = state->chestSlots[0];
    result->slot0After = state->chestSlots[0];
    result->slot1Before = state->chestSlots[1];
    result->slot1After = state->chestSlots[1];
    result->slot2Before = state->chestSlots[2];
    result->slot2After = state->chestSlots[2];
    result->slot3Before = state->chestSlots[3];
    result->slot3After = state->chestSlots[3];
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->panelContentBefore = state->panelContent;
    result->panelContentAfter = state->panelContent;
    result->c040OpenBefore = state->c040PanelOpen;
    result->c040OpenAfter = state->c040PanelOpen;
    result->candidateOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateOrdinalAfter = state->candidateChampionOrdinal;
    result->inventoryOrdinalBefore = state->inventoryChampionOrdinal;
    result->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    result->partyCountBefore = state->partyChampionCount;
    result->partyCountAfter = state->partyChampionCount;
    result->f0355ToggleCountBefore = state->f0355ToggleCount;
    result->f0355ToggleCountAfter = state->f0355ToggleCount;
    result->f0334CloseCountBefore = state->f0334CloseCount;
    result->f0334CloseCountAfter = state->f0334CloseCount;
    result->f0302SlotDispatchCountBefore = state->f0302SlotDispatchCount;
    result->f0302SlotDispatchCountAfter = state->f0302SlotDispatchCount;
    result->f0302SwapCountBefore = state->f0302SwapCount;
    result->f0302SwapCountAfter = state->f0302SwapCount;
    result->blockedInventoryCloseCountBefore =
        state->blockedInventoryCloseCount;
    result->blockedInventoryCloseCountAfter =
        state->blockedInventoryCloseCount;
    result->explicitC040CancelCountBefore = state->explicitC040CancelCount;
    result->explicitC040CancelCountAfter = state->explicitC040CancelCount;
    result->candidateClearCountBefore = state->candidateClearCount;
    result->candidateClearCountAfter = state->candidateClearCount;
    result->partyDecrementCountBefore = state->partyDecrementCount;
    result->partyDecrementCountAfter = state->partyDecrementCount;
    result->chestSlotClearCountBefore = state->chestSlotClearCount;
    result->chestSlotClearCountAfter = state->chestSlotClearCount;
    result->chestFirstSlotWriteCountBefore = state->chestFirstSlotWriteCount;
    result->chestFirstSlotWriteCountAfter = state->chestFirstSlotWriteCount;
    result->chestRelinkCountBefore = state->chestRelinkCount;
    result->chestRelinkCountAfter = state->chestRelinkCount;
}

static int first_non_empty_slot(
    const Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state)
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (state->chestSlots[i] !=
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT) {
            return i;
        }
    }
    return DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
}

static void close_chest_via_f0334(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state)
{
    int i;
    int firstIndex;

    if (state->openChestThing ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT) {
        return;
    }

    ++state->f0334CloseCount;
    state->openChestThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    state->containerHeadThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    firstIndex = first_non_empty_slot(state);
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (state->chestSlots[i] !=
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT) {
            if (i == firstIndex) {
                state->containerHeadThing = state->chestSlots[i];
                ++state->chestFirstSlotWriteCount;
            } else {
                ++state->chestRelinkCount;
            }
            state->chestSlots[i] =
                DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
            ++state->chestSlotClearCount;
        }
    }
}

static void close_inventory_via_f0355(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state)
{
    ++state->f0355ToggleCount;
    if (state->inventoryChampionOrdinal != 0u) {
        state->inventoryChampionOrdinal = 0u;
        close_chest_via_f0334(state);
    }
}

static void capture_after(
    const Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->openChestAfter = state->openChestThing;
    result->containerHeadAfter = state->containerHeadThing;
    result->slot0After = state->chestSlots[0];
    result->slot1After = state->chestSlots[1];
    result->slot2After = state->chestSlots[2];
    result->slot3After = state->chestSlots[3];
    result->leaderHandAfter = state->leaderHandThing;
    result->panelContentAfter = state->panelContent;
    result->c040OpenAfter = state->c040PanelOpen;
    result->candidateOrdinalAfter = state->candidateChampionOrdinal;
    result->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    result->partyCountAfter = state->partyChampionCount;
    result->f0355ToggleCountAfter = state->f0355ToggleCount;
    result->f0334CloseCountAfter = state->f0334CloseCount;
    result->f0302SlotDispatchCountAfter = state->f0302SlotDispatchCount;
    result->f0302SwapCountAfter = state->f0302SwapCount;
    result->blockedInventoryCloseCountAfter =
        state->blockedInventoryCloseCount;
    result->explicitC040CancelCountAfter = state->explicitC040CancelCount;
    result->candidateClearCountAfter = state->candidateClearCount;
    result->partyDecrementCountAfter = state->partyDecrementCount;
    result->chestSlotClearCountAfter = state->chestSlotClearCount;
    result->chestFirstSlotWriteCountAfter = state->chestFirstSlotWriteCount;
    result->chestRelinkCountAfter = state->chestRelinkCount;
    result->chestOpenPreserved =
        result->openChestBefore == result->openChestAfter;
    result->chestSlotsPreserved =
        result->slot0Before == result->slot0After &&
        result->slot1Before == result->slot1After &&
        result->slot2Before == result->slot2After &&
        result->slot3Before == result->slot3After;
    result->candidatePreserved =
        result->candidateOrdinalBefore == result->candidateOrdinalAfter &&
        result->partyCountBefore == result->partyCountAfter;
    result->panelPreserved =
        result->panelContentBefore == result->panelContentAfter &&
        result->c040OpenBefore == result->c040OpenAfter;
    result->inventoryPreserved =
        result->inventoryOrdinalBefore == result->inventoryOrdinalAfter;
    result->noChestCloseSideEffects =
        result->f0355ToggleCountBefore == result->f0355ToggleCountAfter &&
        result->f0334CloseCountBefore == result->f0334CloseCountAfter &&
        result->chestSlotClearCountBefore == result->chestSlotClearCountAfter &&
        result->chestFirstSlotWriteCountBefore ==
            result->chestFirstSlotWriteCountAfter &&
        result->chestRelinkCountBefore == result->chestRelinkCountAfter;
    result->closeRepackedNonEmptySlots =
        result->dispatchedF0334 &&
        result->openChestAfter ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT &&
        result->containerHeadAfter !=
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT &&
        result->chestFirstSlotWriteCountAfter ==
            result->chestFirstSlotWriteCountBefore + 1 &&
        result->chestRelinkCountAfter == result->chestRelinkCountBefore + 2 &&
        result->chestSlotClearCountAfter == result->chestSlotClearCountBefore + 3;
}

void DM1_V1_MirrorCandidateChestClosePendingPanel_InitPc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_M568_C040_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->inventoryChampionOrdinal = kInventoryOrdinal;
    state->partyChampionCount = kPartyCountWithCandidate;
    state->leaderIndex = kLeaderIndex;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_LEADER_HAND_PC34_COMPAT;
    state->openChestThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_OPEN_CHEST_PC34_COMPAT;
    state->containerHeadThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT;
    fill_fixture_slots(state->chestSlots);
}

int DM1_V1_MirrorCandidateChestClosePendingPanel_AttemptInventoryClosePc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat localResult;
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *result =
        outResult ? outResult : &localResult;

    capture_before(
        state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C011_CLOSE_INVENTORY_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT,
        result);
    if (!state) {
        return 0;
    }
    if (state->candidateChampionOrdinal != 0u) {
        ++state->blockedInventoryCloseCount;
        result->blockedByCandidate = 1;
        result->ignored = 1;
        capture_after(state, result);
        return 0;
    }

    close_inventory_via_f0355(state);
    result->accepted = 1;
    result->dispatchedF0355 = 1;
    result->dispatchedF0334 =
        result->f0334CloseCountBefore != state->f0334CloseCount;
    capture_after(state, result);
    return 1;
}

int DM1_V1_MirrorCandidateChestClosePendingPanel_SwapChestSlotPc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    int chestSlotIndex,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat localResult;
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *result =
        outResult ? outResult : &localResult;
    int slotThing;

    capture_before(
        state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C038_CHEST_SLOT_1_PC34_COMPAT +
            chestSlotIndex,
        chestSlotIndex,
        result);
    if (!state || !valid_slot_index(chestSlotIndex)) {
        result->ignored = 1;
        return 0;
    }

    ++state->f0302SlotDispatchCount;
    result->dispatchedF0302 = 1;
    slotThing = state->chestSlots[chestSlotIndex];
    if (slotThing ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT &&
        state->leaderHandThing ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT) {
        result->ignored = 1;
        capture_after(state, result);
        return 0;
    }

    state->chestSlots[chestSlotIndex] = state->leaderHandThing;
    state->leaderHandThing = slotThing;
    ++state->f0302SwapCount;
    result->accepted = 1;
    result->leaderHandSwapped = 1;
    capture_after(state, result);
    return 1;
}

int DM1_V1_MirrorCandidateChestClosePendingPanel_CancelC040Pc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat localResult;
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *result =
        outResult ? outResult : &localResult;

    capture_before(
        state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C162_CANCEL_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT,
        result);
    if (!state || state->candidateChampionOrdinal == 0u) {
        result->ignored = 1;
        return 0;
    }

    ++state->explicitC040CancelCount;
    result->explicitC040Cancel = 1;
    close_inventory_via_f0355(state);
    result->dispatchedF0355 = 1;
    result->dispatchedF0334 =
        result->f0334CloseCountBefore != state->f0334CloseCount;
    state->candidateChampionOrdinal = 0u;
    ++state->candidateClearCount;
    if (state->partyChampionCount > 0u) {
        --state->partyChampionCount;
        ++state->partyDecrementCount;
    }
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    result->accepted = 1;
    capture_after(state, result);
    return 1;
}

const Dm1V1MirrorCandidateChestClosePendingPanelEvidencePc34Compat *
DM1_V1_MirrorCandidateChestClosePendingPanel_EvidencePc34Compat(void)
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

int dm1_v1_mirror_candidate_chest_close_pending_panel_run(
    int *passed,
    int *failed)
{
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat state;
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat result;

    if (!passed || !failed) {
        return 0;
    }
    *passed = 0;
    *failed = 0;

    DM1_V1_MirrorCandidateChestClosePendingPanel_InitPc34Compat(&state);
    (void)
        DM1_V1_MirrorCandidateChestClosePendingPanel_AttemptInventoryClosePc34Compat(
            &state, &result);
    self_check(result.blockedByCandidate == 1, passed, failed);
    self_check(result.noChestCloseSideEffects == 1, passed, failed);
    self_check(state.openChestThing ==
                   DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_OPEN_CHEST_PC34_COMPAT,
               passed,
               failed);

    (void)DM1_V1_MirrorCandidateChestClosePendingPanel_SwapChestSlotPc34Compat(
        &state, 0, &result);
    self_check(result.dispatchedF0302 == 1, passed, failed);
    self_check(result.noChestCloseSideEffects == 1, passed, failed);
    self_check(state.leaderHandThing ==
                   DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT0_THING_PC34_COMPAT,
               passed,
               failed);

    (void)DM1_V1_MirrorCandidateChestClosePendingPanel_CancelC040Pc34Compat(
        &state, &result);
    self_check(result.dispatchedF0334 == 1, passed, failed);
    self_check(result.closeRepackedNonEmptySlots == 1, passed, failed);
    self_check(state.candidateChampionOrdinal == 0u, passed, failed);
    return *failed == 0;
}
