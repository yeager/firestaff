#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_pc34_compat.h"

#include <string.h>

enum {
    kPanelFoodWater = 1,
    kPanelClosed = 0,
    kGraphicC045ObjectIcons096To127 = 45,
    kGraphicC040ResurrectReincarnate = 40,
    kCommandC038InventoryNeck = 38,
    kCommandC018SetLeader2 = 18,
    kCommandC019SetLeader3 = 19,
    kCommandC061Chest4 = 61,
    kCommandC503CloseButton = 503,
    kZoneC540Chest4 = 540,
    kChestSlotC30Offset = 30,
    kC540SlotIndex = 3,
    kLeaderIndex = 0,
    kInventoryChampionOrdinal = 1,
    kOpenChestThing = 0x6420,
    kLeaderHandThing = 0x4c30,
    kFirstVisibleThing = 0x6100
};

/*
 * ReDMCSB source-lock map:
 * - CHEST.C F0333:30-67 materializes G0425 from the open chest chain.
 * - CHEST.C F0334:113-132 clears G0426 and relinks non-empty G0425 slots.
 * - CHAMPION.C F0297:243-298/F0298:270-298 mutate the leader hand.
 * - CHAMPION.C F0300:511-515/F0301:606-614 mutate C30+ chest slots.
 * - CHAMPION.C F0302:662-714 dispatches C537..C544 chest slot clicks.
 * - REVIVE.C F0280:124-132 is the empty-leader-hand C040 candidate gate.
 * - REVIVE.C F0282:744-806 is the C040 resurrect/reincarnate panel path.
 * - COMMAND.C F0359:1985-1990 reaches F0282 only for M568/C040 and only
 *   when the leader hand is empty.
 * - PANEL.C F0344:1493-1561/F0345:1563-1616 draw/read food and water.
 * - PANEL.C F0354:2299-2352 closes inventory/panel chrome and calls F0334
 *   only when it is closing the inventory, which this synthetic C045 close
 *   explicitly does not do.
 * - DEFS.H C040/C045/C537..C544/C030/G0425/G0426 fix the constants.
 *
 * This is contract-only runtime evidence for a C045 food/water close that is
 * observed after a non-candidate panel transition.  The pin is that an
 * occupied leader-hand C30 thing and the C540 visible slot route survive the
 * transition and the close; no F0333/F0334/F0280/F0282 path may be used.
 */
static const Dm1V1MirrorCandidateC045AfterNonCandidateEvidencePc34Compat
    kEvidence = {
        "CHEST.C F0333:30-67 materializes G0425 from the open chest chain",
        "CHEST.C F0334:113-132 clears G0426 and relinks visible slots",
        "CHAMPION.C F0297:243-298, F0298:270-298, F0300:511-515, "
        "F0301:606-614, F0302:662-714 cover leader hand, C30, and C537..C544",
        "REVIVE.C F0280:124-132 and F0282:744-806 are the blocked C040 path",
        "COMMAND.C F0359:1985-1990 dispatches M568/C040 only through F0282",
        "PANEL.C F0344:1493-1561, F0345:1563-1616, F0354:2299-2352",
        "DEFS.H:267 C030, 282 C045 command, 1906 C030 icon, "
        "2200 C040, 2205 C045, 2999-3008 M565/M568, "
        "3906-3913 C537..C544, 5876-5881 G0425/G0426",
        "Disjoint from pass674/pass686/pass710/pass711/pass736/pass745/"
        "pass765plus: this covers C045 close after a non-candidate panel "
        "transition with an occupied leader-hand C30 thing and C540 route."
    };

static const char kSourceEvidence[] =
    "CHEST.C F0333:30-67; CHEST.C F0334:113-132; "
    "CHAMPION.C F0297:243-298/F0298:270-298/F0300:511-515/"
    "F0301:606-614/F0302:662-714; REVIVE.C F0280:124-132/"
    "F0282:744-806; COMMAND.C F0359:1985-1990; PANEL.C "
    "F0344:1493-1561/F0345:1563-1616/F0354:2299-2352; "
    "DEFS.H:267 C030, 282 C045, 1906 C030 icon, 2200 C040, "
    "2205 C045, 3906-3913 C537..C544, 5876-5881 G0425/G0426.";

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static uint32_t hash_close_surface(
    const Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->leaderHandC30Thing);
    hash = hash_step(hash, (unsigned int)state->g0426OpenChest);
    hash = hash_step(hash, (unsigned int)state->c540SlotIndex);
    hash = hash_step(hash, (unsigned int)state->c540Zone);
    hash = hash_step(hash, (unsigned int)state->c540Command);
    hash = hash_step(hash, (unsigned int)state->c045Graphic);
    hash = hash_step(hash, (unsigned int)state->panelContentAfterClose);
    hash = hash_step(hash, (unsigned int)state->panelOpen);
    hash = hash_step(hash, (unsigned int)state->f0280Entered);
    hash = hash_step(hash, (unsigned int)state->f0282Entered);
    hash = hash_step(hash, (unsigned int)state->f0333MaterializeCount);
    hash = hash_step(hash, (unsigned int)state->f0334RelinkCount);
    hash = hash_step(hash, (unsigned int)state->f0297PutLeaderHandCount);
    hash = hash_step(hash, (unsigned int)state->f0298RemoveLeaderHandCount);
    hash = hash_step(hash, (unsigned int)state->f0300RemoveSlotCount);
    hash = hash_step(hash, (unsigned int)state->f0301AddSlotCount);
    for (i = 0; i < DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->sourceChain[i]);
        hash = hash_step(hash, (unsigned int)state->visibleSlots[i]);
    }
    return hash;
}

static uint32_t hash_transition_surface(
    const Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state)
{
    uint32_t hash = hash_close_surface(state);

    hash = hash_step(hash, (unsigned int)state->transitionKind);
    hash = hash_step(hash, (unsigned int)state->transitionApplied);
    hash = hash_step(hash, (unsigned int)state->panelContentAfterTransition);
    hash = hash_step(hash, (unsigned int)state->panelRedrawCount);
    hash = hash_step(hash, (unsigned int)state->f0344FoodWaterReadCount);
    hash = hash_step(hash, (unsigned int)state->f0345FoodWaterDrawCount);
    hash = hash_step(hash, (unsigned int)state->c503CloseDispatches);
    return hash;
}

static void seed_slots(uint16_t slots[])
{
    int i;

    for (i = 0; i < DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34; ++i) {
        slots[i] = (uint16_t)(kFirstVisibleThing + i);
    }
}

static void copy_slots(uint16_t dst[], const uint16_t src[])
{
    int i;

    for (i = 0; i < DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34; ++i) {
        dst[i] = src[i];
    }
}

static int slots_equal(const uint16_t lhs[], const uint16_t rhs[])
{
    int i;

    for (i = 0; i < DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34; ++i) {
        if (lhs[i] != rhs[i]) {
            return 0;
        }
    }
    return 1;
}

void dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->transitionKind =
        DM1_V1_MC_C045_AFTER_NC_TRANSITION_C040_CHROME_PC34;
    state->nonCandidateTransition = 1;
    state->leaderIndex = kLeaderIndex;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->leaderEmptyHanded = 0;
    state->leaderHandC30Thing = (uint16_t)kLeaderHandThing;
    state->g0426OpenChest = (uint16_t)kOpenChestThing;
    seed_slots(state->sourceChain);
    copy_slots(state->visibleSlots, state->sourceChain);
    state->c540SlotIndex = kC540SlotIndex;
    state->c540Zone = kZoneC540Chest4;
    state->c540Command = kCommandC061Chest4;
    state->c045Graphic = kGraphicC045ObjectIcons096To127;
    state->panelContentBefore = kPanelFoodWater;
    state->panelContentAfterTransition = kPanelFoodWater;
    state->panelContentAfterClose = kPanelFoodWater;
    state->panelOpen = 1;
    state->candidateChampionOrdinal = 0;
    state->c018LeaderTransitionCommand = kCommandC018SetLeader2;
    state->c019LeaderTransitionCommand = kCommandC019SetLeader3;
    state->c038CancelCommand = kCommandC038InventoryNeck;
    state->f0344FoodWaterReadCount = 2;
    state->f0345FoodWaterDrawCount = 1;
    state->beforeHash = hash_transition_surface(state);
}

static int ready(
    const Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state)
{
    return state && state->contractOnly && !state->leaderEmptyHanded &&
           state->leaderHandC30Thing !=
               DM1_V1_MC_C045_AFTER_NC_NONE_PC34 &&
           state->candidateChampionOrdinal == 0 &&
           state->g0426OpenChest != DM1_V1_MC_C045_AFTER_NC_NONE_PC34 &&
           state->panelOpen && state->panelContentBefore == kPanelFoodWater &&
           state->panelContentAfterTransition == kPanelFoodWater &&
           state->c540SlotIndex == kC540SlotIndex &&
           state->c540Zone == kZoneC540Chest4 &&
           state->c540Command == kCommandC061Chest4 &&
           state->c045Graphic == kGraphicC045ObjectIcons096To127;
}

static int apply_non_candidate_transition(
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state)
{
    if (!ready(state)) {
        return 0;
    }

    /*
     * ReDMCSB: COMMAND.C F0359:1985-1990 only dispatches M568/C040 when the
     * panel route is resurrect/reincarnate and the leader hand is empty.
     * These non-candidate transitions deliberately leave G0299 at zero and
     * preserve the occupied leader hand, so no REVIVE.C F0280/F0282 count is
     * incremented.
     */
    switch (state->transitionKind) {
    case DM1_V1_MC_C045_AFTER_NC_TRANSITION_NONE_PC34:
        state->transitionApplied = 0;
        break;
    case DM1_V1_MC_C045_AFTER_NC_TRANSITION_C038_CANCEL_PC34:
        state->transitionApplied = 1;
        state->panelRedrawCount += 1;
        break;
    case DM1_V1_MC_C045_AFTER_NC_TRANSITION_C040_CHROME_PC34:
        state->transitionApplied = 1;
        state->panelRedrawCount += 2;
        state->f0344FoodWaterReadCount += 2;
        state->f0345FoodWaterDrawCount += 1;
        break;
    case DM1_V1_MC_C045_AFTER_NC_TRANSITION_C503_C018_CHROME_PC34:
        state->transitionApplied = 1;
        state->c503CloseDispatches += 1;
        state->panelRedrawCount += 1;
        break;
    case DM1_V1_MC_C045_AFTER_NC_TRANSITION_PANEL_REDRAW_PC34:
        state->transitionApplied = 1;
        state->panelRedrawCount += 3;
        state->f0345FoodWaterDrawCount += 1;
        break;
    default:
        return 0;
    }

    state->panelContentAfterTransition = kPanelFoodWater;
    state->transitionHash = hash_transition_surface(state);
    return 1;
}

static int close_c045_after_transition(
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state)
{
    if (!ready(state)) {
        return 0;
    }

    /*
     * ReDMCSB: PANEL.C F0354:2299-2352 is the panel close/chrome owner.
     * This C045 close is scoped to food/water panel chrome, not inventory
     * teardown, so it must not call CHEST.C F0334.  Because it never clicks a
     * C537..C544 slot through CHAMPION.C F0302:662-714, it also must not
     * route through F0300/F0301 or rematerialize the chain via F0333.
     */
    ++state->f0354CloseCount;
    ++state->c503CloseDispatches;
    state->panelOpen = 0;
    state->panelContentAfterClose = kPanelClosed;
    state->closeHash = hash_close_surface(state);
    return 1;
}

static uint32_t baseline_no_transition_close_hash(
    const Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *base)
{
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat probe = *base;

    probe.transitionKind = DM1_V1_MC_C045_AFTER_NC_TRANSITION_NONE_PC34;
    probe.transitionApplied = 0;
    probe.transitionHash = 0;
    if (!close_c045_after_transition(&probe)) {
        return 0;
    }
    return probe.closeHash;
}

static int guard_rejects(
    const Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *base,
    int kind)
{
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat probe = *base;
    Dm1V1MirrorCandidateC045AfterNonCandidateResultPc34Compat result;

    if (kind == 0) {
        probe.leaderEmptyHanded = 1;
        probe.leaderHandC30Thing = DM1_V1_MC_C045_AFTER_NC_NONE_PC34;
    } else if (kind == 1) {
        probe.candidateChampionOrdinal = 4;
    } else if (kind == 2) {
        probe.c540Zone = 541;
    } else {
        probe.g0426OpenChest = DM1_V1_MC_C045_AFTER_NC_NONE_PC34;
    }
    return dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
               &probe, &result) == 0;
}

int dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state,
    Dm1V1MirrorCandidateC045AfterNonCandidateResultPc34Compat *result)
{
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat base;
    int transitioned;
    int closed;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!ready(state)) {
        return 0;
    }

    base = *state;
    result->leaderHandBefore = state->leaderHandC30Thing;
    result->g0426Before = state->g0426OpenChest;
    transitioned = apply_non_candidate_transition(state);
    closed = close_c045_after_transition(state);

    result->transitionWasAfterNonCandidate =
        transitioned && state->nonCandidateTransition &&
        state->candidateChampionOrdinal == 0;
    result->closeFiredAfterTransition =
        closed &&
        (state->transitionKind ==
             DM1_V1_MC_C045_AFTER_NC_TRANSITION_NONE_PC34 ||
         state->transitionApplied);
    result->leaderHandAfter = state->leaderHandC30Thing;
    result->leaderHandPreserved =
        state->leaderHandC30Thing == base.leaderHandC30Thing &&
        !state->leaderEmptyHanded;
    result->c30ChainPreserved =
        slots_equal(state->sourceChain, base.sourceChain) &&
        slots_equal(state->visibleSlots, base.visibleSlots);
    result->noLeaderHandMutation =
        state->f0297PutLeaderHandCount == 0 &&
        state->f0298RemoveLeaderHandCount == 0 &&
        state->f0300RemoveSlotCount == 0 &&
        state->f0301AddSlotCount == 0 &&
        state->f0302SlotCommandCount == 0;
    result->noF0280C040Entry = state->f0280Entered == 0;
    result->noF0282CandidateEntry = state->f0282Entered == 0;
    result->g0426After = state->g0426OpenChest;
    result->g0426Preserved = state->g0426OpenChest == base.g0426OpenChest;
    result->visibleSlotsPreserved =
        slots_equal(state->visibleSlots, base.visibleSlots);
    result->c540RoutePreserved =
        state->c540SlotIndex == base.c540SlotIndex &&
        state->c540Zone == kZoneC540Chest4 &&
        state->c540Command == base.c540Command;
    result->noF0333MaterializeOnClose = state->f0333MaterializeCount == 0;
    result->noF0334RelinkOnClose = state->f0334RelinkCount == 0;
    result->noC040Dispatch =
        state->f0359C040DispatchCount == 0 &&
        state->c045Graphic != kGraphicC040ResurrectReincarnate;
    result->c045PanelClosed =
        state->panelOpen == 0 && state->panelContentAfterClose == kPanelClosed;
    result->c503CloseObserved =
        state->c503CloseDispatches >= base.c503CloseDispatches + 1 &&
        state->f0354CloseCount == base.f0354CloseCount + 1;
    result->foodWaterReadStable =
        state->f0344FoodWaterReadCount >= base.f0344FoodWaterReadCount &&
        state->f0345FoodWaterDrawCount >= base.f0345FoodWaterDrawCount;
    result->baselineCloseHash = baseline_no_transition_close_hash(&base);
    result->deterministicAgainstNoTransition =
        state->closeHash == result->baselineCloseHash;
    result->transitionHashChanged =
        state->transitionKind ==
            DM1_V1_MC_C045_AFTER_NC_TRANSITION_NONE_PC34 ||
        state->transitionHash != base.beforeHash;
    result->closeHashStable = state->closeHash != 0u;
    result->guardRejectsEmptyLeaderHand = guard_rejects(&base, 0);
    result->guardRejectsCandidate = guard_rejects(&base, 1);
    result->guardRejectsWrongRoute = guard_rejects(&base, 2);
    result->guardRejectsClosedChest = guard_rejects(&base, 3);
    copy_slots(result->visibleSlotsAfter, state->visibleSlots);
    result->hash = hash_step(state->closeHash, (unsigned int)result->accepted);
    result->accepted =
        result->transitionWasAfterNonCandidate &&
        result->closeFiredAfterTransition &&
        result->leaderHandPreserved && result->c30ChainPreserved &&
        result->noLeaderHandMutation && result->noF0280C040Entry &&
        result->noF0282CandidateEntry && result->g0426Preserved &&
        result->visibleSlotsPreserved && result->c540RoutePreserved &&
        result->noF0333MaterializeOnClose && result->noF0334RelinkOnClose &&
        result->noC040Dispatch && result->c045PanelClosed &&
        result->c503CloseObserved && result->foodWaterReadStable &&
        result->deterministicAgainstNoTransition &&
        result->transitionHashChanged && result->closeHashStable &&
        result->guardRejectsEmptyLeaderHand && result->guardRejectsCandidate &&
        result->guardRejectsWrongRoute && result->guardRejectsClosedChest;
    result->hash = hash_step(state->closeHash, (unsigned int)result->accepted);
    return result->accepted;
}

const Dm1V1MirrorCandidateC045AfterNonCandidateEvidencePc34Compat *
dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_evidence_pc34(void)
{
    return &kEvidence;
}

const char *
dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_source_evidence_pc34(void)
{
    return kSourceEvidence;
}
