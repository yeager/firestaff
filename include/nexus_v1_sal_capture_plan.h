#ifndef NEXUS_V1_SAL_CAPTURE_PLAN_H
#define NEXUS_V1_SAL_CAPTURE_PLAN_H

#include "nexus_v1_engine.h"

#define NEXUS_V1_SAL_CAPTURE_PLAN_MAX_TARGETS (16U * 256U)

typedef struct {
    const Nexus_V1_LevelSoundRouteReceipt *route;
    const Nexus_V1_LevelSoundCaptureTargetReceipt *target;
    const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace;
    const Nexus_V1_SalDispatchEvidenceReceipt *evidence;
} Nexus_V1_SalCapturePlanInputRow;

typedef struct {
    const Nexus_V1_SalCapturePlanInputRow *rows;
    size_t row_count;
} Nexus_V1_SalCapturePlanInput;

/* One source-bound, occurrence-only Saturn capture request. No field is a
 * selector meaning or SAL payload interpretation. */
typedef struct {
    int valid;
    uint32_t level_index;
    uint32_t raw_map_selector;
    uint64_t canonical_sal_fnv1a64;
    uint64_t canonical_map_fnv1a64;
    char canonical_driver_md5[33];
    uint32_t sal_offset;
    uint32_t sal_size;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    uint32_t selector_dispatch_pc;
    uint32_t sal_read_pc;
    uint32_t driver_output_pc;
    int source_order_required;
    int original_saturn_trace_required;
    int no_playback_only;
    int fallback_script_permitted;
} Nexus_V1_SalCapturePlanTarget;

typedef struct {
    int valid;
    size_t target_count;
    Nexus_V1_SalCapturePlanTarget targets[NEXUS_V1_SAL_CAPTURE_PLAN_MAX_TARGETS];
    int no_playback_only;
    int fallback_script_permitted;
} Nexus_V1_SalCapturePlan;

/* Rejects duplicate or ambiguous selector routes, identity drift, unordered
 * observations, and every payload/decode/driver semantic claim. */
int nexus_v1_sal_capture_plan_build(const Nexus_V1_SalCapturePlanInput *input,
                                    Nexus_V1_SalCapturePlan *out_plan);

#endif
