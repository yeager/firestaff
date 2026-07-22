#include "nexus_v1_prs3_vdp1_capture_replay.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_DgnActiveLevelRendererSourceReceipt dgn;
    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt evidence;
    Nexus_V1_Prs3Vdp1CaptureReceipt trace;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt binding;
    Nexus_V1_Prs3Vdp1CaptureReplayInput input;
    Nexus_V1_Prs3Vdp1CaptureReplayState state;
    Nexus_V1_Prs3DgnPlacementAdapterReceipt receipt;

    memset(&dgn, 0, sizeof(dgn)); memset(&evidence, 0, sizeof(evidence));
    memset(&trace, 0, sizeof(trace)); memset(&binding, 0, sizeof(binding));
    memset(&input, 0, sizeof(input)); memset(&state, 0, sizeof(state));
    dgn.valid = dgn.package_source_bound = dgn.vdp1_command_format_framed =
        dgn.vdp1_coordinate_words_framed = 1;
    dgn.source_bytes_fnv1a64 = 7U;
    dgn.vdp1_command_framing.command.xa = 1;
    evidence.status = NEXUS_V1_PRS3_VDP1_CONSUMER_READY_BLOCKED;
    evidence.raw_trace_authenticated = evidence.candidate_spans_bound = 1;
    evidence.trace_artifact_available = evidence.trace_binds_dm_bin = 1;
    evidence.trace_binds_menu_bpk_entry5 = evidence.trace_binds_lev00_structure2 = 1;
    evidence.raw_trace_fnv1a64 = 9U; evidence.raw_trace_size = 1U;
    evidence.prs3_stream_offset = 16U; evidence.prs3_stream_size = 12U;
    evidence.prs3_header_span_fnv1a64 = 11U;
    evidence.prs3_bitmap_candidate_fnv1a64 = 12U;
    evidence.prs3_bitmap_candidate_offset = 20U;
    evidence.prs3_bitmap_candidate_size = 8U;
    evidence.palt_candidate_fnv1a64 = 13U; evidence.palt_candidate_size = 512U;
    binding.valid = binding.dgn_placement_observed = 1;
    trace.valid = trace.complete_capture = trace.dgn_placement_observed = 1;
    trace.schema_version = 10U; trace.dgn_fnv1a64 = 7U;
    trace.dgn_descriptor_index = 2U; trace.dgn_descriptor_fnv1a64 = 14U;
    trace.dgn_frame_sequence = 10U; trace.dgn_command_sequence = 11U;
    trace.vdp1_command_sequence = 11U; trace.dgn_command_xa = 1U;
    input.placement.dgn = &dgn; input.placement.prs3 = &evidence;
    input.placement.capture = &trace; input.placement.trace_fnv1a64 = 9U;
    input.placement.trace_size = 1U;
    input.placement.traced_dgn_fnv1a64 = 7U; input.placement.descriptor_index = 2U;
    input.placement.descriptor_fnv1a64 = 14U;
    input.placement.descriptor_envelope_valid = 1; input.placement.traced_xa = 1;
    input.capture_binding = &binding; input.consumer_evidence = &evidence;
    if (!nexus_v1_prs3_vdp1_capture_replay_admit(&state, &input, &receipt) ||
        !receipt.valid || !receipt.no_draw_only ||
        receipt.fallback_visuals_permitted || !state.valid) return 1;
    if (nexus_v1_prs3_vdp1_capture_replay_admit(&state, &input, &receipt)) return 1;
    trace.dgn_frame_sequence = 11U; trace.dgn_command_sequence = 12U;
    if (nexus_v1_prs3_vdp1_capture_replay_admit(&state, &input, &receipt)) return 1;
    trace.dgn_descriptor_index = 3U; input.placement.descriptor_index = 3U;
    trace.dgn_descriptor_fnv1a64 = input.placement.descriptor_fnv1a64 = 15U;
    trace.vdp1_command_sequence = trace.dgn_command_sequence;
    if (!nexus_v1_prs3_vdp1_capture_replay_admit(&state, &input, &receipt) ||
        receipt.fallback_visuals_permitted || !receipt.no_draw_only) return 1;
    trace.schema_version = 9U;
    if (nexus_v1_prs3_vdp1_capture_replay_admit(&state, &input, &receipt)) return 1;
    puts("prs3 vdp1 capture replay: PASS");
    return 0;
}
