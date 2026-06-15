#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_c040_eye_live_candidate_pc34_compat.h"

#include <string.h>

enum {
    kThingNone = -1,
    kPanelChest = 569,
    kPanelC040 = 568,
    kGraphicC040 = 40,
    kZoneC546Eye = 546,
    kIconEyeNotLooking = 202,
    kIconEyeLooking = 203,
    kLeaderHandObject = 0x4d21,
    kOpenChestThing = 0x6b38
};

/* ReDMCSB anchors for this contract-only runtime gate:
 * REVIVE.C F0280:124-132 leaves G0299_ui_CandidateChampionOrdinal live.
 * PANEL.C F0352:2123-2159 presses C546 and temporarily draws the eye panel.
 * PANEL.C F0353:2174-2192 releases C546, calls F0347, and redraws viewport.
 * PANEL.C F0347:1651-1656 closes any chest, then gives live C040 priority.
 * CHEST.C F0334:112-132 clears G0426 and the visible G0425 chest slots.
 * COMMAND.C F0359:1985-1990 is the only C040 confirm/cancel dispatch path.
 */
static const DM1_V1_MirrorCandidateC040EyeLiveCandidateSpecPc34 s_spec = {
    "REVIVE.C F0280:124-132; PANEL.C F0352:2123-2159; "
    "PANEL.C F0353:2174-2192; PANEL.C F0347:1651-1656; "
    "CHEST.C F0334:112-132; COMMAND.C F0359:1985-1990",
    "non-overlap: this gate is C546 eye press/release over a live C040 "
    "candidate with a chest still open; it does not cover C545 food/water, "
    "scroll wheel, keyboard browse, direct C040 confirm/cancel, save/load, "
    "teleporter survival, or ordinary chest close/click mutation",
    "REVIVE.C F0280:124-132",
    "PANEL.C F0352:2123-2159",
    "PANEL.C F0353:2174-2192",
    "PANEL.C F0347:1651-1656",
    "CHEST.C F0334:112-132",
    "COMMAND.C F0359:1985-1990",
    kPanelC040,
    kGraphicC040,
    kZoneC546Eye,
    kIconEyeLooking,
    kIconEyeNotLooking
};

const DM1_V1_MirrorCandidateC040EyeLiveCandidateSpecPc34 *
dm1_v1_mirror_candidate_c040_eye_live_candidate_spec_pc34(void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_c040_eye_live_candidate_source_evidence_pc34(void)
{
    return s_spec.sourceEvidence;
}

void dm1_v1_mirror_candidate_c040_eye_live_candidate_init_pc34(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->candidateChampionOrdinal = 3;
    state->leaderEmptyHanded = 0;
    state->leaderHandThing = kLeaderHandObject;
    state->panelContent = kPanelC040;
    state->panelGraphic = kGraphicC040;
    state->openChestThing = kOpenChestThing;
    state->eyeIconGraphic = kIconEyeNotLooking;
    for (i = 0; i < DM1_V1_MC040ELC_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = 0x7000 + i;
    }
}

static void close_chest(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state)
{
    int i;

    if (!state || state->openChestThing == kThingNone) {
        return;
    }
    state->openChestThing = kThingNone;
    for (i = 0; i < DM1_V1_MC040ELC_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = kThingNone;
    }
    ++state->chestCloseCount;
}

static void draw_c040(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state)
{
    state->panelContent = kPanelC040;
    state->panelGraphic = kGraphicC040;
    ++state->c040RedrawCount;
}

static void draw_panel_f0347(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state)
{
    if (!state) {
        return;
    }
    close_chest(state);
    if (state->candidateChampionOrdinal != 0) {
        draw_c040(state);
        return;
    }
    state->panelContent = 565;
}

static void press_eye(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state)
{
    if (!state) {
        return;
    }
    state->ignoreMouseMovements = 1;
    state->pressingEye = 1;
    state->pointerHidden = 1;
    state->eyeIconGraphic = kIconEyeLooking;
    if (state->leaderEmptyHanded) {
        ++state->championStatsDrawCount;
        state->panelContent = 551;
    } else {
        ++state->objectPanelDrawCount;
        state->panelContent = 542;
    }
    ++state->viewportRedrawCount;
}

static void release_eye(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state)
{
    if (!state) {
        return;
    }
    state->eyeIconGraphic = kIconEyeNotLooking;
    draw_panel_f0347(state);
    ++state->viewportRedrawCount;
    state->pointerHidden = 0;
    state->pressingEye = 0;
    state->ignoreMouseMovements = 0;
}

static int slots_cleared(
    const DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 *state)
{
    int i;

    if (!state) {
        return 0;
    }
    for (i = 0; i < DM1_V1_MC040ELC_CHEST_SLOT_COUNT_PC34; ++i) {
        if (state->chestSlots[i] != kThingNone) {
            return 0;
        }
    }
    return 1;
}

int dm1_v1_mirror_candidate_c040_eye_live_candidate_run_pc34(
    DM1_V1_MirrorCandidateC040EyeLiveCandidateResultPc34 *out)
{
    DM1_V1_MirrorCandidateC040EyeLiveCandidateStatePc34 state;
    int leaderHandBefore;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_c040_eye_live_candidate_init_pc34(&state);
    leaderHandBefore = state.leaderHandThing;
    out->assertionCount = 12;
    out->candidateOrdinalBefore = state.candidateChampionOrdinal;
    out->panelContentBefore = state.panelContent;
    out->openChestBefore = state.openChestThing;

    press_eye(&state);
    out->candidateOrdinalAfterPress = state.candidateChampionOrdinal;
    out->panelContentAfterPress = state.panelContent;
    out->pressingEyeAfterPress = state.pressingEye;
    out->pointerHiddenAfterPress = state.pointerHidden;

    release_eye(&state);
    out->candidateOrdinalAfterRelease = state.candidateChampionOrdinal;
    out->panelContentAfterRelease = state.panelContent;
    out->panelGraphicAfterRelease = state.panelGraphic;
    out->pressingEyeAfterRelease = state.pressingEye;
    out->pointerHiddenAfterRelease = state.pointerHidden;
    out->openChestAfterRelease = state.openChestThing;
    out->chestClosedOnRelease =
        state.chestCloseCount == 1 && state.openChestThing == kThingNone;
    out->chestSlotsCleared = slots_cleared(&state);
    out->f0347RedrewC040 =
        state.c040RedrawCount == 1 && state.panelContent == kPanelC040 &&
        state.panelGraphic == kGraphicC040;
    out->f0282NotDispatched = state.f0282DispatchCount == 0;
    out->leaderHandPreserved = state.leaderHandThing == leaderHandBefore;
    out->objectPanelDrawnDuringPress =
        state.objectPanelDrawCount == 1 && state.championStatsDrawCount == 0;
    out->viewportRedrawnForPressAndRelease = state.viewportRedrawCount == 2;

    out->accepted =
        out->candidateOrdinalBefore == 3 &&
        out->candidateOrdinalAfterPress == out->candidateOrdinalBefore &&
        out->candidateOrdinalAfterRelease == out->candidateOrdinalBefore &&
        out->panelContentBefore == kPanelC040 &&
        out->panelContentAfterPress != kPanelC040 &&
        out->pressingEyeAfterPress == 1 &&
        out->pressingEyeAfterRelease == 0 &&
        out->chestClosedOnRelease &&
        out->chestSlotsCleared &&
        out->f0347RedrewC040 &&
        out->f0282NotDispatched &&
        out->leaderHandPreserved &&
        out->objectPanelDrawnDuringPress &&
        out->viewportRedrawnForPressAndRelease;
    return out->accepted;
}
