#include "dm1_v1_mirror_candidate_full_chain_pc34_compat.h"

#include <string.h>

/* Full-chain source-lock anchors:
 * ReDMCSB CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143
 * loops every party champion and calls F0292 on each click-time redraw.
 * ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection:93-131 rotates
 * champion Cell/Direction with the party direction delta.
 * ReDMCSB CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand:243-268 rejects
 * THING_NONE and otherwise fills G4055/G0415, refreshes the pointer/name,
 * and redraws the leader state.
 * ReDMCSB COMMAND.C F0359:1985-1990 keeps M568/C040 candidate-panel input
 * behind G0415_ui_LeaderEmptyHanded, and REVIVE.C F0282:744-806 owns the
 * panel clear paths after a candidate decision.
 */

enum {
    kLeaderRow = 0,
    kNonLeaderRow = 1,
    kFirstCandidateRow = 2,
    kRotatedCandidateRow = 3,
    kLeaderOrdinal = 1,
    kNonLeaderOrdinal = 2,
    kFirstCandidateOrdinal = 3,
    kRotatedCandidateOrdinal = 4,
    kInitialPartyDirection = 0,
    kRotatedPartyDirection = 1,
    kBlockingHandThing = 0x0BEEu
};

static const Dm1V1MirrorCandidateFullChainEvidencePc34Compat s_evidence = {
    "ReDMCSB CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143",
    "ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection:93-131",
    "ReDMCSB CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand:243-268",
    "ReDMCSB COMMAND.C F0359:1985-1990 M568/C040 leader-empty panel gate",
    "ReDMCSB REVIVE.C F0280:124-132,272-276 publish; F0282:744-806 clear",
    "non-overlap: this is the open -> candidate icon click -> party "
    "rotation/fresh candidate -> rotated icon click -> leader-hand pickup "
    "chain; it is not an isolated C159, cancel, close, keyboard, spell, "
    "inventory, right-click, pending-hand, or occupied-hand-only slice",
    "contract-only deterministic DM1 V1 mirror candidate runtime chain; no "
    "real-asset portrait or framebuffer parity is claimed"
};

static int normalize_direction(int direction)
{
    while (direction < 0) {
        direction += 4;
    }
    return direction & 3;
}

static void init_champion(
    Dm1V1MirrorCandidateFullChainChampionPc34Compat *champion,
    unsigned int ordinal,
    unsigned int handThing,
    int direction,
    int wounded,
    int poisoned)
{
    memset(champion, 0, sizeof(*champion));
    champion->ordinal = ordinal;
    champion->handThing = handThing;
    champion->present = 1;
    champion->direction = direction;
    champion->cell = direction;
    champion->wounded = wounded;
    champion->poisoned = poisoned;
}

void DM1_V1_MirrorCandidateFullChain_InitPc34Compat(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount =
        DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_CHAMPION_COUNT_PC34_COMPAT;
    state->leaderIndex = kLeaderRow;
    state->nonLeaderChampionCount = 1;
    state->partyDirection = kInitialPartyDirection;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_THING_NONE_PC34_COMPAT;
    state->leaderHandEmpty = 1;
    state->candidateChampionOrdinal = kFirstCandidateOrdinal;
    state->activeCandidateOrdinal = kFirstCandidateOrdinal;
    state->activeCandidateRowIndex = kFirstCandidateRow;

    init_champion(&state->champions[kLeaderRow],
                  kLeaderOrdinal,
                  0xC001u,
                  kInitialPartyDirection,
                  0,
                  0);
    init_champion(&state->champions[kNonLeaderRow],
                  kNonLeaderOrdinal,
                  0xC002u,
                  kInitialPartyDirection,
                  0,
                  0);
    init_champion(&state->champions[kFirstCandidateRow],
                  kFirstCandidateOrdinal,
                  0xC159u,
                  kInitialPartyDirection,
                  0,
                  0);
    init_champion(&state->champions[kRotatedCandidateRow],
                  kRotatedCandidateOrdinal,
                  0xC15Au,
                  kInitialPartyDirection,
                  1,
                  1);
}

void DM1_V1_MirrorCandidateFullChain_SetLeaderHandFullBeforePickupPc34Compat(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    int leaderHandFullBeforePickup)
{
    if (!state) {
        return;
    }
    state->leaderHandFullBeforePickup = leaderHandFullBeforePickup ? 1 : 0;
}

static void open_candidate_panel(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    Dm1V1MirrorCandidateFullChainResultPc34Compat *result)
{
    if (!state->contractOnly || state->candidateChampionOrdinal == 0u ||
        state->panelContent != 0 || !state->leaderHandEmpty) {
        return;
    }
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_M568_PANEL_PC34_COMPAT;
    state->c040PanelOpen = 1;
    state->c040PanelPixelsDrawn =
        DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_C040_GRAPHIC_PC34_COMPAT;
    ++state->openedPanelCount;
    result->openedPanel = 1;
}

static void draw_all_champion_states(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state)
{
    int i;

    ++state->allStateDrawCount;
    for (i = 0; i < state->partyChampionCount; ++i) {
        Dm1V1MirrorCandidateFullChainChampionPc34Compat *champion =
            &state->champions[i];
        if (!champion->present) {
            continue;
        }
        ++champion->drawStateCount;
        if (champion->wounded || champion->poisoned) {
            ++state->woundedPoisonedDrawCount;
        }
    }
}

static void click_candidate_icon(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    int rowIndex,
    Dm1V1MirrorCandidateFullChainResultPc34Compat *result)
{
    Dm1V1MirrorCandidateFullChainChampionPc34Compat *champion;

    if (!state->c040PanelOpen || rowIndex < 0 ||
        rowIndex >= state->partyChampionCount) {
        return;
    }
    champion = &state->champions[rowIndex];
    if (!champion->present || champion->ordinal == 0u) {
        return;
    }

    /* ReDMCSB CHAMDRAW.C F0293:1117-1143 redraws all party states at the
     * icon click boundary, including wounded/poisoned state badges. */
    draw_all_champion_states(state);
    ++champion->iconClickCount;
    state->activeCandidateRowIndex = rowIndex;
    state->activeCandidateOrdinal = champion->ordinal;
    state->candidateChampionOrdinal = champion->ordinal;
    if (rowIndex == kFirstCandidateRow) {
        ++state->firstCandidateIconClickCount;
        result->firstIconClicked = 1;
    } else if (rowIndex == kRotatedCandidateRow) {
        ++state->rotatedCandidateIconClickCount;
        result->rotatedIconClicked = 1;
    }
}

static void set_party_direction(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    int newDirection)
{
    int delta;
    int i;

    if (state->partyDirection == newDirection) {
        return;
    }
    delta = newDirection - state->partyDirection;
    if (delta < 0) {
        delta += 4;
    }
    for (i = 0; i < state->partyChampionCount; ++i) {
        Dm1V1MirrorCandidateFullChainChampionPc34Compat *champion =
            &state->champions[i];
        if (!champion->present) {
            continue;
        }
        champion->cell = normalize_direction(champion->cell + delta);
        champion->direction = normalize_direction(champion->direction + delta);
    }
    state->partyDirection = newDirection;
    ++state->partyDirectionSetCount;
}

static void rotate_to_fresh_candidate(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    Dm1V1MirrorCandidateFullChainResultPc34Compat *result)
{
    if (!state->c040PanelOpen ||
        state->activeCandidateRowIndex != kFirstCandidateRow) {
        return;
    }
    result->activeCandidateBeforeRotation = (int)state->activeCandidateOrdinal;
    result->partyDirectionBeforeRotation = state->partyDirection;
    result->rotatedCandidateDirectionBefore =
        state->champions[kRotatedCandidateRow].direction;

    set_party_direction(state, kRotatedPartyDirection);
    state->activeCandidateRowIndex = kRotatedCandidateRow;
    state->activeCandidateOrdinal =
        state->champions[kRotatedCandidateRow].ordinal;
    state->candidateChampionOrdinal = state->activeCandidateOrdinal;
    ++state->rotatedCandidateCount;

    result->rotatedCandidate = 1;
    result->activeCandidateAfterRotation = (int)state->activeCandidateOrdinal;
    result->partyDirectionAfterRotation = state->partyDirection;
    result->rotatedCandidateDirectionAfter =
        state->champions[kRotatedCandidateRow].direction;
}

static void put_object_in_leader_hand(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    unsigned int thing,
    Dm1V1MirrorCandidateFullChainResultPc34Compat *result)
{
    result->pickupAttempted = 1;
    result->leaderHandEmptyBeforePickup = state->leaderHandEmpty;
    result->leaderHandThingBeforePickup = state->leaderHandThing;

    if (!state->leaderHandEmpty &&
        state->leaderHandThing !=
            DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_THING_NONE_PC34_COMPAT) {
        ++state->occupiedHandRejectCount;
        result->pickupRejectedOccupiedHand = 1;
        return;
    }
    if (thing ==
        DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_THING_NONE_PC34_COMPAT) {
        return;
    }

    /* ReDMCSB CHAMPION.C F0297:250-266 fills the leader hand and redraws
     * leader state for a real object; the candidate panel then clears. */
    state->leaderHandThing = thing;
    state->leaderHandEmpty = 0;
    ++state->leaderHandPutCount;
    state->candidateChampionOrdinal = 0u;
    state->activeCandidateOrdinal = 0u;
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    ++state->candidateClearCount;
    result->pickupSucceeded = 1;
}

static void capture_finish(
    const Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    Dm1V1MirrorCandidateFullChainResultPc34Compat *result)
{
    result->leaderHandEmptyAfterPickup = state->leaderHandEmpty;
    result->leaderHandThingAfterPickup = state->leaderHandThing;
    result->panelOpenAfterPickup = state->c040PanelOpen;
    result->candidateOrdinalAfterPickup = (int)state->candidateChampionOrdinal;
    result->leaderHandPutCountAfterPickup = state->leaderHandPutCount;
    result->occupiedHandRejectCountAfterPickup = state->occupiedHandRejectCount;
    result->candidateClearCountAfterPickup = state->candidateClearCount;
    result->sourceLockedFullChain =
        result->openedPanel &&
        result->firstIconClicked &&
        result->rotatedCandidate &&
        result->rotatedIconClicked &&
        result->pickupAttempted &&
        result->nonLeaderChampionPresent &&
        result->allStateDrawCountAfterFirstClick == 1 &&
        result->allStateDrawCountAfterRotatedClick == 2;
}

int DM1_V1_MirrorCandidateFullChain_RunPc34Compat(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    Dm1V1MirrorCandidateFullChainResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateFullChainResultPc34Compat localResult;
    Dm1V1MirrorCandidateFullChainResultPc34Compat *result =
        outResult ? outResult : &localResult;

    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    if (!state || !state->contractOnly) {
        return 0;
    }
    result->candidateOrdinalBeforeOpen = (int)state->candidateChampionOrdinal;
    result->nonLeaderChampionPresent =
        state->partyChampionCount >= 2 &&
        state->champions[kNonLeaderRow].present &&
        state->champions[kNonLeaderRow].ordinal == kNonLeaderOrdinal;

    open_candidate_panel(state, result);
    result->panelOpenAfterOpen = state->c040PanelOpen;
    result->candidateOrdinalAfterOpen = (int)state->candidateChampionOrdinal;

    click_candidate_icon(state, kFirstCandidateRow, result);
    result->panelOpenAfterFirstClick = state->c040PanelOpen;
    result->candidateOrdinalAfterFirstClick =
        (int)state->candidateChampionOrdinal;
    result->allStateDrawCountAfterFirstClick = state->allStateDrawCount;

    rotate_to_fresh_candidate(state, result);
    result->panelOpenAfterRotation = state->c040PanelOpen;
    result->candidateOrdinalAfterRotation =
        (int)state->candidateChampionOrdinal;

    click_candidate_icon(state, kRotatedCandidateRow, result);
    result->panelOpenAfterRotatedClick = state->c040PanelOpen;
    result->candidateOrdinalAfterRotatedClick =
        (int)state->candidateChampionOrdinal;
    result->allStateDrawCountAfterRotatedClick = state->allStateDrawCount;
    result->woundedPoisonedDrawCountAfterRotatedClick =
        state->woundedPoisonedDrawCount;

    if (state->leaderHandFullBeforePickup) {
        state->leaderHandEmpty = 0;
        state->leaderHandThing = kBlockingHandThing;
    }
    put_object_in_leader_hand(
        state,
        state->champions[kRotatedCandidateRow].handThing,
        result);
    capture_finish(state, result);

    if (state->leaderHandFullBeforePickup) {
        return result->sourceLockedFullChain &&
               result->pickupRejectedOccupiedHand &&
               !result->pickupSucceeded &&
               result->panelOpenAfterPickup == 1 &&
               result->candidateOrdinalAfterPickup == kRotatedCandidateOrdinal;
    }
    return result->sourceLockedFullChain &&
           result->pickupSucceeded &&
           result->leaderHandThingAfterPickup ==
               state->champions[kRotatedCandidateRow].handThing &&
           result->candidateOrdinalAfterPickup == 0 &&
           result->panelOpenAfterPickup == 0;
}

const Dm1V1MirrorCandidateFullChainEvidencePc34Compat *
DM1_V1_MirrorCandidateFullChain_EvidencePc34Compat(void)
{
    return &s_evidence;
}
