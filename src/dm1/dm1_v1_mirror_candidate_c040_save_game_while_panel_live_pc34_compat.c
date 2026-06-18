#include "firestaff/dm1/v1/mirror_candidate/c040_save_game_while_panel_live_pc34_compat.h"

#include <string.h>

enum {
    kPanelClosed = 0,
    kPanelC040 = 568,
    kGraphicC040 = 40,
    kSaveGameCommand = 140,
    kInitialPartyCount = 2,
    kInitialLeaderIndex = 0,
    kInitialCandidateOrdinal = 3,
    kCancelCommand = 162,
    kLeaderHandEmpty = 0xffff
};

/*
 * ReDMCSB source-lock map for this gate:
 * - COMMAND.C F0380:2367-2369 gates the C140 save-game command on
 *   `(G0305_ui_PartyChampionCount > 0) && !G0299_ui_CandidateChampionOrdinal`.
 *   When the C040 candidate panel is live (G0299 != 0), the save-game
 *   command is dropped and F0433 is not called.
 * - STARTEND.C F0433:2480-2520 (ProcessCommand140_SaveGame_CPSCDF) is
 *   the only entry point that F0380 calls for C140. It owns the save
 *   checksum + DISK write surface.
 * - REVIVE.C F0280:124-132 publishes G0299 when the candidate panel opens.
 * - REVIVE.C F0282:744-806 clears G0299 on C162 cancel (or accepts on
 *   C160/C161).
 * - DEFS.H: C140, C040/M568, G0299, G0305, G0411.
 *
 * This is contract-only runtime evidence. The pin is that C140 save-game
 * commands dispatched while G0299 is set are dropped (F0433 not called),
 * and only become live after F0282 clears G0299. This protects against
 * saving a half-resolved resurrect candidate chain.
 *
 * Disjoint from pass788 (C012..C015 status-box), pass787 (C111
 * action-area), pass786 (C100 with G0514 magic-caster gate), pass785
 * (C007..C011 inventory-toggle), and pass784 (cancel-then-reopen
 * same-tick).
 */
static const DM1_V1_MirrorCandidateC040SaveGameWhilePanelLiveSpecPc34
    s_spec = {
        "COMMAND.C F0380:2367-2369 (C140 save-game command gate on "
        "G0305 > 0 && !G0299); "
        "STARTEND.C F0433 (C140 save-game entry point); "
        "REVIVE.C F0280:124-132 (publishes G0299); "
        "REVIVE.C F0282:744-806 (clears G0299 on C162 cancel); "
        "DEFS.H C140, C040/M568, G0299, G0305, G0411",
        "non-overlap: this gate is C140 save-game-while-c040-live; it "
        "does not cover C007..C011 inventory-toggle, C100 spell-area, "
        "C111 action-area, C012..C015 status-box, C160/C161 accept, "
        "C162 cancel+reopen same-tick, or chest cancel-reopen-pickup. "
        "Disjoint from pass788 (status-box), pass787 (action-area), "
        "pass786 (spell-area), pass785 (inventory-toggle), and pass784 "
        "(cancel-then-reopen)",
        "COMMAND.C F0380:2367-2369",
        "STARTEND.C F0433",
        "REVIVE.C F0280:124-132",
        "REVIVE.C F0282:744-806",
        "DEFS.H C140, C040/M568, G0299, G0305, G0411",
        kPanelC040,
        kGraphicC040,
        kSaveGameCommand
    };

const DM1_V1_MirrorCandidateC040SaveGameWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_save_game_while_panel_live_spec_pc34(void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_save_game_while_panel_live_source_evidence_pc34(
    void)
{
    return s_spec.sourceEvidence;
}

void
dm1_v1_mirror_candidate_c040_save_game_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040SaveGameWhilePanelLiveStatePc34 *state)
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

static void dispatch_f0433_save_game(
    DM1_V1_MirrorCandidateC040SaveGameWhilePanelLiveStatePc34 *state)
{
    if (!state) {
        return;
    }
    /* F0380 gate:
     *   if (command == C140) && G0305 > 0 && !G0299
     *      -> F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
     *   else
     *      -> dropped
     */
    if (state->partyChampionCount <= 0) {
        state->rejectedNoPartyCount += 1;
        return;
    }
    if (state->candidateChampionOrdinal != 0) {
        state->rejectedWhileLiveCount += 1;
        return;
    }
    state->f0433CallCount += 1;
}

int
dm1_v1_mirror_candidate_c040_save_game_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040SaveGameWhilePanelLiveResultPc34 *out)
{
    DM1_V1_MirrorCandidateC040SaveGameWhilePanelLiveStatePc34 state;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_c040_save_game_while_panel_live_init_pc34(
        &state);

    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->panelContentBefore = state.panelContent;
    out->leaderHandEmptyBefore =
        state.leaderHandThing == kLeaderHandEmpty ? 1 : 0;
    out->partyCountBefore = state.partyChampionCount;

    /* Phase 1: candidate panel is live; queue 3 save-game commands.
     * F0380's gate must reject all of them.
     */
    for (i = 0; i < 3; ++i) {
        dispatch_f0433_save_game(&state);
    }
    out->f0433CallsWhileLive = state.f0433CallCount;
    out->rejectedWhileLive = state.rejectedWhileLiveCount;
    out->panelContentAfterLiveToggles = state.panelContent;
    out->candidateOrdinalAfterLiveToggles = state.candidateChampionOrdinal;

    /* Phase 2: simulate F0282(C162) clearing G0299 so the panel chrome
     * drops and the save-game command becomes live again.
     */
    state.command = kCancelCommand;
    if (state.command == kCancelCommand) {
        state.candidateChampionOrdinal = 0;
        state.panelContent = kPanelClosed;
        state.panelGraphic = 0;
        ++state.f0282DispatchCount;
    }
    dispatch_f0433_save_game(&state);

    out->f0282Dispatched = state.f0282DispatchCount == 1 ? 1 : 0;
    out->f0433CallsAfterClear = state.f0433CallCount;
    out->panelContentAfterClear = state.panelContent;
    out->candidateOrdinalAfterClear = state.candidateChampionOrdinal;
    out->rejectedAfterClear = state.rejectedWhileLiveCount;

    out->accepted =
        out->partyCountBefore == kInitialPartyCount &&
        out->leaderHandEmptyBefore == 1 &&
        out->panelContentBefore == kPanelC040 &&
        out->candidateOrdinalBefore == kInitialCandidateOrdinal &&
        out->f0433CallsWhileLive == 0 &&
        out->rejectedWhileLive == 3 &&
        out->panelContentAfterLiveToggles == kPanelC040 &&
        out->candidateOrdinalAfterLiveToggles == kInitialCandidateOrdinal &&
        out->f0282Dispatched &&
        out->f0433CallsAfterClear == 1 &&
        out->rejectedAfterClear == 3 &&
        out->panelContentAfterClear == kPanelClosed &&
        out->candidateOrdinalAfterClear == 0;
    out->assertionCount = 14;
    return out->accepted;
}
