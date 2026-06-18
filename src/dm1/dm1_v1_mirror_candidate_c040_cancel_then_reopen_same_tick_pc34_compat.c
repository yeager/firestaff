#include "firestaff/dm1/v1/mirror_candidate/c040_cancel_then_reopen_same_tick_pc34_compat.h"

#include <string.h>

enum {
    kPanelClosed = 0,
    kPanelFoodWater = 565,
    kPanelC040 = 568,
    kPanelChest = 569,
    kGraphicC040 = 40,
    kLeaderHandEmpty = 0xffff,
    kThingNone = -1,
    kPartyCountTwo = 2,
    kPartyCountOne = 1,
    kCancelCommand = 162,
    kInitialCandidateOrdinal = 3,
    kReopenedCandidateOrdinal = 4,
    kInitialLeaderHand = 0x4c30,
    kF0333AnchorCell = 0x6420,
    kSensorThing = 0x5500
};

/*
 * ReDMCSB source-lock map for this gate:
 * - REVIVE.C F0280:124-132 is the empty-leader-hand C040 candidate gate that
 *   adds a champion mirror candidate to the party and sets the C040 panel.
 * - REVIVE.C F0282:744-806 is the C160/C161/C162 click-in-resurrect-panel
 *   resolver. The C162 cancel branch (744-783) toggles F0355
 *   (C04_CHAMPION_CLOSE_INVENTORY), clears G0299, possibly calls F0368 to
 *   set the leader to CM1_CHAMPION_NONE, decrements G0305 party count, and
 *   returns. It does NOT unlink champion slots or disable the sensor — that
 *   only happens in the C160/C161 path.
 * - PANEL.C F0355:2299-2318 closes the inventory chrome and calls F0334
 *   only when closing the inventory (which is C04_CHAMPION_CLOSE_INVENTORY
 *   routed through here).
 * - COMMAND.C F0378:1956-1990 is the only entry point that dispatches
 *   C160/C161/C162 clicks; the M568_PANEL_RESURRECT_REINCARNATE branch
 *   requires G0415_ui_LeaderEmptyHanded to be true and the leader hand
 *   to be empty.
 * - MOVESENS.C F0275:1502 is the cell-sensor scanner dispatch (C127 case);
 *   a fresh sensor trigger on the same map cell at the new party position
 *   in the same tick re-enters F0280, which sees G0415_ui_LeaderEmptyHanded=1
 *   and G0305_ui_PartyChampionCount<4 and re-opens the C040 panel.
 * - DEFS.H: C030, C040/M568, C127_SENSOR_WALL_CHAMPION_PORTRAIT, C162, G0299, G0305, G0415, G0424.
 *
 * This is contract-only runtime evidence. The pin is that a single tick
 * drives both F0282(C162) AND F0280(new sensor) in order, ending with a
 * fresh C040 panel on a fresh candidate ordinal; the previous candidate's
 * slots, stats, and chrome are gone.
 */
static const DM1_V1_MirrorCandidateC040CancelThenReopenSameTickSpecPc34
    s_spec = {
        "REVIVE.C F0280:124-132 (C040 empty-leader candidate gate); "
        "REVIVE.C F0282:744-806 (C160/C161/C162 panel resolver, C162 cancel "
        "branch 744-783); PANEL.C F0355:2299-2318 (inventory close); "
        "COMMAND.C F0378:1956-1990 (M568 panel click dispatch); "
        "MOVESENS.C F0275:1502 (C127 champion portrait sensor -> F0280); "
        "DEFS.H C040/M568, C127, C162, G0299, G0305, G0415, G0424",
        "non-overlap: this gate is C162 cancel followed by a new-sensor "
        "F0280 reopen in the same tick; it does not cover C160 resurrect, "
        "C161 reincarnate, C545 food/water, scroll, save/load, teleporter, "
        "chest cancel-reopen-pickup, or multi-tick close-reopen chains. "
        "Disjoint from pass760 mirror-candidate chrome-after-non-candidate "
        "and pass762 mirror-candidate rotate-in-progress-open and the "
        "chest_c040_cancel_reopen_pickup gate (M569 chest, not M568 mirror). "
        "C162 cancel followed by a new-sensor F0280 reopen in the same tick.",
        "REVIVE.C F0280:124-132",
        "REVIVE.C F0282:744-806",
        "PANEL.C F0355:2299-2318",
        "COMMAND.C F0378:1956-1990",
        "MOVESENS.C F0275:1502",
        "DEFS.H C040/M568, C127, C162, G0299, G0305, G0415, G0424",
        kPanelC040,
        kGraphicC040,
        kPartyCountTwo,
        kPartyCountOne,
        kInitialCandidateOrdinal,
        kReopenedCandidateOrdinal
    };

const DM1_V1_MirrorCandidateC040CancelThenReopenSameTickSpecPc34 *
dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_spec_pc34(void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_source_evidence_pc34(
    void)
{
    return s_spec.sourceEvidence;
}

void
dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_init_pc34(
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34 *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->leaderHandThing = kLeaderHandEmpty;
    state->panelContent = kPanelC040;
    state->panelGraphic = kGraphicC040;
    state->partyChampionCount = kPartyCountTwo;
    state->candidateChampionOrdinal = kInitialCandidateOrdinal;
    state->partyMapX = 10;
    state->partyMapY = 10;
    state->partyDirection = 0;
    state->sensorCellThing = kSensorThing;
    state->mirrorCellThing = kF0333AnchorCell;
}

static void dispatch_f0282_cancel(
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34 *state)
{
    if (!state || state->command != kCancelCommand) {
        return;
    }
    /* F0282 C162 cancel branch:
     *   F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)
     *   G0299 = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)
     *   if (G0305 == 1) F0368_COMMAND_SetLeader(CM1_CHAMPION_NONE)
     *   G0305--
     *   F0077_MOUSE_EnableScreenUpdate
     *   F0457_START_DrawEnabledMenus
     *   F0078_MOUSE_DisableScreenUpdate
     *   return
     * Slots and sensors are NOT mutated in the C162 branch.
     */
    ++state->f0282DispatchCount;
    ++state->f0355CallCount;
    state->candidateChampionOrdinal = 0;
    state->panelContent = kPanelClosed;
    state->panelGraphic = 0;
    if (state->partyChampionCount == kPartyCountOne) {
        state->leaderHandThing = kLeaderHandEmpty;
    }
    state->partyChampionCount -= 1;
    if (state->partyChampionCount < 0) {
        state->partyChampionCount = 0;
    }
}

static void dispatch_f0280_new_sensor(
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34 *state)
{
    /* F0280 CHAMPION_AddCandidateChampionToParty:
     *   if (!G0415_ui_LeaderEmptyHanded) return
     *   if (G0305_ui_PartyChampionCount >= 4) return
     *   ...promotes the candidate and sets the C040 panel.
     */
    if (!state) {
        return;
    }
    if (state->leaderHandThing != kLeaderHandEmpty) {
        state->f0280Rejected = 1;
        return;
    }
    if (state->partyChampionCount >= 4) {
        state->f0280Rejected = 1;
        return;
    }
    state->partyChampionCount += 1;
    state->candidateChampionOrdinal = kReopenedCandidateOrdinal;
    state->panelContent = kPanelC040;
    state->panelGraphic = kGraphicC040;
    ++state->f0280DispatchCount;
}

int dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_run_pc34(
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickResultPc34 *out)
{
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34 state;
    int panelContentMid;
    int partyCountMid;
    int candidateMid;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_init_pc34(
        &state);

    out->partyCountBefore = state.partyChampionCount;
    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->panelContentBefore = state.panelContent;
    out->panelGraphicBefore = state.panelGraphic;
    out->leaderHandEmptyBefore =
        state.leaderHandThing == kLeaderHandEmpty ? 1 : 0;

    state.command = kCancelCommand;
    dispatch_f0282_cancel(&state);
    panelContentMid = state.panelContent;
    partyCountMid = state.partyChampionCount;
    candidateMid = state.candidateChampionOrdinal;

    /* The same tick's queued movement puts the party onto a new champion
     * mirror square. SENSOR.C F0212 fires the new sensor, which calls F0280.
     */
    state.partyMapX += 1;
    dispatch_f0280_new_sensor(&state);

    out->partyCountMid = partyCountMid;
    out->candidateOrdinalMid = candidateMid;
    out->panelContentMid = panelContentMid;
    out->f0282Dispatched = state.f0282DispatchCount == 1 ? 1 : 0;
    out->f0355Called = state.f0355CallCount == 1 ? 1 : 0;
    out->f0280Dispatched = state.f0280DispatchCount == 1 ? 1 : 0;
    out->f0280NotRejected = state.f0280Rejected == 0 ? 1 : 0;
    out->partyCountAfter = state.partyChampionCount;
    out->candidateOrdinalAfter = state.candidateChampionOrdinal;
    out->panelContentAfter = state.panelContent;
    out->panelGraphicAfter = state.panelGraphic;
    out->leaderHandEmptyAfter =
        state.leaderHandThing == kLeaderHandEmpty ? 1 : 0;
    out->partyMovedToFreshSensor =
        state.partyMapX == 11 && state.partyMapY == 10 ? 1 : 0;
    out->sameTick = state.f0282DispatchCount == 1 &&
        state.f0280DispatchCount == 1 ? 1 : 0;

    out->accepted =
        out->partyCountBefore == kPartyCountTwo &&
        out->panelContentBefore == kPanelC040 &&
        out->panelGraphicBefore == kGraphicC040 &&
        out->candidateOrdinalBefore == kInitialCandidateOrdinal &&
        out->leaderHandEmptyBefore == 1 &&
        out->f0282Dispatched &&
        out->f0355Called &&
        out->panelContentMid == kPanelClosed &&
        out->candidateOrdinalMid == 0 &&
        out->partyCountMid == kPartyCountOne &&
        out->f0280Dispatched &&
        out->f0280NotRejected &&
        out->panelContentAfter == kPanelC040 &&
        out->panelGraphicAfter == kGraphicC040 &&
        out->candidateOrdinalAfter == kReopenedCandidateOrdinal &&
        out->partyCountAfter == kPartyCountTwo &&
        out->leaderHandEmptyAfter == 1 &&
        out->partyMovedToFreshSensor &&
        out->sameTick;
    out->assertionCount = 18;
    return out->accepted;
}
