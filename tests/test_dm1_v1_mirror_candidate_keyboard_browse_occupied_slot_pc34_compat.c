#include "dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL %s actual=%d expected=%d [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_uint_eq(unsigned int actual, unsigned int expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL %s actual=%u expected=%u [%s]\n",
               message, actual, expected, anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL %s missing=%s [%s]\n",
               message, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static int expected_before_type(int index)
{
    return index == 1 ? 0x7538 : 0x7300 + index;
}

static int expected_before_weight(int index)
{
    return index == 1 ? 21 : 4 + index;
}

static int expected_after_type(int index)
{
    return index == 1 ? 40 : 0x7300 + index;
}

static int expected_after_weight(int index)
{
    return index == 1 ? 2 : 4 + index;
}

static void test_source_evidence(
    const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 *spec)
{
    check_contains(spec->sourceEvidence, "CHEST.C F0333:30-67",
                   "sourceEvidence pins open chest",
                   spec->chestOpenAnchor);
    check_contains(spec->sourceEvidence, "CHEST.C F0334:117-132",
                   "sourceEvidence pins close relink",
                   spec->chestCloseAnchor);
    check_contains(spec->sourceEvidence, "CHAMPION.C F0284:93-130",
                   "sourceEvidence pins leader/follower identity",
                   spec->championIdentityAnchor);
    check_contains(spec->sourceEvidence, "CHAMPION.C F0297:243-268",
                   "sourceEvidence pins leader-hand put",
                   spec->leaderHandPutAnchor);
    check_contains(spec->sourceEvidence, "CHAMPION.C F0298:270-298",
                   "sourceEvidence pins leader-hand remove",
                   spec->leaderHandRemoveAnchor);
    check_contains(spec->sourceEvidence, "CHAMPION.C F0300:511-515",
                   "sourceEvidence pins C30 clear",
                   spec->slotClearAnchor);
    check_contains(spec->sourceEvidence, "CHAMPION.C F0301:606-614",
                   "sourceEvidence pins C30 write",
                   spec->slotWriteAnchor);
    check_contains(spec->sourceEvidence, "CHAMPION.C F0302:662-710",
                   "sourceEvidence pins occupied-slot swap",
                   spec->occupiedSlotSwapAnchor);
    check_contains(spec->sourceEvidence, "COMMAND.C F0359:1985-1990",
                   "sourceEvidence pins C040 routing",
                   spec->clickToC040Anchor);
    check_contains(spec->sourceEvidence, "COMMAND.C F0380:2075-2156",
                   "sourceEvidence pins keyboard browse",
                   spec->queueKeyboardBrowseAnchor);
    check_contains(spec->sourceEvidence, "REVIVE.C F0280:124-132",
                   "sourceEvidence pins candidate activation",
                   spec->candidateActivationAnchor);
    check_contains(spec->sourceEvidence, "REVIVE.C F0282:744-806",
                   "sourceEvidence pins candidate panel clear path",
                   spec->candidatePanelAnchor);
    check_contains(spec->sourceEvidence, "PANEL.C F0344:1895-1944",
                   "sourceEvidence pins chest mouse click",
                   spec->panelClickAnchor);
    check_contains(spec->sourceEvidence, "PANEL.C F0345:1946-1999",
                   "sourceEvidence pins chest mouse release",
                   spec->panelReleaseAnchor);
    check_contains(spec->sourceEvidence, "UTAMSCR.C F0077:147-151",
                   "sourceEvidence pins update enable",
                   spec->screenUpdateAnchor);
    check_contains(spec->sourceEvidence, "F0078:141-145",
                   "sourceEvidence pins update disable",
                   spec->screenUpdateAnchor);
    check_contains(spec->sourceEvidence, "OBJECT.C F0033:147-212",
                   "sourceEvidence pins icon identity",
                   spec->objectIconAnchor);
    check_contains(spec->sourceEvidence, "BLITMASK.C F0133:30-33",
                   "sourceEvidence pins partial mask",
                   spec->partialMaskAnchor);
    check_contains(spec->sourceEvidence, "DEFS.H:2088",
                   "sourceEvidence pins C10_COLOR_FLESH",
                   spec->defsAnchor);
    check_contains(spec->sourceEvidence, "C30..C36:810-816",
                   "sourceEvidence pins C30..C36",
                   spec->defsAnchor);
    check_contains(spec->sourceEvidence, "C537..C544:3906-3913",
                   "sourceEvidence pins C537..C544",
                   spec->defsAnchor);
    check_contains(spec->sourceEvidence, "C040/G0299/G0425/G0426/G4055/M070/M516",
                   "sourceEvidence pins globals/macros",
                   spec->defsAnchor);
}

static void test_spec_metadata(
    const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 *spec)
{
    check_true(spec != NULL, "spec accessor returns data",
               "REVIVE.C F0280:124-132");
    check_int_eq(spec->partyCount, 2, "party count",
                 spec->championIdentityAnchor);
    check_int_eq(spec->leaderIndex, 0, "leader index",
                 spec->championIdentityAnchor);
    check_uint_eq(spec->leaderOrdinal, 1u, "leader ordinal",
                  spec->championIdentityAnchor);
    check_uint_eq(spec->initialCandidateOrdinal, 1u,
                  "initial candidate starts on leader ordinal",
                  spec->candidateActivationAnchor);
    check_uint_eq(spec->browsedCandidateOrdinal, 2u,
                  "browsed candidate ordinal is follower",
                  spec->queueKeyboardBrowseAnchor);
    check_int_eq(spec->inventoryChampionIndex, 0,
                 "inventory champion index", spec->occupiedSlotSwapAnchor);
    check_uint_eq(spec->inventoryChampionOrdinal, 1u,
                  "inventory champion ordinal", spec->defsAnchor);
    check_int_eq(spec->openChestThing, 0x6b38,
                 "open chest thing sentinel", spec->chestOpenAnchor);
    check_int_eq(spec->c538ChestSlotIndex, 1,
                 "C538 visible slot index", spec->defsAnchor);
    check_int_eq(spec->c538Pc34Slot, DM1_PC34_SLOT_CHEST_2,
                 "C538 maps to second C30+ source slot", spec->defsAnchor);
    check_int_eq(spec->c538DisplayZone, 538,
                 "display zone C538", spec->defsAnchor);
    check_int_eq(spec->c040ScrollThing, 40, "C040 scroll/panel sentinel",
                 spec->clickToC040Anchor);
    check_int_eq(spec->c538OriginalOccupantThing, 0x7538,
                 "original occupied C538 item sentinel",
                 spec->objectIconAnchor);
    check_int_eq(spec->c10ColorFlesh, 10, "C10_COLOR_FLESH value",
                 spec->defsAnchor);
    check_contains(spec->occupiedSlotSwapAnchor, "M070/M516",
                   "swap anchor names M070/M516", spec->occupiedSlotSwapAnchor);
    check_contains(spec->defsAnchor, "G0299", "defs anchor names G0299",
                   spec->defsAnchor);
    check_contains(spec->defsAnchor, "G0425", "defs anchor names G0425",
                   spec->defsAnchor);
    check_contains(spec->defsAnchor, "G0426", "defs anchor names G0426",
                   spec->defsAnchor);
    check_contains(spec->defsAnchor, "G4055", "defs anchor names G4055",
                   spec->defsAnchor);
}

static void test_probe_result(
    const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotResultPc34 *result)
{
    const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 *spec =
        result->spec;
    int i;

    check_true(result->accepted, "probe accepted",
               spec->occupiedSlotSwapAnchor);
    check_true(result->sourceLockedContractOnly,
               "probe is source-locked contract only", spec->defsAnchor);
    check_true(result->chestOpenDispatched, "F0333 open dispatched",
               spec->chestOpenAnchor);
    check_true(result->sameOpenDisplayGuardHeld,
               "same-open display guard held", spec->chestOpenAnchor);
    check_true(result->panelClickDispatched, "F0344 click dispatched",
               spec->panelClickAnchor);
    check_true(result->panelReleaseDispatched, "F0345 release dispatched",
               spec->panelReleaseAnchor);
    check_true(result->keyboardBrowseDispatched,
               "F0380 keyboard browse dispatched",
               spec->queueKeyboardBrowseAnchor);
    check_int_eq(result->keyboardLeftCount, 1,
                 "left browse count", spec->queueKeyboardBrowseAnchor);
    check_int_eq(result->keyboardRightCount, 2,
                 "right browse count", spec->queueKeyboardBrowseAnchor);
    check_true(result->candidateBecameNonLeader,
               "keyboard browse surfaces non-leader candidate",
               spec->championIdentityAnchor);
    check_true(result->candidatePanelActiveBeforeSwap,
               "G0299 panel active before swap", spec->candidateActivationAnchor);
    check_true(result->candidatePanelActiveAfterSwap,
               "G0299 panel active after swap", spec->candidatePanelAnchor);
    check_true(result->candidatePanelActiveAfterClose,
               "G0299 panel active after close", spec->candidatePanelAnchor);
    check_true(result->candidateStatePreservedThroughSwap,
               "G0299 candidate state preserved through swap",
               spec->candidatePanelAnchor);
    check_true(result->candidateStatePreservedThroughClose,
               "G0299 candidate state preserved through close",
               spec->candidatePanelAnchor);
    check_true(result->candidateNoPanelClear,
               "F0282 clear path not reached", spec->candidatePanelAnchor);
    check_true(result->c040RoutingPreserved,
               "C040 routing remains pending", spec->clickToC040Anchor);
    check_true(result->leaderHandCanEquipC538,
               "C040 leader hand passes C538 mask", spec->occupiedSlotSwapAnchor);
    check_true(result->occupiedSlotSwapDispatched,
               "F0302 occupied-slot swap dispatched",
               spec->occupiedSlotSwapAnchor);
    check_true(result->occupiedSlotSwapAccepted,
               "F0302 occupied-slot swap accepted",
               spec->occupiedSlotSwapAnchor);
    check_true(!result->occupiedSlotSwapRejected,
               "F0302 occupied-slot rejection did not fire",
               spec->occupiedSlotSwapAnchor);
    check_true(result->f0298RemovedLeaderHand,
               "F0298 removes C040 leader hand", spec->leaderHandRemoveAnchor);
    check_true(result->f0300ClearedC538,
               "F0300 clears occupied C538", spec->slotClearAnchor);
    check_true(result->f0297PutC538OccupantInLeaderHand,
               "F0297 puts old C538 item in leader hand",
               spec->leaderHandPutAnchor);
    check_true(result->f0301WroteScrollToC538,
               "F0301 writes C040 into C538", spec->slotWriteAnchor);
    check_int_eq(result->screenUpdateEnableCount, 1,
                 "F0077 enable count", spec->screenUpdateAnchor);
    check_int_eq(result->screenUpdateDisableCount, 1,
                 "F0078 disable count", spec->screenUpdateAnchor);
    check_true(result->partialMaskPresented,
               "partial mask presented", spec->partialMaskAnchor);
    check_true(result->iconIdentityPreserved,
               "object icon identity preserved", spec->objectIconAnchor);
    check_true(result->chestCloseDispatched,
               "F0334 close dispatched", spec->chestCloseAnchor);
    check_int_eq(result->closedChestCount, 8,
                 "F0334 relinks all non-empty slots", spec->chestCloseAnchor);
    check_int_eq(result->openChestThingAfterClose, 0,
                 "G0426 cleared after close", spec->chestCloseAnchor);
    check_true(result->closedChainMatchesOpenPostSwap,
               "closed chain matches post-swap C537..C544",
               spec->chestCloseAnchor);

    check_uint_eq(result->candidateOrdinalBeforeBrowse,
                  spec->initialCandidateOrdinal,
                  "candidate ordinal before browse",
                  spec->candidateActivationAnchor);
    check_uint_eq(result->candidateOrdinalAfterBrowse,
                  spec->browsedCandidateOrdinal,
                  "candidate ordinal after browse",
                  spec->queueKeyboardBrowseAnchor);
    check_uint_eq(result->candidateOrdinalAfterSwap,
                  spec->browsedCandidateOrdinal,
                  "candidate ordinal after swap", spec->candidatePanelAnchor);
    check_uint_eq(result->candidateOrdinalAfterClose,
                  spec->browsedCandidateOrdinal,
                  "candidate ordinal after close", spec->candidatePanelAnchor);
    check_int_eq(result->activeRosterIndexBeforeBrowse, 0,
                 "active roster before browse", spec->queueKeyboardBrowseAnchor);
    check_int_eq(result->activeRosterIndexAfterBrowse, 1,
                 "active roster after browse", spec->queueKeyboardBrowseAnchor);
    check_int_eq(result->activeRosterIndexAfterSwap, 1,
                 "active roster after swap", spec->candidatePanelAnchor);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];

        snprintf(label, sizeof(label), "champion %d ordinal", i);
        check_int_eq(result->championOrdinals[i], i + 1, label,
                     spec->championIdentityAnchor);
        snprintf(label, sizeof(label), "champion %d leader flag", i);
        check_int_eq(result->championIsLeader[i], i == 0 ? 1 : 0, label,
                     spec->championIdentityAnchor);
        snprintf(label, sizeof(label), "champion %d current health", i);
        check_true(result->championCurrentHealth[i] > 0, label,
                   spec->occupiedSlotSwapAnchor);
    }

    check_int_eq(result->leaderHandTypeBeforeSwap, spec->c040ScrollThing,
                 "leader hand before swap is C040",
                 spec->leaderHandRemoveAnchor);
    check_int_eq(result->leaderHandWeightBeforeSwap, 2,
                 "leader hand before swap weight",
                 spec->leaderHandRemoveAnchor);
    check_int_eq(result->leaderHandChargesBeforeSwap, 1,
                 "leader hand before swap charges",
                 spec->leaderHandRemoveAnchor);
    check_int_eq(result->leaderHandAllowedSlotsBeforeSwap,
                 DM1_PC34_ALLOWED_CONTAINER,
                 "leader hand before swap allowed slots",
                 spec->occupiedSlotSwapAnchor);
    check_int_eq(result->leaderHandTypeAfterSwap,
                 spec->c538OriginalOccupantThing,
                 "leader hand after swap is old C538 occupant",
                 spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandWeightAfterSwap, 21,
                 "leader hand after swap weight", spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandChargesAfterSwap, 3,
                 "leader hand after swap charges", spec->leaderHandPutAnchor);
    check_int_eq(result->leaderHandAllowedSlotsAfterSwap,
                 DM1_PC34_ALLOWED_CONTAINER,
                 "leader hand after swap allowed slots",
                 spec->leaderHandPutAnchor);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_BROWSE_STEP_COUNT_PC34_COMPAT;
         ++i) {
        const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotStepPc34 *step =
            &result->browseSteps[i];
        char label[96];

        snprintf(label, sizeof(label), "browse step %d queue dispatched", i);
        check_true(step->queueDispatched, label, spec->queueKeyboardBrowseAnchor);
        snprintf(label, sizeof(label), "browse step %d panel active", i);
        check_true(step->panelStillActive, label, spec->candidatePanelAnchor);
    }
    check_int_eq(result->browseSteps[0].key,
                 DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_RIGHT_PC34_COMPAT,
                 "browse step 0 right", spec->queueKeyboardBrowseAnchor);
    check_uint_eq(result->browseSteps[0].candidateOrdinalBefore, 1u,
                  "step 0 ordinal before", spec->queueKeyboardBrowseAnchor);
    check_uint_eq(result->browseSteps[0].candidateOrdinalAfter, 2u,
                  "step 0 ordinal after", spec->queueKeyboardBrowseAnchor);
    check_int_eq(result->browseSteps[1].key,
                 DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_LEFT_PC34_COMPAT,
                 "browse step 1 left", spec->queueKeyboardBrowseAnchor);
    check_uint_eq(result->browseSteps[1].candidateOrdinalBefore, 2u,
                  "step 1 ordinal before", spec->queueKeyboardBrowseAnchor);
    check_uint_eq(result->browseSteps[1].candidateOrdinalAfter, 1u,
                  "step 1 ordinal after", spec->queueKeyboardBrowseAnchor);
    check_int_eq(result->browseSteps[2].key,
                 DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_RIGHT_PC34_COMPAT,
                 "browse step 2 right", spec->queueKeyboardBrowseAnchor);
    check_uint_eq(result->browseSteps[2].candidateOrdinalBefore, 1u,
                  "step 2 ordinal before", spec->queueKeyboardBrowseAnchor);
    check_uint_eq(result->browseSteps[2].candidateOrdinalAfter, 2u,
                  "step 2 ordinal after", spec->queueKeyboardBrowseAnchor);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[128];

        snprintf(label, sizeof(label), "C%d before type",
                 537 + i);
        check_int_eq(result->c537ToC544TypesBefore[i],
                     expected_before_type(i), label,
                     spec->chestOpenAnchor);
        snprintf(label, sizeof(label), "C%d before weight",
                 537 + i);
        check_int_eq(result->c537ToC544WeightsBefore[i],
                     expected_before_weight(i), label,
                     spec->chestOpenAnchor);
        snprintf(label, sizeof(label), "C%d after type",
                 537 + i);
        check_int_eq(result->c537ToC544TypesAfter[i],
                     expected_after_type(i), label,
                     i == 1 ? spec->slotWriteAnchor : spec->chestOpenAnchor);
        snprintf(label, sizeof(label), "C%d after weight",
                 537 + i);
        check_int_eq(result->c537ToC544WeightsAfter[i],
                     expected_after_weight(i), label,
                     i == 1 ? spec->slotWriteAnchor : spec->chestOpenAnchor);
        snprintf(label, sizeof(label), "expected C%d after type", 537 + i);
        check_int_eq(result->expectedTypesAfter[i], expected_after_type(i),
                     label, spec->occupiedSlotSwapAnchor);
        snprintf(label, sizeof(label), "expected C%d after weight", 537 + i);
        check_int_eq(result->expectedWeightsAfter[i], expected_after_weight(i),
                     label, spec->occupiedSlotSwapAnchor);
        snprintf(label, sizeof(label), "closed chain C%d type", 537 + i);
        check_int_eq(result->closedChainTypes[i], expected_after_type(i),
                     label, spec->chestCloseAnchor);
    }
}

int main(void)
{
    const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 *spec =
        dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_spec_pc34();
    DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotResultPc34 result;
    int ok;

    test_source_evidence(spec);
    test_spec_metadata(spec);

    ok = dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_probe_pc34(
        &result);
    check_int_eq(ok, 1, "probe return",
                 "CHAMPION.C F0302:662-710");
    check_true(result.spec == spec, "result references public spec",
               "DEFS.H C040/G0299/G0425/G0426/G4055/M070/M516");
    test_probe_result(&result);

    if (gAssertions < 60) {
        ++gFailures;
        printf("FAIL expected at least 60 assertions, got %d\n", gAssertions);
    }
    if (gFailures == 0) {
        printf("PASS dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_pc34_compat %d assertions\n",
               gAssertions);
    } else {
        printf("FAIL dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_pc34_compat %d assertions %d failures\n",
               gAssertions, gFailures);
    }
    return gFailures == 0 ? 0 : 1;
}
