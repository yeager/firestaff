#include "firestaff/dm1/v1/mirror_candidate/c040_inventory_toggle_while_panel_live_pc34_compat.h"

#include <string.h>

enum {
    kPanelClosed = 0,
    kPanelC040 = 568,
    kGraphicC040 = 40,
    kInventoryToggleChampion0 = 7,
    kInventoryToggleChampion1 = 8,
    kInventoryToggleChampion2 = 9,
    kInventoryToggleChampion3 = 10,
    kCloseInventory = 11,
    kInitialPartyCount = 2,
    kInitialLeaderIndex = 0,
    kInitialCandidateOrdinal = 3,
    kCancelCommand = 162,
    kLeaderHandEmpty = 0xffff,
    kOpenChestThing = 0x6420
};

/*
 * ReDMCSB source-lock map for this gate:
 * - COMMAND.C F0380:2181-2183 gates the C007..C011 inventory-toggle
 *   commands on `!G0299_ui_CandidateChampionOrdinal`. When the C040
 *   candidate panel is live (G0299 != 0), the inventory toggle is dropped
 *   and the candidate panel chrome remains untouched.
 * - PANEL.C F0355:2299-2318 is the only entry point that F0380 calls for
 *   inventory toggle. It owns the F0334 close-chest call when closing
 *   the inventory.
 * - REVIVE.C F0280:124-132 publishes G0299 when the candidate panel opens.
 * - REVIVE.C F0282:744-806 clears G0299 on C162 cancel (or accepts on
 *   C160/C161).
 * - DEFS.H: C007..C011, C040/M568, G0299, G0305, G0411.
 *
 * This is contract-only runtime evidence. The pin is that inventory
 * toggle commands C007..C010 dispatched while G0299 is set are dropped
 * (F0355 not called), and only become live after F0282 clears G0299.
 */
static const DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveSpecPc34
    s_spec = {
        "COMMAND.C F0380:2181-2183 (C007..C011 inventory-toggle gate on "
        "!G0299); PANEL.C F0355:2299-2318 (inventory toggle entry point); "
        "REVIVE.C F0280:124-132 (publishes G0299); "
        "REVIVE.C F0282:744-806 (clears G0299 on C162 cancel); "
        "DEFS.H C007..C011, C040/M568, G0299, G0305, G0411",
        "non-overlap: this gate is inventory-toggle-while-c040-live; it "
        "does not cover C012..C016 status-box gating, C022 spell menu, "
        "C027 action menu, C140 save gating, the C160/C161 accept path, "
        "the C162 cancel+reopen same-tick path, or chest cancel-reopen-pickup. "
        "Disjoint from pass784 mirror-candidate cancel-then-reopen-same-tick "
        "(pass784 covers F0282(C162) + new-sensor F0280 in the same tick; "
        "this gate covers F0380:2181-2183 inventory-toggle rejection while "
        "the panel is live).",
        "COMMAND.C F0380:2181-2183",
        "PANEL.C F0355:2299-2318",
        "REVIVE.C F0280:124-132",
        "REVIVE.C F0282:744-806",
        "DEFS.H C007..C011, C040/M568, G0299, G0305, G0411",
        kPanelC040,
        kGraphicC040,
        kInventoryToggleChampion0,
        kInventoryToggleChampion3,
        kCloseInventory
    };

const DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_spec_pc34(
    void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_source_evidence_pc34(
    void)
{
    return s_spec.sourceEvidence;
}

void
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveStatePc34 *state)
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
    state->openChestThing = kOpenChestThing;
}

static void dispatch_f0355_inventory_toggle(
    DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveStatePc34 *state,
    int command)
{
    if (!state) {
        return;
    }
    /* F0380 gate:
     *   if (command in C007..C011) && champion_in_range_or_close
     *      && !G0299_ui_CandidateChampionOrdinal
     *      -> F0355_INVENTORY_Toggle_CPSE(...)
     *   else
     *      -> dropped
     */
    if (command < kInventoryToggleChampion0 ||
        command > kCloseInventory) {
        return;
    }
    if (state->candidateChampionOrdinal != 0) {
        state->rejectedWhileLiveCount += 1;
        return;
    }
    state->f0355CallCount += 1;
    if (command == kCloseInventory) {
        state->panelContent = kPanelClosed;
    }
}

int
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveResultPc34 *out)
{
    DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveStatePc34 state;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_init_pc34(
        &state);

    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->panelContentBefore = state.panelContent;
    out->leaderHandEmptyBefore =
        state.leaderHandThing == kLeaderHandEmpty ? 1 : 0;
    out->partyCountBefore = state.partyChampionCount;

    /* Phase 1: candidate panel is live; queue C007..C010 toggles + C011
     * close. F0380's gate must reject all of them.
     */
    for (i = kInventoryToggleChampion0; i <= kInventoryToggleChampion3; ++i) {
        dispatch_f0355_inventory_toggle(&state, i);
    }
    dispatch_f0355_inventory_toggle(&state, kCloseInventory);

    out->f0355CallsWhileLive = state.f0355CallCount;
    out->rejectedWhileLive = state.rejectedWhileLiveCount;
    out->panelContentAfterLiveToggles = state.panelContent;
    out->candidateOrdinalAfterLiveToggles = state.candidateChampionOrdinal;

    /* Phase 2: simulate F0282(C162) clearing G0299 so the panel chrome
     * drops and the inventory toggle becomes live again.
     */
    state.command = kCancelCommand;
    if (state.command == kCancelCommand) {
        state.candidateChampionOrdinal = 0;
        state.panelContent = kPanelClosed;
        state.panelGraphic = 0;
        ++state.f0282DispatchCount;
    }

    /* Now C007 (champion 0) should fire F0355. */
    dispatch_f0355_inventory_toggle(&state, kInventoryToggleChampion0);

    out->f0282Dispatched = state.f0282DispatchCount == 1 ? 1 : 0;
    out->f0355CallsAfterClear = state.f0355CallCount;
    out->panelContentAfterClear = state.panelContent;
    out->candidateOrdinalAfterClear = state.candidateChampionOrdinal;
    out->rejectedAfterClear = state.rejectedWhileLiveCount;

    out->accepted =
        out->partyCountBefore == kInitialPartyCount &&
        out->leaderHandEmptyBefore == 1 &&
        out->panelContentBefore == kPanelC040 &&
        out->candidateOrdinalBefore == kInitialCandidateOrdinal &&
        out->f0355CallsWhileLive == 0 &&
        out->rejectedWhileLive == 5 &&
        out->panelContentAfterLiveToggles == kPanelC040 &&
        out->candidateOrdinalAfterLiveToggles == kInitialCandidateOrdinal &&
        out->f0282Dispatched &&
        out->f0355CallsAfterClear == 1 &&
        out->rejectedAfterClear == 5 &&
        out->panelContentAfterClear == kPanelClosed &&
        out->candidateOrdinalAfterClear == 0;
    out->assertionCount = 14;
    return out->accepted;
}
