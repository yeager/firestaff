#ifndef NEXUS_V1_PRS3_SH2_SUBSET_TRACE_H
#define NEXUS_V1_PRS3_SH2_SUBSET_TRACE_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_prs3_loader_control_flow.h"

typedef enum {
    NEXUS_V1_PRS3_SH2_SUBSET_READY_BLOCKED = 0,
    NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_INPUT = 1,
    NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_CONTROL_FLOW = 2,
    NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_MENU_STREAM = 3,
    NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION = 4
} Nexus_V1_Prs3Sh2SubsetTraceStatus;

typedef struct {
    const uint8_t *dm_bin;
    size_t dm_bin_size;
    int dm_bin_source_verified;
    const uint8_t *menu_bpk;
    size_t menu_bpk_size;
    int menu_bpk_source_verified;
    uint32_t entry_index;
    uint32_t max_steps;
} Nexus_V1_Prs3Sh2SubsetTraceInput;

typedef struct {
    Nexus_V1_Prs3Sh2SubsetTraceStatus status;
    int dm_bin_source_verified;
    int menu_bpk_source_verified;
    int control_flow_bound;
    int menu_stream_bound;
    uint32_t entry_index;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint32_t body_offset;
    uint32_t body_size;
    int source_includes_frame_word;
    int linear_output_store_instructions_proven;
    uint32_t expected_output_bytes;
    uint32_t payload_ram_address;
    uint32_t output_ram_address;
    uint32_t first_pc;
    uint32_t last_pc;
    uint32_t executed_steps;
    uint32_t input_cursor_register;
    uint32_t output_base_register;
    uint32_t output_index_register;
    uint32_t remaining_source_counter_register;
    uint32_t control_word_register;
    uint32_t output_window_mask_register;
    uint32_t first_input_offset;
    uint32_t last_input_offset;
    uint32_t input_read_bytes;
    uint32_t nonzero_control_count;
    uint32_t zero_control_count;
    uint32_t refill_count;
    uint32_t output_store_count;
    uint32_t ring_store_count;
    uint32_t linear_output_store_count;
    uint32_t output_index_advance_count;
    uint32_t zero_merge_count;
    uint32_t zero_indexed_read_count;
    uint32_t first_nonzero_input_byte;
    uint32_t first_nonzero_output_byte;
    uint32_t first_zero_byte0;
    uint32_t first_zero_byte1;
    uint32_t first_zero_merged_value;
    uint32_t final_r12_offset;
    uint32_t final_r14_remaining;
    uint32_t final_r6_index;
    uint32_t final_r10_output_offset;
    uint32_t final_r11_control_word;
    uint64_t output_prefix_fnv1a64;
    uint64_t control_trace_fnv1a64;
    int full_expected_output_observed;
    int positive_output_vector_bound;
    int zero_side_copy_semantics_proven;
    int original_saturn_execution_authenticated;
    int opcode_grammar_proven;
    int decoder_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Sh2SubsetTraceReceipt;

int nexus_v1_prs3_sh2_subset_trace_run(
    const Nexus_V1_Prs3Sh2SubsetTraceInput *input,
    Nexus_V1_Prs3Sh2SubsetTraceReceipt *out_receipt);

const char *nexus_v1_prs3_sh2_subset_trace_status_name(
    Nexus_V1_Prs3Sh2SubsetTraceStatus status);

#endif
