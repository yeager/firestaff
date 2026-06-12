#ifndef FIRESTAFF_CSB_V1_COMMAND_CHAIN_MOVE_ATTACK_CAST_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_COMMAND_CHAIN_MOVE_ATTACK_CAST_RUNTIME_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_COMMAND_CHAIN_MAX_CHAMPIONS_PC34 4
#define CSB_V1_COMMAND_CHAIN_SOURCE_QUEUE_STORAGE_PC34 8
#define CSB_V1_COMMAND_CHAIN_SOURCE_REGULAR_BUDGET_PC34 5
#define CSB_V1_COMMAND_CHAIN_MAX_CAPACITY_PC34 5
#define CSB_V1_COMMAND_CHAIN_TARGET_COUNT_PC34 16

#define CSB_V1_COMMAND_CHAIN_MOVE_FORWARD_PC34 0
#define CSB_V1_COMMAND_CHAIN_MOVE_RIGHT_PC34 1
#define CSB_V1_COMMAND_CHAIN_MOVE_BACKWARD_PC34 2
#define CSB_V1_COMMAND_CHAIN_MOVE_LEFT_PC34 3

typedef enum {
    CSB_V1_COMMAND_CHAIN_NONE_PC34 = 0,
    CSB_V1_COMMAND_CHAIN_MOVE_PC34 = 3,
    CSB_V1_COMMAND_CHAIN_ATTACK_PC34 = 111,
    CSB_V1_COMMAND_CHAIN_CAST_PC34 = 100
} CSB_V1_CommandChainCommandTypePc34Compat;

typedef struct {
    int move_direction;
    int target_id;
    int target_alive;
    int attack_action_index;
    int attack_cooldown_ticks;
    int cast_script_id;
    int cast_symbol_seed;
} CSB_V1_CommandChainTargetInfoPc34Compat;

typedef struct {
    int command_type;
    int champion_index;
    int sequence_id;
    int enqueue_tick;
    CSB_V1_CommandChainTargetInfoPc34Compat target_info;
} CSB_V1_CommandChainQueuedCommandPc34Compat;

typedef struct {
    int command_type;
    int champion_index;
    int sequence_id;
    int tick_index;
    int queue_count_after;
    int moved;
    int old_x;
    int old_y;
    int new_x;
    int new_y;
    int attack_attempted;
    int attack_hit;
    int attack_failed_target_dead;
    int attack_cooldown_ticks;
    int cast_started;
    int chaos_cast_status;
    int cast_script_id;
    int cast_symbol_seed;
} CSB_V1_CommandChainDispatchPc34Compat;

typedef struct {
    int x;
    int y;
    int dir;
    int attack_cooldown_ticks;
    int attack_attempts;
    int attack_hits;
    int attack_failures;
    int chaos_cast_status;
    int chaos_cast_script_id;
    int chaos_cast_symbol_seed;
    int casts_started;
} CSB_V1_CommandChainChampionPc34Compat;

typedef struct {
    int party_count;
    int command_queue_capacity;
    int tick_count;
    int enqueue_sequence;
    int next_champion_scan;
    int overflow_count;
    int canceled_count;
    int target_alive[CSB_V1_COMMAND_CHAIN_TARGET_COUNT_PC34];
    CSB_V1_CommandChainChampionPc34Compat champions[
        CSB_V1_COMMAND_CHAIN_MAX_CHAMPIONS_PC34];
    CSB_V1_CommandChainQueuedCommandPc34Compat queues[
        CSB_V1_COMMAND_CHAIN_MAX_CHAMPIONS_PC34]
        [CSB_V1_COMMAND_CHAIN_MAX_CAPACITY_PC34];
    int queue_counts[CSB_V1_COMMAND_CHAIN_MAX_CHAMPIONS_PC34];
    CSB_V1_CommandChainDispatchPc34Compat last_dispatch;
} CSB_V1_CommandChainStatePc34Compat;

void csb_v1_command_chain_init(
    CSB_V1_CommandChainStatePc34Compat *state,
    int party_count,
    int command_queue_capacity);

int csb_v1_command_chain_push(
    CSB_V1_CommandChainStatePc34Compat *state,
    int champion_index,
    int command_type,
    CSB_V1_CommandChainTargetInfoPc34Compat target_info);

CSB_V1_CommandChainDispatchPc34Compat csb_v1_command_chain_tick(
    CSB_V1_CommandChainStatePc34Compat *state);

int csb_v1_command_chain_cancel_at(
    CSB_V1_CommandChainStatePc34Compat *state,
    int champion_index,
    int position);

const char *csb_v1_command_chain_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
