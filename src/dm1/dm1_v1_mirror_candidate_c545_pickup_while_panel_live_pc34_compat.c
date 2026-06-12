#include "dm1_v1_mirror_candidate_c545_pickup_while_panel_live_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract_only=1 runtime regression:
 * CHEST.C F0333:30-67 and F0334:117-132 define C537..C544/G0425 chest
 * inventory pickup/drop materialization, which must remain untouched by this
 * floor C545 pickup while C040 is live. CHAMPION.C F0297:243-268,
 * F0298:270-298, F0300:511-584, F0301:606-660, and F0302:662-713 define the
 * leader-hand and slot priority order. COMMAND.C F0378:1973-1983 and
 * F0380:2045-2159 route the pickup flow. REVIVE.C F0280:124-132 and
 * F0282:744-806 are guarded because pickup must not resurrect/reincarnate or
 * silently republish G0299. PANEL.C F0346/F0347:1619-1657 keeps the C040
 * panel redrawn; UTAMSCR.C F0077/F0078:141-150 brackets screen updates;
 * OBJECT.C F0033:147-212 supplies object lookup; BLITMASK.C F0133:30-33
 * anchors masked redraw. DEFS.H:338-340, 810-817, 1874-1878, 2200,
 * 3001-3008, 3906-3913, 4205-4207, 5694, and 5876-5881 name C162,
 * C30..C37, C38, C040, M568/M569, C537..C544, ornament, G0299, G0425, and
 * G0426.
 */

enum {
    kLeaderIndex = 0,
    kPartyChampionCount = 3,
    kCandidateOrdinal = 3,
    kC545FloorThing = 0x7545,
    kAlreadyHeldThing = 0x6A31,
    kPreviousFloorThing = 0x5120,
    kLayerCookie = 0x40C0,
    kZOrderCookie = 0x5450,
    kFirstChestSlotThing = 0x7200,
    kFnvOffset = 2166136261u,
    kFnvPrime = 16777619u
};

static const Dm1V1MirrorC545PickupEvidencePc34Compat s_evidence = {
    1,
    1,
    1,
    "CHEST.C F0333:30-67 G0426 open and C537..C544 materialization",
    "CHEST.C F0334:117-132 G0426 close and G0425 relink",
    "CHAMPION.C F0297:243-268 leader-hand put after pickup",
    "CHAMPION.C F0298:270-298 leader-hand remove guard",
    "CHAMPION.C F0300:511-584 C30+ slot removal guard",
    "CHAMPION.C F0301:606-660 C30+ slot add guard",
    "CHAMPION.C F0302:662-713 hand-slot priority and exchange order",
    "COMMAND.C F0378:1973-1983 command dispatch",
    "COMMAND.C F0380:2045-2159 pickup flow and queue identity",
    "REVIVE.C F0280:124-132 candidate publish gate",
    "REVIVE.C F0282:744-806 resurrect/reincarnate click path",
    "PANEL.C F0346/F0347:1619-1657 C040 draw/refresh while live",
    "UTAMSCR.C F0077/F0078:141-150 utility/action dispatch bracket",
    "OBJECT.C F0033:147-212 object category/icon lookup",
    "BLITMASK.C F0133:30-33 masked redraw operation",
    "DEFS.H:338-340 C162; DEFS.H:810-817 C30..C37; "
        "DEFS.H:1874-1878 C38; DEFS.H:2200 C040; "
        "DEFS.H:3001-3008 M568/M569; DEFS.H:3906-3913 C537..C544; "
        "DEFS.H:4205-4207 ornament; DEFS.H:5694 G0299; "
        "DEFS.H:5876-5881 G0425/G0426",
    "source_locked_contract_only=1 no_real_asset_bitmap_parity=1 "
        "no_game_data_load=1 DM1 V1 C545 floor pickup while M568/C040 "
        "mirror-candidate panel is live; complementary to the C545 drop "
        "direction and guarding against resurrect/reincarnate side effects"
};

static const Dm1V1MirrorC545PickupSpecPc34Compat s_spec = {
    0xC545727u,
    kLeaderIndex,
    kPartyChampionCount,
    kCandidateOrdinal,
    kC545FloorThing,
    kAlreadyHeldThing,
    kPreviousFloorThing,
    DM1_V1_MIRROR_C545_PICKUP_C040_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_C545_PICKUP_M568_CANDIDATE_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_C545_PICKUP_C545_ZONE_PC34_COMPAT,
    DM1_V1_MIRROR_C545_PICKUP_C162_CANCEL_PC34_COMPAT
};

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and copies C537..C544/G0425 slots\n"
    "CHEST.C F0334:117-132 closes G0426 and relinks G0425 slots\n"
    "CHAMPION.C F0297:243-268 puts a picked object in the leader hand\n"
    "CHAMPION.C F0298:270-298 removes a leader-hand object when dropping\n"
    "CHAMPION.C F0300:511-584 removes C30+ slot objects\n"
    "CHAMPION.C F0301:606-660 adds C30+ slot objects\n"
    "CHAMPION.C F0302:662-713 resolves hand-slot exchange priority\n"
    "COMMAND.C F0378:1973-1983 dispatches panel/input commands\n"
    "COMMAND.C F0380:2045-2159 processes pickup queue identity\n"
    "REVIVE.C F0280:124-132 publishes G0299 only through revive open\n"
    "REVIVE.C F0282:744-806 handles resurrect/reincarnate click cleanup\n"
    "PANEL.C F0346/F0347:1619-1657 redraws C040 while candidate is live\n"
    "UTAMSCR.C F0077/F0078:141-150 brackets utility/action updates\n"
    "OBJECT.C F0033:147-212 looks up object category/icon\n"
    "BLITMASK.C F0133:30-33 performs masked redraw operations\n"
    "DEFS.H C162 C30..C37 C38 C040 M568/M569 C537..C544 ornament "
    "G0299 G0425/G0426";

static unsigned int fnv1a_mix(unsigned int hash, unsigned int value)
{
    int shift;

    for (shift = 0; shift < 32; shift += 8) {
        hash ^= (value >> shift) & 0xFFu;
        hash *= kFnvPrime;
    }
    return hash;
}

static void seed_slots(int slots[])
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT; ++i) {
        slots[i] = kFirstChestSlotThing + i;
    }
}

static void copy_slots(int dst[], const int src[])
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT; ++i) {
        dst[i] = src[i];
    }
}

static int slots_equal(const int a[], const int b[])
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static unsigned int panel_hash(
    const Dm1V1MirrorC545PickupStatePc34Compat *state)
{
    unsigned int hash = kFnvOffset;

    hash = fnv1a_mix(hash, (unsigned int)state->c040PanelOpen);
    hash = fnv1a_mix(hash, (unsigned int)state->c040PanelContent);
    hash = fnv1a_mix(hash, (unsigned int)state->c040LayerCookie);
    hash = fnv1a_mix(hash, (unsigned int)state->c040ZOrderCookie);
    hash = fnv1a_mix(hash, state->g0299CandidateOrdinal);
    return hash;
}

static unsigned int deterministic_hash(
    const Dm1V1MirrorC545PickupStatePc34Compat *state,
    const Dm1V1MirrorC545PickupResultPc34Compat *result)
{
    unsigned int hash = kFnvOffset;

    hash = fnv1a_mix(hash, s_spec.deterministicSeed);
    hash = fnv1a_mix(hash, (unsigned int)result->accepted);
    hash = fnv1a_mix(hash, (unsigned int)result->negativeRejected);
    hash = fnv1a_mix(hash, (unsigned int)state->leaderHandThing);
    hash = fnv1a_mix(hash, state->g0299CandidateOrdinal);
    hash = fnv1a_mix(hash, (unsigned int)state->floorThingCount);
    hash = fnv1a_mix(hash, (unsigned int)result->c040Redraws);
    hash = fnv1a_mix(hash, (unsigned int)result->handTransitions);
    hash = fnv1a_mix(hash, (unsigned int)result->panelRejections);
    hash = fnv1a_mix(hash, (unsigned int)result->mirrorCandidateGuard);
    hash = fnv1a_mix(hash, result->afterClosePanelHash);
    return hash;
}

void DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(
    Dm1V1MirrorC545PickupStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->sourceLockedContractOnly = 1;
    state->noRealAssetBitmapParity = 1;
    state->noGameDataLoad = 1;
    state->leaderIndex = kLeaderIndex;
    state->partyChampionCount = kPartyChampionCount;
    state->candidateOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->leaderHandThing =
        DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT;
    state->floorThings[0] = kC545FloorThing;
    state->floorThings[1] = kPreviousFloorThing;
    state->floorThingCount = 2;
    state->c040PanelOpen = 1;
    state->c040PanelContent =
        DM1_V1_MIRROR_C545_PICKUP_M568_CANDIDATE_PANEL_PC34_COMPAT;
    state->c040LayerCookie = kLayerCookie;
    state->c040ZOrderCookie = kZOrderCookie;
    state->c545Zone = DM1_V1_MIRROR_C545_PICKUP_C545_ZONE_PC34_COMPAT;
    seed_slots(state->visibleC537ToC544);
    seed_slots(state->g0425ChestSlots);
    state->f0280ReviveOpenCount = 1;
    state->f0346PanelDrawCount = 1;
    state->f0347PanelRefreshCount = 1;
    state->f0133MaskCount = 1;
}

static int contract_ready(const Dm1V1MirrorC545PickupStatePc34Compat *state)
{
    return state &&
           state->sourceLockedContractOnly &&
           state->noRealAssetBitmapParity &&
           state->noGameDataLoad &&
           state->leaderIndex == kLeaderIndex &&
           state->partyChampionCount == kPartyChampionCount &&
           state->c040PanelOpen &&
           state->c040PanelContent ==
               DM1_V1_MIRROR_C545_PICKUP_M568_CANDIDATE_PANEL_PC34_COMPAT &&
           state->g0299CandidateOrdinal == state->candidateOrdinal &&
           state->candidateOrdinal == kCandidateOrdinal &&
           state->leaderHandThing ==
               DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT &&
           state->floorThingCount >= 1 &&
           state->floorThings[0] == kC545FloorThing &&
           state->c545Zone ==
               DM1_V1_MIRROR_C545_PICKUP_C545_ZONE_PC34_COMPAT;
}

static int c545_pickup_from_floor(
    Dm1V1MirrorC545PickupStatePc34Compat *state,
    int *outPickedThing)
{
    int pickedThing;

    if (!contract_ready(state)) {
        if (state && state->leaderHandThing !=
            DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT) {
            ++state->panelRejectionCount;
        }
        return 0;
    }

    ++state->f0380PickupFlowCount;
    ++state->f0378DispatchCount;
    ++state->f0077EnableCount;
    ++state->f0033ObjectLookupCount;

    /*
     * ReDMCSB COMMAND.C F0380:2045-2159 reaches pickup without going through
     * REVIVE.C F0282:744-806. The live G0299/C040 panel is preserved while
     * F0297 puts the object into the leader hand.
     */
    pickedThing = state->floorThings[0];
    state->floorThings[0] = state->floorThings[1];
    state->floorThings[1] = 0;
    --state->floorThingCount;
    state->leaderHandThing = pickedThing;
    ++state->f0297PutCount;
    ++state->handTransitionCount;
    ++state->f0347PanelRefreshCount;
    ++state->f0133MaskCount;
    ++state->f0078DisableCount;
    if (outPickedThing) {
        *outPickedThing = pickedThing;
    }
    return 1;
}

static int close_c040_without_revive(
    Dm1V1MirrorC545PickupStatePc34Compat *state)
{
    if (!state || !state->c040PanelOpen ||
        state->leaderHandThing ==
            DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT) {
        return 0;
    }

    ++state->f0380PickupFlowCount;
    ++state->f0378DispatchCount;
    ++state->f0077EnableCount;
    state->c040PanelOpen = 0;
    state->c040PanelContent = 0;
    ++state->c040CloseCount;
    ++state->f0347PanelRefreshCount;
    ++state->f0133MaskCount;
    ++state->f0078DisableCount;
    return 1;
}

static int mutation_guard_rejects(
    const Dm1V1MirrorC545PickupStatePc34Compat *base,
    int guardKind)
{
    Dm1V1MirrorC545PickupStatePc34Compat probe;
    int beforeFloorCount;
    int accepted;

    probe = *base;
    beforeFloorCount = probe.floorThingCount;
    switch (guardKind) {
    case 0:
        probe.sourceLockedContractOnly = 0;
        break;
    case 1:
        probe.c040PanelOpen = 0;
        break;
    case 2:
        probe.g0299CandidateOrdinal = 0;
        break;
    case 3:
        probe.leaderHandThing = kAlreadyHeldThing;
        break;
    case 4:
        probe.floorThings[0] = DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT;
        break;
    case 5:
        probe.c545Zone = 0;
        break;
    default:
        return 0;
    }

    accepted = c545_pickup_from_floor(&probe, 0);
    return !accepted &&
           probe.floorThingCount == beforeFloorCount &&
           ((guardKind == 2 && probe.g0299CandidateOrdinal == 0) ||
            (guardKind != 2 &&
             probe.g0299CandidateOrdinal == base->g0299CandidateOrdinal));
}

static int run_mutation_guards(
    const Dm1V1MirrorC545PickupStatePc34Compat *base,
    Dm1V1MirrorC545PickupResultPc34Compat *outResult)
{
    outResult->rejectsNullState =
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            0, outResult) == 0;
    outResult->rejectsNullResult =
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            (Dm1V1MirrorC545PickupStatePc34Compat *)base, 0) == 0;
    outResult->rejectsNonContract = mutation_guard_rejects(base, 0);
    outResult->rejectsNoPanel = mutation_guard_rejects(base, 1);
    outResult->rejectsNoCandidate = mutation_guard_rejects(base, 2);
    outResult->rejectsHandFull = mutation_guard_rejects(base, 3);
    outResult->rejectsNoFloorItem = mutation_guard_rejects(base, 4);
    outResult->rejectsWrongZone = mutation_guard_rejects(base, 5);

    return outResult->rejectsNullState &&
           outResult->rejectsNullResult &&
           outResult->rejectsNonContract &&
           outResult->rejectsNoPanel &&
           outResult->rejectsNoCandidate &&
           outResult->rejectsHandFull &&
           outResult->rejectsNoFloorItem &&
           outResult->rejectsWrongZone;
}

static int run_negative_hand_full(
    const Dm1V1MirrorC545PickupStatePc34Compat *base,
    Dm1V1MirrorC545PickupResultPc34Compat *outResult)
{
    Dm1V1MirrorC545PickupStatePc34Compat probe = *base;
    unsigned int beforePanelHash;
    int accepted;

    probe.leaderHandThing = kAlreadyHeldThing;
    outResult->negativeInitialHand = probe.leaderHandThing;
    outResult->negativeInitialFloorCount = probe.floorThingCount;
    beforePanelHash = panel_hash(&probe);
    accepted = c545_pickup_from_floor(&probe, 0);
    outResult->negativeRejected = !accepted;
    outResult->negativeFinalHand = probe.leaderHandThing;
    outResult->negativeFinalFloorCount = probe.floorThingCount;
    outResult->negativePanelStable = beforePanelHash == panel_hash(&probe);
    outResult->negativeCandidateOrdinal = probe.g0299CandidateOrdinal;
    outResult->negativeItemLost =
        probe.floorThings[0] != kC545FloorThing ||
        probe.floorThingCount != outResult->negativeInitialFloorCount;
    outResult->panelRejections = probe.panelRejectionCount;
    return outResult->negativeRejected &&
           outResult->negativeFinalHand == kAlreadyHeldThing &&
           outResult->negativeFinalFloorCount ==
               outResult->negativeInitialFloorCount &&
           outResult->negativePanelStable &&
           outResult->negativeCandidateOrdinal == kCandidateOrdinal &&
           !outResult->negativeItemLost;
}

int DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
    Dm1V1MirrorC545PickupStatePc34Compat *state,
    Dm1V1MirrorC545PickupResultPc34Compat *outResult)
{
    Dm1V1MirrorC545PickupStatePc34Compat guardBase;
    int visibleBefore[DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int chestBefore[DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int pickedThing = 0;
    int pickupAccepted;
    int closeAccepted;
    int guardsOk;
    int negativeOk;

    if (!state || !outResult) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    outResult->evidence = &s_evidence;
    outResult->spec = &s_spec;
    outResult->initialLeaderHand = state->leaderHandThing;
    outResult->initialFloorThingCount = state->floorThingCount;
    outResult->initialCandidateOrdinal = state->g0299CandidateOrdinal;
    outResult->initialPanelHash = panel_hash(state);
    copy_slots(visibleBefore, state->visibleC537ToC544);
    copy_slots(chestBefore, state->g0425ChestSlots);

    guardBase = *state;
    pickupAccepted = c545_pickup_from_floor(state, &pickedThing);
    if (!pickupAccepted) {
        return 0;
    }

    outResult->pickedThing = pickedThing;
    outResult->afterPickupPanelHash = panel_hash(state);
    outResult->finalLeaderHand = state->leaderHandThing;
    outResult->floorThingCountAfterPickup = state->floorThingCount;
    outResult->candidateOrdinalAfterPickup = state->g0299CandidateOrdinal;
    outResult->panelOpenAfterPickup = state->c040PanelOpen;
    outResult->panelStableAfterPickup =
        outResult->afterPickupPanelHash == outResult->initialPanelHash;

    guardsOk = run_mutation_guards(&guardBase, outResult);
    negativeOk = run_negative_hand_full(&guardBase, outResult);

    closeAccepted = close_c040_without_revive(state);
    outResult->afterClosePanelHash = panel_hash(state);
    outResult->floorThingCountAfterClose = state->floorThingCount;
    outResult->candidateOrdinalAfterClose = state->g0299CandidateOrdinal;
    outResult->panelOpenAfterClose = state->c040PanelOpen;
    outResult->closeCleanWithHandFull =
        closeAccepted &&
        state->leaderHandThing == kC545FloorThing &&
        state->g0299CandidateOrdinal == kCandidateOrdinal;

    outResult->noPanelFlicker = state->panelFlickerCount == 0;
    outResult->noZOrderCorruption = state->zOrderCorruptionCount == 0;
    outResult->noMirrorCandidateSideEffect =
        state->mirrorCandidateSideEffectCount == 0 &&
        state->g0299CandidateOrdinal == kCandidateOrdinal;
    outResult->noReviveTriggerOnPickup =
        state->f0280ReviveOpenCount == guardBase.f0280ReviveOpenCount &&
        state->f0282ReviveClickCount == 0;
    outResult->c040Redraws =
        state->f0346PanelDrawCount + state->f0347PanelRefreshCount;
    outResult->handTransitions = state->handTransitionCount;
    outResult->mirrorCandidateGuard = guardsOk ? 1 : 0;

    outResult->f0333OpenCount = state->f0333OpenCount;
    outResult->f0334CloseCount = state->f0334CloseCount;
    outResult->f0297PutCount = state->f0297PutCount;
    outResult->f0298RemoveCount = state->f0298RemoveCount;
    outResult->f0300SlotRemoveCount = state->f0300SlotRemoveCount;
    outResult->f0301SlotAddCount = state->f0301SlotAddCount;
    outResult->f0302SlotDispatchCount = state->f0302SlotDispatchCount;
    outResult->f0378DispatchCount = state->f0378DispatchCount;
    outResult->f0380PickupFlowCount = state->f0380PickupFlowCount;
    outResult->f0280ReviveOpenCount = state->f0280ReviveOpenCount;
    outResult->f0282ReviveClickCount = state->f0282ReviveClickCount;
    outResult->f0346PanelDrawCount = state->f0346PanelDrawCount;
    outResult->f0347PanelRefreshCount = state->f0347PanelRefreshCount;
    outResult->f0077EnableCount = state->f0077EnableCount;
    outResult->f0078DisableCount = state->f0078DisableCount;
    outResult->f0033ObjectLookupCount = state->f0033ObjectLookupCount;
    outResult->f0133MaskCount = state->f0133MaskCount;

    outResult->accepted =
        pickupAccepted &&
        outResult->pickedThing == kC545FloorThing &&
        outResult->initialLeaderHand ==
            DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT &&
        outResult->finalLeaderHand == kC545FloorThing &&
        outResult->floorThingCountAfterPickup ==
            outResult->initialFloorThingCount - 1 &&
        outResult->panelOpenAfterPickup == 1 &&
        outResult->panelStableAfterPickup &&
        outResult->noPanelFlicker &&
        outResult->noZOrderCorruption &&
        outResult->noMirrorCandidateSideEffect &&
        outResult->noReviveTriggerOnPickup &&
        outResult->closeCleanWithHandFull &&
        slots_equal(visibleBefore, state->visibleC537ToC544) &&
        slots_equal(chestBefore, state->g0425ChestSlots) &&
        negativeOk &&
        guardsOk;
    outResult->deterministicHash = deterministic_hash(state, outResult);
    return outResult->accepted;
}

const Dm1V1MirrorC545PickupEvidencePc34Compat *
DM1_V1_MirrorCandidateC545PickupWhilePanelLive_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const Dm1V1MirrorC545PickupSpecPc34Compat *
DM1_V1_MirrorCandidateC545PickupWhilePanelLive_SpecPc34Compat(void)
{
    return &s_spec;
}

const char *
DM1_V1_MirrorCandidateC545PickupWhilePanelLive_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}
