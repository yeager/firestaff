#include "dm1_v1_mirror_candidate_keyboard_browse_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
        printf("PASS: %s [%s]\n", msg, anchor); \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_next_champion_stays_inside_visible_page(void)
{
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat state;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat first;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat second;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat third;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat fourth;
    const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardBrowse_EvidencePc34Compat();

    DM1_V1_MirrorCandidateKeyboardBrowse_InitPc34Compat(&state);

    CHECK_REDMCSB(state.panelActive == 1 && state.rosterCount == 8,
                  "fixture opens a deterministic eight-candidate roster",
                  e->contractScope);
    CHECK_REDMCSB(state.pageSize == 4 && state.pageStartIndex == 0,
                  "fixture starts on the first four-slot page",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(state.visibleIndex == 0 &&
                      state.highlightedRosterIndex == 0,
                  "fixture starts with candidate zero highlighted",
                  e->commandKeyboardQueueAnchor);

    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT,
        &first);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT,
        &second);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT,
        &third);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT,
        &fourth);

    CHECK_REDMCSB(first.consumed == 1 && second.consumed == 1,
                  "keyboard next commands are consumed by browse",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(first.pageStartBefore == 0 && first.pageStartAfter == 0,
                  "first next command keeps the current page",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(first.visibleIndexBefore == 0 &&
                      first.visibleIndexAfter == 1,
                  "first next command advances visible index",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(first.highlightedRosterIndexAfter == 1 &&
                      first.highlightedChampionOrdinalAfter == 2u,
                  "first next command highlights roster candidate one",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(second.visibleIndexBefore == 1 &&
                      second.visibleIndexAfter == 2,
                  "second next command advances within the same page",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(second.highlightedRosterIndexAfter == 2 &&
                      second.highlightedChampionOrdinalAfter == 3u,
                  "second next command updates highlight identity",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(third.visibleIndexBefore == 2 &&
                      third.visibleIndexAfter == 3,
                  "third next command reaches the fourth visible slot",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(fourth.visibleIndexBefore == 3 &&
                      fourth.visibleIndexAfter == 0,
                  "fourth next command cycles within the visible page",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(fourth.pageStartBefore == 0 &&
                      fourth.pageStartAfter == 0 &&
                      fourth.highlightedRosterIndexAfter == 0,
                  "next cycle wraps highlight without changing page",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(first.visibleIndexStayedInPage == 1 &&
                      second.visibleIndexStayedInPage == 1 &&
                      third.visibleIndexStayedInPage == 1 &&
                      fourth.visibleIndexStayedInPage == 1,
                  "visible index remains within the four-slot page",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(second.nonTriggerContractHeld == 1 &&
                      third.nonTriggerContractHeld == 1 &&
                      fourth.nonTriggerContractHeld == 1,
                  "next browse does not trigger candidate panel side effects",
                  e->commandCandidateGateAnchor);
}

static void test_page_down_advances_and_refreshes_portrait(void)
{
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat state;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat next;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat pageDown;
    const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardBrowse_EvidencePc34Compat();

    DM1_V1_MirrorCandidateKeyboardBrowse_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT,
        &next);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_DOWN_PC34_COMPAT,
        &pageDown);

    CHECK_REDMCSB(pageDown.pageStartBefore == 0 &&
                      pageDown.pageStartAfter == 4,
                  "page down advances to the next four-candidate page",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(pageDown.visibleIndexBefore == 1 &&
                      pageDown.visibleIndexAfter == 0,
                  "page down resets visible index to the first page slot",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(pageDown.pageDownResetVisibleIndex == 1,
                  "page-down reset flag is set",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(pageDown.highlightedRosterIndexAfter == 4 &&
                      pageDown.highlightedChampionOrdinalAfter == 5u,
                  "page down highlights first candidate on the new page",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(pageDown.portraitReadCountAfter ==
                      pageDown.portraitReadCountBefore + 1,
                  "page down re-reads the highlighted candidate portrait",
                  e->chamdrawIconBitmapAnchor);
    CHECK_REDMCSB(pageDown.portraitSensorIndexAfter == 131,
                  "page down reads through the new C127 portrait sensor token",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(pageDown.portraitBitmapTokenAfter !=
                      pageDown.portraitBitmapTokenBefore,
                  "page down changes the synthetic portrait bitmap token",
                  e->chamdrawIconBitmapAnchor);
    CHECK_REDMCSB(pageDown.portraitRefreshContractOnly == 1,
                  "portrait refresh is marked contract-only, not real parity",
                  e->contractScope);
    CHECK_REDMCSB(pageDown.nonTriggerContractHeld == 1,
                  "page down does not trigger resurrect/reincarnate/inventory",
                  e->commandPanelRouteAnchor);
}

static void test_page_up_wraps_previous_page(void)
{
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat state;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat pageUp;
    const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardBrowse_EvidencePc34Compat();

    DM1_V1_MirrorCandidateKeyboardBrowse_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_PAGE_UP_PC34_COMPAT,
        &pageUp);

    CHECK_REDMCSB(pageUp.pageStartBefore == 0 &&
                      pageUp.pageStartAfter == 4,
                  "page up from the first page wraps to the previous page",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(pageUp.visibleIndexAfter == 0,
                  "page up wrap resets visible index",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(pageUp.pageUpWrappedPreviousPage == 1,
                  "page-up wrap flag is set",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(pageUp.highlightedRosterIndexAfter == 4 &&
                      pageUp.highlightedChampionOrdinalAfter == 5u,
                  "page up wrap lands on the previous page first slot",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(pageUp.visibleIndexStayedInPage == 1,
                  "wrapped visible index remains in the four-slot page",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(pageUp.nonTriggerContractHeld == 1,
                  "page up does not trigger panel side effects",
                  e->commandPanelRouteAnchor);
}

static void test_partial_press_release_cleans_deadzone(void)
{
    Dm1V1MirrorCandidateKeyboardBrowseStatePc34Compat state;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat press;
    Dm1V1MirrorCandidateKeyboardBrowseResultPc34Compat release;
    const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardBrowse_EvidencePc34Compat();

    DM1_V1_MirrorCandidateKeyboardBrowse_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_NEXT_PC34_COMPAT,
        &press);
    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_SELECT_PRESS_PC34_COMPAT,
        &press);

    CHECK_REDMCSB(press.pendingPressChampionOrdinalAfter == 2u,
                  "partial press records the highlighted candidate only",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(press.selectedChampionOrdinalAfter == 0u &&
                      press.candidateChampionOrdinalAfter == 0u,
                  "partial press does not finalize a candidate",
                  e->commandCandidateGateAnchor);

    (void)DM1_V1_MirrorCandidateKeyboardBrowse_ApplyKeyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_KEY_RELEASE_PC34_COMPAT,
        &release);

    CHECK_REDMCSB(release.pendingPressChampionOrdinalBefore == 2u &&
                      release.pendingPressChampionOrdinalAfter == 0u,
                  "key release clears the partial press",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(release.deadzoneCancelCountAfter ==
                      release.deadzoneCancelCountBefore + 1,
                  "key release records deadzone/cancel cleanup",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(release.partialPressReleasedCleanly == 1,
                  "release does not leave a half-selected candidate",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(release.selectedChampionOrdinalAfter == 0u &&
                      release.candidateChampionOrdinalAfter == 0u,
                  "release does not promote or append the candidate",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(release.nonTriggerContractHeld == 1,
                  "release does not trigger resurrect/reincarnate/inventory",
                  e->commandPanelRouteAnchor);
}

static void test_source_lock_and_non_overlap_evidence(void)
{
    const Dm1V1MirrorCandidateKeyboardBrowseEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardBrowse_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence accessor returns source-lock metadata",
                  "metadata");
    CHECK_REDMCSB(strstr(e->chamdrawIconBitmapAnchor, "F0622") != NULL,
                  "evidence cites CHAMDRAW.C F0622",
                  e->chamdrawIconBitmapAnchor);
    CHECK_REDMCSB(strstr(e->championInterfaceInputAnchor, "F0331") != NULL,
                  "evidence cites CHAMPION.C keyboard install",
                  e->championInterfaceInputAnchor);
    CHECK_REDMCSB(strstr(e->commandKeyboardQueueAnchor, "F0361") != NULL,
                  "evidence cites COMMAND.C keyboard dispatch",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelRouteAnchor, "C160/C161/C162") != NULL,
                  "evidence cites panel resurrect/reincarnate/cancel routes",
                  e->commandPanelRouteAnchor);
    CHECK_REDMCSB(strstr(e->commandCandidateGateAnchor, "F0380") != NULL,
                  "evidence cites COMMAND.C F0380 candidate gates",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(strstr(e->clikmenuMovementHighlightAnchor, "CLIKMENU.C") != NULL,
                  "evidence cites CLIKMENU.C as outside browse path",
                  e->clikmenuMovementHighlightAnchor);
    CHECK_REDMCSB(strstr(e->coordLayoutAnchor, "F0640") != NULL,
                  "evidence cites F0640 layout route",
                  e->coordLayoutAnchor);
    CHECK_REDMCSB(strstr(e->nonOverlapNote,
                         "keyboard-driven paged browse") != NULL,
                  "evidence includes the required non-overlap note",
                  e->nonOverlapNote);
    CHECK_REDMCSB(strstr(e->contractScope, "do not claim real-asset parity") !=
                      NULL,
                  "evidence rejects real-asset portrait parity claims",
                  e->contractScope);
}

int main(void)
{
    test_next_champion_stays_inside_visible_page();
    test_page_down_advances_and_refreshes_portrait();
    test_page_up_wraps_previous_page();
    test_partial_press_release_cleans_deadzone();
    test_source_lock_and_non_overlap_evidence();

    if (gPasses != gTests) {
        printf("FAIL dm1_v1_mirror_candidate_keyboard_browse_pc34_compat "
               "%d/%d assertions\n",
               gPasses, gTests);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_keyboard_browse_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return 0;
}
