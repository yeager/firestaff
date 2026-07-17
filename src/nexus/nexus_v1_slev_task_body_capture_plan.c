#include "nexus_v1_slev_task_body_capture_plan.h"

#include <string.h>

static int row_is_exact(const Nexus_V1_SlevTaskBodyCapturePlanInputRow *row,
                        uint32_t level)
{
    const Nexus_V1_LevelScriptCaptureTargetReceipt *target;
    const Nexus_V1_LevelScriptTraceAdmissionReceipt *trace;
    const Nexus_V1_SlevDispatchEvidenceReceipt *evidence;
    const Nexus_V1_SlevTaskBodyAuthenticatedLocation *location;

    if (!row || !(target = row->target) || !(trace = row->trace) ||
        !(evidence = row->evidence)) return 0;
    location = &row->location;
    return target->valid && target->level_index == (int)level &&
        target->source_fnv1a64 && target->task_header_size > 0 &&
        target->primary_literal_address && target->auxiliary_literal_address &&
        target->original_saturn_execution_required && !target->task_body_dispatch_proven &&
        target->no_dispatch_only && !target->fallback_visuals_permitted &&
        trace->status == NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE &&
        trace->level_index == (int)level && trace->capture_target_bound &&
        trace->mednafen_debugger_provenance && trace->original_saturn_execution_claimed &&
        trace->trace_sha256_present && trace->source_fnv1a64 == target->source_fnv1a64 &&
        trace->raw_trace_bytes_bound && trace->raw_trace_fnv1a64 &&
        trace->raw_trace_byte_count && trace->trace_chain_complete &&
        !trace->task_body_dispatch_proven && !trace->dispatch_permitted &&
        trace->blocks_real_script_dispatch && !trace->fallback_visuals_permitted &&
        evidence->status == NEXUS_V1_SLEV_DISPATCH_EVIDENCE_OBSERVED &&
        evidence->level_index == (int)level && evidence->raw_trace_bound &&
        evidence->raw_trace_fnv1a64 == trace->raw_trace_fnv1a64 &&
        evidence->raw_trace_byte_count == trace->raw_trace_byte_count &&
        evidence->entry_observed && evidence->task_body_observed &&
        evidence->callback_or_write_observed && evidence->observation_order_proven &&
        evidence->primary_literal_observed && evidence->auxiliary_literal_observed &&
        evidence->literal_observation_proven && !evidence->task_body_dispatch_proven &&
        !evidence->dispatch_permitted && evidence->blocks_real_script_dispatch &&
        !evidence->fallback_visuals_permitted && location->entry_pc == trace->entry_pc &&
        location->task_body_pc == trace->task_body_pc &&
        location->task_body_opcode == trace->task_body_opcode &&
        location->callback_or_write_pc == trace->callback_or_write_pc &&
        location->callback_or_write_is_write == trace->callback_or_write_is_write &&
        location->raw_trace_fnv1a64 == trace->raw_trace_fnv1a64 &&
        location->raw_trace_byte_count == trace->raw_trace_byte_count &&
        location->task_body_opcode_known_opaque && location->callback_owner_known_opaque;
}

int nexus_v1_slev_task_body_capture_plan_build(
    const Nexus_V1_SlevTaskBodyCapturePlanInput *input,
    Nexus_V1_SlevTaskBodyCapturePlan *out_plan)
{
    Nexus_V1_SlevTaskBodyCapturePlan plan;
    uint32_t level;

    if (!out_plan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.no_dispatch_only = 1;
    if (!input) { *out_plan = plan; return 0; }
    for (level = 0; level < NEXUS_V1_SLEV_TASK_BODY_CAPTURE_LEVEL_COUNT; ++level) {
        const Nexus_V1_SlevTaskBodyCapturePlanInputRow *row = &input->rows[level];
        const Nexus_V1_LevelScriptCaptureTargetReceipt *target = row->target;
        const Nexus_V1_SlevTaskBodyAuthenticatedLocation *location = &row->location;
        Nexus_V1_SlevTaskBodyCaptureTarget *out = &plan.targets[level];
        if (!row_is_exact(row, level)) { *out_plan = plan; return 0; }
        out->valid = 1;
        out->level_index = level;
        out->source_fnv1a64 = target->source_fnv1a64;
        out->entry_pc = location->entry_pc;
        out->task_body_pc = location->task_body_pc;
        out->task_body_opcode = location->task_body_opcode;
        out->callback_or_write_pc = location->callback_or_write_pc;
        out->callback_or_write_is_write = location->callback_or_write_is_write;
        out->raw_trace_fnv1a64 = location->raw_trace_fnv1a64;
        out->raw_trace_byte_count = location->raw_trace_byte_count;
        out->source_order_required = 1;
        out->original_saturn_trace_required = 1;
        out->no_dispatch_only = 1;
    }
    plan.valid = 1;
    *out_plan = plan;
    return 1;
}
