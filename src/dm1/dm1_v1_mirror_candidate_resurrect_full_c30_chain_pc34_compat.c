#include "dm1_v1_mirror_candidate_resurrect_full_c30_chain_pc34_compat.h"

#include <stdint.h>
#include <string.h>

enum {
    kCandidateOrdinal = 3,
    kPartyChampionCount = 3,
    kLeaderIndex = 0,
    kFirstC30Thing = 0x7300u,
    kFirstChampionThing = 0x5100u,
    kResurrectedMarkerThing = 0x7A42u,
    kInitialHash = 0x742C30u
};

static int gAssertions;
static int gFailures;
static unsigned int gLastHash;

static const Dm1V1MirrorCandidateResurrectFullC30EvidencePc34Compat
    s_evidence = {
        1,
        "COMMAND.C F0359:1985-1990 M568/C040 dispatch requires "
            "G0415_ui_LeaderEmptyHanded",
        "REVIVE.C F0280:124-132 candidate publish requires empty hand and "
            "party room",
        "REVIVE.C F0282:744-806 C160/C161/C162 consume or clear G0299",
        "CHEST.C F0333:30-67 materializes G0425 and F0334:113-132 rewrites "
            "G0426 from non-empty C30..C37 slots",
        "CHAMPION.C F0284:93-131 party Cell/Direction mutation remains "
            "inactive for this no-rotate gate",
        "CHAMPION.C F0297:243-268 and F0298:270-298 leader-hand "
            "put/remove stay inactive on full-C30 rejection",
        "CHAMPION.C F0300:511-584, F0301:606-614, F0302:662-713 C30 slot "
            "read/swap guards stay non-destructive when no empty C30 slot exists",
        "PANEL.C F0344/F0345 panel click/release, F0352 eye dispatch, and "
            "F0346/F0347:1619-1657 C040 redraw ownership",
        "DEFS.H:810-817 C30..C37, 873/876 M516, 1878 M070, 2088 C10, "
            "2200 C040, 3001-3008 M568/M569, 5876-5881 G0425/G0426",
        "contract_only=1 pass742 full-C30 resurrect rejection; distinct from "
            "pass722/pass723/pass727/pass728/pass732/pass736/pass738 C040 "
            "redraw, close, pickup, pending-hand, and confirm-interrupt gates"
    };

static const char s_source_evidence[] =
    "COMMAND.C F0359:1985-1990 M568/C040 empty-hand dispatch\n"
    "REVIVE.C F0280:124-132 candidate publish guard\n"
    "REVIVE.C F0282:744-806 candidate consume/clear path\n"
    "CHEST.C F0333:30-67 and F0334:113-132 G0425/G0426 C30 chain\n"
    "CHAMPION.C F0284:93-131 party direction\n"
    "CHAMPION.C F0297:243-268/F0298:270-298 leader hand\n"
    "CHAMPION.C F0300:511-584/F0301:606-614/F0302:662-713 slots\n"
    "PANEL.C F0344/F0345/F0352/F0346/F0347:1619-1657\n"
    "DEFS.H:810-817 C30..C37; 873/876 M516; 1878 M070; 2088 C10; "
    "2200 C040; 3001-3008 M568/M569; 5876-5881 G0425/G0426";

static uint32_t fnv1a_step(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & UINT32_C(0xff);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static void update_hash(
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state,
    unsigned int tag,
    unsigned int value)
{
    if (!state) {
        return;
    }
    state->deterministicHash = fnv1a_step(state->deterministicHash, tag);
    state->deterministicHash = fnv1a_step(state->deterministicHash, value);
}

static int c30_has_empty_slot(
    const Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state)
{
    int i;

    if (!state) {
        return 0;
    }
    for (i = 0; i < DM1_V1_MCRFC30_CHAIN_COUNT_PC34_COMPAT; ++i) {
        if (state->c30Chain[i] == DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT) {
            return 1;
        }
    }
    return 0;
}

static int first_empty_c30_slot(
    const Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state)
{
    int i;

    if (!state) {
        return -1;
    }
    for (i = 0; i < DM1_V1_MCRFC30_CHAIN_COUNT_PC34_COMPAT; ++i) {
        if (state->c30Chain[i] == DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT) {
            return i;
        }
    }
    return -1;
}

static void snapshot_result(
    const Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state,
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat *result,
    const char *anchor)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->anchor = anchor;
    result->candidateBefore = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    result->candidateAfter = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    result->leaderHandBefore = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    result->leaderHandAfter = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    result->c30Slot0Before = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    result->c30Slot0After = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    result->c30Slot7Before = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    result->c30Slot7After = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    if (!state) {
        return;
    }
    result->candidateBefore = state->candidateOrdinal;
    result->candidateAfter = state->candidateOrdinal;
    result->panelOpenBefore = state->c040PanelOpen;
    result->panelOpenAfter = state->c040PanelOpen;
    result->leaderHandBefore = state->leaderHandThing;
    result->leaderHandAfter = state->leaderHandThing;
    result->c30Slot0Before = state->c30Chain[0];
    result->c30Slot0After = state->c30Chain[0];
    result->c30Slot7Before = state->c30Chain[7];
    result->c30Slot7After = state->c30Chain[7];
    result->f0282Before = state->f0282FinishCount;
    result->f0282After = state->f0282FinishCount;
    result->f0334Before = state->f0334CloseChestCount;
    result->f0334After = state->f0334CloseChestCount;
    result->deterministicHashAfter = state->deterministicHash;
}

static int c30_chain_unchanged(
    const Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state,
    const Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat *result)
{
    int i;

    if (!state || !result) {
        return 0;
    }
    if (state->c30Chain[0] != result->c30Slot0Before ||
        state->c30Chain[7] != result->c30Slot7Before) {
        return 0;
    }
    for (i = 0; i < DM1_V1_MCRFC30_CHAIN_COUNT_PC34_COMPAT; ++i) {
        if (state->c30Chain[i] != kFirstC30Thing + (unsigned int)i) {
            return 0;
        }
    }
    return 1;
}

static int champion_slots_seeded(
    const Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state)
{
    int i;

    if (!state) {
        return 0;
    }
    for (i = 0; i < DM1_V1_MCRFC30_CHAMPION_SLOT_COUNT_PC34_COMPAT; ++i) {
        if (state->championSlots[i] != kFirstChampionThing + (unsigned int)i) {
            return 0;
        }
    }
    return 1;
}

static void finish_result(
    const Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state,
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->candidateAfter = state->candidateOrdinal;
    result->panelOpenAfter = state->c040PanelOpen;
    result->leaderHandAfter = state->leaderHandThing;
    result->c30Slot0After = state->c30Chain[0];
    result->c30Slot7After = state->c30Chain[7];
    result->f0282After = state->f0282FinishCount;
    result->f0334After = state->f0334CloseChestCount;
    result->c30Unchanged = c30_chain_unchanged(state, result);
    result->championSlotsUnchanged = champion_slots_seeded(state);
    result->cleanFailure =
        result->rejectedNoEmptyC30 == 1 &&
        result->accepted == 0 &&
        state->candidateOrdinal == kCandidateOrdinal &&
        state->activePanelCandidateOrdinal == kCandidateOrdinal &&
        state->c040PanelOpen == 1 &&
        state->panelContent == DM1_V1_MCRFC30_M568_PANEL_PC34_COMPAT &&
        state->leaderEmptyHanded == 1 &&
        state->leaderHandThing == DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT &&
        state->f0282FinishCount == result->f0282Before &&
        state->f0334CloseChestCount == result->f0334Before &&
        result->c30Unchanged == 1 &&
        result->championSlotsUnchanged == 1;
    result->cleanSuccess =
        result->accepted == 1 &&
        result->rejectedNoEmptyC30 == 0 &&
        state->candidateOrdinal == 0u &&
        state->activePanelCandidateOrdinal == 0u &&
        state->c040PanelOpen == 0 &&
        state->f0282FinishCount == result->f0282Before + 1;
    result->deterministicHashAfter = state->deterministicHash;
}

void dm1_v1_mirror_candidate_resurrect_full_c30_init_pc34_compat(
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->candidateOrdinal = kCandidateOrdinal;
    state->activePanelCandidateOrdinal = kCandidateOrdinal;
    state->partyChampionCount = kPartyChampionCount;
    state->leaderIndex = kLeaderIndex;
    state->leaderEmptyHanded = 1;
    state->leaderHandThing = DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    state->panelContent = DM1_V1_MCRFC30_M568_PANEL_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->openChestThing = DM1_V1_MCRFC30_G0426_OPEN_CHEST_PC34_COMPAT;
    state->fullC30Chain = 1;
    state->noCrashGuard = 1;
    state->f0280OpenCount = 1;
    state->f0333OpenChestCount = 1;
    state->f0346DrawC040Count = 1;
    state->f0347DrawPanelCount = 1;
    state->deterministicHash = kInitialHash;
    for (i = 0; i < DM1_V1_MCRFC30_CHAIN_COUNT_PC34_COMPAT; ++i) {
        state->c30Chain[i] = kFirstC30Thing + (unsigned int)i;
        update_hash(state, 10u + (unsigned int)i, state->c30Chain[i]);
    }
    for (i = 0; i < DM1_V1_MCRFC30_CHAMPION_SLOT_COUNT_PC34_COMPAT; ++i) {
        state->championSlots[i] = kFirstChampionThing + (unsigned int)i;
    }
    update_hash(state, 1u, state->candidateOrdinal);
    update_hash(state, 2u, state->panelContent);
    update_hash(state, 3u, state->openChestThing);
}

int dm1_v1_mirror_candidate_resurrect_full_c30_attempt_pc34_compat(
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state,
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat localResult;
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat *result =
        outResult ? outResult : &localResult;
    int freeSlot;

    snapshot_result(state, result,
                    "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806; "
                    "CHAMPION.C F0302:662-713");
    if (!state || !state->contractOnly || !state->c040PanelOpen ||
        state->candidateOrdinal == 0u || !state->leaderEmptyHanded) {
        finish_result(state, result);
        return 0;
    }

    ++state->f0359PanelDispatchCount;
    ++state->f0344PanelClickCount;
    ++state->f0345PanelReleaseCount;
    ++state->f0352EyeClickCount;
    update_hash(state, 40u, DM1_V1_MCRFC30_C160_RESURRECT_PC34_COMPAT);

    /* ReDMCSB: F0359 lines 1985-1990 admits the C160 click only with an
     * empty leader hand.  This regression freezes Firestaff's C30 handoff
     * preflight before F0282 lines 785-806 can clear G0299: a saturated
     * G0425 C30..C37 chain must reject without mutating the candidate,
     * leader hand, champion slots, or CHEST.C F0334 close/relink state.
     */
    if (!c30_has_empty_slot(state)) {
        state->fullC30Chain = 1;
        ++state->noEmptyC30RejectCount;
        result->rejectedNoEmptyC30 = 1;
        update_hash(state, 41u, (unsigned int)state->noEmptyC30RejectCount);
        finish_result(state, result);
        return 0;
    }

    freeSlot = first_empty_c30_slot(state);
    if (freeSlot < 0) {
        ++state->noEmptyC30RejectCount;
        result->rejectedNoEmptyC30 = 1;
        finish_result(state, result);
        return 0;
    }
    state->fullC30Chain = 0;
    state->c30Chain[freeSlot] = kResurrectedMarkerThing;
    ++state->f0282FinishCount;
    ++state->f0301AddSlotCount;
    state->candidateOrdinal = 0u;
    state->activePanelCandidateOrdinal = 0u;
    state->panelContent = 0;
    state->c040PanelOpen = 0;
    result->accepted = 1;
    update_hash(state, 42u, (unsigned int)freeSlot);
    update_hash(state, 43u, kResurrectedMarkerThing);
    finish_result(state, result);
    return 1;
}

void dm1_v1_mirror_candidate_resurrect_full_c30_free_last_slot_pc34_compat(
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state)
{
    if (!state) {
        return;
    }
    state->c30Chain[DM1_V1_MCRFC30_CHAIN_COUNT_PC34_COMPAT - 1] =
        DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT;
    state->fullC30Chain = 0;
    update_hash(state, 50u, DM1_V1_MCRFC30_C37_SLOT_PC34_COMPAT);
}

const Dm1V1MirrorCandidateResurrectFullC30EvidencePc34Compat *
dm1_v1_mirror_candidate_resurrect_full_c30_evidence_pc34_compat(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_resurrect_full_c30_source_evidence_pc34_compat(void)
{
    return s_source_evidence;
}

static void self_check(int condition)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
    }
}

int dm1_v1_mirror_candidate_resurrect_full_c30_run_self_test_pc34_compat(void)
{
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat state;
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat rejected;
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat accepted;
    int ok;

    gAssertions = 0;
    gFailures = 0;
    gLastHash = 0u;

    dm1_v1_mirror_candidate_resurrect_full_c30_init_pc34_compat(&state);
    self_check(state.contractOnly == 1);
    self_check(state.candidateOrdinal == kCandidateOrdinal);
    self_check(state.partyChampionCount == kPartyChampionCount);
    self_check(state.leaderEmptyHanded == 1);
    self_check(state.c040PanelOpen == 1);
    self_check(state.panelContent == DM1_V1_MCRFC30_M568_PANEL_PC34_COMPAT);
    self_check(state.openChestThing ==
               DM1_V1_MCRFC30_G0426_OPEN_CHEST_PC34_COMPAT);
    self_check(state.c30Chain[0] == kFirstC30Thing);
    self_check(state.c30Chain[7] == kFirstC30Thing + 7u);
    self_check(champion_slots_seeded(&state) == 1);
    self_check(c30_has_empty_slot(&state) == 0);

    ok = dm1_v1_mirror_candidate_resurrect_full_c30_attempt_pc34_compat(
        &state, &rejected);
    self_check(ok == 0);
    self_check(rejected.rejectedNoEmptyC30 == 1);
    self_check(rejected.accepted == 0);
    self_check(rejected.cleanFailure == 1);
    self_check(rejected.c30Unchanged == 1);
    self_check(rejected.championSlotsUnchanged == 1);
    self_check(state.candidateOrdinal == kCandidateOrdinal);
    self_check(state.activePanelCandidateOrdinal == kCandidateOrdinal);
    self_check(state.c040PanelOpen == 1);
    self_check(state.panelContent == DM1_V1_MCRFC30_M568_PANEL_PC34_COMPAT);
    self_check(state.leaderHandThing == DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT);
    self_check(state.f0282FinishCount == 0);
    self_check(state.f0334CloseChestCount == 0);
    self_check(state.f0300RemoveSlotCount == 0);
    self_check(state.f0301AddSlotCount == 0);
    self_check(state.noEmptyC30RejectCount == 1);
    self_check(state.noCrashGuard == 1);

    dm1_v1_mirror_candidate_resurrect_full_c30_free_last_slot_pc34_compat(
        &state);
    ok = dm1_v1_mirror_candidate_resurrect_full_c30_attempt_pc34_compat(
        &state, &accepted);
    self_check(ok == 1);
    self_check(accepted.accepted == 1);
    self_check(accepted.cleanSuccess == 1);
    self_check(accepted.rejectedNoEmptyC30 == 0);
    self_check(state.candidateOrdinal == 0u);
    self_check(state.c040PanelOpen == 0);
    self_check(state.f0282FinishCount == 1);
    self_check(state.f0301AddSlotCount == 1);
    self_check(state.c30Chain[7] == kResurrectedMarkerThing);
    self_check(state.f0334CloseChestCount == 0);

    gLastHash = state.deterministicHash;
    gLastHash = fnv1a_step(gLastHash, (unsigned int)gAssertions);
    gLastHash = fnv1a_step(gLastHash, (unsigned int)gFailures);
    return gFailures == 0;
}

int dm1_v1_mirror_candidate_resurrect_full_c30_assertions_pc34_compat(void)
{
    return gAssertions;
}

int dm1_v1_mirror_candidate_resurrect_full_c30_failures_pc34_compat(void)
{
    return gFailures;
}

unsigned int dm1_v1_mirror_candidate_resurrect_full_c30_hash_pc34_compat(void)
{
    return gLastHash;
}
