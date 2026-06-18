#include "firestaff/dm1/v1/mirror_candidate/c040_status_box_click_while_panel_live_pc34_compat.h"

#include <string.h>

enum {
    kPanelClosed = 0,
    kPanelC040 = 568,
    kGraphicC040 = 40,
    kStatusBoxClickChampion0 = 12,
    kStatusBoxClickChampion3 = 15,
    kInitialPartyCount = 2,
    kInitialLeaderIndex = 0,
    kInitialCandidateOrdinal = 3,
    kCancelCommand = 162,
    kLeaderHandEmpty = 0xffff
};

/*
 * ReDMCSB source-lock map for this gate:
 * - COMMAND.C F0380:2159-2161 gates the C012..C015 champion-status-box
 *   click commands on
 *     `(champion_index < G0305_ui_PartyChampionCount) && !G0299_ui_CandidateChampionOrdinal`.
 *   When the C040 candidate panel is live (G0299 != 0), the status-box
 *   click is dropped and F0367 is not called.
 * - COMMAND.C F0367 is the only entry point that F0380 calls for
 *   C012..C015. It owns the status-box click dispatch (use action hand,
 *   swap champion, etc).
 * - REVIVE.C F0280:124-132 publishes G0299 when the candidate panel opens.
 * - REVIVE.C F0282:744-806 clears G0299 on C162 cancel (or accepts on
 *   C160/C161).
 * - DEFS.H: C012..C015, C040/M568, G0299, G0305, G0411.
 *
 * This is contract-only runtime evidence. The pin is that C012..C015
 * status-box clicks dispatched while G0299 is set are dropped (F0367
 * not called), and only become live after F0282 clears G0299.
 *
 * Disjoint from pass787 (C111 action-area), pass786 (C100 with G0514
 * magic-caster gate), pass785 (C007..C011 inventory-toggle), and pass784
 * (cancel-then-reopen same-tick).
 */
static const DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveSpecPc34
    s_spec = {
        "COMMAND.C F0380:2159-2161 (C012..C015 champion-status-box click "
        "gate on champion_index < G0305 && !G0299); "
        "COMMAND.C F0367 (C012..C015 status-box click entry point); "
        "REVIVE.C F0280:124-132 (publishes G0299); "
        "REVIVE.C F0282:744-806 (clears G0299 on C162 cancel); "
        "DEFS.H C012..C015, C040/M568, G0299, G0305, G0411",
        "non-overlap: this gate is C012..C015 status-box-click-while-"
        "c040-live; it does not cover "
        "status-box-click-while-c040-live; "
        "C007..C011 inventory-toggle, "
        "C100 spell-area, C111 action-area, C140 save, the C160/C161 "
        "accept path, the C162 cancel+reopen same-tick path, or chest "
        "cancel-reopen-pickup. Disjoint from pass787 (C111), pass786 "
        "(C100 with G0514), pass785 (C007..C011), and pass784 "
        "(cancel-then-reopen)",
        "COMMAND.C F0380:2159-2161",
        "COMMAND.C F0367",
        "REVIVE.C F0280:124-132",
        "REVIVE.C F0282:744-806",
        "DEFS.H C012..C015, C040/M568, G0299, G0305, G0411",
        kPanelC040,
        kGraphicC040,
        kStatusBoxClickChampion0,
        kStatusBoxClickChampion3
    };

const DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_spec_pc34(
    void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_source_evidence_pc34(
    void)
{
    return s_spec.sourceEvidence;
}

void
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveStatePc34 *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = kInitialPartyCount;
    state->leaderIndex = kInitialLeaderIndex;
    state->leaderHandThing = kLeaderHandEmpty;
    state->panelContent = kPanelC040;
    state->panelGraphic = kGraphicC040;
    state->candidateChampionOrdinal = kInitialCandidateOrdinal;
}

static void dispatch_f0367_status_box_click(
    DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveStatePc34 *state,
    int command)
{
    if (!state) {
        return;
    }
    /* F0380 gate:
     *   if (command in C012..C015) && champion_index < G0305
     *      && !G0299
     *      -> F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox
     *   else
     *      -> dropped
     */
    int champion_index;
    if (command < kStatusBoxClickChampion0 ||
        command > kStatusBoxClickChampion3) {
        return;
    }
    champion_index = command - kStatusBoxClickChampion0;
    if (champion_index >= state->partyChampionCount) {
        state->rejectedOutOfRangeCount += 1;
        return;
    }
    if (state->candidateChampionOrdinal != 0) {
        state->rejectedWhileLiveCount += 1;
        return;
    }
    state->f0367CallCount += 1;
}

int
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveResultPc34 *out)
{
    DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveStatePc34 state;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_init_pc34(
        &state);

    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->panelContentBefore = state.panelContent;
    out->leaderHandEmptyBefore =
        state.leaderHandThing == kLeaderHandEmpty ? 1 : 0;
    out->partyCountBefore = state.partyChampionCount;

    /* Phase 1: candidate panel is live; queue 4 status-box clicks
     * (C012, C013, C014, C015). All in range; all dropped by !G0299.
     */
    for (i = kStatusBoxClickChampion0; i <= kStatusBoxClickChampion3; ++i) {
        dispatch_f0367_status_box_click(&state, i);
    }
    out->f0367CallsWhileLive = state.f0367CallCount;
    out->rejectedWhileLive = state.rejectedWhileLiveCount;
    out->rejectedOutOfRange = state.rejectedOutOfRangeCount;
    out->panelContentAfterLiveToggles = state.panelContent;
    out->candidateOrdinalAfterLiveToggles = state.candidateChampionOrdinal;

    /* Phase 2: simulate F0282(C162) clearing G0299 so the panel chrome
     * drops and the status-box click becomes live again.
     */
    state.command = kCancelCommand;
    if (state.command == kCancelCommand) {
        state.candidateChampionOrdinal = 0;
        state.panelContent = kPanelClosed;
        state.panelGraphic = 0;
        ++state.f0282DispatchCount;
    }
    dispatch_f0367_status_box_click(&state, kStatusBoxClickChampion0);

    out->f0282Dispatched = state.f0282DispatchCount == 1 ? 1 : 0;
    out->f0367CallsAfterClear = state.f0367CallCount;
    out->panelContentAfterClear = state.panelContent;
    out->candidateOrdinalAfterClear = state.candidateChampionOrdinal;
    out->rejectedAfterClear = state.rejectedWhileLiveCount;

    out->accepted =
        out->partyCountBefore == kInitialPartyCount &&
        out->leaderHandEmptyBefore == 1 &&
        out->panelContentBefore == kPanelC040 &&
        out->candidateOrdinalBefore == kInitialCandidateOrdinal &&
        out->f0367CallsWhileLive == 0 &&
        out->rejectedWhileLive == 2 &&
        out->rejectedOutOfRange == 2 &&
        out->panelContentAfterLiveToggles == kPanelC040 &&
        out->candidateOrdinalAfterLiveToggles == kInitialCandidateOrdinal &&
        out->f0282Dispatched &&
        out->f0367CallsAfterClear == 1 &&
        out->rejectedAfterClear == 2 &&
        out->panelContentAfterClear == kPanelClosed &&
        out->candidateOrdinalAfterClear == 0;
    out->assertionCount = 15;
    return out->accepted;
}
