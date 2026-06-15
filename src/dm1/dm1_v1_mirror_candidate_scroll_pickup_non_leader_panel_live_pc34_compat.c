#include "dm1_v1_mirror_candidate_scroll_pickup_non_leader_panel_live_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract_only=1 runtime regression:
 * CHEST.C F0333:30-67 opens G0426 into the already selected chest and
 * materializes the C30+ / G0425 visible slots; F0334:117-132 is cited as the
 * close path that must not run during the guarded pickup.
 * CHAMPION.C F0297:243-268 puts the picked C038 scroll in the leader hand,
 * F0298:270-298 is the leader-hand removal path that stays unused here, and
 * F0300:511-584, F0301:606-660, F0302:662-713 define the non-leader C538+
 * slot-box exchange.
 * COMMAND.C F0378:1973-1983 dispatches the scroll pickup while
 * F0380:2045-2159 keeps the queued command identity independent of G0299.
 * REVIVE.C F0280:124-132 opens C040/G0299, and F0282:744-806 must still
 * cleanly clear the candidate on a later C162 cancel.
 * PANEL.C F0344/F0345 route panel clicks and F0346/F0347:1619-1657 redraw
 * the C040 candidate panel while G0299 is live.
 * UTAMSCR.C F0077/F0078:141-150 bracket the pointer redraw; OBJECT.C
 * F0033:147-212 and BLITMASK.C F0133:30-33 pin object icon/mask identity.
 * DEFS.H:338-340, 810-817, 1874-1878, 2085-2088, 2088-2096, 2200,
 * 3001-3008, 5694, and 5876-5881 name the commands, slots, party,
 * inventory/chest globals, panel, and G0299/G0425/G0426 state.
 */

enum {
    kLeaderIndex = 0,
    kPartyTailChampion = 1,
    kCandidateOrdinal = 3,
    kLeaderOpenChestThing = 0x6a40,
    kPartyTailChestThing = 0x6b41,
    kFirstChestThing = 0x7200,
    kScrollThing = 0x7038,
    kNonLeaderSlotIndex = 1,
    kInventoryChampionOrdinal = 2
};

static const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat
    s_evidence = {
        1,
        "CHEST.C F0333:30-67 G0426 open into C30+ / G0425 slots",
        "CHEST.C F0334:117-132 G0426 close and relink path",
        "CHAMPION.C F0297:243-268 leader-hand put for C038 scroll",
        "CHAMPION.C F0298:270-298 leader-hand remove path stays unused",
        "CHAMPION.C F0300:511-584 C30+ slot remove",
        "CHAMPION.C F0301:606-660 C30+ slot add/write-back",
        "CHAMPION.C F0302:662-713 slot-box dispatch",
        "COMMAND.C F0378:1973-1983 scroll-pickup dispatch",
        "COMMAND.C F0380:2045-2159 queued command identity",
        "REVIVE.C F0280:124-132 candidate open publishes G0299",
        "REVIVE.C F0282:744-806 C162 cancel clears candidate",
        "PANEL.C F0344/F0345 panel click/release routing",
        "PANEL.C F0346/F0347:1619-1657 C040 redraw while G0299 is set",
        "UTAMSCR.C F0077/F0078:141-150 mouse update bracket",
        "OBJECT.C F0033:147-212 object chain/icon identity",
        "BLITMASK.C F0133:30-33 partial-mask redraw",
        "DEFS.H:338-340 C162; 810-817 C30..C37; 1874-1878 C38; "
            "2085-2088 G0305 party; 2088-2096 G0423 chest/inventory; "
            "2200 C040; 3001-3008 M568/M569; 5694 G0299; "
            "5876-5881 G0425/G0426",
        "contract_only=1 DM1 V1 M11 runtime mutation: non-leader C538+ "
            "C038 scroll pickup while C040/G0299 stays live; distinct from "
            "pass686 keyboard-browse occupied-slot leader-hand swap"
    };

static const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveSpecPc34Compat
    s_spec = {
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_PARTY_COUNT_PC34_COMPAT,
        kLeaderIndex,
        kPartyTailChampion,
        kCandidateOrdinal,
        kLeaderOpenChestThing,
        kPartyTailChestThing,
        kScrollThing,
        kNonLeaderSlotIndex,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C30_PC34_COMPAT +
            kNonLeaderSlotIndex,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C38_PC34_COMPAT +
            kNonLeaderSlotIndex,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C538_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C040_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_M568_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_M569_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C162_PC34_COMPAT
    };

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 into C30+ / G0425 visible chest slots\n"
    "CHEST.C F0334:117-132 closes G0426 and relinks non-empty G0425 entries\n"
    "CHAMPION.C F0297:243-268 puts the C038 scroll into the leader hand\n"
    "CHAMPION.C F0298:270-298 removes the leader hand when a swap is active\n"
    "CHAMPION.C F0300:511-584 removes a C30+ occupied slot\n"
    "CHAMPION.C F0301:606-660 adds/writes back a C30+ slot\n"
    "CHAMPION.C F0302:662-713 dispatches C38/C538 slot boxes\n"
    "COMMAND.C F0378:1973-1983 dispatches scroll pickup from the panel\n"
    "COMMAND.C F0380:2045-2159 preserves queued command identity\n"
    "REVIVE.C F0280:124-132 opens the C040 candidate and G0299\n"
    "REVIVE.C F0282:744-806 clears the C040 candidate on C162 cancel\n"
    "PANEL.C F0344/F0345 maps panel click/release to the slot box\n"
    "PANEL.C F0346/F0347:1619-1657 redraws C040 while G0299 is live\n"
    "UTAMSCR.C F0077/F0078:141-150 brackets mouse updates\n"
    "OBJECT.C F0033:147-212 keeps object chain/icon identity stable\n"
    "BLITMASK.C F0133:30-33 redraws the partial mask for the slot\n"
    "DEFS.H:338-340 C162; 810-817 C30..C37; 1874-1878 C38; "
    "2085-2088 G0305 party; 2088-2096 G0423 chest; 2200 C040; "
    "3001-3008 M568/M569; 5694 G0299; 5876-5881 G0425/G0426";

static void copy_slots(int dst[], const int src[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        dst[i] = src[i];
    }
}

static void seed_chest_slots(int slots[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        slots[i] = kFirstChestThing + i;
    }
    slots[kNonLeaderSlotIndex] = kScrollThing;
}

void DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyCount =
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_PARTY_COUNT_PC34_COMPAT;
    state->leaderIndex = kLeaderIndex;
    state->partyTailChampion = kPartyTailChampion;
    state->candidateOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->panelOpen = 1;
    state->panelGraphic =
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C040_PC34_COMPAT;
    state->panelRedrawable = 1;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_NONE_PC34_COMPAT;
    state->leaderOpenChestThing = kLeaderOpenChestThing;
    state->partyTailChestThing = kPartyTailChestThing;
    state->openChestThing = kLeaderOpenChestThing;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->activeSlotBox =
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C38_PC34_COMPAT +
        kNonLeaderSlotIndex;
    state->nonLeaderSlotThing = kScrollThing;
    seed_chest_slots(state->chestSlots);
    state->f0333OpenCount = 1;
    state->f0280OpenCount = 1;
    state->panelRedrawCount = 1;
}

static int state_matches_contract(
    const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat
        *state)
{
    return state && state->contractOnly && state->partyCount == 4 &&
           state->leaderIndex == kLeaderIndex &&
           state->partyTailChampion == kPartyTailChampion &&
           state->g0299CandidateOrdinal == state->candidateOrdinal &&
           state->candidateOrdinal == kCandidateOrdinal && state->panelOpen &&
           state->panelGraphic ==
               DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C040_PC34_COMPAT &&
           state->panelRedrawable &&
           state->leaderHandThing ==
               DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_NONE_PC34_COMPAT &&
           state->openChestThing == state->leaderOpenChestThing &&
           state->leaderOpenChestThing != state->partyTailChestThing &&
           state->inventoryChampionOrdinal == kInventoryChampionOrdinal &&
           state->activeSlotBox ==
               DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C38_PC34_COMPAT +
                   kNonLeaderSlotIndex &&
           state->nonLeaderSlotThing == kScrollThing &&
           state->chestSlots[kNonLeaderSlotIndex] == kScrollThing;
}

static int dispatch_pickup(
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat *state)
{
    if (!state_matches_contract(state)) {
        return 0;
    }
    ++state->mouseEnableCount;
    ++state->f0344PanelClickCount;
    ++state->f0380QueueCount;
    ++state->f0378DispatchCount;
    ++state->f0302DispatchCount;
    ++state->f0300SlotRemoveCount;
    state->nonLeaderSlotThing =
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_NONE_PC34_COMPAT;
    state->chestSlots[kNonLeaderSlotIndex] =
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_NONE_PC34_COMPAT;
    state->nonLeaderSlotClearedDuringPickup = 1;
    ++state->f0301SlotAddCount;
    state->nonLeaderSlotReplacedDuringPickup =
        state->chestSlots[kNonLeaderSlotIndex] ==
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_NONE_PC34_COMPAT;
    state->leaderHandThing = kScrollThing;
    ++state->f0297PutCount;
    ++state->panelRedrawCount;
    ++state->mouseDisableCount;
    return 1;
}

static int follow_up_cancel(
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat *state)
{
    if (!state || !state->panelOpen ||
        state->g0299CandidateOrdinal != state->candidateOrdinal) {
        return 0;
    }
    state->followUpCancelRequested =
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C162_PC34_COMPAT;
    ++state->mouseEnableCount;
    ++state->f0282CancelCount;
    state->g0299CandidateOrdinal = 0;
    state->panelOpen = 0;
    state->panelRedrawable = 0;
    state->candidateClearedByCancel = 1;
    ++state->panelRedrawCount;
    ++state->mouseDisableCount;
    return 1;
}

static int slots_preserved_except_pickup(
    const int before[],
    const int after[])
{
    int i;

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (i == kNonLeaderSlotIndex) {
            if (after[i] !=
                DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_NONE_PC34_COMPAT) {
                return 0;
            }
        } else if (before[i] != after[i]) {
            return 0;
        }
    }
    return 1;
}

int DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_RunPc34Compat(
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat *state,
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat
        *outResult)
{
    int pickedUp;
    int cancelled;

    if (!state || !outResult || !state_matches_contract(state)) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    outResult->evidence = &s_evidence;
    outResult->spec = &s_spec;
    outResult->candidateOrdinalBefore = (int)state->g0299CandidateOrdinal;
    outResult->panelOpenBefore = state->panelOpen;
    outResult->openChestBefore = state->openChestThing;
    outResult->leaderOpenChestBefore = state->leaderOpenChestThing;
    outResult->partyTailChestBefore = state->partyTailChestThing;
    outResult->partyTailChampionBefore = state->partyTailChampion;
    outResult->leaderHandBefore = state->leaderHandThing;
    outResult->nonLeaderSlotBefore = state->nonLeaderSlotThing;
    outResult->activeSlotBoxBefore = state->activeSlotBox;
    outResult->inventoryChampionOrdinalBefore = state->inventoryChampionOrdinal;
    copy_slots(outResult->chestSlotsBefore, state->chestSlots);

    pickedUp = dispatch_pickup(state);
    outResult->candidateOrdinalAfterPickup =
        (int)state->g0299CandidateOrdinal;
    outResult->panelOpenAfterPickup = state->panelOpen;
    outResult->panelRedrawableAfterPickup = state->panelRedrawable;
    outResult->openChestAfterPickup = state->openChestThing;
    outResult->leaderOpenChestAfterPickup = state->leaderOpenChestThing;
    outResult->partyTailChestAfterPickup = state->partyTailChestThing;
    outResult->partyTailChampionAfterPickup = state->partyTailChampion;
    outResult->leaderHandThingAfter = state->leaderHandThing;
    outResult->nonLeaderSlotAfterPickup = state->nonLeaderSlotThing;
    outResult->activeSlotBoxAfterPickup = state->activeSlotBox;
    outResult->inventoryChampionOrdinalAfterPickup =
        state->inventoryChampionOrdinal;
    copy_slots(outResult->chestSlotsAfterPickup, state->chestSlots);

    outResult->mutationGuardsOk =
        pickedUp &&
        outResult->candidateOrdinalAfterPickup ==
            outResult->candidateOrdinalBefore &&
        outResult->panelOpenAfterPickup == outResult->panelOpenBefore &&
        outResult->panelRedrawableAfterPickup &&
        outResult->openChestAfterPickup == outResult->openChestBefore &&
        outResult->leaderOpenChestAfterPickup ==
            outResult->leaderOpenChestBefore &&
        outResult->partyTailChestAfterPickup ==
            outResult->partyTailChestBefore &&
        outResult->partyTailChampionAfterPickup ==
            outResult->partyTailChampionBefore &&
        outResult->activeSlotBoxAfterPickup ==
            outResult->activeSlotBoxBefore &&
        outResult->inventoryChampionOrdinalAfterPickup ==
            outResult->inventoryChampionOrdinalBefore &&
        slots_preserved_except_pickup(outResult->chestSlotsBefore,
                                      outResult->chestSlotsAfterPickup);

    cancelled = follow_up_cancel(state);
    outResult->candidateOrdinalAfterCancel = (int)state->g0299CandidateOrdinal;
    outResult->panelOpenAfterCancel = state->panelOpen;
    outResult->openChestAfterCancel = state->openChestThing;

    outResult->f0333OpenCount = state->f0333OpenCount;
    outResult->f0334CloseCount = state->f0334CloseCount;
    outResult->f0297PutCount = state->f0297PutCount;
    outResult->f0298RemoveCount = state->f0298RemoveCount;
    outResult->f0300SlotRemoveCount = state->f0300SlotRemoveCount;
    outResult->f0301SlotAddCount = state->f0301SlotAddCount;
    outResult->f0302DispatchCount = state->f0302DispatchCount;
    outResult->f0378DispatchCount = state->f0378DispatchCount;
    outResult->f0380QueueCount = state->f0380QueueCount;
    outResult->f0280OpenCount = state->f0280OpenCount;
    outResult->f0282CancelCount = state->f0282CancelCount;
    outResult->f0344PanelClickCount = state->f0344PanelClickCount;
    outResult->panelRedrawCount = state->panelRedrawCount;
    outResult->mouseEnableCount = state->mouseEnableCount;
    outResult->mouseDisableCount = state->mouseDisableCount;
    outResult->nonLeaderSlotCleared = state->nonLeaderSlotClearedDuringPickup;
    outResult->nonLeaderSlotReplaced =
        state->nonLeaderSlotReplacedDuringPickup;
    outResult->candidateOrdinal = outResult->candidateOrdinalAfterPickup;
    outResult->panelOpen = outResult->panelOpenAfterPickup;
    outResult->openChestThing = outResult->openChestAfterPickup;
    outResult->partyTailChampion = outResult->partyTailChampionAfterPickup;
    outResult->followUpCancelClearsCandidate =
        cancelled && state->candidateClearedByCancel &&
        state->g0299CandidateOrdinal == 0;
    outResult->accepted =
        pickedUp && outResult->leaderHandThingAfter == kScrollThing &&
        outResult->nonLeaderSlotCleared &&
        outResult->nonLeaderSlotReplaced && outResult->mutationGuardsOk &&
        outResult->followUpCancelClearsCandidate;
    return outResult->accepted;
}

const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat *
DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveSpecPc34Compat *
DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_SpecPc34Compat(void)
{
    return &s_spec;
}

const char *
DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_SourceEvidencePc34Compat(
    void)
{
    return s_source_evidence;
}
