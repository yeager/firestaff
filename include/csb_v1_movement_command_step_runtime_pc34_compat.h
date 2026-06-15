#ifndef FIRESTAFF_CSB_V1_MOVEMENT_COMMAND_STEP_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_MOVEMENT_COMMAND_STEP_RUNTIME_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*CSB_V1_MovementStepWallProbePc34Compat)(
    const CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    void *context);

typedef struct {
    struct Dm1V1InputQueueProcessResultPc34Compat queue_result;
    int command_handled;
    int step_attempted;
    int step_applied;
    int blocked_by_wall;
    int wall_probe_called;
    int unsupported_runtime_command;
    int old_party_x;
    int old_party_y;
    int old_party_dir;
    int destination_x;
    int destination_y;
    int new_party_x;
    int new_party_y;
    int new_party_dir;
    int party_state_changed;
    int disabled_movement_ticks_after;
} CSB_V1_MovementCommandStepRuntimeResultPc34Compat;

int csb_v1_movement_command_step_runtime_apply_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    int command,
    CSB_V1_MovementStepWallProbePc34Compat wall_probe,
    void *wall_probe_context,
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat *out_result);

int csb_v1_movement_command_step_runtime_process_queue_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction,
    CSB_V1_MovementStepWallProbePc34Compat wall_probe,
    void *wall_probe_context,
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat *out_result);

const char *csb_v1_movement_command_step_runtime_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
