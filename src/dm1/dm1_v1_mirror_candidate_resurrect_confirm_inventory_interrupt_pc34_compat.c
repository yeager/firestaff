#include "firestaff/dm1/v1/mirror_candidate/resurrect_confirm_inventory_interrupt_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    kCandidateOrdinal = 2,
    kPartyChampionCount = 3,
    kLeaderHandThing = 0x7131,
    kLeaderHandIcon = 0x31,
    kLeaderHandNameId = 0x18,
    kSourceC30Thing = 0x7038,
    kSourceC30Icon = 0x30,
    kInitialHash = 0x738C040
};

static int gAssertions;
static int gFailures;
static int gLastHash;

static const Dm1V1MirrorRciiEvidencePc34Compat s_evidence = {
    1,
    "REVIVE.C F0280:124-132 G0299/C040 candidate open guard",
    "REVIVE.C F0282:744-806 C160/C161 candidate finish and G0299 clear",
    "CHEST.C F0333:30-67 and F0334:113-132 G0425/G0426 chest ownership",
    "CHAMPION.C F0297:243-268 leader-hand object/icon/name metadata",
    "CHAMPION.C F0300:511-584, F0301:606-614, F0302:662-713 C30+ slot swap",
    "COMMAND.C F0359:1985-1990 M568/C040 dispatch; F0380:2045-2156 queue drain",
    "PANEL.C F0346/F0347:1619-1657 C040 redraw owner while G0299 is set",
    "UTAMSCR.C F0077:147-151 and F0078:141-145 screen-update bracket",
    "OBJECT.C F0033:147-212 icon lookup and F0038:395-423 slot icon draw",
    "contract_only=1 pass738 confirm-boundary chest/inventory interrupt; "
        "distinct from pass736 close-while-pending pickup and pass732 reselect "
        "queue coverage"
};

static const char s_source_evidence[] =
    "REVIVE.C F0280:124-132 opens G0299/C040 candidate\n"
    "REVIVE.C F0282:744-806 owns C160/C161 finish and candidate clear\n"
    "CHEST.C F0333:30-67/F0334:113-132 own G0425/G0426\n"
    "CHAMPION.C F0297:243-268/F0300:511-584/F0301:606-614/F0302:662-713\n"
    "COMMAND.C F0359:1985-1990 and F0380:2045-2156\n"
    "PANEL.C F0346/F0347:1619-1657\n"
    "UTAMSCR.C F0077:147-151/F0078:141-145\n"
    "OBJECT.C F0033:147-212/F0038:395-423";

static uint32_t mix_hash(uint32_t hash, uint32_t value)
{
    hash ^= value + UINT32_C(0x9e3779b9) + (hash << 6) + (hash >> 2);
    hash *= UINT32_C(16777619);
    return hash;
}

static void update_hash(Dm1V1MirrorRciiStatePc34Compat *state,
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

static void snapshot_result(const Dm1V1MirrorRciiStatePc34Compat *state,
                            Dm1V1MirrorRciiResultPc34Compat *result,
                            const char *anchor)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->anchor = anchor;
    if (!state) {
        result->candidateBefore = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
        result->candidateAfter = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
        return;
    }
    result->candidateBefore = state->candidateChampionOrdinal;
    result->candidateAfter = state->candidateChampionOrdinal;
    result->pendingFinishBefore = state->pendingFinishCommand;
    result->pendingFinishAfter = state->pendingFinishCommand;
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->leaderHandIconBefore = state->leaderHandIcon;
    result->leaderHandIconAfter = state->leaderHandIcon;
    result->sourceC30Before = state->sourceC30Thing;
    result->sourceC30After = state->sourceC30Thing;
    result->sourceC30IconBefore = state->sourceC30Icon;
    result->sourceC30IconAfter = state->sourceC30Icon;
    result->queuedCommandBefore = state->queuedCommand;
    result->queuedCommandAfter = state->queuedCommand;
    result->panelOwnerBefore = state->panelRedrawOwner;
    result->panelOwnerAfter = state->panelRedrawOwner;
    result->f0282Before = state->f0282FinishCount;
    result->f0282After = state->f0282FinishCount;
    result->deterministicHashAfter = state->deterministicHash;
}

static void finish_result(const Dm1V1MirrorRciiStatePc34Compat *state,
                          Dm1V1MirrorRciiResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->candidateAfter = state->candidateChampionOrdinal;
    result->pendingFinishAfter = state->pendingFinishCommand;
    result->leaderHandAfter = state->leaderHandThing;
    result->leaderHandIconAfter = state->leaderHandIcon;
    result->sourceC30After = state->sourceC30Thing;
    result->sourceC30IconAfter = state->sourceC30Icon;
    result->queuedCommandAfter = state->queuedCommand;
    result->panelOwnerAfter = state->panelRedrawOwner;
    result->f0282After = state->f0282FinishCount;
    result->candidateIdentityPreserved =
        result->candidateBefore == kCandidateOrdinal &&
        (result->candidateAfter == kCandidateOrdinal ||
         result->candidateAfter == DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT) &&
        state->activePanelCandidateOrdinal == kCandidateOrdinal;
    result->handMetadataPreserved =
        result->leaderHandBefore == kLeaderHandThing &&
        result->leaderHandAfter == kLeaderHandThing &&
        result->leaderHandIconBefore == kLeaderHandIcon &&
        result->leaderHandIconAfter == kLeaderHandIcon &&
        state->leaderHandObjectNameId == kLeaderHandNameId;
    result->panelRedrawOwnershipPreserved =
        result->panelOwnerBefore == DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT &&
        result->panelOwnerAfter == DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT &&
        state->panelRedrawSawCandidate == kCandidateOrdinal;
    result->queuedMetadataPreserved =
        state->queuedThing == kSourceC30Thing &&
        state->queuedIcon == kSourceC30Icon;
    result->deterministicHashAfter = state->deterministicHash;
}

void dm1_v1_mirror_candidate_rcii_init_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->activePanelCandidateOrdinal = kCandidateOrdinal;
    state->selectedChampionOrdinal = kCandidateOrdinal;
    state->partyChampionCount = kPartyChampionCount;
    state->leaderEmptyHanded = 0;
    state->leaderHandThing = kLeaderHandThing;
    state->leaderHandIcon = kLeaderHandIcon;
    state->leaderHandObjectNameId = kLeaderHandNameId;
    state->sourceC30Thing = kSourceC30Thing;
    state->sourceC30Icon = kSourceC30Icon;
    state->chestSlot0Thing = kSourceC30Thing;
    state->queuedCommand = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
    state->queuedSlot = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
    state->queuedThing = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
    state->queuedIcon = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
    state->pendingFinishCommand = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
    state->panelContent = DM1_V1_MIRROR_RCII_M568_PANEL_PC34_COMPAT;
    state->panelGraphic = DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT;
    state->panelRedrawOwner = DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT;
    state->panelRedrawSawCandidate = kCandidateOrdinal;
    state->panelRedrawSawThing = kSourceC30Thing;
    state->f0280OpenCount = 1;
    state->f0333OpenChestCount = 1;
    state->f0346DrawC040Count = 1;
    state->f0347DrawPanelCount = 1;
    state->f0033IconLookups = 2;
    state->f0038SlotDraws = 1;
    state->deterministicHash = kInitialHash;
    update_hash(state, 1, state->candidateChampionOrdinal);
    update_hash(state, 2, state->leaderHandThing);
    update_hash(state, 3, state->sourceC30Thing);
}

int dm1_v1_mirror_candidate_rcii_begin_confirm_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state,
    Dm1V1MirrorRciiFinishPc34Compat finish,
    Dm1V1MirrorRciiResultPc34Compat *outResult)
{
    Dm1V1MirrorRciiResultPc34Compat localResult;
    Dm1V1MirrorRciiResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result,
                    "COMMAND.C F0359:1985-1990; PANEL.C F0346/F0347:1619-1657");
    if (!state || state->candidateChampionOrdinal != kCandidateOrdinal ||
        state->pendingFinishCommand != DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT) {
        finish_result(state, result);
        return 0;
    }

    /* ReDMCSB: COMMAND.C F0359 lines 1985-1990 is the only M568/C040
     * dispatch to F0282. This fixture freezes the boundary after command
     * identity is known but before REVIVE.C F0282 clears G0299.
     */
    ++state->f0359PanelDispatchCount;
    state->pendingFinishCommand =
        (finish == DM1_V1_MIRROR_RCII_FINISH_REINCARNATE_PC34_COMPAT)
            ? DM1_V1_MIRROR_RCII_C161_REINCARNATE_PC34_COMPAT
            : DM1_V1_MIRROR_RCII_C160_RESURRECT_PC34_COMPAT;
    state->panelRedrawOwner = DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT;
    state->panelRedrawSawCandidate = state->candidateChampionOrdinal;
    ++state->f0346DrawC040Count;
    ++state->f0347DrawPanelCount;
    update_hash(state, 10, state->pendingFinishCommand);
    result->accepted = 1;
    finish_result(state, result);
    return 1;
}

int dm1_v1_mirror_candidate_rcii_inventory_interrupt_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state,
    Dm1V1MirrorRciiResultPc34Compat *outResult)
{
    Dm1V1MirrorRciiResultPc34Compat localResult;
    Dm1V1MirrorRciiResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result,
                    "CHAMPION.C F0302:662-713; CHEST.C F0333:30-67; "
                    "OBJECT.C F0033:147-212");
    if (!state || state->pendingFinishCommand == DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT) {
        finish_result(state, result);
        return 0;
    }

    /* ReDMCSB: F0302 lines 688-690 reads the same C30/G0425 slot metadata
     * that CHEST.C F0333 published. While the confirm command is pending,
     * keep the click as queued work so REVIVE.C F0282 remains the only owner
     * of G0299 and PANEL.C F0347 keeps C040 redraw ownership.
     */
    state->queuedCommand = DM1_V1_MIRROR_RCII_C038_SLOT_BOX_PC34_COMPAT;
    state->queuedSlot = DM1_V1_MIRROR_RCII_C30_CHEST_SLOT_PC34_COMPAT;
    state->queuedThing = state->sourceC30Thing;
    state->queuedIcon = state->sourceC30Icon;
    ++state->interruptQueuedCount;
    ++state->f0033IconLookups;
    state->panelRedrawOwner = DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT;
    state->panelRedrawSawCandidate = state->candidateChampionOrdinal;
    state->panelRedrawSawThing = state->queuedThing;
    ++state->f0347DrawPanelCount;
    update_hash(state, 20, state->queuedThing);
    update_hash(state, 21, state->leaderHandIcon);
    result->queued = 1;
    finish_result(state, result);
    return 1;
}

int dm1_v1_mirror_candidate_rcii_finish_confirm_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state,
    Dm1V1MirrorRciiResultPc34Compat *outResult)
{
    Dm1V1MirrorRciiResultPc34Compat localResult;
    Dm1V1MirrorRciiResultPc34Compat *result =
        outResult ? outResult : &localResult;

    snapshot_result(state, result,
                    "REVIVE.C F0282:744-806; COMMAND.C F0380:2045-2156");
    if (!state || state->candidateChampionOrdinal != kCandidateOrdinal ||
        state->pendingFinishCommand == DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT) {
        finish_result(state, result);
        return 0;
    }

    /* ReDMCSB: REVIVE.C F0282 lines 785-806 clears G0299 before downstream
     * inventory work can observe a stale candidate; F0380 then drains the
     * queued slot-box click with its original hand/object metadata.
     */
    ++state->f0282FinishCount;
    if (state->pendingFinishCommand ==
        DM1_V1_MIRROR_RCII_C161_REINCARNATE_PC34_COMPAT) {
        ++state->reincarnateFinishCount;
    } else {
        ++state->resurrectFinishCount;
    }
    state->candidateChampionOrdinal = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
    state->pendingFinishCommand = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
    ++state->f0334CloseChestCount;
    if (state->queuedCommand == DM1_V1_MIRROR_RCII_C038_SLOT_BOX_PC34_COMPAT) {
        ++state->f0380QueueDrainCount;
        ++state->f0302SlotBoxCount;
        ++state->f0300RemoveSlotCount;
        ++state->f0297PutLeaderHandCount;
        ++state->interruptDispatchedAfterFinishCount;
        state->chestSlot0Thing = state->queuedThing;
        state->sourceC30Thing = state->queuedThing;
        state->sourceC30Icon = state->queuedIcon;
        state->queuedCommand = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
        state->queuedSlot = DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT;
        ++state->f0077EnableCount;
        ++state->f0038SlotDraws;
        ++state->f0078DisableCount;
        result->dispatched = 1;
    }
    update_hash(state, 30, state->f0282FinishCount);
    update_hash(state, 31, state->interruptDispatchedAfterFinishCount);
    result->accepted = 1;
    finish_result(state, result);
    return 1;
}

const Dm1V1MirrorRciiEvidencePc34Compat *
dm1_v1_mirror_candidate_rcii_evidence_pc34_compat(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_rcii_source_evidence_pc34_compat(void)
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

static void run_path(Dm1V1MirrorRciiFinishPc34Compat finish)
{
    Dm1V1MirrorRciiStatePc34Compat state;
    Dm1V1MirrorRciiResultPc34Compat begin;
    Dm1V1MirrorRciiResultPc34Compat interrupt;
    Dm1V1MirrorRciiResultPc34Compat done;

    dm1_v1_mirror_candidate_rcii_init_pc34_compat(&state);
    check_true(state.candidateChampionOrdinal == kCandidateOrdinal &&
                   state.panelRedrawOwner ==
                       DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT,
               "fixture starts with C040 candidate owner",
               "REVIVE.C F0280:124-132; PANEL.C F0346/F0347:1619-1657");
    check_true(state.leaderHandThing == kLeaderHandThing &&
                   state.leaderHandIcon == kLeaderHandIcon,
               "fixture starts with leader-hand metadata",
               "CHAMPION.C F0297:243-268; OBJECT.C F0033:147-212");

    check_true(dm1_v1_mirror_candidate_rcii_begin_confirm_pc34_compat(
                   &state, finish, &begin) == 1,
               "confirm command reaches C040 boundary",
               "COMMAND.C F0359:1985-1990");
    check_true(begin.candidateIdentityPreserved == 1 &&
                   begin.panelRedrawOwnershipPreserved == 1,
               "candidate and panel owner survive confirm boundary",
               "REVIVE.C F0282:744-806; PANEL.C F0347:1651-1657");

    check_true(dm1_v1_mirror_candidate_rcii_inventory_interrupt_pc34_compat(
                   &state, &interrupt) == 1,
               "chest/inventory click interrupts pending confirm",
               "CHAMPION.C F0302:662-713");
    check_true(interrupt.queued == 1 &&
                   interrupt.candidateIdentityPreserved == 1,
               "interrupt queues without losing candidate identity",
               "COMMAND.C F0380:2045-2156; REVIVE.C F0282:744-806");
    check_true(interrupt.handMetadataPreserved == 1 &&
                   interrupt.queuedMetadataPreserved == 1,
               "interrupt preserves hand and queued object metadata",
               "CHAMPION.C F0297/F0300/F0302; OBJECT.C F0033:147-212");
    check_true(interrupt.panelRedrawOwnershipPreserved == 1 &&
                   state.f0282FinishCount == 0,
               "C040 keeps redraw ownership before F0282",
               "PANEL.C F0346/F0347:1619-1657");

    check_true(dm1_v1_mirror_candidate_rcii_finish_confirm_pc34_compat(
                   &state, &done) == 1,
               "F0282 completes the pending confirm",
               "REVIVE.C F0282:744-806");
    check_true(done.candidateAfter == DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT &&
                   done.dispatched == 1,
               "candidate clears before queued inventory dispatch",
               "REVIVE.C F0282:785; COMMAND.C F0380:2045-2156");
    check_true(done.handMetadataPreserved == 1 &&
                   state.leaderHandThing == kLeaderHandThing &&
                   state.sourceC30Thing == kSourceC30Thing,
               "post-confirm dispatch preserves hand object and C30 source",
               "CHAMPION.C F0297/F0300/F0301/F0302");
    check_true(state.f0077EnableCount == 1 && state.f0078DisableCount == 1 &&
                   state.f0038SlotDraws == 2,
               "queued dispatch keeps redraw bracket and slot draw identity",
               "UTAMSCR.C F0077/F0078; OBJECT.C F0038:395-423");
    check_true((finish == DM1_V1_MIRROR_RCII_FINISH_REINCARNATE_PC34_COMPAT &&
                state.reincarnateFinishCount == 1) ||
                   (finish == DM1_V1_MIRROR_RCII_FINISH_RESURRECT_PC34_COMPAT &&
                    state.resurrectFinishCount == 1),
               "self-test covers resurrect and reincarnate confirm routes",
               "REVIVE.C F0282:785-806");
    gLastHash ^= state.deterministicHash;
}

int dm1_v1_mirror_candidate_rcii_run_self_test_pc34_compat(void)
{
    const Dm1V1MirrorRciiEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_rcii_evidence_pc34_compat();
    const char *source =
        dm1_v1_mirror_candidate_rcii_source_evidence_pc34_compat();

    gAssertions = 0;
    gFailures = 0;
    gLastHash = 0;

    check_true(e != NULL && e->contractOnly == 1,
               "evidence exists and marks contract-only fixture",
               "metadata");
    check_true(strstr(e->reviveOpenAnchor, "F0280:124-132") != NULL &&
                   strstr(e->reviveFinishAnchor, "F0282:744-806") != NULL,
               "evidence cites REVIVE.C F0280/F0282",
               "REVIVE.C F0280/F0282");
    check_true(strstr(e->chestAnchor, "F0333:30-67") != NULL &&
                   strstr(e->chestAnchor, "F0334:113-132") != NULL,
               "evidence cites CHEST.C F0333/F0334",
               e->chestAnchor);
    check_true(strstr(e->championHandAnchor, "F0297:243-268") != NULL &&
                   strstr(e->championSlotAnchor, "F0300:511-584") != NULL &&
                   strstr(e->championSlotAnchor, "F0301:606-614") != NULL &&
                   strstr(e->championSlotAnchor, "F0302:662-713") != NULL,
               "evidence cites CHAMPION.C hand and slot routes",
               "CHAMPION.C F0297/F0300/F0301/F0302");
    check_true(strstr(e->commandAnchor, "F0359:1985-1990") != NULL &&
                   strstr(e->commandAnchor, "F0380:2045-2156") != NULL,
               "evidence cites COMMAND.C dispatch/queue",
               e->commandAnchor);
    check_true(strstr(e->panelAnchor, "F0346/F0347:1619-1657") != NULL,
               "evidence cites PANEL.C C040 redraw owner",
               e->panelAnchor);
    check_true(strstr(e->utamscrAnchor, "F0077:147-151") != NULL &&
                   strstr(e->utamscrAnchor, "F0078:141-145") != NULL,
               "evidence cites UTAMSCR.C redraw bracket",
               e->utamscrAnchor);
    check_true(strstr(e->objectAnchor, "F0033:147-212") != NULL &&
                   strstr(e->objectAnchor, "F0038:395-423") != NULL,
               "evidence cites OBJECT.C identity/draw routes",
               e->objectAnchor);
    check_true(strstr(e->scope, "pass738") != NULL &&
                   strstr(e->scope, "pass736") != NULL,
               "scope distinguishes this pass from pass736",
               e->scope);
    check_true(strstr(source, "REVIVE.C F0282:744-806") != NULL &&
                   strstr(source, "OBJECT.C F0033:147-212") != NULL,
               "source evidence string is populated",
               "source evidence");

    run_path(DM1_V1_MIRROR_RCII_FINISH_RESURRECT_PC34_COMPAT);
    run_path(DM1_V1_MIRROR_RCII_FINISH_REINCARNATE_PC34_COMPAT);
    gLastHash = (int)mix_hash((uint32_t)gLastHash, (uint32_t)gAssertions);
    gLastHash = (int)mix_hash((uint32_t)gLastHash, (uint32_t)gFailures);
    return gFailures == 0;
}

int dm1_v1_mirror_candidate_rcii_assertions_pc34_compat(void)
{
    return gAssertions;
}

int dm1_v1_mirror_candidate_rcii_failures_pc34_compat(void)
{
    return gFailures;
}

int dm1_v1_mirror_candidate_rcii_hash_pc34_compat(void)
{
    return gLastHash;
}
