#include "src/dm1/dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.h"

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

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_keyrot_combo_invclick_evidence_pc34_compat();

    CHECK_REDMCSB(e != NULL,
                  "source-lock evidence is available",
                  "metadata");
    CHECK_REDMCSB(e->contractOnly == 1 &&
                      strstr(e->nonDuplicateScope, "key-driven TURN_*") != NULL &&
                      strstr(e->nonDuplicateScope, "not the in-progress") != NULL,
                  "fixture is explicitly non-duplicative",
                  e->nonDuplicateScope);
    CHECK_REDMCSB(strstr(e->f0359PanelDispatchAnchor, "F0359") != NULL &&
                      strstr(e->f0359PanelDispatchAnchor, "1985-1990") != NULL &&
                      strstr(e->f0359PanelDispatchAnchor, "M568/C040") != NULL,
                  "COMMAND.C F0359 panel dispatch anchor is cited",
                  e->f0359PanelDispatchAnchor);
    CHECK_REDMCSB(strstr(e->f0361KeyboardQueueAnchor, "F0361") != NULL &&
                      strstr(e->f0361KeyboardQueueAnchor, "1709-1806") != NULL,
                  "COMMAND.C F0361 keyboard queue write anchor is cited",
                  e->f0361KeyboardQueueAnchor);
    CHECK_REDMCSB(strstr(e->f0380QueueDispatchAnchor, "F0380") != NULL &&
                      strstr(e->f0380QueueDispatchAnchor, "2045-2156") != NULL &&
                      strstr(e->f0380QueueDispatchAnchor, "C001/C002") != NULL,
                  "COMMAND.C F0380 queue dispatch anchor is cited",
                  e->f0380QueueDispatchAnchor);
    CHECK_REDMCSB(strstr(e->f0280CandidatePendingAnchor, "F0280") != NULL &&
                      strstr(e->f0280CandidatePendingAnchor, "124-132") != NULL,
                  "REVIVE.C F0280 candidate-pending anchor is cited",
                  e->f0280CandidatePendingAnchor);
    CHECK_REDMCSB(strstr(e->f0282CandidateClearAnchor, "F0282") != NULL &&
                      strstr(e->f0282CandidateClearAnchor, "744-806") != NULL,
                  "REVIVE.C F0282 candidate-clear anchor is cited",
                  e->f0282CandidateClearAnchor);
    CHECK_REDMCSB(strstr(e->f0297LeaderHandPutAnchor, "F0297") != NULL &&
                      strstr(e->f0297LeaderHandPutAnchor, "243-268") != NULL,
                  "CHAMPION.C F0297 leader-hand put anchor is cited",
                  e->f0297LeaderHandPutAnchor);
    CHECK_REDMCSB(strstr(e->f0302OccupiedSlotClickAnchor, "F0302") != NULL &&
                      strstr(e->f0302OccupiedSlotClickAnchor, "662-710") != NULL,
                  "CHAMPION.C F0302 occupied-slot click anchor is cited",
                  e->f0302OccupiedSlotClickAnchor);
    CHECK_REDMCSB(strstr(e->f0291StatusHandInteractionAnchor, "F0291") != NULL &&
                      strstr(e->f0291StatusHandInteractionAnchor, "621-630") != NULL,
                  "CHAMDRAW.C F0291 status-hand interaction anchor is cited",
                  e->f0291StatusHandInteractionAnchor);
    CHECK_REDMCSB(strstr(e->f0292DrawStateAnchor, "F0292") != NULL &&
                      strstr(e->f0292DrawStateAnchor, "703-735") != NULL,
                  "CHAMDRAW.C F0292 draw-state anchor is cited",
                  e->f0292DrawStateAnchor);
    CHECK_REDMCSB(strstr(e->f0293RedrawAnchor, "F0293") != NULL &&
                      strstr(e->f0293RedrawAnchor, "1117-1143") != NULL,
                  "CHAMDRAW.C F0293 redraw anchor is cited",
                  e->f0293RedrawAnchor);
    CHECK_REDMCSB(strstr(e->defsAnchor, "DEFS.H:2088") != NULL &&
                      strstr(e->defsAnchor, "C30") != NULL &&
                      strstr(e->defsAnchor, "G0425") != NULL &&
                      strstr(e->defsAnchor, "G0426") != NULL &&
                      strstr(e->defsAnchor, "G0423") != NULL &&
                      strstr(e->defsAnchor, "G0305") != NULL &&
                      strstr(e->defsAnchor, "M070") != NULL &&
                      strstr(e->defsAnchor, "M516") != NULL &&
                      strstr(e->defsAnchor, "C040") != NULL,
                  "DEFS.H constant/global anchor is cited",
                  e->defsAnchor);
}

static void check_runtime_case(
    Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat turn,
    int expectedCommand,
    int expectedDirection)
{
    Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat result;
    const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_keyrot_combo_invclick_evidence_pc34_compat();
    int ok = dm1_v1_mirror_candidate_keyrot_combo_invclick_run_pc34_compat(
        turn, &result);

    CHECK_REDMCSB(ok == 1,
                  "runtime helper reports the key-rotation inventory-click race locked",
                  e->f0380QueueDispatchAnchor);
    CHECK_REDMCSB(result.candidateBefore == 1u &&
                      result.candidateAfter == result.candidateBefore &&
                      result.clickDidNotClearCandidate == 1,
                  "candidate remains pending and inventory click does not clear it",
                  e->f0280CandidatePendingAnchor);
    CHECK_REDMCSB(result.keyQueuedCommand == expectedCommand,
                  "TURN_* key was written through the F0361 queue path",
                  e->f0361KeyboardQueueAnchor);
    CHECK_REDMCSB(result.f0380InFlightObservedByClick == 1 &&
                      result.pendingClickCommand ==
                          DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TOGGLE_INVENTORY_PC34 &&
                      result.deferredClickCommand ==
                          result.pendingClickCommand,
                  "inventory portrait click races while F0380 is in-flight and is deferred",
                  e->f0380QueueDispatchAnchor);
    CHECK_REDMCSB(result.rotationProcessedNormally == 1 &&
                      result.directionAfterClick == expectedDirection &&
                      result.directionAfterClick == result.directionAfterNoClick,
                  "rotation is processed normally despite the simultaneous click",
                  e->f0380QueueDispatchAnchor);
    CHECK_REDMCSB(result.clickDidNotDispatchInventoryMutation == 1,
                  "inventory click does not run occupied-slot or leader-hand mutation",
                  e->f0302OccupiedSlotClickAnchor);
    CHECK_REDMCSB(result.redrawByteIdenticalToNoClick == 1 &&
                      memcmp(result.noClickRedraw,
                             result.withClickRedraw,
                             sizeof(result.withClickRedraw)) == 0,
                  "F0293 redraw tuple is byte-identical to a no-click rotation",
                  e->f0293RedrawAnchor);
}

static void test_required_runtime_edges(void)
{
    check_runtime_case(
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_TURN_LEFT_PC34,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TURN_LEFT_PC34,
        3);
    check_runtime_case(
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_TURN_RIGHT_PC34,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TURN_RIGHT_PC34,
        1);
}

static void test_inventory_slot_negative_control(void)
{
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat state;
    Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat result;
    const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_keyrot_combo_invclick_evidence_pc34_compat();
    int ok;

    dm1_v1_mirror_candidate_keyrot_combo_invclick_init_pc34_compat(&state);
    ok = dm1_v1_mirror_candidate_keyrot_combo_invclick_run_case_pc34_compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_TURN_RIGHT_PC34,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_SLOT_BOX_20_PC34,
        &result);

    CHECK_REDMCSB(ok == 1 &&
                      result.deferredClickCommand ==
                          DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_SLOT_BOX_20_PC34,
                  "C040 inventory-slot click is deferred during key dispatch",
                  e->defsAnchor);
    CHECK_REDMCSB(result.clickDidNotDispatchInventoryMutation == 1,
                  "deferred C040 click does not enter F0302/F0297 during the rotation frame",
                  e->f0297LeaderHandPutAnchor);
    CHECK_REDMCSB(result.redrawByteIdenticalToNoClick == 1,
                  "C040 negative-control redraw still matches no-click rotation",
                  e->f0291StatusHandInteractionAnchor);
}

static void test_clear_path_negative_control(void)
{
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat state;
    Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat result;
    const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_keyrot_combo_invclick_evidence_pc34_compat();
    int ok;

    dm1_v1_mirror_candidate_keyrot_combo_invclick_init_pc34_compat(&state);
    ok = dm1_v1_mirror_candidate_keyrot_combo_invclick_run_case_pc34_compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_TURN_LEFT_PC34,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_PANEL_CANCEL_PC34,
        &result);

    CHECK_REDMCSB(ok == 0 &&
                      result.clickDidNotClearCandidate == 0 &&
                      result.candidateAfter == 0u,
                  "panel cancel remains the modeled F0282 candidate-clear path",
                  e->f0282CandidateClearAnchor);
    CHECK_REDMCSB(result.redrawByteIdenticalToNoClick == 0,
                  "candidate clear negative control is not promoted as the race case",
                  e->f0359PanelDispatchAnchor);
}

int main(void)
{
    test_source_lock_metadata();
    test_required_runtime_edges();
    test_inventory_slot_negative_control();
    test_clear_path_negative_control();

    printf("PASS dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
