#include "firestaff/dm1/v1/mirror_candidate/c040_spell_area_click_while_panel_live_pc34_compat.h"

#include <string.h>

enum {
    kPanelClosed = 0,
    kPanelC040 = 568,
    kGraphicC040 = 40,
    kSpellAreaClick = 100,
    kInitialPartyCount = 2,
    kInitialLeaderIndex = 0,
    kInitialMagicCasterIndex = 0,
    kInitialCandidateOrdinal = 3,
    kCancelCommand = 162,
    kLeaderHandEmpty = 0xffff,
    kNoMagicCaster = -1
};

/*
 * ReDMCSB source-lock map for this gate:
 * - COMMAND.C F0380:2303-2306 gates the C100 spell-area click command
 *   on `!G0299_ui_CandidateChampionOrdinal && G0514_i_MagicCasterChampionIndex
 *   != CM1_CHAMPION_NONE`. When the C040 candidate panel is live
 *   (G0299 != 0), the spell-area click is dropped and F0370 is not called.
 * - COMMAND.C F0370:2482-2520 is the only entry point that F0380 calls
 *   for C100. It owns the spell-menu redraw + cast-dispatch surface.
 * - REVIVE.C F0280:124-132 publishes G0299 when the candidate panel opens.
 * - REVIVE.C F0282:744-806 clears G0299 on C162 cancel (or accepts on
 *   C160/C161).
 * - DEFS.H: C100, C040/M568, G0299, G0305, G0411, G0514.
 *
 * This is contract-only runtime evidence. The pin is that C100 spell-area
 * clicks dispatched while G0299 is set are dropped (F0370 not called),
 * and only become live after F0282 clears G0299. Magic-caster state
 * (G0514) must be valid (non-NONE) for the post-clear click to fire.
 */
static const DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveSpecPc34
    s_spec = {
        "COMMAND.C F0380:2303-2306 (C100 spell-area click gate on "
        "!G0299 && G0514 != CM1_CHAMPION_NONE); "
        "COMMAND.C F0370:2482-2520 (C100 spell-area entry point); "
        "REVIVE.C F0280:124-132 (publishes G0299); "
        "REVIVE.C F0282:744-806 (clears G0299 on C162 cancel); "
        "DEFS.H C100, C040/M568, G0299, G0305, G0411, G0514",
        "non-overlap: this gate is C100 spell-area-click-while-c040-live; "
        "it does not cover C111 action-area, C007..C011 inventory-toggle, "
        "C012..C016 status-box, C140 save, the C160/C161 accept path, the "
        "C162 cancel+reopen same-tick path, or chest cancel-reopen-pickup. "
        "Disjoint from pass785 (inventory-toggle) and pass784 "
        "(cancel-then-reopen same-tick)",
        "COMMAND.C F0380:2303-2306",
        "COMMAND.C F0370:2482-2520",
        "REVIVE.C F0280:124-132",
        "REVIVE.C F0282:744-806",
        "DEFS.H C100, C040/M568, G0299, G0305, G0411, G0514",
        kPanelC040,
        kGraphicC040,
        kSpellAreaClick
    };

const DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_spec_pc34(
    void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_source_evidence_pc34(
    void)
{
    return s_spec.sourceEvidence;
}

void
dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveStatePc34 *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = kInitialPartyCount;
    state->leaderIndex = kInitialLeaderIndex;
    state->magicCasterChampionIndex = kInitialMagicCasterIndex;
    state->leaderHandThing = kLeaderHandEmpty;
    state->panelContent = kPanelC040;
    state->panelGraphic = kGraphicC040;
    state->candidateChampionOrdinal = kInitialCandidateOrdinal;
}

static void dispatch_f0370_spell_area_click(
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveStatePc34 *state)
{
    if (!state) {
        return;
    }
    /* F0380 gate:
     *   if (command == C100) && !G0299 && G0514 != CM1_CHAMPION_NONE
     *      -> F0370_COMMAND_ProcessType100_ClickInSpellArea_CPSE
     *   else
     *      -> dropped
     */
    if (state->candidateChampionOrdinal != 0) {
        state->rejectedWhileLiveCount += 1;
        return;
    }
    if (state->magicCasterChampionIndex == kNoMagicCaster) {
        state->rejectedNoCasterCount += 1;
        return;
    }
    state->f0370CallCount += 1;
}

int
dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveResultPc34 *out)
{
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveStatePc34 state;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_init_pc34(
        &state);

    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->panelContentBefore = state.panelContent;
    out->magicCasterBefore = state.magicCasterChampionIndex;
    out->partyCountBefore = state.partyChampionCount;

    /* Phase 1: candidate panel is live; queue 3 spell-area clicks. F0380's
     * gate must reject all of them.
     */
    for (i = 0; i < 3; ++i) {
        dispatch_f0370_spell_area_click(&state);
    }
    out->f0370CallsWhileLive = state.f0370CallCount;
    out->rejectedWhileLive = state.rejectedWhileLiveCount;
    out->panelContentAfterLiveToggles = state.panelContent;
    out->candidateOrdinalAfterLiveToggles = state.candidateChampionOrdinal;

    /* Phase 2: simulate F0282(C162) clearing G0299 so the panel chrome
     * drops and the spell-area click becomes live again.
     */
    state.command = kCancelCommand;
    if (state.command == kCancelCommand) {
        state.candidateChampionOrdinal = 0;
        state.panelContent = kPanelClosed;
        state.panelGraphic = 0;
        ++state.f0282DispatchCount;
    }

    /* Magic-caster still valid (G0514 != NONE): spell-area click should
     * fire F0370 exactly once.
     */
    dispatch_f0370_spell_area_click(&state);

    out->f0282Dispatched = state.f0282DispatchCount == 1 ? 1 : 0;
    out->f0370CallsAfterClear = state.f0370CallCount;
    out->panelContentAfterClear = state.panelContent;
    out->candidateOrdinalAfterClear = state.candidateChampionOrdinal;
    out->magicCasterAfterClear = state.magicCasterChampionIndex;

    /* Phase 3: drop the magic caster (G0514 = CM1_CHAMPION_NONE) and
     * confirm that even after F0282 cleared G0299, the C100 click is
     * dropped because G0514 is NONE.
     */
    state.magicCasterChampionIndex = kNoMagicCaster;
    dispatch_f0370_spell_area_click(&state);
    out->f0370CallsAfterCasterDrop = state.f0370CallCount;
    out->rejectedNoCaster = state.rejectedNoCasterCount;

    out->accepted =
        out->partyCountBefore == kInitialPartyCount &&
        out->magicCasterBefore == kInitialMagicCasterIndex &&
        out->panelContentBefore == kPanelC040 &&
        out->candidateOrdinalBefore == kInitialCandidateOrdinal &&
        out->f0370CallsWhileLive == 0 &&
        out->rejectedWhileLive == 3 &&
        out->panelContentAfterLiveToggles == kPanelC040 &&
        out->candidateOrdinalAfterLiveToggles == kInitialCandidateOrdinal &&
        out->f0282Dispatched &&
        out->f0370CallsAfterClear == 1 &&
        out->panelContentAfterClear == kPanelClosed &&
        out->candidateOrdinalAfterClear == 0 &&
        out->magicCasterAfterClear == kInitialMagicCasterIndex &&
        out->f0370CallsAfterCasterDrop == 1 &&
        out->rejectedNoCaster == 1;
    out->assertionCount = 16;
    return out->accepted;
}
