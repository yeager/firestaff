/*
 * DM1 V1 Hall of Champions portrait ordinal 13 (WUUF) double_click_stability
 * runtime/contract gate.
 *
 * Narrow, non-duplicative slice that closes the rapid-double-tap row
 * specifically for the WUUF Hall mirror ordinal.  All other ordinal-13
 * portrait coverage lives in:
 *   - firestaff_dm1_v1_champion_portrait_13_east_walkpath_portrait_rect_
 *     position_runtime_probe.c (east_walkpath portrait_rect_position)
 *   - firestaff_dm1_v1_champion_mirror_portrait_13_south_return_portrait_
 *     rect_position_runtime_probe.c (south_return portrait_rect_position)
 *   - firestaff_dm1_v1_champion_mirror_ordinal_13_wuuf_west_negative_
 *     portrait_rect_position_runtime_probe.c (west_negative
 *     portrait_rect_position)
 *   - firestaff_dm1_v1_champion_mirror_ordinal_13_wuuf_after_party_shuffle_
 *     portrait_rect_position_runtime_probe.c (after_party_shuffle
 *     portrait_rect_position)
 *
 * Those probes assert the framebuffer-side rectangle position of the C026
 * ordinal-13 portrait atlas cell.  None of them lock the
 * candidate-panel + rapid double-tap contract for ordinal 13.  The existing
 * tests/test_dm1_v1_mirror_candidate_double_open_close_pc34_compat.c drives
 * the rapid double-tap dispatch with the historical DEFAULT_CANDIDATE = 4
 * (and never reads ordinal 13 from any event), so it cannot lock that
 * ordinal 13 survives rapid taps and close-then-open cycles.
 *
 * Source evidence (ReDMCSB DEFS.H:821-826 + DUNVIEW.C:3913-3928 +
 * DUNVIEW.C:525 + DUNGEON.C:2608-2612 + MOVESENS.C:1501-1503 +
 * REVIVE.C:272-276 + REVIVE.C:744-758 + COMMAND.C F0359:1452-1661 +
 * COMMAND.C F0378:1985-1991 + DEFS.H C026 + DEFS.H M027/M028 +
 * DEFS.H C01_COLOR_DARK_GRAY=1):
 *
 *   - C026 portrait atlas is 256x87 (8 columns x 3 rows of 32x29 portraits).
 *   - ordinal 13 (WUUF) -> atlas cell (col 5, row 1) -> source rect
 *     (160, 29, 32, 29) via M027_PORTRAIT_X(13)=160 and
 *     M028_PORTRAIT_Y(13)=29.
 *   - DUNVIEW.C:3913-3928 blits the C026 source rect onto the fixed D1C
 *     wall-ornament viewport cutout at (96, 35) of size 32x29 (with C01
 *     transparency).
 *   - Rapid double-tap on the resurrect icon (C162 click or its mirror
 *     icon) while the C040 panel is already live must NOT re-append the
 *     candidate champion (REVIVE.C F0280:272-276 only fires once per
 *     panel).  COMMAND.C F0359:1452-1661 / F0380:2045-2156 still
 *     dequeue both clicks, so queueDispatchCount must advance while
 *     candidateAppendCount stays at 1.
 *   - Close-then-open at the same ordinal must reinstall the candidate
 *     (REVIVE.C F0280:272-276) without flipping to a different ordinal
 *     and without doubling the party count beyond the expected
 *     kCandidatePartyChampionCount = 3.
 *
 * The test is data-free: it never loads real DM1 GRAPHICS.DAT or
 * DUNGEON.DAT, never drives M11, never schedules real F0064/F0238 calls,
 * never claims DOSBox original parity.  It locks the same PC 3.4
 * candidate-panel dispatch invariants that
 * test_dm1_v1_mirror_candidate_double_open_close_pc34_compat.c already
 * locks, but at the WUUF (ordinal 13) Hall mirror slot rather than the
 * historical default slot (ordinal 4).  This is the only gate that
 * exercises the runtime with an explicit event-supplied ordinal other
 * than 4.
 *
 * Non-claim: this is a runtime dispatch contract gate.  It does not
 * capture original DM1 PC 3.4 portrait pixels, does not drive the M11
 * game-view pipeline, does not assert z-order over the side walls,
 * and does not claim any portrait_rect_position vs the live
 * framebuffer; that work lives in the existing ordinal-13 portrait
 * probes listed above.
 */
#include "dm1_v1_mirror_candidate_double_open_close_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK(cond, msg) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s\n", msg); \
    } \
} while (0)

static Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat event_of(
    int kind,
    int tick,
    unsigned int candidateOrdinal)
{
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = kind;
    event.tick = tick;
    event.candidateChampionOrdinal = candidateOrdinal;
    return event;
}

static Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat run_events(
    const Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat *events,
    unsigned int count)
{
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat state;
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;
    int dispatched;

    memset(&result, 0, sizeof(result));
    DM1_V1_MirrorCandidateDoubleOpenClose_InitPc34Compat(&state);
    dispatched = DM1_V1_MirrorCandidateDoubleOpenClose_DispatchPc34Compat(
        &state, events, count, &result);
    CHECK(dispatched == 1,
          "rapid WUUF candidate sequence dispatches through the runtime gate");
    return result;
}

static void test_atlas_and_viewport_constants_are_pinned(void)
{
    /* The WUUF ordinal is 13 -> atlas cell (col 5, row 1).  The source
     * rect is locked to (160, 29, 32, 29).  Any drift would let the
     * champion portrait render the wrong atlas cell on the D1C cutout. */
    CHECK(DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_ORDINAL_PC34_COMPAT == 13u,
          "WUUF ordinal is 13 (DEFS.H:821-826 C026 atlas macro)");
    CHECK(DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_SRC_X_PC34_COMPAT == 160,
          "WUUF atlas source X = (13 & 7) * 32 = 160");
    CHECK(DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_HOC_WUUF_SRC_Y_PC34_COMPAT == 29,
          "WUUF atlas source Y = (13 >> 3) * 29 = 29");
    CHECK(DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_W_PC34_COMPAT == 32,
          "C026 portrait width is 32 (DEFS.H:821-826)");
    CHECK(DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_H_PC34_COMPAT == 29,
          "C026 portrait height is 29 (DEFS.H:821-826)");
    CHECK(DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_VP_X_PC34_COMPAT == 96,
          "D1C wall-ornament viewport X is 96 (DUNVIEW.C:3913-3928)");
    CHECK(DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_PORTRAIT_VP_Y_PC34_COMPAT == 35,
          "D1C wall-ornament viewport Y is 35 (DUNVIEW.C:525 G0109 box)");
}

static void test_double_resurrect_icon_tap_at_wuuf(void)
{
    /* Rapid double-tap on the resurrect icon at WUUF (ordinal 13).
     * The runtime must open the C040 panel exactly once with the
     * candidate identity pinned to 13, not 4. */
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        100, 13u);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        101, 13u);
    result = run_events(events, 2u);

    CHECK(result.eventsProcessed == 2,
          "double WUUF resurrect-icon tap processes both events");
    CHECK(result.finalPanelOpen == 1 && result.finalPanelPixelsDrawn == 1,
          "C040 panel stays open after a rapid WUUF double-tap");
    CHECK(result.openDispatchCount == 1 &&
              result.candidateAppendCount == 1,
          "WUUF double-tap appends the candidate exactly once "
          "(REVIVE.C F0280:272-276)");
    CHECK(result.duplicateOpenSuppressedCount == 1,
          "second rapid WUUF resurrect-icon tap is suppressed "
          "(COMMAND.C F0359:1452-1661)");
    CHECK(result.queueDispatchCount == 2,
          "both WUUF clicks still pass through the command queue "
          "(COMMAND.C F0380:2045-2156)");
    CHECK(result.finalCandidateChampionOrdinal == 13 &&
              result.finalSelectedCandidateChampionOrdinal == 13 &&
              result.finalInventoryChampionOrdinal == 13,
          "WUUF candidate identity (13) survives the rapid double-tap");
    CHECK(result.expectedCandidateOrdinal == 13 &&
              result.candidateOrdinalMatchesExpected == 1,
          "expected ordinal flag pins 13 and the runtime keeps "
          "selectedCandidateChampionOrdinal == 13 after the rapid taps");
    CHECK(result.finalPartyChampionCount == 3,
          "WUUF open installs the standard 3-champion candidate-party");
    CHECK(result.iconRefreshSuppressedCount == 1,
          "icon refresh honours the WUUF candidate panel early return "
          "(CHAMDRAW.C F0296:1249-1252)");
    CHECK(result.actionGateBlockedWhileOpen == 1,
          "resurrect + reincarnate gates are blocked while WUUF panel is live");
}

static void test_double_close_button_at_wuuf(void)
{
    /* Open WUUF panel then rapid-close.  Close must clear the
     * candidate identity back to 0 (party_count returns to 2). */
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[3];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        200, 13u);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT,
        201, 13u);
    events[2] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT,
        202, 13u);
    result = run_events(events, 3u);

    CHECK(result.finalPanelOpen == 0 && result.finalPanelPixelsDrawn == 0,
          "double close clears C040 for WUUF (REVIVE.C F0282:744-758)");
    CHECK(result.closeDispatchCount == 1 &&
              result.duplicateCloseSuppressedCount == 1,
          "WUUF double-close dispatches close once, suppresses once "
          "(COMMAND.C F0378:1985-1991)");
    CHECK(result.finalCandidateChampionOrdinal == 0 &&
              result.finalInventoryChampionOrdinal == 0,
          "WUUF close clears candidate and inventory ownership");
    CHECK(result.finalPartyChampionCount == 2,
          "WUUF close restores the pre-C040 party count of 2");
    CHECK(result.actionGateOpenAfterClose == 1,
          "action dispatch gate is open after WUUF close "
          "(COMMAND.C F0380:2302-2311)");
    CHECK(result.expectedCandidateOrdinal == 13,
          "expected ordinal stays pinned to 13 even after close "
          "(reset to 0 is the closed-state contract)");
    /* After close the selected ordinal is 0, so candidateOrdinalMatchesExpected
     * evaluates the closed-state branch (selected==0 and expected seen). */
    CHECK(result.candidateOrdinalMatchesExpected == 1,
          "WUUF close clears the candidate cleanly (selected==0 while "
          "expected stays 13 for the next re-open)");
}

static void test_close_then_open_preserves_wuuf(void)
{
    /* Open WUUF panel, close it, then open it again.  The re-opened
     * panel must reinstall WUUF (13) as the candidate -- it must NOT
     * fall back to the historical default ordinal 4. */
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[3];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT,
        300, 13u);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT,
        301, 13u);
    events[2] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT,
        302, 13u);
    result = run_events(events, 3u);

    CHECK(result.openDispatchCount == 2 &&
              result.closeDispatchCount == 1,
          "WUUF close-then-open dispatches 2 opens + 1 close");
    CHECK(result.candidateAppendCount == 2,
          "WUUF close-then-open appends the candidate once per open panel "
          "(REVIVE.C F0280:272-276)");
    CHECK(result.finalPanelOpen == 1 &&
              result.finalCandidateChampionOrdinal == 13 &&
              result.finalSelectedCandidateChampionOrdinal == 13 &&
              result.finalInventoryChampionOrdinal == 13,
          "WUUF re-open reinstalls the candidate at ordinal 13 (no flip)");
    CHECK(result.expectedCandidateOrdinal == 13 &&
              result.candidateOrdinalMatchesExpected == 1,
          "WUUF close-then-open keeps selectedCandidateChampionOrdinal "
          "pinned to the originally-expected 13");
    CHECK(result.finalPartyChampionCount == 3,
          "WUUF re-open restores the 3-champion candidate-party");
}

static void test_hotkey_then_mirror_icon_at_wuuf(void)
{
    /* Mixed open-kind burst: resurrect hotkey (C161) then mirror icon
     * (C162-style) at WUUF.  Only the first open-kind event should
     * append; the second must be a no-op against the live WUUF panel. */
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[2];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT,
        400, 13u);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT,
        401, 13u);
    result = run_events(events, 2u);

    CHECK(result.openDispatchCount == 1 &&
              result.candidateAppendCount == 1 &&
              result.duplicateOpenSuppressedCount == 1,
          "WUUF hotkey then mirror-icon: only the first open appends");
    CHECK(result.finalCandidateChampionOrdinal == 13 &&
              result.finalSelectedCandidateChampionOrdinal == 13,
          "WUUF candidate identity stays at 13 across hotkey + mirror-icon");
    CHECK(result.queueDispatchCount == 2,
          "both WUUF open-kind events still pass through the command queue");
}

static void test_mixed_open_then_deadzone_then_open_at_wuuf(void)
{
    /* Rapid burst: open, deadzone click, open.  The middle deadzone
     * click must not pre-append the WUUF candidate; the third open
     * must open the WUUF panel exactly once. */
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[3];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        500, 13u);
    events[1] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_DEADZONE_CLICK_PC34_COMPAT,
        501, 13u);
    events[2] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT,
        502, 13u);
    result = run_events(events, 3u);

    CHECK(result.openDispatchCount == 1 &&
              result.candidateAppendCount == 1 &&
              result.deadzoneSuppressedCount == 1,
          "WUUF open -> deadzone -> open: one open + one deadzone suppression");
    CHECK(result.finalPanelOpen == 1 &&
              result.finalCandidateChampionOrdinal == 13 &&
              result.finalSelectedCandidateChampionOrdinal == 13,
          "WUUF panel still resolves to ordinal 13 after the deadzone click");
    CHECK(result.expectedCandidateOrdinal == 13 &&
              result.candidateOrdinalMatchesExpected == 1,
          "expected ordinal stays pinned at 13 across deadzone + open burst");
}

static void test_no_open_event_keeps_expected_unseen(void)
{
    /* Negative control: only a deadzone click (no open-kind event).
     * expectedCandidateOrdinalSeen must stay 0 and the panel must
     * remain closed. */
    Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat events[1];
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat result;

    events[0] = event_of(
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_DEADZONE_CLICK_PC34_COMPAT,
        600, 13u);
    result = run_events(events, 1u);

    CHECK(result.finalPanelOpen == 0,
          "WUUF deadzone-only burst does not open the C040 panel");
    CHECK(result.openDispatchCount == 0 &&
              result.candidateAppendCount == 0 &&
              result.deadzoneSuppressedCount == 1,
          "WUUF deadzone-only burst suppresses the deadzone click");
    CHECK(result.expectedCandidateOrdinal == 0 &&
              result.candidateOrdinalMatchesExpected == 1,
          "expectedCandidateOrdinal stays 0 (no open-kind event) and "
          "candidateOrdinalMatchesExpected is vacuously true");
    CHECK(result.finalCandidateChampionOrdinal == 0 &&
              result.finalSelectedCandidateChampionOrdinal == 0 &&
              result.finalInventoryChampionOrdinal == 0,
          "no candidate ownership is installed by the deadzone-only burst");
}

int main(void)
{
    test_atlas_and_viewport_constants_are_pinned();
    test_double_resurrect_icon_tap_at_wuuf();
    test_double_close_button_at_wuuf();
    test_close_then_open_preserves_wuuf();
    test_hotkey_then_mirror_icon_at_wuuf();
    test_mixed_open_then_deadzone_then_open_at_wuuf();
    test_no_open_event_keeps_expected_unseen();

    printf("PASS dm1_v1_hoc_champion_portrait_13_double_click_stability_"
           "pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
