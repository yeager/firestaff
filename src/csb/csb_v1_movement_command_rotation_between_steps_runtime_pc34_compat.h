#ifndef FIRESTAFF_CSB_V1_MOVEMENT_COMMAND_ROTATION_BETWEEN_STEPS_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_MOVEMENT_COMMAND_ROTATION_BETWEEN_STEPS_RUNTIME_PC34_COMPAT_H

#include "csb_v1_movement_command_step_runtime_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int base_party_x;
    int base_party_y;
    int initial_party_dir;
    int champion_count;
} CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec;

typedef struct {
    uint8_t cell;
    uint8_t direction;
} CSB_V1_MovementCommandRotationChampionSnapshotPc34;

typedef struct {
    int party_x_after_step1;
    int party_y_after_step1;
    int party_dir_after_step1;
    int party_x_after_rotation;
    int party_y_after_rotation;
    int party_dir_after_rotation;
    int party_x_after_step2;
    int party_y_after_step2;
    int party_dir_after_step2;
    int queue_count_after_step1;
    int queue_empty_after_step1;
    int queue_count_after_rotation;
    int queue_empty_after_rotation;
    int queue_count_after_step2;
    int queue_empty_after_step2;
    int first_forward_queued;
    int first_forward_processed;
    int turn_right_queued;
    int turn_right_processed;
    int second_forward_queued;
    int second_forward_processed;
    struct Dm1V1InputQueueProcessResultPc34Compat step1_queue_result;
    struct Dm1V1InputQueueProcessResultPc34Compat turn_queue_result;
    struct Dm1V1InputQueueProcessResultPc34Compat step2_queue_result;
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat step1_result;
    CSB_V1_InputCommandRuntimeResult turn_result;
    CSB_V1_MovementCommandStepRuntimeResultPc34Compat step2_result;
    CSB_V1_MovementCommandRotationChampionSnapshotPc34 champions_after_step1[CSB_V1_MAX_CHAMPIONS];
    CSB_V1_MovementCommandRotationChampionSnapshotPc34 champions_after_rotation[CSB_V1_MAX_CHAMPIONS];
    CSB_V1_MovementCommandRotationChampionSnapshotPc34 champions_after_step2[CSB_V1_MAX_CHAMPIONS];
} CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result;

const char *csb_v1_movement_command_rotation_between_steps_source_evidence_pc34(void);

int csb_v1_movement_command_rotation_between_steps_simulate_pc34(
    const CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Spec *spec,
    CSB_V1_MovementCommandRotationBetweenStepsRuntimePc34Result *out_result);

#ifdef __cplusplus
}
#endif

#endif
