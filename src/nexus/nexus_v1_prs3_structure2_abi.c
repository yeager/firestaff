#include "nexus_v1_prs3_structure2_abi.h"

#include <string.h>

static void reset_receipt(Nexus_V1_Prs3Structure2AbiReceipt *receipt,
                          Nexus_V1_Prs3Structure2AbiStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->no_draw_only = 1;
}

static int bind_positive_vector(
    const Nexus_V1_Prs3Structure2AbiInput *input,
    Nexus_V1_Prs3Structure2AbiReceipt *receipt)
{
    Nexus_V1_Prs3Sh2SubsetTraceInput trace_input;
    Nexus_V1_Prs3Sh2SubsetTraceReceipt trace;
    Nexus_V1_BpkPrs3StreamPlan plan;

    memset(&trace_input, 0, sizeof(trace_input));
    trace_input.dm_bin = input->dm_bin;
    trace_input.dm_bin_size = input->dm_bin_size;
    trace_input.dm_bin_source_verified = input->dm_bin_source_verified;
    trace_input.menu_bpk = input->menu_bpk;
    trace_input.menu_bpk_size = input->menu_bpk_size;
    trace_input.menu_bpk_source_verified = input->menu_bpk_source_verified;
    trace_input.entry_index = input->prs3_entry_index;

    if (!nexus_v1_prs3_sh2_subset_trace_run(&trace_input, &trace) ||
        !trace.full_expected_output_observed ||
        !trace.positive_output_vector_bound ||
        !trace.zero_side_copy_semantics_proven ||
        trace.original_saturn_execution_authenticated ||
        trace.opcode_grammar_proven ||
        trace.decoder_promoted ||
        trace.fallback_visuals_permitted ||
        nexus_v1_bpk_archive_prs3_stream_plan(
            input->menu_bpk, input->menu_bpk_size, input->prs3_entry_index,
            &plan) != NEXUS_V1_BPK_PRS3_STREAM_OK) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_VECTOR;
        return 0;
    }

    receipt->positive_prs3_vector_bound = 1;
    receipt->prs3_entry_index = input->prs3_entry_index;
    receipt->prs3_stream_offset = trace.stream_offset;
    receipt->prs3_stream_size = trace.stream_size;
    receipt->prs3_expected_output_bytes = trace.expected_output_bytes;
    receipt->prs3_width = plan.width;
    receipt->prs3_height = plan.height;
    receipt->prs3_bpp = plan.bpp;
    receipt->prs3_output_fnv1a64 = trace.output_prefix_fnv1a64;
    receipt->prs3_header_span_fnv1a64 = plan.header_span_fnv1a64;
    receipt->prs3_bitmap_candidate_fnv1a64 = plan.body_span_fnv1a64;
    receipt->prs3_bitmap_candidate_offset = plan.body_offset;
    receipt->prs3_bitmap_candidate_size = plan.body_size;
    receipt->prs3_input_read_bytes = trace.input_read_bytes;
    receipt->prs3_output_store_count = trace.output_store_count;
    receipt->prs3_zero_merge_count = trace.zero_merge_count;
    receipt->prs3_zero_copy_count = trace.zero_indexed_read_count;
    return 1;
}

static int bind_palt(
    const Nexus_V1_Prs3Structure2AbiInput *input,
    Nexus_V1_Prs3Structure2AbiReceipt *receipt)
{
    Nexus_V1_BpkPaletteTrailerReceipt palt;

    if (nexus_v1_bpk_archive_inspect_palette_trailer(
            input->menu_bpk, input->menu_bpk_size, &palt) != 0 ||
        !palt.valid ||
        palt.entry_count != 256U ||
        palt.entry_bytes != 512U ||
        !palt.raw_entries_are_be16 ||
        palt.palette_format_proven ||
        palt.decoder_promoted ||
        palt.fallback_visuals_permitted ||
        palt.entry_bytes_fnv1a64 == 0U) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_PALT;
        return 0;
    }
    receipt->palt_trailer_bound = 1;
    receipt->palt_entries_fnv1a64 = palt.entry_bytes_fnv1a64;
    receipt->palt_entries_are_be16 = 1;
    receipt->palt_candidate_fnv1a64 = palt.entry_bytes_fnv1a64;
    receipt->palt_candidate_size = palt.entry_bytes;
    return 1;
}

static int bind_structure2(
    const Nexus_V1_Prs3Structure2AbiInput *input,
    Nexus_V1_Prs3Structure2AbiReceipt *receipt)
{
    Nexus_V1_Prs3Structure2IntakeInput intake_input;
    Nexus_V1_Prs3Structure2IntakeReceipt intake;

    memset(&intake_input, 0, sizeof(intake_input));
    intake_input.menu_bpk = input->menu_bpk;
    intake_input.menu_bpk_size = input->menu_bpk_size;
    intake_input.menu_bpk_source_verified = input->menu_bpk_source_verified;
    intake_input.level = input->level;
    intake_input.level_index = input->level_index;
    intake_input.level_source_verified = input->level_source_verified;

    if (!nexus_v1_prs3_structure2_intake_admit(&intake_input, &intake) ||
        intake.status != NEXUS_V1_PRS3_STRUCTURE2_INTAKE_READY_NO_DRAW ||
        !intake.structure2_descriptor_bound ||
        !intake.structure2_payload_anchor_intake_bound ||
        intake.structure2_pixel_span_proven ||
        intake.structure2_palette_addressing_proven ||
        intake.structure2_decoder_permitted ||
        intake.can_submit_structure2_pixels ||
        intake.can_submit_palette ||
        intake.runtime_render_permitted ||
        intake.fallback_visuals_permitted) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_STRUCTURE2;
        return 0;
    }
    receipt->structure2_intake_bound = 1;
    receipt->structure2_descriptor_count = intake.structure2_descriptor_count;
    receipt->structure2_image_anchor_count =
        intake.structure2_image_anchor_count;
    receipt->structure2_palette_anchor_count =
        intake.structure2_palette_anchor_count;
    receipt->structure2_palette_absent_count =
        intake.structure2_palette_absent_count;
    receipt->structure2_encoding_0x0008_count =
        intake.structure2_encoding_0x0008_count;
    receipt->structure2_encoding_0x0028_count =
        intake.structure2_encoding_0x0028_count;
    return 1;
}

int nexus_v1_prs3_structure2_abi_admit(
    const Nexus_V1_Prs3Structure2AbiInput *input,
    Nexus_V1_Prs3Structure2AbiReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    reset_receipt(out_receipt, NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_INPUT);
    if (!input || !input->dm_bin || !input->menu_bpk || !input->level ||
        !input->dm_bin_source_verified || !input->menu_bpk_source_verified ||
        !input->level_source_verified || input->prs3_entry_index == 0U) {
        return 0;
    }
    out_receipt->dm_bin_source_verified = 1;
    out_receipt->menu_bpk_source_verified = 1;
    out_receipt->level_source_verified = 1;

    if (!bind_positive_vector(input, out_receipt) ||
        !bind_palt(input, out_receipt) ||
        !bind_structure2(input, out_receipt)) {
        return 0;
    }

    out_receipt->independent_saturn_trace_bound =
        input->independent_saturn_trace_bound ? 1 : 0;
    out_receipt->vdp1_consumer_semantics_proven =
        input->vdp1_consumer_semantics_proven ? 1 : 0;
    out_receipt->pixel_format_proven =
        input->pixel_format_proven ? 1 : 0;
    out_receipt->palette_application_proven =
        input->palette_application_proven ? 1 : 0;
    out_receipt->structure2_placement_proven =
        input->structure2_placement_proven ? 1 : 0;

    out_receipt->decoder_output_to_structure2_bound =
        out_receipt->positive_prs3_vector_bound &&
        out_receipt->palt_trailer_bound &&
        out_receipt->structure2_intake_bound;
    out_receipt->can_submit_structure2_pixels = 0;
    out_receipt->can_submit_palette = 0;
    out_receipt->runtime_m11_handoff_permitted = 0;
    out_receipt->fallback_visuals_permitted = 0;
    out_receipt->no_draw_only = 1;

    if (!out_receipt->independent_saturn_trace_bound ||
        !out_receipt->vdp1_consumer_semantics_proven ||
        !out_receipt->pixel_format_proven ||
        !out_receipt->palette_application_proven ||
        !out_receipt->structure2_placement_proven) {
        out_receipt->status =
            NEXUS_V1_PRS3_STRUCTURE2_ABI_READY_BLOCKED;
        return 1;
    }

    out_receipt->status =
        NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_AUTH_TRACE;
    return 0;
}

const char *nexus_v1_prs3_structure2_abi_status_name(
    Nexus_V1_Prs3Structure2AbiStatus status)
{
    switch (status) {
    case NEXUS_V1_PRS3_STRUCTURE2_ABI_READY_BLOCKED:
        return "ready-blocked";
    case NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_VECTOR:
        return "blocked-vector";
    case NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_PALT:
        return "blocked-palt";
    case NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_STRUCTURE2:
        return "blocked-structure2";
    case NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_AUTH_TRACE:
        return "blocked-auth-trace";
    default:
        return "unknown";
    }
}
