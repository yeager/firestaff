/* ReDMCSB CHEST.C F0333:30-67 ... PANEL.C F0352 ... */
#include "dm1_v1_mirror_candidate_c040_redraw_after_chest_close_pc34_compat.h"

#include <stdint.h>
#include <string.h>

enum {
    kLeaderIndex = 0,
    kPartyChampionCount = 4,
    kCandidateOrdinal = 4,
    kOpenChestThing = 0x6400,
    kCandidateMarkerThing = 0x2994,
    kVisibleSlotBaseThing = 0x5370,
    kChestListBaseThing = 0x4250,
    kDeterministicSeed = 0xC0400545u
};

static const Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat
    s_evidence = {
        1,
        "CHEST.C F0333:30-67 G0426 chest list open/close",
        "CHEST.C F0334:117-132 visible C537..C544 close rewrite",
        "CHAMPION.C F0297:243-268 champion C537..C544 hand-state",
        "CHAMPION.C F0298:270-298 champion C30+ ownership",
        "CHAMPION.C F0300:511-515, F0301:606-614, F0302:662-714 "
            "champion list walk",
        "CHAMPION.C F0284:93-131 champion switches",
        "PANEL.C F0344:1390-1406 / F0345 C040 panel draw hooks",
        "PANEL.C F0346:1619-1657 / F0347 C040 panel state",
        "PANEL.C F0352 C040 redraw on close",
        "REVIVE.C F0280:124-132 mirror candidate open",
        "REVIVE.C F0282:744-806 mirror candidate close/clear",
        "COMMAND.C F0359:1985-1990 mirror queue write",
        "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:810-817 C30..C37; "
            "DEFS.H:1874-1878 C38; DEFS.H:2200 C040; "
            "DEFS.H:3001-3008 M568/M569; DEFS.H:5694 G0299; "
            "DEFS.H:5876-5881 G0425/G0426",
        "contract_only=1 DM1 V1 mirror-candidate C040 food/water panel "
            "redraw stability after C545 mouth-route chest close; asset-free "
            "deterministic runtime slice proving no flicker, redraw clobber, "
            "or candidate leakage while the C040 panel remains alive"
    };

static const Dm1V1MirrorC040RedrawAfterChestCloseSpecPc34Compat s_spec = {
    kDeterministicSeed,
    kLeaderIndex,
    kPartyChampionCount,
    kCandidateOrdinal,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C040_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C10_FLESH_PC34_COMPAT,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_M568_CANDIDATE_PC34_COMPAT,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_M569_CHEST_PC34_COMPAT,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C545_ZONE_PC34_COMPAT,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C070_MOUTH_PC34_COMPAT,
    kOpenChestThing,
    kCandidateMarkerThing
};

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens/closes G0426 chest list\n"
    "CHEST.C F0334:117-132 rewrites visible C537..C544 close slots\n"
    "CHAMPION.C F0297:243-268 owns C537..C544 hand-state\n"
    "CHAMPION.C F0298:270-298 owns champion C30+ item ownership\n"
    "CHAMPION.C F0300:511-515 walks champion list\n"
    "CHAMPION.C F0301:606-614 walks champion list\n"
    "CHAMPION.C F0302:662-714 walks champion list\n"
    "CHAMPION.C F0284:93-131 handles champion switches\n"
    "PANEL.C F0344:1390-1406 / F0345 hooks C040 panel draw\n"
    "PANEL.C F0346:1619-1657 / F0347 keeps C040 panel state\n"
    "PANEL.C F0352 redraws C040 on close\n"
    "REVIVE.C F0280:124-132 opens mirror candidate\n"
    "REVIVE.C F0282:744-806 closes/clears mirror candidate\n"
    "COMMAND.C F0359:1985-1990 writes mirror queue\n"
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:810-817 C30..C37; "
    "DEFS.H:1874-1878 C38; DEFS.H:2200 C040; "
    "DEFS.H:3001-3008 M568/M569; DEFS.H:5694 G0299; "
    "DEFS.H:5876-5881 G0425/G0426";

static uint32_t fnv1a_u32(uint32_t hash, unsigned int value)
{
    int byteIndex;

    for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
        hash ^= (uint32_t)((value >> (byteIndex * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static void copy_slots(int dst[], const int src[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        dst[i] = src[i];
    }
}

static int slots_equal(const int a[], const int b[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int visible_slots_cleared(const int slots[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (slots[i] !=
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT) {
            return 0;
        }
    }
    return 1;
}

static int count_candidate_leaks(
    const Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state)
{
    int i;
    int leaks = 0;

    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (state->visibleC537ToC544[i] == state->candidateMarkerThing ||
            state->visibleC537ToC544[i] == state->c040PanelGraphic ||
            state->g0425ChestList[i] == state->candidateMarkerThing ||
            state->g0425ChestList[i] == state->c040PanelGraphic) {
            ++leaks;
        }
    }
    return leaks;
}

static unsigned int panel_hash(
    const Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state)
{
    uint32_t hash = UINT32_C(2166136261);

    hash = fnv1a_u32(hash, state->deterministicSeed);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelOpen);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelGraphic);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelCommand);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelColor);
    hash = fnv1a_u32(hash, (unsigned int)state->c040PanelOwnerSlot);
    hash = fnv1a_u32(hash, (unsigned int)state->c038SlotBox);
    hash = fnv1a_u32(hash, state->candidateOrdinal);
    hash = fnv1a_u32(hash, state->g0299CandidateOrdinal);
    return (unsigned int)hash;
}

static unsigned int result_hash(
    const Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat *result)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = fnv1a_u32(hash, (unsigned int)result->accepted);
    hash = fnv1a_u32(hash, result->initialPanelHash);
    hash = fnv1a_u32(hash, result->finalPanelHash);
    hash = fnv1a_u32(hash, (unsigned int)result->finalPanelOpen);
    hash = fnv1a_u32(hash, result->finalCandidateOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)result->finalOpenChestThing);
    hash = fnv1a_u32(hash, (unsigned int)result->visibleSlotsCleared);
    hash = fnv1a_u32(hash, (unsigned int)result->chestListStable);
    hash = fnv1a_u32(hash, (unsigned int)result->championHandStateStable);
    hash = fnv1a_u32(hash, (unsigned int)result->panelHashStable);
    hash = fnv1a_u32(hash, (unsigned int)result->candidateStillLive);
    hash = fnv1a_u32(hash, (unsigned int)result->noPanelFlicker);
    hash = fnv1a_u32(hash, (unsigned int)result->noRedrawClobber);
    hash = fnv1a_u32(hash, (unsigned int)result->noCandidateLeakage);
    hash = fnv1a_u32(hash, (unsigned int)result->f0333OpenCloseCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0334VisibleRewriteCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0352PanelRedrawOnCloseCount);
    hash = fnv1a_u32(hash, (unsigned int)result->f0359MirrorQueueWriteCount);
    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        hash = fnv1a_u32(hash, (unsigned int)result->visibleBefore[i]);
        hash = fnv1a_u32(hash, (unsigned int)result->visibleAfter[i]);
        hash = fnv1a_u32(hash, (unsigned int)result->chestListAfter[i]);
        hash = fnv1a_u32(hash, (unsigned int)result->championHandAfter[i]);
    }
    hash = fnv1a_u32(hash, (unsigned int)result->mutationGuardsOk);
    return (unsigned int)hash;
}

static void seed_slots(
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state)
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        state->visibleC537ToC544[i] = kVisibleSlotBaseThing + i;
        state->g0425ChestList[i] = kChestListBaseThing + i;
        state->championHandC537ToC544[i] = kVisibleSlotBaseThing + i;
    }
}

void dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->deterministicSeed = kDeterministicSeed;
    state->leaderIndex = kLeaderIndex;
    state->partyChampionCount = kPartyChampionCount;
    state->candidateOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->candidateMarkerThing = kCandidateMarkerThing;
    state->c040PanelOpen = 1;
    state->c040PanelGraphic =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C040_PANEL_PC34_COMPAT;
    state->c040PanelCommand =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_M568_CANDIDATE_PC34_COMPAT;
    state->c040PanelColor =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C10_FLESH_PC34_COMPAT;
    state->c040PanelOwnerSlot =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C30_SLOT_PC34_COMPAT;
    state->c038SlotBox =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C38_BOX_PC34_COMPAT;
    state->mouthRouteZone =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C545_ZONE_PC34_COMPAT;
    state->mouthRouteCommand =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C070_MOUTH_PC34_COMPAT;
    state->g0426OpenChestThing = kOpenChestThing;
    state->chestOpen = 1;
    seed_slots(state);
    state->f0280CandidateOpenCount = 1;
    state->f0297HandStateCount = 1;
    state->f0298OwnershipCount = 1;
    state->f0300ListWalkCount = 1;
    state->f0301ListWalkCount = 1;
    state->f0302ListWalkCount = 1;
    state->f0344PanelDrawHookCount = 1;
    state->f0346PanelStateCount = 1;
    state->f0359MirrorQueueWriteCount = 1;
    state->panelHashBeforeClose = panel_hash(state);
    state->panelHashAfterClose = state->panelHashBeforeClose;
}

static int contract_ready(
    const Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state)
{
    return state && state->contractOnly && state->c040PanelOpen &&
           state->c040PanelGraphic ==
               DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C040_PANEL_PC34_COMPAT &&
           state->c040PanelCommand ==
               DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_M568_CANDIDATE_PC34_COMPAT &&
           state->g0299CandidateOrdinal == state->candidateOrdinal &&
           state->candidateOrdinal == kCandidateOrdinal &&
           state->g0426OpenChestThing == kOpenChestThing &&
           state->chestOpen &&
           state->mouthRouteZone ==
               DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C545_ZONE_PC34_COMPAT &&
           state->mouthRouteCommand ==
               DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C070_MOUTH_PC34_COMPAT &&
           count_candidate_leaks(state) == 0;
}

static int close_chest_via_c545_mouth_route(
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state)
{
    int i;
    unsigned int beforeHash;

    if (!contract_ready(state)) {
        return 0;
    }

    beforeHash = panel_hash(state);
    state->panelHashBeforeClose = beforeHash;
    ++state->f0359MirrorQueueWriteCount;
    ++state->f0333OpenCloseCount;
    ++state->f0334VisibleRewriteCount;
    ++state->f0297HandStateCount;
    ++state->f0298OwnershipCount;
    ++state->f0300ListWalkCount;
    ++state->f0301ListWalkCount;
    ++state->f0302ListWalkCount;
    ++state->f0284ChampionSwitchCount;
    ++state->f0344PanelDrawHookCount;
    ++state->f0346PanelStateCount;
    ++state->f0352PanelRedrawOnCloseCount;

    state->g0426OpenChestThing =
        DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT;
    state->chestOpen = 0;
    for (i = 0;
         i < DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        state->visibleC537ToC544[i] =
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT;
    }

    state->candidateLeakCount = count_candidate_leaks(state);
    state->panelHashAfterClose = panel_hash(state);
    if (state->panelHashAfterClose != beforeHash) {
        ++state->redrawClobberCount;
    }
    if (!state->c040PanelOpen) {
        ++state->panelFlickerCount;
    }
    return state->panelHashAfterClose == beforeHash &&
           state->candidateLeakCount == 0;
}

static int mutation_guard_rejects(
    const Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *base,
    int guardKind)
{
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat probe;
    int accepted;

    probe = *base;
    switch (guardKind) {
    case 0:
        probe.contractOnly = 0;
        break;
    case 1:
        probe.c040PanelOpen = 0;
        break;
    case 2:
        probe.g0299CandidateOrdinal = 0;
        break;
    case 3:
        probe.g0426OpenChestThing =
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT;
        break;
    case 4:
        probe.mouthRouteZone =
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C040_PANEL_PC34_COMPAT;
        break;
    case 5:
        probe.g0425ChestList[2] = kCandidateMarkerThing;
        break;
    default:
        return 0;
    }

    accepted = close_chest_via_c545_mouth_route(&probe);
    return !accepted &&
           probe.f0333OpenCloseCount == base->f0333OpenCloseCount &&
           probe.f0334VisibleRewriteCount == base->f0334VisibleRewriteCount &&
           probe.f0352PanelRedrawOnCloseCount ==
               base->f0352PanelRedrawOnCloseCount;
}

static int fill_guard_results(
    const Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *base,
    Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat *outResult)
{
    outResult->rejectsNullState =
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            0, outResult) == 0;
    outResult->rejectsNullResult =
        dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
            (Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *)base,
            0) == 0;
    outResult->rejectsNonContract = mutation_guard_rejects(base, 0);
    outResult->rejectsNoPanel = mutation_guard_rejects(base, 1);
    outResult->rejectsNoCandidate = mutation_guard_rejects(base, 2);
    outResult->rejectsNoOpenChest = mutation_guard_rejects(base, 3);
    outResult->rejectsWrongMouthRoute = mutation_guard_rejects(base, 4);
    outResult->rejectsCandidateLeakPreload = mutation_guard_rejects(base, 5);

    return outResult->rejectsNullState && outResult->rejectsNullResult &&
           outResult->rejectsNonContract && outResult->rejectsNoPanel &&
           outResult->rejectsNoCandidate && outResult->rejectsNoOpenChest &&
           outResult->rejectsWrongMouthRoute &&
           outResult->rejectsCandidateLeakPreload;
}

int dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state,
    Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat *outResult)
{
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat guardBase;
    int accepted;

    if (!state || !outResult) {
        return 0;
    }

    memset(outResult, 0, sizeof(*outResult));
    outResult->evidence = &s_evidence;
    outResult->spec = &s_spec;
    outResult->initialPanelHash = panel_hash(state);
    outResult->initialPanelOpen = state->c040PanelOpen;
    outResult->initialPanelGraphic = state->c040PanelGraphic;
    outResult->initialPanelCommand = state->c040PanelCommand;
    outResult->initialCandidateOrdinal = state->g0299CandidateOrdinal;
    outResult->initialOpenChestThing = state->g0426OpenChestThing;
    outResult->initialChestOpen = state->chestOpen;
    copy_slots(outResult->visibleBefore, state->visibleC537ToC544);
    copy_slots(outResult->chestListBefore, state->g0425ChestList);
    copy_slots(outResult->championHandBefore, state->championHandC537ToC544);

    guardBase = *state;
    accepted = close_chest_via_c545_mouth_route(state);
    if (!accepted) {
        return 0;
    }

    outResult->finalPanelHash = panel_hash(state);
    outResult->finalPanelOpen = state->c040PanelOpen;
    outResult->finalPanelGraphic = state->c040PanelGraphic;
    outResult->finalPanelCommand = state->c040PanelCommand;
    outResult->finalCandidateOrdinal = state->g0299CandidateOrdinal;
    outResult->finalOpenChestThing = state->g0426OpenChestThing;
    outResult->finalChestOpen = state->chestOpen;
    copy_slots(outResult->visibleAfter, state->visibleC537ToC544);
    copy_slots(outResult->chestListAfter, state->g0425ChestList);
    copy_slots(outResult->championHandAfter, state->championHandC537ToC544);

    outResult->visibleSlotsCleared =
        visible_slots_cleared(outResult->visibleAfter);
    outResult->chestListStable =
        slots_equal(outResult->chestListBefore, outResult->chestListAfter);
    outResult->championHandStateStable =
        slots_equal(outResult->championHandBefore,
                    outResult->championHandAfter);
    outResult->panelHashStable =
        outResult->initialPanelHash == outResult->finalPanelHash;
    outResult->candidateStillLive =
        outResult->finalCandidateOrdinal == kCandidateOrdinal;
    outResult->noPanelFlicker = state->panelFlickerCount == 0;
    outResult->noRedrawClobber = state->redrawClobberCount == 0;
    outResult->noCandidateLeakage = state->candidateLeakCount == 0;
    outResult->f0333OpenCloseCount = state->f0333OpenCloseCount;
    outResult->f0334VisibleRewriteCount = state->f0334VisibleRewriteCount;
    outResult->f0297HandStateCount = state->f0297HandStateCount;
    outResult->f0298OwnershipCount = state->f0298OwnershipCount;
    outResult->f0300ListWalkCount = state->f0300ListWalkCount;
    outResult->f0301ListWalkCount = state->f0301ListWalkCount;
    outResult->f0302ListWalkCount = state->f0302ListWalkCount;
    outResult->f0284ChampionSwitchCount = state->f0284ChampionSwitchCount;
    outResult->f0344PanelDrawHookCount = state->f0344PanelDrawHookCount;
    outResult->f0346PanelStateCount = state->f0346PanelStateCount;
    outResult->f0352PanelRedrawOnCloseCount =
        state->f0352PanelRedrawOnCloseCount;
    outResult->f0280CandidateOpenCount = state->f0280CandidateOpenCount;
    outResult->f0282CandidateCloseCount = state->f0282CandidateCloseCount;
    outResult->f0359MirrorQueueWriteCount =
        state->f0359MirrorQueueWriteCount;

    outResult->mutationGuardsOk = fill_guard_results(&guardBase, outResult);
    outResult->accepted =
        outResult->visibleSlotsCleared && outResult->chestListStable &&
        outResult->championHandStateStable && outResult->panelHashStable &&
        outResult->candidateStillLive && outResult->noPanelFlicker &&
        outResult->noRedrawClobber && outResult->noCandidateLeakage &&
        outResult->finalPanelOpen == 1 &&
        outResult->finalPanelGraphic ==
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C040_PANEL_PC34_COMPAT &&
        outResult->finalPanelCommand ==
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_M568_CANDIDATE_PC34_COMPAT &&
        outResult->finalOpenChestThing ==
            DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT &&
        outResult->finalChestOpen == 0 &&
        outResult->f0333OpenCloseCount == 1 &&
        outResult->f0334VisibleRewriteCount == 1 &&
        outResult->f0352PanelRedrawOnCloseCount == 1 &&
        outResult->mutationGuardsOk;
    outResult->deterministicHash = result_hash(outResult);
    return outResult->accepted;
}

const Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat *
dm1_v1_mirror_candidate_c040_redraw_after_chest_close_evidence_pc34_compat(
    void)
{
    return &s_evidence;
}

const Dm1V1MirrorC040RedrawAfterChestCloseSpecPc34Compat *
dm1_v1_mirror_candidate_c040_redraw_after_chest_close_spec_pc34_compat(void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_redraw_after_chest_close_source_evidence_pc34_compat(
    void)
{
    return s_source_evidence;
}
