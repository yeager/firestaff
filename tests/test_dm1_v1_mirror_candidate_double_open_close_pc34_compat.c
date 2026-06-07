#include "dm1_v1_mirror_candidate_double_open_close_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat event_of(
    int kind,
    int tick)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = kind;
    event.tick = tick;
    event.candidateChampionOrdinal =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_DEFAULT_CANDIDATE_PC34_COMPAT;
    return event;
}

static Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat run_events(
    const Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat *events,
    unsigned int count,
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *outState)
{
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;
    int dispatched;

    DM1_V1_MirrorCandidateDoubleOpenClose_InitPc34Compat(&state);
    dispatched = DM1_V1_MirrorCandidateDoubleOpenClose_DispatchPc34Compat(
        &state, events, count, &result);
    CHECK_REDMCSB(dispatched == 1,
                  "rapid candidate sequence dispatches through the runtime gate",
                  "COMMAND.C F0359 lines 1452-1661; F0380 lines 2045-2156");
    if (outState) {
        *outState = state;
    }
    return result;
}

static void check_common_integrity(
    const Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat *result)
{
    CHECK_REDMCSB(result->openedAtMostOncePerLivePanel == 1,
                  "candidate append count matches real open dispatches",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result->closedAtMostOncePerLivePanel == 1,
                  "close dispatch never exceeds live panel ownership",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result->liveChampionPreserved == 1,
                  "rapid panel route does not mutate the live champion",
                  "CHAMPION.C F0302 lines 662-706");
    CHECK_REDMCSB(result->leaderHandPreserved == 1,
                  "rapid panel route does not enter leader-hand put/remove",
                  "CHAMPION.C F0297/F0298 lines 243-285");
    CHECK_REDMCSB(result->sideEffectFinalizeCount == 0,
                  "rapid open/close does not duplicate resurrect finalization",
                  "REVIVE.C F0282 lines 785-806");
    CHECK_REDMCSB(result->actionGateBlockedWhileOpen == 1,
                  "candidate-owned C040 blocks action and spell gates while open",
                  "COMMAND.C lines 2159-2181; 2302-2311");
}

static void test_double_click_resurrect_icon_opens_once(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        100);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        101);
    result = run_events(events, 2u, 0);

    CHECK_REDMCSB(result.finalPanelOpen == 1 &&
                      result.finalPanelPixelsDrawn == 1,
                  "double resurrect-icon tap leaves one C040 panel open",
                  "COMMAND.C G0457 lines 508-511; PANEL.C F0346 lines 1619-1635");
    CHECK_REDMCSB(result.openDispatchCount == 1 &&
                      result.candidateAppendCount == 1,
                  "double resurrect-icon tap appends the candidate once",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.duplicateOpenSuppressedCount == 1,
                  "second rapid resurrect-icon tap is suppressed",
                  "CHAMPION.C C00512_FALSE line 30; COMMAND.C F0359 lines 1452-1661");
    CHECK_REDMCSB(result.finalCandidateChampionOrdinal == 4 &&
                      result.finalSelectedCandidateChampionOrdinal == 4,
                  "candidate identity remains the mirror champion",
                  "DUNVIEW.C lines 3913-3928");
    CHECK_REDMCSB(result.iconRefreshSuppressedCount == 1,
                  "icon refresh honors the candidate panel early return",
                  "CHAMDRAW.C F0296 lines 1210-1213");
    CHECK_REDMCSB(result.queueDispatchCount == 2,
                  "both rapid taps still pass through the command queue",
                  "COMMAND.C F0359 lines 1452-1661; F0380 lines 2045-2156");
    check_common_integrity(&result);
}

static void test_double_click_close_button_closes_once(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[3];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        200);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT,
        201);
    events[2] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT,
        202);
    result = run_events(events, 3u, 0);

    CHECK_REDMCSB(result.finalPanelOpen == 0 &&
                      result.finalPanelPixelsDrawn == 0,
                  "double close leaves C040 cleared",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.closeDispatchCount == 1 &&
                      result.duplicateCloseSuppressedCount == 1,
                  "double close button click closes once",
                  "COMMAND.C F0378 lines 1985-1991; REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.finalCandidateChampionOrdinal == 0 &&
                      result.finalInventoryChampionOrdinal == 0,
                  "close clears candidate and inventory ownership",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.finalPartyChampionCount == 2,
                  "close restores the pre-C040 party count",
                  "REVIVE.C F0282 line 757");
    CHECK_REDMCSB(result.actionGateOpenAfterClose == 1,
                  "action dispatch gate is open after close",
                  "COMMAND.C lines 2302-2311");
    check_common_integrity(&result);
}

static void test_hotkey_then_mirror_icon_opens_correct_candidate(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT,
        300);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT,
        301);
    result = run_events(events, 2u, 0);

    CHECK_REDMCSB(result.openDispatchCount == 1 &&
                      result.duplicateOpenSuppressedCount == 1,
                  "hotkey then mirror icon opens only one candidate panel",
                  "CHAMPION.C C00512_FALSE line 30; COMMAND.C F0361 lines 1709-1813");
    CHECK_REDMCSB(result.finalPanelOpen == 1 &&
                      result.finalCandidateChampionOrdinal == 4,
                  "hotkey then mirror icon keeps the correct candidate",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.finalSelectedCandidateChampionOrdinal == 4,
                  "selected candidate survives the second open request",
                  "DUNVIEW.C lines 3913-3928");
    check_common_integrity(&result);
}

static void test_mirror_icon_then_hotkey_opens_correct_candidate(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT,
        400);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT,
        401);
    result = run_events(events, 2u, 0);

    CHECK_REDMCSB(result.openDispatchCount == 1 &&
                      result.candidateAppendCount == 1,
                  "mirror icon then hotkey appends once",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.finalCandidateChampionOrdinal == 4 &&
                      result.finalSelectedCandidateChampionOrdinal == 4,
                  "mirror icon then hotkey keeps the same candidate",
                  "DUNVIEW.C lines 3913-3928");
    CHECK_REDMCSB(result.duplicateOpenSuppressedCount == 1,
                  "hotkey is a no-op against the already-open panel",
                  "COMMAND.C lines 2159-2181; 2302-2311");
    check_common_integrity(&result);
}

static void test_double_resurrect_hotkey_is_noop(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT,
        500);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT,
        501);
    result = run_events(events, 2u, 0);

    CHECK_REDMCSB(result.openDispatchCount == 1 &&
                      result.duplicateOpenSuppressedCount == 1,
                  "double resurrect hotkey opens once",
                  "CHAMPION.C C00512_FALSE line 30; COMMAND.C F0361 lines 1709-1813");
    CHECK_REDMCSB(result.sideEffectFinalizeCount == 0 &&
                      result.finalLiveChampionHealth == 72,
                  "double hotkey does not run the resurrect finalize branch",
                  "REVIVE.C F0282 lines 785-806");
    check_common_integrity(&result);
}

static void test_close_while_opening_cancels_cleanly(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        600);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT,
        600);
    result = run_events(events, 2u, 0);

    CHECK_REDMCSB(result.openDispatchCount == 1 &&
                      result.closeDispatchCount == 1,
                  "same-tick close-while-opening has one open and one close",
                  "COMMAND.C F0359 lines 1452-1661; REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.finalPanelOpen == 0 &&
                      result.finalCandidateChampionOrdinal == 0,
                  "same-tick close clears panel and candidate",
                  "REVIVE.C F0282 lines 744-758");
    CHECK_REDMCSB(result.finalSelectedCandidateChampionOrdinal == 4,
                  "selected mirror candidate remains available after cancel",
                  "DUNVIEW.C lines 3913-3928");
    check_common_integrity(&result);
}

static void test_deadzone_suppression_works_for_rapid_taps(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[3];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_DEADZONE_CLICK_PC34_COMPAT,
        700);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_DEADZONE_CLICK_PC34_COMPAT,
        701);
    events[2] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        702);
    result = run_events(events, 3u, 0);

    CHECK_REDMCSB(result.deadzoneSuppressedCount == 2,
                  "rapid deadzone taps are suppressed before candidate open",
                  "MOVESENS.C lines 1501-1503; COMMAND.C F0358 lines 1379-1449");
    CHECK_REDMCSB(result.openDispatchCount == 1 &&
                      result.candidateAppendCount == 1,
                  "deadzone taps do not pre-append the candidate",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.finalPanelOpen == 1 &&
                      result.finalCandidateChampionOrdinal == 4,
                  "valid resurrect tap after deadzone opens the right candidate",
                  "DUNVIEW.C lines 3913-3928");
    check_common_integrity(&result);
}

static void test_candidate_selection_survives_close_then_open(void)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[3];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT,
        800);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT,
        801);
    events[2] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT,
        802);
    result = run_events(events, 3u, 0);

    CHECK_REDMCSB(result.openDispatchCount == 2 &&
                      result.closeDispatchCount == 1,
                  "rapid close-then-open creates a fresh single live panel",
                  "REVIVE.C F0280 lines 272-276; F0282 lines 744-758");
    CHECK_REDMCSB(result.finalPanelOpen == 1 &&
                      result.finalCandidateChampionOrdinal == 4,
                  "candidate is live after rapid close-then-open",
                  "REVIVE.C F0280 lines 272-276");
    CHECK_REDMCSB(result.finalSelectedCandidateChampionOrdinal == 4 &&
                      result.candidateSelectionPreserved == 1,
                  "candidate selection survives close-then-open",
                  "DUNVIEW.C lines 3913-3928");
    CHECK_REDMCSB(result.candidateAppendCount == 2,
                  "close-then-open appends exactly once per open panel",
                  "REVIVE.C F0280 lines 272-276");
    check_common_integrity(&result);
}

static void test_spec_and_evidence(void)
{
    const Dm1V1MirrorCandidateDoubleOpenCloseSpecPc34Compat *spec =
        DM1_V1_MirrorCandidateDoubleOpenClose_SpecPc34Compat();
    const char *evidence =
        DM1_V1_MirrorCandidateDoubleOpenClose_SourceEvidencePc34Compat();

    CHECK_REDMCSB(spec == &DM1_V1_MirrorCandidateDoubleOpenCloseSpecPc34Compat,
                  "spec accessor returns the double-open/close spec",
                  "contract marker");
    CHECK_REDMCSB(spec->rapidWindowTicks == 2,
                  "spec records the two-tick rapid-tap window",
                  "CHAMPION.C C00512_FALSE line 30");
    CHECK_REDMCSB(evidence == spec->sourceEvidence,
                  "source evidence accessor matches the spec",
                  "source evidence");
    CHECK_REDMCSB(strstr(evidence, "CHAMPION.C C00512_FALSE line 30") != 0,
                  "evidence cites event-22 timing anchor",
                  "CHAMPION.C line 30");
    CHECK_REDMCSB(strstr(evidence, "COMMAND.C G0457 lines 508-511") != 0 &&
                      strstr(evidence, "F0359 lines 1452-1661") != 0,
                  "evidence cites panel mouse input and command queue routes",
                  "COMMAND.C 508-511; 1452-1661");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0280 lines 272-276") != 0 &&
                      strstr(evidence, "REVIVE.C F0282 lines 744-758") != 0 &&
                      strstr(evidence, "785-806") != 0,
                  "evidence cites append, cancel, and finalize branches",
                  "REVIVE.C F0280/F0282");
    CHECK_REDMCSB(strstr(evidence, "COMMAND.C lines 2159-2181") != 0 &&
                      strstr(evidence, "2302-2311") != 0,
                  "evidence cites G0299 command gates",
                  "COMMAND.C 2159-2181; 2302-2311");
}

int main(void)
{
    test_double_click_resurrect_icon_opens_once();
    test_double_click_close_button_closes_once();
    test_hotkey_then_mirror_icon_opens_correct_candidate();
    test_mirror_icon_then_hotkey_opens_correct_candidate();
    test_double_resurrect_hotkey_is_noop();
    test_close_while_opening_cancels_cleanly();
    test_deadzone_suppression_works_for_rapid_taps();
    test_candidate_selection_survives_close_then_open();
    test_spec_and_evidence();

    printf("PASS dm1_v1_mirror_candidate_double_open_close_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
