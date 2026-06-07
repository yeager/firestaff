#include "dm1_v1_mirror_candidate_party_direction_pc34_compat.h"

#include <string.h>

/* Non-overlap: this gate is not a C040 button, click-cancel/deadzone,
 * icon-refresh, inventory-toggle, spell-rune, or keyboard-browse test. It
 * covers a C040-live party-direction input run followed by a champion
 * status-box/name click that would normally enter G0455's C159/C016
 * set-leader route, while G0299 still owns the candidate panel.
 *
 * ReDMCSB anchors:
 * COMMAND.C F0359/F0361/F0380:1709-1806 and 2045-2162 queue/dequeue turns
 * and gate C012..C015 status-box dispatch on !G0299.
 * COMMAND.C G0455:202-215 and 484-496 maps C159 to C016 only after F0367.
 * COMMAND.C F0380:2180-2182,2302-2311,2336-2368 applies sibling !G0299
 * inventory, spell/action, rest, and save gates.
 * CHAMPION.C F0297/F0300/F0301:243-268,489-585,587-625 are object/weight
 * paths intentionally not reached here; CLIKCHAM.C F0368:51-72 would stamp a
 * leader Direction from G0308 but skips candidate redraw while G0299 matches.
 * CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF:2333-2335 skips the live
 * G0299 candidate for time effects.
 * DUNGEON.C:2608-2612 and DUNVIEW.C:3913-3928 carry the C127 portrait route.
 * REVIVE.C F0280:124-132,176-182,272-276 appends/publishes G0299, and
 * REVIVE.C F0282:744-806 owns the C160/C161/C162 C040 panel decisions.
 * PANEL.C F0346:1619-1635 blits C040 with transparency.
 */

enum {
    kCandidateOrdinal = 3,
    kCandidateIdentity = 0x0420,
    kPartyChampionCount = 3,
    kInitialPartyDirection = 0,
    kInitialLeaderIndex = 0,
    kInitialPanelContent = DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_M568_PANEL_PC34_COMPAT,
    kPanelSeedPixel = 0x77,
    kPanelOpaquePixel = DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C040_GRAPHIC_PC34_COMPAT
};

static const unsigned int kSyntheticPortraitToken = 0xC1270420u;

static const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat
    s_evidence = {
        "COMMAND.C F0359/F0361/F0380:1709-1806,2045-2162,2158-2162; "
        "G0455 C159/C016 rows:202-215,484-496; C007/C140/C145/C100/C111 "
        "G0299 gates:2180-2182,2302-2311,2336-2368",
        "CLIKCHAM.C F0367/F0368:24-35,51-72; CHAMPION.C F0297/F0300/F0301:"
        "243-268,489-585,587-625; F0331 time effects:2333-2335",
        "DUNGEON.C:2608-2612 C127 portrait ordinal; DUNVIEW.C:3913-3928 "
        "D1C champion portrait blit route",
        "REVIVE.C F0280:124-132,176-182,272-276; REVIVE.C F0282:744-806",
        "PANEL.C F0346:1619-1635 C040 panel blit; DEFS.H C040 graphic:2200",
        "G0299_ui_CandidateChampionOrdinal live panel anchor",
        "G0420 candidate identity marker used by this deterministic gate; "
        "ReDMCSB G0420_B_MousePointerHidden... is only cited as a name "
        "collision anchor in CHAMDRAW.C:1150,1173-1175,1214-1260",
        "non-overlap: exercises five turn inputs plus C159/C016 set-leader "
        "attempt while C040/G0299 is live; does not press C160/C161/C162, "
        "does not browse pages, and does not test inventory/spell/chest paths",
        "deterministic contract-only harness with synthetic C127 portrait "
        "token; no real-asset portrait parity is claimed"
    };

static int normalize_direction(int direction)
{
    while (direction < 0) {
        direction += 4;
    }
    return direction & 3;
}

static void blit_c040_panel(unsigned char *dst, const unsigned char *src)
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_W_PC34_COMPAT *
                    DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_H_PC34_COMPAT;
         ++i) {
        if (src[i] !=
            DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C10_TRANSPARENT_PC34_COMPAT) {
            dst[i] = src[i];
        }
    }
}

static void capture_before(
    const Dm1V1MirrorCandidatePartyDirectionStatePc34Compat *state,
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->candidateIdentityBefore = state->candidateIdentityAnchor;
    result->candidateIdentityAfter = state->candidateIdentityAnchor;
    result->partyChampionCountBefore = state->partyChampionCount;
    result->partyChampionCountAfter = state->partyChampionCount;
    result->partyDirectionBefore = state->partyDirection;
    result->partyDirectionAfter = state->partyDirection;
    result->candidateDirectionBefore = state->candidateDirection;
    result->candidateDirectionAfter = state->candidateDirection;
    result->leaderIndexBefore = state->leaderIndex;
    result->leaderIndexAfter = state->leaderIndex;
    result->panelContentBefore = state->panelContent;
    result->panelContentAfter = state->panelContent;
    result->c040PanelOpenBefore = state->c040PanelOpen;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->inventoryChampionOrdinalBefore = state->inventoryChampionOrdinal;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->c159NestedReachedBefore = state->c159NestedReached;
    result->c159NestedReachedAfter = state->c159NestedReached;
    result->f0367StatusDispatchCountBefore = state->f0367StatusDispatchCount;
    result->f0367StatusDispatchCountAfter = state->f0367StatusDispatchCount;
    result->f0368SetLeaderCountBefore = state->f0368SetLeaderCount;
    result->f0368SetLeaderCountAfter = state->f0368SetLeaderCount;
    result->duplicateCandidateAppendCountBefore =
        state->duplicateCandidateAppendCount;
    result->duplicateCandidateAppendCountAfter =
        state->duplicateCandidateAppendCount;
    result->resurrectDispatchCountBefore = state->resurrectDispatchCount;
    result->resurrectDispatchCountAfter = state->resurrectDispatchCount;
    result->reincarnateDispatchCountBefore = state->reincarnateDispatchCount;
    result->reincarnateDispatchCountAfter = state->reincarnateDispatchCount;
    result->cancelDispatchCountBefore = state->cancelDispatchCount;
    result->cancelDispatchCountAfter = state->cancelDispatchCount;
    result->restDispatchCountBefore = state->restDispatchCount;
    result->restDispatchCountAfter = state->restDispatchCount;
    result->saveDispatchCountBefore = state->saveDispatchCount;
    result->saveDispatchCountAfter = state->saveDispatchCount;
}

static void capture_after(
    const Dm1V1MirrorCandidatePartyDirectionStatePc34Compat *state,
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat *result)
{
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    result->candidateIdentityAfter = state->candidateIdentityAnchor;
    result->partyChampionCountAfter = state->partyChampionCount;
    result->partyDirectionAfter = state->partyDirection;
    result->candidateDirectionAfter = state->candidateDirection;
    result->leaderIndexAfter = state->leaderIndex;
    result->panelContentAfter = state->panelContent;
    result->c040PanelOpenAfter = state->c040PanelOpen;
    result->inventoryChampionOrdinalAfter = state->inventoryChampionOrdinal;
    result->c159NestedReachedAfter = state->c159NestedReached;
    result->f0367StatusDispatchCountAfter = state->f0367StatusDispatchCount;
    result->f0368SetLeaderCountAfter = state->f0368SetLeaderCount;
    result->duplicateCandidateAppendCountAfter =
        state->duplicateCandidateAppendCount;
    result->resurrectDispatchCountAfter = state->resurrectDispatchCount;
    result->reincarnateDispatchCountAfter = state->reincarnateDispatchCount;
    result->cancelDispatchCountAfter = state->cancelDispatchCount;
    result->restDispatchCountAfter = state->restDispatchCount;
    result->saveDispatchCountAfter = state->saveDispatchCount;
    result->c10TransparentPixelPreserved = state->panelDestination[0] ==
        kPanelSeedPixel;
    result->c040OpaquePanelPixelCopied = state->panelDestination[1] ==
        kPanelOpaquePixel;
    result->panelPixelContractReal =
        result->c10TransparentPixelPreserved &&
        result->c040OpaquePanelPixelCopied;
    result->portraitContractOnly =
        state->syntheticC127PortraitToken == kSyntheticPortraitToken &&
        state->realAssetPortraitParityClaimed == 0;
    result->g0299AnchorPreserved =
        result->candidateChampionOrdinalBefore ==
        result->candidateChampionOrdinalAfter &&
        result->candidateChampionOrdinalAfter == kCandidateOrdinal;
    result->g0420IdentityPreserved =
        result->candidateIdentityBefore == result->candidateIdentityAfter &&
        result->candidateIdentityAfter == kCandidateIdentity;
    result->panelOwnerPreserved =
        result->panelContentAfter == kInitialPanelContent &&
        result->c040PanelOpenAfter == 1 &&
        result->inventoryChampionOrdinalAfter == (int)kCandidateOrdinal;
    result->noDuplicateCandidate =
        result->partyChampionCountBefore == result->partyChampionCountAfter &&
        result->duplicateCandidateAppendCountBefore ==
            result->duplicateCandidateAppendCountAfter;
}

void DM1_V1_MirrorCandidatePartyDirection_InitPc34Compat(
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->candidateIdentityAnchor = kCandidateIdentity;
    state->syntheticC127PortraitToken = kSyntheticPortraitToken;
    state->partyChampionCount = kPartyChampionCount;
    state->partyDirection = kInitialPartyDirection;
    state->candidateDirection = kInitialPartyDirection;
    state->leaderIndex = kInitialLeaderIndex;
    state->panelContent = kInitialPanelContent;
    state->c040PanelOpen = 1;
    state->inventoryChampionOrdinal = kCandidateOrdinal;
    DM1_V1_InputCommandQueue_InitPc34Compat(&state->queue);

    for (i = 0; i < DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_W_PC34_COMPAT *
                    DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_H_PC34_COMPAT;
         ++i) {
        state->panelSource[i] = (unsigned char)(kPanelOpaquePixel + i);
        state->panelDestination[i] = kPanelSeedPixel;
    }
    state->panelSource[0] =
        DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C10_TRANSPARENT_PC34_COMPAT;
    state->panelSource[1] = kPanelOpaquePixel;
    blit_c040_panel(state->panelDestination, state->panelSource);
}

static void process_turn_or_status(
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat *state,
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat *result)
{
    struct Dm1V1InputQueueProcessResultPc34Compat queueResult =
        DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
            &state->queue, state->partyDirection, 0, 0, 0);

    if (!queueResult.dequeued) {
        return;
    }
    ++result->commandDequeuedCount;
    if (queueResult.command == DM1_V1_COMMAND_TURN_LEFT) {
        state->partyDirection = normalize_direction(state->partyDirection - 1);
        ++result->turnDispatchCount;
        return;
    }
    if (queueResult.command == DM1_V1_COMMAND_TURN_RIGHT) {
        state->partyDirection = normalize_direction(state->partyDirection + 1);
        ++result->turnDispatchCount;
        return;
    }
    if (queueResult.command >= DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0 &&
        queueResult.command <= DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_3) {
        result->statusCommandDequeued = queueResult.command;
        /* ReDMCSB COMMAND.C F0380:2158-2162 blocks F0367 while G0299 is
         * live, so G0455/C159 is never scanned and F0368 cannot steal C040. */
        if (state->candidateChampionOrdinal != 0u) {
            result->statusGateBlockedByG0299 = 1;
            return;
        }
        ++state->f0367StatusDispatchCount;
        if (queueResult.command == DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0) {
            state->c159NestedReached = 1;
            state->leaderIndex = 0;
            state->candidateDirection = state->partyDirection;
            ++state->f0368SetLeaderCount;
        }
    }
}

int DM1_V1_MirrorCandidatePartyDirection_RunFiveTurnC159ScenarioPc34Compat(
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat *state,
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat *outResult)
{
    static const int kTurnCommands[5] = {
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_TURN_LEFT,
        DM1_V1_COMMAND_TURN_RIGHT,
        DM1_V1_COMMAND_TURN_RIGHT
    };
    int i;

    if (!state || !outResult) {
        return 0;
    }
    capture_before(state, outResult);

    for (i = 0; i < 5; ++i) {
        outResult->commandQueuedCount +=
            DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
                &state->queue, kTurnCommands[i], 0, 0);
        process_turn_or_status(state, outResult);
    }

    outResult->commandQueuedCount +=
        DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
            &state->queue,
            DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C012_PC34_COMPAT,
            4,
            3);
    process_turn_or_status(state, outResult);

    capture_after(state, outResult);
    return outResult->g0299AnchorPreserved &&
           outResult->g0420IdentityPreserved &&
           outResult->panelOwnerPreserved &&
           outResult->noDuplicateCandidate &&
           outResult->panelPixelContractReal &&
           outResult->statusGateBlockedByG0299;
}

const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *
DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat(void)
{
    return &s_evidence;
}
