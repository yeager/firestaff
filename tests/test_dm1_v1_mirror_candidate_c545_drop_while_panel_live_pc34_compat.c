#include "dm1_v1_mirror_candidate_c545_drop_while_panel_live_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock evidence for this contract test:
 * CHEST.C F0334:117-132; CHAMPION.C F0297:243-268 and F0298:270-298;
 * COMMAND.C F0378:1973-1983 and F0380:2045-2159; REVIVE.C F0280:124-132
 * and F0282:744-806; PANEL.C F0346/F0347:1619-1657; UTAMSCR.C
 * F0077/F0078:141-150; BLITMASK.C F0133:30-33; DEFS.H:338-340,
 * 810-817, 1874-1878, 2200, 3001-3008, 5694, and 5876-5881.
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

static void check_uint_eq(unsigned int actual, unsigned int expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%u expected=%u [%s]\n",
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

static int expected_chest_slot(int index)
{
    return 0x7200 + index;
}

static void test_source_metadata(void)
{
    const Dm1V1MirrorC545DropEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_EvidencePc34Compat();
    const char *text =
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_SourceEvidencePc34Compat();

    check_true(e != NULL, "evidence accessor returns metadata",
               "COMMAND.C F0380:2045-2159");
    check_int_eq(e ? e->contractOnly : 0, 1, "contract-only evidence flag",
                 "COMMAND.C F0380:2045-2159");
    check_contains(e->chestCloseAnchor, "CHEST.C F0334:117-132",
                   "evidence cites chest close/relink",
                   "CHEST.C F0334:117-132");
    check_contains(e->leaderHandPutAnchor, "CHAMPION.C F0297:243-268",
                   "evidence cites leader-hand put",
                   "CHAMPION.C F0297:243-268");
    check_contains(e->leaderHandRemoveAnchor, "CHAMPION.C F0298:270-298",
                   "evidence cites leader-hand remove",
                   "CHAMPION.C F0298:270-298");
    check_contains(e->commandDispatchAnchor, "COMMAND.C F0378:1973-1983",
                   "evidence cites C545 dispatch",
                   "COMMAND.C F0378:1973-1983");
    check_contains(e->commandQueueAnchor, "COMMAND.C F0380:2045-2159",
                   "evidence cites queue identity",
                   "COMMAND.C F0380:2045-2159");
    check_contains(e->candidateOpenAnchor, "REVIVE.C F0280:124-132",
                   "evidence cites candidate publish",
                   "REVIVE.C F0280:124-132");
    check_contains(e->candidateClickAnchor, "REVIVE.C F0282:744-806",
                   "evidence cites candidate cleanup",
                   "REVIVE.C F0282:744-806");
    check_contains(e->panelAnchor, "PANEL.C F0346/F0347:1619-1657",
                   "evidence cites C040 redraw",
                   "PANEL.C F0346/F0347:1619-1657");
    check_contains(e->mouseAnchor, "UTAMSCR.C F0077/F0078:141-150",
                   "evidence cites mouse bracket",
                   "UTAMSCR.C F0077/F0078:141-150");
    check_contains(e->blitmaskAnchor, "BLITMASK.C F0133:30-33",
                   "evidence cites masked redraw",
                   "BLITMASK.C F0133:30-33");
    check_contains(e->defsAnchor, "DEFS.H:338-340 C162",
                   "evidence cites C162", "DEFS.H:338-340");
    check_contains(e->defsAnchor, "DEFS.H:810-817 C30..C37",
                   "evidence cites C30..C37", "DEFS.H:810-817");
    check_contains(e->defsAnchor, "DEFS.H:1874-1878 C38",
                   "evidence cites C38", "DEFS.H:1874-1878");
    check_contains(e->defsAnchor, "DEFS.H:2200 C040",
                   "evidence cites C040", "DEFS.H:2200");
    check_contains(e->defsAnchor, "DEFS.H:3001-3008 M568/M569",
                   "evidence cites M568/M569", "DEFS.H:3001-3008");
    check_contains(e->defsAnchor, "DEFS.H:5694 G0299",
                   "evidence cites G0299", "DEFS.H:5694");
    check_contains(e->defsAnchor, "DEFS.H:5876-5881 G0425/G0426",
                   "evidence cites G0425/G0426", "DEFS.H:5876-5881");
    check_contains(e->contractScope, "C545 leader-hand drop-to-floor",
                   "contract names C545 scenario", e->contractScope);
    check_contains(e->contractScope, "pass707",
                   "contract excludes pass707", e->contractScope);
    check_contains(e->contractScope, "pass702",
                   "contract excludes pass702", e->contractScope);
    check_contains(e->contractScope, "pass674",
                   "contract excludes pass674", e->contractScope);
    check_contains(e->contractScope, "pass698",
                   "contract excludes pass698", e->contractScope);

    check_contains(text, "CHEST.C F0334:117-132",
                   "source string cites F0334", "CHEST.C F0334:117-132");
    check_contains(text, "CHAMPION.C F0297:243-268",
                   "source string cites F0297",
                   "CHAMPION.C F0297:243-268");
    check_contains(text, "CHAMPION.C F0298:270-298",
                   "source string cites F0298",
                   "CHAMPION.C F0298:270-298");
    check_contains(text, "COMMAND.C F0378:1973-1983",
                   "source string cites F0378",
                   "COMMAND.C F0378:1973-1983");
    check_contains(text, "COMMAND.C F0380:2045-2159",
                   "source string cites F0380",
                   "COMMAND.C F0380:2045-2159");
    check_contains(text, "REVIVE.C F0280:124-132",
                   "source string cites F0280", "REVIVE.C F0280:124-132");
    check_contains(text, "REVIVE.C F0282:744-806",
                   "source string cites F0282", "REVIVE.C F0282:744-806");
    check_contains(text, "PANEL.C F0346/F0347:1619-1657",
                   "source string cites panel redraw",
                   "PANEL.C F0346/F0347:1619-1657");
    check_contains(text, "UTAMSCR.C F0077/F0078:141-150",
                   "source string cites mouse bracket",
                   "UTAMSCR.C F0077/F0078:141-150");
    check_contains(text, "BLITMASK.C F0133:30-33",
                   "source string cites blitmask",
                   "BLITMASK.C F0133:30-33");
    check_contains(text, "DEFS.H:338-340 C162",
                   "source string cites C162", "DEFS.H:338-340");
    check_contains(text, "DEFS.H:810-817 C30..C37",
                   "source string cites C30..C37", "DEFS.H:810-817");
    check_contains(text, "DEFS.H:1874-1878 C38",
                   "source string cites C38", "DEFS.H:1874-1878");
    check_contains(text, "DEFS.H:2200 C040",
                   "source string cites C040", "DEFS.H:2200");
    check_contains(text, "DEFS.H:3001-3008 M568/M569",
                   "source string cites M568/M569", "DEFS.H:3001-3008");
    check_contains(text, "DEFS.H:5694 G0299",
                   "source string cites G0299", "DEFS.H:5694");
    check_contains(text, "DEFS.H:5876-5881 G0425/G0426",
                   "source string cites G0425/G0426",
                   "DEFS.H:5876-5881");
    check_contains(text, "C545", "source string cites C545",
                   "COMMAND.C F0378:1973-1983");
}

static void test_spec_metadata(void)
{
    const Dm1V1MirrorC545DropSpecPc34Compat *spec =
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_SpecPc34Compat();

    check_true(spec != NULL, "spec accessor returns metadata",
               "REVIVE.C F0280:124-132");
    check_int_eq(spec->leaderIndex, 0, "spec leader index",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(spec->partyChampionCount, 4, "spec party count",
                 "REVIVE.C F0280:124-132");
    check_uint_eq(spec->candidateOrdinal, 4, "spec candidate ordinal",
                  "REVIVE.C F0280:124-132");
    check_int_eq(spec->partyTailChampion, 3, "spec party tail champion",
                 "REVIVE.C F0282:744-806");
    check_int_eq(spec->leaderHandThing, 0x7345, "spec leader-hand thing",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(spec->previousCellThing, 0x5120, "spec previous cell thing",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(spec->openChestThing, 0x6400, "spec open chest thing",
                 "CHEST.C F0334:117-132");
    check_int_eq(spec->panelGraphic,
                 DM1_V1_MIRROR_C545_DROP_C040_PANEL_PC34_COMPAT,
                 "spec C040 panel", "DEFS.H:2200");
    check_int_eq(spec->panelId,
                 DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT,
                 "spec M568 panel id", "DEFS.H:3001-3008");
    check_int_eq(spec->c545Zone,
                 DM1_V1_MIRROR_C545_DROP_C545_ZONE_PC34_COMPAT,
                 "spec C545 zone", "COMMAND.C F0378:1973-1983");
    check_int_eq(spec->c070Command,
                 DM1_V1_MIRROR_C545_DROP_C070_MOUTH_PC34_COMPAT,
                 "spec C070 command", "COMMAND.C F0378:1973-1983");
}

static void test_initial_state(void)
{
    Dm1V1MirrorC545DropStatePc34Compat state;
    int i;

    DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(&state);

    check_int_eq(state.contractOnly, 1, "initial contract-only flag",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.leaderIndex, 0, "initial leader index",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(state.partyChampionCount, 4, "initial party count includes tail",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.partyTailChampion, 3, "initial party tail preserved",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.leaderHandThing, 0x7345,
                 "initial leader hand full",
                 "CHAMPION.C F0297:243-268");
    check_uint_eq(state.candidateOrdinal, 4, "initial candidate ordinal",
                  "REVIVE.C F0280:124-132");
    check_uint_eq(state.g0299CandidateOrdinal, 4,
                  "initial G0299 candidate set",
                  "DEFS.H:5694");
    check_int_eq(state.panelOpen, 1, "initial C040 panel live",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.panelContent,
                 DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT,
                 "initial M568 candidate panel",
                 "DEFS.H:3001-3008");
    check_int_eq(state.openChestThing, 0x6400, "initial open chest state",
                 "DEFS.H:5876-5881");
    check_int_eq(state.cellThingCount, 1, "initial cell list count",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.cellThings[0], 0x5120, "initial cell list head",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.f0280OpenCount, 1, "initial F0280 open count",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.f0297PutCount, 1, "initial F0297 put count",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.f0298RemoveCount, 0, "initial F0298 remove count",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(state.f0334CloseCount, 0, "initial F0334 close count",
                 "CHEST.C F0334:117-132");
    check_int_eq(state.panelRedrawCount, 1, "initial panel redraw count",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.blitmaskCount, 1, "initial masked redraw count",
                 "BLITMASK.C F0133:30-33");
    for (i = 0; i < DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT; ++i) {
        char label[80];

        snprintf(label, sizeof(label), "initial G0425 slot %d", i);
        check_int_eq(state.chestSlots[i], expected_chest_slot(i), label,
                     "DEFS.H:5876-5881");
    }
}

static void check_run_result(
    const Dm1V1MirrorC545DropResultPc34Compat *result)
{
    int i;
    const Dm1V1MirrorC545DropEvidencePc34Compat *e = result->evidence;

    check_true(result->accepted, "runtime mutation accepted",
               e->contractScope);
    check_int_eq(result->initialLeaderHand, 0x7345,
                 "result captures full leader hand before drop",
                 e->leaderHandPutAnchor);
    check_int_eq(result->initialPanelOpen, 1,
                 "result captures live C040 before drop",
                 e->panelAnchor);
    check_int_eq(result->initialPanelContent,
                 DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT,
                 "result captures M568 before drop",
                 e->defsAnchor);
    check_int_eq(result->initialOpenChestThing, 0x6400,
                 "result captures open chest before drop",
                 e->chestCloseAnchor);
    check_int_eq(result->initialPartyTailChampion, 3,
                 "result captures tail before drop",
                 e->candidateClickAnchor);
    check_uint_eq(result->initialCandidateOrdinal, 4,
                  "result captures candidate before drop",
                  e->candidateOpenAnchor);
    check_int_eq(result->initialCellThingCount, 1,
                 "result captures initial cell list count",
                 e->commandQueueAnchor);
    check_int_eq(result->droppedThing, 0x7345,
                 "C545 drops leader-hand thing",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->firstCellThing, 0x5120,
                 "existing cell list head preserved",
                 e->commandQueueAnchor);
    check_int_eq(result->droppedCellThing, 0x7345,
                 "dropped leader-hand thing linked to cell list",
                 e->leaderHandRemoveAnchor);

    check_int_eq(result->f0378DispatchCount, 1,
                 "post-drop F0378 dispatch count",
                 e->commandDispatchAnchor);
    check_int_eq(result->f0380QueueCount, 1,
                 "post-drop F0380 queue count",
                 e->commandQueueAnchor);
    check_int_eq(result->f0334CloseCount, 1,
                 "post-drop F0334 close/relink count",
                 e->chestCloseAnchor);
    check_int_eq(result->f0298RemoveCount, 1,
                 "post-drop F0298 remove count",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->f0280OpenCount, 1,
                 "post-drop F0280 open count preserved",
                 e->candidateOpenAnchor);
    check_int_eq(result->f0282CancelCount, 1,
                 "after-click F0282 cleanup count",
                 e->candidateClickAnchor);
    check_int_eq(result->f0297PutCount, 1,
                 "F0297 put count remains initial put only",
                 e->leaderHandPutAnchor);
    check_int_eq(result->cellThingAddedCount, 1,
                 "cell thing-list gained one object",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->leaderHandEmpty, 1,
                 "leader hand empty after C545 drop",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->panelOpen, 1,
                 "C040 panel remains open after drop",
                 e->panelAnchor);
    check_uint_eq(result->candidateOrdinal, 4,
                  "G0299 candidate ordinal preserved after drop",
                  e->candidateOpenAnchor);
    check_int_eq(result->openChestThing, 0x6400,
                 "open chest state preserved after drop",
                 e->chestCloseAnchor);
    check_int_eq(result->partyTailChampion, 3,
                 "party tail champion preserved after drop",
                 e->candidateClickAnchor);
    check_int_eq(result->cellThingCountAfterDrop, 2,
                 "cell thing-list count after drop",
                 e->leaderHandRemoveAnchor);

    check_int_eq(result->thenResurrectClickClearsCandidate, 1,
                 "follow-up C040 resurrect click clears candidate",
                 e->candidateClickAnchor);
    check_int_eq(result->panelOpenAfterResurrectClick, 0,
                 "panel closes after C040 resurrect click",
                 e->candidateClickAnchor);
    check_uint_eq(result->candidateOrdinalAfterResurrectClick, 0,
                  "G0299 clears after C040 resurrect click",
                  e->candidateClickAnchor);
    check_int_eq(result->partyTailAfterResurrectClick, 3,
                 "party tail remains the same after resurrect click",
                 e->candidateClickAnchor);
    check_int_eq(result->openChestAfterResurrectClick, 0x6400,
                 "open chest still preserved after resurrect click",
                 e->chestCloseAnchor);
    check_int_eq(result->cellThingCountAfterResurrectClick, 2,
                 "cell thing-list stable after resurrect click",
                 e->candidateClickAnchor);
    check_int_eq(result->mutationGuardsOk, 1,
                 "mutation guard matrix passed",
                 e->commandQueueAnchor);

    for (i = 0; i < DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT; ++i) {
        char label[96];
        int expected = expected_chest_slot(i);

        snprintf(label, sizeof(label), "G0425 before slot %d", i);
        check_int_eq(result->chestSlotsBefore[i], expected, label,
                     e->chestCloseAnchor);
        snprintf(label, sizeof(label), "G0425 after drop slot %d", i);
        check_int_eq(result->chestSlotsAfterDrop[i], expected, label,
                     e->chestCloseAnchor);
        snprintf(label, sizeof(label), "G0425 after click slot %d", i);
        check_int_eq(result->chestSlotsAfterResurrectClick[i], expected,
                     label, e->candidateClickAnchor);
    }

    check_int_eq(result->rejectsNullState, 1,
                 "guard rejects null state",
                 e->commandQueueAnchor);
    check_int_eq(result->rejectsNullResult, 1,
                 "guard rejects null result",
                 e->commandQueueAnchor);
    check_int_eq(result->rejectsNonContract, 1,
                 "guard rejects non-contract state",
                 e->commandQueueAnchor);
    check_int_eq(result->rejectsNoPanel, 1,
                 "guard rejects missing live C040 panel",
                 e->panelAnchor);
    check_int_eq(result->rejectsNoCandidate, 1,
                 "guard rejects missing G0299 candidate",
                 e->candidateOpenAnchor);
    check_int_eq(result->rejectsEmptyLeaderHand, 1,
                 "guard rejects empty leader hand",
                 e->leaderHandRemoveAnchor);
    check_int_eq(result->rejectsNoOpenChest, 1,
                 "guard rejects missing open chest",
                 e->chestCloseAnchor);
    check_int_eq(result->rejectsTailMismatch, 1,
                 "guard rejects tail mismatch",
                 e->candidateClickAnchor);
}

static void test_run_sequence(void)
{
    Dm1V1MirrorC545DropStatePc34Compat state;
    Dm1V1MirrorC545DropResultPc34Compat result;
    int ok;

    DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(&state);
    ok = DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
        &state, &result);

    check_int_eq(ok, 1, "run returns accepted",
                 "COMMAND.C F0380:2045-2159");
    check_run_result(&result);
    check_int_eq(state.leaderHandThing,
                 DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT,
                 "state leader hand empty after full run",
                 "CHAMPION.C F0298:270-298");
    check_uint_eq(state.g0299CandidateOrdinal, 0,
                  "state G0299 cleared after follow-up click",
                  "REVIVE.C F0282:744-806");
    check_int_eq(state.panelOpen, 0,
                 "state panel closed after follow-up click",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.openChestThing, 0x6400,
                 "state open chest preserved after run",
                 "CHEST.C F0334:117-132");
    check_int_eq(state.partyTailChampion, 3,
                 "state party tail preserved after run",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.cellThingCount, 2,
                 "state cell thing-list has dropped object",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(state.f0378DispatchCount, 2,
                 "state F0378 covers drop and resurrect click",
                 "COMMAND.C F0378:1973-1983");
    check_int_eq(state.f0380QueueCount, 2,
                 "state F0380 covers drop and resurrect click",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.f0334CloseCount, 1,
                 "state F0334 close count after run",
                 "CHEST.C F0334:117-132");
    check_int_eq(state.f0298RemoveCount, 1,
                 "state F0298 remove count after run",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(state.f0282CancelCount, 1,
                 "state F0282 cleanup count after run",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.mouseEnableCount, 2,
                 "state mouse enable bracket count",
                 "UTAMSCR.C F0077/F0078:141-150");
    check_int_eq(state.mouseDisableCount, 2,
                 "state mouse disable bracket count",
                 "UTAMSCR.C F0077/F0078:141-150");
    check_int_eq(state.blitmaskCount, 2,
                 "state masked redraw includes live panel mutation",
                 "BLITMASK.C F0133:30-33");
    check_int_eq(state.mutationGuardCount, 8,
                 "state records guard coverage",
                 "COMMAND.C F0380:2045-2159");
}

static void test_rejects_invalid_inputs(void)
{
    Dm1V1MirrorC545DropStatePc34Compat state;
    Dm1V1MirrorC545DropResultPc34Compat result;

    check_int_eq(
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            NULL, &result),
        0,
        "run rejects null state",
        "COMMAND.C F0380:2045-2159");
    DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(&state);
    check_int_eq(
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            &state, NULL),
        0,
        "run rejects null result",
        "COMMAND.C F0380:2045-2159");
    state.contractOnly = 0;
    check_int_eq(
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects non-contract state",
        "COMMAND.C F0380:2045-2159");
    DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(&state);
    state.panelOpen = 0;
    check_int_eq(
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects state with no live C040 panel",
        "PANEL.C F0346/F0347:1619-1657");
    DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(&state);
    state.g0299CandidateOrdinal = 0;
    check_int_eq(
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects state with no G0299 candidate",
        "REVIVE.C F0280:124-132");
    DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(&state);
    state.leaderHandThing = DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT;
    check_int_eq(
        DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects state with empty leader hand",
        "CHAMPION.C F0298:270-298");
}

int main(void)
{
    printf("=== DM1 V1 mirror-candidate C545 drop while C040 panel live ===\n");
    test_source_metadata();
    test_spec_metadata();
    test_initial_state();
    test_run_sequence();
    test_rejects_invalid_inputs();
    if (gFailures) {
        printf("FAIL: assertions=%d failures=%d\n", gAssertions, gFailures);
        return 1;
    }
    printf("PASS: assertions=%d failures=0\n", gAssertions);
    return gAssertions >= 100 ? 0 : 1;
}
