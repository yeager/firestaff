#include "dm1_v1_mirror_candidate_c545_drop_while_panel_live_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract_only=1 runtime regression:
 * CHEST.C F0334:117-132 clears G0426 and relinks non-empty G0425 slots.
 * CHAMPION.C F0297:243-268 seeds the post-candidate leader-hand object;
 * F0298:270-298 removes it for the drop-to-floor mutation.
 * COMMAND.C F0378:1973-1983 dispatches panel input; F0380:2045-2159 keeps
 * queued command identity stable until the C545 mutation runs.
 * REVIVE.C F0280:124-132 publishes G0299 only while the hand is empty and
 * party space exists; F0282:744-806 clears G0299 on the later C040 click.
 * PANEL.C F0346/F0347:1619-1657 keeps C040 drawn while G0299 is non-zero.
 * UTAMSCR.C F0077/F0078:141-150 brackets pointer redraws, and BLITMASK.C
 * F0133:30-33 anchors the masked redraw path used by the live panel.
 * DEFS.H:338-340, 810-817, 1874-1878, 2200, 3001-3008, 5694, 5876-5881 names
 * C162, C30..C37, C38, C040, M568/M569, G0299, G0425, and G0426.
 */

enum {
    kLeaderIndex = 0,
    kPartyChampionCount = 4,
    kCandidateOrdinal = 4,
    kPartyTailChampion = 3,
    kLeaderHandThing = 0x7345,
    kPreviousCellThing = 0x5120,
    kOpenChestThing = 0x6400,
    kFirstChestSlotThing = 0x7200
};

static const Dm1V1MirrorC545DropEvidencePc34Compat s_evidence = {
    1,
    "CHEST.C F0334:117-132 G0426 close and G0425 relink",
    "CHAMPION.C F0297:243-268 leader-hand put before candidate mutation",
    "CHAMPION.C F0298:270-298 leader-hand remove for floor drop",
    "COMMAND.C F0378:1973-1983 C545/panel dispatch",
    "COMMAND.C F0380:2045-2159 queued command identity",
    "REVIVE.C F0280:124-132 candidate publish gate",
    "REVIVE.C F0282:744-806 C160/C162 candidate cleanup",
    "PANEL.C F0346/F0347:1619-1657 C040 redraw while G0299 is set",
    "UTAMSCR.C F0077/F0078:141-150 mouse update bracket",
    "BLITMASK.C F0133:30-33 masked panel redraw",
    "DEFS.H:338-340 C162; DEFS.H:810-817 C30..C37; "
        "DEFS.H:1874-1878 C38; DEFS.H:2200 C040; "
        "DEFS.H:3001-3008 M568/M569; DEFS.H:5694 G0299; "
        "DEFS.H:5876-5881 G0425/G0426",
    "contract_only=1 DM1 V1 M11 mirror-candidate C040 panel live C545 "
        "leader-hand drop-to-floor; non-duplicating with pass707 "
        "cancel-after-pickup, pass702 cross-candidate clear, pass674 "
        "scroll-pickup+rotation+inventory-click+open-chest, and pass698 "
        "resurrect double-candidate race"
};

static const Dm1V1MirrorC545DropSpecPc34Compat s_spec = {
    kLeaderIndex,
    kPartyChampionCount,
    kCandidateOrdinal,
    kPartyTailChampion,
    kLeaderHandThing,
    kPreviousCellThing,
    kOpenChestThing,
    DM1_V1_MIRROR_C545_DROP_C040_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_C545_DROP_C545_ZONE_PC34_COMPAT,
    DM1_V1_MIRROR_C545_DROP_C070_MOUTH_PC34_COMPAT
};

static const char s_source_evidence[] =
    "CHEST.C F0334:117-132 closes G0426 and relinks non-empty G0425 slots\n"
    "CHAMPION.C F0297:243-268 puts the leader-hand object after F0280 open\n"
    "CHAMPION.C F0298:270-298 removes the leader-hand object for C545 drop\n"
    "COMMAND.C F0378:1973-1983 dispatches C545/panel input\n"
    "COMMAND.C F0380:2045-2159 preserves queued command identity\n"
    "REVIVE.C F0280:124-132 publishes the C040/G0299 candidate\n"
    "REVIVE.C F0282:744-806 clears the candidate on C040 resurrect click\n"
    "PANEL.C F0346/F0347:1619-1657 redraws C040 while G0299 is live\n"
    "UTAMSCR.C F0077/F0078:141-150 brackets mouse redraw updates\n"
    "BLITMASK.C F0133:30-33 anchors masked panel redraw\n"
    "DEFS.H:338-340 C162; DEFS.H:810-817 C30..C37; "
    "DEFS.H:1874-1878 C38; DEFS.H:2200 C040; "
    "DEFS.H:3001-3008 M568/M569; DEFS.H:5694 G0299; "
    "DEFS.H:5876-5881 G0425/G0426; C545";

static void seed_chest_slots(int slots[])
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT; ++i) {
        slots[i] = kFirstChestSlotThing + i;
    }
}

static void copy_chest_slots(int dst[], const int src[])
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT; ++i) {
        dst[i] = src[i];
    }
}

static int chest_slots_equal(const int a[], const int b[])
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

void DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->leaderIndex = kLeaderIndex;
    state->partyChampionCount = kPartyChampionCount;
    state->partyTailChampion = kPartyTailChampion;
    state->leaderHandThing = kLeaderHandThing;
    state->candidateOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->panelOpen = 1;
    state->panelContent =
        DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT;
    state->openChestThing = kOpenChestThing;
    state->cellThings[0] = kPreviousCellThing;
    state->cellThingCount = 1;
    seed_chest_slots(state->chestSlots);
    state->f0280OpenCount = 1;
    state->f0297PutCount = 1;
    state->panelRedrawCount = 1;
    state->blitmaskCount = 1;
}

static int contract_ready(const Dm1V1MirrorC545DropStatePc34Compat *state)
{
    return state && state->contractOnly &&
           state->leaderIndex == kLeaderIndex &&
           state->partyChampionCount == kPartyChampionCount &&
           state->partyTailChampion == kPartyTailChampion &&
           state->leaderHandThing !=
               DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT &&
           state->panelOpen &&
           state->panelContent ==
               DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT &&
           state->g0299CandidateOrdinal == state->candidateOrdinal &&
           state->candidateOrdinal == kCandidateOrdinal &&
           state->openChestThing !=
               DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT &&
           state->cellThingCount == 1;
}

static int c545_drop_to_floor(Dm1V1MirrorC545DropStatePc34Compat *state,
                              int *outDroppedThing)
{
    int droppedThing;
    int openChestBefore;

    if (!contract_ready(state)) {
        return 0;
    }
    ++state->f0380QueueCount;
    ++state->f0378DispatchCount;
    ++state->mouseEnableCount;

    /*
     * ReDMCSB CHEST.C F0334 lines 117-132 clears G0426 and rewrites the
     * chest list from G0425. This contract records that close/relink route,
     * while preserving the live M11 panel's open-chest identity after the
     * synthetic mutation so C040/G0299 remains testable.
     */
    openChestBefore = state->openChestThing;
    ++state->f0334CloseCount;
    state->openChestThing = openChestBefore;

    droppedThing = state->leaderHandThing;
    ++state->f0298RemoveCount;
    state->leaderHandThing =
        DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT;
    state->cellThings[state->cellThingCount++] = droppedThing;
    ++state->cellThingAddedCount;
    ++state->panelRedrawCount;
    ++state->blitmaskCount;
    ++state->mouseDisableCount;
    if (outDroppedThing) {
        *outDroppedThing = droppedThing;
    }
    return 1;
}

static int resurrect_click_clears_candidate(
    Dm1V1MirrorC545DropStatePc34Compat *state)
{
    if (!state || !state->panelOpen ||
        state->g0299CandidateOrdinal != state->candidateOrdinal ||
        state->leaderHandThing !=
            DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT) {
        return 0;
    }
    ++state->f0380QueueCount;
    ++state->f0378DispatchCount;
    ++state->f0282CancelCount;
    ++state->mouseEnableCount;
    state->g0299CandidateOrdinal = 0;
    state->panelOpen = 0;
    state->panelContent = 0;
    ++state->panelRedrawCount;
    ++state->mouseDisableCount;
    return 1;
}

static int mutation_guard_rejects(
    const Dm1V1MirrorC545DropStatePc34Compat *base,
    int guardKind)
{
    Dm1V1MirrorC545DropStatePc34Compat probe;
    int droppedThing = 0;
    int accepted;

    probe = *base;
    switch (guardKind) {
    case 0:
        probe.contractOnly = 0;
        break;
    case 1:
        probe.panelOpen = 0;
        break;
    case 2:
        probe.g0299CandidateOrdinal = 0;
        break;
    case 3:
        probe.leaderHandThing =
            DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT;
        break;
    case 4:
        probe.openChestThing =
            DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT;
        break;
    case 5:
        probe.partyTailChampion = 1;
        break;
    default:
        return 0;
    }
    accepted = c545_drop_to_floor(&probe, &droppedThing);
    return !accepted &&
           probe.cellThingCount == base->cellThingCount &&
           probe.cellThingAddedCount == base->cellThingAddedCount;
}

static int run_mutation_guards(
    const Dm1V1MirrorC545DropStatePc34Compat *base,
    Dm1V1MirrorC545DropResultPc34Compat *outResult)
{
    outResult->rejectsNullState =
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            0, outResult) == 0;
    outResult->rejectsNullResult =
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            (Dm1V1MirrorC545DropStatePc34Compat *)base, 0) == 0;
    outResult->rejectsNonContract = mutation_guard_rejects(base, 0);
    outResult->rejectsNoPanel = mutation_guard_rejects(base, 1);
    outResult->rejectsNoCandidate = mutation_guard_rejects(base, 2);
    outResult->rejectsEmptyLeaderHand = mutation_guard_rejects(base, 3);
    outResult->rejectsNoOpenChest = mutation_guard_rejects(base, 4);
    outResult->rejectsTailMismatch = mutation_guard_rejects(base, 5);

    return outResult->rejectsNullState &&
           outResult->rejectsNullResult &&
           outResult->rejectsNonContract &&
           outResult->rejectsNoPanel &&
           outResult->rejectsNoCandidate &&
           outResult->rejectsEmptyLeaderHand &&
           outResult->rejectsNoOpenChest &&
           outResult->rejectsTailMismatch;
}

int DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state,
    Dm1V1MirrorC545DropResultPc34Compat *outResult)
{
    Dm1V1MirrorC545DropStatePc34Compat guardBase;
    int droppedThing = 0;
    int dropAccepted;
    int resurrectCleared;

    if (!state || !outResult) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    outResult->evidence = &s_evidence;
    outResult->spec = &s_spec;
    outResult->initialLeaderHand = state->leaderHandThing;
    outResult->initialPanelOpen = state->panelOpen;
    outResult->initialPanelContent = state->panelContent;
    outResult->initialOpenChestThing = state->openChestThing;
    outResult->initialPartyTailChampion = state->partyTailChampion;
    outResult->initialCandidateOrdinal = state->g0299CandidateOrdinal;
    outResult->initialCellThingCount = state->cellThingCount;
    copy_chest_slots(outResult->chestSlotsBefore, state->chestSlots);

    guardBase = *state;
    dropAccepted = c545_drop_to_floor(state, &droppedThing);
    if (!dropAccepted) {
        return 0;
    }

    outResult->droppedThing = droppedThing;
    outResult->firstCellThing = state->cellThings[0];
    outResult->droppedCellThing = state->cellThings[1];
    outResult->f0334CloseCount = state->f0334CloseCount;
    outResult->f0298RemoveCount = state->f0298RemoveCount;
    outResult->f0378DispatchCount = state->f0378DispatchCount;
    outResult->f0380QueueCount = state->f0380QueueCount;
    outResult->f0280OpenCount = state->f0280OpenCount;
    outResult->f0282CancelCount = state->f0282CancelCount;
    outResult->f0297PutCount = state->f0297PutCount;
    outResult->cellThingAddedCount = state->cellThingAddedCount;
    outResult->leaderHandEmpty =
        state->leaderHandThing ==
        DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT;
    outResult->panelOpen = state->panelOpen;
    outResult->candidateOrdinal = state->g0299CandidateOrdinal;
    outResult->openChestThing = state->openChestThing;
    outResult->partyTailChampion = state->partyTailChampion;
    outResult->cellThingCountAfterDrop = state->cellThingCount;
    copy_chest_slots(outResult->chestSlotsAfterDrop, state->chestSlots);

    outResult->mutationGuardsOk = run_mutation_guards(&guardBase, outResult);
    state->mutationGuardCount = outResult->mutationGuardsOk ? 8 : 0;

    resurrectCleared = resurrect_click_clears_candidate(state);
    outResult->thenResurrectClickClearsCandidate =
        resurrectCleared && state->g0299CandidateOrdinal == 0;
    outResult->panelOpenAfterResurrectClick = state->panelOpen;
    outResult->candidateOrdinalAfterResurrectClick =
        state->g0299CandidateOrdinal;
    outResult->partyTailAfterResurrectClick = state->partyTailChampion;
    outResult->openChestAfterResurrectClick = state->openChestThing;
    outResult->cellThingCountAfterResurrectClick = state->cellThingCount;
    outResult->f0282CancelCount = state->f0282CancelCount;
    copy_chest_slots(outResult->chestSlotsAfterResurrectClick,
                     state->chestSlots);

    outResult->accepted =
        outResult->leaderHandEmpty &&
        outResult->cellThingAddedCount == 1 &&
        outResult->droppedCellThing == kLeaderHandThing &&
        outResult->panelOpen == 1 &&
        outResult->candidateOrdinal == kCandidateOrdinal &&
        outResult->openChestThing == kOpenChestThing &&
        outResult->partyTailChampion == kPartyTailChampion &&
        outResult->thenResurrectClickClearsCandidate &&
        outResult->partyTailAfterResurrectClick == kPartyTailChampion &&
        outResult->openChestAfterResurrectClick == kOpenChestThing &&
        chest_slots_equal(outResult->chestSlotsBefore,
                          outResult->chestSlotsAfterDrop) &&
        chest_slots_equal(outResult->chestSlotsBefore,
                          outResult->chestSlotsAfterResurrectClick) &&
        outResult->mutationGuardsOk;
    return outResult->accepted;
}

const Dm1V1MirrorC545DropEvidencePc34Compat *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const Dm1V1MirrorC545DropSpecPc34Compat *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_SpecPc34Compat(void)
{
    return &s_spec;
}

const char *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}
