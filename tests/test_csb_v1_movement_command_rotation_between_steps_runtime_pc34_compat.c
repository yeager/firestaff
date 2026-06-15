#include "csb/csb_v1_movement_command_rotation_between_steps_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, msg) do { \
    int got_value = (int)(got); \
    int want_value = (int)(want); \
    if (got_value == want_value) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s got=%d want=%d\n", msg, got_value, want_value); } \
} while (0)

#define CHECK_NE(got, unwanted, msg) do { \
    int got_value = (int)(got); \
    int unwanted_value = (int)(unwanted); \
    if (got_value != unwanted_value) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s got unwanted=%d\n", msg, got_value); } \
} while (0)

static CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec make_spec(void)
{
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec spec;
    spec.base_party_x = 12;
    spec.base_party_y = 14;
    spec.initial_party_dir = CSB_V1_DIR_NORTH;
    spec.champion_count = CSB_V1_MAX_CHAMPIONS;
    return spec;
}

static void test_source_evidence(void)
{
    const char *evidence =
        csb_v1_movement_command_rotation_between_steps_source_evidence_pc34();

    CHECK(evidence != NULL, "source evidence string is present");
    CHECK(strstr(evidence, "COMMAND.C F0380 lines 2075-2156") != NULL,
          "source evidence cites queue dispatch");
    CHECK(strstr(evidence, "COMMAND.C F0359 lines 1452-1661") != NULL,
          "source evidence cites mouse queue write");
    CHECK(strstr(evidence, "COMMAND.C F0361 lines 1709-1813") != NULL,
          "source evidence cites keyboard queue write");
    CHECK(strstr(evidence, "CLIKMENU.C F0366 lines 224-351") != NULL,
          "source evidence cites movement command handling");
    CHECK(strstr(evidence, "DUNGEON.C F0150 lines 1389-1391") != NULL,
          "source evidence cites party-forward coordinate delta");
    CHECK(strstr(evidence, "DUNGEON.C F0149") != NULL,
          "source evidence cites try-move party position update");
    CHECK(strstr(evidence, "DUNGEON.C F0151") != NULL,
          "source evidence cites party direction rotation");
    CHECK(strstr(evidence, "DUNGEON.C F0163 lines 1769-1838") != NULL,
          "source evidence cites square refresh");
    CHECK(strstr(evidence, "MOVESENS.C F0267 lines 316-328") != NULL,
          "source evidence cites movement sensor boundary");
    CHECK(strstr(evidence, "CHAMPION.C F0284 lines 93-130") != NULL,
          "source evidence cites champion rotation cascade");
    CHECK(strstr(evidence, "CHAMPION.C F0287 lines 243-268") != NULL,
          "source evidence cites leader-hand boundary");
    CHECK(strstr(evidence, "DEFS.H C30") != NULL,
          "source evidence cites champion Cell/Direction offsets");
}

static void check_champion_snapshots(
    const CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result *result)
{
    int i;

    for (i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
        CHECK_EQ(result->champions_after_step1[i].cell, i,
                 "step 1 keeps imported champion Cell before rotation");
        CHECK_EQ(result->champions_after_step1[i].direction, CSB_V1_DIR_NORTH,
                 "step 1 keeps imported champion Direction north");
        CHECK_EQ(result->champions_after_rotation[i].cell, (i + 1) & 3,
                 "C002 rotation advances imported champion Cell");
        CHECK_EQ(result->champions_after_rotation[i].direction, CSB_V1_DIR_EAST,
                 "C002 rotation advances imported champion Direction east");
        CHECK_EQ(result->champions_after_step2[i].cell, (i + 1) & 3,
                 "step 2 preserves post-rotation imported champion Cell");
        CHECK_EQ(result->champions_after_step2[i].direction, CSB_V1_DIR_EAST,
                 "step 2 preserves post-rotation imported champion Direction");
    }
}

static void test_forward_rotate_forward_uses_post_rotation_facing(void)
{
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec spec = make_spec();
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result result;

    CHECK_EQ(csb_v1_movement_command_rotation_between_steps_simulate_pc34(
                 &spec, &result),
             0,
             "forward-turn-forward simulation succeeds");

    CHECK_EQ(result.first_forward_queued, 1, "first forward key queues");
    CHECK_EQ(result.first_forward_processed, 1, "first forward dispatches");
    CHECK_EQ(result.step1_queue_result.dequeued, 1, "step 1 dequeues one command");
    CHECK_EQ(result.step1_queue_result.command, DM1_V1_COMMAND_MOVE_FORWARD,
             "step 1 command is C003 move-forward");
    CHECK_EQ(result.step1_queue_result.dispatchedMove, 1,
             "step 1 is classified as movement");
    CHECK_EQ(result.step1_result.step_attempted, 1, "step 1 attempts movement");
    CHECK_EQ(result.step1_result.step_applied, 1, "step 1 applies movement");
    CHECK_EQ(result.step1_result.destination_x, spec.base_party_x,
             "step 1 north destination x is unchanged");
    CHECK_EQ(result.step1_result.destination_y, spec.base_party_y - 1,
             "step 1 north destination y decrements");
    CHECK_EQ(result.party_x_after_step1, spec.base_party_x,
             "party x after step 1 remains base");
    CHECK_EQ(result.party_y_after_step1, spec.base_party_y - 1,
             "party y after step 1 moves north");
    CHECK_EQ(result.party_dir_after_step1, CSB_V1_DIR_NORTH,
             "party direction after step 1 remains north");
    CHECK_EQ(result.queue_count_after_step1, 0,
             "queue count after step 1 is empty before rotation command");
    CHECK_EQ(result.queue_empty_after_step1, 1,
             "queue empty flag after step 1 is true");

    CHECK_EQ(result.turn_right_queued, 1, "turn-right key queues between steps");
    CHECK_EQ(result.turn_right_processed, 1, "turn-right dispatches between steps");
    CHECK_EQ(result.turn_queue_result.dequeued, 1, "turn dequeues one command");
    CHECK_EQ(result.turn_queue_result.command, DM1_V1_COMMAND_TURN_RIGHT,
             "turn command is C002 turn-right");
    CHECK_EQ(result.turn_queue_result.dispatchedTurn, 1,
             "turn command is classified as rotation");
    CHECK_EQ(result.turn_result.old_party_dir, CSB_V1_DIR_NORTH,
             "turn starts from north");
    CHECK_EQ(result.turn_result.new_party_dir, CSB_V1_DIR_EAST,
             "turn mutates party_dir to east");
    CHECK_EQ(result.turn_result.runtime_state_changed, 1,
             "turn reports runtime state change");
    CHECK_EQ(result.party_x_after_rotation, spec.base_party_x,
             "turn does not change party x");
    CHECK_EQ(result.party_y_after_rotation, spec.base_party_y - 1,
             "turn does not change party y");
    CHECK_EQ(result.party_dir_after_rotation, CSB_V1_DIR_EAST,
             "party direction after rotation is east");
    CHECK_EQ(result.queue_count_after_rotation, 0,
             "queue count after rotation is empty before second forward");
    CHECK_EQ(result.queue_empty_after_rotation, 1,
             "queue empty flag after rotation is true");

    CHECK_EQ(result.second_forward_queued, 1, "second forward key queues");
    CHECK_EQ(result.second_forward_processed, 1, "second forward dispatches");
    CHECK_EQ(result.step2_queue_result.dequeued, 1, "step 2 dequeues one command");
    CHECK_EQ(result.step2_queue_result.command, DM1_V1_COMMAND_MOVE_FORWARD,
             "step 2 command is C003 move-forward");
    CHECK_EQ(result.step2_queue_result.dispatchedMove, 1,
             "step 2 is classified as movement");
    CHECK_EQ(result.step2_result.step_attempted, 1, "step 2 attempts movement");
    CHECK_EQ(result.step2_result.step_applied, 1, "step 2 applies movement");
    CHECK_EQ(result.step2_result.old_party_dir, CSB_V1_DIR_EAST,
             "step 2 sees the post-rotation facing");
    CHECK_EQ(result.step2_result.destination_x, spec.base_party_x + 1,
             "step 2 east destination x increments");
    CHECK_EQ(result.step2_result.destination_y, spec.base_party_y - 1,
             "step 2 east destination y stays after north step");
    CHECK_EQ(result.party_x_after_step2, spec.base_party_x + 1,
             "party x after step 2 moves east");
    CHECK_EQ(result.party_y_after_step2, spec.base_party_y - 1,
             "party y after step 2 remains one north of base");
    CHECK_EQ(result.party_dir_after_step2, CSB_V1_DIR_EAST,
             "party direction after step 2 remains east");
    CHECK_EQ(result.queue_count_after_step2, 0,
             "queue count after step 2 is empty");
    CHECK_EQ(result.queue_empty_after_step2, 1,
             "queue empty flag after step 2 is true");
    CHECK_EQ(result.step2_result.party_state_changed, 1,
             "step 2 reports party coordinate mutation");
    CHECK_EQ(result.step2_result.disabled_movement_ticks_after, 1,
             "step 2 records movement timing gate");

    check_champion_snapshots(&result);
}

static void test_second_step_does_not_continue_north(void)
{
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec spec = make_spec();
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result result;

    CHECK_EQ(csb_v1_movement_command_rotation_between_steps_simulate_pc34(
                 &spec, &result),
             0,
             "negative-path simulation succeeds");

    CHECK_NE(result.party_dir_after_step2, CSB_V1_DIR_NORTH,
             "party direction after step 2 is not north");
    CHECK_NE(result.step2_result.old_party_dir, CSB_V1_DIR_NORTH,
             "step 2 did not dispatch with stale north facing");
    CHECK_NE(result.step2_result.destination_y, spec.base_party_y - 2,
             "step 2 did not choose a second north destination");
    CHECK_EQ(result.party_dir_after_step2, CSB_V1_DIR_EAST,
             "negative path confirms step 2 remains east-facing");
    CHECK_EQ(result.party_x_after_step2, spec.base_party_x + 1,
             "negative path confirms east movement changed x");
    CHECK_EQ(result.party_y_after_step2, spec.base_party_y - 1,
             "negative path confirms y did not move north twice");
    CHECK_EQ(result.step2_queue_result.command, DM1_V1_COMMAND_MOVE_FORWARD,
             "negative path still dispatched a forward command");
    CHECK_EQ(result.turn_queue_result.command, DM1_V1_COMMAND_TURN_RIGHT,
             "negative path turn boundary was C002");
    CHECK_EQ(result.queue_empty_after_step2, 1,
             "negative path leaves no queued command collapsed behind step 2");
}

static void test_invalid_specs_are_rejected(void)
{
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec spec = make_spec();
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result result;

    CHECK_EQ(csb_v1_movement_command_rotation_between_steps_simulate_pc34(
                 NULL, &result),
             -1,
             "NULL spec is rejected");
    CHECK_EQ(csb_v1_movement_command_rotation_between_steps_simulate_pc34(
                 &spec, NULL),
             -1,
             "NULL result is rejected");
    spec.champion_count = 0;
    CHECK_EQ(csb_v1_movement_command_rotation_between_steps_simulate_pc34(
                 &spec, &result),
             -1,
             "zero champions are rejected");
    spec.champion_count = CSB_V1_MAX_CHAMPIONS + 1;
    CHECK_EQ(csb_v1_movement_command_rotation_between_steps_simulate_pc34(
                 &spec, &result),
             -1,
             "too many champions are rejected");
}

int main(void)
{
    printf("=== CSB V1 Movement Command Rotation Between Steps Runtime Gate ===\n\n");
    test_source_evidence();
    test_forward_rotate_forward_uses_post_rotation_facing();
    test_second_step_does_not_continue_north();
    test_invalid_specs_are_rejected();
    CHECK(passed + failed >= 80,
          "regression keeps at least 80 assertions active");
    printf("\nPASSED: %d\nFAILED: %d\nASSERTIONS: %d\n", passed, failed, passed + failed);
    if (failed == 0) {
        puts("all PASS: queued forward, turn-right, queued forward dispatches use the rotated CSB V1 party_dir and preserve imported champion Cell/Direction snapshots");
        puts("sourceEvidence=ReDMCSB COMMAND.C F0380 lines 2075-2156; COMMAND.C F0359 lines 1452-1661; COMMAND.C F0361 lines 1709-1813; CLIKMENU.C F0366 lines 224-351; DUNGEON.C F0150 lines 1389-1391; DUNGEON.C F0149; DUNGEON.C F0151; DUNGEON.C F0163 lines 1769-1838; MOVESENS.C F0267 lines 316-328; CHAMPION.C F0284 lines 93-130; CHAMPION.C F0287 lines 243-268; DEFS.H C30");
    }
    return failed == 0 ? 0 : 1;
}
