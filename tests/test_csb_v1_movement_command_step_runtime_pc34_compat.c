#include "csb_v1_movement_command_step_runtime_pc34_compat.h"

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

typedef struct {
    int wall_x;
    int wall_y;
    int calls;
} WallProbeContext;

static int test_wall_probe(const CSB_V1_RuntimeProfile *profile,
                           int map_x,
                           int map_y,
                           void *context)
{
    WallProbeContext *wall = (WallProbeContext *)context;
    (void)profile;
    wall->calls++;
    return map_x == wall->wall_x && map_y == wall->wall_y;
}

static void seed_profile(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_PartyState *party,
                         int x,
                         int y,
                         int dir)
{
    int i;
    csb_v1_runtime_init(profile, NULL);
    csb_v1_character_init_default(party);
    party->ChampionCount = CSB_V1_MAX_CHAMPIONS;
    party->ImportedFromDM1 = 1;
    party->PartyDirection = dir;
    party->LeaderIndex = 0;
    party->MagicCasterIndex = -1;
    party->PartyMapX = x;
    party->PartyMapY = y;
    for (i = 0; i < party->ChampionCount; ++i) {
        party->Champions[i].CurrentHealth = (int16_t)(90 + i);
        party->Champions[i].MaximumHealth = (int16_t)(120 + i);
        party->Champions[i].Cell = (uint8_t)i;
        party->Champions[i].Direction = (uint8_t)((dir + i) & 3);
    }
    CHECK_EQ(csb_v1_runtime_set_party_state(profile, party), 0,
             "fixture party enters CSB runtime profile");
    profile->party_x = x;
    profile->party_y = y;
    profile->party_dir = dir & 3;
    profile->party_state.PartyMapX = x;
    profile->party_state.PartyMapY = y;
    profile->party_state.PartyDirection = dir & 3;
}

static void enqueue_forward(struct Dm1V1InputCommandQueuePc34Compat *queue,
                            const char *label)
{
    CHECK_EQ(DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
                 queue,
                 (struct Dm1V1InputEventPc34Compat){
                     DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }),
             1,
             label);
}

static void test_source_evidence(void)
{
    const char *evidence =
        csb_v1_movement_command_step_runtime_source_evidence_pc34_compat();
    CHECK(evidence != NULL, "source evidence string is present");
    CHECK(strstr(evidence, "COMMAND.C F0380") != NULL,
          "source evidence cites command queue dispatch");
    CHECK(strstr(evidence, "CLIKMENU.C F0366") != NULL,
          "source evidence cites move-party step logic");
    CHECK(strstr(evidence, "DUNGEON.C F0150") != NULL,
          "source evidence cites relative coordinate update");
    CHECK(strstr(evidence, "MOVESENS.C F0267") != NULL,
          "source evidence cites move-result entrypoint");
    CHECK(strstr(evidence, "CHAMPION.C F0284") != NULL,
          "source evidence cites imported champion Cell/Direction rotation");
}

static void test_one_forward_step_through_queue(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat result;
    int i;

    seed_profile(&profile, &party, 10, 10, CSB_V1_DIR_NORTH);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    enqueue_forward(&queue, "PC-34 forward key queues one V1 command");
    CHECK_EQ(queue.count, 1, "queue contains one command before dispatch");

    CHECK_EQ(csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
                 &profile, &queue, 0, 0, 0, NULL, NULL, &result),
             1,
             "CSB step runtime processes one queued command");
    CHECK_EQ(result.queue_result.dequeued, 1, "queued step was dequeued");
    CHECK_EQ(result.queue_result.command, DM1_V1_COMMAND_MOVE_FORWARD,
             "dequeued command is C003 move-forward");
    CHECK_EQ(result.queue_result.dispatchedMove, 1,
             "shared queue classifies command as movement");
    CHECK_EQ(result.command_handled, 1, "runtime step command is handled");
    CHECK_EQ(result.step_attempted, 1, "runtime attempted a step");
    CHECK_EQ(result.step_applied, 1, "runtime applied the step");
    CHECK_EQ(result.blocked_by_wall, 0, "open destination is not wall-blocked");
    CHECK_EQ(result.wall_probe_called, 0, "no wall probe means open synthetic path");
    CHECK_EQ(result.old_party_y, 10, "result captures old party y");
    CHECK_EQ(result.destination_x, 10, "north forward destination x is unchanged");
    CHECK_EQ(result.destination_y, 9, "north forward destination y advances one cell");
    CHECK_EQ(result.new_party_x, 10, "result captures new party x");
    CHECK_EQ(result.new_party_y, 9, "result captures new party y");
    CHECK_EQ(profile.party_x, 10, "runtime party x follows destination");
    CHECK_EQ(profile.party_y, 9, "runtime party y follows destination");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_NORTH, "runtime facing remains north");
    CHECK_EQ(profile.party_state.PartyMapX, 10, "imported party map x mirrors runtime");
    CHECK_EQ(profile.party_state.PartyMapY, 9, "imported party map y mirrors runtime");
    CHECK_EQ(profile.party_state.PartyDirection, CSB_V1_DIR_NORTH,
             "imported party direction mirrors runtime");
    CHECK_EQ(result.party_state_changed, 1, "result reports coordinate mutation");
    CHECK_EQ(result.disabled_movement_ticks_after, 1,
             "step reports one unresolved movement tick");
    CHECK_EQ(queue.count, 0, "queue is empty after one consumed command");

    for (i = 0; i < profile.party_state.ChampionCount; ++i) {
        CHECK_EQ(profile.party_state.Champions[i].Cell, i,
                 "forward step leaves imported champion Cell stable");
        CHECK_EQ(profile.party_state.Champions[i].Direction, i & 3,
                 "forward step leaves imported champion Direction stable");
    }
}

static void test_forward_direction_matrix(void)
{
    static const int expected_x[4] = { 12, 13, 12, 11 };
    static const int expected_y[4] = { 11, 12, 13, 12 };
    int dir;

    for (dir = CSB_V1_DIR_NORTH; dir <= CSB_V1_DIR_WEST; ++dir) {
        CSB_V1_RuntimeProfile profile;
        CSB_V1_PartyState party;
        struct Dm1V1InputCommandQueuePc34Compat queue;
        CSB_V1_MovementCommandStepRuntimeResultPc34Compat result;

        seed_profile(&profile, &party, 12, 12, dir);
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        enqueue_forward(&queue, "matrix forward command queues");
        CHECK_EQ(csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
                     &profile, &queue, 0, 0, 0, NULL, NULL, &result),
                 1,
                 "matrix forward command dispatches");
        CHECK_EQ(result.queue_result.command, DM1_V1_COMMAND_MOVE_FORWARD,
                 "matrix preserves forward command id");
        CHECK_EQ(result.step_applied, 1, "matrix step applies");
        CHECK_EQ(profile.party_x, expected_x[dir],
                 "matrix x advances along facing direction");
        CHECK_EQ(profile.party_y, expected_y[dir],
                 "matrix y advances along facing direction");
        CHECK_EQ(profile.party_dir, dir, "matrix facing is unchanged by stepping");
        CHECK_EQ(profile.party_state.PartyDirection, dir,
                 "matrix imported party direction stays consistent");
    }
}

static void test_wall_collision_does_not_advance(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat result;
    WallProbeContext wall = { 7, 6, 0 };

    seed_profile(&profile, &party, 7, 7, CSB_V1_DIR_NORTH);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    enqueue_forward(&queue, "wall test forward command queues");

    CHECK_EQ(csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
                 &profile, &queue, 0, 0, 0, test_wall_probe, &wall, &result),
             1,
             "wall test dispatches one queued command");
    CHECK_EQ(result.queue_result.dequeued, 1, "wall test dequeues command");
    CHECK_EQ(result.step_attempted, 1, "wall test attempts movement");
    CHECK_EQ(result.wall_probe_called, 1, "wall test calls wall probe");
    CHECK_EQ(wall.calls, 1, "wall probe called once");
    CHECK_EQ(result.destination_x, 7, "wall destination x is in front");
    CHECK_EQ(result.destination_y, 6, "wall destination y is in front");
    CHECK_EQ(result.blocked_by_wall, 1, "wall collision is reported");
    CHECK_EQ(result.step_applied, 0, "wall collision does not apply step");
    CHECK_EQ(result.party_state_changed, 0, "wall collision reports no state mutation");
    CHECK_EQ(profile.party_x, 7, "wall collision leaves runtime x unchanged");
    CHECK_EQ(profile.party_y, 7, "wall collision leaves runtime y unchanged");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_NORTH,
             "wall collision leaves facing unchanged");
    CHECK_EQ(profile.party_state.PartyMapX, 7,
             "wall collision leaves imported map x unchanged");
    CHECK_EQ(profile.party_state.PartyMapY, 7,
             "wall collision leaves imported map y unchanged");
    CHECK_EQ(profile.party_state.Champions[0].Cell, 0,
             "wall collision leaves champion Cell unchanged");
    CHECK_EQ(profile.party_state.Champions[0].Direction, CSB_V1_DIR_NORTH,
             "wall collision leaves champion Direction unchanged");
    CHECK_EQ(queue.count, 0,
             "wall collision consumed the already-dispatched command");
}

static void test_second_step_waits_for_first_resolution(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat result;

    seed_profile(&profile, &party, 20, 20, CSB_V1_DIR_NORTH);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    enqueue_forward(&queue, "first forward command queues");
    enqueue_forward(&queue, "second forward command queues");
    CHECK_EQ(queue.count, 2, "two commands are queued before dispatch");

    CHECK_EQ(csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
                 &profile, &queue, 0, 0, 0, NULL, NULL, &result),
             1,
             "first queued step dispatches");
    CHECK_EQ(result.step_applied, 1, "first queued step applies");
    CHECK_EQ(profile.party_x, 20, "first step x unchanged while facing north");
    CHECK_EQ(profile.party_y, 19, "first step advances one cell north");
    CHECK_EQ(queue.count, 1, "second command remains queued after first dispatch");
    CHECK_EQ(result.disabled_movement_ticks_after, 1,
             "first step reports unresolved movement timing");

    CHECK_EQ(csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
                 &profile, &queue, 1, 0, 0, NULL, NULL, &result),
             0,
             "movement-disabled gate blocks same-cycle second dispatch");
    CHECK_EQ(result.queue_result.dequeued, 0,
             "same-cycle second step is not dequeued");
    CHECK_EQ(result.queue_result.movementDisabledGate, 1,
             "same-cycle second step hits movement-disabled gate");
    CHECK_EQ(result.step_applied, 0, "same-cycle second step does not apply");
    CHECK_EQ(profile.party_y, 19, "same-cycle gate leaves party position unchanged");
    CHECK_EQ(queue.count, 1, "second command remains queued while first resolves");

    CHECK_EQ(csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
                 &profile, &queue, 0, 0, 0, NULL, NULL, &result),
             1,
             "second queued step dispatches after first resolves");
    CHECK_EQ(result.queue_result.dequeued, 1,
             "resolved second step is dequeued");
    CHECK_EQ(result.step_applied, 1, "resolved second step applies");
    CHECK_EQ(profile.party_x, 20, "second step x remains aligned to north");
    CHECK_EQ(profile.party_y, 18, "second step advances one more cell north");
    CHECK_EQ(profile.party_state.PartyMapY, 18,
             "imported party map y mirrors second step");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_NORTH,
             "second step keeps facing unchanged");
    CHECK_EQ(queue.count, 0, "queue is empty after second resolved step");
}

int main(void)
{
    printf("=== CSB V1 Movement Command Step Runtime Gate ===\n\n");
    test_source_evidence();
    test_one_forward_step_through_queue();
    test_forward_direction_matrix();
    test_wall_collision_does_not_advance();
    test_second_step_waits_for_first_resolution();
    printf("\nPASSED: %d\nFAILED: %d\nASSERTIONS: %d\n", passed, failed, passed + failed);
    if (failed == 0) {
        puts("ok: CSB V1 movement command step runtime applies one queued forward step, preserves facing/champion Cell/Direction invariants, blocks walls, and waits for the first step to resolve before dispatching a second queued step");
        puts("sourceEvidence=ReDMCSB COMMAND.C F0380 lines 2075-2127,2154-2156; CLIKMENU.C F0366 lines 224-233,269,277-304,325-351; DUNGEON.C F0150 lines 1389-1391; MOVESENS.C F0267 lines 316-328; CHAMPION.C F0284 lines 117-130");
    }
    return failed == 0 ? 0 : 1;
}
