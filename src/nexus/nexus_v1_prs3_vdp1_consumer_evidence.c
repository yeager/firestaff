#include "nexus_v1_prs3_vdp1_consumer_evidence.h"

#include <string.h>

static void reset_receipt(Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *receipt,
                          Nexus_V1_Prs3Vdp1ConsumerEvidenceStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->requires_independent_trace = 1;
    receipt->rejects_collector_manifest_only = 1;
    receipt->no_draw_only = 1;
}

static int copy_abi(const Nexus_V1_Prs3Structure2AbiReceipt *abi,
                    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *receipt)
{
    if (!abi ||
        abi->status != NEXUS_V1_PRS3_STRUCTURE2_ABI_READY_BLOCKED ||
        !abi->decoder_output_to_structure2_bound ||
        !abi->positive_prs3_vector_bound ||
        !abi->palt_trailer_bound ||
        !abi->structure2_intake_bound ||
        abi->prs3_entry_index != 5U ||
        abi->prs3_expected_output_bytes != 1674U ||
        abi->prs3_output_fnv1a64 != UINT64_C(0x14cacc01cee292aa) ||
        !abi->prs3_header_span_fnv1a64 ||
        !abi->prs3_bitmap_candidate_fnv1a64 ||
        abi->prs3_stream_offset > UINT32_MAX - 4U ||
        abi->prs3_stream_size < 4U ||
        abi->prs3_bitmap_candidate_offset != abi->prs3_stream_offset + 4U ||
        abi->prs3_bitmap_candidate_size != abi->prs3_stream_size - 4U ||
        abi->palt_entries_fnv1a64 != UINT64_C(0x0ec4e98ca3a18f85) ||
        !abi->palt_entries_are_be16 ||
        abi->palt_candidate_fnv1a64 != abi->palt_entries_fnv1a64 ||
        abi->palt_candidate_size != 512U ||
        abi->can_submit_structure2_pixels ||
        abi->can_submit_palette ||
        abi->runtime_m11_handoff_permitted ||
        abi->fallback_visuals_permitted) {
        receipt->status = NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_ABI;
        return 0;
    }

    receipt->capture_target_bound = 1;
    receipt->prs3_entry_index = abi->prs3_entry_index;
    receipt->prs3_stream_offset = abi->prs3_stream_offset;
    receipt->prs3_stream_size = abi->prs3_stream_size;
    receipt->prs3_expected_output_bytes = abi->prs3_expected_output_bytes;
    receipt->prs3_width = abi->prs3_width;
    receipt->prs3_height = abi->prs3_height;
    receipt->prs3_bpp = abi->prs3_bpp;
    receipt->prs3_output_fnv1a64 = abi->prs3_output_fnv1a64;
    receipt->palt_entries_fnv1a64 = abi->palt_entries_fnv1a64;
    receipt->palt_entries_are_be16 = abi->palt_entries_are_be16;
    receipt->structure2_descriptor_count = abi->structure2_descriptor_count;
    receipt->structure2_image_anchor_count =
        abi->structure2_image_anchor_count;
    receipt->structure2_palette_anchor_count =
        abi->structure2_palette_anchor_count;
    receipt->structure2_palette_absent_count =
        abi->structure2_palette_absent_count;
    return 1;
}

int nexus_v1_prs3_vdp1_consumer_evidence_admit(
    const Nexus_V1_Prs3Vdp1ConsumerEvidenceInput *input,
    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    reset_receipt(out_receipt, NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_INPUT);
    if (!input) return 0;

    out_receipt->retail_media_available =
        input->retail_media_available ? 1 : 0;
    out_receipt->saturn_emulator_available =
        input->saturn_emulator_available ? 1 : 0;
    out_receipt->saturn_bios_available =
        input->saturn_bios_available ? 1 : 0;
    out_receipt->trace_artifact_available =
        input->raw_trace_artifact_available ? 1 : 0;
    out_receipt->raw_trace_fnv1a64 = input->raw_trace_fnv1a64;
    out_receipt->raw_trace_size = input->raw_trace_size;
    out_receipt->raw_trace_authenticated =
        input->raw_trace_authenticated ? 1 : 0;
    out_receipt->trace_binds_dm_bin = input->trace_binds_dm_bin ? 1 : 0;
    out_receipt->trace_binds_menu_bpk_entry5 =
        input->trace_binds_menu_bpk_entry5 ? 1 : 0;
    out_receipt->trace_binds_lev00_structure2 =
        input->trace_binds_lev00_structure2 ? 1 : 0;
    out_receipt->vdp1_command_observed =
        input->vdp1_command_observed ? 1 : 0;
    out_receipt->vdp1_texture_window_observed =
        input->vdp1_texture_window_observed ? 1 : 0;
    out_receipt->be16_palette_application_observed =
        input->be16_palette_application_observed ? 1 : 0;
    out_receipt->structure2_descriptor_selection_observed =
        input->structure2_descriptor_selection_observed ? 1 : 0;
    out_receipt->texture_placement_observed =
        input->texture_placement_observed ? 1 : 0;
    out_receipt->candidate_spans_bound = input->prs3_header_span_fnv1a64 &&
        input->prs3_bitmap_candidate_fnv1a64 &&
        input->palt_candidate_fnv1a64 && input->prs3_bitmap_candidate_size &&
        input->palt_candidate_size == 512U && input->abi &&
        input->abi->prs3_stream_offset <= UINT32_MAX - 4U &&
        input->abi->prs3_stream_size >= 4U &&
        input->prs3_bitmap_candidate_offset ==
            input->abi->prs3_stream_offset + 4U &&
        input->prs3_bitmap_candidate_size ==
            input->abi->prs3_stream_size - 4U &&
        input->palt_candidate_fnv1a64 == input->abi->palt_entries_fnv1a64;

    if (!copy_abi(input->abi, out_receipt)) return 0;

    if (out_receipt->candidate_spans_bound) {
        out_receipt->prs3_header_span_fnv1a64 =
            input->prs3_header_span_fnv1a64;
        out_receipt->prs3_bitmap_candidate_fnv1a64 =
            input->prs3_bitmap_candidate_fnv1a64;
        out_receipt->prs3_bitmap_candidate_offset =
            input->prs3_bitmap_candidate_offset;
        out_receipt->prs3_bitmap_candidate_size =
            input->prs3_bitmap_candidate_size;
        out_receipt->palt_candidate_fnv1a64 = input->palt_candidate_fnv1a64;
        out_receipt->palt_candidate_size = input->palt_candidate_size;
    }

    if (!input->retail_media_available ||
        !input->saturn_emulator_available ||
        !input->saturn_bios_available) {
        out_receipt->status =
            NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_EMULATOR;
        return 1;
    }

    if (!input->raw_trace_artifact_available ||
        input->raw_trace_size == 0U ||
        input->raw_trace_fnv1a64 == 0U ||
        !input->raw_trace_authenticated ||
        input->collector_manifest_only ||
        !input->trace_binds_dm_bin ||
        !input->trace_binds_menu_bpk_entry5 ||
        !input->trace_binds_lev00_structure2) {
        out_receipt->status =
            NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE;
        return 1;
    }
    if (!out_receipt->candidate_spans_bound) {
        out_receipt->status = NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE;
        return 1;
    }

    if (!input->vdp1_command_observed ||
        !input->vdp1_texture_window_observed ||
        !input->be16_palette_application_observed ||
        !input->structure2_descriptor_selection_observed ||
        !input->texture_placement_observed) {
        out_receipt->status =
            NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_CONSUMER_SEMANTICS;
        return 1;
    }

    out_receipt->status = NEXUS_V1_PRS3_VDP1_CONSUMER_READY_BLOCKED;
    return 1;
}

int nexus_v1_prs3_vdp1_consumer_evidence_admit_capture_binding(
    const Nexus_V1_Prs3Structure2AbiReceipt *abi,
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace,
    const Nexus_V1_Prs3Vdp1CaptureBindingReceipt *binding,
    uint64_t trace_fnv1a64, uint32_t trace_size,
    int original_saturn_capture_authenticated,
    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *out_receipt)
{
    Nexus_V1_Prs3Vdp1ConsumerEvidenceInput input;
    int trace_bound;

    memset(&input, 0, sizeof(input));
    input.abi = abi;
    input.retail_media_available = 1;
    input.saturn_emulator_available = 1;
    input.saturn_bios_available = 1;
    trace_bound = trace && binding && trace->valid && trace->complete_capture &&
        trace->schema_version >= 3U && binding->valid &&
        binding->menu_bpk_matches && binding->dm_bin_matches &&
        binding->entry_plan_matches && binding->payload_span_matches &&
        binding->exact_vdp1_handoff_observed &&
        binding->vdp1_texture_consumption_observed &&
        binding->vdp1_command_consumption_observed &&
        binding->palette_consumption_observed &&
        !binding->decoder_promoted && !binding->fallback_visuals_permitted &&
        abi && trace->entry_index == abi->prs3_entry_index &&
        trace->stream_offset == abi->prs3_stream_offset &&
        trace->stream_size == abi->prs3_stream_size &&
        trace->expected_output_bytes == abi->prs3_expected_output_bytes;
    input.raw_trace_artifact_available = trace_bound && trace_size > 0U &&
        trace_fnv1a64 != 0U;
    input.raw_trace_fnv1a64 = trace_fnv1a64;
    input.raw_trace_size = trace_size;
    input.prs3_header_span_fnv1a64 =
        trace_bound && abi ? abi->prs3_header_span_fnv1a64 : 0U;
    input.prs3_bitmap_candidate_fnv1a64 =
        trace_bound && abi ? abi->prs3_bitmap_candidate_fnv1a64 : 0U;
    input.prs3_bitmap_candidate_offset =
        trace_bound && abi ? abi->prs3_bitmap_candidate_offset : 0U;
    input.prs3_bitmap_candidate_size =
        trace_bound && abi ? abi->prs3_bitmap_candidate_size : 0U;
    input.palt_candidate_fnv1a64 =
        trace_bound && abi ? abi->palt_candidate_fnv1a64 : 0U;
    input.palt_candidate_size =
        trace_bound && abi ? abi->palt_candidate_size : 0U;
    input.raw_trace_authenticated = trace_bound &&
        original_saturn_capture_authenticated;
    input.trace_binds_dm_bin = trace_bound;
    input.trace_binds_menu_bpk_entry5 = trace_bound;
    input.trace_binds_lev00_structure2 = trace_bound;
    input.vdp1_command_observed = trace_bound;
    input.vdp1_texture_window_observed = trace_bound;
    input.be16_palette_application_observed = trace_bound;
    /* PRS3 traces do not identify a Structure2 descriptor or placement. */
    input.structure2_descriptor_selection_observed = 0;
    input.texture_placement_observed = 0;
    return nexus_v1_prs3_vdp1_consumer_evidence_admit(&input, out_receipt);
}

const char *nexus_v1_prs3_vdp1_consumer_evidence_status_name(
    Nexus_V1_Prs3Vdp1ConsumerEvidenceStatus status)
{
    switch (status) {
    case NEXUS_V1_PRS3_VDP1_CONSUMER_READY_BLOCKED:
        return "ready-blocked";
    case NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_ABI:
        return "blocked-abi";
    case NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_EMULATOR:
        return "blocked-emulator";
    case NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE:
        return "blocked-trace";
    case NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_CONSUMER_SEMANTICS:
        return "blocked-consumer-semantics";
    default:
        return "unknown";
    }
}
