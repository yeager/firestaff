#include "nexus_v1_prs3_vdp1_capture_replay.h"

#include <string.h>

int nexus_v1_prs3_vdp1_capture_replay_admit(
    Nexus_V1_Prs3Vdp1CaptureReplayState *state,
    const Nexus_V1_Prs3Vdp1CaptureReplayInput *input,
    Nexus_V1_Prs3DgnPlacementAdapterReceipt *out_receipt)
{
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace;
    const Nexus_V1_Prs3Vdp1CaptureBindingReceipt *binding;
    const Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *evidence;
    Nexus_V1_Prs3DgnPlacementAdapterReceipt candidate;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->no_draw_only = 1;
    out_receipt->blocks_real_dgn_mesh_render = 1;
    if (!state || !input || !(trace = input->placement.capture) ||
        !(binding = input->capture_binding) ||
        !(evidence = input->consumer_evidence) ||
        trace->schema_version != 10U || !trace->valid ||
        !trace->complete_capture || !trace->dgn_placement_observed ||
        !trace->dgn_descriptor_fnv1a64 || !binding->valid ||
        !binding->dgn_placement_observed ||
        evidence->status != NEXUS_V1_PRS3_VDP1_CONSUMER_READY_BLOCKED ||
        !evidence->raw_trace_authenticated || !evidence->candidate_spans_bound ||
        !evidence->trace_binds_menu_bpk_entry5 ||
        !evidence->trace_binds_lev00_structure2 ||
        !evidence->prs3_header_span_fnv1a64 ||
        !evidence->prs3_bitmap_candidate_fnv1a64 ||
        !evidence->prs3_bitmap_candidate_size ||
        !evidence->palt_candidate_fnv1a64 ||
        evidence->palt_candidate_size != 512U ||
        input->placement.trace_fnv1a64 != evidence->raw_trace_fnv1a64 ||
        !input->placement.trace_fnv1a64 ||
        (state->valid &&
         (trace->dgn_frame_sequence <= state->last_frame_sequence ||
          trace->dgn_command_sequence <= state->last_command_sequence ||
          trace->dgn_descriptor_index <= state->last_descriptor_index))) {
        return 0;
    }
    memset(&candidate, 0, sizeof(candidate));
    if (!nexus_v1_prs3_dgn_placement_adapter_admit(
            &input->placement, &candidate) || !candidate.valid ||
        !candidate.no_draw_only || candidate.fallback_visuals_permitted ||
        candidate.descriptor_fnv1a64 != trace->dgn_descriptor_fnv1a64 ||
        candidate.frame_sequence != trace->dgn_frame_sequence ||
        candidate.command_sequence != trace->dgn_command_sequence) {
        return 0;
    }
    *out_receipt = candidate;
    state->valid = 1;
    state->last_frame_sequence = trace->dgn_frame_sequence;
    state->last_command_sequence = trace->dgn_command_sequence;
    state->last_descriptor_index = trace->dgn_descriptor_index;
    return 1;
}
