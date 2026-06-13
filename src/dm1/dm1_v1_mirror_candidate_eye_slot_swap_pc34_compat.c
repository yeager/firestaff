#include "firestaff/dm1/v1/mirror_candidate/eye_slot_swap_pc34_compat.h"

#include <string.h>

/* Contract-only synthetic runtime for the DM1 V1 mirror-candidate
 * C546 eye-route C09-leader-hand-chest slot-swap gate. Mirrors the
 * ReDMCSB (PC 3.4 / MEDIA529) PANEL.C F0352/F0353/F0342/F0347/F0346 +
 * CHEST.C F0333/F0334 dispatch path:
 *
 *   press  : F0352 -> F0342(pressingEye=C1_TRUE)
 *              -> F0334 (CHANGE7_27_FIX close prior open chest)
 *              -> F0333(leaderHandContainer, P0694_B_PressingEye=1)
 *                 (F0333:43-46 skips C145 open-icon draw in C09)
 *   release: F0353 -> F0347
 *              -> F0334 (CHANGE8_09_FIX always-close at F0347:1650)
 *              -> F0346 (C040 resurrect/reincarnate redraw,
 *                       G0299 still live so F0347:1654 routes to F0346)
 */

enum {
    kThingNone = -1,
    kThingTypeContainer = 9,          /* C09_THING_TYPE_CONTAINER */
    /* M012_TYPE(thing) = ((thing & 0x3C00) >> 10). A C09 container
     * therefore has bits 10..13 == 0b1001, so the encoded thing is
     * (9 << 10) | index. Index 0x38 -> 0x2638, index 0x39 -> 0x2639. */
    kChestThingA = 0x2638,            /* a C09 container, leader-hand chest */
    kChestThingB = 0x2639,            /* a different C09 container, prior open */
    kPanelC040 = 568,                 /* M568_PANEL_RESURRECT_REINCARNATE */
    kPanelChest = 569,                /* M569_PANEL_CHEST */
    kGraphicC040 = 40,                /* C040_COMMAND panel graphic */
    kZoneC546Eye = 546,               /* C546_ZONE_EYE */
    kIconEyeNotLooking = 202,         /* C202_ICON_EYE_NOT_LOOKING */
    kIconEyeLooking = 203,            /* C203_ICON_EYE_LOOKING */
    kIconContainerClosed = 144,       /* C144_ICON_CONTAINER_CHEST_CLOSED */
    kIconContainerOpen = 145          /* C145_ICON_CONTAINER_CHEST_OPEN */
};

static const DM1_V1_MirrorCandidateEyeSlotSwapSpecPc34 s_spec = {
    "PANEL.C F0352:2111-2159; PANEL.C F0353:2162-2192; "
    "PANEL.C F0342:1055-1180; PANEL.C F0347:1639-1693; "
    "PANEL.C F0346:1619-1637; CHEST.C F0333:30-67; "
    "CHEST.C F0334:79-130; DEFS.H:2088 C040/C160/C546/M568/C09/C144/C145/C202/C203/G0299/G0424/G0425/G0426",
    "non-overlap: this gate is the C546 eye press/release with a C09 "
    "container in the leader hand while a C040 candidate is live; it "
    "does not cover C160 close, C045 food/water, C544 chest pickup, "
    "C543 chest drop, C545 mouth, F0319 leader death, C028 resurrect, "
    "scroll wheel, keyboard browse, save/load, teleporter, party "
    "rotate, chest auto-close, champion switch, or the existing "
    "C040_eye_live_candidate 'chest already open' sibling",
    "PANEL.C F0352:2111-2159",
    "PANEL.C F0353:2162-2192",
    "PANEL.C F0342:1055-1180",
    "PANEL.C F0347:1639-1693",
    "PANEL.C F0346:1619-1637",
    "CHEST.C F0333:30-67",
    "CHEST.C F0334:79-130",
    kPanelC040,
    kPanelChest,
    kZoneC546Eye,
    kIconEyeLooking,
    kIconEyeNotLooking,
    kIconContainerClosed,
    kIconContainerOpen,
    kThingTypeContainer
};

const DM1_V1_MirrorCandidateEyeSlotSwapSpecPc34 *
dm1_v1_mirror_candidate_eye_slot_swap_spec_pc34(void)
{
    return &s_spec;
}

const char *
dm1_v1_mirror_candidate_eye_slot_swap_source_evidence_pc34(void)
{
    return s_spec.sourceEvidence;
}

const char *
dm1_v1_mirror_candidate_eye_slot_swap_non_overlap_pc34(void)
{
    return s_spec.nonOverlap;
}

void dm1_v1_mirror_candidate_eye_slot_swap_init_pc34(
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->candidateChampionOrdinal = 3;       /* G0299 = 3 (party slot 3) */
    state->leaderHandThing = kChestThingA;     /* C09 container in C01 */
    state->openChestThing = kChestThingB;      /* prior G0426 = chest B */
    state->v1OpenChestOpenedByEye = 0;         /* chest B was opened
                                                * via the non-eye F0333
                                                * path, so v1OpenChest
                                                * OpenedByEye = 0 and
                                                * C09 icon = C145 open */
    state->c09ActionHandIcon = kIconContainerOpen;
    state->panelContent = kPanelC040;          /* G0424 = 568 (C040) */
    state->eyeIconGraphic = kIconEyeNotLooking;
    state->pressingEye = 0;
    state->ignoreMouseMovements = 0;
    state->pointerHidden = 0;
    state->f0333OpenCount = 0;
    state->f0334CloseCount = 0;
    state->objectPanelDrawCount = 0;
    state->c040RedrawCount = 0;
    state->viewportRedrawCount = 0;
    for (i = 0; i < DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = 0x7000 + i;     /* chest B Slot list */
    }
}

static int container_type(unsigned short thing)
{
    if (thing == (unsigned short)kThingNone) {
        return 0;
    }
    /* M012_TYPE bit pattern: bits 10..13. */
    return (((unsigned short)thing) >> 10) & 0x000F;
}

static void close_chest_f0334(
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    if (state->openChestThing == kThingNone) {
        return;
    }
    state->openChestThing = kThingNone;
    state->v1OpenChestOpenedByEye = 0;
    for (i = 0; i < DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = kThingNone;
    }
    ++state->f0334CloseCount;
}

static void open_chest_f0333(
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 *state,
    unsigned short thing,
    int pressingEye)
{
    int i;

    if (!state) {
        return;
    }
    /* F0333:30-32 same-chest reopen guard. */
    if (state->openChestThing == (int)thing) {
        return;
    }
    if (state->openChestThing != kThingNone) {
        close_chest_f0334(state);
    }
    state->openChestThing = (int)thing;
    state->v1OpenChestOpenedByEye = pressingEye ? 1 : 0;
    /* F0333:36 (MEDIA720) G0424_i_PanelContent = M569_PANEL_CHEST. */
    state->panelContent = kPanelChest;
    /* F0333:43-46 P0694_B_PressingEye suppression: the C09 action-hand
     * C145 open-icon draw is skipped when pressingEye is set. The icon
     * stays at C144 (closed) across the F0333 call when the eye route
     * is active. */
    if (!pressingEye) {
        state->c09ActionHandIcon = kIconContainerOpen;
    } else {
        state->c09ActionHandIcon = kIconContainerClosed;
    }
    /* Populate G0425_aT_ChestSlots[0..7] from the container Slot list.
     * F0333:46-67 + CHANGE8_08_FIX caps at 8 visible. */
    for (i = 0; i < DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = 0x7000 + i;
    }
    ++state->f0333OpenCount;
}

static void draw_panel_f0347(
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 *state)
{
    if (!state) {
        return;
    }
    /* F0347:1650 CHANGE8_09_FIX: always close the prior open chest
     * before re-deriving the panel. */
    close_chest_f0334(state);
    /* F0347:1654 G0299 routing: live C040 candidate takes priority. */
    if (state->candidateChampionOrdinal != 0) {
        state->panelContent = kPanelC040;
        ++state->c040RedrawCount;
        return;
    }
    /* Non-C040 fallback: by action-hand thing type. Not on the lane. */
    state->panelContent = kPanelChest;
}

static void press_eye_f0352(
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 *state)
{
    if (!state) {
        return;
    }
    state->ignoreMouseMovements = 1;
    state->pressingEye = 1;
    state->pointerHidden = 1;
    state->eyeIconGraphic = kIconEyeLooking;
    /* F0352:2157 calls F0342(leaderHandObject, C1_TRUE). F0342 with
     * pressingEye = 1 always closes the prior open chest (F0342:1119
     * CHANGE7_27_FIX) and then dispatches the C09 container route to
     * F0333(thing, container, P0694_B_PressingEye = 1). The F0342
     * C09 branch counts as one F0342 object-panel draw. */
    if (container_type((unsigned short)state->leaderHandThing) ==
        kThingTypeContainer) {
        open_chest_f0333(state, (unsigned short)state->leaderHandThing, 1);
        ++state->objectPanelDrawCount;
    } else {
        /* The lane is C09-only. Empty-hand and other types fall
         * through to F0351 (champion stats) or non-container F0342
         * branches; the lane asserts this branch is never reached. */
        state->panelContent = 551;     /* champion stats panel (F0351) */
    }
    ++state->viewportRedrawCount;
}

static void release_eye_f0353(
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 *state)
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

int dm1_v1_mirror_candidate_eye_slot_swap_run_pc34(
    DM1_V1_MirrorCandidateEyeSlotSwapResultPc34 *out)
{
    DM1_V1_MirrorCandidateEyeSlotSwapStatePc34 state;
    int leaderHandBefore;
    int c09IconBefore;
    int eyeIconBefore;
    int openChestBefore;
    int panelContentBefore;
    int candidateBefore;
    int slotsPopulatedOnPress = 0;
    int slotsClearedOnRelease = 0;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    dm1_v1_mirror_candidate_eye_slot_swap_init_pc34(&state);
    leaderHandBefore = state.leaderHandThing;
    c09IconBefore = state.c09ActionHandIcon;
    eyeIconBefore = state.eyeIconGraphic;
    openChestBefore = state.openChestThing;
    panelContentBefore = state.panelContent;
    candidateBefore = state.candidateChampionOrdinal;

    press_eye_f0352(&state);

    out->assertionCount = 24;
    out->candidateOrdinalBefore = candidateBefore;
    out->candidateOrdinalAfterPress = state.candidateChampionOrdinal;
    out->panelContentBefore = panelContentBefore;
    out->panelContentAfterPress = state.panelContent;
    out->openChestBefore = openChestBefore;
    out->openChestAfterPress = state.openChestThing;
    out->openedByEyeAfterPress = state.v1OpenChestOpenedByEye;
    out->c09IconBefore = c09IconBefore;
    out->c09IconAfterPress = state.c09ActionHandIcon;
    out->eyeIconBefore = eyeIconBefore;
    out->eyeIconAfterPress = state.eyeIconGraphic;
    out->pressingEyeAfterPress = state.pressingEye;
    out->pointerHiddenAfterPress = state.pointerHidden;
    out->ignoreMouseAfterPress = state.ignoreMouseMovements;
    for (i = 0; i < DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34; ++i) {
        if (state.chestSlots[i] != kThingNone) {
            ++slotsPopulatedOnPress;
        }
    }
    out->chestSlotsPopulatedOnPress = slotsPopulatedOnPress;
    out->f0333CalledOnce = state.f0333OpenCount == 1;
    out->f0334CalledOnce = state.f0334CloseCount == 1;
    out->objectPanelDrawnOnPress = state.objectPanelDrawCount == 1;
    out->viewportRedrawCount = state.viewportRedrawCount;

    release_eye_f0353(&state);

    out->candidateOrdinalAfterRelease = state.candidateChampionOrdinal;
    out->panelContentAfterRelease = state.panelContent;
    out->openChestAfterRelease = state.openChestThing;
    out->openedByEyeAfterRelease = state.v1OpenChestOpenedByEye;
    out->c09IconAfterRelease = state.c09ActionHandIcon;
    out->eyeIconAfterRelease = state.eyeIconGraphic;
    out->pressingEyeAfterRelease = state.pressingEye;
    out->pointerHiddenAfterRelease = state.pointerHidden;
    out->ignoreMouseAfterRelease = state.ignoreMouseMovements;
    out->leaderHandPreserved = state.leaderHandThing == leaderHandBefore;
    for (i = 0; i < DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34; ++i) {
        if (state.chestSlots[i] == kThingNone) {
            ++slotsClearedOnRelease;
        }
    }
    out->chestSlotsClearedOnRelease =
        slotsClearedOnRelease ==
        DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34;
    out->f0333CalledOnce = state.f0333OpenCount == 1;
    out->f0334CalledOnce = state.f0334CloseCount == 2;
    out->c040RedrawnOnRelease = state.c040RedrawCount == 1;
    out->viewportRedrawCount = state.viewportRedrawCount;

    out->accepted =
        out->candidateOrdinalBefore == 3 &&
        out->candidateOrdinalAfterPress == out->candidateOrdinalBefore &&
        out->candidateOrdinalAfterRelease == out->candidateOrdinalBefore &&
        out->panelContentBefore == kPanelC040 &&
        out->panelContentAfterPress == kPanelChest &&
        out->panelContentAfterRelease == kPanelC040 &&
        out->openChestBefore == (int)kChestThingB &&
        out->openChestAfterPress == (int)kChestThingA &&
        out->openChestAfterRelease == kThingNone &&
        out->openedByEyeAfterPress == 1 &&
        out->openedByEyeAfterRelease == 0 &&
        out->c09IconBefore == kIconContainerOpen &&
        out->c09IconAfterPress == kIconContainerClosed &&
        out->c09IconAfterRelease == kIconContainerClosed &&
        out->eyeIconBefore == kIconEyeNotLooking &&
        out->eyeIconAfterPress == kIconEyeLooking &&
        out->eyeIconAfterRelease == kIconEyeNotLooking &&
        out->pressingEyeAfterPress == 1 &&
        out->pressingEyeAfterRelease == 0 &&
        out->pointerHiddenAfterPress == 1 &&
        out->pointerHiddenAfterRelease == 0 &&
        out->ignoreMouseAfterPress == 1 &&
        out->ignoreMouseAfterRelease == 0 &&
        out->chestSlotsPopulatedOnPress ==
            DM1_V1_MIRROR_CANDIDATE_EYE_SLOT_SWAP_CHEST_SLOT_COUNT_PC34 &&
        out->chestSlotsClearedOnRelease &&
        out->f0333CalledOnce &&
        out->f0334CalledOnce &&
        out->objectPanelDrawnOnPress &&
        out->c040RedrawnOnRelease &&
        out->leaderHandPreserved &&
        out->viewportRedrawCount == 2;
    return out->accepted;
}
