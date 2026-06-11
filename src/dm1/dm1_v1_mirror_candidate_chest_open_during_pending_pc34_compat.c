#include "dm1_v1_mirror_candidate_chest_open_during_pending_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors (contract-only synthetic runtime regression):
 * CHEST.C F0333:30-32 same G0426_T_OpenChest requests return before any of
 *               F0333:43-76 materialization runs, so G0425_aT_ChestSlots is
 *               preserved and the leader hand is not mutated.
 * CHEST.C F0333:53-76 first-eight G0425 slot materialization runs only when
 *               the new chest differs from G0426.  This gate materializes
 *               eight slot writes and relinks the remaining visible slots.
 * COMMAND.C F0359:1985-1990 M568 panel route.  When a C040 panel is open,
 *               an action-hand click is consumed by F0282 candidate clear
 *               (the panel handles its own internal commands) and then
 *               forwarded to F0333 if a new chest is targeted.
 * REVIVE.C F0280:124-132 F0280 candidate admission requires
 *               G0415_ui_LeaderEmptyHanded and a free party slot.
 * REVIVE.C F0282:744-806 F0282 candidate clear decrements G0305_partyCount
 *               and clears G0299_ui_CandidateChampionOrdinal when C162 is
 *               pressed; this gate models the same clearing effect that
 *               happens when an action-hand click closes a pending C040
 *               panel without choosing a candidate.
 * CHAMPION.C F0297:243-267 F0297 leader-hand put.  This gate does not call
 *               F0297 directly (the click opens a chest, it does not put
 *               something in the leader hand).
 * CHAMPION.C F0298:270-298 F0298 leader-hand remove.  Same: not invoked.
 * DEFS.H C30_SLOT_CHEST_1, G0425_aT_ChestSlots[8], G0426_T_OpenChest,
 *         M070_HAND_SLOT_INDEX, M516_CHAMPIONS, C040_COMMAND_… bindings.
 */

enum {
    kCandidateOrdinal = 2,
    kPartyCountWithCandidate = 2,
    kLeaderIndex = 0
};

static const int kNewChestSlotThings
    [DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SLOT_COUNT_PC34_COMPAT] = {
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT0_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT1_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT2_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT3_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT4_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT5_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT6_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT7_PC34_COMPAT
};

static const int kPriorChestSlotThings
    [DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SLOT_COUNT_PC34_COMPAT] = {
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT0_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT1_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT2_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT3_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT4_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT5_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT6_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT7_PC34_COMPAT
};

static const Dm1V1MirrorCandidateChestOpenDuringPendingEvidencePc34Compat
    s_evidence = {
        "ReDMCSB CHEST.C F0333:30-32 same G0426_T_OpenChest requests return "
        "before any first-eight G0425 materialization runs",
        "ReDMCSB CHEST.C F0333:53-76 first-eight G0425_aT_ChestSlots[0..7] "
        "materialization from the linked container chain, padded with "
        "C0xFFFF_THING_NONE for the remaining visible slots",
        "ReDMCSB COMMAND.C F0359:1985-1990 M568/C040 panel dispatch "
        "consumes the action-hand click and routes the result through F0282",
        "ReDMCSB REVIVE.C F0280:124-132 candidate admission requires a live "
        "leader hand and a free party slot before F0280 proceeds",
        "ReDMCSB REVIVE.C F0282:744-806 C162 cancel clears "
        "G0299_ui_CandidateChampionOrdinal and decrements "
        "G0305_ui_PartyChampionCount; F0282 also clears G0299 on confirm",
        "ReDMCSB CHAMPION.C F0297:243-267 leader-hand put writes "
        "G4055_s_LeaderHandObject.Thing and is not invoked by this gate",
        "ReDMCSB CHAMPION.C F0298:270-298 leader-hand remove clears "
        "G4055_s_LeaderHandObject.Thing and is not invoked by this gate",
        "ReDMCSB DEFS.H C30_SLOT_CHEST_1, G0425_aT_ChestSlots[8], "
        "G0426_T_OpenChest, M070_HAND_SLOT_INDEX, M516_CHAMPIONS, C040_… "
        "bindings that wire F0333 and F0282 to the chest and candidate state",
        "contract-only synthetic runtime regression; no real chest, dungeon, "
        "bitmap, savegame, or asset data is loaded",
        "non-overlap: covers an action-hand click on a NEW C508 chest while "
        "C040 is pending, the SAME-chest early-return path while C040 is "
        "pending, and a NON-chest / wall or open floor cell click while C040 "
        "is pending; distinct from the chest-close-while-pending, "
        "occupied-hand, pending-hand-queue, open-then-reselect, and "
        "full-chain gates"
    };

static void fill_prior_chest_slots(int slots[
    DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SLOT_COUNT_PC34_COMPAT])
{
    int i;
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        slots[i] = kPriorChestSlotThings[i];
    }
}

static int valid_click_kind(int clickKind)
{
    return clickKind ==
               DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT ||
           clickKind ==
               DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT ||
           clickKind ==
               DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NON_CHEST_CELL_PC34_COMPAT ||
           clickKind ==
               DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_ACTION_HAND_CLICK_PC34_COMPAT;
}

static void capture_before(
    const Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state,
    int clickKind,
    int clickedChestThing,
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->clickKind = clickKind;
    result->clickedChestThing = clickedChestThing;
    result->f0333PathTaken =
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT;
    if (!state) {
        result->ignored = 1;
        return;
    }
    result->panelContentBefore = state->panelContent;
    result->panelContentAfter = state->panelContent;
    result->c040OpenBefore = state->c040PanelOpen;
    result->c040OpenAfter = state->c040PanelOpen;
    result->candidateOrdinalBefore = (int)state->candidateChampionOrdinal;
    result->candidateOrdinalAfter = (int)state->candidateChampionOrdinal;
    result->partyCountBefore = (int)state->partyChampionCount;
    result->partyCountAfter = (int)state->partyChampionCount;
    result->currentOpenChestBefore = state->currentOpenChestThing;
    result->currentOpenChestAfter = state->currentOpenChestThing;
    result->priorOpenChestBefore = state->priorOpenChestThing;
    result->priorOpenChestAfter = state->priorOpenChestThing;
    result->newOpenChestBefore = state->newOpenChestThing;
    result->newOpenChestAfter = state->newOpenChestThing;
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->slot0Before = state->currentChestSlots[0];
    result->slot0After = state->currentChestSlots[0];
    result->slot1Before = state->currentChestSlots[1];
    result->slot1After = state->currentChestSlots[1];
    result->slot2Before = state->currentChestSlots[2];
    result->slot2After = state->currentChestSlots[2];
    result->slot3Before = state->currentChestSlots[3];
    result->slot3After = state->currentChestSlots[3];
    result->slot4Before = state->currentChestSlots[4];
    result->slot4After = state->currentChestSlots[4];
    result->slot5Before = state->currentChestSlots[5];
    result->slot5After = state->currentChestSlots[5];
    result->slot6Before = state->currentChestSlots[6];
    result->slot6After = state->currentChestSlots[6];
    result->slot7Before = state->currentChestSlots[7];
    result->slot7After = state->currentChestSlots[7];
    result->candidateClearCountBefore = state->candidateClearCount;
    result->candidateClearCountAfter = state->candidateClearCount;
    result->partyDecrementCountBefore = state->partyDecrementCount;
    result->partyDecrementCountAfter = state->partyDecrementCount;
    result->commandClickConsumeCountBefore = state->commandClickConsumeCount;
    result->commandClickConsumeCountAfter = state->commandClickConsumeCount;
    result->f0333SameOpenEarlyReturnCountBefore =
        state->f0333SameOpenEarlyReturnCount;
    result->f0333SameOpenEarlyReturnCountAfter =
        state->f0333SameOpenEarlyReturnCount;
    result->f0333FirstOpenMaterializeCountBefore =
        state->f0333FirstOpenMaterializeCount;
    result->f0333FirstOpenMaterializeCountAfter =
        state->f0333FirstOpenMaterializeCount;
    result->f0333FirstOpenMaterializeSlotsBefore =
        state->f0333FirstOpenMaterializeSlots;
    result->f0333FirstOpenMaterializeSlotsAfter =
        state->f0333FirstOpenMaterializeSlots;
    result->f0333FirstOpenFirstSlotWriteCountBefore =
        state->f0333FirstOpenFirstSlotWriteCount;
    result->f0333FirstOpenFirstSlotWriteCountAfter =
        state->f0333FirstOpenFirstSlotWriteCount;
    result->f0333FirstOpenRelinkCountBefore =
        state->f0333FirstOpenRelinkCount;
    result->f0333FirstOpenRelinkCountAfter =
        state->f0333FirstOpenRelinkCount;
    result->f0333NoOpenCountBefore = state->f0333NoOpenCount;
    result->f0333NoOpenCountAfter = state->f0333NoOpenCount;
    result->f0297LeaderHandPutCountBefore = state->f0297LeaderHandPutCount;
    result->f0297LeaderHandPutCountAfter = state->f0297LeaderHandPutCount;
    result->f0298LeaderHandRemoveCountBefore =
        state->f0298LeaderHandRemoveCount;
    result->f0298LeaderHandRemoveCountAfter =
        state->f0298LeaderHandRemoveCount;
}

static void capture_after(
    const Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state,
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->panelContentAfter = state->panelContent;
    result->c040OpenAfter = state->c040PanelOpen;
    result->candidateOrdinalAfter = (int)state->candidateChampionOrdinal;
    result->partyCountAfter = (int)state->partyChampionCount;
    result->currentOpenChestAfter = state->currentOpenChestThing;
    result->priorOpenChestAfter = state->priorOpenChestThing;
    result->newOpenChestAfter = state->newOpenChestThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->slot0After = state->currentChestSlots[0];
    result->slot1After = state->currentChestSlots[1];
    result->slot2After = state->currentChestSlots[2];
    result->slot3After = state->currentChestSlots[3];
    result->slot4After = state->currentChestSlots[4];
    result->slot5After = state->currentChestSlots[5];
    result->slot6After = state->currentChestSlots[6];
    result->slot7After = state->currentChestSlots[7];
    result->candidateClearCountAfter = state->candidateClearCount;
    result->partyDecrementCountAfter = state->partyDecrementCount;
    result->commandClickConsumeCountAfter = state->commandClickConsumeCount;
    result->f0333SameOpenEarlyReturnCountAfter =
        state->f0333SameOpenEarlyReturnCount;
    result->f0333FirstOpenMaterializeCountAfter =
        state->f0333FirstOpenMaterializeCount;
    result->f0333FirstOpenMaterializeSlotsAfter =
        state->f0333FirstOpenMaterializeSlots;
    result->f0333FirstOpenFirstSlotWriteCountAfter =
        state->f0333FirstOpenFirstSlotWriteCount;
    result->f0333FirstOpenRelinkCountAfter =
        state->f0333FirstOpenRelinkCount;
    result->f0333NoOpenCountAfter = state->f0333NoOpenCount;
    result->f0297LeaderHandPutCountAfter = state->f0297LeaderHandPutCount;
    result->f0298LeaderHandRemoveCountAfter = state->f0298LeaderHandRemoveCount;
    result->leaderHandMutationObserved =
        result->leaderHandBefore != result->leaderHandAfter;
    result->chestHandSwapObserved =
        result->leaderHandMutationObserved &&
        (result->leaderHandBefore !=
             DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT ||
         result->leaderHandAfter !=
             DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT);
    result->g0425MaterializedFromNewChest =
        result->f0333FirstOpenMaterializeCountAfter ==
            result->f0333FirstOpenMaterializeCountBefore + 1 &&
        result->currentOpenChestAfter ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT &&
        result->slot0After ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT0_PC34_COMPAT &&
        result->slot7After ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT7_PC34_COMPAT;
    result->g0425PreservedFromPriorChest =
        result->f0333SameOpenEarlyReturnCountAfter ==
            result->f0333SameOpenEarlyReturnCountBefore + 1 &&
        result->slot0After == result->slot0Before &&
        result->slot1After == result->slot1Before &&
        result->slot2After == result->slot2Before &&
        result->slot3After == result->slot3Before &&
        result->slot4After == result->slot4Before &&
        result->slot5After == result->slot5Before &&
        result->slot6After == result->slot6Before &&
        result->slot7After == result->slot7Before &&
        result->currentOpenChestAfter == result->currentOpenChestBefore;
    result->noChestOpened =
        result->f0333NoOpenCountAfter ==
            result->f0333NoOpenCountBefore + 1 &&
        result->currentOpenChestAfter == result->currentOpenChestBefore &&
        result->slot0After == result->slot0Before &&
        result->slot1After == result->slot1Before &&
        result->slot2After == result->slot2Before &&
        result->slot3After == result->slot3Before &&
        result->slot4After == result->slot4Before &&
        result->slot5After == result->slot5Before &&
        result->slot6After == result->slot6Before &&
        result->slot7After == result->slot7Before;
    result->candidateCleared =
        result->candidateClearCountAfter ==
            result->candidateClearCountBefore + 1 &&
        result->candidateOrdinalAfter == 0 &&
        result->c040OpenAfter == 0 &&
        result->panelContentAfter == 0;
    result->panelClosed =
        result->c040OpenBefore == 1 && result->c040OpenAfter == 0 &&
        result->panelContentBefore ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_C040_PANEL_PC34_COMPAT &&
        result->panelContentAfter == 0;
}

static void clear_c040_candidate_via_f0282(
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state,
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat *result)
{
    if (state->candidateChampionOrdinal == 0u) {
        return;
    }
    state->candidateChampionOrdinal = 0u;
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    ++state->candidateClearCount;
    if (state->partyChampionCount > 0u) {
        --state->partyChampionCount;
        ++state->partyDecrementCount;
    }
    result->dispatchedF0282 = 1;
    result->clickConsumedByCandidateClear = 1;
    state->clickConsumedByCandidateClear = 1;
}

static void open_new_chest_via_f0333(
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state,
    int newChestThing,
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat *result)
{
    int i;
    int firstSlotSeen = 0;

    /* CHEST.C F0333:30-32 same-open early return path.  In this gate the
     * candidate clear has already reset currentOpenChestThing, so a fresh
     * chest always takes the first-open path; the same-open path is taken
     * only when the caller asks for a chest that equals currentOpenChest.
     */
    if (state->currentOpenChestThing == newChestThing &&
        newChestThing !=
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT) {
        ++state->f0333SameOpenEarlyReturnCount;
        result->f0333PathTaken =
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SAME_OPEN_GUARD_PC34_COMPAT;
        result->dispatchedF0333 = 1;
        result->clickConsumedByChestOpen = 1;
        state->clickConsumedByChestOpen = 1;
        return;
    }

    /* CHEST.C F0333:53-76 first-eight G0425 slot materialization. */
    state->currentOpenChestThing = newChestThing;
    ++state->f0333FirstOpenMaterializeCount;
    result->f0333PathTaken =
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_FIRST_OPEN_PC34_COMPAT;
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        state->currentChestSlots[i] = kNewChestSlotThings[i];
        ++state->f0333FirstOpenMaterializeSlots;
        if (!firstSlotSeen) {
            ++state->f0333FirstOpenFirstSlotWriteCount;
            firstSlotSeen = 1;
        } else {
            ++state->f0333FirstOpenRelinkCount;
        }
    }
    result->dispatchedF0333 = 1;
    result->clickConsumedByChestOpen = 1;
    state->clickConsumedByChestOpen = 1;
}

void DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_C040_PANEL_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->partyChampionCount = kPartyCountWithCandidate;
    state->leaderIndex = kLeaderIndex;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT;
    state->priorOpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT;
    state->newOpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT;
    state->currentOpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT;
    fill_prior_chest_slots(state->currentChestSlots);
}

int DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state,
    int clickKind,
    int clickedChestThing,
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat localResult;
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat *result =
        outResult ? outResult : &localResult;

    capture_before(state, clickKind, clickedChestThing, result);
    if (!state) {
        result->ignored = 1;
        return 0;
    }
    if (!valid_click_kind(clickKind)) {
        result->ignored = 1;
        capture_after(state, result);
        return 0;
    }

    /* COMMAND.C F0359:1985-1990 M568/C040 panel route always consumes the
     * action-hand click — first by the candidate clear, then by F0333 if
     * the click targeted a fresh chest cell. */
    ++state->commandClickConsumeCount;
    result->clickConsumed = 1;
    clear_c040_candidate_via_f0282(state, result);

    if (clickKind ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT ||
        clickKind ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_ACTION_HAND_CLICK_PC34_COMPAT) {
        open_new_chest_via_f0333(
            state,
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
            result);
        result->accepted = 1;
    } else if (clickKind ==
               DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT) {
        /* CHEST.C F0333:30-32 same-open early return path.  Re-target the
         * prior chest that originally opened the C040 panel. */
        open_new_chest_via_f0333(
            state,
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
            result);
        result->accepted = 1;
    } else {
        /* NON_CHEST_CELL: action-hand click on a wall or open floor.  F0333
         * is not invoked; the click is consumed by the candidate clear
         * path. */
        ++state->f0333NoOpenCount;
        result->f0333PathTaken =
            DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NON_CHEST_CELL_PC34_COMPAT;
        result->accepted = 1;
    }

    capture_after(state, result);
    return 1;
}

const Dm1V1MirrorCandidateChestOpenDuringPendingEvidencePc34Compat *
DM1_V1_MirrorCandidateChestOpenDuringPending_EvidencePc34Compat(void)
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

int dm1_v1_mirror_candidate_chest_open_during_pending_run(
    int *passed,
    int *failed)
{
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat state;
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat result;

    if (!passed || !failed) {
        return 0;
    }
    *passed = 0;
    *failed = 0;

    /* Scenario A — new chest opens. */
    DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
        &result);
    self_check(result.clickConsumed == 1, passed, failed);
    self_check(result.dispatchedF0282 == 1, passed, failed);
    self_check(result.dispatchedF0333 == 1, passed, failed);
    self_check(result.g0425MaterializedFromNewChest == 1, passed, failed);
    self_check(result.candidateCleared == 1, passed, failed);
    self_check(result.leaderHandMutationObserved == 0, passed, failed);
    self_check(result.f0333PathTaken ==
                   DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_FIRST_OPEN_PC34_COMPAT,
               passed,
               failed);
    self_check(result.panelClosed == 1, passed, failed);

    /* Scenario B — same chest early return. */
    DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
        &result);
    self_check(result.g0425PreservedFromPriorChest == 1, passed, failed);
    self_check(result.leaderHandMutationObserved == 0, passed, failed);
    self_check(result.candidateCleared == 1, passed, failed);
    self_check(result.f0333PathTaken ==
                   DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SAME_OPEN_GUARD_PC34_COMPAT,
               passed,
               failed);
    self_check(result.f0333FirstOpenMaterializeCountAfter ==
                   result.f0333FirstOpenMaterializeCountBefore,
               passed,
               failed);

    /* Scenario C — non-chest cell. */
    DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NON_CHEST_CELL_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT,
        &result);
    self_check(result.noChestOpened == 1, passed, failed);
    self_check(result.leaderHandMutationObserved == 0, passed, failed);
    self_check(result.candidateCleared == 1, passed, failed);
    self_check(result.f0333PathTaken ==
                   DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NON_CHEST_CELL_PC34_COMPAT,
               passed,
               failed);
    self_check(result.dispatchedF0333 == 0, passed, failed);
    self_check(result.clickConsumedByChestOpen == 0, passed, failed);

    return *failed == 0;
}
