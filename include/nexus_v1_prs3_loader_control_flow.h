#ifndef NEXUS_V1_PRS3_LOADER_CONTROL_FLOW_H
#define NEXUS_V1_PRS3_LOADER_CONTROL_FLOW_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_prs3_capture_trace_schema.h"

typedef enum {
    NEXUS_V1_PRS3_LOADER_CONTROL_READY_BLOCKED = 0,
    NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_INPUT = 1,
    NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_EXECUTABLE = 2,
    NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_MENU_STREAM = 3,
    NEXUS_V1_PRS3_LOADER_CONTROL_BLOCKED_VECTOR_PROOF = 4
} Nexus_V1_Prs3LoaderControlStatus;

typedef struct {
    const uint8_t *dm_bin;
    size_t dm_bin_size;
    int dm_bin_source_verified;
    const uint8_t *menu_bpk;
    size_t menu_bpk_size;
    int menu_bpk_source_verified;
    int expected_output_vector_bound;
    int opcode_grammar_reviewed;
    int original_saturn_execution_authenticated;
} Nexus_V1_Prs3LoaderControlInput;

typedef struct {
    Nexus_V1_Prs3LoaderControlStatus status;
    int dm_bin_source_verified;
    int menu_bpk_source_verified;
    int executable_control_flow_bound;
    uint32_t callee_offset;
    uint32_t control_reentry_offset;
    uint32_t control_shift_offset;
    uint32_t control_low_bit_test_offset;
    uint32_t control_zero_branch_offset;
    uint32_t control_zero_branch_target_offset;
    uint32_t nonzero_counter_decrement_offset;
    uint32_t nonzero_input_read_offset;
    uint32_t nonzero_output_store_offset;
    uint32_t nonzero_post_store_increment_offset;
    uint32_t nonzero_reentry_branch_offset;
    uint32_t zero_counter_decrement_offset;
    uint32_t zero_first_input_read_offset;
    uint32_t zero_second_input_read_offset;
    uint32_t zero_merge_or_offset;
    uint32_t zero_indexed_byte_read_offset;
    uint32_t zero_repeat_branch_offset;
    uint32_t zero_outer_loop_branch_offset;
    uint32_t terminal_result_offset;
    uint32_t terminal_return_offset;
    uint32_t input_cursor_register;
    uint32_t output_base_register;
    uint32_t output_index_register;
    uint32_t output_effective_index_register;
    uint32_t remaining_source_counter_register;
    uint32_t control_word_register;
    uint32_t control_mask_register;
    uint32_t output_window_mask_register;
    uint32_t zero_compare_value_register;
    uint32_t zero_repeat_counter_register;
    uint16_t control_sentinel_word;
    uint16_t zero_upper_mask_word;
    uint16_t zero_index_mask_word;
    uint64_t zero_side_linear_fnv1a64;
    int nonzero_path_is_direct_source_to_output;
    int zero_side_two_source_bytes_merge;
    int zero_side_indexed_output_window_read;
    int zero_side_direct_output_store_absent;
    int zero_side_copy_semantics_proven;
    int menu_streams_bound;
    uint32_t menu_prs3_stream_count;
    uint32_t first_menu_entry_index;
    uint32_t first_menu_stream_offset;
    uint32_t first_menu_stream_size;
    uint32_t first_menu_expected_output_bytes;
    int expected_output_vector_bound;
    int opcode_grammar_reviewed;
    int original_saturn_execution_authenticated;
    int decoder_implementation_permitted;
    int decoder_promoted;
    uint32_t decoded_pixels_emitted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3LoaderControlReceipt;

int nexus_v1_prs3_loader_control_flow_probe(
    const Nexus_V1_Prs3LoaderControlInput *input,
    Nexus_V1_Prs3LoaderControlReceipt *out_receipt);

const char *nexus_v1_prs3_loader_control_status_name(
    Nexus_V1_Prs3LoaderControlStatus status);

#endif
