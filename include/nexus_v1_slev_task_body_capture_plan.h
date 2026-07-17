#ifndef NEXUS_V1_SLEV_TASK_BODY_CAPTURE_PLAN_H
#define NEXUS_V1_SLEV_TASK_BODY_CAPTURE_PLAN_H

#include "nexus_v1_engine.h"

#define NEXUS_V1_SLEV_TASK_BODY_CAPTURE_LEVEL_COUNT 16U

/* These flags state only that an external source-bound reviewer has named the
 * observed byte/location. They do not assign an instruction or callback ABI. */
typedef struct {
    uint32_t entry_pc;
    uint32_t task_body_pc;
    uint32_t task_body_opcode;
    uint32_t callback_or_write_pc;
    int callback_or_write_is_write;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    int task_body_opcode_known_opaque;
    int callback_owner_known_opaque;
} Nexus_V1_SlevTaskBodyAuthenticatedLocation;

typedef struct {
    const Nexus_V1_LevelScriptCaptureTargetReceipt *target;
    const Nexus_V1_LevelScriptTraceAdmissionReceipt *trace;
    const Nexus_V1_SlevDispatchEvidenceReceipt *evidence;
    Nexus_V1_SlevTaskBodyAuthenticatedLocation location;
} Nexus_V1_SlevTaskBodyCapturePlanInputRow;

typedef struct {
    Nexus_V1_SlevTaskBodyCapturePlanInputRow rows[NEXUS_V1_SLEV_TASK_BODY_CAPTURE_LEVEL_COUNT];
} Nexus_V1_SlevTaskBodyCapturePlanInput;

typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t source_fnv1a64;
    uint32_t entry_pc;
    uint32_t task_body_pc;
    uint32_t task_body_opcode;
    uint32_t callback_or_write_pc;
    int callback_or_write_is_write;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    int source_order_required;
    int original_saturn_trace_required;
    int no_dispatch_only;
    int fallback_script_permitted;
} Nexus_V1_SlevTaskBodyCaptureTarget;

typedef struct {
    int valid;
    Nexus_V1_SlevTaskBodyCaptureTarget targets[NEXUS_V1_SLEV_TASK_BODY_CAPTURE_LEVEL_COUNT];
    int no_dispatch_only;
    int fallback_script_permitted;
} Nexus_V1_SlevTaskBodyCapturePlan;

/* Builds all SLEV00..15 targets only when every existing header/literal,
 * trace, and ordering receipt agrees with one authenticated location. */
int nexus_v1_slev_task_body_capture_plan_build(
    const Nexus_V1_SlevTaskBodyCapturePlanInput *input,
    Nexus_V1_SlevTaskBodyCapturePlan *out_plan);

#endif
