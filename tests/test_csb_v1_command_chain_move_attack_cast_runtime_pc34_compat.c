#include "csb_v1_command_chain_move_attack_cast_runtime_pc34_compat.h"

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

static CSB_V1_CommandChainTargetInfoPc34Compat make_move(int direction)
{
    CSB_V1_CommandChainTargetInfoPc34Compat info;

    memset(&info, 0, sizeof(info));
    info.move_direction = direction;
    info.target_id = -1;
    info.target_alive = 1;
    info.cast_script_id = -1;
    return info;
}

static CSB_V1_CommandChainTargetInfoPc34Compat make_attack(
    int target_id,
    int target_alive,
    int cooldown_ticks)
{
    CSB_V1_CommandChainTargetInfoPc34Compat info;

    memset(&info, 0, sizeof(info));
    info.target_id = target_id;
    info.target_alive = target_alive;
    info.attack_action_index = 25;
    info.attack_cooldown_ticks = cooldown_ticks;
    info.cast_script_id = -1;
    return info;
}

static CSB_V1_CommandChainTargetInfoPc34Compat make_cast(
    int script_id,
    int symbol_seed)
{
    CSB_V1_CommandChainTargetInfoPc34Compat info;

    memset(&info, 0, sizeof(info));
    info.target_id = -1;
    info.target_alive = 1;
    info.cast_script_id = script_id;
    info.cast_symbol_seed = symbol_seed;
    return info;
}

static void seed_champion(CSB_V1_CommandChainStatePc34Compat *state,
                          int champion_index,
                          int x,
                          int y,
                          int dir)
{
    state->champions[champion_index].x = x;
    state->champions[champion_index].y = y;
    state->champions[champion_index].dir = dir;
    state->champions[champion_index].chaos_cast_status = 0;
    state->champions[champion_index].chaos_cast_script_id = -1;
    state->champions[champion_index].chaos_cast_symbol_seed = 0;
}

static void check_empty_queues(const CSB_V1_CommandChainStatePc34Compat *state,
                               const char *label)
{
    int i;

    for (i = 0; i < state->party_count; ++i) {
        CHECK_EQ(state->queue_counts[i], 0, label);
    }
}

static void test_source_evidence(void)
{
    const char *evidence =
        csb_v1_command_chain_source_evidence_pc34_compat();

    CHECK(evidence != NULL, "source evidence string is present");
    CHECK(strstr(evidence, "DEFS.H lines 240-243") != NULL,
          "evidence cites command ids for movement");
    CHECK(strstr(evidence, "DEFS.H lines 3263-3264") != NULL,
          "evidence cites PC-34 queue storage size");
    CHECK(strstr(evidence, "C5/C7") != NULL,
          "evidence cites regular and reserved command budgets");
    CHECK(strstr(evidence, "COMMAND.C F0359") != NULL,
          "evidence cites mouse queue writes");
    CHECK(strstr(evidence, "COMMAND.C F0361") != NULL,
          "evidence cites keyboard queue writes");
    CHECK(strstr(evidence, "COMMAND.C F0380") != NULL,
          "evidence cites single command dequeue");
    CHECK(strstr(evidence, "CLIKMENU.C F0366") != NULL,
          "evidence cites move click-menu path");
    CHECK(strstr(evidence, "CLIKMENU.C F0370") != NULL,
          "evidence cites spell cast path");
    CHECK(strstr(evidence, "CLIKMENU.C F0371") != NULL,
          "evidence cites action-area path");
    CHECK(strstr(evidence, "MENU.C F0407") != NULL,
          "evidence cites action cooldown path");
    CHECK(strstr(evidence, "CHAMPION.C F0330") != NULL,
          "evidence cites per-champion action cooldown");
    CHECK_EQ(CSB_V1_COMMAND_CHAIN_SOURCE_QUEUE_STORAGE_PC34, 8,
             "source queue storage remains eight slots");
    CHECK_EQ(CSB_V1_COMMAND_CHAIN_SOURCE_REGULAR_BUDGET_PC34, 5,
             "regular source queue budget remains C5");
    CHECK_EQ(CSB_V1_COMMAND_CHAIN_MAX_CAPACITY_PC34, 5,
             "contract queue clamps to the source regular budget");
}

static void test_move_attack_cast_three_ticks(void)
{
    CSB_V1_CommandChainStatePc34Compat state;
    CSB_V1_CommandChainDispatchPc34Compat dispatch;

    csb_v1_command_chain_init(&state, 1, 3);
    seed_champion(&state, 0, 10, 10, 0);

    CHECK_EQ(state.party_count, 1, "fixture has one champion");
    CHECK_EQ(state.command_queue_capacity, 3, "fixture queue capacity is three");
    CHECK_EQ(state.tick_count, 0, "fixture begins at tick zero");
    CHECK_EQ(state.queue_counts[0], 0, "fixture queue starts empty");
    CHECK_EQ(state.champions[0].x, 10, "fixture champion x starts at ten");
    CHECK_EQ(state.champions[0].y, 10, "fixture champion y starts at ten");
    CHECK_EQ(state.champions[0].dir, 0, "fixture champion faces north");

    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
                 make_move(CSB_V1_COMMAND_CHAIN_MOVE_FORWARD_PC34)), 0,
             "push MOVE succeeds");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
                 make_attack(2, 1, 20)), 0,
             "push ATTACK succeeds");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_CAST_PC34,
                 make_cast(7, 0x42)), 0,
             "push CAST succeeds");

    CHECK_EQ(state.queue_counts[0], 3, "three commands are queued");
    CHECK_EQ(state.queues[0][0].command_type, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "queued command zero is MOVE");
    CHECK_EQ(state.queues[0][1].command_type, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
             "queued command one is ATTACK");
    CHECK_EQ(state.queues[0][2].command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "queued command two is CAST");
    CHECK_EQ(state.queues[0][0].sequence_id, 1, "MOVE sequence id is first");
    CHECK_EQ(state.queues[0][1].sequence_id, 2, "ATTACK sequence id is second");
    CHECK_EQ(state.queues[0][2].sequence_id, 3, "CAST sequence id is third");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.tick_index, 0, "T0 dispatch reports tick zero");
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "T0 dispatch consumes MOVE");
    CHECK_EQ(dispatch.champion_index, 0, "T0 dispatch belongs to champion zero");
    CHECK_EQ(dispatch.sequence_id, 1, "T0 dispatch preserves MOVE identity");
    CHECK_EQ(dispatch.moved, 1, "T0 dispatch applies movement");
    CHECK_EQ(dispatch.old_x, 10, "T0 move starts at x ten");
    CHECK_EQ(dispatch.old_y, 10, "T0 move starts at y ten");
    CHECK_EQ(dispatch.new_x, 10, "T0 north move keeps x");
    CHECK_EQ(dispatch.new_y, 9, "T0 north move advances one cell");
    CHECK_EQ(state.champions[0].x, 10, "T0 champion x is updated");
    CHECK_EQ(state.champions[0].y, 9, "T0 champion y is updated");
    CHECK_EQ(state.queue_counts[0], 2, "T0 frees one queue slot");
    CHECK_EQ(state.queues[0][0].command_type, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
             "ATTACK remains at queue head after MOVE");
    CHECK_EQ(state.queues[0][1].command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "CAST remains second after MOVE");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.tick_index, 1, "T1 dispatch reports tick one");
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
             "T1 dispatch consumes ATTACK");
    CHECK_EQ(dispatch.sequence_id, 2, "T1 dispatch preserves ATTACK identity");
    CHECK_EQ(dispatch.attack_attempted, 1, "T1 attack is attempted");
    CHECK_EQ(dispatch.attack_hit, 1, "T1 attack sees the live target");
    CHECK_EQ(dispatch.attack_failed_target_dead, 0,
             "T1 attack does not report a dead target");
    CHECK_EQ(dispatch.attack_cooldown_ticks, 20,
             "T1 attack sets source MELEE cooldown");
    CHECK_EQ(state.champions[0].attack_attempts, 1,
             "champion attack attempt count increments");
    CHECK_EQ(state.champions[0].attack_hits, 1,
             "champion attack hit count increments");
    CHECK_EQ(state.champions[0].attack_failures, 0,
             "champion attack failure count is still zero");
    CHECK_EQ(state.champions[0].attack_cooldown_ticks, 20,
             "champion action cooldown is stored");
    CHECK_EQ(state.queue_counts[0], 1, "T1 frees a second queue slot");
    CHECK_EQ(state.queues[0][0].command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "CAST remains queued after ATTACK");
    CHECK_EQ(state.queues[0][0].sequence_id, 3,
             "CAST sequence identity survives ATTACK dispatch");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.tick_index, 2, "T2 dispatch reports tick two");
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "T2 dispatch consumes CAST");
    CHECK_EQ(dispatch.sequence_id, 3, "T2 dispatch preserves CAST identity");
    CHECK_EQ(dispatch.cast_started, 1, "T2 starts the cast");
    CHECK_EQ(dispatch.chaos_cast_status, 1,
             "T2 cast enters CSB_V1_CHAOS_CAST_RUNNING value");
    CHECK_EQ(dispatch.cast_script_id, 7,
             "T2 cast keeps the original script id");
    CHECK_EQ(dispatch.cast_symbol_seed, 0x42,
             "T2 cast keeps the original symbol seed");
    CHECK_EQ(state.champions[0].chaos_cast_status, 1,
             "champion cast state is running after T2");
    CHECK_EQ(state.champions[0].chaos_cast_script_id, 7,
             "champion stores cast script id after T2");
    CHECK_EQ(state.champions[0].chaos_cast_symbol_seed, 0x42,
             "champion stores cast symbol seed after T2");
    CHECK_EQ(state.champions[0].casts_started, 1,
             "champion cast counter increments once");
    CHECK_EQ(state.queue_counts[0], 0, "queue is empty after T2");
    CHECK_EQ(state.tick_count, 3, "three ticks elapsed");
    check_empty_queues(&state, "all queues empty after chain completes");
}

static void test_chain_full_rejects_fourth_without_ticks(void)
{
    CSB_V1_CommandChainStatePc34Compat state;

    csb_v1_command_chain_init(&state, 1, 3);
    seed_champion(&state, 0, 4, 4, 1);

    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
                 make_move(CSB_V1_COMMAND_CHAIN_MOVE_RIGHT_PC34)), 0,
             "overflow fixture accepts first command");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
                 make_attack(1, 1, 20)), 0,
             "overflow fixture accepts second command");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_CAST_PC34,
                 make_cast(11, 0x11)), 0,
             "overflow fixture accepts third command");
    CHECK_EQ(state.queue_counts[0], 3, "overflow fixture queue is full");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
                 make_move(CSB_V1_COMMAND_CHAIN_MOVE_LEFT_PC34)), -1,
             "fourth command is rejected before any tick frees a slot");
    CHECK_EQ(state.overflow_count, 1, "overflow attempt increments counter");
    CHECK_EQ(state.queue_counts[0], 3, "overflow does not change queue count");
    CHECK_EQ(state.queues[0][0].command_type, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "overflow preserves first queued command");
    CHECK_EQ(state.queues[0][1].command_type, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
             "overflow preserves second queued command");
    CHECK_EQ(state.queues[0][2].command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "overflow preserves third queued command");
    CHECK_EQ(state.tick_count, 0, "overflow test has not advanced time");

    CHECK_EQ(csb_v1_command_chain_tick(&state).command_type,
             CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "first tick after overflow still dispatches MOVE");
    CHECK_EQ(state.queue_counts[0], 2, "dispatch frees a slot after overflow");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
                 make_move(CSB_V1_COMMAND_CHAIN_MOVE_LEFT_PC34)), 0,
             "queue accepts another command after dispatch frees headroom");
    CHECK_EQ(state.queue_counts[0], 3,
             "queue returns to full after refilling freed slot");
}

static void test_attack_failure_does_not_abort_cast(void)
{
    CSB_V1_CommandChainStatePc34Compat state;
    CSB_V1_CommandChainDispatchPc34Compat dispatch;

    csb_v1_command_chain_init(&state, 1, 3);
    seed_champion(&state, 0, 8, 8, 0);

    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
                 make_move(CSB_V1_COMMAND_CHAIN_MOVE_FORWARD_PC34)), 0,
             "failure fixture pushes MOVE");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
                 make_attack(3, 1, 20)), 0,
             "failure fixture pushes ATTACK with live target snapshot");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_CAST_PC34,
                 make_cast(13, 0x7a)), 0,
             "failure fixture pushes CAST with original chaos state");
    CHECK_EQ(state.target_alive[3], 1, "target starts alive at enqueue time");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "failure fixture T0 consumes MOVE");
    CHECK_EQ(state.champions[0].y, 7,
             "failure fixture T0 moves champion north");
    CHECK_EQ(state.queue_counts[0], 2,
             "failure fixture keeps ATTACK and CAST queued after T0");

    state.target_alive[3] = 0;
    CHECK_EQ(state.target_alive[3], 0,
             "target dies between T0 and T1");
    CHECK_EQ(state.queues[0][0].command_type, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
             "ATTACK is still queued after target death");
    CHECK_EQ(state.queues[0][1].command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "CAST is still queued after target death");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.tick_index, 1, "failure fixture T1 reports tick one");
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
             "failure fixture T1 consumes ATTACK");
    CHECK_EQ(dispatch.sequence_id, 2,
             "failure fixture T1 preserves ATTACK sequence");
    CHECK_EQ(dispatch.attack_attempted, 1,
             "dead-target attack still dispatches");
    CHECK_EQ(dispatch.attack_hit, 0,
             "dead-target attack does not hit");
    CHECK_EQ(dispatch.attack_failed_target_dead, 1,
             "dead-target attack reports failure");
    CHECK_EQ(dispatch.attack_cooldown_ticks, 10,
             "failed melee cooldown is halved per ReDMCSB action path");
    CHECK_EQ(state.champions[0].attack_attempts, 1,
             "failed attack increments attempts");
    CHECK_EQ(state.champions[0].attack_hits, 0,
             "failed attack does not increment hits");
    CHECK_EQ(state.champions[0].attack_failures, 1,
             "failed attack increments failures");
    CHECK_EQ(state.queue_counts[0], 1,
             "CAST remains queued after failed ATTACK");
    CHECK_EQ(state.queues[0][0].command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "CAST is now at queue head");
    CHECK_EQ(state.queues[0][0].target_info.cast_script_id, 13,
             "CAST retains original script id after attack failure");
    CHECK_EQ(state.queues[0][0].target_info.cast_symbol_seed, 0x7a,
             "CAST retains original symbol seed after attack failure");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.tick_index, 2, "failure fixture T2 reports tick two");
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "failure fixture T2 consumes CAST");
    CHECK_EQ(dispatch.sequence_id, 3,
             "failure fixture T2 preserves CAST sequence");
    CHECK_EQ(dispatch.cast_started, 1,
             "CAST still starts after failed ATTACK");
    CHECK_EQ(dispatch.chaos_cast_status, 1,
             "CAST enters running state after failed ATTACK");
    CHECK_EQ(dispatch.cast_script_id, 13,
             "CAST dispatch uses original script id");
    CHECK_EQ(dispatch.cast_symbol_seed, 0x7a,
             "CAST dispatch uses original symbol seed");
    CHECK_EQ(state.queue_counts[0], 0,
             "failure fixture queue is empty after T2");
    CHECK_EQ(state.tick_count, 3,
             "failure fixture consumed three ticks");
}

static void test_two_champions_do_not_cross_contaminate(void)
{
    CSB_V1_CommandChainStatePc34Compat state;
    CSB_V1_CommandChainDispatchPc34Compat dispatch;

    csb_v1_command_chain_init(&state, 2, 3);
    seed_champion(&state, 0, 20, 20, 0);
    seed_champion(&state, 1, 30, 30, 1);

    CHECK_EQ(state.party_count, 2, "two-champion fixture has two champions");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
                 make_move(CSB_V1_COMMAND_CHAIN_MOVE_FORWARD_PC34)), 0,
             "champion zero queues MOVE");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
                 make_attack(4, 1, 20)), 0,
             "champion zero queues ATTACK");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 1, CSB_V1_COMMAND_CHAIN_CAST_PC34,
                 make_cast(21, 0x21)), 0,
             "champion one queues CAST at the same tick");
    CHECK_EQ(state.queue_counts[0], 2,
             "champion zero owns two queued commands");
    CHECK_EQ(state.queue_counts[1], 1,
             "champion one owns one queued command");
    CHECK_EQ(state.queues[0][0].champion_index, 0,
             "champion zero queue stores champion zero owner");
    CHECK_EQ(state.queues[0][1].champion_index, 0,
             "champion zero second command stores champion zero owner");
    CHECK_EQ(state.queues[1][0].champion_index, 1,
             "champion one queue stores champion one owner");
    CHECK_EQ(state.queues[0][0].sequence_id, 1,
             "champion zero first sequence is one");
    CHECK_EQ(state.queues[0][1].sequence_id, 2,
             "champion zero second sequence is two");
    CHECK_EQ(state.queues[1][0].sequence_id, 3,
             "champion one sequence is three");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "two-champion T0 dispatches champion zero MOVE");
    CHECK_EQ(dispatch.champion_index, 0,
             "two-champion T0 belongs to champion zero");
    CHECK_EQ(dispatch.sequence_id, 1,
             "two-champion T0 preserves champion zero sequence");
    CHECK_EQ(state.champions[0].x, 20, "champion zero x remains twenty");
    CHECK_EQ(state.champions[0].y, 19, "champion zero y moves north");
    CHECK_EQ(state.champions[1].x, 30, "champion one x is untouched by T0");
    CHECK_EQ(state.champions[1].y, 30, "champion one y is untouched by T0");
    CHECK_EQ(state.queue_counts[0], 1,
             "champion zero queue decrements after T0");
    CHECK_EQ(state.queue_counts[1], 1,
             "champion one queue is unchanged after T0");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "two-champion T1 dispatches champion one CAST by round-robin");
    CHECK_EQ(dispatch.champion_index, 1,
             "two-champion T1 belongs to champion one");
    CHECK_EQ(dispatch.sequence_id, 3,
             "two-champion T1 preserves champion one sequence");
    CHECK_EQ(dispatch.cast_script_id, 21,
             "champion one cast script is preserved");
    CHECK_EQ(state.champions[1].chaos_cast_status, 1,
             "champion one cast state is running");
    CHECK_EQ(state.champions[0].chaos_cast_status, 0,
             "champion zero cast state is untouched by champion one");
    CHECK_EQ(state.queue_counts[0], 1,
             "champion zero ATTACK remains queued after champion one CAST");
    CHECK_EQ(state.queue_counts[1], 0,
             "champion one queue is empty after CAST");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
             "two-champion T2 dispatches champion zero ATTACK");
    CHECK_EQ(dispatch.champion_index, 0,
             "two-champion T2 belongs to champion zero");
    CHECK_EQ(dispatch.sequence_id, 2,
             "two-champion T2 preserves champion zero ATTACK sequence");
    CHECK_EQ(dispatch.attack_hit, 1,
             "champion zero attack hits its live target");
    CHECK_EQ(state.champions[0].attack_cooldown_ticks, 20,
             "champion zero attack cooldown is set");
    CHECK_EQ(state.champions[1].attack_cooldown_ticks, 0,
             "champion one attack cooldown remains untouched");
    CHECK_EQ(state.queue_counts[0], 0,
             "champion zero queue is empty after T2");
    CHECK_EQ(state.queue_counts[1], 0,
             "champion one queue remains empty after T2");
    CHECK_EQ(state.tick_count, 3,
             "two-champion fixture has consumed three ticks");
}

static void test_cancel_removes_one_queued_command(void)
{
    CSB_V1_CommandChainStatePc34Compat state;
    CSB_V1_CommandChainDispatchPc34Compat dispatch;

    csb_v1_command_chain_init(&state, 1, 3);
    seed_champion(&state, 0, 12, 12, 0);

    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
                 make_move(CSB_V1_COMMAND_CHAIN_MOVE_FORWARD_PC34)), 0,
             "cancel fixture queues MOVE");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_ATTACK_PC34,
                 make_attack(5, 1, 20)), 0,
             "cancel fixture queues ATTACK");
    CHECK_EQ(csb_v1_command_chain_push(
                 &state, 0, CSB_V1_COMMAND_CHAIN_CAST_PC34,
                 make_cast(31, 0x31)), 0,
             "cancel fixture queues CAST");
    CHECK_EQ(state.queue_counts[0], 3, "cancel fixture queue starts full");
    CHECK_EQ(csb_v1_command_chain_cancel_at(&state, 0, 1), 0,
             "cancel removes the middle ATTACK command");
    CHECK_EQ(state.canceled_count, 1, "cancel counter increments");
    CHECK_EQ(state.queue_counts[0], 2, "cancel reduces queue count");
    CHECK_EQ(state.queues[0][0].command_type, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "cancel preserves first MOVE command");
    CHECK_EQ(state.queues[0][1].command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "cancel shifts CAST into second position");
    CHECK_EQ(state.queues[0][1].sequence_id, 3,
             "cancel preserves shifted CAST sequence id");
    CHECK_EQ(csb_v1_command_chain_cancel_at(&state, 0, 3), -1,
             "cancel rejects out-of-range position");
    CHECK_EQ(csb_v1_command_chain_cancel_at(&state, 1, 0), -1,
             "cancel rejects out-of-range champion");

    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_MOVE_PC34,
             "cancel fixture T0 dispatches MOVE");
    CHECK_EQ(state.queue_counts[0], 1, "cancel fixture leaves one command");
    dispatch = csb_v1_command_chain_tick(&state);
    CHECK_EQ(dispatch.command_type, CSB_V1_COMMAND_CHAIN_CAST_PC34,
             "cancel fixture T1 dispatches CAST after canceled ATTACK");
    CHECK_EQ(dispatch.sequence_id, 3,
             "cancel fixture CAST keeps original identity");
    CHECK_EQ(state.champions[0].attack_attempts, 0,
             "canceled ATTACK never attempts");
    CHECK_EQ(state.champions[0].chaos_cast_status, 1,
             "CAST still starts after cancel");
    CHECK_EQ(state.queue_counts[0], 0,
             "cancel fixture queue is empty after two ticks");
}

int main(void)
{
    printf("=== CSB V1 Command Chain Move Attack Cast Runtime Gate ===\n\n");
    test_source_evidence();
    test_move_attack_cast_three_ticks();
    test_chain_full_rejects_fourth_without_ticks();
    test_attack_failure_does_not_abort_cast();
    test_two_champions_do_not_cross_contaminate();
    test_cancel_removes_one_queued_command();
    printf("\nPASSED: %d\nFAILED: %d\nASSERTIONS: %d\n",
           passed,
           failed,
           passed + failed);
    if (failed == 0) {
        puts("ok: CSB V1 command-chain runtime preserves MOVE, ATTACK, and CAST identity across sequential ticks, keeps per-champion queues isolated, rejects overflow, and continues CAST after a failed ATTACK");
        puts(csb_v1_command_chain_source_evidence_pc34_compat());
    }
    return failed == 0 ? 0 : 1;
}
