#include "dm1_v1_mirror_candidate_thought_project_cancel_after_pickup_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock evidence for this contract test:
 * COMMAND.C F0378:1956-1994 and F0380:2045-2159 pin C162 panel/queue
 * identity; CHEST.C F0333:30-67 and F0334:113-132 pin G0425/G0426;
 * CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-584, F0301:606-660,
 * and F0302:662-713 pin hand/C30+ exchange; REVIVE.C F0280:124-132 and
 * F0282:744-806 pin candidate open/cancel cleanup; PANEL.C F0346/F0347:
 * 1619-1657 pins C040 redraw; UTAMSCR.C F0077/F0078:141-150 pins redraw
 * bracketing; DEFS.H:338-340, 810-817, 1874-1878, 2200, 3001-3008,
 * 5694, 5876-5881 pins constants/globals.
 */

static int gAssertions;
static int gFailures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL: %s missing=%s [%s]\n",
               message,
               needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static int expected_initial_slot(int index)
{
    if (index == 2) {
        return 0x7157;
    }
    return 0x7200 + index;
}

static void test_source_metadata(void)
{
    const Dm1V1MirrorCancelAfterPickupEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_EvidencePc34Compat();
    const char *text =
        DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_SourceEvidencePc34Compat();

    check_true(e != NULL && e->contractOnly == 1,
               "evidence accessor returns contract-only metadata",
               "COMMAND.C F0378:1956-1994");
    check_contains(e->commandPanelAnchor, "F0378:1956-1994",
                   "panel evidence cites F0378",
                   "COMMAND.C F0378:1956-1994");
    check_contains(e->commandQueueAnchor, "F0380:2045-2159",
                   "queue evidence cites F0380",
                   "COMMAND.C F0380:2045-2159");
    check_contains(e->chestOpenAnchor, "F0333:30-67",
                   "chest open evidence cites F0333",
                   "CHEST.C F0333:30-67");
    check_contains(e->chestCloseAnchor, "F0334:113-132",
                   "chest close evidence cites F0334",
                   "CHEST.C F0334:113-132");
    check_contains(e->leaderHandPutAnchor, "F0297:243-268",
                   "leader-hand put evidence cites F0297",
                   "CHAMPION.C F0297:243-268");
    check_contains(e->leaderHandRemoveAnchor, "F0298:270-298",
                   "leader-hand remove evidence cites F0298",
                   "CHAMPION.C F0298:270-298");
    check_contains(e->slotRemoveAnchor, "F0300:511-584",
                   "slot remove evidence cites F0300",
                   "CHAMPION.C F0300:511-584");
    check_contains(e->slotAddAnchor, "F0301:606-660",
                   "slot add evidence cites F0301",
                   "CHAMPION.C F0301:606-660");
    check_contains(e->slotDispatchAnchor, "F0302:662-713",
                   "slot dispatch evidence cites F0302",
                   "CHAMPION.C F0302:662-713");
    check_contains(e->candidateOpenAnchor, "F0280:124-132",
                   "candidate open evidence cites F0280",
                   "REVIVE.C F0280:124-132");
    check_contains(e->candidateCancelAnchor, "F0282:744-806",
                   "candidate cancel evidence cites F0282",
                   "REVIVE.C F0282:744-806");
    check_contains(e->panelAnchor, "F0346/F0347:1619-1657",
                   "panel evidence cites C040 draw path",
                   "PANEL.C F0346/F0347:1619-1657");
    check_contains(e->mouseAnchor, "F0077/F0078:141-150",
                   "mouse evidence cites update bracket",
                   "UTAMSCR.C F0077/F0078:141-150");
    check_contains(e->defsAnchor, "C30..C37",
                   "defs evidence cites C30 chest slots",
                   "DEFS.H:810-817");
    check_contains(e->contractScope, "scroll-pickup followed by ESC/C162 cancel",
                   "contract names chosen slice",
                   e->contractScope);
    check_contains(e->contractScope, "pass674",
                   "contract explicitly avoids pass674",
                   e->contractScope);
    check_contains(e->contractScope, "pass686",
                   "contract explicitly avoids pass686",
                   e->contractScope);
    check_contains(text, "CHEST.C F0333:30-67",
                   "source evidence string cites F0333",
                   "CHEST.C F0333:30-67");
    check_contains(text, "DEFS.H C162/C30..C37/C38/C040/M568/M569/G0299/G0425/G0426",
                   "source evidence string cites defs constants",
                   "DEFS.H:338-340,810-817,1874-1878,2200,3001-3008");
}

static void test_spec_metadata(void)
{
    const Dm1V1MirrorCancelAfterPickupSpecPc34Compat *spec =
        DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_SpecPc34Compat();

    check_true(spec != NULL, "spec accessor returns metadata",
               "DEFS.H:338-340,810-817,1874-1878");
    check_int_eq(spec->leaderIndex, 0, "spec leader index",
                 "CHAMPION.C F0297:243-268");
    check_int_eq((int)spec->candidateOrdinal, 3,
                 "spec candidate ordinal",
                 "REVIVE.C F0280:124-132");
    check_int_eq(spec->thoughtCellX, 11, "spec thought cell x",
                 "REVIVE.C F0282:786-792");
    check_int_eq(spec->thoughtCellY, 7, "spec thought cell y",
                 "REVIVE.C F0282:786-792");
    check_int_eq(spec->chestThing, 0x6400, "spec open chest thing",
                 "CHEST.C F0333:30-67");
    check_int_eq(spec->scrollThing, 0x7157, "spec thought scroll thing",
                 "CHAMPION.C F0302:688-710");
    check_int_eq(spec->pickedSlotIndex, 2, "spec picked slot index",
                 "DEFS.H:810-817");
    check_int_eq(spec->pickedSlotId,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C30_SLOT_CHEST_1_PC34_COMPAT +
                     2,
                 "spec picked C30 slot id",
                 "DEFS.H:810-817");
    check_int_eq(spec->pickedSlotBox,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C38_SLOT_BOX_CHEST_1_PC34_COMPAT +
                     2,
                 "spec picked C38 slot box",
                 "DEFS.H:1874-1878");
    check_int_eq(spec->panelGraphic,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C040_PANEL_PC34_COMPAT,
                 "spec C040 panel graphic",
                 "DEFS.H:2200");
    check_int_eq(spec->resurrectPanelId,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M568_PANEL_PC34_COMPAT,
                 "spec M568 panel id",
                 "DEFS.H:3001-3008");
    check_int_eq(spec->chestPanelId,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M569_PANEL_PC34_COMPAT,
                 "spec M569 panel id",
                 "DEFS.H:3001-3008");
}

static void test_initial_state(void)
{
    Dm1V1MirrorCancelAfterPickupStatePc34Compat state;
    int i;

    DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_InitPc34Compat(
        &state);

    check_int_eq(state.contractOnly, 1, "initial contract-only flag",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.partyCount, 3, "initial party count leaves candidate tail",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.leaderIndex, 0, "initial leader index",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.leaderHandThing, 0, "initial leader hand empty",
                 "REVIVE.C F0280:124-132");
    check_int_eq((int)state.g0299CandidateOrdinal, 3,
                 "initial candidate ordinal published",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.panelOpen, 1, "initial C040 panel open",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.panelContent,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M568_PANEL_PC34_COMPAT,
                 "initial panel content M568",
                 "DEFS.H:3001-3008");
    check_int_eq(state.openChestThing, 0x6400, "initial chest open",
                 "CHEST.C F0333:30-67");
    check_int_eq(state.cellThing, 0x5120, "initial thought cell has prior thing",
                 "REVIVE.C F0282:786-792");
    check_int_eq(state.previousCellThing, 0x5120,
                 "initial previous-cell snapshot recorded",
                 "REVIVE.C F0282:786-792");
    check_int_eq(state.thoughtThing, 0x7157, "initial thought thing",
                 "CHAMPION.C F0302:688-710");
    check_int_eq(state.f0333OpenCount, 1, "initial F0333 open count",
                 "CHEST.C F0333:30-67");
    check_int_eq(state.f0280OpenCount, 1, "initial F0280 open count",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.panelRedrawCount, 1, "initial panel redraw count",
                 "PANEL.C F0346/F0347:1619-1657");
    check_true(strstr(state.thoughtText, "FUL BRO NETA") != NULL,
               "initial thought text seeded",
               "COMMAND.C F0380:2045-2159");
    for (i = 0;
         i < DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[80];

        snprintf(label, sizeof(label), "initial chest slot %d", i);
        check_int_eq(state.chestSlots[i], expected_initial_slot(i), label,
                     "CHEST.C F0333:30-67");
    }
}

static void check_run_result(
    const Dm1V1MirrorCancelAfterPickupResultPc34Compat *result)
{
    const Dm1V1MirrorCancelAfterPickupEvidencePc34Compat *e =
        result->evidence;
    int i;

    check_true(result->accepted, "full sequence accepted",
               e->contractScope);
    check_true(result->thoughtProjected, "thought was projected before pickup",
               e->commandQueueAnchor);
    check_true(result->scrollPickupDispatched,
               "scroll pickup dispatched through slot box",
               e->slotDispatchAnchor);
    check_true(result->cancelDispatched, "C162 cancel dispatched",
               e->candidateCancelAnchor);
    check_true(result->handEmptied, "cancel emptied leader hand",
               e->leaderHandRemoveAnchor);
    check_true(result->previousCellRestored,
               "cancel restored previous thought cell thing",
               e->candidateCancelAnchor);
    check_true(result->thoughtNoLongerProjected,
               "cancel clears thought-project active flag",
               e->candidateCancelAnchor);
    check_true(result->candidateCleared, "cancel clears G0299 candidate",
               e->candidateCancelAnchor);
    check_true(result->chestClosed, "cancel closes the open chest",
               e->chestCloseAnchor);
    check_true(result->pickedSlotRemainsEmpty,
               "picked C30+ slot remains empty after cancel",
               e->slotRemoveAnchor);
    check_true(result->unrelatedChestSlotsPreserved,
               "unrelated chest slots survive cancel",
               e->chestCloseAnchor);
    check_true(result->textPreservedForAudit,
               "thought text remains available for audit",
               e->commandQueueAnchor);

    check_int_eq(result->candidateOrdinalBefore, 3,
                 "candidate ordinal before sequence",
                 e->candidateOpenAnchor);
    check_int_eq(result->candidateOrdinalAfterPickup, 3,
                 "candidate ordinal after pickup",
                 e->slotDispatchAnchor);
    check_int_eq(result->candidateOrdinalAfterCancel, 0,
                 "candidate ordinal after cancel",
                 e->candidateCancelAnchor);
    check_int_eq(result->panelOpenBefore, 1, "panel open before sequence",
                 e->panelAnchor);
    check_int_eq(result->panelOpenAfterPickup, 1,
                 "panel remains open after pickup",
                 e->panelAnchor);
    check_int_eq(result->panelOpenAfterCancel, 0,
                 "panel closes after cancel",
                 e->candidateCancelAnchor);
    check_int_eq(result->panelContentBefore,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M568_PANEL_PC34_COMPAT,
                 "panel content before sequence",
                 e->defsAnchor);
    check_int_eq(result->panelContentAfterPickup,
                 DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M568_PANEL_PC34_COMPAT,
                 "panel content remains M568 after pickup",
                 e->panelAnchor);
    check_int_eq(result->panelContentAfterCancel, 0,
                 "panel content clears after cancel",
                 e->candidateCancelAnchor);
    check_int_eq(result->openChestBefore, 0x6400,
                 "open chest before sequence",
                 e->chestOpenAnchor);
    check_int_eq(result->openChestAfterPickup, 0x6400,
                 "open chest remains open after pickup",
                 e->slotDispatchAnchor);
    check_int_eq(result->openChestAfterCancel, 0,
                 "open chest clears after cancel",
                 e->chestCloseAnchor);
    check_int_eq(result->leaderHandBefore, 0,
                 "leader hand empty before pickup",
                 e->candidateOpenAnchor);
    check_int_eq(result->leaderHandAfterPickup, 0x7157,
                 "leader hand holds thought scroll after pickup",
                 e->leaderHandPutAnchor);
    check_int_eq(result->leaderHandAfterCancel, 0,
                 "leader hand empty after cancel",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->pickedSlotBefore, 0x7157,
                 "picked slot contains thought scroll before pickup",
                 e->chestOpenAnchor);
    check_int_eq(result->pickedSlotAfterPickup, 0,
                 "picked slot empty after pickup",
                 e->slotRemoveAnchor);
    check_int_eq(result->pickedSlotAfterCancel, 0,
                 "picked slot still empty after cancel",
                 e->chestCloseAnchor);
    check_int_eq(result->cellThingBeforeProject, 0x5120,
                 "cell has previous thing before project",
                 e->candidateOpenAnchor);
    check_int_eq(result->cellThingAfterProject, 0x7157,
                 "cell has thought thing after project",
                 e->commandQueueAnchor);
    check_int_eq(result->cellThingAfterPickup, 0x7157,
                 "cell remains projected while hand carries scroll",
                 e->slotDispatchAnchor);
    check_int_eq(result->cellThingAfterCancel, 0x5120,
                 "cell restores previous thing after cancel",
                 e->candidateCancelAnchor);
    check_int_eq(result->previousCellThing, 0x5120,
                 "result records previous cell thing",
                 e->candidateCancelAnchor);
    check_int_eq(result->thoughtThing, 0x7157,
                 "result records thought thing",
                 e->slotDispatchAnchor);

    for (i = 0; i < 12; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "counter %d monotonic after pickup", i);
        check_true(result->countersAfterPickup[i] >= result->countersBefore[i],
                   label, e->commandQueueAnchor);
        snprintf(label, sizeof(label), "counter %d monotonic after cancel", i);
        check_true(result->countersAfterCancel[i] >=
                       result->countersAfterPickup[i],
                   label, e->candidateCancelAnchor);
    }

    check_int_eq(result->countersBefore[0], 0, "project count before",
                 e->commandQueueAnchor);
    check_int_eq(result->countersAfterPickup[0], 1,
                 "project count after pickup",
                 e->commandQueueAnchor);
    check_int_eq(result->countersAfterCancel[0], 1,
                 "project count stable after cancel",
                 e->candidateCancelAnchor);
    check_int_eq(result->countersAfterPickup[1], 1,
                 "pickup count after pickup",
                 e->slotDispatchAnchor);
    check_int_eq(result->countersAfterCancel[2], 1,
                 "cancel count after cancel",
                 e->candidateCancelAnchor);
    check_int_eq(result->countersAfterCancel[4], 1,
                 "F0334 close count after cancel",
                 e->chestCloseAnchor);
    check_int_eq(result->countersAfterPickup[5], 1,
                 "F0297 put count after pickup",
                 e->leaderHandPutAnchor);
    check_int_eq(result->countersAfterCancel[6], 1,
                 "F0298 remove count after cancel",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->countersAfterPickup[7], 1,
                 "F0300 slot remove count after pickup",
                 e->slotRemoveAnchor);
    check_int_eq(result->countersAfterCancel[8], 0,
                 "F0301 slot add stays unused on cancel",
                 e->slotAddAnchor);
    check_int_eq(result->countersAfterPickup[9], 1,
                 "F0302 slot dispatch count after pickup",
                 e->slotDispatchAnchor);
    check_int_eq(result->countersAfterCancel[10], 1,
                 "F0280 open count preserved",
                 e->candidateOpenAnchor);
    check_int_eq(result->countersAfterCancel[11], 1,
                 "F0282 cancel count after cancel",
                 e->candidateCancelAnchor);

    for (i = 0;
         i < DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        char label[96];
        int expectedAfter = i == 2 ? 0 : expected_initial_slot(i);

        snprintf(label, sizeof(label), "slot %d before sequence", i);
        check_int_eq(result->chestSlotsBefore[i],
                     expected_initial_slot(i),
                     label,
                     e->chestOpenAnchor);
        snprintf(label, sizeof(label), "slot %d after pickup", i);
        check_int_eq(result->chestSlotsAfterPickup[i],
                     expectedAfter,
                     label,
                     e->slotDispatchAnchor);
        snprintf(label, sizeof(label), "slot %d after cancel", i);
        check_int_eq(result->chestSlotsAfterCancel[i],
                     expectedAfter,
                     label,
                     e->chestCloseAnchor);
    }

    check_contains(result->thoughtTextAfterCancel, "FUL BRO NETA",
                   "thought text after cancel remains original",
                   e->commandQueueAnchor);
}

static void test_run_sequence(void)
{
    Dm1V1MirrorCancelAfterPickupStatePc34Compat state;
    Dm1V1MirrorCancelAfterPickupResultPc34Compat result;
    int ok;

    DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_InitPc34Compat(
        &state);
    ok = DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_RunPc34Compat(
        &state, &result);

    check_true(ok == 1, "run function returns accepted",
               "COMMAND.C F0380:2045-2159");
    check_run_result(&result);
    check_int_eq(state.leaderHandThing, 0,
                 "state leader hand empty after run",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(state.cellThing, state.previousCellThing,
                 "state cell restored after run",
                 "REVIVE.C F0282:744-806");
    check_int_eq((int)state.g0299CandidateOrdinal, 0,
                 "state candidate cleared after run",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.thoughtProjected, 0,
                 "state thought projection cleared after run",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.thoughtRestoredByCancel, 1,
                 "state records cancel restore",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.leaderHandWasEmptiedByCancel, 1,
                 "state records hand clear",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(state.mouseEnableCount, 2,
                 "mouse enable count covers pickup and cancel",
                 "UTAMSCR.C F0077:147-150");
    check_int_eq(state.mouseDisableCount, 2,
                 "mouse disable count covers pickup and cancel",
                 "UTAMSCR.C F0078:141-145");
}

static void test_rejects_invalid_inputs(void)
{
    Dm1V1MirrorCancelAfterPickupStatePc34Compat state;
    Dm1V1MirrorCancelAfterPickupResultPc34Compat result;

    check_int_eq(
        DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_RunPc34Compat(
            NULL, &result),
        0,
        "run rejects null state",
        "COMMAND.C F0380:2045-2159");
    DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_InitPc34Compat(
        &state);
    check_int_eq(
        DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_RunPc34Compat(
            &state, NULL),
        0,
        "run rejects null result",
        "COMMAND.C F0380:2045-2159");
    state.contractOnly = 0;
    check_int_eq(
        DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_RunPc34Compat(
            &state, &result),
        0,
        "run rejects non-contract state",
        "COMMAND.C F0380:2045-2159");
}

int main(void)
{
    printf("=== DM1 V1 mirror-candidate thought-project cancel-after-pickup gate ===\n");
    test_source_metadata();
    test_spec_metadata();
    test_initial_state();
    test_run_sequence();
    test_rejects_invalid_inputs();
    if (gFailures) {
        printf("FAIL: %d assertion(s) failed out of %d\n",
               gFailures,
               gAssertions);
        return 1;
    }
    printf("PASS: assertions=%d failures=0\n", gAssertions);
    return 0;
}
