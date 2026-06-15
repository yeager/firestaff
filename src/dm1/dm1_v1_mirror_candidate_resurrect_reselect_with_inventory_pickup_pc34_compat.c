#include "dm1_v1_mirror_candidate_resurrect_reselect_with_inventory_pickup_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Contract-only DM1 V1 mirror candidate overlap regression.
 *
 * ReDMCSB anchors:
 * CHAMPION.C F0284:93-131 keeps party/champion direction redraw state stable.
 * CHAMPION.C F0297:243-268 puts an object in the leader hand.
 * CHAMPION.C F0302:662-713 rejects status-box clicks while G0299 is live and
 * then swaps C30+ chest slots through G0425 after the panel is available.
 * REVIVE.C F0280:124-132 opens G0299/C040 only for an empty leader hand and a
 * party below four champions.
 * REVIVE.C F0282:744-806 clears G0299 on cancel or resurrect/reincarnate.
 * PANEL.C F0344/F0345 and F0346/F0347:1619-1657 redraw panel contents and
 * prefer C040 while G0299 is set.
 * COMMAND.C F0359:1985-1990 routes M568/C040 panel clicks to F0282, while
 * COMMAND.C F0380:2045-2156 drains queued work and pending clicks in order.
 * CHEST.C F0333/F0334:30-132 own G0426/G0425 chest materialization/relinking.
 * DEFS.H lines 275, 810, 873/876, 1876/1878, 2200, 3002/3008, 5694, and
 * 5878/5881 define C038, C30, M516, M070, C040, M568, G0299, G0425/G0426.
 */

enum {
    kSelectedChampionOrdinal = 2,
    kPartyCountWithCandidate = 3,
    kSourceC30ScrollThing = 0x7038,
    kExistingChestThing = 0x7401,
    kEmptyHand = 0,
    kInitialHash = 0x4D315231
};

static int gAssertions;
static int gFailures;
static int gLastHash;

static const Dm1V1MirrorCandidateRripEvidencePc34Compat s_evidence = {
    1,
    "CHAMPION.C F0284:93-131 party direction/champion redraw state",
    "CHAMPION.C F0297:243-268 leader-hand object ownership",
    "CHAMPION.C F0302:662-713 G0299 status-click guard and C30+ slot swap",
    "REVIVE.C F0280:124-132 G0299/C040 candidate open guard",
    "REVIVE.C F0282:744-806 C160/C162 candidate finish clears G0299",
    "PANEL.C F0344/F0345 and F0346/F0347:1619-1657 C040 panel redraw",
    "COMMAND.C F0359:1985-1990 M568/C040 panel dispatch to F0282",
    "COMMAND.C F0380:2045-2156 queued command and pending-click drain order",
    "CHEST.C F0333:30-67 G0426/G0425 open/materialize chest slots",
    "CHEST.C F0334:113-132 close/relink G0425 visible chest slots",
    "DEFS.H C038:275 C30:810 C040:2200 G0425:5878 G0426:5881 "
        "M070:1878 M516:873/876 M568:3002/3008 G0299:5694",
    "contract_only=1 resurrect reselect plus in-flight C038 inventory pickup; "
        "distinct from inventory-click-during-rotation, open-then-reselect, "
        "scroll-pickup, pending-hand-queue, and chest-close pending-panel gates"
};

static const char s_source_evidence[] =
    "CHAMPION.C F0284:93-131 party/champion direction redraw state\n"
    "CHAMPION.C F0297:243-268 leader-hand put path\n"
    "CHAMPION.C F0302:662-713 G0299 guard plus C30/G0425 slot-box path\n"
    "REVIVE.C F0280:124-132 G0299/C040 candidate open guard\n"
    "REVIVE.C F0282:744-806 C160/C162 confirm/cancel cleanup\n"
    "PANEL.C F0344/F0345 and F0346/F0347:1619-1657 panel/C040 redraw\n"
    "COMMAND.C F0359:1985-1990 M568/C040 panel dispatch\n"
    "COMMAND.C F0380:2045-2156 queue/pending-click drain\n"
    "CHEST.C F0333:30-67 and F0334:113-132 G0426/G0425 chest ownership\n"
    "DEFS.H C038:275 C30:810 C040:2200 G0425:5878 G0426:5881 "
    "M070:1878 M516:873/876 M568:3002/3008 G0299:5694";

static uint32_t mix_hash(uint32_t hash, uint32_t value)
{
    hash ^= value + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    hash *= 16777619u;
    return hash;
}

static void update_hash(Dm1V1MirrorCandidateRripStatePc34Compat *state,
                        int tag,
                        int value)
{
    uint32_t hash;

    if (!state) {
        return;
    }
    hash = (uint32_t)state->deterministicHash;
    hash = mix_hash(hash, (uint32_t)tag);
    hash = mix_hash(hash, (uint32_t)value);
    state->deterministicHash = (int)hash;
}

static void snapshot_result(
    const Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripResultPc34Compat *result,
    const char *anchor)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->anchor = anchor;
    result->queuedSlotBefore =
        DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
    result->queuedSlotAfter =
        DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
    if (!state) {
        result->candidateBefore =
            DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
        result->candidateAfter =
            DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
        return;
    }
    result->candidateBefore = state->candidateChampionOrdinal;
    result->candidateAfter = state->candidateChampionOrdinal;
    result->selectedBefore = state->selectedChampionOrdinal;
    result->selectedAfter = state->selectedChampionOrdinal;
    result->sourceC30Before = state->sourceC30Thing;
    result->sourceC30After = state->sourceC30Thing;
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->chestSlot0Before = state->chestSlot0Thing;
    result->chestSlot0After = state->chestSlot0Thing;
    result->queuedCommandBefore = state->queuedInventoryCommand;
    result->queuedCommandAfter = state->queuedInventoryCommand;
    result->queuedSlotBefore = state->queuedInventorySlot;
    result->queuedSlotAfter = state->queuedInventorySlot;
    result->f0359PanelDispatchBefore = state->f0359PanelDispatchCount;
    result->f0359PanelDispatchAfter = state->f0359PanelDispatchCount;
    result->f0380QueueDrainBefore = state->f0380QueueDrainCount;
    result->f0380QueueDrainAfter = state->f0380QueueDrainCount;
    result->blockedInventoryBefore = state->blockedInventoryClicks;
    result->blockedInventoryAfter = state->blockedInventoryClicks;
    result->deterministicHashAfter = state->deterministicHash;
}

static void finish_result(
    const Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->candidateAfter = state->candidateChampionOrdinal;
    result->selectedAfter = state->selectedChampionOrdinal;
    result->sourceC30After = state->sourceC30Thing;
    result->leaderHandAfter = state->leaderHandThing;
    result->chestSlot0After = state->chestSlot0Thing;
    result->queuedCommandAfter = state->queuedInventoryCommand;
    result->queuedSlotAfter = state->queuedInventorySlot;
    result->f0359PanelDispatchAfter = state->f0359PanelDispatchCount;
    result->f0380QueueDrainAfter = state->f0380QueueDrainCount;
    result->blockedInventoryAfter = state->blockedInventoryClicks;
    result->candidateBoundToSelectedChampion =
        state->selectedChampionOrdinal == kSelectedChampionOrdinal &&
        (state->candidateChampionOrdinal == kSelectedChampionOrdinal ||
         state->candidateChampionOrdinal == 0);
    result->handPreservedSourceC30 =
        result->sourceC30Before == kSourceC30ScrollThing &&
        result->sourceC30After == kSourceC30ScrollThing &&
        result->leaderHandBefore == kEmptyHand &&
        result->leaderHandAfter == kEmptyHand;
    result->queuedCommandPreserved =
        result->queuedCommandBefore ==
            DM1_V1_MIRROR_CANDIDATE_RRIP_C038_SCROLL_PICKUP_PC34_COMPAT ||
        result->queuedCommandAfter ==
            DM1_V1_MIRROR_CANDIDATE_RRIP_C038_SCROLL_PICKUP_PC34_COMPAT;
    result->dispatchWaitedForCandidateFinish =
        result->f0380QueueDrainAfter > result->f0380QueueDrainBefore &&
        result->candidateBefore != 0 &&
        result->candidateAfter == 0;
    result->deterministicHashAfter = state->deterministicHash;
}

void DM1_V1_MirrorCandidateRrip_InitPc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->partyChampionCount = kPartyCountWithCandidate;
    state->selectedChampionOrdinal = kSelectedChampionOrdinal;
    state->candidateChampionOrdinal = kSelectedChampionOrdinal;
    state->inventoryChampionOrdinal = kSelectedChampionOrdinal;
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_RRIP_M568_PANEL_PC34_COMPAT;
    state->panelGraphic =
        DM1_V1_MIRROR_CANDIDATE_RRIP_C040_GRAPHIC_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->leaderEmptyHanded = 1;
    state->leaderHandThing = kEmptyHand;
    state->sourceC30Thing = kSourceC30ScrollThing;
    state->chestSlot0Thing = kExistingChestThing;
    state->queuedInventoryCommand =
        DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
    state->queuedInventorySlot =
        DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
    state->queuedInventoryThing =
        DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
    state->f0280CandidateOpenCount = 1;
    state->f0333OpenChestCount = 1;
    state->f0346DrawC040Count = 1;
    state->f0347DrawPanelCount = 1;
    state->deterministicHash = kInitialHash;
    update_hash(state, 1, state->candidateChampionOrdinal);
    update_hash(state, 2, state->sourceC30Thing);
}

int DM1_V1_MirrorCandidateRrip_ReselectSameChampionPc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateRripResultPc34Compat localResult;
    Dm1V1MirrorCandidateRripResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result, "REVIVE.C F0280:124-132; "
                                   "PANEL.C F0346/F0347:1619-1657");
    if (!state || state->candidateChampionOrdinal == 0 ||
        state->candidateChampionOrdinal != state->selectedChampionOrdinal) {
        finish_result(state, result);
        return 0;
    }

    /* ReDMCSB: F0280 keeps C040/G0299 candidate state live; a same-portrait
     * reselect reissues the open path without changing the candidate ordinal.
     */
    ++state->reselectCount;
    ++state->reselectReissueCount;
    ++state->f0280CandidateOpenCount;
    ++state->f0346DrawC040Count;
    ++state->f0347DrawPanelCount;
    update_hash(state, 10, state->candidateChampionOrdinal);
    result->accepted = 1;
    finish_result(state, result);
    return 1;
}

int DM1_V1_MirrorCandidateRrip_InventoryClickDuringReselectPc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateRripResultPc34Compat localResult;
    Dm1V1MirrorCandidateRripResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result, "CHAMPION.C F0302:662-713; "
                                   "COMMAND.C F0380:2045-2156");
    if (!state) {
        finish_result(state, result);
        return 0;
    }

    state->queuedInventoryCommand =
        DM1_V1_MIRROR_CANDIDATE_RRIP_C038_SCROLL_PICKUP_PC34_COMPAT;
    state->queuedInventorySlot =
        DM1_V1_MIRROR_CANDIDATE_RRIP_C30_CHEST_SLOT_PC34_COMPAT;
    state->queuedInventoryThing = state->sourceC30Thing;
    ++state->queuedWhileCandidateAlive;
    if (state->candidateChampionOrdinal != 0) {
        /* ReDMCSB: F0302 lines 677-679 refuses status-box hand routing while
         * G0299 is live; F0380 keeps the click pending until C040 is finished.
         */
        ++state->refusedDuringCandidateAlive;
        ++state->blockedInventoryClicks;
        result->blocked = 1;
        result->queued = 1;
        update_hash(state, 20, state->queuedInventoryCommand);
        finish_result(state, result);
        return 0;
    }

    ++state->f0302SlotBoxCount;
    ++state->f0297PutLeaderHandCount;
    state->chestSlot0Thing = state->sourceC30Thing;
    result->accepted = 1;
    result->dispatched = 1;
    finish_result(state, result);
    return 1;
}

static void dispatch_queued_inventory_click(
    Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripResultPc34Compat *result)
{
    if (!state ||
        state->queuedInventoryCommand !=
            DM1_V1_MIRROR_CANDIDATE_RRIP_C038_SCROLL_PICKUP_PC34_COMPAT ||
        state->candidateChampionOrdinal != 0) {
        return;
    }

    /* ReDMCSB: COMMAND.C F0380 lines 2045-2156 drains queued clicks after
     * F0282 clears G0299; F0302 then sees the same C30/G0425 source thing.
     */
    ++state->f0380QueueDrainCount;
    ++state->f0302SlotBoxCount;
    ++state->f0297PutLeaderHandCount;
    ++state->dispatchAfterCandidateFinished;
    state->chestSlot0Thing = state->queuedInventoryThing;
    state->queuedInventoryCommand =
        DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
    state->queuedInventorySlot =
        DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT;
    if (state->sourceC30Thing == kSourceC30ScrollThing &&
        state->leaderHandThing == kEmptyHand) {
        ++state->handPreservedCount;
    }
    update_hash(state, 30, state->chestSlot0Thing);
    if (result) {
        result->dispatched = 1;
    }
}

int DM1_V1_MirrorCandidateRrip_FinishCandidatePc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripFinishPc34Compat finish,
    Dm1V1MirrorCandidateRripResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateRripResultPc34Compat localResult;
    Dm1V1MirrorCandidateRripResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result, "REVIVE.C F0282:744-806; "
                                   "COMMAND.C F0359:1985-1990");
    if (!state || state->candidateChampionOrdinal == 0) {
        finish_result(state, result);
        return 0;
    }

    ++state->f0359PanelDispatchCount;
    ++state->f0282FinishCount;
    if (finish ==
        DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CANCEL_PC34_COMPAT) {
        ++state->cancelRouteCount;
        if (state->partyChampionCount > 0) {
            --state->partyChampionCount;
        }
    } else {
        ++state->resurrectRouteCount;
    }
    state->candidateChampionOrdinal = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    state->c040PanelOpen = 0;
    ++state->f0334CloseChestCount;
    ++state->f0344PanelBarCount;
    ++state->f0345PanelFoodWaterCount;
    update_hash(state, 40, (int)finish);
    dispatch_queued_inventory_click(state, result);
    result->accepted = 1;
    finish_result(state, result);
    return 1;
}

const Dm1V1MirrorCandidateRripEvidencePc34Compat *
DM1_V1_MirrorCandidateRrip_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const char *
DM1_V1_MirrorCandidateRrip_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor);
    }
}

static void run_one_path(Dm1V1MirrorCandidateRripFinishPc34Compat finish)
{
    Dm1V1MirrorCandidateRripStatePc34Compat state;
    Dm1V1MirrorCandidateRripResultPc34Compat reselect;
    Dm1V1MirrorCandidateRripResultPc34Compat inventory;
    Dm1V1MirrorCandidateRripResultPc34Compat done;

    DM1_V1_MirrorCandidateRrip_InitPc34Compat(&state);
    check_true(state.candidateChampionOrdinal == kSelectedChampionOrdinal,
               "fixture opens C040 candidate for selected champion",
               "REVIVE.C F0280:124-132");
    check_true(state.sourceC30Thing == kSourceC30ScrollThing &&
                   state.leaderHandThing == kEmptyHand,
               "fixture starts with source C30 scroll and empty hand",
               "DEFS.H C30:810; CHAMPION.C F0297:243-268");

    check_true(
        DM1_V1_MirrorCandidateRrip_ReselectSameChampionPc34Compat(
            &state, &reselect) == 1,
        "same champion reselect is accepted",
        "REVIVE.C F0280:124-132");
    check_true(reselect.candidateBoundToSelectedChampion == 1 &&
                   state.candidateChampionOrdinal == kSelectedChampionOrdinal,
               "candidate remains bound to same champion after reselect",
               "REVIVE.C F0280:124-132; PANEL.C F0346/F0347:1619-1657");

    check_true(
        DM1_V1_MirrorCandidateRrip_InventoryClickDuringReselectPc34Compat(
            &state, &inventory) == 0,
        "in-flight inventory click is refused while C040 candidate is alive",
        "CHAMPION.C F0302:662-713");
    check_true(inventory.blocked == 1 && inventory.queued == 1,
               "inventory click is queued rather than dispatched",
               "COMMAND.C F0380:2045-2156");
    check_true(state.queuedInventoryCommand ==
                   DM1_V1_MIRROR_CANDIDATE_RRIP_C038_SCROLL_PICKUP_PC34_COMPAT,
               "queued command preserves C038 scroll pickup identity",
               "DEFS.H C038:275; COMMAND.C F0380:2045-2156");
    check_true(state.queuedInventorySlot ==
                   DM1_V1_MIRROR_CANDIDATE_RRIP_C30_CHEST_SLOT_PC34_COMPAT,
               "queued command preserves C30 chest-slot identity",
               "DEFS.H C30:810; CHAMPION.C F0302:689-690");
    check_true(state.sourceC30Thing == kSourceC30ScrollThing &&
                   state.leaderHandThing == kEmptyHand,
               "blocked click preserves source C30 and champion hand",
               "CHAMPION.C F0297:243-268; F0302:704-709");

    check_true(
        DM1_V1_MirrorCandidateRrip_FinishCandidatePc34Compat(
            &state, finish, &done) == 1,
        "candidate finish dispatches through C040 panel route",
        "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806");
    check_true(done.candidateAfter == 0 && state.candidateChampionOrdinal == 0,
               "candidate is cleared before queued inventory dispatch",
               "REVIVE.C F0282:744-806");
    check_true(done.dispatched == 1 &&
                   done.dispatchWaitedForCandidateFinish == 1,
               "queued inventory click dispatches after C040 is gone",
               "COMMAND.C F0380:2045-2156");
    check_true(state.chestSlot0Thing == kSourceC30ScrollThing,
               "queued scroll pickup lands in chest/mirror C30 slot",
               "CHEST.C F0333:30-67; CHAMPION.C F0302:689-709");
    check_true(state.sourceC30Thing == kSourceC30ScrollThing &&
                   state.leaderHandThing == kEmptyHand &&
                   state.handPreservedCount == 1,
               "champion hand state preserves the C30 source thing",
               "CHAMPION.C F0297:243-268; DEFS.H M516:873/876");
    check_true(state.blockedInventoryClicks == 1 &&
                   state.dispatchAfterCandidateFinished == 1,
               "exactly one blocked click becomes one post-candidate dispatch",
               "COMMAND.C F0380:2045-2156");
    check_true((finish ==
                    DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CONFIRM_PC34_COMPAT &&
                state.resurrectRouteCount == 1) ||
                   (finish ==
                        DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CANCEL_PC34_COMPAT &&
                    state.cancelRouteCount == 1),
               "path covers the requested candidate confirm/cancel branch",
               "REVIVE.C F0282:744-806");
    gLastHash ^= state.deterministicHash;
}

int DM1_V1_MirrorCandidateRrip_RunSelfTestPc34Compat(void)
{
    const Dm1V1MirrorCandidateRripEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateRrip_EvidencePc34Compat();
    const char *source = DM1_V1_MirrorCandidateRrip_SourceEvidencePc34Compat();

    gAssertions = 0;
    gFailures = 0;
    gLastHash = 0;

    check_true(e != NULL && e->contractOnly == 1,
               "evidence exists and marks contract-only fixture",
               "metadata");
    check_true(strstr(e->championDirectionAnchor, "F0284:93-131") != NULL,
               "evidence cites CHAMPION.C F0284",
               e->championDirectionAnchor);
    check_true(strstr(e->championLeaderHandAnchor, "F0297:243-268") != NULL,
               "evidence cites CHAMPION.C F0297",
               e->championLeaderHandAnchor);
    check_true(strstr(e->championSlotBoxAnchor, "F0302:662-713") != NULL,
               "evidence cites CHAMPION.C F0302",
               e->championSlotBoxAnchor);
    check_true(strstr(e->reviveOpenAnchor, "F0280:124-132") != NULL,
               "evidence cites REVIVE.C F0280",
               e->reviveOpenAnchor);
    check_true(strstr(e->reviveFinishAnchor, "F0282:744-806") != NULL,
               "evidence cites REVIVE.C F0282",
               e->reviveFinishAnchor);
    check_true(strstr(e->panelDrawAnchor, "F0344/F0345") != NULL &&
                   strstr(e->panelDrawAnchor, "F0346/F0347:1619-1657") != NULL,
               "evidence cites PANEL.C panel routes",
               e->panelDrawAnchor);
    check_true(strstr(e->commandPanelAnchor, "F0359:1985-1990") != NULL,
               "evidence cites COMMAND.C F0359",
               e->commandPanelAnchor);
    check_true(strstr(e->commandQueueAnchor, "F0380:2045-2156") != NULL,
               "evidence cites COMMAND.C F0380",
               e->commandQueueAnchor);
    check_true(strstr(e->chestOpenAnchor, "F0333:30-67") != NULL &&
                   strstr(e->chestCloseAnchor, "F0334:113-132") != NULL,
               "evidence cites CHEST.C F0333/F0334",
               "CHEST.C F0333/F0334");
    check_true(strstr(e->defsAnchor, "C038") != NULL &&
                   strstr(e->defsAnchor, "C30") != NULL &&
                   strstr(e->defsAnchor, "C040") != NULL &&
                   strstr(e->defsAnchor, "G0425") != NULL &&
                   strstr(e->defsAnchor, "G0426") != NULL &&
                   strstr(e->defsAnchor, "M070") != NULL &&
                   strstr(e->defsAnchor, "M516") != NULL,
               "evidence cites requested DEFS.H symbols",
               e->defsAnchor);
    check_true(strstr(e->nonDuplicationScope, "reselect") != NULL &&
                   strstr(e->nonDuplicationScope, "C038") != NULL,
               "non-duplication scope names this overlap path",
               e->nonDuplicationScope);
    check_true(strstr(source, "COMMAND.C F0359:1985-1990") != NULL &&
                   strstr(source, "REVIVE.C F0282:744-806") != NULL,
               "source evidence string includes dispatch and finish anchors",
               "source evidence");

    run_one_path(
        DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CONFIRM_PC34_COMPAT);
    run_one_path(DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CANCEL_PC34_COMPAT);
    gLastHash = (int)mix_hash((uint32_t)gLastHash, (uint32_t)gAssertions);
    gLastHash = (int)mix_hash((uint32_t)gLastHash, (uint32_t)gFailures);
    return gFailures == 0;
}

int DM1_V1_MirrorCandidateRrip_AssertionsPc34Compat(void)
{
    return gAssertions;
}

int DM1_V1_MirrorCandidateRrip_FailuresPc34Compat(void)
{
    return gFailures;
}

int DM1_V1_MirrorCandidateRrip_DeterministicHashPc34Compat(void)
{
    return gLastHash;
}
