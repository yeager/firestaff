#include "nexus_v1_prs3_loader_control_flow.h"

#include <string.h>

static void reset_receipt(Nexus_V1_Prs3LoaderControlReceipt *receipt,
                          Nexus_V1_Prs3LoaderControlStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
}

static int bind_dm_bin_control(
    const Nexus_V1_Prs3LoaderControlInput *input,
    Nexus_V1_Prs3LoaderControlReceipt *receipt)
{
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;

    if (!input->dm_bin || input->dm_bin_size == 0U ||
        !input->dm_bin_source_verified) {
        receipt->status = NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_INPUT;
        return 0;
    }
    receipt->dm_bin_source_verified = 1;
    if (!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            input->dm_bin, input->dm_bin_size, 1, &sh2) ||
        !sh2.sh2_nonzero_direct_byte_path_proven ||
        !sh2.sh2_nonzero_output_commit_reentry_proven ||
        !sh2.sh2_zero_side_two_byte_input_span_proven ||
        !sh2.sh2_zero_byte_merge_order_proven ||
        !sh2.sh2_zero_side_repeat_control_verified ||
        !sh2.sh2_zero_indexed_byte_control_operands_proven ||
        !sh2.sh2_zero_side_has_no_direct_output_store) {
        receipt->status =
            NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_EXECUTABLE;
        return 0;
    }

    receipt->executable_control_flow_bound = 1;
    receipt->callee_offset = sh2.v1_callee_offset;
    receipt->control_reentry_offset = sh2.control_reentry_offset;
    receipt->control_shift_offset = sh2.control_shift_offset;
    receipt->control_low_bit_test_offset = sh2.control_low_bit_test_offset;
    receipt->control_zero_branch_offset = sh2.control_zero_branch_offset;
    receipt->control_zero_branch_target_offset =
        sh2.control_zero_branch_target_offset;
    receipt->nonzero_counter_decrement_offset =
        sh2.nonzero_source_counter_decrement_offset;
    receipt->nonzero_input_read_offset = sh2.stream_byte_read_offset;
    receipt->nonzero_output_store_offset = sh2.output_byte_store_offset;
    receipt->nonzero_post_store_increment_offset =
        sh2.nonzero_post_store_r6_increment_offset;
    receipt->nonzero_reentry_branch_offset =
        sh2.nonzero_control_reentry_branch_offset;
    receipt->zero_counter_decrement_offset =
        sh2.zero_source_counter_decrement_offset;
    receipt->zero_first_input_read_offset = sh2.zero_first_byte_read_offset;
    receipt->zero_second_input_read_offset = sh2.zero_second_byte_read_offset;
    receipt->zero_merge_or_offset = sh2.zero_merge_or_offset;
    receipt->zero_indexed_byte_read_offset = sh2.zero_indexed_byte_read_offset;
    receipt->zero_repeat_branch_offset = sh2.zero_repeat_branch_offset;
    receipt->zero_outer_loop_branch_offset = sh2.zero_outer_loop_branch_offset;
    receipt->terminal_result_offset = sh2.terminal_result_offset;
    receipt->terminal_return_offset = sh2.terminal_return_offset;

    receipt->input_cursor_register = 12U;
    receipt->output_base_register = 13U;
    receipt->output_index_register = 6U;
    receipt->output_effective_index_register = 0U;
    receipt->remaining_source_counter_register = 14U;
    receipt->control_word_register = 11U;
    receipt->control_mask_register = 3U;
    receipt->output_window_mask_register = 5U;
    receipt->zero_compare_value_register = 1U;
    receipt->zero_repeat_counter_register = 10U;
    receipt->control_sentinel_word = sh2.control_sentinel_word;
    receipt->zero_upper_mask_word = sh2.zero_upper_mask_word;
    receipt->zero_index_mask_word = sh2.zero_index_mask_word;
    receipt->zero_side_linear_fnv1a64 = sh2.zero_side_linear_fnv1a64;
    receipt->nonzero_path_is_direct_source_to_output =
        sh2.sh2_nonzero_direct_byte_path_proven &&
        sh2.sh2_output_store_predecessor_verified &&
        sh2.sh2_nonzero_output_commit_reentry_proven;
    receipt->zero_side_two_source_bytes_merge =
        sh2.sh2_zero_side_two_byte_input_span_proven &&
        sh2.sh2_zero_byte_merge_order_proven;
    receipt->zero_side_indexed_output_window_read =
        sh2.sh2_zero_indexed_byte_control_operands_proven;
    receipt->zero_side_direct_output_store_absent =
        sh2.sh2_zero_side_has_no_direct_output_store;
    receipt->zero_side_copy_semantics_proven =
        sh2.zero_side_copy_or_backreference_proven ? 1 : 0;
    return 1;
}

static int bind_menu_bpk_streams(
    const Nexus_V1_Prs3LoaderControlInput *input,
    Nexus_V1_Prs3LoaderControlReceipt *receipt)
{
    Nexus_V1_BpkArchiveInfo archive;
    Nexus_V1_BpkPrs3StreamPlan plan;
    uint32_t index;

    if (!input->menu_bpk || input->menu_bpk_size == 0U ||
        !input->menu_bpk_source_verified) {
        receipt->status = NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_INPUT;
        return 0;
    }
    receipt->menu_bpk_source_verified = 1;
    if (nexus_v1_bpk_archive_parse(input->menu_bpk, input->menu_bpk_size,
                                   &archive) != 0 ||
        archive.prs3_payload_count == 0U) {
        receipt->status =
            NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_MENU_STREAM;
        return 0;
    }
    receipt->menu_prs3_stream_count = archive.prs3_payload_count;
    for (index = 0U; index < archive.candidate_offset_count; ++index) {
        if (nexus_v1_bpk_archive_prs3_stream_plan(
                input->menu_bpk, input->menu_bpk_size, index, &plan) ==
            NEXUS_V1_BPK_PRS3_STREAM_OK) {
            receipt->menu_streams_bound = 1;
            receipt->first_menu_entry_index = index;
            receipt->first_menu_stream_offset = plan.stream_offset;
            receipt->first_menu_stream_size = plan.stream_size;
            receipt->first_menu_expected_output_bytes =
                plan.expected_output_bytes;
            return 1;
        }
    }
    receipt->status = NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_MENU_STREAM;
    return 0;
}

int nexus_v1_prs3_loader_control_flow_probe(
    const Nexus_V1_Prs3LoaderControlInput *input,
    Nexus_V1_Prs3LoaderControlReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    reset_receipt(out_receipt,
                  NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_INPUT);
    if (!input) return 0;
    if (!bind_dm_bin_control(input, out_receipt) ||
        !bind_menu_bpk_streams(input, out_receipt)) {
        return 0;
    }

    out_receipt->expected_output_vector_bound =
        input->expected_output_vector_bound ? 1 : 0;
    out_receipt->opcode_grammar_reviewed =
        input->opcode_grammar_reviewed ? 1 : 0;
    out_receipt->original_saturn_execution_authenticated =
        input->original_saturn_execution_authenticated ? 1 : 0;
    out_receipt->decoder_implementation_permitted =
        out_receipt->expected_output_vector_bound &&
        out_receipt->opcode_grammar_reviewed &&
        out_receipt->original_saturn_execution_authenticated &&
        out_receipt->zero_side_copy_semantics_proven;
    out_receipt->decoder_promoted = 0;
    out_receipt->decoded_pixels_emitted = 0U;
    out_receipt->fallback_visuals_permitted = 0;
    out_receipt->status =
        out_receipt->decoder_implementation_permitted
            ? NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_VECTOR_PROOF
            : NEXUS_V1_PRS3_LOADER_CONTROL_READY_BLOCKED;
    return 1;
}

const char *nexus_v1_prs3_loader_control_status_name(
    Nexus_V1_Prs3LoaderControlStatus status)
{
    switch (status) {
    case NEXUS_V1_PRS3_LOADER_CONTROL_READY_BLOCKED:
        return "ready-blocked";
    case NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_EXECUTABLE:
        return "blocked-executable";
    case NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_MENU_STREAM:
        return "blocked-menu-stream";
    case NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_VECTOR_PROOF:
        return "blocked-vector-proof";
    default:
        return "unknown";
    }
}
