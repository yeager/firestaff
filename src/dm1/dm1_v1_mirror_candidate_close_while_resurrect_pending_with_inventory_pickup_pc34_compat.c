/* ReDMCSB anchors: COMMAND.C F0359:1985-1990 gates C040/M568 panel clicks
 * on G0415 leader-hand emptiness; REVIVE.C F0280:124-132 opens G0299/C040
 * and F0282:744-806 is the only candidate-consuming route; CHAMPION.C
 * F0297/F0298/F0300/F0301/F0302 move objects between leader hand and C30+
 * slots; CHEST.C F0333/F0334 materialize and rewrite G0425/G0426; PANEL.C
 * F0344/F0345/F0352/F0346/F0347 redraws C040 chrome; DEFS.H:2088 names
 * C30/G0425/G0426/M070/M516/C040 plus C537/M568/G0299.
 */
#include "firestaff/dm1/v1/mirror_candidate/close_while_resurrect_pending_with_inventory_pickup_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    kOriginalCandidateOrdinal = 3,
    kInventoryChampionOrdinal = 3,
    kPartyChampionCount = 3,
    kLeaderIndex = 0,
    kOpenChestThing = 0x6401,
    kPickedThing = 0x7037,
    kOtherThingBase = 0x7200,
    kInitialSeed = 0x736C040u
};

static int gAssertions;
static int gFailures;
static unsigned int gLastHash;

static const Dm1V1MirrorCwrpipEvidencePc34Compat s_evidence = {
    1,
    "COMMAND.C F0359:1985-1990 M568/C040 dispatch is skipped when "
        "G0415_ui_LeaderEmptyHanded is false",
    "REVIVE.C F0280:124-132 opens G0299/C040 candidate only with empty hand "
        "and party room",
    "REVIVE.C F0282:744-806 consumes/clears G0299 only on C160/C161/C162",
    "CHAMPION.C F0297:243-268 and F0298:270-298 leader-hand put/remove",
    "CHAMPION.C F0300:511-584, F0301:606-660, F0302:662-713 C30+ slot "
        "pickup/swap identity",
    "CHEST.C F0333:30-67 and F0334:113-132 G0425/G0426 chest chain",
    "PANEL.C F0344/F0345/F0352 plus F0346/F0347:1619-1657 C040 chrome "
        "redraw while G0299 remains set",
    "DEFS.H:2088 C30/G0425/G0426/M070/M516/C040; C537 visible chest zone; "
        "M568 panel; G0299 candidate ordinal",
    "contract_only=1 close-while-resurrect-pending timing with C537 pickup "
        "before C040 close; distinct from pass698 double-candidate race, "
        "pass732 reselect-while-pending, and pass727/pass728 panel-live "
        "C045/C038 gates"
};

static const char s_source_evidence[] =
    "COMMAND.C F0359:1985-1990 gates M568/C040 dispatch on empty leader hand\n"
    "REVIVE.C F0280:124-132 opens G0299/C040 candidate\n"
    "REVIVE.C F0282:744-806 consumes and clears G0299\n"
    "CHAMPION.C F0297/F0298/F0300/F0301/F0302 move leader hand and C30+ slots\n"
    "CHEST.C F0333:30-67 and F0334:113-132 materialize/rewrite G0425/G0426\n"
    "PANEL.C F0344/F0345/F0352/F0346/F0347:1619-1657 redraw C040 chrome\n"
    "DEFS.H:2088 C30/G0425/G0426/M070/M516/C040; C537/M568/G0299";

static uint32_t mix_hash(uint32_t hash, unsigned int value)
{
    hash ^= (uint32_t)value + UINT32_C(0x9e3779b9) + (hash << 6) +
            (hash >> 2);
    hash *= UINT32_C(16777619);
    return hash;
}

static void update_hash(Dm1V1MirrorCwrpipStatePc34Compat *state,
                        unsigned int tag,
                        unsigned int value)
{
    if (!state) {
        return;
    }
    state->deterministicHash =
        mix_hash(mix_hash(state->deterministicHash, tag), value);
}

static int pending_is_original(
    const Dm1V1MirrorCwrpipStatePc34Compat *state)
{
    return state &&
           state->dominantStep ==
               DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT &&
           state->pendingCandidateOrdinal == state->originalCandidateOrdinal &&
           state->pendingCandidateOrdinal != 0u;
}

static void snapshot_result(
    const Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *result,
    const char *anchor)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->anchor = anchor;
    result->c30Slot0Before = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    result->c30Slot0After = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    result->c537Slot0Before = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    result->c537Slot0After = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    if (!state) {
        return;
    }
    result->stepBefore = state->currentStep;
    result->stepAfter = state->currentStep;
    result->dominantStepAfter = state->dominantStep;
    result->candidateBefore = state->pendingCandidateOrdinal;
    result->candidateAfter = state->pendingCandidateOrdinal;
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->c30Slot0Before = state->c30Chain[0];
    result->c30Slot0After = state->c30Chain[0];
    result->c537Slot0Before = state->c537Chain[0];
    result->c537Slot0After = state->c537Chain[0];
    result->f0282Before = state->f0282CandidateConsumeCount;
    result->f0282After = state->f0282CandidateConsumeCount;
    result->f0359Before = state->f0359C040CloseDispatchCount;
    result->f0359After = state->f0359C040CloseDispatchCount;
    result->deterministicHashAfter = state->deterministicHash;
}

static void finish_result(const Dm1V1MirrorCwrpipStatePc34Compat *state,
                          Dm1V1MirrorCwrpipResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->stepAfter = state->currentStep;
    result->dominantStepAfter = state->dominantStep;
    result->candidateAfter = state->pendingCandidateOrdinal;
    result->leaderHandAfter = state->leaderHandThing;
    result->c30Slot0After = state->c30Chain[0];
    result->c537Slot0After = state->c537Chain[0];
    result->f0282After = state->f0282CandidateConsumeCount;
    result->f0359After = state->f0359C040CloseDispatchCount;
    result->dominantPending = pending_is_original(state);
    result->pickupLandedInLeaderHandFromC30 =
        state->leaderHandThing == kPickedThing &&
        state->pickedThing == kPickedThing &&
        state->pickedSourceC30Slot ==
            DM1_V1_MIRROR_CWRPIP_C30_CHEST_SLOT_PC34_COMPAT &&
        state->pickedSourceC537Zone ==
            DM1_V1_MIRROR_CWRPIP_C537_VISIBLE_SLOT_PC34_COMPAT;
    result->pendingCandidateNotConsumed =
        state->candidateConsumedByPickupCount == 0 &&
        state->candidateConsumedByCloseCount == 0 &&
        state->f0282CandidateConsumeCount == 0 &&
        pending_is_original(state);
    result->closeBlockedByLeaderHand =
        state->f0359C040CloseBlockedByLeaderHandCount > 0 &&
        state->f0359C040CloseDispatchCount == result->f0359Before;
    result->panelRedrewAgainstPickupModifiedChain =
        state->panelRedrawSawLeaderHandThing == kPickedThing &&
        state->panelRedrawSawC30Slot0Thing ==
            DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT &&
        state->panelRedrawSawPendingOrdinal ==
            (int)state->originalCandidateOrdinal &&
        state->f0346DrawC040Count > 1 &&
        state->f0347DrawPanelCount > 1;
    result->reopenRefiredOriginalChampion =
        state->nextOpenRefiredOrdinal == (int)state->originalCandidateOrdinal &&
        pending_is_original(state);
    result->deterministicHashAfter = state->deterministicHash;
}

static void redraw_c040_panel_against_chain(
    Dm1V1MirrorCwrpipStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    ++state->f0334CloseChestCount;
    ++state->f0344PanelChromeCount;
    ++state->f0345PanelChromeCount;
    ++state->f0346DrawC040Count;
    ++state->f0347DrawPanelCount;
    ++state->f0352PanelRedrawCount;
    state->panelContent = DM1_V1_MIRROR_CWRPIP_M568_PANEL_PC34_COMPAT;
    state->panelGraphic = DM1_V1_MIRROR_CWRPIP_C040_PANEL_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->activePanelCandidateOrdinal = state->pendingCandidateOrdinal;
    state->panelRedrawSawLeaderHandThing = state->leaderHandThing;
    state->panelRedrawSawC30Slot0Thing = state->c30Chain[0];
    state->panelRedrawSawPendingOrdinal = (int)state->pendingCandidateOrdinal;
    update_hash(state, 40u, (unsigned int)state->panelRedrawSawLeaderHandThing);
    update_hash(state, 41u, (unsigned int)state->panelRedrawSawC30Slot0Thing);
}

void dm1_v1_mirror_candidate_cwrpip_init_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->deterministicSeed = kInitialSeed;
    state->currentStep =
        DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT;
    state->dominantStep =
        DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT;
    state->originalCandidateOrdinal = kOriginalCandidateOrdinal;
    state->pendingCandidateOrdinal = kOriginalCandidateOrdinal;
    state->activePanelCandidateOrdinal = kOriginalCandidateOrdinal;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->partyChampionCount = kPartyChampionCount;
    state->leaderIndex = kLeaderIndex;
    state->leaderEmptyHanded = 1;
    state->leaderHandThing = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->panelContent = DM1_V1_MIRROR_CWRPIP_M568_PANEL_PC34_COMPAT;
    state->panelGraphic = DM1_V1_MIRROR_CWRPIP_C040_PANEL_PC34_COMPAT;
    state->openChestThing = kOpenChestThing;
    for (i = 0; i < DM1_V1_MIRROR_CWRPIP_CHAIN_COUNT_PC34_COMPAT; ++i) {
        state->c30Chain[i] = kOtherThingBase + i;
        state->c537Chain[i] =
            DM1_V1_MIRROR_CWRPIP_C537_VISIBLE_SLOT_PC34_COMPAT + i;
    }
    state->c30Chain[0] = kPickedThing;
    state->pickedThing = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    state->pickedSourceC30Slot = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    state->pickedSourceC537Zone = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    state->panelRedrawSawLeaderHandThing =
        DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    state->panelRedrawSawC30Slot0Thing = state->c30Chain[0];
    state->panelRedrawSawPendingOrdinal = (int)state->pendingCandidateOrdinal;
    state->f0280CandidateOpenCount = 1;
    state->f0333OpenChestCount = 1;
    state->f0346DrawC040Count = 1;
    state->f0347DrawPanelCount = 1;
    state->deterministicHash = kInitialSeed;
    update_hash(state, 1u, state->pendingCandidateOrdinal);
    update_hash(state, 2u, (unsigned int)state->c30Chain[0]);
}

int dm1_v1_mirror_candidate_cwrpip_c537_pickup_before_close_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *outResult)
{
    Dm1V1MirrorCwrpipResultPc34Compat localResult;
    Dm1V1MirrorCwrpipResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result,
                    "CHAMPION.C F0302:662-713; F0297:243-268; "
                    "DEFS.H C30/C537");
    if (!state || !pending_is_original(state) || !state->leaderEmptyHanded ||
        state->c30Chain[0] == DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT) {
        result->rejected = 1;
        finish_result(state, result);
        return 0;
    }

    /* ReDMCSB: F0302 reads C30+ via G0425 for a C537 chest-zone click,
     * removes that C30 thing, then F0297 places it in the leader hand.
     * This route does not call REVIVE.C F0282, so G0299 remains dominant.
     */
    state->currentStep =
        DM1_V1_MIRROR_CWRPIP_STEP_C537_PICKUP_BEFORE_CLOSE_PC34_COMPAT;
    state->dominantStep =
        DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT;
    ++state->f0302SlotBoxCount;
    ++state->f0300RemoveC30Count;
    ++state->f0297PutLeaderHandCount;
    ++state->c537PickupCount;
    state->pickedThing = state->c30Chain[0];
    state->pickedSourceC30Slot =
        DM1_V1_MIRROR_CWRPIP_C30_CHEST_SLOT_PC34_COMPAT;
    state->pickedSourceC537Zone =
        DM1_V1_MIRROR_CWRPIP_C537_VISIBLE_SLOT_PC34_COMPAT;
    state->leaderHandThing = state->pickedThing;
    state->leaderEmptyHanded = 0;
    state->c30Chain[0] = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    state->c537Chain[0] = DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT;
    update_hash(state, 10u, (unsigned int)state->leaderHandThing);
    result->accepted = 1;
    finish_result(state, result);
    return 1;
}

int dm1_v1_mirror_candidate_cwrpip_c040_close_while_pending_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *outResult)
{
    Dm1V1MirrorCwrpipResultPc34Compat localResult;
    Dm1V1MirrorCwrpipResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result,
                    "COMMAND.C F0359:1985-1990; PANEL.C F0346/F0347:1619-1657");
    if (!state || !pending_is_original(state) || !state->c040PanelOpen) {
        result->rejected = 1;
        finish_result(state, result);
        return 0;
    }

    state->currentStep =
        DM1_V1_MIRROR_CWRPIP_STEP_C040_CLOSE_WHILE_PENDING_PC34_COMPAT;
    state->dominantStep =
        DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT;

    if (!state->leaderEmptyHanded) {
        /* ReDMCSB: COMMAND.C F0359 lines 1985-1990 breaks out before F0282
         * when the leader hand is occupied. The pending G0299 candidate must
         * therefore remain dominant while panel redraw sees the modified
         * C30/G0425 chain produced by the preceding C537 pickup.
         */
        ++state->f0359C040CloseBlockedByLeaderHandCount;
        redraw_c040_panel_against_chain(state);
        update_hash(state, 20u, state->pendingCandidateOrdinal);
        result->accepted = 1;
        finish_result(state, result);
        return 1;
    }

    ++state->f0359C040CloseDispatchCount;
    ++state->f0282CandidateConsumeCount;
    ++state->candidateConsumedByCloseCount;
    state->pendingCandidateOrdinal = 0u;
    state->dominantStep = DM1_V1_MIRROR_CWRPIP_STEP_BOOTSTRAP_PC34_COMPAT;
    result->accepted = 1;
    finish_result(state, result);
    return 1;
}

int dm1_v1_mirror_candidate_cwrpip_next_open_refire_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *outResult)
{
    Dm1V1MirrorCwrpipResultPc34Compat localResult;
    Dm1V1MirrorCwrpipResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result,
                    "REVIVE.C F0280:124-132; PANEL.C F0346/F0347:1619-1657");
    if (!state || !pending_is_original(state)) {
        result->rejected = 1;
        finish_result(state, result);
        return 0;
    }

    /* The next mirror-candidate open reuses the original unconsumed ordinal.
     * This pins close-while-pending behavior, not the pass732 reselect path.
     */
    state->currentStep =
        DM1_V1_MIRROR_CWRPIP_STEP_NEXT_OPEN_REFIRE_PC34_COMPAT;
    state->dominantStep =
        DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT;
    ++state->f0280CandidateOpenCount;
    ++state->f0346DrawC040Count;
    ++state->f0347DrawPanelCount;
    state->nextOpenRefiredOrdinal = (int)state->pendingCandidateOrdinal;
    state->activePanelCandidateOrdinal = state->pendingCandidateOrdinal;
    update_hash(state, 30u, state->pendingCandidateOrdinal);
    result->accepted = 1;
    finish_result(state, result);
    return 1;
}

int dm1_v1_mirror_candidate_cwrpip_drive_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *steps,
    int stepCapacity)
{
    int count = 0;

    if (!state || !steps || stepCapacity < 3) {
        return 0;
    }
    if (!dm1_v1_mirror_candidate_cwrpip_c537_pickup_before_close_pc34_compat(
            state, &steps[count])) {
        return count;
    }
    ++count;
    if (!dm1_v1_mirror_candidate_cwrpip_c040_close_while_pending_pc34_compat(
            state, &steps[count])) {
        return count;
    }
    ++count;
    if (!dm1_v1_mirror_candidate_cwrpip_next_open_refire_pc34_compat(
            state, &steps[count])) {
        return count;
    }
    ++count;
    return count;
}

const Dm1V1MirrorCwrpipEvidencePc34Compat *
dm1_v1_mirror_candidate_cwrpip_evidence_pc34_compat(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_cwrpip_source_evidence_pc34_compat(void)
{
    return s_source_evidence;
}

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

int dm1_v1_mirror_candidate_cwrpip_run_self_test_pc34_compat(void)
{
    Dm1V1MirrorCwrpipStatePc34Compat state;
    Dm1V1MirrorCwrpipResultPc34Compat steps[3];
    const Dm1V1MirrorCwrpipEvidencePc34Compat *evidence =
        dm1_v1_mirror_candidate_cwrpip_evidence_pc34_compat();
    int count;

    gAssertions = 0;
    gFailures = 0;
    gLastHash = 0u;
    dm1_v1_mirror_candidate_cwrpip_init_pc34_compat(&state);

    check_true(evidence != NULL && evidence->contractOnly == 1,
               "contract-only evidence exists",
               "metadata");
    check_true(strstr(evidence->commandC040Anchor, "F0359:1985-1990") != NULL,
               "COMMAND.C F0359 anchor is cited",
               evidence->commandC040Anchor);
    check_true(strstr(evidence->reviveOpenAnchor, "F0280:124-132") != NULL,
               "REVIVE.C F0280 anchor is cited",
               evidence->reviveOpenAnchor);
    check_true(strstr(evidence->reviveFinishAnchor, "F0282:744-806") != NULL,
               "REVIVE.C F0282 anchor is cited",
               evidence->reviveFinishAnchor);
    check_true(strstr(evidence->championHandAnchor, "F0297") != NULL &&
                   strstr(evidence->championSlotAnchor, "F0302:662-713") !=
                       NULL,
               "CHAMPION.C hand and slot anchors are cited",
               "CHAMPION.C");
    check_true(strstr(evidence->chestAnchor, "F0333") != NULL &&
                   strstr(evidence->chestAnchor, "F0334") != NULL,
               "CHEST.C anchors are cited",
               evidence->chestAnchor);
    check_true(strstr(evidence->panelAnchor, "F0346/F0347:1619-1657") != NULL,
               "PANEL.C C040 redraw anchor is cited",
               evidence->panelAnchor);
    check_true(strstr(evidence->defsAnchor, "C30") != NULL &&
                   strstr(evidence->defsAnchor, "C537") != NULL &&
                   strstr(evidence->defsAnchor, "M568") != NULL,
               "DEFS.H C30/C537/M568 anchors are cited",
               evidence->defsAnchor);
    check_true(strstr(evidence->scope, "close-while-resurrect-pending") !=
                   NULL &&
                   strstr(evidence->scope, "pass732") != NULL,
               "scope distinguishes close-while-pending from pass732",
               evidence->scope);

    check_true(state.currentStep ==
                   DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT,
               "fixture starts in STEP_RESURRECT_CANDIDATE_PENDING",
               "REVIVE.C F0280:124-132");
    check_true(pending_is_original(&state),
               "original champion is the pending candidate",
               "REVIVE.C F0280:124-132");
    check_true(state.c30Chain[0] == kPickedThing &&
                   state.leaderEmptyHanded == 1,
               "fixture starts with C537/C30 pickup source and empty hand",
               "CHAMPION.C F0302:662-713");

    count = dm1_v1_mirror_candidate_cwrpip_drive_pc34_compat(
        &state, steps, 3);
    check_true(count == 3,
               "driver executes pickup, close, and next-open steps",
               "state-machine driver");
    check_true(steps[0].dominantPending == 1 &&
                   steps[0].pickupLandedInLeaderHandFromC30 == 1 &&
                   steps[0].pendingCandidateNotConsumed == 1,
               "C537 pickup lands in leader hand without consuming candidate",
               "CHAMPION.C F0297/F0302; REVIVE.C F0282");
    check_true(steps[0].candidateAfter == kOriginalCandidateOrdinal &&
                   steps[0].leaderHandAfter == kPickedThing &&
                   steps[0].c30Slot0After ==
                       DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT,
               "pickup modified C30 chain but left candidate ordinal intact",
               "DEFS.H C30/G0425/C537/G0299");
    check_true(steps[1].dominantPending == 1 &&
                   steps[1].closeBlockedByLeaderHand == 1 &&
                   steps[1].pendingCandidateNotConsumed == 1,
               "C040 close is blocked by occupied leader hand while pending "
               "state remains dominant",
               "COMMAND.C F0359:1985-1990");
    check_true(steps[1].panelRedrewAgainstPickupModifiedChain == 1 &&
                   state.panelRedrawSawLeaderHandThing == kPickedThing &&
                   state.panelRedrawSawC30Slot0Thing ==
                       DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT,
               "C040 panel redraw observes the pickup-modified C30 chain",
               "PANEL.C F0346/F0347:1619-1657; CHEST.C F0334");
    check_true(steps[1].f0282After == steps[1].f0282Before &&
                   state.candidateConsumedByCloseCount == 0,
               "close did not route through F0282",
               "REVIVE.C F0282:744-806");
    check_true(steps[2].reopenRefiredOriginalChampion == 1 &&
                   steps[2].candidateAfter == kOriginalCandidateOrdinal,
               "next mirror-candidate open re-fires original champion",
               "REVIVE.C F0280:124-132");
    check_true(state.f0280CandidateOpenCount == 2 &&
                   state.f0282CandidateConsumeCount == 0 &&
                   state.candidateConsumedByPickupCount == 0,
               "open count advances but no candidate consume path fires",
               "REVIVE.C F0280/F0282");

    gLastHash = mix_hash(state.deterministicHash, (unsigned int)gAssertions);
    gLastHash = mix_hash(gLastHash, (unsigned int)gFailures);
    return gFailures == 0;
}

int dm1_v1_mirror_candidate_cwrpip_assertions_pc34_compat(void)
{
    return gAssertions;
}

int dm1_v1_mirror_candidate_cwrpip_failures_pc34_compat(void)
{
    return gFailures;
}

unsigned int dm1_v1_mirror_candidate_cwrpip_hash_pc34_compat(void)
{
    return gLastHash;
}
