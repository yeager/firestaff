#include "firestaff/dm1/v1/mirror_candidate/c040_action_area_click_while_panel_live_pc34_compat.h"

#include <string.h>

enum {
    kPanelClosed = 0,
    kPanelC040 = 568,
    kGraphicC040 = 40,
    kActionAreaClick = 111,
    kInitialPartyCount = 2,
    kInitialLeaderIndex = 0,
    kInitialCandidateOrdinal = 3,
    kCancelCommand = 162,
    kLeaderHandEmpty = 0xffff
};

/*
 * ReDMCSB source-lock map for this gate:
 * - COMMAND.C F0380:2309-2311 gates the C111 action-area click command
 *   on `!G0299_ui_CandidateChampionOrdinal`. When the C040 candidate
 *   panel is live (G0299 != 0), the action-area click is dropped and
 *   F0371 is not called.
 * - COMMAND.C F0371 is the only entry point that F0380 calls for C111.
 *   It owns the action-menu redraw + cast-dispatch surface (use-item,
 *   drop, throw, etc).
 * - REVIVE.C F0280:124-132 publishes G0299 when the candidate panel opens.
 * - REVIVE.C F0282:744-806 clears G0299 on C162 cancel (or accepts on
 *   C160/C161).
 * - DEFS.H: C111, C040/M568, G0299, G0305, G0411.
 *
 * This is contract-only runtime evidence. The pin is that C111
 * action-area clicks dispatched while G0299 is set are dropped (F0371
 * not called), and only become live after F0282 clears G0299.
 *
 * Disjoint from pass785 (inventory-toggle covers C007..C011), pass786
 * (spell-area covers C100 with G0514 magic-caster gate), and pass784
 * (cancel-then-reopen same-tick).
 */
static const DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveSpecPc34
    s_spec = {
        "COMMAND.C F0380:2309-2311 (C111 action-area click gate on "
        "!G0299); "
        "COMMAND.C F0371 (C111 action-area entry point); "
        "REVIVE.C F0280:124-132 (publishes G0299); "
        "REVIVE.C F0282:744-806 (clears G0299 on C162 cancel); "
        "DEFS.H C111, C040/M568, G0299, G0305, G0411",
        "non-overlap: this gate is C111 action-area-click-while-c040-live; "
        "it does not cover C007..C011 inventory-toggle, C100 spell-area, "
        "C012..C016 status-box, C140 save, the C160/C161 accept path, the "
        "C162 cancel+reopen same-tick path, or chest cancel-reopen-pickup. "
        "Disjoint from pass786 (which covers C100 with the G0514 magic-"
        "caster gate) and pass785 (which covers C007..C011 without the "
        "G0514 magic-caster gate) and pass784 (cancel-then-reopen)",
        "COMMAND.C F0380:2309-2311",
        "COMMAND.C F0371",
        "REVIVE.C F0280:124-132",
        "REVIVE.C F0282:744-806",
        "DEFS.H C111, C040/M568, G0299, G0305, G0411",
        kPanelC040,
        kGraphicC040,
        kActionAreaClick
    };

const DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_spec_pc34(
    void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_source_evidence_pc34(
    void)
{
    return s_spec.sourceEvidence;
}

void
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveStatePc34 *state)
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

static void dispatch_f0371_action_area_click(
    DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveStatePc34 *state)
{
    if (!state) {
        return;
    }
    /* F0380 gate:
     *   if (command == C111) && !G0299
     *      -> F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE
     *   else
     *      -> dropped
     */
    if (state->candidateChampionOrdinal != 0) {
        state->rejectedWhileLiveCount += 1;
        return;
    }
    state->f0371CallCount += 1;
}

int
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveResultPc34 *out)
{
    DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveStatePc34 state;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_init_pc34(
        &state);

    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->panelContentBefore = state.panelContent;
    out->leaderHandEmptyBefore =
        state.leaderHandThing == kLeaderHandEmpty ? 1 : 0;
    out->partyCountBefore = state.partyChampionCount;

    /* Phase 1: candidate panel is live; queue 3 action-area clicks.
     * F0380's gate must reject all of them.
     */
    for (i = 0; i < 3; ++i) {
        dispatch_f0371_action_area_click(&state);
    }
    out->f0371CallsWhileLive = state.f0371CallCount;
    out->rejectedWhileLive = state.rejectedWhileLiveCount;
    out->panelContentAfterLiveToggles = state.panelContent;
    out->candidateOrdinalAfterLiveToggles = state.candidateChampionOrdinal;

    /* Phase 2: simulate F0282(C162) clearing G0299 so the panel chrome
     * drops and the action-area click becomes live again.
     */
    state.command = kCancelCommand;
    if (state.command == kCancelCommand) {
        state.candidateChampionOrdinal = 0;
        state.panelContent = kPanelClosed;
        state.panelGraphic = 0;
        ++state.f0282DispatchCount;
    }
    dispatch_f0371_action_area_click(&state);

    out->f0282Dispatched = state.f0282DispatchCount == 1 ? 1 : 0;
    out->f0371CallsAfterClear = state.f0371CallCount;
    out->panelContentAfterClear = state.panelContent;
    out->candidateOrdinalAfterClear = state.candidateChampionOrdinal;
    out->rejectedAfterClear = state.rejectedWhileLiveCount;

    out->accepted =
        out->partyCountBefore == kInitialPartyCount &&
        out->leaderHandEmptyBefore == 1 &&
        out->panelContentBefore == kPanelC040 &&
        out->candidateOrdinalBefore == kInitialCandidateOrdinal &&
        out->f0371CallsWhileLive == 0 &&
        out->rejectedWhileLive == 3 &&
        out->panelContentAfterLiveToggles == kPanelC040 &&
        out->candidateOrdinalAfterLiveToggles == kInitialCandidateOrdinal &&
        out->f0282Dispatched &&
        out->f0371CallsAfterClear == 1 &&
        out->rejectedAfterClear == 3 &&
        out->panelContentAfterClear == kPanelClosed &&
        out->candidateOrdinalAfterClear == 0;
    out->assertionCount = 14;
    return out->accepted;
}
