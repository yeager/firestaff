#include "dm1/dm1_v1_mirror_candidate_thought_project_traversal_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gAssertions; \
    if (!(cond)) { \
        ++gFailures; \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL && e->contractOnly == 1,
                  "evidence accessor returns contract-only metadata",
                  "COMMAND.C:F0361_COMMAND_ProcessKeyPress:1709-1806");
    CHECK_REDMCSB(strstr(e->commandKeyDispatchAnchor, "1709-1806") != NULL,
                  "evidence cites F0361 key queue insertion",
                  e->commandKeyDispatchAnchor);
    CHECK_REDMCSB(strstr(e->commandQueueDispatchAnchor, "2045-2127") != NULL,
                  "evidence cites F0380 queue dequeue path",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelDispatchAnchor, "1956-1994") != NULL,
                  "evidence cites C040 panel dispatch",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(strstr(e->candidatePublishAnchor, "124-276") != NULL,
                  "evidence cites candidate G0299 publication",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(strstr(e->candidateCloseAnchor, "704-806") != NULL,
                  "evidence cites mirror candidate close/finish cleanup",
                  e->candidateCloseAnchor);
    CHECK_REDMCSB(strstr(e->thoughtTraversalAnchor, "2158-2182") != NULL,
                  "evidence cites live-candidate traversal guard region",
                  e->thoughtTraversalAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract_only=1") != NULL &&
                      strstr(e->contractScope, "C157/C158") != NULL,
                  "contract scope names synthetic thought-project commands",
                  e->commandKeyDispatchAnchor);
}

static void test_c157_projects_live_candidate_without_swap(void)
{
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat state;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat project;
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(&state);

    CHECK_REDMCSB(state.panelOpen == 1,
                  "fixture starts with mirror candidate panel open",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.activeCandidateIndex == 0 &&
                      state.g0299CandidateChampionOrdinal == 3u,
                  "fixture starts with candidate zero as the G0299 live candidate",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.g0420CandidateIdentityOrdinal == 3u,
                  "fixture starts with G0420 candidate identity ordinal three",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.pendingThoughtActive == 0,
                  "fixture starts with no pending thought project",
                  e->commandQueueDispatchAnchor);

    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
        &state, &project);

    CHECK_REDMCSB(project.command ==
                      DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C157_PROJECT_PC34_COMPAT,
                  "C157 project command is recorded",
                  e->commandKeyDispatchAnchor);
    CHECK_REDMCSB(project.panelOpenBefore == 1 && project.panelOpenAfter == 1,
                  "C157 keeps the mirror candidate panel open",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(project.liveCandidatePreserved == 1,
                  "C157 project does not swap the live candidate",
                  e->thoughtTraversalAnchor);
    CHECK_REDMCSB(project.thoughtProjected == 1,
                  "C157 projects the candidate thought into the status box",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(project.statusBoxAfter, "FUL BRO NETA") == 0,
                  "C157 status box text comes from candidate zero",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(project.pendingActiveAfter == 1 &&
                      project.pendingCandidateAfter == 0,
                  "C157 records pending thought state for candidate zero",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(project.projectDispatchCountAfter ==
                      project.projectDispatchCountBefore + 1,
                  "C157 increments project dispatch exactly once",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(state.g0299CandidateChampionOrdinal == 3u &&
                      state.activeCandidateIndex == 0,
                  "state still points to candidate zero after C157",
                  e->thoughtTraversalAnchor);
}

static void test_c158_commits_without_swap(void)
{
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat state;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat project;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat commit;
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
        &state, &project);
    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_CommitPc34Compat(
        &state, &commit);

    CHECK_REDMCSB(commit.command ==
                      DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C158_COMMIT_PC34_COMPAT,
                  "C158 commit command is recorded",
                  e->commandKeyDispatchAnchor);
    CHECK_REDMCSB(commit.panelOpenBefore == 1 && commit.panelOpenAfter == 1,
                  "C158 keeps the mirror candidate panel open",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(commit.liveCandidatePreserved == 1,
                  "C158 commit does not swap the live candidate",
                  e->thoughtTraversalAnchor);
    CHECK_REDMCSB(commit.thoughtCommitted == 1,
                  "C158 commits the pending thought to the live candidate",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(state.candidates[0].committedThought,
                         "FUL BRO NETA") == 0,
                  "candidate zero receives the committed thought text",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(state.candidates[1].committedThought, "") == 0,
                  "candidate one is untouched by candidate zero commit",
                  e->thoughtTraversalAnchor);
    CHECK_REDMCSB(commit.pendingActiveAfter == 1 &&
                      commit.pendingCandidateAfter == 0,
                  "C158 leaves the local pending projection tied to candidate zero",
                  e->commandQueueDispatchAnchor);
}

static void test_candidate_swap_then_c157_pulls_candidate_one(void)
{
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat state;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat projectOne;
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(&state);
    CHECK_REDMCSB(DM1_V1_MirrorCandidateThoughtProjectTraversal_SwapCandidatePc34Compat(
                      &state, 1) == 1,
                  "explicit candidate swap selects candidate one",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.activeCandidateIndex == 1 &&
                      state.g0299CandidateChampionOrdinal == 4u,
                  "candidate one becomes the live G0299 candidate after swap",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.pendingThoughtActive == 0,
                  "candidate swap clears prior thought-project state",
                  e->candidateCloseAnchor);

    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
        &state, &projectOne);

    CHECK_REDMCSB(projectOne.liveCandidatePreserved == 1,
                  "C157 after swap does not swap away from candidate one",
                  e->thoughtTraversalAnchor);
    CHECK_REDMCSB(projectOne.pendingCandidateAfter == 1,
                  "C157 after swap binds pending thought to candidate one",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(projectOne.statusBoxAfter, "DES EW SAR") == 0,
                  "C157 after swap pulls candidate one's thought slot",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(projectOne.statusBoxAfter,
                         state.candidates[1].thoughtSlots[0]) == 0,
                  "status box matches candidate one first thought slot",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(projectOne.statusBoxAfter,
                         state.candidates[0].thoughtSlots[0]) != 0,
                  "status box does not reuse candidate zero thought after swap",
                  e->thoughtTraversalAnchor);
}

static void test_close_clears_and_reopen_has_no_orphan(void)
{
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat state;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat project;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat commit;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat closeResult;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat reopenResult;
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
        &state, &project);
    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_CommitPc34Compat(
        &state, &commit);
    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ClosePc34Compat(
        &state, &closeResult);

    CHECK_REDMCSB(closeResult.command ==
                      DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C162_CLOSE_PC34_COMPAT,
                  "close path records C162 mirror-panel close command",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(closeResult.panelOpenBefore == 1 &&
                      closeResult.panelOpenAfter == 0,
                  "close path closes the mirror candidate panel",
                  e->candidateCloseAnchor);
    CHECK_REDMCSB(closeResult.projectStateCleared == 1,
                  "close path clears pending thought-project state",
                  e->candidateCloseAnchor);
    CHECK_REDMCSB(closeResult.closeDispatchCountAfter ==
                      closeResult.closeDispatchCountBefore + 1,
                  "close path increments close dispatch once",
                  e->candidateCloseAnchor);
    CHECK_REDMCSB(state.pendingThoughtActive == 0 &&
                      state.pendingThoughtCandidateIndex ==
                          DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT,
                  "state has no orphan pending thought after close",
                  e->candidateCloseAnchor);
    CHECK_REDMCSB(state.statusBoxText[0] == '\0',
                  "close path clears the projected status-box text",
                  e->candidateCloseAnchor);
    CHECK_REDMCSB(strcmp(state.candidates[0].committedThought,
                         "FUL BRO NETA") == 0,
                  "close does not erase the committed candidate thought",
                  e->candidateCloseAnchor);

    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ReopenPc34Compat(
        &state, &reopenResult);

    CHECK_REDMCSB(reopenResult.panelOpenBefore == 0 &&
                      reopenResult.panelOpenAfter == 1,
                  "reopen path opens the mirror candidate panel again",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(reopenResult.reopenReset == 1,
                  "reopen path reports project-state reset",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.pendingThoughtActive == 0,
                  "reopen leaves no orphan thought pending",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.pendingThoughtText[0] == '\0' &&
                      state.statusBoxText[0] == '\0',
                  "reopen leaves projected text buffers empty",
                  e->candidatePublishAnchor);
    CHECK_REDMCSB(state.activeCandidateIndex == 0 &&
                      state.g0299CandidateChampionOrdinal == 3u,
                  "reopen preserves the candidate zero live slot",
                  e->candidatePublishAnchor);
}

static void test_empty_thought_c157_is_no_op(void)
{
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat state;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat result;
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(&state);
    state.candidates[0].thoughtSlots[0][0] = '\0';
    strcpy(state.statusBoxText, "UNCHANGED");

    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
        &state, &result);

    CHECK_REDMCSB(result.emptyThoughtNoOp == 1,
                  "C157 with empty thought slot is a no-op",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(result.projectDispatchCountBefore ==
                      result.projectDispatchCountAfter,
                  "empty thought does not increment project dispatch",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(result.statusBoxAfter, "UNCHANGED") == 0,
                  "empty thought leaves status box unchanged",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(result.pendingActiveAfter == 0,
                  "empty thought does not create pending project state",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(result.liveCandidatePreserved == 1,
                  "empty thought no-op does not swap the live candidate",
                  e->thoughtTraversalAnchor);
}

static void test_multi_slot_thought_projects_first_slot_only(void)
{
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat state;
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat result;
    const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(&state);
    strcpy(state.candidates[0].thoughtSlots[0], "FIRST SLOT");
    strcpy(state.candidates[0].thoughtSlots[1], "SECOND SLOT");

    (void)DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
        &state, &result);

    CHECK_REDMCSB(result.thoughtProjected == 1,
                  "multi-slot candidate still projects a thought",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(result.firstSlotOnly == 1,
                  "multi-slot candidate projects only one slot",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(result.projectedSlotIndexAfter == 0,
                  "multi-slot candidate projection uses slot zero",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(result.projectedSlotCountAfter == 1,
                  "multi-slot candidate projection count is one",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(result.statusBoxAfter, "FIRST SLOT") == 0,
                  "multi-slot candidate projects first slot text",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strstr(result.statusBoxAfter, "SECOND") == NULL,
                  "multi-slot candidate does not project second slot text",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(strcmp(result.pendingTextAfter, "FIRST SLOT") == 0,
                  "pending text mirrors only the first slot",
                  e->commandQueueDispatchAnchor);
    CHECK_REDMCSB(result.liveCandidatePreserved == 1,
                  "multi-slot projection does not swap the live candidate",
                  e->thoughtTraversalAnchor);
}

int main(void)
{
    test_source_lock_metadata();
    test_c157_projects_live_candidate_without_swap();
    test_c158_commits_without_swap();
    test_candidate_swap_then_c157_pulls_candidate_one();
    test_close_clears_and_reopen_has_no_orphan();
    test_empty_thought_c157_is_no_op();
    test_multi_slot_thought_projects_first_slot_only();

    printf("assertions=%d failures=%d\n", gAssertions, gFailures);
    return gFailures == 0 && gAssertions >= 25 ? 0 : 1;
}
