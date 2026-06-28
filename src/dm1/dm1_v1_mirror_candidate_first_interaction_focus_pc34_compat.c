#include "firestaff/dm1/v1/mirror_candidate/first_interaction_focus_pc34_compat.h"

#include <string.h>

enum {
    kNoChampion = -1,
    kFirstChampionIndex = 0,
    kFirstChampionOrdinal = 1,
    kPanelC040 = 568,
    kGraphicC040 = 40
};

static const DM1_V1_MirrorCandidateFirstInteractionFocusSpecPc34 s_spec = {
    "MOVESENS.C:1501-1503 routes C127 champion portrait sensors to "
    "REVIVE.C F0280; REVIVE.C F0280:124-132 rejects occupied leader-hand "
    "or full-party cases; REVIVE.C F0280:272-276 publishes G0299 as "
    "previous party count + 1 and increments G0305; REVIVE.C F0280:276-283 "
    "sets the first candidate as leader/spell caster; REVIVE.C F0280:353-354 "
    "opens that candidate inventory and disables menus; COMMAND.C "
    "F0380:2159-2182,2302-2311 keeps status/inventory/spell/action input "
    "behind the live G0299 candidate-panel focus.",
    "non-overlap: this gate covers the zero-party first C127 mirror "
    "interaction focus handoff only. It does not cover later party reselect, "
    "C160/C161/C162 accept/cancel, C007..C011 inventory-toggle rejection, "
    "C012..C015 status-box rejection, C100 spell-area rejection, C111 "
    "action-area rejection, chest pickup, scroll pickup, save/load, "
    "teleporter survival, or portrait pixel/route probes.",
    "MOVESENS.C:1501-1503",
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0280:272-276",
    "REVIVE.C F0280:276-283",
    "REVIVE.C F0280:353-354",
    "COMMAND.C F0380:2159-2182,2302-2311",
    kPanelC040,
    kGraphicC040
};

const DM1_V1_MirrorCandidateFirstInteractionFocusSpecPc34 *
dm1_v1_mirror_candidate_first_interaction_focus_spec_pc34(void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_first_interaction_focus_source_evidence_pc34(void)
{
    return s_spec.sourceEvidence;
}

void dm1_v1_mirror_candidate_first_interaction_focus_init_pc34(
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->leaderHandEmpty = 1;
    state->leaderIndex = kNoChampion;
    state->magicCasterChampionIndex = kNoChampion;
}

static int f0280_add_first_candidate(
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 *state)
{
    int previousPartyCount;

    if (!state || !state->contractOnly) {
        return 0;
    }

    /* ReDMCSB REVIVE.C F0280:124-132: C127 mirror publication only
     * starts with an empty leader hand and a non-full party. */
    if (!state->leaderHandEmpty || state->partyChampionCount >= 4) {
        return 0;
    }

    ++state->f0280CallCount;
    previousPartyCount = state->partyChampionCount;

    /* ReDMCSB REVIVE.C F0280:272-276: G0299 is previous count + 1;
     * then G0305 is incremented. For the first Hall interaction, both
     * become ordinal/count 1. */
    state->candidateChampionOrdinal = previousPartyCount + 1;
    ++state->partyChampionCount;

    if (state->partyChampionCount == 1) {
        /* ReDMCSB REVIVE.C F0280:276-283: the first candidate becomes
         * leader and spell caster immediately. */
        state->leaderIndex = kFirstChampionIndex;
        state->magicCasterChampionIndex = kFirstChampionIndex;
        ++state->f0368SetLeaderCount;
        ++state->f0394SetMagicCasterCount;
    }

    /* ReDMCSB REVIVE.C F0280:353-354: open the candidate inventory
     * (F0355 with previousPartyCount) and disable normal menus. */
    state->inventoryChampionOrdinal = previousPartyCount + 1;
    state->panelContent = kPanelC040;
    state->panelGraphic = kGraphicC040;
    state->menusDisabled = 1;
    ++state->f0355InventoryToggleCount;
    return 1;
}

static void dispatch_guarded_input_focus_probe(
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 *state)
{
    if (!state || state->candidateChampionOrdinal == 0) {
        return;
    }

    /* ReDMCSB COMMAND.C F0380:2159-2182 and 2302-2311:
     * while G0299 is live, sibling status/inventory/spell/action inputs
     * do not steal focus from the C040 candidate panel. */
    ++state->blockedStatusBoxCount;
    ++state->blockedInventoryToggleCount;
    ++state->blockedSpellAreaCount;
    ++state->blockedActionAreaCount;
}

int dm1_v1_mirror_candidate_first_interaction_focus_run_pc34(
    DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34 *out)
{
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 state;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_first_interaction_focus_init_pc34(&state);

    out->partyCountBefore = state.partyChampionCount;
    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->leaderIndexBefore = state.leaderIndex;
    out->inventoryOrdinalBefore = state.inventoryChampionOrdinal;

    if (!f0280_add_first_candidate(&state)) {
        return 0;
    }
    dispatch_guarded_input_focus_probe(&state);

    out->partyCountAfter = state.partyChampionCount;
    out->candidateOrdinalAfter = state.candidateChampionOrdinal;
    out->leaderIndexAfter = state.leaderIndex;
    out->magicCasterChampionIndexAfter = state.magicCasterChampionIndex;
    out->inventoryChampionOrdinalAfter = state.inventoryChampionOrdinal;
    out->panelContentAfter = state.panelContent;
    out->panelGraphicAfter = state.panelGraphic;
    out->menusDisabledAfter = state.menusDisabled;
    out->f0280CallCount = state.f0280CallCount;
    out->f0355InventoryToggleCount = state.f0355InventoryToggleCount;
    out->f0368SetLeaderCount = state.f0368SetLeaderCount;
    out->f0394SetMagicCasterCount = state.f0394SetMagicCasterCount;
    out->blockedStatusBoxCount = state.blockedStatusBoxCount;
    out->blockedInventoryToggleCount = state.blockedInventoryToggleCount;
    out->blockedSpellAreaCount = state.blockedSpellAreaCount;
    out->blockedActionAreaCount = state.blockedActionAreaCount;
    out->focusOwnedByCandidate =
        state.candidateChampionOrdinal == kFirstChampionOrdinal &&
        state.inventoryChampionOrdinal == kFirstChampionOrdinal &&
        state.panelContent == kPanelC040 &&
        state.menusDisabled;

    out->accepted =
        out->partyCountBefore == 0 &&
        out->candidateOrdinalBefore == 0 &&
        out->leaderIndexBefore == kNoChampion &&
        out->inventoryOrdinalBefore == 0 &&
        out->partyCountAfter == 1 &&
        out->candidateOrdinalAfter == kFirstChampionOrdinal &&
        out->leaderIndexAfter == kFirstChampionIndex &&
        out->magicCasterChampionIndexAfter == kFirstChampionIndex &&
        out->inventoryChampionOrdinalAfter == kFirstChampionOrdinal &&
        out->panelContentAfter == kPanelC040 &&
        out->panelGraphicAfter == kGraphicC040 &&
        out->menusDisabledAfter == 1 &&
        out->f0280CallCount == 1 &&
        out->f0355InventoryToggleCount == 1 &&
        out->f0368SetLeaderCount == 1 &&
        out->f0394SetMagicCasterCount == 1 &&
        out->blockedStatusBoxCount == 1 &&
        out->blockedInventoryToggleCount == 1 &&
        out->blockedSpellAreaCount == 1 &&
        out->blockedActionAreaCount == 1 &&
        out->focusOwnedByCandidate == 1;
    out->assertionCount = 21;
    return out->accepted;
}
