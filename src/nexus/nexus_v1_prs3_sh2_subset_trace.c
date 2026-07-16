#include "nexus_v1_prs3_sh2_subset_trace.h"

#include <stdlib.h>
#include <string.h>

#define NEXUS_V1_PRS3_SUBSET_PAYLOAD_RAM UINT32_C(0x06020000)
#define NEXUS_V1_PRS3_SUBSET_OUTPUT_RAM UINT32_C(0x06030000)
#define NEXUS_V1_PRS3_SUBSET_OUTPUT_WINDOW UINT32_C(4096)
#define NEXUS_V1_PRS3_SUBSET_OUTPUT_MASK UINT32_C(0x0fff)
#define NEXUS_V1_PRS3_SUBSET_DEFAULT_STEPS UINT32_C(200000)

static uint64_t fnv1a64_update(uint64_t hash, uint32_t value)
{
    hash ^= (uint64_t)(value & 0xffU);
    hash *= UINT64_C(1099511628211);
    hash ^= (uint64_t)((value >> 8) & 0xffU);
    hash *= UINT64_C(1099511628211);
    hash ^= (uint64_t)((value >> 16) & 0xffU);
    hash *= UINT64_C(1099511628211);
    hash ^= (uint64_t)((value >> 24) & 0xffU);
    hash *= UINT64_C(1099511628211);
    return hash;
}

static uint64_t fnv1a64_bytes(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    if (!data || size == 0U) return 0U;
    for (index = 0U; index < size; ++index) {
        hash ^= (uint64_t)data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void reset_receipt(Nexus_V1_Prs3Sh2SubsetTraceReceipt *receipt,
                          Nexus_V1_Prs3Sh2SubsetTraceStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->payload_ram_address = NEXUS_V1_PRS3_SUBSET_PAYLOAD_RAM;
    receipt->output_ram_address = NEXUS_V1_PRS3_SUBSET_OUTPUT_RAM;
    receipt->input_cursor_register = 12U;
    receipt->output_base_register = 13U;
    receipt->output_index_register = 6U;
    receipt->remaining_source_counter_register = 14U;
    receipt->control_word_register = 11U;
    receipt->output_window_mask_register = 5U;
    receipt->fallback_visuals_permitted = 0;
}

static int bind_control_and_stream(
    const Nexus_V1_Prs3Sh2SubsetTraceInput *input,
    Nexus_V1_Prs3Sh2SubsetTraceReceipt *receipt,
    Nexus_V1_BpkPrs3StreamPlan *out_plan)
{
    Nexus_V1_Prs3LoaderControlInput control_input;
    Nexus_V1_Prs3LoaderControlReceipt control;
    uint32_t index;

    if (!input->dm_bin || input->dm_bin_size == 0U ||
        !input->menu_bpk || input->menu_bpk_size == 0U ||
        !input->dm_bin_source_verified ||
        !input->menu_bpk_source_verified) {
        receipt->status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_INPUT;
        return 0;
    }
    receipt->dm_bin_source_verified = 1;
    receipt->menu_bpk_source_verified = 1;

    memset(&control_input, 0, sizeof(control_input));
    control_input.dm_bin = input->dm_bin;
    control_input.dm_bin_size = input->dm_bin_size;
    control_input.dm_bin_source_verified = 1;
    control_input.menu_bpk = input->menu_bpk;
    control_input.menu_bpk_size = input->menu_bpk_size;
    control_input.menu_bpk_source_verified = 1;
    if (!nexus_v1_prs3_loader_control_flow_probe(&control_input, &control) ||
        !control.executable_control_flow_bound ||
        !control.zero_side_direct_output_store_absent ||
        control.decoder_promoted ||
        control.fallback_visuals_permitted) {
        receipt->status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_CONTROL_FLOW;
        return 0;
    }
    receipt->control_flow_bound = 1;
    receipt->first_pc = control.control_reentry_offset;

    if (input->entry_index != 0U) {
        index = input->entry_index;
    } else {
        index = control.first_menu_entry_index;
    }
    if (nexus_v1_bpk_archive_prs3_stream_plan(
            input->menu_bpk, input->menu_bpk_size, index, out_plan) !=
            NEXUS_V1_BPK_PRS3_STREAM_OK ||
        out_plan->body_size == 0U ||
        out_plan->expected_output_bytes == 0U) {
        receipt->status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_MENU_STREAM;
        return 0;
    }
    receipt->menu_stream_bound = 1;
    receipt->entry_index = index;
    receipt->stream_offset = out_plan->stream_offset;
    receipt->stream_size = out_plan->stream_size;
    receipt->body_offset = out_plan->stream_offset;
    receipt->body_size = out_plan->stream_size;
    receipt->source_includes_frame_word = 1;
    receipt->expected_output_bytes = out_plan->expected_output_bytes;
    if (input->dm_bin_size <= 85537U ||
        input->dm_bin[85468U] != 0x2aU ||
        input->dm_bin[85469U] != 0x20U ||
        input->dm_bin[85528U] != 0x23U ||
        input->dm_bin[85529U] != 0x10U ||
        input->dm_bin[85530U] != 0x2aU ||
        input->dm_bin[85531U] != 0x10U ||
        input->dm_bin[85532U] != 0x7aU ||
        input->dm_bin[85533U] != 0x01U ||
        input->dm_bin[85536U] != 0x26U ||
        input->dm_bin[85537U] != 0x59U) {
        receipt->status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_CONTROL_FLOW;
        return 0;
    }
    receipt->linear_output_store_instructions_proven = 1;
    return 1;
}

int nexus_v1_prs3_sh2_subset_trace_run(
    const Nexus_V1_Prs3Sh2SubsetTraceInput *input,
    Nexus_V1_Prs3Sh2SubsetTraceReceipt *out_receipt)
{
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3Sh2SubsetTraceReceipt receipt;
    uint8_t *window;
    uint8_t *output;
    const uint8_t *src;
    uint32_t src_offset = 0U;
    uint32_t remaining;
    uint32_t r6 = 0U;
    uint32_t r10 = 0U;
    uint32_t r11 = 0U;
    uint32_t output_stores = 0U;
    int first_nonzero_seen = 0;
    uint32_t steps = 0U;
    uint32_t max_steps;
    uint64_t control_hash = UINT64_C(1469598103934665603);

    if (!out_receipt) return 0;
    reset_receipt(&receipt, NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_INPUT);
    if (!input) {
        *out_receipt = receipt;
        return 0;
    }
    if (!bind_control_and_stream(input, &receipt, &plan)) {
        *out_receipt = receipt;
        return 0;
    }

    window = (uint8_t *)calloc(NEXUS_V1_PRS3_SUBSET_OUTPUT_WINDOW, 1U);
    output = (uint8_t *)calloc(plan.expected_output_bytes, 1U);
    if (!window || !output) {
        receipt.status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION;
        free(window);
        free(output);
        *out_receipt = receipt;
        return 0;
    }

    src = input->menu_bpk + plan.stream_offset;
    remaining = plan.stream_size;
    max_steps = input->max_steps ? input->max_steps :
        NEXUS_V1_PRS3_SUBSET_DEFAULT_STEPS;
    receipt.first_input_offset = UINT32_MAX;

    while (steps < max_steps && output_stores < plan.expected_output_bytes) {
        ++steps;
        receipt.last_pc = 85428U;
        r11 >>= 1;
        control_hash = fnv1a64_update(control_hash, r11);
        if ((r11 & 0x0100U) == 0U) {
            if (remaining == 0U) {
                receipt.status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION;
                break;
            }
            receipt.last_pc = 85440U;
            if (receipt.first_input_offset == UINT32_MAX) {
                receipt.first_input_offset = src_offset;
            }
            r11 = (uint32_t)src[src_offset++] | 0xff00U;
            --remaining;
            ++receipt.refill_count;
            ++receipt.input_read_bytes;
            receipt.last_input_offset = src_offset - 1U;
            control_hash = fnv1a64_update(control_hash, r11);
        }

        receipt.last_pc = 85450U;
        if ((r11 & 1U) != 0U) {
            uint8_t value;
            ++receipt.nonzero_control_count;
            if (remaining == 0U) {
                receipt.status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION;
                break;
            }
            receipt.last_pc = 85460U;
            if (receipt.first_input_offset == UINT32_MAX) {
                receipt.first_input_offset = src_offset;
            }
            value = src[src_offset++];
            --remaining;
            ++receipt.input_read_bytes;
            receipt.last_input_offset = src_offset - 1U;
            if (!first_nonzero_seen) {
                receipt.first_nonzero_input_byte = value;
                receipt.first_nonzero_output_byte = value;
                first_nonzero_seen = 1;
            }
            receipt.last_pc = 85464U;
            window[r6 & NEXUS_V1_PRS3_SUBSET_OUTPUT_MASK] = value;
            output[r10] = value;
            ++output_stores;
            ++receipt.output_store_count;
            ++receipt.ring_store_count;
            ++receipt.linear_output_store_count;
            ++receipt.output_index_advance_count;
            r6 = (r6 + 1U) & NEXUS_V1_PRS3_SUBSET_OUTPUT_MASK;
            ++r10;
        } else {
            uint32_t b0, b1, merged, low, end;
            ++receipt.zero_control_count;
            if (remaining < 2U) {
                receipt.status = NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION;
                break;
            }
            receipt.last_pc = 85484U;
            if (receipt.first_input_offset == UINT32_MAX) {
                receipt.first_input_offset = src_offset;
            }
            b0 = src[src_offset++];
            receipt.last_pc = 85488U;
            b1 = src[src_offset++];
            remaining -= 2U;
            receipt.input_read_bytes += 2U;
            receipt.last_input_offset = src_offset - 1U;
            merged = b0 | ((b1 << 4U) & 0x0f00U);
            low = b1 & 0x0fU;
            end = merged + low + 2U;
            ++receipt.zero_merge_count;
            if (receipt.zero_merge_count == 1U) {
                receipt.first_zero_byte0 = b0;
                receipt.first_zero_byte1 = b1;
                receipt.first_zero_merged_value = merged;
            }
            if (merged > end) {
                continue;
            }
            for (;;) {
                uint32_t r0 = merged & NEXUS_V1_PRS3_SUBSET_OUTPUT_MASK;
                uint8_t observed = window[r0];
                uint32_t old_r6 = r6;
                ++receipt.zero_indexed_read_count;
                window[old_r6 & NEXUS_V1_PRS3_SUBSET_OUTPUT_MASK] =
                    observed;
                output[r10] = observed;
                ++output_stores;
                ++receipt.output_store_count;
                ++receipt.ring_store_count;
                ++receipt.linear_output_store_count;
                ++receipt.output_index_advance_count;
                r6 = (r6 + 1U) & NEXUS_V1_PRS3_SUBSET_OUTPUT_MASK;
                ++merged;
                ++r10;
                if (output_stores >= plan.expected_output_bytes ||
                    merged > end) {
                    break;
                }
            }
        }
    }

    receipt.executed_steps = steps;
    receipt.final_r12_offset = src_offset;
    receipt.final_r14_remaining = remaining;
    receipt.final_r6_index = r6;
    receipt.final_r10_output_offset = r10;
    receipt.final_r11_control_word = r11;
    receipt.output_prefix_fnv1a64 = fnv1a64_bytes(output, output_stores);
    receipt.control_trace_fnv1a64 = control_hash;
    if (receipt.first_input_offset == UINT32_MAX) {
        receipt.first_input_offset = 0U;
    }
    receipt.full_expected_output_observed =
        output_stores == plan.expected_output_bytes;
    receipt.positive_output_vector_bound =
        receipt.full_expected_output_observed &&
        receipt.input_read_bytes <= plan.body_size;
    receipt.zero_side_copy_semantics_proven =
        receipt.zero_control_count > 0U &&
        receipt.zero_indexed_read_count > 0U &&
        receipt.full_expected_output_observed;
    receipt.original_saturn_execution_authenticated = 0;
    receipt.opcode_grammar_proven = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    if (receipt.status != NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION) {
        receipt.status = NEXUS_V1_PRS3_SH2_SUBSET_READY_BLOCKED;
    }
    free(window);
    free(output);
    *out_receipt = receipt;
    return receipt.control_flow_bound && receipt.menu_stream_bound;
}

const char *nexus_v1_prs3_sh2_subset_trace_status_name(
    Nexus_V1_Prs3Sh2SubsetTraceStatus status)
{
    switch (status) {
    case NEXUS_V1_PRS3_SH2_SUBSET_READY_BLOCKED:
        return "ready-blocked";
    case NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_CONTROL_FLOW:
        return "blocked-control-flow";
    case NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_MENU_STREAM:
        return "blocked-menu-stream";
    case NEXUS_V1_PRS3_SH2_SUBSET_BLOCKED_EXECUTION:
        return "blocked-execution";
    default:
        return "unknown";
    }
}
