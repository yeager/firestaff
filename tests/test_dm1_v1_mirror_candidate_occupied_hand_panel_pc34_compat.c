/* ReDMCSB source-lock evidence:
 * ReDMCSB COMMAND.C F0359:1985-1990 gates the M568 C040 panel route on
 * G0415_ui_LeaderEmptyHanded before scanning C160/C161/C162.
 * ReDMCSB REVIVE.C F0282:744-806 owns the cancel/resurrect/reincarnate
 * side effects only after COMMAND.C dispatches a C040 panel command.
 * ReDMCSB REVIVE.C F0280:124-132 refuses candidate publication when the
 * leader hand is already occupied; this gate starts after publication to
 * isolate the C040 button regression.
 */
#include "dm1_v1_mirror_candidate_occupied_hand_panel_pc34_compat.h"

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

static void test_fixture_starts_after_publication_with_occupied_hand(void)
{
    Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat state;

    DM1_V1_MirrorCandidateOccupiedHandPanel_InitPc34Compat(&state);

    CHECK_REDMCSB(state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_M568_PC34_COMPAT,
                  "fixture starts on the M568 C040 panel",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.c040PanelOpen == 1,
                  "fixture starts with C040 open",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.leaderHandEmpty == 0,
                  "fixture pins the occupied leader-hand precondition",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.leaderHandObject ==
                      DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_OBJECT_PC34_COMPAT,
                  "fixture records the object blocking the panel route",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 4u,
                  "fixture preserves a live candidate ordinal",
                  "ReDMCSB REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.partyChampionCount == 4u,
                  "fixture starts after the candidate has been appended",
                  "ReDMCSB REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.inventoryChampionOrdinal == 4u,
                  "fixture keeps the candidate inventory owner selected",
                  "ReDMCSB REVIVE.C F0282:744-806");
}

static void check_occupied_hand_button_is_ignored(int command,
                                                  const char *label)
{
    Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat state;
    Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidateOccupiedHandPanel_InitPc34Compat(&state);
    accepted = DM1_V1_MirrorCandidateOccupiedHandPanel_ClickPc34Compat(
        &state, command, &result);

    CHECK_REDMCSB(result.command == command,
                  label,
                  "ReDMCSB COMMAND.C mouse table:508-512");
    CHECK_REDMCSB(accepted == 0 && result.accepted == 0,
                  "occupied hand rejects the panel command",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.blockedByOccupiedLeaderHand == 1,
                  "occupied hand records the G0415 break",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.f0358HitScanSkipped == 1 &&
                      result.f0358HitScanCountBefore == 0 &&
                      result.f0358HitScanCountAfter == 0,
                  "occupied hand skips the C160/C161/C162 hit scan",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.f0282NotInvoked == 1 &&
                      result.f0282DispatchCountBefore == 0 &&
                      result.f0282DispatchCountAfter == 0,
                  "occupied hand never dispatches F0282",
                  "ReDMCSB REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.commandIgnored == 1 &&
                      result.clickConsumedCountBefore == 0 &&
                      result.clickConsumedCountAfter == 0,
                  "occupied hand leaves the panel click unconsumed",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.candidateStatePreserved == 1,
                  "candidate ordinal, identity, count, and inventory survive",
                  "ReDMCSB REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.panelStatePreserved == 1 &&
                      result.c040PanelOpenAfter == 1,
                  "C040 remains open after the ignored button",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.leaderHandPreserved == 1 &&
                      result.leaderHandObjectAfter ==
                          DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_OBJECT_PC34_COMPAT,
                  "leader-hand object is not removed or swapped",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.noReviveSideEffects == 1,
                  "cancel/resurrect/reincarnate side effects do not run",
                  "ReDMCSB REVIVE.C F0282:744-806");
}

static void test_all_c040_buttons_are_blocked_before_f0282(void)
{
    check_occupied_hand_button_is_ignored(
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C160_PC34_COMPAT,
        "C160 resurrect command is the first C040 button");
    check_occupied_hand_button_is_ignored(
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C161_PC34_COMPAT,
        "C161 reincarnate command is the second C040 button");
    check_occupied_hand_button_is_ignored(
        DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C162_PC34_COMPAT,
        "C162 cancel command is the third C040 button");
}

static void test_source_lock_and_non_overlap_evidence(void)
{
    const Dm1V1MirrorCandidateOccupiedHandPanelEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateOccupiedHandPanel_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence metadata is available",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(strstr(e->commandPanelGateAnchor, "F0359:1985-1990") != NULL,
                  "evidence cites the occupied-hand panel gate",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(strstr(e->commandPanelButtonsAnchor, "508-512") != NULL,
                  "evidence cites the C160/C161/C162 mouse table",
                  "ReDMCSB COMMAND.C mouse table:508-512");
    CHECK_REDMCSB(strstr(e->revivePanelEffectsAnchor, "F0282:744-806") != NULL,
                  "evidence cites the skipped revive panel effects",
                  "ReDMCSB REVIVE.C F0282:744-806");
    CHECK_REDMCSB(strstr(e->revivePublishHandAnchor, "F0280:124-132") != NULL,
                  "evidence cites the publication hand precondition",
                  "ReDMCSB REVIVE.C F0280:124-132");
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "occupied leader-hand") != NULL &&
                      strstr(e->nonOverlapNote, "not cover no-pending") != NULL,
                  "evidence records non-overlap with existing gates",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "evidence marks this as a deterministic contract gate",
                  "ReDMCSB COMMAND.C F0359:1985-1990");
}

int main(void)
{
    test_fixture_starts_after_publication_with_occupied_hand();
    test_all_c040_buttons_are_blocked_before_f0282();
    test_source_lock_and_non_overlap_evidence();

    printf("assertions=%d\n", gTests);
    if (gPasses != gTests) {
        printf("FAIL dm1_v1_mirror_candidate_occupied_hand_panel_pc34_compat "
               "passed=%d/%d\n",
               gPasses, gTests);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_occupied_hand_panel_pc34_compat "
           "assertions=%d\n",
           gTests);
    return 0;
}
