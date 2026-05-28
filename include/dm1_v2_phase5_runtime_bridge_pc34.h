#ifndef FIRESTAFF_DM1_V2_PHASE5_RUNTIME_BRIDGE_PC34_H
#define FIRESTAFF_DM1_V2_PHASE5_RUNTIME_BRIDGE_PC34_H

#include <stdint.h>

#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "dm1_v2_camera_controller_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sourceTickAccepted;
    int sourceStepAccepted;
    int sourceTurnAccepted;
    int sourceMovementBlocked;
    int cameraStarted;
    int cameraMoveStarted;
    int cameraTurnStarted;
    int sourceCooldownTicks;
    int sourceProjectileCooldownTicks;
    unsigned long sourceGameTick;
    unsigned long sourceLastPartyMovementTime;
    int viewportRedrawRequested;
    int redrawCadencePreserved;
    int sourceMutationForbidden;
    const char* sourceEvidence;
} DM1_V2_Phase5RuntimeBridgeResultPc34;

/*
 * Phase 5 runtime bridge: consume an already processed, source-accepted V1
 * movement tick and start V2 presentation interpolation from the resulting
 * source party tuple. The V1 pipeline, result, and party pointers are const:
 * this seam is allowed to touch only the V2 camera controller.
 */
int dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_pc34(
    const struct Dm1V1MovementPipelinePc34Compat* sourcePipeline,
    const struct Dm1V1MovementPipelineResultPc34Compat* sourceTick,
    const struct PartyState_Compat* acceptedParty,
    DM1_V2_CameraController* camera,
    int32_t cameraDurationMs,
    DM1_V2_Phase5RuntimeBridgeResultPc34* outResult);

int dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_ex_pc34(
    const struct Dm1V1MovementPipelinePc34Compat* sourcePipeline,
    const struct Dm1V1MovementPipelineResultPc34Compat* sourceTick,
    const struct PartyState_Compat* acceptedParty,
    DM1_V2_CameraController* camera,
    int32_t cameraDurationMs,
    int smoothTurnPanEnabled,
    DM1_V2_Phase5RuntimeBridgeResultPc34* outResult);

const char* dm1_v2_phase5_runtime_bridge_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_PHASE5_RUNTIME_BRIDGE_PC34_H */
