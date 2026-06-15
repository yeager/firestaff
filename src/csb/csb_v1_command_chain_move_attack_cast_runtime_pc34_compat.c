#include "csb_v1_command_chain_move_attack_cast_runtime_pc34_compat.h"

#include <string.h>

#define CSB_V1_CHAIN_DIR_NORTH 0
#define CSB_V1_CHAIN_DIR_EAST 1
#define CSB_V1_CHAIN_DIR_SOUTH 2
#define CSB_V1_CHAIN_DIR_WEST 3

#define CSB_V1_CHAIN_CHAOS_CAST_READY 0
#define CSB_V1_CHAIN_CHAOS_CAST_RUNNING 1

static int csb_v1_chain_clamp_capacity(int command_queue_capacity)
{
    if (command_queue_capacity < 1) {
        return 1;
    }
    if (command_queue_capacity > CSB_V1_COMMAND_CHAIN_MAX_CAPACITY_PC34) {
        return CSB_V1_COMMAND_CHAIN_MAX_CAPACITY_PC34;
    }
    return command_queue_capacity;
}

static int csb_v1_chain_valid_command(int command_type)
{
    return command_type == CSB_V1_COMMAND_CHAIN_MOVE_PC34 ||
           command_type == CSB_V1_COMMAND_CHAIN_ATTACK_PC34 ||
           command_type == CSB_V1_COMMAND_CHAIN_CAST_PC34;
}

static void csb_v1_chain_shift_queue(
    CSB_V1_CommandChainStatePc34Compat *state,
    int champion_index,
    int position)
{
    int i;
    int count = state->queue_counts[champion_index];

    for (i = position; i + 1 < count; ++i) {
        state->queues[champion_index][i] =
            state->queues[champion_index][i + 1];
    }
    if (count > 0) {
        memset(&state->queues[champion_index][count - 1],
               0,
               sizeof(state->queues[champion_index][count - 1]));
        state->queue_counts[champion_index]--;
    }
}

static int csb_v1_chain_target_is_alive(
    const CSB_V1_CommandChainStatePc34Compat *state,
    const CSB_V1_CommandChainTargetInfoPc34Compat *target_info)
{
    if (target_info->target_id < 0 ||
        target_info->target_id >= CSB_V1_COMMAND_CHAIN_TARGET_COUNT_PC34) {
        return target_info->target_alive ? 1 : 0;
    }
    return state->target_alive[target_info->target_id] ? 1 : 0;
}

static int csb_v1_chain_attack_cooldown(
    int target_alive,
    const CSB_V1_CommandChainTargetInfoPc34Compat *target_info)
{
    int ticks = target_info->attack_cooldown_ticks;

    if (ticks <= 0) {
        /* ReDMCSB MENU.C F0407 lines 157,183 define MELEE disabled ticks
         * as 20; lines 1331-1344 halve the disabled-action delay when the
         * melee action fails, and CHAMPION.C F0330 lines 2233-2251 records
         * the per-champion enable-action event. */
        ticks = 20;
    }
    return target_alive ? ticks : (ticks >> 1);
}

static void csb_v1_chain_apply_move(
    CSB_V1_CommandChainChampionPc34Compat *champion,
    const CSB_V1_CommandChainTargetInfoPc34Compat *target_info,
    CSB_V1_CommandChainDispatchPc34Compat *dispatch)
{
    static const int east_count[4] = { 0, 1, 0, -1 };
    static const int north_count[4] = { -1, 0, 1, 0 };
    static const int forward_count[4] = { 1, 0, -1, 0 };
    static const int right_count[4] = { 0, 1, 0, -1 };
    int move_direction = target_info->move_direction & 3;
    int dir = champion->dir & 3;

    dispatch->old_x = champion->x;
    dispatch->old_y = champion->y;
    champion->x += east_count[dir] * forward_count[move_direction] +
                   east_count[(dir + 1) & 3] * right_count[move_direction];
    champion->y += north_count[dir] * forward_count[move_direction] +
                   north_count[(dir + 1) & 3] * right_count[move_direction];
    dispatch->new_x = champion->x;
    dispatch->new_y = champion->y;
    dispatch->moved = 1;
}

static void csb_v1_chain_apply_attack(
    CSB_V1_CommandChainStatePc34Compat *state,
    CSB_V1_CommandChainChampionPc34Compat *champion,
    const CSB_V1_CommandChainTargetInfoPc34Compat *target_info,
    CSB_V1_CommandChainDispatchPc34Compat *dispatch)
{
    int alive = csb_v1_chain_target_is_alive(state, target_info);
    int cooldown = csb_v1_chain_attack_cooldown(alive, target_info);

    champion->attack_attempts++;
    champion->attack_cooldown_ticks = cooldown;
    dispatch->attack_attempted = 1;
    dispatch->attack_cooldown_ticks = cooldown;
    if (alive) {
        champion->attack_hits++;
        dispatch->attack_hit = 1;
    } else {
        champion->attack_failures++;
        dispatch->attack_failed_target_dead = 1;
    }
}

static void csb_v1_chain_apply_cast(
    CSB_V1_CommandChainChampionPc34Compat *champion,
    const CSB_V1_CommandChainTargetInfoPc34Compat *target_info,
    CSB_V1_CommandChainDispatchPc34Compat *dispatch)
{
    champion->chaos_cast_status = CSB_V1_CHAIN_CHAOS_CAST_RUNNING;
    champion->chaos_cast_script_id = target_info->cast_script_id;
    champion->chaos_cast_symbol_seed = target_info->cast_symbol_seed;
    champion->casts_started++;
    dispatch->cast_started = 1;
    dispatch->chaos_cast_status = champion->chaos_cast_status;
    dispatch->cast_script_id = champion->chaos_cast_script_id;
    dispatch->cast_symbol_seed = champion->chaos_cast_symbol_seed;
}

void csb_v1_command_chain_init(
    CSB_V1_CommandChainStatePc34Compat *state,
    int party_count,
    int command_queue_capacity)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    if (party_count < 0) {
        party_count = 0;
    }
    if (party_count > CSB_V1_COMMAND_CHAIN_MAX_CHAMPIONS_PC34) {
        party_count = CSB_V1_COMMAND_CHAIN_MAX_CHAMPIONS_PC34;
    }
    state->party_count = party_count;
    state->command_queue_capacity =
        csb_v1_chain_clamp_capacity(command_queue_capacity);
    for (i = 0; i < CSB_V1_COMMAND_CHAIN_TARGET_COUNT_PC34; ++i) {
        state->target_alive[i] = 1;
    }
    for (i = 0; i < state->party_count; ++i) {
        state->champions[i].dir = CSB_V1_CHAIN_DIR_NORTH;
        state->champions[i].chaos_cast_status =
            CSB_V1_CHAIN_CHAOS_CAST_READY;
        state->champions[i].chaos_cast_script_id = -1;
    }
}

int csb_v1_command_chain_push(
    CSB_V1_CommandChainStatePc34Compat *state,
    int champion_index,
    int command_type,
    CSB_V1_CommandChainTargetInfoPc34Compat target_info)
{
    int count;

    if (!state ||
        champion_index < 0 ||
        champion_index >= state->party_count ||
        !csb_v1_chain_valid_command(command_type)) {
        return -1;
    }
    count = state->queue_counts[champion_index];
    if (count >= state->command_queue_capacity) {
        state->overflow_count++;
        return -1;
    }
    if (target_info.target_id >= 0 &&
        target_info.target_id < CSB_V1_COMMAND_CHAIN_TARGET_COUNT_PC34) {
        state->target_alive[target_info.target_id] =
            target_info.target_alive ? 1 : 0;
    }
    state->queues[champion_index][count].command_type = command_type;
    state->queues[champion_index][count].champion_index = champion_index;
    state->queues[champion_index][count].sequence_id =
        ++state->enqueue_sequence;
    state->queues[champion_index][count].enqueue_tick = state->tick_count;
    state->queues[champion_index][count].target_info = target_info;
    state->queue_counts[champion_index]++;
    return 0;
}

CSB_V1_CommandChainDispatchPc34Compat csb_v1_command_chain_tick(
    CSB_V1_CommandChainStatePc34Compat *state)
{
    CSB_V1_CommandChainDispatchPc34Compat dispatch;
    int scanned;

    memset(&dispatch, 0, sizeof(dispatch));
    if (!state) {
        return dispatch;
    }
    dispatch.tick_index = state->tick_count;

    /* Source-lock: ReDMCSB COMMAND.C F0380 lines 2095-2126 copies the
     * command payload, decrements G2153_i_QueuedCommandsCount, then advances
     * the queue before dispatching the command at lines 2154-2156, 2302-2306,
     * and 2308-2312.  CLIKMENU.C F0366/F0370/F0371 then handles MOVE, CAST,
     * and ATTACK identities separately, so this contract gate dequeues one
     * preserved command identity per tick instead of collapsing a chain into
     * one composite action. */
    for (scanned = 0; scanned < state->party_count; ++scanned) {
        int champion_index =
            (state->next_champion_scan + scanned) % state->party_count;
        CSB_V1_CommandChainQueuedCommandPc34Compat command;
        CSB_V1_CommandChainChampionPc34Compat *champion;

        if (state->queue_counts[champion_index] <= 0) {
            continue;
        }
        command = state->queues[champion_index][0];
        csb_v1_chain_shift_queue(state, champion_index, 0);
        champion = &state->champions[champion_index];
        dispatch.command_type = command.command_type;
        dispatch.champion_index = champion_index;
        dispatch.sequence_id = command.sequence_id;
        dispatch.queue_count_after = state->queue_counts[champion_index];
        dispatch.old_x = champion->x;
        dispatch.old_y = champion->y;
        dispatch.new_x = champion->x;
        dispatch.new_y = champion->y;

        switch (command.command_type) {
            case CSB_V1_COMMAND_CHAIN_MOVE_PC34:
                csb_v1_chain_apply_move(champion,
                                        &command.target_info,
                                        &dispatch);
                break;
            case CSB_V1_COMMAND_CHAIN_ATTACK_PC34:
                csb_v1_chain_apply_attack(state,
                                          champion,
                                          &command.target_info,
                                          &dispatch);
                break;
            case CSB_V1_COMMAND_CHAIN_CAST_PC34:
                csb_v1_chain_apply_cast(champion,
                                        &command.target_info,
                                        &dispatch);
                break;
            default:
                break;
        }
        state->next_champion_scan = (champion_index + 1) % state->party_count;
        break;
    }
    state->tick_count++;
    state->last_dispatch = dispatch;
    return dispatch;
}

int csb_v1_command_chain_cancel_at(
    CSB_V1_CommandChainStatePc34Compat *state,
    int champion_index,
    int position)
{
    if (!state ||
        champion_index < 0 ||
        champion_index >= state->party_count ||
        position < 0 ||
        position >= state->queue_counts[champion_index]) {
        return -1;
    }
    csb_v1_chain_shift_queue(state, champion_index, position);
    state->canceled_count++;
    return 0;
}

const char *csb_v1_command_chain_source_evidence_pc34_compat(void)
{
    return "ReDMCSB DEFS.H lines 240-243 MOVE command ids, 308-318 CAST/ACTION command ids; "
           "DEFS.H lines 3263-3264 M529_COMMAND_QUEUE_SIZE=8 and lines 3507-3509 C5/C7 command-count budgets; "
           "COMMAND.C F0359 lines 1506-1514,1628-1660 mouse queue write preserves command/x/y; "
           "COMMAND.C F0361 lines 1744-1768 keyboard queue write preserves one command and C5 headroom; "
           "COMMAND.C F0380 lines 2095-2126 dequeues one command before dispatch, lines 2154-2156 MOVE, lines 2302-2306 CAST, lines 2308-2312 ACTION; "
           "CLIKMENU.C F0366 lines 224-269 maps movement arrows to one-cell destination and lines 325-347 commits one movement step; "
           "CLIKMENU.C F0370 lines 484-497 keeps CAST state in the spell-area command path; "
           "CLIKMENU.C F0371 lines 529-573 keeps ACTION dispatch separate from command dequeue; "
           "MENU.C F0407 lines 1263-1275 seeds action state, lines 1331-1344 halves failed melee delay, lines 1620-1627 applies per-champion action budget; "
           "CHAMPION.C F0330 lines 2233-2251 records per-champion action cooldown.";
}
