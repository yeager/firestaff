#include "nexus_v1_sal_capture_plan.h"

#include <string.h>

static int row_is_exact(const Nexus_V1_SalCapturePlanInputRow *row)
{
    const Nexus_V1_LevelSoundRouteReceipt *route;
    const Nexus_V1_LevelSoundCaptureTargetReceipt *target;
    const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace;
    const Nexus_V1_SalDispatchEvidenceReceipt *evidence;

    if (!row || !(route = row->route) || !(target = row->target) ||
        !(trace = row->trace) || !(evidence = row->evidence)) return 0;
    return route->status == NEXUS_V1_LEVEL_SOUND_ROUTE_BOUND_OPAQUE &&
        route->level_index >= 0 && route->raw_map_selector >= 0 &&
        route->raw_map_selector <= 0xff && route->sal_offset >= 0 && route->sal_size > 0 &&
        route->canonical_sal_source_verified && route->canonical_map_source_verified &&
        route->canonical_sound_driver_source_verified && route->map_window_unique_and_bounded &&
        !route->saturn_event_dispatch_proven && !route->sal_decode_proven &&
        !route->playback_permitted && route->blocks_real_sfx_playback &&
        !route->fallback_visuals_permitted && target->valid &&
        target->level_index == route->level_index &&
        target->raw_map_selector == route->raw_map_selector &&
        target->sal_offset == route->sal_offset && target->sal_size == route->sal_size &&
        target->canonical_sal_fnv1a64 && target->canonical_map_fnv1a64 &&
        target->canonical_driver_md5[0] && target->original_saturn_driver_capture_required &&
        !target->sal_decode_proven && !target->playback_permitted &&
        target->no_playback_only && !target->fallback_visuals_permitted &&
        trace->status == NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE &&
        trace->level_index == target->level_index &&
        trace->raw_map_selector == target->raw_map_selector &&
        trace->map_attribute == target->map_attribute && trace->sal_offset == target->sal_offset &&
        trace->sal_size == target->sal_size &&
        trace->canonical_sal_fnv1a64 == target->canonical_sal_fnv1a64 &&
        trace->canonical_map_fnv1a64 == target->canonical_map_fnv1a64 &&
        trace->capture_target_bound && trace->mednafen_debugger_provenance &&
        trace->original_saturn_execution_claimed && trace->trace_sha256_present &&
        trace->raw_trace_bytes_bound && trace->raw_trace_fnv1a64 &&
        trace->raw_trace_byte_count && trace->selector_dispatch_pc && trace->sal_read_pc &&
        trace->driver_output_pc && trace->trace_chain_complete &&
        !trace->driver_dispatch_proven && !trace->sal_decode_proven &&
        !trace->playback_permitted && trace->blocks_real_sfx_playback &&
        !trace->fallback_visuals_permitted &&
        evidence->status == NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED &&
        evidence->level_index == trace->level_index && evidence->raw_trace_bound &&
        evidence->raw_trace_fnv1a64 == trace->raw_trace_fnv1a64 &&
        evidence->raw_trace_byte_count == trace->raw_trace_byte_count &&
        evidence->selector_dispatch_observed && evidence->sal_read_observed &&
        evidence->driver_output_observed && evidence->observation_order_proven &&
        !evidence->driver_dispatch_proven && !evidence->sal_decode_proven &&
        !evidence->playback_permitted && evidence->blocks_real_sfx_playback &&
        !evidence->fallback_visuals_permitted;
}

int nexus_v1_sal_capture_plan_build(const Nexus_V1_SalCapturePlanInput *input,
                                    Nexus_V1_SalCapturePlan *out_plan)
{
    Nexus_V1_SalCapturePlan plan;
    size_t i;
    size_t j;

    if (!out_plan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.no_playback_only = 1;
    if (!input || !input->rows || !input->row_count ||
        input->row_count > NEXUS_V1_SAL_CAPTURE_PLAN_MAX_TARGETS) {
        *out_plan = plan;
        return 0;
    }
    for (i = 0; i < input->row_count; ++i) {
        const Nexus_V1_SalCapturePlanInputRow *row = &input->rows[i];
        const Nexus_V1_LevelSoundCaptureTargetReceipt *target;
        const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace;
        Nexus_V1_SalCapturePlanTarget *out = &plan.targets[i];
        if (!row_is_exact(row)) { *out_plan = plan; return 0; }
        target = row->target;
        trace = row->trace;
        for (j = 0; j < i; ++j) {
            if (input->rows[j].target->level_index == target->level_index &&
                input->rows[j].target->raw_map_selector == target->raw_map_selector) {
                *out_plan = plan;
                return 0;
            }
        }
        out->valid = 1; out->level_index = (uint32_t)target->level_index;
        out->raw_map_selector = (uint32_t)target->raw_map_selector;
        out->canonical_sal_fnv1a64 = target->canonical_sal_fnv1a64;
        out->canonical_map_fnv1a64 = target->canonical_map_fnv1a64;
        memcpy(out->canonical_driver_md5, target->canonical_driver_md5,
               sizeof(out->canonical_driver_md5));
        out->sal_offset = (uint32_t)target->sal_offset; out->sal_size = (uint32_t)target->sal_size;
        out->raw_trace_fnv1a64 = trace->raw_trace_fnv1a64;
        out->raw_trace_byte_count = trace->raw_trace_byte_count;
        out->selector_dispatch_pc = trace->selector_dispatch_pc;
        out->sal_read_pc = trace->sal_read_pc; out->driver_output_pc = trace->driver_output_pc;
        out->source_order_required = out->original_saturn_trace_required = out->no_playback_only = 1;
    }
    plan.valid = 1; plan.target_count = input->row_count;
    *out_plan = plan;
    return 1;
}
