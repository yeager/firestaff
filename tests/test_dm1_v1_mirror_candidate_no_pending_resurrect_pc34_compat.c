#include "dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++g_assertions; \
    if (cond) { \
        printf("PASS: %s [%s]\n", msg, anchor); \
    } else { \
        ++g_failures; \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static int has_phrase(const char *text, const char *phrase)
{
    return text && phrase && strstr(text, phrase) != NULL;
}

static void test_contract_fields(void)
{
    const Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat *contract =
        dm1_v1_mirror_candidate_no_pending_resurrect_contract_pc34_compat();

    CHECK_REDMCSB(contract != NULL,
                  "contract accessor returns the no-pending resurrect slice",
                  "ReDMCSB REVIVE.C F0282:744-806");
    CHECK_REDMCSB(contract->contract_only == 1,
                  "contract_only marks this as a regression contract gate",
                  "ReDMCSB REVIVE.C F0280:272-276");
    CHECK_REDMCSB(contract->g0299_panel_live_at_start == 1,
                  "fixture starts with G0299 mirror panel route live",
                  contract->redmcsb_f0280_anchor);
    CHECK_REDMCSB(contract->g0305_no_pending_at_start == 0,
                  "fixture starts with no G0305 pending candidate",
                  contract->redmcsb_f0280_anchor);
    CHECK_REDMCSB(contract->resurrect_action_invoked_with_no_pending == 1,
                  "resurrect action is dispatched while no candidate is pending",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(contract->g0299_unchanged_after_no_op == 1,
                  "no-pending resurrect does not close G0299",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(contract->g0305_unchanged_after_no_op == 1,
                  "no-pending resurrect does not change G0305",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(contract->f0282_not_invoked == 1,
                  "no-pending resurrect does not invoke F0282",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(contract->champion_state_unchanged == 1,
                  "no-pending resurrect does not re-arm a champion",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(contract->c040_panel_state_unchanged == 1,
                  "no-pending resurrect leaves C040 panel state unchanged",
                  contract->redmcsb_command_2159_anchor);
    CHECK_REDMCSB(contract->inventory_state_unchanged == 1,
                  "no-pending resurrect leaves inventory state unchanged",
                  contract->redmcsb_command_2159_anchor);
    CHECK_REDMCSB(contract->resurrect_click_consumed == 0,
                  "no-pending resurrect does not consume the click",
                  contract->redmcsb_command_2302_anchor);
}

static void test_required_phrases(void)
{
    const Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat *contract =
        dm1_v1_mirror_candidate_no_pending_resurrect_contract_pc34_compat();

    CHECK_REDMCSB(has_phrase(contract->non_op_note, "contract_only=1") &&
                      has_phrase(contract->source_summary, "contract_only=1"),
                  "notes include contract_only=1",
                  "ReDMCSB REVIVE.C F0280:272-276");
    CHECK_REDMCSB(has_phrase(contract->non_op_note, "REVIVE.C F0280") &&
                      has_phrase(contract->source_summary, "REVIVE.C F0280"),
                  "notes cite REVIVE.C F0280",
                  contract->redmcsb_f0280_anchor);
    CHECK_REDMCSB(has_phrase(contract->non_op_note,
                             "REVIVE.C F0282:744-806") &&
                      has_phrase(contract->source_summary,
                                 "REVIVE.C F0282:744-806"),
                  "notes cite REVIVE.C F0282:744-806",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(has_phrase(contract->source_summary,
                             "COMMAND.C:2159-2181"),
                  "summary cites COMMAND.C:2159-2181",
                  contract->redmcsb_command_2159_anchor);
    CHECK_REDMCSB(has_phrase(contract->source_summary,
                             "COMMAND.C:2302-2311"),
                  "summary cites COMMAND.C:2302-2311",
                  contract->redmcsb_command_2302_anchor);
    CHECK_REDMCSB(has_phrase(contract->non_op_note, "G0299 unchanged") &&
                      has_phrase(contract->source_summary, "G0299 unchanged"),
                  "notes record G0299 unchanged",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(has_phrase(contract->non_op_note, "G0305 unchanged") &&
                      has_phrase(contract->source_summary, "G0305 unchanged"),
                  "notes record G0305 unchanged",
                  contract->redmcsb_f0282_anchor);
    CHECK_REDMCSB(has_phrase(contract->non_op_note, "F0282 not invoked") &&
                      has_phrase(contract->source_summary,
                                 "F0282 not invoked"),
                  "notes record F0282 not invoked",
                  contract->redmcsb_f0282_anchor);
}

static void test_anchor_strings(void)
{
    const Dm1V1MirrorCandidateNoPendingResurrectContractPc34Compat *contract =
        dm1_v1_mirror_candidate_no_pending_resurrect_contract_pc34_compat();

    CHECK_REDMCSB(has_phrase(contract->redmcsb_f0280_anchor,
                             "REVIVE.C F0280:272-276"),
                  "F0280 anchor cites exact publish lines",
                  "ReDMCSB REVIVE.C F0280:272-276");
    CHECK_REDMCSB(has_phrase(contract->redmcsb_f0282_anchor,
                             "REVIVE.C F0282:744-806"),
                  "F0282 anchor cites exact no-op/cleanup range",
                  "ReDMCSB REVIVE.C F0282:744-806");
    CHECK_REDMCSB(has_phrase(contract->redmcsb_command_2159_anchor,
                             "COMMAND.C:2159-2181"),
                  "COMMAND 2159 anchor cites status/inventory gate",
                  "ReDMCSB COMMAND.C:2159-2181");
    CHECK_REDMCSB(has_phrase(contract->redmcsb_command_2302_anchor,
                             "COMMAND.C:2302-2311"),
                  "COMMAND 2302 anchor cites spell/action gate",
                  "ReDMCSB COMMAND.C:2302-2311");
}

static void test_steps(void)
{
    Dm1V1MirrorCandidateNoPendingResurrectStepPc34Compat steps[8];
    size_t count =
        dm1_v1_mirror_candidate_no_pending_resurrect_steps_pc34_compat(
            steps, sizeof(steps) / sizeof(steps[0]));

    CHECK_REDMCSB(count == 6,
                  "step helper reports the six no-op regression checkpoints",
                  "ReDMCSB REVIVE.C F0282:744-806");
    CHECK_REDMCSB(steps[0].id == STEP_BEGIN_G0299_LIVE_G0305_ZERO &&
                      strcmp(steps[0].name,
                             "STEP_BEGIN_G0299_LIVE_G0305_ZERO") == 0,
                  "step 0 records G0299 live and G0305 zero",
                  steps[0].redmcsb_anchor);
    CHECK_REDMCSB(steps[1].id == STEP_RESURRECT_CLICK_DISPATCHED &&
                      strcmp(steps[1].name,
                             "STEP_RESURRECT_CLICK_DISPATCHED") == 0,
                  "step 1 records resurrect click dispatch",
                  steps[1].redmcsb_anchor);
    CHECK_REDMCSB(steps[2].id == STEP_GATE_NO_CANDIDATE_BLOCKS_F0282 &&
                      strcmp(steps[2].name,
                             "STEP_GATE_NO_CANDIDATE_BLOCKS_F0282") == 0,
                  "step 2 records the no-candidate F0282 block",
                  steps[2].redmcsb_anchor);
    CHECK_REDMCSB(steps[3].id == STEP_F0282_NOT_INVOKED &&
                      strcmp(steps[3].name,
                             "STEP_F0282_NOT_INVOKED") == 0,
                  "step 3 records F0282 not invoked",
                  steps[3].redmcsb_anchor);
    CHECK_REDMCSB(steps[4].id == STEP_ASSERT_G0299_G0305_UNCHANGED &&
                      strcmp(steps[4].name,
                             "STEP_ASSERT_G0299_G0305_UNCHANGED") == 0,
                  "step 4 records G0299/G0305 unchanged assertions",
                  steps[4].redmcsb_anchor);
    CHECK_REDMCSB(steps[5].id ==
                      STEP_ASSERT_CHAMPION_INVENTORY_PANEL_UNCHANGED &&
                      strcmp(steps[5].name,
                             "STEP_ASSERT_CHAMPION_INVENTORY_PANEL_UNCHANGED")
                          == 0,
                  "step 5 records champion, inventory, and panel preservation",
                  steps[5].redmcsb_anchor);
}

int main(void)
{
    test_contract_fields();
    test_required_phrases();
    test_anchor_strings();
    test_steps();

    printf("assertions=%d\n", g_assertions);
    if (g_failures) {
        printf("FAIL dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat "
               "failures=%d\n",
               g_failures);
    } else {
        printf("PASS dm1_v1_mirror_candidate_no_pending_resurrect_pc34_compat\n");
    }
    return g_failures == 0 ? 0 : 1;
}
