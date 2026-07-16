#include "nexus_v1_prs3_decoder_admission.h"

#include <string.h>

static void reset_receipt(Nexus_V1_Prs3DecoderAdmissionReceipt *receipt,
                          Nexus_V1_Prs3DecoderAdmissionStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
}

static int bind_executable(
    const Nexus_V1_Prs3DecoderAdmissionInput *input,
    Nexus_V1_Prs3DecoderAdmissionReceipt *receipt)
{
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;

    if (!input->dm_bin || input->dm_bin_size == 0U ||
        !input->dm_bin_source_verified) {
        receipt->status = NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_SOURCE;
        return 0;
    }
    receipt->dm_bin_source_verified = 1;
    if (!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            input->dm_bin, input->dm_bin_size, 1, &sh2) ||
        !sh2.sh2_control_path_verified ||
        !sh2.sh2_stream_read_verified ||
        !sh2.sh2_output_store_verified ||
        !sh2.sh2_nonzero_direct_byte_path_proven ||
        !sh2.sh2_nonzero_output_commit_reentry_proven ||
        !sh2.sh2_zero_side_two_byte_input_span_proven ||
        !sh2.sh2_zero_byte_merge_order_proven ||
        !sh2.sh2_zero_side_linear_route_verified ||
        !sh2.sh2_zero_side_has_no_direct_output_store ||
        sh2.menu_frame_binding_proven ||
        sh2.opcode_grammar_proven ||
        sh2.decoder_promoted) {
        receipt->status =
            NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_EXECUTABLE;
        return 0;
    }
    receipt->dm_bin_v1_loader_bound = 1;
    receipt->dm_bin_v1_callee_offset = sh2.v1_callee_offset;
    receipt->dm_bin_control_test_offset = sh2.control_low_bit_test_offset;
    receipt->dm_bin_nonzero_read_offset = sh2.stream_byte_read_offset;
    receipt->dm_bin_output_store_offset = sh2.output_byte_store_offset;
    receipt->dm_bin_zero_first_read_offset = sh2.zero_first_byte_read_offset;
    receipt->dm_bin_zero_second_read_offset = sh2.zero_second_byte_read_offset;
    receipt->nonzero_direct_byte_path_proven = 1;
    receipt->zero_side_two_byte_merge_proven = 1;
    receipt->zero_side_has_no_direct_output_store = 1;
    return 1;
}

static int bind_menu_streams(
    const Nexus_V1_Prs3DecoderAdmissionInput *input,
    Nexus_V1_Prs3DecoderAdmissionReceipt *receipt)
{
    Nexus_V1_BpkArchiveInfo archive;
    Nexus_V1_BpkPrs3StreamPlan plan;
    uint32_t index;

    if (!input->menu_bpk || input->menu_bpk_size == 0U ||
        !input->menu_bpk_source_verified) {
        receipt->status = NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_SOURCE;
        return 0;
    }
    receipt->menu_bpk_source_verified = 1;
    if (nexus_v1_bpk_archive_parse(input->menu_bpk, input->menu_bpk_size,
                                   &archive) != 0 ||
        archive.candidate_offset_count == 0U ||
        archive.prs3_payload_count == 0U) {
        receipt->status =
            NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_MENU_STREAM;
        return 0;
    }
    receipt->menu_bpk_v1_streams_bound = 1;
    receipt->menu_bpk_entry_count = archive.candidate_offset_count;
    receipt->menu_bpk_prs3_stream_count = archive.prs3_payload_count;

    for (index = 0U; index < archive.candidate_offset_count; ++index) {
        if (nexus_v1_bpk_archive_prs3_stream_plan(
                input->menu_bpk, input->menu_bpk_size, index, &plan) ==
            NEXUS_V1_BPK_PRS3_STREAM_OK) {
            receipt->first_stream_entry_index = index;
            receipt->first_stream_offset = plan.stream_offset;
            receipt->first_stream_size = plan.stream_size;
            receipt->first_expected_output_bytes =
                plan.expected_output_bytes;
            return 1;
        }
    }
    receipt->status =
        NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_MENU_STREAM;
    return 0;
}

static int bind_differential_trials(
    const Nexus_V1_Prs3DecoderAdmissionInput *input,
    Nexus_V1_Prs3DecoderAdmissionReceipt *receipt)
{
    Nexus_V1_BpkPrs3FramedEvalSummary lsb;
    Nexus_V1_BpkPrs3FramedEvalSummary msb;

    if (nexus_v1_bpk_archive_prs3_framed_decode_evidence(
            input->menu_bpk, input->menu_bpk_size,
            NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_LSB_FIRST,
            NULL, 0U, &lsb) != 0 ||
        nexus_v1_bpk_archive_prs3_framed_decode_evidence(
            input->menu_bpk, input->menu_bpk_size,
            NEXUS_V1_BPK_PRS3_CANDIDATE_BIT_ORDER_MSB_FIRST,
            NULL, 0U, &msb) != 0 ||
        lsb.prs3_surfaces == 0U || msb.prs3_surfaces == 0U ||
        lsb.decoder_promoted || msb.decoder_promoted ||
        lsb.decoded_pixels_emitted != 0U ||
        msb.decoded_pixels_emitted != 0U) {
        receipt->status =
            NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_DIFFERENTIAL;
        return 0;
    }

    receipt->lsb_trial_evaluated = lsb.evaluated;
    receipt->lsb_trial_complete_exact = lsb.complete_exact;
    receipt->lsb_trial_complete_trailing = lsb.complete_trailing;
    receipt->lsb_trial_failures = lsb.command_failures;
    receipt->msb_trial_evaluated = msb.evaluated;
    receipt->msb_trial_complete_exact = msb.complete_exact;
    receipt->msb_trial_complete_trailing = msb.complete_trailing;
    receipt->msb_trial_failures = msb.command_failures;
    receipt->simple_lsb_msb_decoder_disproven =
        lsb.complete_exact == 0U && lsb.complete_trailing == 0U &&
        msb.complete_exact == 0U && msb.complete_trailing == 0U &&
        lsb.command_failures > 0U && msb.command_failures > 0U;
    if (!receipt->simple_lsb_msb_decoder_disproven) {
        receipt->status =
            NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_DIFFERENTIAL;
        return 0;
    }
    return 1;
}

int nexus_v1_prs3_decoder_admission_evaluate(
    const Nexus_V1_Prs3DecoderAdmissionInput *input,
    Nexus_V1_Prs3DecoderAdmissionReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    reset_receipt(out_receipt,
                  NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_INPUT);
    if (!input) return 0;

    if (!bind_executable(input, out_receipt) ||
        !bind_menu_streams(input, out_receipt) ||
        !bind_differential_trials(input, out_receipt)) {
        return 0;
    }

    (void)nexus_v1_bpk_archive_prs3_decoded_output_proof_gate(
        input->menu_bpk, input->menu_bpk_size,
        out_receipt->first_stream_entry_index,
        input->decoded_output, input->decoded_output_size,
        input->decoded_output_fnv1a64,
        input->decoded_output_source_bound,
        input->original_saturn_provenance_verified,
        &out_receipt->output_proof);
    out_receipt->expected_output_bound =
        out_receipt->output_proof.decoded_output_proof_ready &&
        out_receipt->output_proof.decoded_output_sidecar_bound &&
        out_receipt->output_proof.status ==
            NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_SOURCE_BOUND_NO_RUNTIME;
    out_receipt->original_saturn_provenance_verified =
        input->original_saturn_provenance_verified ? 1 : 0;
    out_receipt->opcode_grammar_proven =
        input->opcode_grammar_proven ? 1 : 0;
    out_receipt->decoder_ready =
        out_receipt->expected_output_bound &&
        out_receipt->original_saturn_provenance_verified &&
        out_receipt->opcode_grammar_proven;
    out_receipt->decoder_promoted = 0;
    out_receipt->decoded_pixels_emitted = 0U;
    out_receipt->runtime_upload_permitted = 0;
    out_receipt->structure2_pixel_intake_permitted = 0;
    out_receipt->fallback_visuals_permitted = 0;

    if (!out_receipt->expected_output_bound ||
        !out_receipt->original_saturn_provenance_verified ||
        !out_receipt->opcode_grammar_proven) {
        out_receipt->status =
            NEXUS_V1_PRS3_DECODER_ADMISSION_READY_BLOCKED;
        return 1;
    }

    out_receipt->status =
        NEXUS_V1_PRS3_DECODER_ADMISSION_READY_DECODER;
    return 1;
}

const char *nexus_v1_prs3_decoder_admission_status_name(
    Nexus_V1_Prs3DecoderAdmissionStatus status)
{
    switch (status) {
    case NEXUS_V1_PRS3_DECODER_ADMISSION_READY_BLOCKED:
        return "ready-blocked";
    case NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_SOURCE:
        return "blocked-source";
    case NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_EXECUTABLE:
        return "blocked-executable";
    case NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_MENU_STREAM:
        return "blocked-menu-stream";
    case NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_DIFFERENTIAL:
        return "blocked-differential";
    case NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_OUTPUT_PROOF:
        return "blocked-output-proof";
    case NEXUS_V1_PRS3_DECODER_ADMISSION_READY_DECODER:
        return "ready-decoder";
    default:
        return "unknown";
    }
}
